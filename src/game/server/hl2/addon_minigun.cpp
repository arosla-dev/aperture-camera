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
#include "te_effect_dispatch.h"
#include "addon_baseshooter.h"
#include "ammodef.h"
#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// minigun addon model
#define MINIGUN_MODEL "models/addons/chaingun.mdl"

//---------------------------------------------------------
// ConVars which allow customization of the minigun
//---------------------------------------------------------
ConVar minigun_dmg( "minigun_dmg", "5" );

//---------------------------------------------------------
// Behavior channels
//---------------------------------------------------------
enum
{
	BEHAVIOR_CHANNEL_MINIGUN = 0,
	BEHAVIOR_CHANNEL_UNUSED,
};

//=========================================================
//=========================================================
// This is the class that represents the actual minigun addon
//=========================================================
//=========================================================

class CAddonMinigun : public CAI_AddOnBaseShooter
{
public:
	DECLARE_CLASS( CAddonMinigun, CAI_AddOnBaseShooter );
	virtual char *GetAddOnModelName() {return MINIGUN_MODEL; }

	//---------------------------------
	// CBaseEntity
	//---------------------------------
	void Precache();
	void MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType );

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
	virtual float GetMinRange()			{ return 36.0f; }
	virtual float GetMaxRange()			{ return 1000.0f; }
	virtual float GetMaxDeflection()	{ return 0.94f; } 
	virtual float GetRateOfFire()		{ return 10.0f; }
	virtual float GetRestInterval()		{ return RandomFloat( 0.8f, 1.2f ); }

	virtual void  ShootAt( CBaseEntity *pTarget );
	virtual void StartShootingBurst( bool bPreDelay )	{ BaseClass::StartShootingBurst( bPreDelay ) ;m_nBurstShotsRemaining = random->RandomInt( 8, 14 ); }

	//---------------------------------
	// Functions specific to this addon
	//---------------------------------
	void ShootBullet( Vector vecBulletDir ); // rename this to shoot
};

class CAI_MinigunBehavior : public CAI_AddOnShooterBehavior
{
	DECLARE_CLASS( CAI_MinigunBehavior, CAI_AddOnShooterBehavior );

public:
	virtual const char *GetName() {	return "minigun"; }
	virtual bool CanSelectSchedule( void );

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_ADDON_AND_BEHAVIOR( ai_addon_minigun, CAddonMinigun, CAI_MinigunBehavior );

BEGIN_DATADESC( CAI_MinigunBehavior )
END_DATADESC()

//---------------------------------------------------------
//---------------------------------------------------------
bool CAI_MinigunBehavior::CanSelectSchedule()
{
	// This is fine if I am the authoritative shooter addon. 
	// If I'm only passive, I should say NO and stick to my
	// channel schedules for opportunistic shooting.
	return ( GetEnemy() != NULL );
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddonMinigun::Precache()
{
	PrecacheScriptSound( "AddOn_Minigun.Shoot" );
	PrecacheParticleSystem( "tracer_smokey" );
	BaseClass::Precache();
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddonMinigun::MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType )
{
	if( RandomInt( 0, 1 ) )
	{
		BaseClass::MakeTracer( vecTracerSrc, tr, iTracerType );
	}
	else
	{
		DispatchParticleEffect( "tracer_smokey", vecTracerSrc, tr.endpos, vec3_angle, NULL );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAddonMinigun::Install( CAI_BaseNPC *pHost, bool bRemoveOnFail )
{
	if( !BaseClass::Install( pHost, bRemoveOnFail ) )
		return false;

	CAI_MinigunBehavior *pBehavior;
	if( GetNPCHost()->GetBehavior( &pBehavior ) )
	{
		pBehavior->StartChannel( BEHAVIOR_CHANNEL_MINIGUN );
	}

	return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
QAngle CAddonMinigun::GetAttachOrientation( QAngle &attachmentAngles )
{
	return BaseClass::GetAttachOrientation( attachmentAngles );
	Assert( GetNPCHost() );
	return attachmentAngles;
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CAddonMinigun::GetAttachOffset( QAngle &attachmentAngles )
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
void CAddonMinigun::ShootBullet( Vector vecBulletDir ) // change this to shoot bullet
{
	// put the shoot code here

	FireBulletsInfo_t bulletInfo;

	if ( GetNPCHost()->GetEnemy() != NULL )
	{

		bulletInfo.m_vecSrc = GetMuzzlePos() + vecBulletDir * 12.0f;
		bulletInfo.m_vecDirShooting = vecBulletDir;
		bulletInfo.m_iTracerFreq = 1;
		bulletInfo.m_iShots = 1;
		bulletInfo.m_pAttacker = GetNPCHost();
		bulletInfo.m_vecSpread = VECTOR_CONE_PRECALCULATED;
		bulletInfo.m_flDistance = MAX_COORD_RANGE;
		bulletInfo.m_pAdditionalIgnoreEnt = GetNPCHost();

		//shoots pistol bullets
		bulletInfo.m_iAmmoType = GetAmmoDef()->Index( "Pistol" );

		FireBullets( bulletInfo );
	}

	EmitSound( "Addon_Minigun.Shoot" );

#if 0
	CEffectData data;
	data.m_nAttachmentIndex = LookupAttachment( "muzzle" );
	data.m_nEntIndex = entindex();
	DispatchEffect( "ChopperMuzzleFlash", data );
#else
	CEffectData data;
	data.m_nEntIndex = entindex();
	data.m_nAttachmentIndex = LookupAttachment( "muzzle" );
	data.m_flScale = 0.25f;
	data.m_fFlags = MUZZLEFLASH_COMBINE;

	DispatchEffect( "MuzzleFlash", data );
#endif

	CAI_AddOnShooterBehavior *pBehavior;
	GetNPCHost()->GetBehavior( &pBehavior );
	if( pBehavior )
	{
		GetNPCHost()->HandleInteraction( g_interactionAddOnShoot, this, GetNPCHost() );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddonMinigun::ShootAt( CBaseEntity *pTarget )
{
	Vector vecTrueTarget = pTarget->WorldSpaceCenter();

	Vector vecRight;
	GetNPCHost()->GetVectors( NULL, &vecRight, NULL );

	float flSpread = 28.0f;

	if( pTarget->IsNPC() )
	{
		flSpread *= 0.5f;
	}

	vecTrueTarget += vecRight * flSpread * sin(gpGlobals->curtime * 4.0f);

	Vector vecDir = vecTrueTarget - GetMuzzlePos();
	VectorNormalize( vecDir );
	ShootBullet( vecDir );
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CAddonMinigun::GetMuzzlePos()
{
	
	Vector vecForward;
	AngleVectors( GetAbsAngles(), &vecForward );
	return GetAbsOrigin() + vecForward * 26.0f;

}