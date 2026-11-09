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
#include "ai_squad.h"
#include "soundenvelope.h"

#include "ai_route.h"
#include "ai_moveprobe.h"

#include "particle_system.h"

ConVar propeller_gravity( "propeller_gravity", "0.75" );
ConVar propeller_timer( "propeller_timer", "4.0" );

//---------------------------------------------------------
//---------------------------------------------------------
#define PROPELLER_MODEL "models/addons/propeller.mdl"
#define PROPELLER_SPEED_UP_RATE			30.0f
#define PROPELLER_SPEED_DOWN_RATE		6.0f
#define PROPELLER_MAX_RATE	120.0f
class CAddOnPropeller : public CAI_AddOn
{
public:
	DECLARE_CLASS( CAddOnPropeller, CAI_AddOn );
	virtual char *GetAddOnModelName() { return PROPELLER_MODEL; }

	//---------------------------------
	// Spawn, etc
	//---------------------------------
	virtual void Spawn()
	{
		BaseClass::Spawn();
		SetTargetSpeed( 0.0f );
	}
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

	void SpinPropeller();

	void StartRotorWash();
	void StopRotorWash();

	//-------------------------------
	// Fields specific to this addon
	//-------------------------------
	float	m_flTargetSpeed;
	float	m_flCurrentSpeed;

	
	CSoundPatch		*m_pEngineSound;

	EHANDLE			m_hRotorWash;


	//-------------------------------
	// Schedules/Tasks/Conditions
	//-------------------------------
	enum 
	{
		SCHED_STOP_PROPELLER = NEXT_SCHEDULE,
		SCHED_START_PROPELLER,
	};

	enum 
	{
		TASK_START_PROPELLER = NEXT_TASK,
		TASK_STOP_PROPELLER,
		TASK_SPIN_PROPELLER,
	};

	enum 
	{
		COND_HOST_JUMPING = NEXT_CONDITION,
		COND_HOST_NOT_JUMPING,
	};


	DECLARE_DATADESC();
	DEFINE_AGENT();
};

//---------------------------------------------------------
//---------------------------------------------------------
class CAI_PropellerAddOnBehavior : public CAI_AddOnBehavior<CAddOnPropeller>
{
	DECLARE_CLASS( CAI_PropellerAddOnBehavior, CAI_SimpleBehavior );

public:
	DECLARE_DATADESC();

	CAI_PropellerAddOnBehavior()
	{
		m_flTimeAttemptJumpNav = gpGlobals->curtime + RandomFloat( 0, 2 );
		m_flJumpGravityVariance = RandomFloat( -0.10, 0.10 );
	}

	virtual const char *GetName() {	return "Propeller"; }
	virtual bool CanSelectSchedule() { return false; }
	virtual void GatherConditionsNotActive();

	virtual float GetJumpGravity() const;
	virtual bool IsJumpLegal( const Vector &startPos, const Vector &apex, const Vector &endPos, float maxUp, float maxDown, float maxDist ) const;
	virtual bool MovementCost( int moveType, const Vector &vecStart, const Vector &vecEnd, float *pCost );

private:
	float	m_flTimeAttemptJumpNav;	// Next time to try to mutate a waypoint in my route to a jump waypoint.
	float	m_flJumpGravityVariance;
};


//---------------------------------------------------------
// !!!BUGBUG - right now ignoring waypoints that should not
// be reduced away
// @NOTE: this function is potentially pretty damned expensive
//---------------------------------------------------------
ConVar min_force_jump_dist("min_force_jump_dist", "45");
ConVar max_force_jump_dist("max_force_jump_dist", "120");
ConVar min_force_jumptime("min_force_jumptime", "5" );
ConVar min_force_jump_min_dist_from_goal("min_force_jump_min_dist_from_goal", "10" );
ConVar max_simultaneous_jumpers("max_simultaneous_sqad_jumpers", "2" );
void CAI_PropellerAddOnBehavior::GatherConditionsNotActive()
{
	BaseClass::GatherConditionsNotActive();

	// If my timer is up, try to force a nav jump.
	if( GetOuter()->IsMoving() && GetOuter()->GetNavigator()->GetNavType() == NAV_GROUND && gpGlobals->curtime > m_flTimeAttemptJumpNav )
	{
		// Set us up to try in the future. Set this timer low, in case we fail
		m_flTimeAttemptJumpNav = gpGlobals->curtime + RandomFloat( 1, 2 );

		CAI_Squad *pSquad = GetOuter()->GetSquad();
		int nAvailableJumpSlots = max_simultaneous_jumpers.GetInt();
		if ( pSquad )
		{
			AISquadIter_t iter;
			for ( CAI_BaseNPC *pSquadMember = pSquad->GetFirstMember( &iter ); nAvailableJumpSlots && pSquadMember; pSquadMember = pSquad->GetNextMember( &iter ) )
			{
				if ( pSquadMember != GetOuter() && pSquadMember->GetNavigator()->GetNavType() == NAV_JUMP )
				{
					nAvailableJumpSlots--;
				}
			}
		}

		if ( nAvailableJumpSlots )
		{
			float flHalfMinForceDistSqr = Square( ( min_force_jump_dist.GetFloat() / 2.0 ) * 12 );
			float flMinForceDistSqr = Square( min_force_jump_dist.GetFloat() * 12 );
			float flMaxForceDistSqr = Square( max_force_jump_dist.GetFloat() * 12 );
			float flMinDistFromGoalSqr = Square( min_force_jump_min_dist_from_goal.GetFloat() * 12 );

			CAI_Path *pPath = GetNavigator()->GetPath();

			if( pPath != NULL && !pPath->IsEmpty() )
			{
				AI_Waypoint_t *pGoal = pPath->GetGoalWaypoint();
				AI_Waypoint_t *pDest = pGoal;

				while( pDest != NULL )
				{
					float flDistSqr = GetAbsOrigin().DistToSqr( pDest->GetPos() );

					if( ( ( pDest == pGoal && flDistSqr >= flHalfMinForceDistSqr) || flDistSqr >= flMinForceDistSqr ) && flDistSqr <= flMaxForceDistSqr )
					{
						if ( pDest == pGoal || pDest->GetPos().DistToSqr( pGoal->GetPos() ) > flMinDistFromGoalSqr )
						{
							// This segment is long enough. Ask the move probe if the jump is legal.
							if( GetOuter()->GetMoveProbe()->MoveLimit( NAV_JUMP, GetAbsOrigin(), pDest->GetPos(), MASK_NPCSOLID, NULL ) )
							{
								// This is a success case. Wait for the full duration of the timer.
								m_flTimeAttemptJumpNav = gpGlobals->curtime + min_force_jumptime.GetFloat() + RandomFloat( 0, 3 );
								//Msg( "Go! %.2f\n", sqrt(flDistSqr) / 12.0 );

								pDest->SetNavType( NAV_JUMP );

								AI_Waypoint_t *pOldCur = pPath->GetCurWaypoint();

								if( pDest->GetPrev() )
								{
									pDest->GetPrev()->SetNext( NULL );
									DeleteAll( pOldCur );
									pPath->SetWaypoints( pDest );
								}

								pPath->PrependWaypoint( GetAbsOrigin(), NAV_GROUND, 0 );
								break;
							}
						}
					}

					pDest = pDest->GetPrev();
				}
			}
		}
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
float CAI_PropellerAddOnBehavior::GetJumpGravity() const
{
	return GetOuter()->GetDefaultJumpGravity() * ( ( propeller_gravity.GetFloat() + m_flJumpGravityVariance ) * NumAddOns() );
}

//---------------------------------------------------------
//---------------------------------------------------------
ConVar propeller_maxdist("propeller_maxdist", "2400" );
bool CAI_PropellerAddOnBehavior::IsJumpLegal( const Vector &startPos, const Vector &apex, const Vector &endPos, float maxUp, float maxDown, float maxDist ) const
{
	Vector vecDiff = endPos - startPos;

	if( fabs(vecDiff.z) <= 24.0f )
	{
		// This jump will end up more or less level with where we are now, so prohibit the jump if it is shorter than 15 feet
		return (vecDiff.Length2DSqr() >= Square( 15.0f * 12.0f ));
	}

	if( fabs( vecDiff.z ) > 900.0f )
		return false;

	if( vecDiff.Length2D() > propeller_maxdist.GetFloat() )
		return false;

	return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
#define FEET	50.0f
bool CAI_PropellerAddOnBehavior::MovementCost( int moveType, const Vector &vecStart, const Vector &vecEnd, float *pCost )
{
	if( moveType == NAV_JUMP )
	{
		Vector vecDiff = vecEnd - vecStart;

		// Now we make jumps with larger vertical components cheaper.
		if( fabs(vecDiff.z) >= (8.0f * 12.0f) )
		{
			//NDebugOverlay::Line( vecStart, vecEnd, 255, 255, 255, false, 20.0f );
			*pCost *= 0.5f;
			return true;
		}

		if( vecDiff.Length2D() > 240.0f && RandomInt(1, 75) == 1 )
		{
			*pCost *= 0.5f;
			return true;
		}
	}

	return false;
}

LINK_ENTITY_TO_ADDON_AND_BEHAVIOR( ai_addon_propeller, CAddOnPropeller, CAI_PropellerAddOnBehavior );

BEGIN_DATADESC(CAddOnPropeller)
	DEFINE_FIELD( m_flTargetSpeed, FIELD_FLOAT ),	
	DEFINE_FIELD( m_flCurrentSpeed, FIELD_FLOAT ),
	DEFINE_SOUNDPATCH( m_pEngineSound ),
	DEFINE_FIELD( m_hRotorWash, FIELD_EHANDLE ),
END_DATADESC()

BEGIN_DATADESC(CAI_PropellerAddOnBehavior)
	DEFINE_FIELD( m_flTimeAttemptJumpNav, FIELD_TIME ),
	DEFINE_FIELD( m_flJumpGravityVariance, FIELD_FLOAT ),
END_DATADESC()

#define PROPELLER_SOUND_PITCH_STARTSTOP	80
#define PROPELLER_SOUND_PITCH_RUNNING	110
//---------------------------------------------------------
//---------------------------------------------------------
bool CAddOnPropeller::Install( CAI_BaseNPC *pHost, bool bRemoveOnFail )
{
	if( !BaseClass::Install( pHost, bRemoveOnFail ) )
		return false;

	// Margarita !!!HACK !!!BUG - This screws us if the NPC naturally had bits_CAP_MOVE_JUMP prior to us being attached.
	// I mean, if we REMOVE this later with the addon and the NPC already had it innately, we screw ourselves.
	GetNPCHost()->CapabilitiesAdd( bits_CAP_MOVE_JUMP );
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	CPASAttenuationFilter filter( this );
	m_pEngineSound = controller.SoundCreate( filter, entindex(), "Addon_Propeller.RunHL1" );
	if ( m_pEngineSound )
	{
		controller.Play( m_pEngineSound, 0.0, 100 );
	}

	return true;
}

void CAddOnPropeller::Remove( void )
{
	BaseClass::Remove();

	if( m_pEngineSound != NULL )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		if ( m_pEngineSound )
		{
			controller.SoundDestroy( m_pEngineSound );
		}
		m_pEngineSound = NULL;
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
int CAddOnPropeller::SelectSchedule()
{
	if( HasCondition( COND_HOST_NOT_JUMPING ) )
		return SCHED_STOP_PROPELLER;

	if( HasCondition( COND_HOST_JUMPING ) )
		return SCHED_START_PROPELLER;

	return CAI_Agent::SelectSchedule();
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnPropeller::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_START_PROPELLER:
		{
			// The Outer has changed its gravity and jumped.
			SetTargetSpeed( PROPELLER_MAX_RATE );
			CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
			if ( m_pEngineSound )
			{
				controller.CommandAdd( m_pEngineSound, 0.0f, SOUNDCTRL_CHANGE_PITCH, 0.5f, PROPELLER_SOUND_PITCH_RUNNING );
				controller.CommandAdd( m_pEngineSound, 0.0f, SOUNDCTRL_CHANGE_VOLUME, 0.5f, 10.0f );
			}

			StartRotorWash();

			TaskComplete();
			break;
		}

	case TASK_STOP_PROPELLER:
		{
			SetTargetSpeed( 0.0f );
			CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
			if ( m_pEngineSound )
			{
				controller.CommandAdd( m_pEngineSound, 0.0f, SOUNDCTRL_CHANGE_PITCH, 0.5f, PROPELLER_SOUND_PITCH_STARTSTOP );
				controller.CommandAdd( m_pEngineSound, 0.0f, SOUNDCTRL_CHANGE_VOLUME, 0.5f, 0.0f );
			}

			StopRotorWash();
			TaskComplete();
			break;
		}

	case TASK_SPIN_PROPELLER:
		break;

	default:
		CAI_Agent::StartTask( pTask );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnPropeller::RunTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_SPIN_PROPELLER:
		SpinPropeller();
		break;

	default:
		CAI_Agent::RunTask( pTask );
		break;
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnPropeller::Precache()
{
	BaseClass::Precache();
	
	PrecacheScriptSound( "Addon_Propeller.Run" );
	PrecacheScriptSound( "Addon_Propeller.RunHL1" );
	PrecacheParticleSystem("soldier_propeller_wash");
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnPropeller::GatherConditions()
{
	BaseClass::GatherConditions();

	// What if we're aren't attached to a host?
	if( GetNPCHost() )
	{
		Activity act = GetNPCHost()->GetIdealActivity();

		if( act == ACT_JUMP || act == ACT_GLIDE )
		{
			SetCondition( COND_HOST_JUMPING );
			ClearCondition( COND_HOST_NOT_JUMPING );
		}
		else
		{
			SetCondition( COND_HOST_NOT_JUMPING );
			ClearCondition( COND_HOST_JUMPING );
		}
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CAddOnPropeller::GetAttachOffset( QAngle &attachmentAngles )
{
	Assert( GetNPCHost() );
	return BaseClass::GetAttachOffset( attachmentAngles );

	Vector vecUp;
	AngleVectors( attachmentAngles, NULL, NULL, &vecUp );
	return vecUp * 3.0f;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnPropeller::SpinPropeller()
{
	if( m_flCurrentSpeed > m_flTargetSpeed )
	{
		m_flCurrentSpeed -= PROPELLER_SPEED_DOWN_RATE;
		m_flCurrentSpeed = max( m_flCurrentSpeed, m_flTargetSpeed );
	}
	else if( m_flCurrentSpeed < m_flTargetSpeed )
	{
		m_flCurrentSpeed += PROPELLER_SPEED_UP_RATE;
		m_flCurrentSpeed = min( m_flCurrentSpeed, m_flTargetSpeed );
	}

	QAngle angle = GetLocalAngles();
	angle.y += m_flCurrentSpeed;
	SetLocalAngles( angle );
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnPropeller::StartRotorWash()
{
	if( m_hRotorWash )
		return;

	CParticleSystem *pEffect = (CParticleSystem *) CreateEntityByName( "info_particle_system" );
	if ( pEffect != NULL )
	{
		pEffect->KeyValue( "start_active", "1" );
		pEffect->KeyValue( "effect_name", "soldier_propeller_wash" );
		pEffect->SetParent( this );
		pEffect->SetLocalOrigin( vec3_origin );
		DispatchSpawn( pEffect );
		pEffect->Activate();
	}

	m_hRotorWash.Set( pEffect );
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnPropeller::StopRotorWash()
{
	if( !m_hRotorWash )
		return;

	UTIL_Remove( m_hRotorWash );
}

AI_BEGIN_AGENT(CAddOnPropeller)

	DECLARE_CONDITION( COND_HOST_JUMPING )
	DECLARE_CONDITION( COND_HOST_NOT_JUMPING )

	DECLARE_TASK( TASK_START_PROPELLER )
	DECLARE_TASK( TASK_STOP_PROPELLER )
	DECLARE_TASK( TASK_SPIN_PROPELLER )

	DEFINE_SCHEDULE
	(
		SCHED_START_PROPELLER,
		"	Tasks"
		"		TASK_START_PROPELLER		0"
		"		TASK_SPIN_PROPELLER			0"
		"	"
		"	Interrupts"
		"		COND_HOST_NOT_JUMPING"
	)
		
	DEFINE_SCHEDULE
	(
		SCHED_STOP_PROPELLER,
		"	Tasks"
		"		TASK_STOP_PROPELLER			0"
		"		TASK_SPIN_PROPELLER			0"
		"	Interrupts"
		"		COND_HOST_JUMPING"
	)

AI_END_AGENT()

