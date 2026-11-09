//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
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
#include "weapon_rpg.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define RPG_MODEL "models/addons/rocket_launcher.mdl"

//---------------------------------------------------------
// Behavior channels
//---------------------------------------------------------
enum
{
	BEHAVIOR_CHANNEL_RPG = 0,
	BEHAVIOR_CHANNEL_UNUSED,
};


//=========================================================
//=========================================================
// This is the class that represents the actual 
//=========================================================
//=========================================================
class CAddOnRPG : public CAI_AddOnBaseShooter
{
public:
	DECLARE_CLASS( CAddOnRPG, CAI_AddOnBaseShooter );
	virtual char *GetAddOnModelName() { return RPG_MODEL; }

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
	virtual float GetMinRange()			{ return 360.0f; }
	virtual float GetMaxRange()			{ return 1600.0f; }
	virtual float GetMaxDeflection()	{ return 0.94f; } 
	virtual float GetRateOfFire()		{ return 1.0; }
	virtual float GetRestInterval()		{ return 5.0f; }

	virtual bool IsTargetLocationOccluded( const Vector &vecTarget );


	virtual void  ShootAt( CBaseEntity *pTarget );
	virtual void StartShootingBurst( bool bPreDelay )	{ BaseClass::StartShootingBurst( bPreDelay ); m_nBurstShotsRemaining = 1; }

	//---------------------------------
	// Functions specific to this addon
	//---------------------------------
	void ShootDart( Vector vecDartDir );
};




//=============================================================================
//=============================================================================
// This is the behavior that gets attached to the NPC that owns a rpg addon.
// This behavior has both NPC-controlling schedules and simple schedules that
// run on a channel and don't disturb the NPC.
//=============================================================================
//=============================================================================
class CAI_RPGBehavior : public CAI_AddOnShooterBehavior
{
	DECLARE_CLASS( CAI_RPGBehavior, CAI_AddOnShooterBehavior );

public:
	virtual const char *GetName() {	return "RPG"; }
	virtual bool CanSelectSchedule( void );

public:
	DECLARE_DATADESC();
};

LINK_ENTITY_TO_ADDON_AND_BEHAVIOR( ai_addon_rpg, CAddOnRPG, CAI_RPGBehavior );

BEGIN_DATADESC( CAI_RPGBehavior )
END_DATADESC()

//---------------------------------------------------------
//---------------------------------------------------------
bool CAI_RPGBehavior::CanSelectSchedule()
{
	// Be passive!
	return false;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnRPG::Precache()
{
	BaseClass::Precache();
	PrecacheScriptSound( "PropAPC.FireRocket" );
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAddOnRPG::Install( CAI_BaseNPC *pHost, bool bRemoveOnFail )
{
	if( !BaseClass::Install( pHost, bRemoveOnFail ) )
		return false;

	CAI_RPGBehavior *pBehavior;
	if ( GetNPCHost()->GetBehavior( &pBehavior ) )
	{
		pBehavior->StartChannel( BEHAVIOR_CHANNEL_RPG );
	}

	return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
QAngle CAddOnRPG::GetAttachOrientation( QAngle &attachmentAngles )
{
	return BaseClass::GetAttachOrientation( attachmentAngles );
	Assert( GetNPCHost() );
	return attachmentAngles;
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CAddOnRPG::GetAttachOffset( QAngle &attachmentAngles )
{
	Assert( GetNPCHost() );
	return BaseClass::GetAttachOffset( attachmentAngles );

	Vector vecUp;
	AngleVectors( attachmentAngles, NULL, NULL, &vecUp );
	return vecUp * -8.0f;
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAddOnRPG::IsTargetLocationOccluded( const Vector &vecTarget )
{
	bool bVisible = FVisible( vecTarget, MASK_SHOT_HULL );
	return !bVisible;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnRPG::ShootAt( CBaseEntity *pTarget )
{
	Vector vecRight, vecUp;

	GetVectors( NULL, &vecRight, &vecUp );

	Vector vecTarget = pTarget->WorldSpaceCenter();
	Vector vecDir = vecTarget - GetMuzzlePos();
	VectorNormalize( vecDir );

	QAngle angles;
	VectorAngles( vecDir, angles );
	CMissile::Create( GetMuzzlePos(), angles, GetNPCHost()->edict() );

	CAI_RPGBehavior *pBehavior;
	GetNPCHost()->GetBehavior( &pBehavior );
	if( pBehavior )
	{
		GetNPCHost()->HandleInteraction( g_interactionAddOnShoot, this, GetNPCHost() );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CAddOnRPG::GetMuzzlePos()
{
	Vector vecForward;
	AngleVectors( GetAbsAngles(), &vecForward );
	return GetAbsOrigin() + vecForward * 42.0f;
}

