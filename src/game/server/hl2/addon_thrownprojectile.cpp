//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "ai_behavior.h"
#include "ai_addon.h"
#include "ai_basenpc.h"
#include "ai_memory.h"
#include "addon_baseshooter.h"
#include "beam_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------
// Behavior channels
//---------------------------------------------------------
enum
{
	BEHAVIOR_CHANNEL_THROWNPROJECTILE = 0,
	BEHAVIOR_CHANNEL_UNUSED,
};


//=========================================================
//=========================================================
// This is the class that represents the actual 
//=========================================================
//=========================================================
class CAddOnThrownProjectile : public CAI_AddOnBaseShooter
{
public:
	DECLARE_CLASS( CAddOnThrownProjectile, CAI_AddOnBaseShooter );
	virtual char *GetAddOnModelName() { return "weapons/w_shotgun.mdl"; }

	//---------------------------------
	// AI_Agent	
	//---------------------------------
	void Precache();

	//---------------------------------
	// Appearance, position
	//---------------------------------
	virtual void PickAttachment( CAI_BaseNPC *pHost, char *pchAttachment )
	{
		char szAttachment[ 256 ];

		Q_strcpy( szAttachment, "addon_bottom" );
		if( IsAddOnAttachmentAvailable(pHost, szAttachment) )
		{
			Q_strcpy( pchAttachment, szAttachment );
		}
	}
	virtual Vector GetAttachOffset( QAngle &attachmentAngles );
	virtual QAngle GetAttachOrientation( QAngle &attachmentAngles );

	//---------------------------------
	// Thinking
	//---------------------------------
	virtual float GetThinkInterval() { return 0.1f; } // Faster than usual.

	//---------------------------------
	// Install/Remove AddOns
	//---------------------------------
	virtual bool Install( CAI_BaseNPC *pHost, bool bRemoveOnFail = true );

	//---------------------------------
	// BaseShooter
	//---------------------------------
	virtual Vector GetMuzzlePos();
	virtual float GetMinRange()			{ return 100.0f; }
	virtual float GetMaxRange()			{ return 1000.0f; }
	virtual float GetMaxDeflection()	{ return 0.94f; } 
	virtual float GetRateOfFire();
	virtual float GetRestInterval()		{ return RandomFloat( 1.25f, 2.0f ); }

	virtual bool IsTargetLocationOccluded( const Vector &vecTarget );


	virtual void ShootAt( CBaseEntity *pTarget );
	virtual void StartShootingBurst( bool bPreDelay );
};




//=============================================================================
//=============================================================================
// This is the behavior that gets attached to the NPC that owns a ThrownProjectile addon.
// This behavior has both NPC-controlling schedules and simple schedules that
// run on a channel and don't disturb the NPC.
//=============================================================================
//=============================================================================
class CAI_ThrownProjectileBehavior : public CAI_AddOnShooterBehavior
{
	DECLARE_CLASS( CAI_ThrownProjectileBehavior, CAI_AddOnShooterBehavior );

public:
	virtual const char *GetName() {	return "THROWNPROJECTILE"; }
	virtual bool CanSelectSchedule( void );

public:
	DECLARE_DATADESC();
};

LINK_ENTITY_TO_ADDON_AND_BEHAVIOR( ai_addon_thrownprojectile, CAddOnThrownProjectile, CAI_ThrownProjectileBehavior );

BEGIN_DATADESC( CAI_ThrownProjectileBehavior )
END_DATADESC()

//---------------------------------------------------------
//---------------------------------------------------------
bool CAI_ThrownProjectileBehavior::CanSelectSchedule()
{
	// Be passive!
	return false;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnThrownProjectile::Precache()
{
	BaseClass::Precache();
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAddOnThrownProjectile::Install( CAI_BaseNPC *pHost, bool bRemoveOnFail )
{
	if( !BaseClass::Install( pHost, bRemoveOnFail ) )
		return false;

	CAI_ThrownProjectileBehavior *pBehavior;
	if ( GetNPCHost()->GetBehavior( &pBehavior ) )
	{
		pBehavior->StartChannel( BEHAVIOR_CHANNEL_THROWNPROJECTILE );
	}

	return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
QAngle CAddOnThrownProjectile::GetAttachOrientation( QAngle &attachmentAngles )
{
	return BaseClass::GetAttachOrientation( attachmentAngles );
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CAddOnThrownProjectile::GetAttachOffset( QAngle &attachmentAngles )
{
	return BaseClass::GetAttachOffset( attachmentAngles );
}

float CAddOnThrownProjectile::GetRateOfFire()
{
	return 1.0;
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAddOnThrownProjectile::IsTargetLocationOccluded( const Vector &vecTarget )
{
	bool bVisible = FVisible( vecTarget, MASK_SHOT_HULL );
	return !bVisible;
}

extern QAngle gangLocalAngles;

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnThrownProjectile::ShootAt( CBaseEntity *pTarget )
{
	CAI_ThrownProjectileBehavior *pBehavior;
	GetNPCHost()->GetBehavior( &pBehavior );
	if( pBehavior )
	{
		GetNPCHost()->HandleInteraction( g_interactionAddOnShoot, this, GetNPCHost() );
	}

	if ( !m_bWasAttached )
	{
		// Not ready to throw yet
		return;
	}

	CBaseEntity *pPhysReplacement = m_hPhysReplacement.Get();

	if ( pPhysReplacement )
	{
		IPhysicsObject *pPhysObject = pPhysReplacement->VPhysicsGetObject();

		if ( pPhysObject )
		{
			// Aim at the head
			Vector vecAimTarget = pTarget->WorldSpaceCenter();
			Vector vecPhysCenter = pPhysReplacement->WorldSpaceCenter();

			m_vecPhysReplacementDetatchForce = ( vecAimTarget - vecPhysCenter ) * 50.0f;

			EmitSound( "Weapon_PhysCannon.Launch" );

			CBeam *pBeam = CBeam::BeamCreate(  "sprites/orangelight1.vmt", 1.8 );

			if ( pBeam != NULL )
			{
				pBeam->PointEntInit( vecPhysCenter, GetNPCHost() );
				pBeam->SetStartEntity( pPhysReplacement );
				pBeam->SetEndAttachment( GetAttachmentID() );
				pBeam->SetWidth( 6.4 );
				pBeam->SetEndWidth( 12.8 );					
				pBeam->SetBrightness( 255 );
				pBeam->SetColor( 255, 255, 255 );
				pBeam->LiveForTime(  0.2f );
				pBeam->RelinkBeam();
				pBeam->SetNoise( 2 );
			}

			Vector	shotDir = m_vecPhysReplacementDetatchForce;
			VectorNormalize( shotDir );

			CPVSFilter filter( vecPhysCenter );
			te->GaussExplosion( filter, 0.0f, vecPhysCenter - ( shotDir * 4.0f ), RandomVector(-1.0f, 1.0f), 0 );
		}
	}

	// Detach it!
	SetThink( &CBaseEntity::SUB_Remove );
}

void CAddOnThrownProjectile::StartShootingBurst( bool bPreDelay )
{
	BaseClass::StartShootingBurst( bPreDelay );
	m_nBurstShotsRemaining = 1;
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CAddOnThrownProjectile::GetMuzzlePos()
{
	Vector vecForward;
	AngleVectors( GetAbsAngles(), &vecForward );
	return GetAbsOrigin() + vecForward * 42.0f;
}

