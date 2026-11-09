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
#include "soundenvelope.h"
#include "particle_system.h"

//---------------------------------------------------------
//---------------------------------------------------------
#define SAW_MODEL "models/props_junk/sawblade001a.mdl"
#define SAW_SPEED_UP_RATE			30.0f
#define SAW_SPEED_DOWN_RATE		6.0f
#define SAW_MAX_RATE	120.0f
class CAddOnSaw : public CAI_AddOn
{
public:
	DECLARE_CLASS( CAddOnSaw, CAI_AddOn );
	virtual char *GetAddOnModelName() { return SAW_MODEL; }

	//---------------------------------
	// Spawn, etc
	//---------------------------------
	virtual void Spawn()
	{
		BaseClass::Spawn();
		SetTargetSpeed( 0.0f );
	}

	virtual void Precache();
	virtual void UpdateOnRemove();

	virtual float GetThinkInterval() { return 0.05f; }

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

		Q_strcpy( szAttachment, "addon_rear_or_front" );
		if( IsAddOnAttachmentAvailable(pHost, szAttachment) )
		{
			Q_strcpy( pchAttachment, szAttachment );
			return;
		}
	}

	virtual Vector GetAttachOffset( QAngle &attachmentAngles );

	//---------------------------------
	// Install/Remove AddOns
	//---------------------------------
	virtual bool Install( CAI_BaseNPC *pHost, bool bRemoveOnFail = true );
	virtual void Remove( void );

	//---------------------------------
	// Functions specific to this addon
	//---------------------------------
	void SetTargetSpeed( float flTargetSpeed )
	{
		m_flTargetSpeed = flTargetSpeed;
	}

	void SpinSaw();

	//-------------------------------
	// Fields specific to this addon
	//-------------------------------
	float	m_flTargetSpeed;
	float	m_flCurrentSpeed;

	
	CSoundPatch		*m_pEngineSound;

	//-------------------------------
	// Schedules/Tasks/Conditions
	//-------------------------------
	enum 
	{
		SCHED_STOP_SAW = NEXT_SCHEDULE,
		SCHED_START_SAW,
	};

	enum 
	{
		TASK_START_SAW = NEXT_TASK,
		TASK_STOP_SAW,
		TASK_SPIN_SAW,
	};

	enum 
	{
		COND_HOST_ENEMY_NEAR = NEXT_CONDITION,
		COND_HOST_ENEMY_NOT_NEAR,
	};

	DECLARE_DATADESC();
	DEFINE_AGENT();
};

//---------------------------------------------------------
//---------------------------------------------------------
class CAI_SawAddOnBehavior : public CAI_AddOnBehavior<CAddOnSaw>
{
	DECLARE_CLASS( CAI_SawAddOnBehavior, CAI_SimpleBehavior );

public:
	DECLARE_DATADESC();

	CAI_SawAddOnBehavior()
	{
	}

	virtual const char *GetName() {	return "Saw"; }
	virtual bool CanSelectSchedule() { return false; }
	virtual void GatherConditionsNotActive();

private:
};


//---------------------------------------------------------
//---------------------------------------------------------
void CAI_SawAddOnBehavior::GatherConditionsNotActive()
{
	BaseClass::GatherConditionsNotActive();
}


LINK_ENTITY_TO_ADDON_AND_BEHAVIOR( ai_addon_saw, CAddOnSaw, CAI_SawAddOnBehavior );

BEGIN_DATADESC(CAddOnSaw)
	DEFINE_FIELD( m_flTargetSpeed, FIELD_FLOAT ),	
	DEFINE_FIELD( m_flCurrentSpeed, FIELD_FLOAT ),
	DEFINE_SOUNDPATCH( m_pEngineSound ),
END_DATADESC()

BEGIN_DATADESC(CAI_SawAddOnBehavior)
END_DATADESC()

#define SAW_SOUND_PITCH_STARTSTOP	80
#define SAW_SOUND_PITCH_RUNNING		130
//---------------------------------------------------------
//---------------------------------------------------------
bool CAddOnSaw::Install( CAI_BaseNPC *pHost, bool bRemoveOnFail )
{
	if( !BaseClass::Install( pHost, bRemoveOnFail ) )
		return false;

	GetNPCHost()->CapabilitiesAdd( bits_CAP_INNATE_MELEE_ATTACK1 );

	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	CPASAttenuationFilter filter( this );
	m_pEngineSound = controller.SoundCreate( filter, entindex(), "Addon_Saw.Run" );
	if ( m_pEngineSound )
	{
		controller.Play( m_pEngineSound, 0.0, 100 );
	}

	return true;
}

void CAddOnSaw::Remove( void )
{
	BaseClass::Remove();

	if( m_pEngineSound != NULL )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.SoundDestroy( m_pEngineSound );
		m_pEngineSound = NULL;
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
int CAddOnSaw::SelectSchedule()
{
	if( GetNPCHost() )
	{
		if( HasCondition( COND_HOST_ENEMY_NOT_NEAR ) )
			return SCHED_STOP_SAW;

		if( HasCondition( COND_HOST_ENEMY_NEAR ) )
			return SCHED_START_SAW;
	}

	return CAI_Agent::SelectSchedule();
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnSaw::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_START_SAW:
		{
			// The Outer has changed its gravity and jumped.
			SetTargetSpeed( SAW_MAX_RATE );
			if( fabs(m_flCurrentSpeed) < 45.0f )
			{
				EmitSound( "Addon_Saw.Start");
			}

			CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
			if ( m_pEngineSound )
			{
				controller.CommandAdd( m_pEngineSound, 0.0f, SOUNDCTRL_CHANGE_PITCH, 0.5f, SAW_SOUND_PITCH_RUNNING );
				controller.CommandAdd( m_pEngineSound, 0.0f, SOUNDCTRL_CHANGE_VOLUME, 0.5f, 10.0f );
			}
			TaskComplete();
			break;
		}

	case TASK_STOP_SAW:
		{
			SetTargetSpeed( 0.0f );
			CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
			if ( m_pEngineSound )
			{
				controller.CommandAdd( m_pEngineSound, 0.0f, SOUNDCTRL_CHANGE_PITCH, 0.5f, SAW_SOUND_PITCH_STARTSTOP );
				controller.CommandAdd( m_pEngineSound, 0.0f, SOUNDCTRL_CHANGE_VOLUME, 0.5f, 0.0f );
			}
			TaskComplete();
			break;
		}

	case TASK_SPIN_SAW:
		break;

	default:
		CAI_Agent::StartTask( pTask );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnSaw::RunTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_SPIN_SAW:
		SpinSaw();
		break;

	default:
		CAI_Agent::RunTask( pTask );
		break;
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnSaw::Precache()
{
	BaseClass::Precache();
	
	PrecacheScriptSound( "Addon_Saw.Run" );
	PrecacheScriptSound( "Addon_Saw.Start" );
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnSaw::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();

	Remove();
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnSaw::GatherConditions()
{
	BaseClass::GatherConditions();

	ClearCondition( COND_HOST_ENEMY_NOT_NEAR );
	ClearCondition( COND_HOST_ENEMY_NEAR );

	// What if we're aren't attached to a host?
	if( GetNPCHost() && GetNPCHost()->GetEnemy() != NULL )
	{
		float flEnemyDist;

		Vector vecDiff = GetAbsOrigin() - GetHostEnemy()->GetAbsOrigin();
		flEnemyDist = vecDiff.Length2D();

		if( flEnemyDist <= 360.0f )
		{
			SetCondition( COND_HOST_ENEMY_NEAR );

			if( flEnemyDist <= 32.0f )
			{
				CTakeDamageInfo info;

				info.SetDamageType( DMG_SLASH | DMG_CRUSH );

				info.SetAttacker( GetNPCHost() );
				info.SetInflictor( this );
				info.SetDamage( GetHostEnemy()->GetHealth() + 1 );

				GetHostEnemy()->TakeDamage( info );
			}
		}
	}

	if( !HasCondition( COND_HOST_ENEMY_NEAR ) ) 
	{
		SetCondition( COND_HOST_ENEMY_NOT_NEAR );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------

Vector CAddOnSaw::GetAttachOffset( QAngle &attachmentAngles )
{
	Assert( GetNPCHost() );

	Vector vecUp;
	AngleVectors( attachmentAngles, NULL, NULL, &vecUp );
	return vecUp * 1.0;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnSaw::SpinSaw()
{
	if( m_flCurrentSpeed > m_flTargetSpeed )
	{
		m_flCurrentSpeed -= SAW_SPEED_DOWN_RATE;
		m_flCurrentSpeed = max( m_flCurrentSpeed, m_flTargetSpeed );
	}
	else if( m_flCurrentSpeed < m_flTargetSpeed )
	{
		m_flCurrentSpeed += SAW_SPEED_UP_RATE;
		m_flCurrentSpeed = min( m_flCurrentSpeed, m_flTargetSpeed );
	}

	QAngle angle = GetLocalAngles();
	angle.y -= m_flCurrentSpeed;
	SetLocalAngles( angle );
}

AI_BEGIN_AGENT(CAddOnSaw)

	DECLARE_CONDITION( COND_HOST_ENEMY_NEAR )
	DECLARE_CONDITION( COND_HOST_ENEMY_NOT_NEAR )

	DECLARE_TASK( TASK_START_SAW )
	DECLARE_TASK( TASK_STOP_SAW )
	DECLARE_TASK( TASK_SPIN_SAW )

	DEFINE_SCHEDULE
	(
		SCHED_START_SAW,
		"	Tasks"
		"		TASK_START_SAW		0"
		"		TASK_SPIN_SAW			0"
		"	"
		"	Interrupts"
		"		COND_HOST_ENEMY_NOT_NEAR"
	)
		
	DEFINE_SCHEDULE
	(
		SCHED_STOP_SAW,
		"	Tasks"
		"		TASK_STOP_SAW			0"
		"		TASK_SPIN_SAW			0"
		"	Interrupts"
		"		COND_HOST_ENEMY_NEAR"
	)

AI_END_AGENT()

