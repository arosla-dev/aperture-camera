//
#include "cbase.h"
#include "ai_addon.h"
#include "ai_basenpc.h"
#include "ai_behavior.h"
#include "ieffects.h"

#define SHIELD_MODEL "models/addons/shield_generator.mdl"
#define SHIELD_EFFECT_MODEL "models/effects/shield.mdl"

#define MAX_SHIELD_POWER		50.0f
#define SHIELD_CHARGE_TIME		15.0f
#define SHIELD_THINK_INTERVAL	0.05f

//---------------------------------------------------------
//---------------------------------------------------------
class CShieldEffect : public CBaseAnimating
{
public:
	DECLARE_CLASS( CShieldEffect, CBaseAnimating );

	void Precache()
	{
		BaseClass::Precache();
		PrecacheModel( SHIELD_EFFECT_MODEL );
	}

	void Spawn()
	{
		BaseClass::Spawn();
		SetModel( SHIELD_EFFECT_MODEL );
		SetSolid( SOLID_NONE );
	}
};
LINK_ENTITY_TO_CLASS(addon_shield_effect, CShieldEffect);

//---------------------------------------------------------
//---------------------------------------------------------
class CAddOnShield : public CAI_AddOn
{
public:
	DECLARE_CLASS( CAddOnShield, CAI_AddOn );
	virtual char *GetAddOnModelName() { return SHIELD_MODEL; }

	//---------------------------------
	// Spawn, etc
	//---------------------------------
	virtual void Spawn();
	virtual void Precache()
	{
		BaseClass::Precache();
		UTIL_PrecacheOther( "addon_shield_effect" );
	}

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

		Q_strcpy( szAttachment, "addon_front" );
		if( IsAddOnAttachmentAvailable(pHost, szAttachment) )
		{
			Q_strcpy( pchAttachment, szAttachment );
			return;
		}
	}

	//---------------------------------
	// Think
	//---------------------------------
	float GetThinkInterval() { return 0.1f; }

	//---------------------------------
	// Install/Remove AddOns
	//---------------------------------
	virtual bool Install( CAI_BaseNPC *pHost, bool bRemoveOnFail = true );

	//---------------------------------
	// Functions specific to this addon
	//---------------------------------
	void EnableShield( bool bEnable );
	bool IsShieldEnabled();
	float GetShieldPower() { return m_flShieldPower; }
	float GetShieldPowerPercent() { return GetShieldPower() / MAX_SHIELD_POWER; }

	//-------------------------------
	// Fields specific to this addon
	//-------------------------------
	CHandle<CShieldEffect>m_hShieldEffect;
	float m_flShieldPower;

	//-------------------------------
	// Schedules/Tasks/Conditions
	//-------------------------------
	enum 
	{
		SCHED_MAINTAIN_SHIELD_OFF = NEXT_SCHEDULE,
		SCHED_MAINTAIN_SHIELD_ON,
	};

	enum 
	{
		TASK_MAINTAIN_SHIELD = NEXT_TASK,
		TASK_SHIELD_POWER,
	};

	enum 
	{
		COND_SHIELD_FULL = NEXT_CONDITION,
		COND_SHIELD_EXHAUSTED,
		COND_REQUEST_SHIELD_ON,
		COND_REQUEST_SHIELD_OFF,
	};

	DECLARE_DATADESC();
	DEFINE_AGENT();
};
BEGIN_DATADESC(CAddOnShield)
	DEFINE_FIELD( m_hShieldEffect, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flShieldPower, FIELD_FLOAT ),
END_DATADESC()

//---------------------------------------------------------
//---------------------------------------------------------
class CAI_ShieldAddOnBehavior : public CAI_AddOnBehavior<CAddOnShield>
{
	DECLARE_CLASS( CAI_ShieldAddOnBehavior, CAI_SimpleBehavior );

public:
	DECLARE_DATADESC();

	virtual const char *GetName() {	return "Shield"; }
	
	virtual void GatherConditions();

	virtual bool CanSelectSchedule( void );
	virtual int SelectSchedule();
	virtual int TranslateSchedule( int scheduleType );

	virtual int	OnTakeDamage_Alive( const CTakeDamageInfo &info );

	bool IsValidCover( const Vector &vLocation, CAI_Hint const *pHint );

	CAddOnShield *AccessShieldAddOn() { return ( m_AddOns.Count() ) ? m_AddOns[0] : NULL; }

	enum
	{
		SCHED_NPC_WAIT_FOR_SHIELD_CHARGE = BaseClass::NEXT_SCHEDULE,
		SCHED_NPC_SHOOT_ON_THE_RUN,
	};

	enum
	{
		COND_NOTIFY_SHIELD_FULLY_CHARGED_DUPLICATE = BaseClass::NEXT_CONDITION,
		COND_NOTIFY_ENEMY_TOO_CLOSE,
	};

public:
	DEFINE_CUSTOM_SCHEDULE_PROVIDER;
};

BEGIN_DATADESC( CAI_ShieldAddOnBehavior )
END_DATADESC()


LINK_ENTITY_TO_ADDON_AND_BEHAVIOR( ai_addon_shield, CAddOnShield, CAI_ShieldAddOnBehavior );

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_ShieldAddOnBehavior::GatherConditions()
{
	BaseClass::GatherConditions();

	if( AccessShieldAddOn()->GetShieldPowerPercent() == 1.0f )
	{
		SetCondition( COND_NOTIFY_SHIELD_FULLY_CHARGED_DUPLICATE );
	}
	else
	{
		ClearCondition( COND_NOTIFY_SHIELD_FULLY_CHARGED_DUPLICATE );
	}

	if( AccessShieldAddOn() && AccessShieldAddOn()->IsShieldEnabled() )
	{
		byte opacity;
		opacity = ((int) (255.0f * AccessShieldAddOn()->GetShieldPowerPercent()) );
		AccessShieldAddOn()->m_hShieldEffect->SetRenderColor( opacity, opacity, opacity, opacity );

		 if( AccessShieldAddOn()->GetShieldPower() <= 0.0f )
		 {
			 AccessShieldAddOn()->EnableShield( false );
		 }
	}

	ClearCondition( COND_NOTIFY_ENEMY_TOO_CLOSE );
	if( GetEnemy() && HasCondition( COND_SEE_ENEMY ) )
	{
		if( GetAbsOrigin().DistTo(GetEnemy()->GetAbsOrigin()) <= (30.0f *12.0f) )
		{
			SetCondition( COND_NOTIFY_ENEMY_TOO_CLOSE );
		}
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAI_ShieldAddOnBehavior::CanSelectSchedule( void )
{
	return true;

	if( GetOuter() && GetOuter()->GetEnemy() )
	{
		return true;
	}

	return false;
}

//---------------------------------------------------------
//---------------------------------------------------------
int CAI_ShieldAddOnBehavior::SelectSchedule()
{
	return BaseClass::SelectSchedule();

	if( HasMemory(bits_MEMORY_INCOVER) && GetEnemy() != NULL )
	{
		if( !HasCondition( COND_CAN_RANGE_ATTACK1 ) && AccessShieldAddOn()->GetShieldPowerPercent() < 1.0f )
		{
			// Opportunistically reload here.
			if( HasCondition(COND_LOW_PRIMARY_AMMO) || HasCondition(COND_NO_PRIMARY_AMMO) )
				return SCHED_RELOAD;

			return SCHED_NPC_WAIT_FOR_SHIELD_CHARGE;
		}

		return SCHED_NPC_SHOOT_ON_THE_RUN;
	}

	if( GetEnemy() != NULL )
	{
		if( HasCondition( COND_NOTIFY_ENEMY_TOO_CLOSE ) ) 
		{
			return SCHED_BACK_AWAY_FROM_ENEMY;
		}
	}

	return SCHED_NPC_SHOOT_ON_THE_RUN;
	
	return BaseClass::SelectSchedule();
}

//---------------------------------------------------------
//---------------------------------------------------------
int CAI_ShieldAddOnBehavior::TranslateSchedule( int scheduleType ) 
{
	return BaseClass::TranslateSchedule( scheduleType );
}


//---------------------------------------------------------
//---------------------------------------------------------
int	CAI_ShieldAddOnBehavior::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	if( AccessShieldAddOn() )
	{
		if( AccessShieldAddOn()->IsShieldEnabled() )
		{
			g_pEffects->Sparks( info.GetDamagePosition(), 3, 2 );
			AccessShieldAddOn()->m_flShieldPower -= info.GetDamage();
			return 0;
		}
		else if( AccessShieldAddOn()->GetShieldPowerPercent() >= 0.8f )
		{
			// Take no damage from this attack and turn the shield on.
			AccessShieldAddOn()->EnableShield(true);
			return 0;
		}
	}

	return BaseClass::OnTakeDamage_Alive( info );
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAI_ShieldAddOnBehavior::IsValidCover( const Vector &vLocation, CAI_Hint const *pHint )
{
	// Don't run any of the code below. That's from an older idea (sjb 12/18/2007)
	return BaseClass::IsValidCover( vLocation, pHint );

	if( !GetOuter()->GetEnemy() )
		return BaseClass::IsValidCover( vLocation, pHint );

	if( BaseClass::IsValidCover( vLocation, pHint ) )
	{
		// If the base class thinks this is cover, then we check it. It has to take us closer to our enemy if we have shield power to cover the move.
		if( AccessShieldAddOn()->GetShieldPower() >= (MAX_SHIELD_POWER * 0.5f) )
		{
			Vector vecEnemyOrigin = GetOuter()->GetEnemy()->GetAbsOrigin();

			float flMyDistToEnemy;
			float flCoverDistToEnemy;
			float flMyDistToCover;

			flMyDistToCover = vLocation.DistToSqr( GetAbsOrigin() );
			flMyDistToEnemy = GetAbsOrigin().DistToSqr( vecEnemyOrigin );
			flCoverDistToEnemy = vLocation.DistToSqr( vecEnemyOrigin );

			if( flCoverDistToEnemy + Square(72.0f) < flMyDistToEnemy && flMyDistToCover >= Square(60.0f) )
			{
				NDebugOverlay::Line( WorldSpaceCenter(), vLocation, 0, 255, 0, false, 5.0f );
				return true;
			}

			NDebugOverlay::Line( WorldSpaceCenter(), vLocation, 255, 0, 0, false, 5.0f );
			return false;
		}
	}

	return false;
}

AI_BEGIN_CUSTOM_SCHEDULE_PROVIDER( CAI_ShieldAddOnBehavior )

	DECLARE_CONDITION( COND_NOTIFY_SHIELD_FULLY_CHARGED_DUPLICATE )
	DECLARE_CONDITION( COND_NOTIFY_ENEMY_TOO_CLOSE )


	//DECLARE_TASK( TASK_ACTBUSY_VERIFY_EXIT )

	//---------------------------------

	DEFINE_SCHEDULE
	( 
		SCHED_NPC_WAIT_FOR_SHIELD_CHARGE,

		"	Tasks"
		"		TASK_STOP_MOVING					0"
		"		TASK_WAIT_INDEFINITE				0"
		""
		"	Interrupts"
		"		COND_SEE_ENEMY"
		"		COND_LIGHT_DAMAGE"
		"		COND_CAN_RANGE_ATTACK1"
		"		COND_IDLE_INTERRUPT"
		"		COND_NOTIFY_SHIELD_FULLY_CHARGED_DUPLICATE"
	)

	DEFINE_SCHEDULE
	(
		SCHED_NPC_SHOOT_ON_THE_RUN,

		"	Tasks"
		"		TASK_FIND_COVER_FROM_ENEMY			0"
		"		TASK_RUN_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_SET_SCHEDULE					SCHEDULE:SCHED_NPC_WAIT_FOR_SHIELD_CHARGE"
		"	Interrupts"
		""
	)
 AI_END_CUSTOM_SCHEDULE_PROVIDER()


//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnShield::Spawn()
{
	BaseClass::Spawn();
	CShieldEffect *pShield = dynamic_cast<CShieldEffect*>(CreateEntityByName( "addon_shield_effect" ));
	Assert( pShield != NULL );
	pShield->Spawn();

	Vector vecForward;

	GetVectors( &vecForward, NULL, NULL );

	pShield->SetAbsOrigin( GetAbsOrigin() );
	pShield->SetAbsAngles( vec3_angle );

	m_hShieldEffect.Set( pShield );
	m_hShieldEffect->SetParent( this );
	m_flShieldPower = MAX_SHIELD_POWER;
	EnableShield( false );
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAddOnShield::Install( CAI_BaseNPC *pHost, bool bRemoveOnFail )
{
	if( FClassnameIs( pHost, "npc_soldier_large") )
	{
		// The present shield effect looks ridiculous on the big soldiers.
		UTIL_Remove( this );
		return false;
	}

	if( !BaseClass::Install( pHost, bRemoveOnFail ) )
		return false;

	m_hShieldEffect->SetAbsOrigin( pHost->GetAbsOrigin() );
	m_hShieldEffect->SetAbsAngles( pHost->GetAbsAngles() );
	m_hShieldEffect->SetParent( pHost );

	return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnShield::EnableShield( bool bEnable )
{
	Assert( m_hShieldEffect != NULL );

	if( bEnable )
	{
		if( GetNPCHost() )
			GetNPCHost()->SetBloodColor( DONT_BLEED );

		m_hShieldEffect->RemoveEffects( EF_NODRAW );
	}
	else
	{
		if( GetNPCHost() )
			GetNPCHost()->SetBloodColor( BLOOD_COLOR_YELLOW );

		m_hShieldEffect->AddEffects( EF_NODRAW );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAddOnShield::IsShieldEnabled()
{
	Assert( m_hShieldEffect != NULL );
	return !m_hShieldEffect->IsEffectActive( EF_NODRAW );
}

//---------------------------------------------------------
//---------------------------------------------------------
int CAddOnShield::SelectSchedule()
{
	if( HasCondition(COND_REQUEST_SHIELD_ON) )
		return SCHED_MAINTAIN_SHIELD_ON;

	if( HasCondition( COND_SHIELD_EXHAUSTED) )
		return SCHED_MAINTAIN_SHIELD_OFF;

	return SCHED_MAINTAIN_SHIELD_OFF;
	return CAI_Agent::SelectSchedule();
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnShield::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_SHIELD_POWER:
		if( pTask->flTaskData == 0 )
		{
			EnableShield( false );
		}
		else
		{
			EnableShield( true );
		}
		TaskComplete();
		break;

	case TASK_MAINTAIN_SHIELD:
		// Infinite... (see Run Task)
		break;

	default:
		CAI_Agent::StartTask( pTask );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnShield::RunTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_MAINTAIN_SHIELD:
		if( IsShieldEnabled() )
		{
			//m_flShieldPower -= 0.1f; Shield doesn't use power unless I'm being harmed.
			m_flShieldPower = max( 0.0f, m_flShieldPower );
		}
		else
		{
			// Recharge.
			m_flShieldPower += (MAX_SHIELD_POWER/SHIELD_CHARGE_TIME) * 0.1f;
			m_flShieldPower = min( MAX_SHIELD_POWER, m_flShieldPower );
		}
		break;

	default:
		CAI_Agent::RunTask( pTask );
		break;
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnShield::GatherConditions()
{
	// Stop the shield power from going below zero and taking too long to recharge
	m_flShieldPower = max( 0.0f, m_flShieldPower );

	BaseClass::GatherConditions();

	if( !GetNPCHost() )
		return;

	if( m_flShieldPower <= 0.0f )
	{
		SetCondition( COND_SHIELD_EXHAUSTED );
	}
	else
	{
		ClearCondition( COND_SHIELD_EXHAUSTED );
	}

	if( m_flShieldPower == MAX_SHIELD_POWER )
	{
		SetCondition( COND_SHIELD_FULL );
	}
	else
	{
		ClearCondition( COND_SHIELD_FULL );
	}

	ClearCondition( COND_REQUEST_SHIELD_OFF );
	ClearCondition( COND_REQUEST_SHIELD_ON );
}

AI_BEGIN_AGENT(CAddOnShield)

DECLARE_CONDITION( COND_SHIELD_EXHAUSTED )
DECLARE_CONDITION( COND_SHIELD_FULL )
DECLARE_CONDITION( COND_REQUEST_SHIELD_ON )
DECLARE_CONDITION( COND_REQUEST_SHIELD_OFF )

DECLARE_TASK( TASK_MAINTAIN_SHIELD )
DECLARE_TASK( TASK_SHIELD_POWER )

DEFINE_SCHEDULE
(
 SCHED_MAINTAIN_SHIELD_OFF,
 "	Tasks"
 "		TASK_SHIELD_POWER					0"
 "		TASK_MAINTAIN_SHIELD				0"
 "	"
 "	Interrupts"
 "		COND_REQUEST_SHIELD_ON"
 )

 DEFINE_SCHEDULE
 (
 SCHED_MAINTAIN_SHIELD_ON,
 "	Tasks"
 "		TASK_SHIELD_POWER					1"
 "		TASK_MAINTAIN_SHIELD				0"
 "	"
 "	Interrupts"
 "		COND_SHIELD_EXHAUSTED"
 "		COND_REQUEST_SHIELD_OFF"
 )

 AI_END_AGENT()

