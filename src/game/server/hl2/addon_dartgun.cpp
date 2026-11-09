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
#include "cbaseprojectile.h"
#include "particle_parse.h"
#include "te_effect_dispatch.h"
#include "addon_baseshooter.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define DARTGUN_MODEL "models/addons/dart_gun.mdl"
#define DART_MODEL "models/weapons/hunter_flechette.mdl"

//---------------------------------------------------------
// ConVars which allow customization of the dartgun
//---------------------------------------------------------
ConVar dartgun_dmg( "dartgun_dmg", "2" );
ConVar dartgun_darts( "dartgun_darts", "4" );
ConVar dartgun_speed("dartgun_speed", "1100" );
ConVar dartgun_rof("dartgun_rof", "1.5" );

//---------------------------------------------------------
// hacky global that alternates between left and right 
// hardpoints when attaching dart guns.
//---------------------------------------------------------
bool bAlternate = true;

//---------------------------------------------------------
// Behavior channels
//---------------------------------------------------------
enum
{
	BEHAVIOR_CHANNEL_DARTGUN = 0,
	BEHAVIOR_CHANNEL_UNUSED,
};


//=========================================================
//=========================================================
// This class is for instances of the actual darts fired 
// by the dartgun.
//=========================================================
//=========================================================
class CDartProjectile : public CBaseProjectile
{
	DECLARE_CLASS(CDartProjectile, CBaseProjectile);

public:
	void	Precache();

protected:
	void	HandleTouch( CBaseEntity *pOther );
};

//---------------------------------------------------------
//---------------------------------------------------------
void CDartProjectile::Precache()
{
	BaseClass::Precache();
	PrecacheModel( DART_MODEL );
	PrecacheScriptSound( "Addon_Dartgun.Shoot" );
	PrecacheParticleSystem( "hunter_flechette_trail" );
	PrecacheParticleSystem( "hunter_projectile_explosion_1" );
}
static int s_dartAttachment = 1;

//---------------------------------------------------------
//---------------------------------------------------------
void CDartProjectile::HandleTouch( CBaseEntity *pOther )
{
	if( !pOther->IsPlayer() && pOther->Classify() != CLASS_COMBINE )
	{
		Vector vecFuse;
		GetAttachment( s_dartAttachment, vecFuse );
		DispatchParticleEffect( "hunter_projectile_explosion_1", vecFuse, GetAbsAngles(), NULL );
	}

	EmitSound( "Weapon_StunStick.Activate" );
	BaseClass::HandleTouch( pOther );
}

LINK_ENTITY_TO_CLASS( projectile_dart, CDartProjectile );




//=========================================================
//=========================================================
// This is the class that represents the actual dartgun addon
//=========================================================
//=========================================================
class CAddOnDartgun : public CAI_AddOnBaseShooter
{
public:
	DECLARE_CLASS( CAddOnDartgun, CAI_AddOnBaseShooter );
	virtual char *GetAddOnModelName() { return DARTGUN_MODEL; }

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
	virtual float GetThinkInterval() { return 0.02f; } // Faster than usual.

	//---------------------------------
	// Install/Remove AddOns
	//---------------------------------
	virtual bool Install( CAI_BaseNPC *pHost, bool bRemoveOnFail = true );

	//---------------------------------
	// BaseShooter
	//---------------------------------
	virtual Vector GetMuzzlePos();
	virtual float GetMinRange()			{ return 12.0f; }
	virtual float GetMaxRange()			{ return 1600.0f; }
	virtual float GetMaxDeflection()	{ return 0.94f; } 
	virtual float GetRateOfFire()		{ return dartgun_rof.GetFloat(); }
	virtual float GetRestInterval()		{ return RandomFloat( 1.25, 1.75 ); }

	virtual bool IsTargetLocationOccluded( const Vector &vecTarget );


	virtual void  ShootAt( CBaseEntity *pTarget );
	virtual void StartShootingBurst( bool bPreDelay )	{ BaseClass::StartShootingBurst( bPreDelay ); m_nBurstShotsRemaining = 2; }

	//---------------------------------
	// Functions specific to this addon
	//---------------------------------
	void ShootDart( Vector vecDartDir );
};




//=============================================================================
//=============================================================================
// This is the behavior that gets attached to the NPC that owns a dartgun addon.
// This behavior has both NPC-controlling schedules and simple schedules that
// run on a channel and don't disturb the NPC.
//=============================================================================
//=============================================================================
class CAI_DartgunBehavior : public CAI_AddOnShooterBehavior
{
	DECLARE_CLASS( CAI_DartgunBehavior, CAI_AddOnShooterBehavior );

public:
	virtual const char *GetName() {	return "Dartgun"; }
	virtual bool CanSelectSchedule( void );

public:
	DECLARE_DATADESC();
};

LINK_ENTITY_TO_ADDON_AND_BEHAVIOR( ai_addon_dartgun, CAddOnDartgun, CAI_DartgunBehavior );

BEGIN_DATADESC( CAI_DartgunBehavior )
END_DATADESC()

//---------------------------------------------------------
//---------------------------------------------------------
bool CAI_DartgunBehavior::CanSelectSchedule()
{
	return ( GetEnemy() != NULL );
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnDartgun::Precache()
{
	BaseClass::Precache();
	UTIL_PrecacheOther( "projectile_dart" );
	PrecacheScriptSound( "Weapon_Alyx_Gun.NPC_Single" );
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAddOnDartgun::Install( CAI_BaseNPC *pHost, bool bRemoveOnFail )
{
	if( !BaseClass::Install( pHost, bRemoveOnFail ) )
		return false;

	CAI_DartgunBehavior *pBehavior;
	if ( GetNPCHost()->GetBehavior( &pBehavior ) )
	{
		pBehavior->StartChannel( BEHAVIOR_CHANNEL_DARTGUN );
	}

	return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
QAngle CAddOnDartgun::GetAttachOrientation( QAngle &attachmentAngles )
{
	return BaseClass::GetAttachOrientation( attachmentAngles );
	Assert( GetNPCHost() );
	return attachmentAngles;
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CAddOnDartgun::GetAttachOffset( QAngle &attachmentAngles )
{
	return BaseClass::GetAttachOffset( attachmentAngles );
	Assert( GetNPCHost() );
	return vec3_origin;

	Vector vecUp;
	AngleVectors( attachmentAngles, NULL, NULL, &vecUp );
	return vecUp * 10.0f;
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAddOnDartgun::IsTargetLocationOccluded( const Vector &vecTarget )
{
	bool bVisible = FVisible( vecTarget, MASK_SHOT_HULL );
	return !bVisible;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnDartgun::ShootDart( Vector vecDartDir )
{
	CDartProjectile *pDart = dynamic_cast<CDartProjectile *>( CreateEntityByName( "projectile_dart" ) );
	pDart->Spawn( DART_MODEL, GetMuzzlePos() + vecDartDir * 2.0f, vecDartDir * dartgun_speed.GetFloat() * RandomFloat( 0.9f, 1.1f ), GetNPCHost()->edict(), MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM, dartgun_dmg.GetFloat(), DMG_SHOCK );
	DispatchParticleEffect( "hunter_flechette_trail", PATTACH_ABSORIGIN_FOLLOW, pDart );
	EmitSound( "Addon_Dartgun.Shoot" );

	CEffectData data;
	data.m_nEntIndex = entindex();
	data.m_nAttachmentIndex = LookupAttachment( "muzzle" );
	data.m_flScale = 1.0f;
	data.m_fFlags = MUZZLEFLASH_COMBINE;
	DispatchEffect( "MuzzleFlash", data );

}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnDartgun::ShootAt( CBaseEntity *pTarget )
{
	Vector vecRight, vecUp;

	GetVectors( NULL, &vecRight, &vecUp );

	for( int i = 0 ; i < dartgun_darts.GetInt() ; i++ )
	{
		Vector vecTarget = pTarget->WorldSpaceCenter();
		vecTarget += vecRight * RandomFloat( -24.0f, 24.0f );
		vecTarget += vecUp * RandomFloat( -12.0f, 24.0f );
		Vector vecDir = vecTarget - GetMuzzlePos();
		VectorNormalize( vecDir );
		ShootDart( vecDir );
	}

	CAI_AddOnShooterBehavior *pBehavior;
	GetNPCHost()->GetBehavior( &pBehavior );
	if( pBehavior )
	{
		GetNPCHost()->HandleInteraction( g_interactionAddOnShoot, this, GetNPCHost() );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CAddOnDartgun::GetMuzzlePos()
{
	Vector vecForward;
	AngleVectors( GetAbsAngles(), &vecForward );
	return GetAbsOrigin() + vecForward * 42.0f;
}

