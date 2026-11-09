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
#include "basecombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------
// Behavior channels
//---------------------------------------------------------
enum
{
	BEHAVIOR_CHANNEL_BASECOMBATWEAPON = 0,
	BEHAVIOR_CHANNEL_UNUSED,
};


//=========================================================
//=========================================================
// This is the class that represents the actual 
//=========================================================
//=========================================================
class CAddOnBaseCombatWeapon : public CAI_AddOnBaseShooter
{
public:
	DECLARE_CLASS( CAddOnBaseCombatWeapon, CAI_AddOnBaseShooter );
	virtual char *GetAddOnModelName() { return "weapons/w_shotgun.mdl"; }

	//---------------------------------
	// AI_Agent	
	//---------------------------------
	void Precache();

	//---------------------------------
	// Appearance, position
	//---------------------------------
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
	virtual float GetMinRange()			{ return 10.0f; }
	virtual float GetMaxRange()			{ return 10000.0f; }
	virtual float GetMaxDeflection()	{ return 0.94f; } 
	virtual float GetRateOfFire();
	virtual float GetRestInterval()		{ return RandomFloat( 1.25f, 2.0f ); }

	virtual bool IsTargetLocationOccluded( const Vector &vecTarget );


	virtual void ShootAt( CBaseEntity *pTarget );
	virtual void StartShootingBurst( bool bPreDelay );
};




//=============================================================================
//=============================================================================
// This is the behavior that gets attached to the NPC that owns a basecombatweapon addon.
// This behavior has both NPC-controlling schedules and simple schedules that
// run on a channel and don't disturb the NPC.
//=============================================================================
//=============================================================================
class CAI_BaseCombatWeaponBehavior : public CAI_AddOnShooterBehavior
{
	DECLARE_CLASS( CAI_BaseCombatWeaponBehavior, CAI_AddOnShooterBehavior );

public:
	virtual const char *GetName() {	return "BASECOMBATWEAPON"; }
	virtual bool CanSelectSchedule( void );

public:
	DECLARE_DATADESC();
};

LINK_ENTITY_TO_ADDON_AND_BEHAVIOR( ai_addon_basecombatweapon, CAddOnBaseCombatWeapon, CAI_BaseCombatWeaponBehavior );

BEGIN_DATADESC( CAI_BaseCombatWeaponBehavior )
END_DATADESC()

//---------------------------------------------------------
//---------------------------------------------------------
bool CAI_BaseCombatWeaponBehavior::CanSelectSchedule()
{
	// Be passive!
	return false;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnBaseCombatWeapon::Precache()
{
	BaseClass::Precache();
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAddOnBaseCombatWeapon::Install( CAI_BaseNPC *pHost, bool bRemoveOnFail )
{
	if( !BaseClass::Install( pHost, bRemoveOnFail ) )
		return false;

	CAI_BaseCombatWeaponBehavior *pBehavior;
	if ( GetNPCHost()->GetBehavior( &pBehavior ) )
	{
		pBehavior->StartChannel( BEHAVIOR_CHANNEL_BASECOMBATWEAPON );
	}

	return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
QAngle CAddOnBaseCombatWeapon::GetAttachOrientation( QAngle &attachmentAngles )
{
	return BaseClass::GetAttachOrientation( attachmentAngles );
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CAddOnBaseCombatWeapon::GetAttachOffset( QAngle &attachmentAngles )
{
	return BaseClass::GetAttachOffset( attachmentAngles );
}

float CAddOnBaseCombatWeapon::GetRateOfFire()
{
	CBaseCombatWeapon *pWeapon = dynamic_cast<CBaseCombatWeapon *>( m_hPhysReplacement.Get() );

	if ( pWeapon )
	{
		// Turn the combat weapons rate into shots per second
		return ( 1.0f / pWeapon->GetFireRate() );
	}

	return 1.0;
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAddOnBaseCombatWeapon::IsTargetLocationOccluded( const Vector &vecTarget )
{
	bool bVisible = FVisible( vecTarget, MASK_SHOT_HULL );
	return !bVisible;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnBaseCombatWeapon::ShootAt( CBaseEntity *pTarget )
{
	CBaseCombatWeapon *pBaseCombatWeapon = dynamic_cast<CBaseCombatWeapon*>( m_hPhysReplacement.Get() );

	if ( pBaseCombatWeapon )
	{
		pBaseCombatWeapon->Operator_ForceNPCFire( GetNPCHost(), false, pTarget );
	}

	CAI_BaseCombatWeaponBehavior *pBehavior;
	GetNPCHost()->GetBehavior( &pBehavior );
	if( pBehavior )
	{
		GetNPCHost()->HandleInteraction( g_interactionAddOnShoot, this, GetNPCHost() );
	}
}

void CAddOnBaseCombatWeapon::StartShootingBurst( bool bPreDelay )
{
	BaseClass::StartShootingBurst( bPreDelay );

	CBaseCombatWeapon *pWeapon = dynamic_cast<CBaseCombatWeapon *>( m_hPhysReplacement.Get() );

	if ( pWeapon )
	{
		m_nBurstShotsRemaining = max( 1, pWeapon->GetMaxClip1() );
	}
	else
	{
		m_nBurstShotsRemaining = 1;
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CAddOnBaseCombatWeapon::GetMuzzlePos()
{
	Vector vecForward;
	AngleVectors( GetAbsAngles(), &vecForward );
	return GetAbsOrigin() + vecForward * 42.0f;
}

