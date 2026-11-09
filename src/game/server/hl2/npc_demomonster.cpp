//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//


// TODO: npc_demomonster is just here for backwards compatibility with levels that use it.
// It has been replaced with npc_blob, and should be deleted ASAP.


#include "cbase.h"
#include "saverestore_utlvector.h"
#include "dt_utlvector_send.h"
#include "npc_demomonster.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
//
// CNPC_BlobDemoMonster
//

//-----------------------------------------------------------------------------
//
// CNPC_BlobDemoMonster
//

//---------------------------------------------------------
// Custom Client entity
//---------------------------------------------------------
IMPLEMENT_SERVERCLASS_ST(CNPC_BlobDemoMonster, DT_NPC_BlobDemoMonster)

	// TODO: Shouldn't we only send as many elements as there are in the vector?

	SendPropUtlVector(
		SENDINFO_UTLVECTOR( m_flSurfaceV ),
		MAX_SURFACE_ELEMENTS, // max elements
		SendPropFloat( NULL, 0, sizeof( float ), 6, 0, 0.0, 1.0 )),
END_SEND_TABLE()

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------

BEGIN_DATADESC( CNPC_BlobDemoMonster )
	DEFINE_UTLVECTOR( m_flSurfaceV,	FIELD_FLOAT ),

	DEFINE_AUTO_ARRAY(  m_nOwnedSlot, FIELD_INTEGER ),
	DEFINE_AUTO_ARRAY(  m_nTargetSlot, FIELD_INTEGER ),
	DEFINE_AUTO_ARRAY(  m_bFloat, FIELD_BOOLEAN ),
	DEFINE_AUTO_ARRAY(  m_nArm, FIELD_INTEGER ),
	DEFINE_AUTO_ARRAY(  m_bContact, FIELD_BOOLEAN ),

	DEFINE_FIELD( m_vecGoal, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecPrevGoal, FIELD_POSITION_VECTOR ),

	DEFINE_FIELD( m_bDoArms, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDoContactZ, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hTarget, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flSimTime, FIELD_TIME ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_STRING, "MoveToPosition", InputMoveToPosition ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "DoArms", InputDoArms ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "DoContactZ", InputDoContactZ ),
END_DATADESC()


LINK_ENTITY_TO_CLASS( npc_blob_demomonster, CNPC_BlobDemoMonster );

void CNPC_BlobDemoMonster::Spawn( void )
{
	BaseClass::Spawn( );

	m_flSurfaceV.EnsureCount( MAX_SURFACE_ELEMENTS );
	for (int i = 0; i < MAX_SURFACE_ELEMENTS; i++)
	{
		m_flSurfaceV[i] = 0.0f;
	}


	m_flSimTime			= 0;
	m_bDoArms = false;

	m_vecPrevGoal = GetAbsOrigin();
}


#if(BLOB_PHYSICS_TEST == 2)

#define NUM_LEVELS 2
#define LEVEL1_BRANCHING 14
#define LEVEL1_PARTICLES LEVEL1_BRANCHING
#define LEVEL2_BRANCHING 4
#define LEVEL2_PARTICLES (LEVEL1_PARTICLES * LEVEL2_BRANCHING)

bool CNPC_BlobDemoMonster::CreateVPhysics( bool bFromRestore )
{
	// FIXME: don't hardcode the number of particles
	//m_nActiveParticles = 256;
	m_nActiveParticles = 1 + LEVEL1_PARTICLES + LEVEL2_PARTICLES;
	m_flRadius = 6.5;

	bool result = BaseClass::CreateVPhysics( bFromRestore );

	objectparams_t params = g_PhysDefaultObjectParams;
	params.pGameData = static_cast<void *>(this);

	springparams_t springParams;
	springParams.constant = 1000.0f;
	springParams.naturalLength = 2.0f * m_flRadius;
	springParams.damping = 0.5f;
	springParams.relativeDamping = 0.0f;
	Vector zero(0.0f, 0.0f, 0.0f);
	springParams.startPosition = zero;
	springParams.endPosition = zero;
	springParams.useLocalPositions = true;
	springParams.onlyStretch = false;

	constraint_lengthparams_t lengthParams;
	lengthParams.constraint.Defaults();
	lengthParams.minLength = 2.0f * m_flRadius;
	lengthParams.totalLength = 2.0f * m_flRadius;
	lengthParams.objectPosition[0].Init();
	lengthParams.objectPosition[1].Init();

	constraint_groupparams_t constraintParams;
	constraintParams.Defaults();

	IPhysicsConstraintGroup* constraint_group = physenv->CreateConstraintGroup(constraintParams);

	m_nParent[0] = -1;
	m_vecPhysParticles[0]->SetMass( 1.0f );
	//m_vecPhysParticles[0]->EnableMotion( false );
	m_vecPhysParticles[0]->EnableGravity( true );
	m_vecPhysParticles[0]->EnableDrag( true );
	//float flDamping = 0.5f;
	//float flAngDamping = 0.5f;
	//m_vecPhysParticles[i]->SetDamping( &flDamping, &flAngDamping );


	for(int i = 0; i < LEVEL1_BRANCHING; i++)
	{
		int parent = 0;
		int child = i + 1;
		//physenv->CreateSpring(m_vecPhysParticles[parent], m_vecPhysParticles[child], &springParams);

		//lengthParams.InitWorldspace(m_vecPhysParticles[parent], m_vecPhysParticles[child], m_vecSurfacePos[parent], m_vecSurfacePos[child], true);
		//lengthParams.minLength = 2.0f * m_flRadius;
		//lengthParams.totalLength = 2.0f * m_flRadius;
		//IPhysicsConstraint* physConstraint = physenv->CreateLengthConstraint(m_vecPhysParticles[parent], m_vecPhysParticles[child], constraint_group, lengthParams);
		//physConstraint->SetGameData(params.pGameData);
		//physConstraint->Activate();
		m_nParent[child] = parent;

		/* // Damping doesn't work for some reason!
		float flDamping = 200000.0f;
		float flAngDamping = 2000.0f;
		m_vecPhysParticles[child]->SetDamping( &flDamping, &flAngDamping );
		*/
		m_vecPhysParticles[child]->SetMass(1.0f);
		m_vecPhysParticles[child]->EnableDrag(true);
		m_vecPhysParticles[child]->EnableGravity(true);
	}

	for(int i = 0; i < LEVEL1_PARTICLES; i++)
	{
		for(int j = 0; j < LEVEL2_BRANCHING; j++)
		{
			int parent = i + 1;
			int child = 1 + LEVEL1_PARTICLES + i*LEVEL2_BRANCHING + j;
			//physenv->CreateSpring(m_vecPhysParticles[parent], m_vecPhysParticles[child], &springParams);
			//physenv->CreateLengthConstraint(m_vecPhysParticles[parent], m_vecPhysParticles[child], constraint_group, lengthParams);
			m_nParent[child] = parent;
			m_vecPhysParticles[child]->SetMass(1.0f);
			m_vecPhysParticles[child]->EnableDrag(true);
			m_vecPhysParticles[child]->EnableGravity(true);
		}
	}

	constraint_group->Activate();

	return result;
}

#else

bool CNPC_BlobDemoMonster::CreateVPhysics()
{
	// FIXME: don't hardcode the number of particles
	m_nActiveParticles = 256;
	m_flRadius = 6.5;

	bool result = BaseClass::CreateVPhysics();

	return result;
}

#endif

void CNPC_BlobDemoMonster::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	BaseClass::VPhysicsCollision( index, pEvent );

	// the sphere hit is:  i = pEvent->pObjects[index]->GetGameIndex();
	// CBaseEntity *pHit = pEvent->pEntities[!index];
	int nSphere = pEvent->pObjects[index]->GetGameIndex();

	m_bContact[nSphere] = true;
}


// Input handlers
void CNPC_BlobDemoMonster::InputMoveToPosition( inputdata_t &inputdata )
{
	if ( inputdata.value.Entity() != NULL )
	{
		SetTarget( inputdata.value.Entity() );
	}
	else if ( inputdata.value.String() != NULL )
	{
		CBaseEntity *pTargetEnt = gEntList.FindEntityByName( NULL, inputdata.value.String() );
		SetTarget( pTargetEnt );
	}
}

void CNPC_BlobDemoMonster::InputDoArms( inputdata_t &inputdata )
{
	m_bDoArms = inputdata.value.Bool();
}

void CNPC_BlobDemoMonster::InputDoContactZ( inputdata_t &inputdata )
{
	m_bDoContactZ = inputdata.value.Bool();
}



int CNPC_BlobDemoMonster::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	if (info.GetDamageType() & (DMG_BULLET | DMG_CLUB | DMG_BLAST))
	{
		Vector dir = m_vecGoal - info.GetInflictor()->GetAbsOrigin();
		VectorNormalize( dir );
		m_vecPrevGoal = m_vecPrevGoal + dir * 8;
	}

	return BaseClass::OnTakeDamage_Alive( info );
}

void CNPC_BlobDemoMonster::PostPhysFrame()
{
#if(BLOB_PHYSICS_TEST == 1 || BLOB_PHYSICS_TEST == 2)
	DoPhysics();
#endif
}

void CNPC_BlobDemoMonster::DoPhysics()
{
	int i;
	// copy physics positions into networked array
	for (i = 0; i < m_nActiveParticles; i++)
	{
		Vector pos;
		QAngle ang;
		m_vecPhysParticles[i]->GetPosition( &pos, &ang );
		BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ) = pos;
	}

#if 0
	// alternate between arm mode and walk mode
	bool bDoArms = ((int)(gpGlobals->curtime / 17.0) % 2) == 1;

	if (bDoArms != m_bDoArms)
	{
		for (i = 0; i < m_nActiveParticles; i++)
		{
			m_nOwnedSlot[ i ] = 0;
			m_nTargetSlot[ i ] = 0;
		}
	}
	m_bDoArms = bDoArms;
#endif

	int nArmLength = 1;
	if (m_bDoArms)
	{
		nArmLength = 16;
		m_flSimTime += 0.025;
	}
	else
	{
		m_flSimTime += 0.1;
	}
	Vector vecGoal = m_vecPrevGoal; // ( cos( m_flSimTime * 0.3 ) * 250, sin( m_flSimTime * 0.2 ) * 250, 0 );

	if (GetTarget() != NULL)
	{
		vecGoal = GetTarget()->GetAbsOrigin();
	}

	// keep track of direction of movement
	Vector forward = vecGoal - m_vecPrevGoal;
	float dist = VectorNormalize( forward );
	if (dist < 1.0)
	{
		forward *= dist;
	}
	else
	{
		vecGoal = m_vecPrevGoal + forward * min( dist, m_bDoArms ? 16 : 48 );
	}


	m_vecPrevGoal = m_vecPrevGoal * 0.8 + vecGoal * 0.2;


	//vecGoal.z = m_flRadius; // Ilya: I don't know why this is here, but it breaks the blob going up and down stuff
	m_vecGoal = vecGoal;


#if(BLOB_PHYSICS_TEST != 2)

	if (!m_bDoArms)
	{
		MoveTowardsGoal( );
	}
	else
	{
		CreateArms( forward );
	}

	RepulseNeighbors( );
#else

	for(int i = 0; i < m_nActiveParticles; i++)
	{
		Vector vecVel;
		Vector parentVel;
		m_vecPhysParticles[i]->GetVelocity( &vecVel, NULL );
		// If the particle has a parent, measure it's velocity relative to it's parent's velocity.
		if(m_nParent[i] == -1)
		{
			continue; // The root node has no viscosity?
		}
		else
		{
			m_vecPhysParticles[m_nParent[i]]->GetVelocity( &parentVel, NULL );
			vecVel -= parentVel;
		}

		float vel = vecVel.Length();
		float maxForce = m_vecPhysParticles[i]->GetMass() * vel; // We don't want a force greater than the velocity * mass... because it will make the particle go backwards.

		float dampForce = 1.0f * vel*vel;
		//float dampForce = 0.1f * vel;
		if(dampForce > maxForce) dampForce = maxForce;

		vecVel *= -dampForce * (1.0f /(vel+FLT_EPSILON));
		m_vecPhysParticles[i]->ApplyForceCenter( vecVel );
	}

	RepulseNeighbors( );

#endif

	// This updates the blob's origin and collision bounds.
	Vector blobCenter;
	blobCenter.Init();
	for (int i = 0; i < m_nActiveParticles; i++)
	{
		blobCenter += BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] );
	}
	blobCenter /= m_nActiveParticles;

	Vector theMins = BLOBPARTICLEPOSITION( m_iParticlePositionIndex[0] );
	Vector theMaxs = BLOBPARTICLEPOSITION( m_iParticlePositionIndex[0] );
	Vector surfaceRadius( m_flRadius, m_flRadius, m_flRadius );
	surfaceRadius *= 2.0f;
	for (int i = 0; i < m_nActiveParticles; i++)
	{
		VectorMin( BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ), theMins, theMins );
		VectorMax( BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ), theMaxs, theMaxs );
	}
	theMins -= (blobCenter + surfaceRadius);
	theMaxs += (surfaceRadius - blobCenter);

	Vector prevOrigin = GetAbsOrigin();
	SetAbsOrigin( blobCenter );
	SetCollisionBounds(theMins, theMaxs);
	// This sets off triggers:
	PhysicsTouchTriggers( &prevOrigin );
}

void CNPC_BlobDemoMonster::RunAI( void )
{
	// Note: I don't copy positions from the physics array to the networked arrays
	// here, but I think that should be ok because it happens on the Phys Frame
#if(BLOB_PHYSICS_TEST == 0)
	DoPhysics();
#endif

	NetworkProp()->NetworkStateForceUpdate();
}


#if (BLOB_PHYSICS_TEST == 2)
// NOTE: a floating particle (with bFloat) is one that is attached to an arm.

void CNPC_BlobDemoMonster::RepulseNeighbors()
{
	float projection = 0.0f;
	for(int i = 0; i < m_nActiveParticles; i++)
	{
		//Vector vecVel;
		//m_vecPhysParticles[i]->GetVelocity( &vecVel, NULL );
		Vector estPos = m_vecSurfacePos[i]; // + vecVel * projection;

		//Vector delta(0.0f, 0.0f, 0.0f);
		Vector delta(RandomFloat( -1, 1 ), RandomFloat( -1, 1 ), RandomFloat( -1, 1 ));

		for(int j = 0; j < m_nActiveParticles; j++)
		{
			if(i != j)
			{
				//m_vecPhysParticles[j]->GetVelocity( &vecVel, NULL );
				Vector estEffectorPos =   m_vecSurfacePos[j]; // + vecVel * projection;
				Vector dir = estPos - estEffectorPos;
				float flDist = dir.Length();
				dir *= (1.0f/(flDist+0.0001f));

				if(m_nParent[i] != j && m_nParent[j] != i)
				{
					//float flDist2 = flDist * flDist;
					//float force = 50.0f * ((2.0f*m_flRadius)/(flDist+0.1f));
					float pos = max(((2.0f*m_flRadius - flDist) / (2.0f*m_flRadius)), 0.0f);
					float force = 100.0f * sqrtf(pos);

					delta += force * dir;
				}
				else
				{
					float pos = (2.0f*m_flRadius - flDist);
					float force = 10.0f * pos;
					delta += (dir * force);
				}
			}
		}

		// TODO: Right now, we calculate forces in both directions.
		//       Could cut the work in half if we calc force in one
		//       direction and apply it in the other direction for particle j
		//       Don't know how efficient this would be.
		//m_vecPhysParticles[i]->EnableGravity(true);
		m_vecPhysParticles[i]->ApplyForceCenter( delta );
	}
}

#else

void CNPC_BlobDemoMonster::RepulseNeighbors()
{
	//float flIdealDistance = m_flRadius * sv_surface_ideal.GetFloat();
	float flNearbyDistance = m_flRadius * sv_surface_nearby.GetFloat();

	m_physParticles.size = m_nActiveParticles;

	PhysTiler *pPhysTiler = PhysTilerFactory::factory->getTiler();

	// Center and scale the lookup cache params.
	pPhysTiler->setInteractionRadius(flNearbyDistance);
	pPhysTiler->beginFrame( Point3D(GetAbsOrigin()) );
	//pPhysTiler->beginFrame( Point3D(0, 0, 0) );

	int nParticles = 0;

	// Move the spheres into particles
	float projection = 0.5;
	for(int i = 0; i < m_nActiveParticles; i++)
	{
		if (m_flSurfaceR[i] > 0.0f)
		{
			CDemoMonsterParticle* particle = &(m_physParticles[nParticles++]);

			Vector vecVel;
			m_vecPhysParticles[i]->GetVelocity( &vecVel, NULL );
			Vector estPos = BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ) + vecVel * projection;

			particle->center = estPos; // * (1.0 / m_flRadius);
			particle->group = 0;
			particle->neighbor_count = 0;
			particle->temp1 = i;
			pPhysTiler->insertParticle(particle);
		}
	}

#if 0
	pPhysTiler->processTiles();

	PhysParticleCache* pCache = pPhysTiler->getParticleCache();

	for (int k = 0; k < nParticles; k++)
	{
		CDemoMonsterParticle *b1 = &(m_physParticles[k]);
		int i = b1->temp1;

		Vector estPos = b1->center.AsVector(); // * m_flRadius;
		Vector delta( 0, 0, 0 );

		// push against nearby spheres
		float flIdealDist2 = (flIdealDistance * flIdealDistance);
		float flNearbyDist2 = (flNearbyDistance * flNearbyDistance);
		bool bFloat = false;
		float flDist2;
		Vector dir;

		PhysParticleAndDist* node = pCache->get(b1);

		while(node->particle != NULL)
		{
			PhysParticle* b2 = node->particle;
			if (b2 == b1)
			{
				node++;
				continue;
			}
			int j = b2->temp1;

			int bSameArm = (m_nArm[i] == m_nArm[j]) && (m_nArm[i] > -1);

			Vector estEffectorPos =  b2->center.AsVector(); // * m_flRadius;
			flDist2 = (estPos - estEffectorPos).LengthSqr();

			flNearbyDist2 = (m_flSurfaceR[i] + m_flSurfaceR[j]) * 0.5 * flNearbyDistance;
			flNearbyDist2 = flNearbyDist2 * flNearbyDist2;

			// NDebugOverlay::Line(m_vecSurfacePos[i], m_vecSurfacePos[j], 0, 255, 0, true, .1);

			if (!bSameArm && m_nTargetSlot[j] != 0)
			{
				if (flDist2 < flIdealDist2)
				{
					// repluse if they're too close, and they're not in the same group, and they're on an arm
					dir = (estPos - estEffectorPos);
					VectorNormalize( dir );
					delta += dir * min( (flIdealDist2 - flDist2), 100 );
					//NDebugOverlay::Line(m_vecSurfacePos[i], m_vecSurfacePos[j], 255, 0, 0, true, .1);
				}
				/*
				else if (flDist2 < flNearbyDist2)
				{
				dir = (estPos - m_vecSurfacePos[j] - vecVel2);
				VectorNormalize( dir );
				delta -= dir * 30;
				NDebugOverlay::Line(m_vecSurfacePos[i], m_vecSurfacePos[j], 255, 0, 0, true, .1);
				}
				*/
				// NDebugOverlay::Line(m_vecSurfacePos[i], m_vecSurfacePos[j], 255, 0, 0, true, .1);
			}

			// A particle will float if it is close to particle 2, and is either above another particle,
			// or is touching an arm particle that is closer to the base than it was floating in a previous frame.
			// So, as soon as a root particle touches the ground, it will stop floating.
			if (!bFloat && flDist2 < flNearbyDist2 && ((BLOBPARTICLEPOSITION( m_iParticlePositionIndex[j] ).z <= BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ).z) || (bSameArm && j < i) && m_bFloat[j] ) )
			{
				//NDebugOverlay::Line(m_vecSurfacePos[j], m_vecSurfacePos[i], 0, 255, 0, true, .1);
				bFloat = true;
			}

			node++;
		}

		// figure out what to do with gravity
		if (!bFloat /* && (i % nArmLength == 0) */)
		{
			/*
			if (delta.z > 0)
			delta.z = 0;
			delta.z -= 300 * 0.1 * m_vecPhysParticles[i]->GetMass();
			*/
			//NDebugOverlay::Box(m_vecSurfacePos[i], Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), 255, 0, 0, 20, .1);
		}

		// To have gravity enabled, a particle should not be floating and should not be in contact.
		// Ilya: I turned off the !m_bContact part, and it didn't seem to make any difference, which
		// is what I would have expected.
		m_vecPhysParticles[i]->EnableGravity( !bFloat /*&& !m_bContact[i]*/ );
		m_bFloat[i] = bFloat;
		m_bContact[i] = false;

#if 0
		// pull toward next in chain
		if (i != 0 /* && i == ((int)gpGlobals->curtime) % m_vecSurfacePos.Count()*/)
		{
			dir = (m_vecSurfacePos[i-1] - estPos);
			if (DotProduct( dir, vecVel ) < 0.0)
			{
				float dist = VectorNormalize( dir );
				delta += dir * min( dist, 100 );
			}
			//NDebugOverlay::Line(m_vecSurfacePos[i-1], estPos, 0, 255, 0, true, .1);
			//Msg("%d : %.1f %.1f %.1f\n", i, delta.x, delta.y, delta.z );
		}
#endif

#if 0
		float dot = DotProduct( vVel, delta );
		if (dot > 0.0)
			delta.Init();
#endif

		// apply the force
		m_vecPhysParticles[i]->ApplyForceCenter( delta );

		// NDebugOverlay::Box(m_vecSurfacePos[i], Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), 0, 255, 0, 20, .1);
	}
#endif

	pPhysTiler->endFrame();

	PhysTilerFactory::factory->returnTiler(pPhysTiler);
}

#endif




void CNPC_BlobDemoMonster::MoveTowardsGoal( void )
{
	int i;

	float tension = sv_surface_tension.GetFloat(); // * (1 - sqrt( fabs( sin( gpGlobals->curtime * 0.3 ) ) ) );

	float projection = 0.5;
	// push spheres around to meet position targets
	for (i = 0; i < m_nActiveParticles; i++)
	{
		Vector vecVel;
		m_vecPhysParticles[i]->GetVelocity( &vecVel, NULL );
		Vector estPos = BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ) + vecVel * projection;
		Vector delta( 0, 0, 0 );
		float dist(0.0f);

		// NDebugOverlay::Box(m_vecSurfacePos[i], Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), m_bFloat[i] * 255, 255, m_bContact[i] * 255, 20, .1);

		// If a particle is floating, touching something, or moving slowly
		// it will be pulled towards the center. (Essentially, if it has traction of any sort)
		if (m_bFloat[i] || m_bContact[i] || fabs( vecVel.z ) < 1.0 )
		{
			m_nArm[i] = -1;
			delta = m_vecGoal - estPos;
			if (m_bDoContactZ)
				delta.z = 0;
			//delta.z *= fabs( cos( gpGlobals->curtime * 0.5 ) );
			dist = VectorNormalize( delta );
			delta = delta * min( max( dist - m_flRadius * 8, 0 ), 500 ) * tension;
			m_nOwnedSlot[i] = 0;
			m_nTargetSlot[i] = 1;
			m_flSurfaceV[i] = Approach( 0.0f, m_flSurfaceV[i], 0.2f );
		}

		// tweak radius
		//float a = (m_bContact[i] || bFloat) ? 1.0 : sv_surface_scale.GetFloat();
		float a = 1.0f; // sv_surface_scale.GetFloat();
		float b = m_flSurfaceR[i];
		float c = Approach( a, b, 0.2f );
		m_flSurfaceR[i] = clamp( c, 0.0f, 1.0f );

		// This basically states that if a particle is floating or if
		// contactZ is on and it is making contact, it should be able to
		// have a vertical force and move upwards. However, this doesn't
		// make much sense because above, if contactZ is on and we are
		// making contact, then the code above will set delta.z to 0.
		// 
		// if m_bDoContactZ is false, when float is false, delta.z gets set to zero.
		// if m_bDoContactZ is true, it has to not be floating and not contacting for delta.z to get set to zero.
		// But, in that case, delta.z will always be set to zero by the code above.

		//if (!(m_bFloat[i] || (m_bDoContactZ && m_bContact[i])))
		//if (m_bFloat[i] == false && (m_bDoContactZ == false || m_bContact[i] == false))) // This is a translation of Ken's code.

		// But really, this code should say that a particle can only move on the surface it is touching.
		/*
		if (m_bFloat[i] == false && m_bContact[i] == false) // This is what I think makes more sense.
		{
		delta.z = 0;
		}
		*/

		// apply the force
		m_vecPhysParticles[i]->ApplyForceCenter( delta );
	}
}



void CNPC_BlobDemoMonster::CreateArms( const Vector &vecForward  )
{
	int i;

	if (!m_bDoArms)
	{
		for (i = 0; i < m_nActiveParticles; i++)
		{
			m_nOwnedSlot[ i ] = 0;
			m_nTargetSlot[ i ] = 0;
		}
	}
	m_bDoArms = true;

	// NDebugOverlay::HorzArrow( GetAbsOrigin(), GetAbsOrigin() + vecForward * 32, 8, 255, 255, 255, 255, true, 0.1 );
	int nArmLength = 8;

	float tension = sv_surface_tension.GetFloat(); // * (1 - sqrt( fabs( sin( gpGlobals->curtime * 0.3 ) ) ) );

	float projection = 0.5;
	// push spheres around to meet position targets
	for (i = 0; i < m_nActiveParticles; i++)
	{
		Vector vecVel;
		m_vecPhysParticles[i]->GetVelocity( &vecVel, NULL );
		Vector estPos = BLOBPARTICLEPOSITION( m_iParticlePositionIndex[i] ) + vecVel * projection;
		Vector delta( 0, 0, 0 );
		float dist(0.0f);

		m_nArm[i] = (i / nArmLength);

		if (!m_bFloat[i])
		{
			m_nTargetSlot[ i ] = 0;
		}

		// NDebugOverlay::Box(m_vecSurfacePos[i], Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), m_bFloat[i] * 255, 255, m_bContact[i] * 255, 20, .1);
		if (m_bFloat[i] || m_bContact[i] || fabs( vecVel.z ) < 1.0 )
		{
			int k = (i % nArmLength);
			int j = i - k;

			if (k == 0)
			{
				// move sphere towards center target
				delta = m_vecGoal - estPos;
				//delta.z *= fabs( cos( gpGlobals->curtime * 0.5 ) );
				dist = VectorNormalize( delta );
				delta = delta * min( max( dist - m_flRadius * 5, 0 ), 500 ) * tension;
				if (m_bDoContactZ)
					delta.z = 0;
				m_nOwnedSlot[i] = 0;
				m_nTargetSlot[i] = 1;
				m_flSurfaceV[i] = Approach( 0.0f, m_flSurfaceV[i], 0.2f );
			}
			else
			{
				// move sphere outward towards end of arm
				Vector out = BLOBPARTICLEPOSITION( m_iParticlePositionIndex[j] ) - m_vecGoal;
				VectorNormalize( out );

				if (m_nTargetSlot[i] > m_nOwnedSlot[j] + 1)
				{
					m_nTargetSlot[i] = m_nOwnedSlot[j] + 1;
				}

				// Vector target =  m_vecSurfacePos[j] + out * flIdealDistance * (m_nTargetSlot[i]);
				int n = min( j + m_nTargetSlot[i], i - 1);
				Vector target =  BLOBPARTICLEPOSITION( m_iParticlePositionIndex[j] );
				if (i != j)
				{
					Vector vecTargetVel;
					m_vecPhysParticles[i-1]->GetVelocity( &vecTargetVel, NULL );
					Vector dir = out + 2 * vecForward * (n - j) / nArmLength;
					VectorNormalize( dir );
					target = BLOBPARTICLEPOSITION( m_iParticlePositionIndex[n] ) + vecTargetVel * 0.1f + dir * m_flRadius * (m_flSurfaceR[n] + m_flSurfaceR[i]);
					// Msg("%d : %d : %.1f : %.1f %.1f\n", i, n, (m_flSurfaceR[j+n] + m_flSurfaceR[i]), m_flSurfaceR[j+n], m_flSurfaceR[i] ) ;
				}

				//target += Vector( 0, 0, m_flRadius ) * sin( j + gpGlobals->curtime * 3.0 + M_PI * (m_nTargetSlot[i] / (float)(nArmLength-1)) );
				delta = target - estPos;
				//NDebugOverlay::Line(m_vecSurfacePos[i], estPos + delta, 0, 255, 0, true, .1);
				//NDebugOverlay::Line(m_vecSurfacePos[i], m_vecSurfacePos[j] + out * flIdealDistance * (m_nTargetSlot[i]), 0, 255, 0, true, .1);
				dist = VectorNormalize( delta );
				if (dist < m_flRadius * m_flSurfaceR[i])
				{
					if (m_nTargetSlot[i] == k)
					{
						m_nOwnedSlot[j] = max( m_nOwnedSlot[j], k );
					}
					else
					{
						m_nTargetSlot[i]++;
					}
				}
				else if (dist >= m_flRadius * m_flSurfaceR[i] * 2 && m_nTargetSlot[i] > 0)
				{
					m_nTargetSlot[i]--;
					if (m_nOwnedSlot[j] > m_nTargetSlot[i])
					{
						m_nOwnedSlot[j] = m_nTargetSlot[i];
					}
					//NDebugOverlay::Box(m_vecSurfacePos[i], Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), 255, 0, 0, 20, .1);
				}

				//NDebugOverlay::Line( m_vecSurfacePos[j], m_vecSurfacePos[j] + out * flIdealDistance * m_nOwnedSlot[j], 255, 0, 0, true, .1);

				delta = delta * min( max( dist - 0, 0), 250 ) * m_vecPhysParticles[i]->GetMass();
				//NDebugOverlay::Line(m_vecSurfacePos[j], m_vecSurfacePos[j] + out * flIdealDistance, 255, 0, 0, true, .1);
				// m_flSurfaceV[i] = clamp( Approach( (m_nTargetSlot[i]) / (float)max( m_nOwnedSlot[j] + 1, m_nTargetSlot[i]), m_flSurfaceV[i], 0.2f ), 0.0f, 1.0f );
				m_flSurfaceV[i] = Approach( (m_nTargetSlot[i]) / (float)nArmLength, m_flSurfaceV[i], 0.2f );
				m_flSurfaceV[i] = clamp( m_flSurfaceV[i], 0.0f, 1.0f );
			}
		}

		// tweak radius
		//float a = (m_bContact[i] || bFloat) ? 1.0 : sv_surface_scale.GetFloat();
		float a = sv_surface_scale.GetFloat();
		if (m_nTargetSlot[i] <= 1 ) a = 1.0;
		if (nArmLength > 1 && m_nTargetSlot[i] >= nArmLength - 1) a = (1.0 + sv_surface_scale.GetFloat()) * 0.5;
		float b = m_flSurfaceR[i];

		// a = 1.0f;
		float c = Approach( a, b, 0.2f );
		m_flSurfaceR[i] = clamp( c, 0.0f, 1.0f );

		if (!m_bFloat[i])
		{
			delta.z = 0;
		}

		// apply the force
		m_vecPhysParticles[i]->ApplyForceCenter( delta );
	}
}
