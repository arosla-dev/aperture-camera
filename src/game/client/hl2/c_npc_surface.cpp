//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "dt_utlvector_recv.h"
#include "bone_setup.h"
#include "c_ai_basenpc.h"
#include "engine/IVDebugOverlay.h"
#include "c_surfacerender.h"

#include "IVRenderView.h"
#include "view_shared.h"
#include "iviewrender.h"

#include "tier0/vprof.h"
#include "soundinfo.h"

// TODO: These should be in public by the time the SDK ships
#if 1
	#include "../../common/blobulator/Implicit/ImpDefines.h"
	#include "../../common/blobulator/Implicit/ImpRenderer.h"
	#include "../../common/blobulator/Implicit/ImpTiler.h"
	#include "../../common/blobulator/Implicit/UserFunctions.h"
#else
	#include "../common/blobulator/Implicit/ImpDefines.h"
	#include "../../common/blobulator/Implicit/SweepRenderer2.h"
	#include "../../common/blobulator/Implicit/ImpTiler2.h"
	#include "../../common/blobulator/Implicit/UserFunctions2.h"
#endif

#include <blob_networkbypass.h>
#include "c_npc_surface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//-----------------------------------------------------------------------------
// Purpose: setup network receive table
//-----------------------------------------------------------------------------

IMPLEMENT_CLIENTCLASS_DT(C_NPC_Surface, DT_NPC_Surface, CNPC_Surface)
	RecvPropFloat	( RECVINFO( m_flRadius ) ),
	RecvPropInt		( RECVINFO( m_nActiveParticles ) ),

	RecvPropUtlVector(
		RECVINFO_UTLVECTOR(m_iParticlePositionIndex),
		MAX_SURFACE_ELEMENTS,
		RecvPropInt(NULL, 0, sizeof(uint16))),

	/*RecvPropUtlVector(
		RECVINFO_UTLVECTOR( m_vecSurfacePos ), 
		MAX_SURFACE_ELEMENTS,
		RecvPropVector(NULL, 0, sizeof( Vector ))),
	RecvPropUtlVector( 
		RECVINFO_UTLVECTOR( m_flSurfaceV ), 
		MAX_SURFACE_ELEMENTS,
		RecvPropFloat(NULL, 0, sizeof( float ))),
	RecvPropUtlVector( 
		RECVINFO_UTLVECTOR( m_flSurfaceR ), 
		MAX_SURFACE_ELEMENTS,
		RecvPropFloat(NULL, 0, sizeof( float ))),*/
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: link networked elements to local data
//-----------------------------------------------------------------------------

C_NPC_Surface::C_NPC_Surface()
{
	m_pMaterial = NULL;

	//theaperturecat - is this needed?
	m_iParticlePositionIndex.EnsureCount(MAX_SURFACE_ELEMENTS);
	
	/*m_vecSurfacePos.EnsureCount(MAX_SURFACE_ELEMENTS);
	m_iv_vecSurfacePos.EnsureCount( MAX_SURFACE_ELEMENTS );

	m_flSurfaceV.EnsureCount( MAX_SURFACE_ELEMENTS );
	m_iv_flSurfaceV.EnsureCount( MAX_SURFACE_ELEMENTS );

	m_flSurfaceR.EnsureCount( MAX_SURFACE_ELEMENTS );
	m_iv_flSurfaceR.EnsureCount( MAX_SURFACE_ELEMENTS );

	for (int i = 0; i < MAX_SURFACE_ELEMENTS; i++)
	{
		IInterpolatedVar *pWatcher = &m_iv_vecSurfacePos.Element( i );
		pWatcher->SetDebugName( "m_iv_vecSurfacePos" );
		AddVar( &m_vecSurfacePos.Element( i ), pWatcher, LATCH_ANIMATION_VAR );

		pWatcher = &m_iv_flSurfaceV.Element( i );
		pWatcher->SetDebugName( "m_iv_flSurfaceV" );
		AddVar( &m_flSurfaceV.Element( i ), pWatcher, LATCH_ANIMATION_VAR );

		pWatcher = &m_iv_flSurfaceR.Element( i );
		pWatcher->SetDebugName( "m_iv_flSurfaceR" );
		AddVar( &m_flSurfaceR.Element( i ), pWatcher, LATCH_ANIMATION_VAR );
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

C_NPC_Surface::~C_NPC_Surface()
{
}





//-----------------------------------------------------------------------------
// Purpose: Custom model rendering
//-----------------------------------------------------------------------------

static ConVar	sv_surface_testshape("surface_testshape", "0", 0, "Use a test shape instead of the hydra");
static ConVar	sv_surface_center("surface_center", "0", FCVAR_ARCHIVE, "Adjust render center");
//static ConVar	sv_surface_fountain("surface_fountain", "0", FCVAR_ARCHIVE, "Turns on settings for rendering the fountain"); no archiving this is really annoying
static ConVar	sv_surface_fountain("surface_fountain", "0", 0, "Turns on settings for rendering the fountain");

static ConVar blob_material("cl_blob_material","models/weapons/w_icegun/ice_surface");

extern ConVar	mat_wireframe;

void C_NPC_Surface::GetRenderBounds( Vector& theMins, Vector& theMaxs )
{
	// BaseClass::GetRenderBounds( theMins, theMaxs );

		theMins = BLOBPARTICLEPOS_INTERP(m_iParticlePositionIndex[0]);
		theMaxs = BLOBPARTICLEPOS_INTERP(m_iParticlePositionIndex[0]);
		float surfaceRadius = m_flRadius * 3.0f;
		for (int i = 0; i < m_nActiveParticles; i++)
		{
			VectorMin(BLOBPARTICLEPOS_INTERP(m_iParticlePositionIndex[i]) - Vector( surfaceRadius, surfaceRadius, surfaceRadius ), theMins, theMins );
			VectorMax(BLOBPARTICLEPOS_INTERP(m_iParticlePositionIndex[i]) + Vector( surfaceRadius, surfaceRadius, surfaceRadius ), theMaxs, theMaxs );
		}
	theMins -= GetRenderOrigin();
	theMaxs -= GetRenderOrigin();

	#if 0
	Vector avg = (theMins + theMaxs) * 0.5f;
	theMins = theMins - ((theMins - avg) * 0.75f);
	theMaxs = theMaxs - ((theMaxs - avg) * 0.75f);
	#endif

	#if 0
	Vector fountainOrigin(-1980, -1792, 1);
	theMins = fountainOrigin + Vector(-10, -10, -20);
	theMaxs = fountainOrigin + Vector(10, 10, 50);
	#endif

	// Msg( "origin  %.2f %.2f %.2f : mins %.2f %.2f %.2f  : maxs %.2f %.2f %.2f\n", GetRenderOrigin().x, GetRenderOrigin().y, GetRenderOrigin().z, theMins.x, theMins.y, theMins.z, theMaxs.x, theMaxs.y, theMaxs.z );

	//debugoverlay->AddBoxOverlay( GetRenderOrigin(), GetRenderOrigin() + Vector(-10,-10,-10), GetRenderOrigin()+Vector(10, 10, 10), QAngle(0, 0, 0), 0, 255, 0, 0, 0);
}

bool C_NPC_Surface::IsTransparent()
{
	// TODO: Fix this
	return true;
}

//-----------------------------------------------------------------------------
// Yes we bloody are
//-----------------------------------------------------------------------------
RenderableTranslucencyType_t C_NPC_Surface::ComputeTranslucencyType()
{
	return RENDERABLE_IS_TRANSLUCENT;
}

bool C_NPC_Surface::UsesPowerOfTwoFrameBufferTexture()
{
	if(!m_pMaterial) return false;
	return m_pMaterial->NeedsPowerOfTwoFrameBufferTexture();
}

bool C_NPC_Surface::UsesFullFrameBufferTexture()
{
	if(!m_pMaterial) return false;
	return m_pMaterial->NeedsFullFrameBufferTexture();
}





void C_NPC_Surface::SetupParticles()
{
	int n_particles = 0;

	for (int i = 0; i < m_nActiveParticles; i++)
	{
		ImpParticleWithOneInterpolant* imp_particle = &g_SurfaceRenderParticles[i];
		//imp_particle->center = m_vecSurfacePos[i];
		//imp_particle->setFieldScale(m_flSurfaceR[i]);
		//imp_particle->interpolants1[3] = m_flSurfaceV[i];
		imp_particle->center = BLOBPARTICLEPOS_INTERP(m_iParticlePositionIndex[i]);
		imp_particle->setFieldScale(BLOBPARTICLERADIUS_INTERP(m_iParticlePositionIndex[i]));
		imp_particle->interpolants1.set(1.0f, 1.0f, 1.0f);
		imp_particle->interpolants1[3] = 0.0f; //is this right?
		n_particles++;
	}

	g_SurfaceRenderParticles.SetCountNonDestructively(n_particles);
}

int C_NPC_Surface::DrawModel(int flags, const RenderableInstance_t& instance)
{
	Vector fountainOrigin(-1980, -1792, 1);
	if (sv_surface_fountain.GetBool())
	{
		modelrender->SetupLighting(fountainOrigin);
	}
	else
	{
		modelrender->SetupLighting(GetRenderOrigin());
		//modelrender->SetupLighting( Vector(0,0,300) );
	}


#define MAX_EXTRA_ELEMENTS 400
	g_SurfaceRenderParticles.SetCount(MAX_SURFACE_ELEMENTS + MAX_EXTRA_ELEMENTS);
	//static ImpParticleWithOneInterpolant imp_particles[MAX_SURFACE_ELEMENTS + MAX_EXTRA_ELEMENTS]; // This doesn't specify alignment, might have problems with SSE
	
	SetupParticles();

	Vector center;
		center.Init();

		/*if (sv_surface_center.GetBool())
		{
			for (int i = 0; i < m_nActiveParticles; i++)
			{
				center += m_vecSurfacePos[i];
			}
			center /= m_nActiveParticles;
		}*/

	m_pMaterial = materials->FindMaterial("models/weapons/w_icegun/ice_surface", TEXTURE_GROUP_OTHER, true);

	//if (sv_surface_center_on_blob.GetBool())
	//{
	//	Surface_Draw(GetClientRenderable(), GetAbsOrigin(), m_pMaterial, 6.5f, true);
	//}
	//else
	//{

	Surface_Draw(GetClientRenderable(), center, m_pMaterial, m_flRadius);// 4.0f);
	//}

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: move the sound source to a point near the player
//-----------------------------------------------------------------------------

bool C_NPC_Surface::GetSoundSpatialization( SpatializationInfo_t& info )
{
	bool bret = BaseClass::GetSoundSpatialization( info );
	// Default things it's audible, put it at a better spot?
	if ( bret )
	{
		// TODO:  Note, this is where you could override the sound position and orientation and use
		//  an attachment points position as the sound source
		// You might have to issue C_BaseAnimating::AllowBoneAccess( true, false ); to allow
		//  bone setup during sound spatialization if you run into asserts...
	}

	return bret;
}



