//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#include "npc_surface.h"

#include "ai_hull.h"
#include "saverestore_utlvector.h"
#include "dt_utlvector_send.h"
#include "physics_saverestore.h"
#include "vphysics/constraints.h"
#include "vcollide_parse.h"
#include "ragdoll_shared.h"
#include "physics_prop_ragdoll.h"
#include "collisionutils.h"
#include "te_effect_dispatch.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
//
// CNPC_Surface
//

CUtlVector<CNPC_Surface*> CNPC_Surface::s_BlobInstances;
float CNPC_Surface::INVALID_PARTICLE_RADIUS = -1;
float CNPC_Surface::DEFAULT_PARTICLE_RADIUS = 6.5f;


//---------------------------------------------------------
// Custom Client entity
//---------------------------------------------------------

// TODO: For the position, we should send a central coordinate, and
// vectors that are distance from the center, instead of sending the
// full vectors.

IMPLEMENT_SERVERCLASS_ST(CNPC_Surface, DT_NPC_Surface)
	// TODO: Shouldn't we only send as many elements as there are in the vector?

	/* Now goes through the network bypass
	SendPropUtlVector(
		SENDINFO_UTLVECTOR( m_vecSurfacePos ),
		MAX_SURFACE_ELEMENTS, // max elements4
		SendPropVector( NULL, 0, sizeof( Vector ), -1, SPROP_COORD )),*/
	SendPropUtlVector(
		SENDINFO_UTLVECTOR( m_iParticlePositionIndex ),
		MAX_SURFACE_ELEMENTS, // max elements4
		SendPropInt( NULL, 0, sizeof( uint16 ), BLOB_MAX_LEVEL_PARTICLES_BITS, SPROP_UNSIGNED )),
	/* Now goes through the network bypass
	SendPropUtlVector(
		SENDINFO_UTLVECTOR( m_flSurfaceR ),
		MAX_SURFACE_ELEMENTS, // max elements
		SendPropFloat( NULL, 0, sizeof( float ), 6, 0, 0.0, 2.0 )),*/

	SendPropInt( SENDINFO( m_nActiveParticles ), MAX_SURFACE_ELEMENTS_BITS, SPROP_UNSIGNED ),
	SendPropFloat( SENDINFO( m_flRadius ), 12, 0, 0.0, 100.0 ),
END_SEND_TABLE()


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_Surface )	
	//DEFINE_UTLVECTOR( m_vecSurfacePos,	FIELD_POSITION_VECTOR ),
	//DEFINE_UTLVECTOR( m_iParticlePositionIndex, FIELD_SHORT ),
	//DEFINE_UTLVECTOR( m_flSurfaceR,	FIELD_FLOAT ),

	DEFINE_KEYFIELD( m_nActiveParticles, FIELD_INTEGER, "particlecount" ),
	DEFINE_KEYFIELD( m_flRadius, FIELD_FLOAT, "particle_radius" ),
END_DATADESC()

//-------------------------------------


// TODO: These should probably be moved into demomonster.
ConVar	sv_surface_tension( "surface_tension", "5", 0, "How strong the surface tries to keep its shape" );
ConVar	sv_surface_ideal( "surface_ideal", "2", 0, "ideal distance (N * radius) between each sphere" );
ConVar	sv_surface_nearby( "surface_nearby", "3", 0, "acceptable distance (N * radius) between each and still be considered touching" );
ConVar	sv_surface_scale( "surface_scale", "0.5" );
ConVar	sv_surface_radius_multiplier( "surface_radius_multiplier", "1", 0, "Multiplies each surface sphere's radius on spawn" );

ConVar	sv_surface_radius("surface_radius", "6.5", 0, "Radius of each sphere");//theaperturecat


ConVar	sv_lj_strength( "lj_strength", "1", 0 );



//-------------------------------------


/*

BEGIN_SIMPLE_DATADESC( CSurfaceController )

	DEFINE_FIELD( m_vecAngular, FIELD_VECTOR ),
	DEFINE_FIELD( m_vecLinear, FIELD_VECTOR ),
	DEFINE_FIELD( m_fIsStopped, FIELD_BOOLEAN ),

END_DATADESC()
*/


//-------------------------------------
// Purpose: Initialize the custom schedules
//-------------------------------------

/*

Typical Construction/Destruction Sequence:

* Constructor
* Spawn (Does not get called on a reload)
* PreCache
* CreateVPhysics
* OnRestore (Only gets called on a reload)
* Activate
* (RunAI/PostPhysFrame/Other) - Here the physics and ai is running
* CleanupOnDeath (Happens if we die)
* UpdateOnRemove
* DestroyVPhysics - I call this from update on remove if CreateVPhysics was called
* Destructor

The following sequence also happens a lot during preloading:
* Constructor
* PreCache - sometimes doesn't occur.
* UpdateOnRemove
* Destructor


For consistency and memory safety, we should try to create and destroy all
physics related stuff in CreateVPhysics and DestroyVPhysics.

I don't know what the best place to do AI initialization is. Right now,
it is in Spawn, but it doesn't get called on Reload.

*/


//-------------------------------------


CNPC_Surface::CNPC_Surface( void )
{
	m_physicsCreated = false;
	//m_vecSurfacePos.EnsureCount( MAX_SURFACE_ELEMENTS );
	m_iParticlePositionIndex.EnsureCount( MAX_SURFACE_ELEMENTS );
	//m_flSurfaceR.EnsureCount( MAX_SURFACE_ELEMENTS );

	m_flRadius = DEFAULT_PARTICLE_RADIUS;

	s_BlobInstances.AddToTail( this );
}


// TODO (Ilya): This doesn't get called on restore (Activate gets called instead).
// This might cause problems because some important things may not get initialized.
void CNPC_Surface::Spawn()
{
	Precache();

	BaseClass::Spawn();

	SetModel( "models/blob.mdl" );

	// TODO (Ilya): I think this doesn't need to be here because I copied it to VPhysics, or it shouldn't be there =)
	SetHullType( HULL_TINY_FLUID );
	SetHullSizeNormal();

	// setup model
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_FORCE_WORLD_ALIGNED | FSOLID_NOT_STANDABLE );

	AddSolidFlags( FSOLID_CUSTOMRAYTEST | FSOLID_CUSTOMBOXTEST );

	AddEFlags( EFL_NO_DISSOLVE );
	SetBloodColor( BLOOD_COLOR_BLOB );
	ClearEffects();
	m_iHealth			= 200;
	m_flFieldOfView		= VIEW_FIELD_FULL;
	m_NPCState			= NPC_STATE_NONE;

	SetAbsAngles( QAngle( 0, 0, 0 ) );
	//SetAbsOrigin( Vector( 0, 0, 0 ) );

	m_flRadius  = ( m_flRadius == INVALID_PARTICLE_RADIUS ) ? DEFAULT_PARTICLE_RADIUS : m_flRadius;
	m_flRadius *= sv_surface_radius_multiplier.GetFloat();

	m_vecStart = GetAbsOrigin( );
	for (int i = 0; i < MAX_SURFACE_ELEMENTS; i++)
	{
		//BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ) = m_vecStart + Vector( RandomFloat( -1, 1 ), RandomFloat( -1, 1 ), RandomFloat( 0, 2 ) ) * m_flRadius;
		//m_flSurfaceV[i] = 0.0f;
		//m_flSurfaceR[i] = 1.0f;
		m_iParticlePositionIndex[i] = 0;
		//m_iType[i] = 0;
	}

	NPCInit();
}

void CNPC_Surface::Precache()
{
	PrecacheModel( "models/blob.mdl" );
	PrecacheEffect( "watersplash" );
	BaseClass::Precache();
}

void CNPC_Surface::Activate( void )
{
	BaseClass::Activate();
}

//-------------------------------------
// TODO: Shouldn't we do something smarter here?
// Perhaps we should reuse some of the spawn code...
// set type, surface velocities, etc...
void CNPC_Surface::Teleport( const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity )
{
	BaseClass::Teleport( newPosition, newAngles, newVelocity );

	m_vecStart = GetAbsOrigin();

	for ( int i = 0; i < m_nActiveParticles; i++)
	{
		BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ) = GetAbsOrigin() + Vector( RandomFloat( -1, 1 ), RandomFloat( -1, 1 ), RandomFloat( 0, 2 ) ) * m_flRadius;
		m_vecPhysParticles[i]->SetPosition( BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ), QAngle( 0.0f, 0.0f, 0.0f ), true );
	}
}

//-----------------------------------
// This receives notifications when the physics system has completed a physics simulation frame.
// TODO: It would be nice to add a hook to VPhysics that allows us to register a listener that listens
// to the beginning and end of each frame.
//-----------------------------------


/*void CNPC_Surface::PostPhysFrame_Static( float deltaTime )
{
	VPROF_BUDGET( "CNPC_Surface::PostPhysFrame_Static", VPROF_BUDGETGROUP_PHYSICS );
	// NOTE: This is not thread safe if blobs can be created/destroyed by different threads
	for(int i = 0; i < s_BlobInstances.Count(); i++)
	{
		s_BlobInstances[i]->PostPhysFrame( deltaTime );
	}
}*/

/*void CNPC_Surface::PostPhysFrame( float deltaTime )
{
	// Do nothing.
}*/



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------


void CNPC_Surface::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	BaseClass::VPhysicsCollision( index, pEvent );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CNPC_Surface::Save( ISave &save )
{
	if( !BaseClass::Save( save ) )
		return 0;

	//save blob particle positions
	save.StartBlock( "SurfaceParticleBypassData" );

	short iParticleCount = m_nActiveParticles;
	save.WriteShort( &iParticleCount );

	for( int i = 0; i != m_nActiveParticles; ++i )
	{
		save.WritePositionVector( BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ) );
	}

	for( int i = 0; i != m_nActiveParticles; ++i )
	{
		save.WriteFloat( &BLOBPARTICLERADIUS( m_iParticlePositionIndex[i] ) );
	}

	for( int i = 0; i != m_nActiveParticles; ++i )
	{
		save.WriteVector( BLOBPARTICLECLOSESTSURFDIR( m_iParticlePositionIndex[i] ) );
	}

	save.EndBlock();

	return 1;
}

int	CNPC_Surface::Restore( IRestore &restore )
{
	if( !BaseClass::Restore( restore ) )
		return 0;

	char szBlockName[SIZE_BLOCK_NAME_BUF];
	restore.StartBlock( szBlockName );

	if( !FStrEq( szBlockName, "SurfaceParticleBypassData" ) ) // Loading a save without particle bypass data?
		return 1;

	short iParticleCount;
	restore.ReadShort( &iParticleCount );

	if( iParticleCount != 0 )
	{
		//Vector *pRead = (Vector *)stackalloc( sizeof( Vector ) * iParticleCount );
		//restore.ReadVector( pRead, iParticleCount );

		m_pRestorePositions = new Vector [iParticleCount];
		restore.ReadPositionVector( m_pRestorePositions, iParticleCount );

		m_pRestoreRadii = new float [iParticleCount];
		restore.ReadFloat( m_pRestoreRadii, iParticleCount );

		m_pRestoreClosestSurfDirs = new Vector [iParticleCount];
		restore.ReadVector( m_pRestoreClosestSurfDirs, iParticleCount );
	}
	

	restore.EndBlock();

	return 1;
}

void CNPC_Surface::OnSave( IEntitySaveUtils *pUtils )
{
	// TODO: copy current physics state to the temp arrays

	BaseClass::OnSave( pUtils );
}

void CNPC_Surface::OnRestore( void )
{
	BaseClass::OnRestore();

	// TODO: recreate physics state from the temp arrays

	Assert( (m_nActiveParticles == 0) || (m_pRestorePositions != NULL) );
	if( m_pRestorePositions )
	{
		for( int i = 0; i != m_nActiveParticles; ++i )
		{
			BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ) = m_pRestorePositions[i];
			m_vecPhysParticles[i]->SetPosition( m_pRestorePositions[i], GetAbsAngles(), true );
		}
		for( int i = 0; i != m_nActiveParticles; ++i )
		{
			BLOBPARTICLERADIUS( m_iParticlePositionIndex[i] ) = m_pRestoreRadii[i];
		}
		for( int i = 0; i != m_nActiveParticles; ++i )
		{
			BLOBPARTICLECLOSESTSURFDIR( m_iParticlePositionIndex[i] ) = m_pRestoreClosestSurfDirs[i];
		}
		
		delete []m_pRestorePositions;
		m_pRestorePositions = NULL;

		delete []m_pRestoreRadii;
		m_pRestoreRadii = NULL;

		delete []m_pRestoreClosestSurfDirs;
		m_pRestoreClosestSurfDirs = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns this monster's place in the relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_Surface::Classify( void )
{
	return CLASS_BLOB; 
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

bool CNPC_Surface::CreateVPhysics()
{
	// TODO (Ilya): I'm not sure if we should call this and when we should call it (it initializes the VPhysics hull)
	//CAI_BaseNPC::CreateVPhysics();

	// TODO (Ilya): I threw this in here to make sure the hull is correct. Not sure if it should be here or not!!!
	SetHullType( HULL_TINY_FLUID );
	SetHullSizeNormal();

	m_physicsCreated = true;
	// NOTE: This is not thread safe if blobs can be created/destroyed by different threads

	CreateBlobPhysics();

	return true;
}

void CNPC_Surface::CreateBlobPhysics()
{
	// setup individual spheres
	m_vecPhysParticles.EnsureCapacity( MAX_SURFACE_ELEMENTS );
	m_iParticlePositionIndex.EnsureCapacity( MAX_SURFACE_ELEMENTS );

	for (int i = 0; i < m_nActiveParticles; i++)
	{
		IPhysicsObject *pPhysObject = CreateParticlePhysics();
		//pPhysObject->SetPosition( BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ), GetAbsAngles(), true );
		Vector vVelocity = Vector( RandomFloat( -1, 1 ), RandomFloat( -1, 1 ), RandomFloat( -1, 1 ) ) * 10.0f;
		pPhysObject->SetVelocity( &vVelocity, NULL );
		m_vecPhysParticles.AddToTail(pPhysObject);
	}

	// If any physics objects failed to be created, this adjusts m_nActiveParticles
	m_nActiveParticles = m_vecPhysParticles.Count();

	if( m_nActiveParticles != 0 )
		VPhysicsSetObject( m_vecPhysParticles[0] );
}

//-----------------------------------------------------------------------------
// Purpose: return a list of all the physics objects
//-----------------------------------------------------------------------------

int CNPC_Surface::VPhysicsGetObjectList( IPhysicsObject **pList, int listMax )
{
	Assert( m_nActiveParticles == m_vecPhysParticles.Count() );

	// TODO: Verify that it is Ok to start iterating with particle 0, in other words,
	// if vphysics expects us to return the physObject with this call

	int count = 0;
	for ( int i = 0; count < listMax && i < m_nActiveParticles; i++ )
	{
		Assert( m_vecPhysParticles[i] != NULL );
		Assert( m_iParticlePositionIndex[i] == s_ExpandedParticleData[ m_vecPhysParticles[i]->GetGameIndex() ].iNetworkBypassIndex );

		if ( BLOBPARTICLERADIUS( m_iParticlePositionIndex[i] ) > 0.0f )
		{
			pList[count++] = m_vecPhysParticles[i];
		}
	}

	return count;
}


bool CNPC_Surface::VPhysicsIsFlesh( void )
{
	return false;
}


void CNPC_Surface::CleanupOnDeath(CBaseEntity *pCulprit, bool bFireDeathOutput)
{
	CAI_BaseNPC::CleanupOnDeath(pCulprit, bFireDeathOutput);
}

void CNPC_Surface::VPhysicsDestroyObject()
{
	// NOTE: This gets called to destroy the root object.
	// It keeps getting called when the hull is adjusted.
	// For some reason, when we restore, this seems to happen
	// a lot more than when we spawn fresh.

	for(int i = m_vecPhysParticles.Count(); --i >=0;  )
	{
		DestroyParticlePhysics( m_vecPhysParticles[i] );
	}
	VPhysicsSetObject( NULL );

	m_vecPhysParticles.RemoveAll();
	//m_nActiveParticles = 0;
}

void CNPC_Surface::UpdateOnRemove()
{
	s_BlobInstances.FindAndRemove( this );
	CBaseEntity::UpdateOnRemove();
}


// This gets called on each particle after vphysics finishes.
// The baseentity_shared sets angles and positions here, but
// we don't want that since we have a single position for each particle.
// TODO: This might be a good place to update the position of particles
// in our internal array.
void CNPC_Surface::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	//CBaseEntity::VPhysicsUpdate(pPhysics);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

void CNPC_Surface::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	CDefaultPlayerPickupVPhysics::OnPhysGunPickup( pPhysGunUser, reason );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

void CNPC_Surface::OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t Reason )
{
	CDefaultPlayerPickupVPhysics::OnPhysGunDrop( pPhysGunUser, Reason );
}

//-----------------------------------------------------------------------------
// Purpose: Detect that the physgun is trying to punt us.  Currently guess about damage
//-----------------------------------------------------------------------------

bool CNPC_Surface::OnAttemptPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	if ( reason == PUNTED_BY_CANNON )
	{
		Vector forward;
		pPhysGunUser->EyeVectors( &forward );

		Vector start, end;
		start = pPhysGunUser->Weapon_ShootPosition();

		float d1, d2;
		for ( int i = 0; i < m_nActiveParticles; i++ )
		{
			if ( ( BLOBPARTICLERADIUS( m_iParticlePositionIndex[i] ) > 0.0f ) &&
				 IntersectInfiniteRayWithSphere( start, forward, BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ), m_flRadius * 3, &d1, &d2 ))
			{
				Vector p1 = start + d1 * forward;

				// no idea what sort of forces to use when punting
				// also, forceoffset just applies a spin, it doesn't act like being hit with a larger sphere
				m_vecPhysParticles[i]->ApplyForceOffset( forward * 1000.0f, p1 );
			}
		}

		return false;
	}
	return CDefaultPlayerPickupVPhysics::OnAttemptPhysGunPickup( pPhysGunUser, reason );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

bool CNPC_Surface::TestCollision( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr )
{
	// FIXME: this needs an acceleration structure (box tree should be fine initially)
	int nLastHit = -1;

	if (ray.m_IsRay)
	{
		float d1, d2;

		for ( int i = 0; i < m_nActiveParticles; i++ )
		{
			if ( ( BLOBPARTICLERADIUS( m_iParticlePositionIndex[i] ) > 0.0f ) &&
				 IntersectRayWithSphere( ray.m_Start, ray.m_Delta, BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ), m_flRadius, &d1, &d2 ))
			{
				if (d1 < tr.fraction)
				{
					// NDebugOverlay::Box(m_vecSurfacePos[i], Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), 255, 0, 0, 20, .1);
					nLastHit = i;
					tr.fraction = d1;
				}
			}
		}
		if (nLastHit != -1)
		{
			tr.m_pEnt = this;
			tr.startpos = ray.m_Start;
			tr.endpos = ray.m_Start + ray.m_Delta * tr.fraction; 
			tr.contents = CONTENTS_SOLID;
			tr.hitbox = nLastHit;
			tr.hitgroup = HITGROUP_GENERIC;
			tr.plane.dist = tr.endpos.Length();
			tr.plane.normal = (tr.endpos - BLOBPARTICLEPOSITION( m_iParticlePositionIndex[nLastHit] )) * (1 / m_flRadius);
			tr.plane.type = 0;
			tr.physicsbone = nLastHit;
		}
	}
	else
	{
		// FIXME: This isn't a valid test, Jay needs to make it real
		Vector vecMin = Vector( -m_flRadius, -m_flRadius, -m_flRadius) - ray.m_Extents;
		Vector vecMax = Vector( m_flRadius, m_flRadius, m_flRadius) + ray.m_Extents;

		trace_t boxtrace;

		tr.fraction = 1.0;

		for ( int i = 0; i < m_nActiveParticles; i++ )
		{
			if ( ( BLOBPARTICLERADIUS( m_iParticlePositionIndex[i] ) > 0.0f ) &&
				 IntersectRayWithBox( BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ) - ray.m_Start, -ray.m_Delta, vecMin, vecMax, 0.0, &boxtrace ))
			{
				if (boxtrace.fraction < tr.fraction)
				{
					tr = boxtrace;
					nLastHit = i;
				}
			}
		}

		/*
		if (tr.startsolid)
		{
			NDebugOverlay::Box( ray.m_Start, -ray.m_Extents, ray.m_Extents, 255, 0, 0, 0, 10.0);
		}
		*/
		//Assert( !tr.startsolid );
		//Assert( !tr.allsolid );

		//Msg("%5.2f (%6.1f %6.1f %6.1f ) : ", gpGlobals->curtime, ray.m_Start.x, ray.m_Start.y, ray.m_Start.z );
		//Msg("%6.1f %6.1f %6.1f : %4.2f : %6.1f %6.1f %6.1f\n", tr.startpos.x, tr.startpos.y, tr.startpos.z, tr.fraction, tr.endpos.x, tr.endpos.y, tr.endpos.z );

		if (tr.fraction < 1.0)
		{
			// TestCollision should just clip off the end of the ray:
			tr.startpos = ray.m_Start + ray.m_StartOffset;
			tr.endpos = tr.startpos + tr.fraction * ray.m_Delta;
			tr.fractionleftsolid = 0;

			tr.contents = CONTENTS_SOLID;
			tr.m_pEnt = this;
			tr.hitbox = nLastHit;
			tr.hitgroup = HITGROUP_GENERIC;
			tr.physicsbone = nLastHit;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

//-------------------------------------

float CNPC_Surface::MaxYawSpeed()
{
	return 180;
}

//-------------------------------------

void CNPC_Surface::HandleAnimEvent( animevent_t *pEvent )
{
	BaseClass::HandleAnimEvent( pEvent );
}

//-------------------------------------

void CNPC_Surface::PrescheduleThink()
{
	BaseClass::PrescheduleThink();
}	

//-------------------------------------

Vector CNPC_Surface::EyePosition( ) 
{
	return GetAbsOrigin(); 
}

const QAngle &CNPC_Surface::EyeAngles()
{
	return GetAbsAngles();
}


Vector CNPC_Surface::BodyTarget( const Vector &posSrc, bool bNoisy)
{
	int iShortest = 0;
	float flShortestDist = (posSrc - BLOBPARTICLEPOSITION( m_iParticlePositionIndex[iShortest] )).LengthSqr();
	for (int i = 1; i < m_nActiveParticles; i++)
	{
		if ( BLOBPARTICLERADIUS( m_iParticlePositionIndex[i] ) > 0.0f)
		{
			float flDist = (posSrc - BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] )).LengthSqr();
			if (flDist < flShortestDist)
			{
				iShortest = i;
				flShortestDist = flDist;
			}
		}
	}

	// NDebugOverlay::Box(m_body[iShortest].vecPos, Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), 0, 0, 255, 20, .1);

	return BLOBPARTICLEPOSITION( m_iParticlePositionIndex[iShortest] );
}


#undef USE_BLOBULATOR // TODO (Ilya): I need to get this working again

#if !defined(_X360) && defined( USE_BLOBULATOR )
//-------------------------------------

static ConVar	lj_InteractionRadius( "lj_InteractionRadius", "3", 0 );
static ConVar	lj_SurfaceTension( "lj_SurfaceTension", "1", 0 );
static ConVar	lj_Repulsion( "lj_Repulsion", "0.1", 0 );
static ConVar	lj_Attraction( "lj_Attraction", "0.1", 0 );
static ConVar	lj_MaxRepulsion( "lj_MaxRepulsion", "1", 0 );
static ConVar	lj_MaxAttraction( "lj_MaxAttraction", "1", 0 );


CLennardJonesForce::CLennardJonesForce( )
{
	m_fInteractionRadius = lj_InteractionRadius.GetFloat();
	m_fSurfaceTension = lj_SurfaceTension.GetFloat();
	m_fLennardJonesRepulsion = lj_Repulsion.GetFloat();
	m_fLennardJonesAttraction = lj_Attraction.GetFloat();
	m_fMaxRepulsion = lj_MaxRepulsion.GetFloat();
	m_fMaxAttraction = lj_MaxAttraction.GetFloat();

	m_pPhysTiler = new PhysTiler(m_fInteractionRadius);
}

CLennardJonesForce::~CLennardJonesForce( )
{
	delete m_pPhysTiler;
}


// TODO: I should make sure I don't have divide by zero errors.
// TODO: ts is not used
void CLennardJonesForce::addParticleForce(PhysParticle* a, PhysParticle* b, float distSq, float flStrength, float ts) const
{
	float d = sqrtf(distSq);

	//========================================================
	// based on equation of force between two molecules which is
	// factor * ((distance/bond_length)^-7 - (distance/bond_length)^-13)

	float f;
	if(a->group == b->group) // In the same group
	{
		float p = a->radius * 2.0f / (d+FLT_EPSILON);
		float p2 = p * p;
		float p4 = p2 * p2;


		// Surface tension:

		//Notes:
		// Can average the neighbor count between the two particles...
		// I tried this, and discovered that rather than averaging, I can take maybe take the
		// larger of the two neighbor counts, so the attraction between two particles on the surface will be strong, but
		// the attraction between a particle inside and a particle on the surface will be weak. I can also try
		// taking the min so that the attraction between a particle on the surface and a particle inside the fluid will
		// be strong, but the attraction between two particles completely on the inside will be weak.
		//
		// int symmetric_neighbor_count = min(a->neighbor_count, b->neighbor_count);
		//
		// Can try having neighbors only cause stronger attraction (no repulsion)
		// Can try lower exponents for the LennardJones forces.

		// This is a trick to prevent single particles from floating off... the less neighbors a particle has.. the more it sticks
		// This also tends to simulate surface tension
		float surface_tension_modifier = ((24.0f * m_fSurfaceTension) / (a->neighbor_count + b->neighbor_count + 0.1f)) + 1.0f;
		//float lennard_jones_force = fLennardJones * 2.0f * (p2 - (p4 * p4));
		float lennard_jones_force = m_fLennardJonesAttraction * p2 - m_fLennardJonesRepulsion*p4;
		f = surface_tension_modifier * lennard_jones_force;

		// This is some older code:
		//f = ((35.0f * LampScene::simulationSurfaceTension) / (a->neighbor_count + 0.1f)) * (p2 - (p4 * p4));
		// used to be 68'


		//float factor = (b->neighbor_count < 13 && neighbor_count < 13 ? 4.0f : 0.5f);
		//f = factor * (p2 - (p2 * p2 * p2 * p2));
	}
	else
	{
		// This was 3.5 ... made 3.0 so particles get closer when they collide
		if(d > a->radius * 3.0f) return;

		float p = a->radius * 4.0f / d;
		f = -1.0f * p * p;
	}

	// These checks are great to have, but are they really necessary?
	// It might also be good to have a limit on velocity

	// Attraction is a positive value.
	// Repulsion is negative.
	if(f < -m_fMaxRepulsion) f = -m_fMaxRepulsion;
	if(f > m_fMaxAttraction) f = m_fMaxAttraction;

	Point3D scaledr = (b->center - a->center) * (f/(d+FLT_EPSILON)) * flStrength; // Dividing by d scales distance down to a unit vector
	a->force.add(scaledr); 
	b->force.subtract(scaledr);
}

void CLennardJonesForce::AddForces( CUtlVector<IPhysicsObject *> &pObject, int nObjects, float flRadius, float flStrength, Vector *pForces )
{
	int nParticles = nObjects;

	// hack: copy cvars into settings so it can be edited live
	m_fInteractionRadius = lj_InteractionRadius.GetFloat();
	m_fSurfaceTension = lj_SurfaceTension.GetFloat();
	m_fLennardJonesRepulsion = lj_Repulsion.GetFloat();
	m_fLennardJonesAttraction = lj_Attraction.GetFloat();
	m_fMaxRepulsion = lj_MaxRepulsion.GetFloat();
	m_fMaxAttraction = lj_MaxAttraction.GetFloat();

	// FIXME: this isn't thread safe
	static SmartArray<PhysParticle> imp_particles_sa; // This doesn't specify alignment, might have problems with SSE
	while(imp_particles_sa.size < nObjects)
	{
		imp_particles_sa.pushAutoSize(PhysParticle());
	}
	
	// centered and scaled?
	m_pPhysTiler->beginFrame(Point3D(0.0f, 0.0f, 0.0f));

	// Move the spheres into particles
	for(int i=0;i<nObjects;i++)
	{
		PhysParticle* particle = &(imp_particles_sa[i]);
		particle->force.clear();

		Vector pos;
		QAngle ang;
		pObject[i]->GetPosition( &pos, &ang );

		particle->center = pos * (1.0 / flRadius);
		particle->group = i/20;
		particle->neighbor_count = 0;
		m_pPhysTiler->insertParticle(particle);
	}

	m_pPhysTiler->processTiles();


	float timeStep = 1.0f; // This should be customizable
	float nearNeighborInteractionRadius = 2.3f;
	float nearNeighborInteractionRadiusSq = nearNeighborInteractionRadius * nearNeighborInteractionRadius;
	
	PhysParticleCache* pCache = m_pPhysTiler->getParticleCache();

	// Calculate number of near neighbors for each particle
	for(int i = 0; i < nParticles; i++)
	{
		PhysParticle *b1 = &(imp_particles_sa[i]);

		PhysParticleAndDist* node = pCache->get(b1);

		while(node->particle != NULL)
		{
			PhysParticle* b2 = node->particle;

			 // Compare addresses of the two particles. This makes sure we apply a force only once between a pair of particles.
			if(b1 < b2 && node->distSq < nearNeighborInteractionRadiusSq)
			{
				b1->neighbor_count++;
				b2->neighbor_count++;
			}

			node++;
		}
	}

	// Calculate forces on particles due to other particles
	for(int i = 0; i < nParticles; i++)
	{
		PhysParticle *b1 = &(imp_particles_sa[i]);

		PhysParticleAndDist* node = pCache->get(b1);

		while(node->particle != NULL)
		{
			PhysParticle* b2 = node->particle;

			// Compare addresses of the two particles. This makes sure we apply a force only once between a pair of particles.
			if(b1 < b2)
			{
				addParticleForce(b1, b2, node->distSq, flStrength, timeStep);
			}

			node++;
		}
	}

	m_pPhysTiler->endFrame();

	// forces into output array
	for(int i=0;i<nObjects;i++)
	{
		pForces[i] = imp_particles_sa[i].force.AsVector() * flRadius;
	}
}


#endif



ExpandedParticleData_t CNPC_Surface::s_ExpandedParticleData[BLOB_MAX_LEVEL_PARTICLES];
static IPhysicsObject *s_pAllocatedPhysObjects[BLOB_MAX_LEVEL_PARTICLES];
static int s_iAllocatedParticleData = 0;

IPhysicsObject *CNPC_Surface::CreateParticlePhysics( void )
{
	objectparams_t params = g_PhysDefaultObjectParams;
	params.pGameData = static_cast<void *>(this);

	int nMaterialIndex = physprops->GetSurfaceIndex("water");

	Vector vPos = GetAbsOrigin();
	IPhysicsObject* p = physenv->CreateSphereObject( m_flRadius, nMaterialIndex, vPos, GetAbsAngles(), &params, false );
	Assert(p != NULL);

	p->SetVelocity( &vec3_origin, NULL );
	PhysSetGameFlags( p, FVPHYSICS_NO_NPC_IMPACT_DMG | FVPHYSICS_NO_SELF_COLLISIONS | FVPHYSICS_MULTIOBJECT_ENTITY ); // call collisionruleschanged if this changes dynamically
	
	p->SetMass( 10.0f );
	p->EnableGravity( true );
	p->EnableDrag( true );
	//p->SetContents( CONTENTS_GRATE );

	// p->EnableMotion( false );

	float flDamping = 0.5f;
	float flAngDamping = 0.5f;
	p->SetDamping( &flDamping, &flAngDamping );
	//p->SetInertia( Vector( 1e30, 1e30, 1e30 ) );


	ExpandedParticleData_t *pExtraData = &s_ExpandedParticleData[s_iAllocatedParticleData];
	s_pAllocatedPhysObjects[s_iAllocatedParticleData] = p;

	p->SetGameIndex( s_iAllocatedParticleData );
	++s_iAllocatedParticleData;

	pExtraData->iEntityIndex = m_vecPhysParticles.Count(); //best guess unless it gets overwritten by calling code
	pExtraData->iNetworkBypassIndex = AllocateBlobNetworkBypassIndex();
	BLOBPARTICLEPOSITION( pExtraData->iNetworkBypassIndex ) = vPos;
	BLOBPARTICLERADIUS( pExtraData->iNetworkBypassIndex ) = 1.0f;
	BLOBPARTICLECLOSESTSURFDIR( pExtraData->iNetworkBypassIndex ) = Vector(0,0,1);
	return p;
}

void CNPC_Surface::DestroyParticlePhysics( IPhysicsObject *pParticle )
{
	int iIndex = pParticle->GetGameIndex();
	ExpandedParticleData_t *pExtraData = &s_ExpandedParticleData[iIndex];
	ReleaseBlobNetworkBypassIndex( pExtraData->iNetworkBypassIndex );
	physenv->DestroyObject( pParticle );
	--s_iAllocatedParticleData;
	
	if( iIndex < s_iAllocatedParticleData )
	{
		*pExtraData = s_ExpandedParticleData[s_iAllocatedParticleData];
		s_pAllocatedPhysObjects[iIndex] = s_pAllocatedPhysObjects[s_iAllocatedParticleData];
		s_pAllocatedPhysObjects[iIndex]->SetGameIndex( iIndex );
	}
}

void CNPC_Surface::SetParticleEntityIndex( IPhysicsObject *pParticle, int iNewIndex )
{
	ExpandedParticleData_t *pExpandedData = &s_ExpandedParticleData[pParticle->GetGameIndex()];
	pExpandedData->iEntityIndex = iNewIndex;
	if( iNewIndex < MAX_SURFACE_ELEMENTS )
	{
		m_iParticlePositionIndex[iNewIndex] = pExpandedData->iNetworkBypassIndex;
	}
}

void CNPC_Surface::UpdateBypassParticleData( void )
{
	QAngle qDummyAngles;
	for( int i = s_BlobInstances.Count(); --i >= 0; )
	{
		CNPC_Surface *pEntity = s_BlobInstances[i];
		for( int j = pEntity->m_vecPhysParticles.Count(); --j >= 0; )
		{
			pEntity->m_vecPhysParticles[j]->GetPosition( &BLOBPARTICLEPOSITION( pEntity->m_iParticlePositionIndex[j] ), &qDummyAngles );
		}
	}

	g_pBlobNetworkBypass->bDataUpdated = true;
	g_pBlobNetworkBypass->fTimeDataUpdated = gpGlobals->curtime;
}
