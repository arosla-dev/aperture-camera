//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#include "blob_particle.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
//
// CBlobParticle
//

CBlobParticle::CBlobParticle()
: m_type(ROOT_PARTICLE)
, m_vphysParticle(NULL)
, m_parent(NULL)
, m_pgoal(NULL)
//, m_treeParent(NULL)
//, m_oldTreeParent(NULL)
, m_group(NULL)
, m_physParticle()
, m_position(0.0f, 0.0f, 0.0f)
, m_velocity(0.0f, 0.0f, 0.0f)
, m_force(0.0f, 0.0f, 0.0f)
, m_mass(1.0f)
, m_targetSlot(0)
, m_ownedSlot(0)
, m_nArm(0)
, m_nParent(0)
, m_surface(0.0f, 0.0f, 0.0f)
, m_isInContact(false)
, m_flags(0)
, m_support(0.0f)
, m_temp(NULL)
, m_contact(0.0f)
, m_fLastContactCountdown(0.0f)
, m_fLastNonStraggler(0.0f)
, m_smoothedVelocity(0.0f, 0.0f, 0.0f)
, m_neighbors()
, m_surfaces()
, m_nWaypointTargetIndex(-1)
{

}

CBlobGroup::CBlobGroup()
: m_root(NULL)
, m_members()
, m_bIsTentacle( false )
{
}

CBlobTentacle::CBlobTentacle()
: m_nAction( BLOB_TENTACLE_ACTION_NONE )
, m_hAnimationEnt( NULL )
, m_nPhase( 0 )
, m_fPhaseStart( 0 )
, m_fPhaseDuration( 0 )
, m_bPhaseIncomplete( false )
, m_bHasSwooshed( false )
, m_fSpeedMultiplier( 1.0f )
, m_pRoot( NULL )
, m_vecDirection(0,0,0)
, m_vecTarget(0,0,0)
, m_fLength(0)
, m_hTargetEnt( NULL )
, m_vecTargetOffset(0,0,0)
, m_blobGroup( NULL )
{
}

BEGIN_DATADESC_NO_BASE( CBlobTentacle )

	DEFINE_FIELD( m_nAction, FIELD_INTEGER ),
	DEFINE_FIELD( m_hAnimationEnt, FIELD_EHANDLE ),

	DEFINE_FIELD( m_nPhase, FIELD_INTEGER ),
	DEFINE_FIELD( m_fPhaseStart, FIELD_TIME ),
	DEFINE_FIELD( m_fPhaseDuration, FIELD_FLOAT ),
	DEFINE_FIELD( m_bPhaseIncomplete, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bHasSwooshed, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fSpeedMultiplier, FIELD_FLOAT ),

//	DEFINE_FIELD( m_pRoot, CBlobParticle* ),
	DEFINE_FIELD( m_vecDirection, FIELD_VECTOR ),
	DEFINE_FIELD( m_vecTarget, FIELD_VECTOR ),
	DEFINE_FIELD( m_fLength, FIELD_FLOAT ),

	DEFINE_FIELD( m_hTargetEnt, FIELD_EHANDLE ),
	DEFINE_FIELD( m_vecTargetOffset, FIELD_VECTOR ),
	
	// m_blobGroup // CBlobGroups don't get saved, they are reconstructed on restore

END_DATADESC();
