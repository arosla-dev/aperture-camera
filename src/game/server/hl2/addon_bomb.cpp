//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"

#include "tier1/strtools.h"

#include "ai_behavior.h"
#include "ai_addon.h"
#include "ai_basenpc.h"
#include "ai_squad.h"
#include "explode.h"
#include "ai_behavior_follow.h"
#include "sprite.h"
#include "particle_parse.h"
#include "particle_system.h"

#include "memdbgon.h"

ConVar ai_addon_bomb_explosion_magnitude( "ai_addon_bomb_explosion_magnitude", "300" );
ConVar ai_addon_bomb_explosion_radius( "ai_addon_bomb_explosion_radius", "100" );
ConVar ai_addon_bomb_explosion_force( "ai_addon_bomb_explosion_force", "0" );
ConVar ai_addon_bomb_explode_if_dropped("ai_addon_bomb_explode_if_dropped", "1" );

//---------------------------------------------------------
//---------------------------------------------------------
#define BOMB_MODEL "models/addons/bomb_hat.mdl"
#define BOMB_PING_MAX_DIST 1500 // used to calculate bomb ping frequency
#define BOMB_PING_MIN_INTERVAL .1 // minimum frequency for bomb ping
#define BOMB_PING_MAX_INTERVAL .8 // maximum frequency for bomb ping

#define BOMB_GLOW_SMALL_SCALE .4f
#define BOMB_GLOW_LARGE_SCALE .6f

class CAddOnBomb : public CAI_AddOn
{
public:
	DECLARE_CLASS( CAddOnBomb, CAI_AddOn );
	virtual char *GetAddOnModelName() { return BOMB_MODEL; }

	//---------------------------------
	// Spawn, etc
	//---------------------------------
	bool KeyValue( const char *szKeyName, const char *szValue );
	virtual void Spawn();
	virtual void Precache();

	//---------------------------------
	// AI_Agent	
	//---------------------------------
	void GatherConditions();
	int SelectSchedule();

	void StartTask( const Task_t *pTask );
	void RunTask( const Task_t *pTask );

	//---------------------------------
	// Appearance, position
	//---------------------------------
	virtual void PickAttachment( CAI_BaseNPC *pHost, char *pchAttachment )
	{
		char szAttachment[ 256 ];

		Q_strcpy( szAttachment, "addon_head" );
		if( IsAddOnAttachmentAvailable(pHost, szAttachment) )
		{
			Q_strcpy( pchAttachment, szAttachment );
			return;
		}
	}

	//---------------------------------
	//---------------------------------
	void CreateEffects();
	void CreateBombFlareEffects();
	virtual void EjectFromHost();

	void BroadcastBomb();
	float GetFrequencyForDist();

	bool GetBombGlowSpriteState();

	void Detonate();

	//-------------------------------
	// Entity I/O
	//-------------------------------
	COutputEvent m_OnDetonate;
	void InputSetTarget( inputdata_t &data );

	//-------------------------------
	// Schedules/Tasks/Conditions
	//-------------------------------
	enum 
	{
		SCHED_EXPLODE = NEXT_SCHEDULE,
		SCHED_BROADCAST,
		SCHED_INACTIVE,
	};

	enum 
	{
		TASK_COUNTDOWN = NEXT_TASK,
		TASK_EXPLODE,
		TASK_BROADCAST,
	};

	enum 
	{
		COND_TRIGGER_BOMB = NEXT_CONDITION,
		COND_HAVE_TARGET,
		COND_NO_TARGET,
	};

	string_t m_iszTarget;
	EHANDLE m_hTarget;
	float m_flTriggerRangeSq;
	CStopwatch m_DetonationTimer;
	CSimpleSimTimer m_SignalTimer;
	int m_nEscortsDesired;
	

	DECLARE_DATADESC();
	DEFINE_AGENT();

protected:
	CHandle<CSprite>		m_hBombGlow;
};

//---------------------------------------------------------
//---------------------------------------------------------
// class CAI_BombAddOnFollowBehavior : public CAI_FollowBehavior
// {
// public:
// };
// 

//---------------------------------------------------------
//---------------------------------------------------------
class CAI_BombAddOnBehavior : public CAI_AddOnBehavior<CAddOnBomb>
{
	DECLARE_CLASS( CAI_BombAddOnBehavior, CAI_SimpleBehavior );

public:
	virtual const char *GetName() {	return "Bomb"; }
	virtual void GatherConditionsNotActive()
	{
		if ( NumAddOns() && GetAddOns()[0]->m_hTarget && GetAddOns()[0]->m_nEscortsDesired )
		{
			CAI_Squad *pSquad = GetOuter()->GetSquad();
			if ( pSquad )
			{
				AISquadIter_t iter;
				for ( CAI_BaseNPC *pSquadMember = pSquad->GetFirstMember( &iter ); GetAddOns()[0]->m_nEscortsDesired-- && pSquadMember; pSquadMember = pSquad->GetNextMember( &iter ) )
				{
					if ( pSquadMember != GetOuter() )
					{
						if ( ( pSquadMember->GetAbsOrigin().AsVector2D() - GetOuter()->GetAbsOrigin().AsVector2D() ).LengthSqr() < Square( 600 ) )
						{
							CAI_FollowBehavior *pFollowBehavior;
							if ( pSquadMember->GetBehavior( &pFollowBehavior ) )
							{
//								pFollowBehavior->SetParameters( AIF_WIDE );
								pFollowBehavior->SetFollowTarget( GetOuter() );
							}
						}
					}
				}
			}
		}
		BaseClass::GatherConditionsNotActive();
	}

	virtual bool CanSelectSchedule() 
	{ 
		return false; 
	}

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_ADDON_AND_BEHAVIOR( ai_addon_bomb, CAddOnBomb, CAI_BombAddOnBehavior );

BEGIN_DATADESC(CAddOnBomb)
	DEFINE_KEYFIELD( m_iszTarget, FIELD_STRING, "Target" ),
	DEFINE_FIELD( m_hTarget, FIELD_EHANDLE ),
	DEFINE_KEYFIELD( m_flTriggerRangeSq, FIELD_FLOAT, "TriggerRange" ),
	DEFINE_EMBEDDED( m_DetonationTimer ),
	DEFINE_EMBEDDED( m_SignalTimer ),
	DEFINE_KEYFIELD( m_nEscortsDesired, FIELD_INTEGER, "MaxEscorts" ),
	DEFINE_OUTPUT( m_OnDetonate, "OnDetonate" ),
	DEFINE_FIELD( m_hBombGlow, FIELD_EHANDLE ),
	DEFINE_INPUTFUNC( FIELD_EHANDLE, "SetTarget", InputSetTarget ),
END_DATADESC()

BEGIN_DATADESC(CAI_BombAddOnBehavior)
END_DATADESC()

//---------------------------------------------------------
//---------------------------------------------------------
bool CAddOnBomb::KeyValue( const char *szKeyName, const char *szValue )
{
	if ( V_strcmp( szKeyName, "DetonationTime" ) == 0 )
	{
		m_DetonationTimer.Set( atof( szValue ) );
		return true;
	}

	return BaseClass::KeyValue( szKeyName, szValue );
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnBomb::Spawn()
{
	m_flTriggerRangeSq = Square( m_flTriggerRangeSq );
	BaseClass::Spawn();

	CreateEffects();
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnBomb::Precache()
{
	PrecacheParticleSystem( "striderbuster_explode_dummy_core" );
	PrecacheParticleSystem( "striderbuster_explode_core" );
	PrecacheParticleSystem( "addon_bomb_flare" );
	PrecacheParticleSystem( "striderbuster_break" );

	BaseClass::Precache();
	PrecacheModel("sprites/greenglow1.vmt");

	PrecacheScriptSound( "Weapon_StriderBuster.Ping" );
	PrecacheScriptSound( "Addon_bomb.Ping" );
	PrecacheScriptSound( "Weapon_StriderBuster.Dud_Detonate" );

	PrecacheScriptSound( "Weapon_StriderBuster.Detonate" );
}

//---------------------------------------------------------
// Enjoy all of the physical detachment benefits of the
// base class, but keep thinking so that I'll explode.
//---------------------------------------------------------
void CAddOnBomb::EjectFromHost()
{
	BaseClass::EjectFromHost();

	if( ai_addon_bomb_explode_if_dropped.GetBool() )
	{
		SetThink( &CAI_AddOn::DispatchAddOnThink );
		SetNextThink( gpGlobals->curtime + 0.1f );
	}

	SetAbsVelocity( vec3_origin );

}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnBomb::CreateEffects()
{
	// create and attach a sprite
	m_hBombGlow = CSprite::SpriteCreate( "sprites/greenglow1.vmt", GetLocalOrigin(), false );

	int	nAttachment = LookupAttachment( "light_1" );
	
	Assert( m_hBombGlow );
	if ( m_hBombGlow != NULL )
	{
		m_hBombGlow->FollowEntity( this );
		m_hBombGlow->SetAttachment( this, nAttachment );
		m_hBombGlow->SetTransparency( kRenderGlow, 255, 255, 255, 200, kRenderFxNoDissipation );
		m_hBombGlow->SetScale( BOMB_GLOW_SMALL_SCALE );
		m_hBombGlow->SetGlowProxySize( 4.0f );
		
	}
	else
	{
		Msg( "sprite did not attach to bomb %s!\n", GetDebugName(), this );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnBomb::CreateBombFlareEffects()
{
	
	CParticleSystem *pBombFlare = (CParticleSystem *) CreateEntityByName( "info_particle_system" );

	if ( pBombFlare != NULL )
	{
		pBombFlare->KeyValue( "start_active", "1" );
		pBombFlare->KeyValue( "effect_name", "addon_bomb_flare" );
		pBombFlare->SetParent( this );
		pBombFlare->SetParentAttachment( "SetParentAttachment", "light_1", false );
		pBombFlare->SetLocalOrigin( vec3_origin );
		DispatchSpawn( pBombFlare );
		pBombFlare->Activate();
	}

}
//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnBomb::BroadcastBomb()
{
	// make sure the handle is valid
	if (m_hBombGlow != NULL )
	{
		if ( m_hBombGlow->GetScale() != BOMB_GLOW_SMALL_SCALE )
		{
			m_hBombGlow->SetScale( BOMB_GLOW_SMALL_SCALE, .1f );
		}
		else
		{
			m_hBombGlow->SetScale( BOMB_GLOW_LARGE_SCALE, .1f );
		}
	}
	// broadcast ping
	EmitSound( "Addon_bomb.Ping" );
	
}
//---------------------------------------------------------
//---------------------------------------------------------
float CAddOnBomb::GetFrequencyForDist()
{
	// make sure the handle is valid
	if ( m_hTarget == NULL )
	{
		// no target! set the ping interval to the maximum ping interval
		return BOMB_PING_MAX_INTERVAL;
	}

	// get distance to target
	
	float flDist = m_hTarget->GetAbsOrigin().DistTo( GetAbsOrigin() );

	// calculate frequency of bomb ping based on distance to target

	float flPingInterval =  flDist / BOMB_PING_MAX_DIST ;

	flPingInterval = clamp( flPingInterval, BOMB_PING_MIN_INTERVAL, BOMB_PING_MAX_INTERVAL );

	return flPingInterval;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnBomb::Detonate()
{
	m_hTarget = NULL;

	if( GetNPCHost() == NULL )
	{
		// Just DUD out
		DispatchParticleEffect( "striderbuster_explode_dummy_core", GetAbsOrigin(), QAngle( 0, 0, 1 ) );
		EmitSound( "Weapon_StriderBuster.Dud_Detonate" );
		UTIL_Remove( this );
	}
	else
	{
		m_OnDetonate.FireOutput( this, this );
		ExplosionCreate( GetAbsOrigin(), GetAbsAngles(), GetNPCHost(), 300, 100, true, ai_addon_bomb_explosion_force.GetFloat());
		DispatchParticleEffect( "striderbuster_explode_core", GetAbsOrigin(), QAngle( 0, 0, 1) );
		EmitSound( "Weapon_StriderBuster.Detonate" );
		UTIL_Remove( this );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
int CAddOnBomb::SelectSchedule()
{
	if( HasCondition( COND_TRIGGER_BOMB ) || (HasCondition( COND_ADDON_LOST_HOST ) && ai_addon_bomb_explode_if_dropped.GetBool()) )
		return SCHED_EXPLODE;

	if ( HasCondition( COND_HAVE_TARGET ) )
		return SCHED_BROADCAST;

	return SCHED_INACTIVE;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnBomb::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_COUNTDOWN:
		m_DetonationTimer.Start();
		m_SignalTimer.Force();

		// the countdown has started so play the bomb arming sound
		EmitSound( "Weapon_StriderBuster.Ping" );
		
		// activate the flare effect
		CreateBombFlareEffects();
		break;


	case TASK_EXPLODE:
		//NDebugOverlay::Text( GetAbsOrigin(), "KA-BEWM!", true, 5 );
		Detonate();
		break;

	case TASK_BROADCAST:
		m_SignalTimer.Force();
		break;

	default:
		BaseClass::StartTask( pTask );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnBomb::RunTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_COUNTDOWN:
		if ( m_DetonationTimer.Expired() )
		{
			TaskComplete();
		}
		else if ( m_SignalTimer.Expired() )
		{
			// we're in the countdown so turn off the bomb glow
			if (m_hBombGlow != NULL )
			{
				m_hBombGlow->TurnOff();				
			}

			BroadcastBomb();
			m_SignalTimer.Set( GetFrequencyForDist() );
		}
		break;

	case TASK_BROADCAST:
		if ( m_SignalTimer.Expired() )
		{
			BroadcastBomb();
			m_SignalTimer.Set( GetFrequencyForDist() );
		}
		break;

	case TASK_EXPLODE:
		break;

	default:
		BaseClass::RunTask( pTask );
		break;
	}
}

//---------------------------------------------------------
//---------------------------------------------------------

ConVar ai_addon_bomb_target_player( "ai_addon_bomb_target_player", "0" );
void CAddOnBomb::GatherConditions()
{
	BaseClass::GatherConditions();

	if ( m_iszTarget != NULL_STRING && !m_hTarget )
	{
		if ( V_stricmp( STRING( m_iszTarget ), "!player" ) == 0 )
		{
			m_hTarget = AI_GetSinglePlayer();
		}
		else
		{
			m_hTarget = gEntList.FindEntityByName( NULL, m_iszTarget );
		}

		if ( !m_iszTarget )
		{
			m_iszTarget = NULL_STRING;
		}
	}

	// Hack --------------------
	if ( ai_addon_bomb_target_player.GetBool() && GetNPCHost() )
	{
		m_hTarget = GetNPCHost()->GetEnemy();
		m_flTriggerRangeSq = Square( 20*12 );
		m_DetonationTimer.Set( 5 );
	}
	// Hack --------------------

	if ( m_hTarget )
	{
		SetCondition( COND_HAVE_TARGET );

		if ( !m_DetonationTimer.IsRunning() )
		{
			if ( ( m_hTarget->GetAbsOrigin() - GetAbsOrigin() ).LengthSqr() < m_flTriggerRangeSq )
			{
				SetCondition( COND_TRIGGER_BOMB );
			}
		}
	}
	else
	{
		SetCondition( COND_NO_TARGET );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnBomb::InputSetTarget( inputdata_t &data )
{
	m_hTarget = data.value.Entity();
}

//---------------------------------------------------------
//---------------------------------------------------------
AI_BEGIN_AGENT_( CAddOnBomb, CAI_AddOn )

	DECLARE_CONDITION( COND_TRIGGER_BOMB )
	DECLARE_CONDITION( COND_HAVE_TARGET )
	DECLARE_CONDITION( COND_NO_TARGET )

	DECLARE_TASK( TASK_COUNTDOWN )
	DECLARE_TASK( TASK_EXPLODE )
	DECLARE_TASK( TASK_BROADCAST )

	DEFINE_SCHEDULE
	(
		SCHED_EXPLODE,

		"	Tasks"
		"		TASK_COUNTDOWN		0"
		"		TASK_EXPLODE		0"
		"	"
		"	Interrupts"
		"		COND_NO_TARGET"
		""
	)
		
	DEFINE_SCHEDULE
	(
		SCHED_BROADCAST,

		"	Tasks"
		"		TASK_BROADCAST		0"
		"		TASK_SET_SCHEDULE	SCHEDULE:SCHED_BROADCAST"
		"	"
		"	Interrupts"
		"		COND_TRIGGER_BOMB"
		"		COND_NO_TARGET"
		"		COND_ADDON_LOST_HOST"
		""
	)

	DEFINE_SCHEDULE
	(
		SCHED_INACTIVE,

		"	Tasks"
		"		TASK_ADDON_WAIT		1"
		"		TASK_SET_SCHEDULE	SCHEDULE:SCHED_INACTIVE"
		"	"
		"	Interrupts"
		"		COND_HAVE_TARGET"
		"		COND_ADDON_LOST_HOST"
		""
		""
	)
		
AI_END_AGENT()

