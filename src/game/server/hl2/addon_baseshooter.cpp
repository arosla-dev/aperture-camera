//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//====================================================================================//

#include "cbase.h"
#include "addon_baseshooter.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar shooter_advance_dist( "shooter_advance_dist", "240" );
ConVar shooter_max_simultaneous( "shooter_max_simultaneous", "3" );

BEGIN_DATADESC( CAI_AddOnBaseShooter )
	DEFINE_FIELD( m_flTimeDoneResting, FIELD_TIME ),
	DEFINE_FIELD( m_nBurstShotsRemaining, FIELD_INTEGER ),
	DEFINE_FIELD( m_flTimeNextShot, FIELD_TIME ),
END_DATADESC()

BEGIN_DATADESC( CAI_AddOnShooterBehavior )
END_DATADESC()

//---------------------------------------------------------
// Interactions
//---------------------------------------------------------
int g_interactionAddOnShoot;			// Fired a single shot
int g_interactionAddOnBeginShooting;	// Going from not shooting state to shooting state

//---------------------------------------------------------
//---------------------------------------------------------
bool CAI_AddOnBaseShooter::IsTargetTooFar( CBaseEntity *pTarget )
{
	if( pTarget->GetAbsOrigin().DistTo( GetAbsOrigin() ) > GetMaxRange() )
		return true;

	return false;
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAI_AddOnBaseShooter::IsTargetTooClose( CBaseEntity *pTarget )
{
	if( pTarget->GetAbsOrigin().DistTo( GetAbsOrigin() ) < GetMinRange() )
		return true;

	return false;
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAI_AddOnBaseShooter::IsTargetOccluded( CBaseEntity *pTarget )
{
	return IsTargetLocationOccluded( pTarget->EyePosition() );
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAI_AddOnBaseShooter::IsTargetLocationOccluded( const Vector &vecTarget )
{
	return !FVisible( vecTarget );
}

//---------------------------------------------------------
// Speculate about whether we could point at the enemy from
// this location
//---------------------------------------------------------
bool CAI_AddOnBaseShooter::CouldPointAtTarget( Vector vecLocation, Vector vecTarget )
{
	Vector vecToTarget, vecToTarget2D;

	vecToTarget = vecTarget - vecLocation;
	vecToTarget2D = vecToTarget;
	vecToTarget2D.z = 0.0f;

	VectorNormalize( vecToTarget );
	VectorNormalize( vecToTarget2D );

	float flDot = DotProduct( vecToTarget, vecToTarget2D );

	if( flDot >= GetMaxDeflection() )
	{
		//NDebugOverlay::Text( vecLocation, CFmtStr( "DOT: %f", flDot ), true, 1.0f );
		return true;
	}

	//NDebugOverlay::Text( vecLocation, CFmtStr( "fail DOT: %f", flDot ), true, 1.0f );
	return false;
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAI_AddOnBaseShooter::IsPointedAtTarget( CBaseEntity *pTarget, bool bCheck2DOnly )
{
	//Vector vecBoreSight = GetMuzzlePos() - GetAbsOrigin();

	// Use the Host NPC's angles for the guns' boresights. This filters out 
	// the noise caused by the side-to-side swinging motion in the host's walk.
	QAngle angles = GetNPCHost()->GetAbsAngles();
	Vector vecBoreSight;
	AngleVectors( angles, &vecBoreSight );


	Vector vecToLocation = pTarget->WorldSpaceCenter() - GetAbsOrigin();

	if( bCheck2DOnly )
	{
		vecBoreSight.z = 0;
		vecToLocation.z = 0;
	}

	VectorNormalize( vecBoreSight );
	VectorNormalize( vecToLocation );

	float flDot = DotProduct( vecBoreSight, vecToLocation );

	if( flDot >= GetMaxDeflection() )
		return true;

#if 1
	// If I'm flying, and pointed at you in 2D, then I'm pointed at you good enough to fire at you.
	if ( !bCheck2DOnly && GetNPCHost()->GetNavigator()->GetNavType() == NAV_JUMP )
	{
		return IsPointedAtTarget( pTarget, true );
	}
#endif 

	return false;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_AddOnBaseShooter::StartShootingBurst( bool bPreDelay )
{
	GetNPCHost()->HandleInteraction( g_interactionAddOnBeginShooting, this, NULL );

	if( bPreDelay )
	{
		// This stops dual addons from firing at the exact same time.
		m_flTimeNextShot = gpGlobals->curtime + RandomFloat( 0.0f, 0.2f );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_AddOnBaseShooter::GatherConditions()
{
	BaseClass::GatherConditions();
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAI_AddOnBaseShooter::Install( CAI_BaseNPC *pHost, bool bRemoveOnFail )
{
	bool success = BaseClass::Install( pHost, bRemoveOnFail );

	if( success )
	{
		pHost->CapabilitiesRemove( bits_CAP_INNATE_MELEE_ATTACK1 );
		pHost->CapabilitiesAdd( bits_CAP_INNATE_RANGE_ATTACK1 );
	}

	return success;
}

void CAI_AddOnBaseShooter::Remove()
{
	CAI_BaseNPC *pHost = GetNPCHost();

	if ( pHost )
	{
		pHost->CapabilitiesRemove( bits_CAP_INNATE_RANGE_ATTACK1 );
		pHost->CapabilitiesAdd( bits_CAP_INNATE_MELEE_ATTACK1 );
	}

	BaseClass::Remove();
}

//---------------------------------------------------------
// Count a shot fired and queue up the time of the next shot
//---------------------------------------------------------
void CAI_AddOnBaseShooter::AdvanceBurst()
{
	m_nBurstShotsRemaining--;
	m_flTimeNextShot = gpGlobals->curtime + (1.0f / GetRateOfFire() );
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_AddOnShooterBehavior::BuildScheduleTestBits()
{
	BaseClass::BuildScheduleTestBits();

	if( IsCurSchedule( SCHED_ESTABLISH_LINE_OF_FIRE_FALLBACK, false ) )
	{
		GetOuter()->SetCustomInterruptCondition( GetClassScheduleIdSpace()->ConditionLocalToGlobal( COND_SHOOTER_CAN_SHOOT ) );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
static CUtlVector<CAI_AddOnShooterBehavior *> g_AddOnShooterBehaviors;

CAI_AddOnShooterBehavior::CAI_AddOnShooterBehavior()
{
	g_AddOnShooterBehaviors.AddToTail( this );
}

//---------------------------------------------------------
//---------------------------------------------------------
CAI_AddOnShooterBehavior::~CAI_AddOnShooterBehavior()
{
	g_AddOnShooterBehaviors.FindAndRemove( this );
}

//---------------------------------------------------------
//---------------------------------------------------------
CAI_AddOnBaseShooter *CAI_AddOnShooterBehavior::AccessShooterAddOnByIndex( int index )
{
	Assert( index >= 0 && index < m_AddOns.Count() );

	return m_AddOns[ index ];
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_AddOnShooterBehavior::AllShootersStartShooting()
{
	for( int i = 0 ; i < m_AddOns.Count() ; i++ )
	{
		AccessShooterAddOnByIndex( i )->StartShootingBurst( (i > 0) );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_AddOnShooterBehavior::AllShootersShootAt( CBaseEntity *pTarget )
{
	for( int i = 0 ; i < m_AddOns.Count() ; i++ )
	{
		CAI_AddOnBaseShooter *pShooter = AccessShooterAddOnByIndex( i );

		if( gpGlobals->curtime > pShooter->m_flTimeNextShot )
		{
			pShooter->ShootAt( pTarget );
			pShooter->AdvanceBurst();
		}
	}
}


//---------------------------------------------------------
//---------------------------------------------------------
void CAI_AddOnShooterBehavior::AllShootersStopShooting()
{
	for( int i = 0 ; i < m_AddOns.Count() ; i++ )
	{
		AccessShooterAddOnByIndex( i )->StopShooting();
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_AddOnShooterBehavior::AllShootersRest()
{
	for( int i = 0 ; i < m_AddOns.Count() ; i++ )
	{
		CAI_AddOnBaseShooter *pShooter;

		pShooter = AccessShooterAddOnByIndex( i );
		pShooter->RestForSeconds( pShooter->GetRestInterval() );
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAI_AddOnShooterBehavior::IsAnyShooterShooting()
{
	for( int i = 0 ; i < m_AddOns.Count() ; i++ )
	{
		if( AccessShooterAddOnByIndex( i )->IsShooting() )
			return true;
	}

	return false;
}

void CAI_AddOnShooterBehavior::GatherAllConditions()
{
	CAI_AddOnBaseShooter *pShooterAddOn = AccessShooterAddOn();
	Assert( pShooterAddOn != NULL );

	ClearCondition( COND_SHOOTER_ENEMY_OCCLUDED );
	ClearCondition( COND_SHOOTER_NOT_POINTED_AT_ENEMY );
	ClearCondition( COND_SHOOTER_POINTED_AT_ENEMY );
	ClearCondition( COND_SHOOTER_ENEMY_TOO_FAR );
	ClearCondition( COND_SHOOTER_ENEMY_TOO_CLOSE );
	ClearCondition( COND_SHOOTER_RESTING );
	ClearCondition( COND_SHOOTER_CAN_SHOOT );
	ClearCondition( COND_SHOOTER_POINTING_IMPOSSIBLE );
	ClearCondition( COND_SHOOTER_OUTER_RANGE );

	CBaseEntity *pEnemy = GetEnemy();
	if( pEnemy != NULL )
	{
		if( pShooterAddOn->IsPointedAtTarget( pEnemy ) )
		{
			SetCondition( COND_SHOOTER_POINTED_AT_ENEMY );
		}
		else
		{
			SetCondition( COND_SHOOTER_NOT_POINTED_AT_ENEMY );

			if( pShooterAddOn->IsPointedAtTarget( pEnemy, true ) )
			{
				SetCondition( COND_SHOOTER_POINTING_IMPOSSIBLE );
			}
		}

		if( pShooterAddOn->IsTargetTooClose( pEnemy ) )
			SetCondition( COND_SHOOTER_ENEMY_TOO_CLOSE );

		if( pShooterAddOn->IsTargetTooFar( pEnemy ) )
		{
			SetCondition( COND_SHOOTER_ENEMY_TOO_FAR );
		}
		else
		{
			if( pShooterAddOn->GetAbsOrigin().DistTo( pEnemy->GetAbsOrigin() ) > (pShooterAddOn->GetMaxRange() / 2) )
			{
				SetCondition( COND_SHOOTER_OUTER_RANGE );
			}
		}

		// !!!HACK - Could micro-optimize this so it doesn't trace every single time.
		if( pShooterAddOn->IsTargetOccluded( pEnemy ) )
			SetCondition( COND_SHOOTER_ENEMY_OCCLUDED );
	}

	if( pShooterAddOn->IsInRestInterval() )
	{
		SetCondition( COND_SHOOTER_RESTING );
	}

	if( HasCondition( COND_SHOOTER_POINTED_AT_ENEMY )		&&
		!HasCondition( COND_SHOOTER_ENEMY_TOO_CLOSE )		&&
		!HasCondition( COND_SHOOTER_ENEMY_TOO_FAR )			&&
		!HasCondition( COND_SHOOTER_ENEMY_OCCLUDED )		&&
		!HasCondition( COND_SHOOTER_RESTING) )
	{
		SetCondition( COND_SHOOTER_CAN_SHOOT );
	}
}



//---------------------------------------------------------
//---------------------------------------------------------
void CAI_AddOnShooterBehavior::GatherConditions()
{
	BaseClass::GatherConditions();
	GatherAllConditions();
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_AddOnShooterBehavior::GatherConditionsNotActive()
{
	BaseClass::GatherConditionsNotActive();
	GatherAllConditions();
}

//---------------------------------------------------------
//---------------------------------------------------------
int CAI_AddOnShooterBehavior::SelectSchedule( int channel )
{
	if( HasCondition( COND_SHOOTER_CAN_SHOOT) )
	{
		// @hackhack ---------------------------
		int nAvailableShootSlots = shooter_max_simultaneous.GetInt();

		for ( int i = 0; i < g_AddOnShooterBehaviors.Count() && nAvailableShootSlots; i++ )
		{
			if ( g_AddOnShooterBehaviors[i] != this && g_AddOnShooterBehaviors[i]->AccessShooterAddOn()->IsShooting() )
			{
				nAvailableShootSlots--;
			}
		}
		// @hackhack ---------------------------
		if ( nAvailableShootSlots )
		{
			return SCHED_SHOOTER_SHOOT;
		}
	}

	if( IsAnyShooterShooting() )
	{
		AllShootersStopShooting();
		AllShootersRest();
	}

	return SCHED_SHOOTER_IDLE;
}

//---------------------------------------------------------
//---------------------------------------------------------
int CAI_AddOnShooterBehavior::SelectSchedule()
{
	if( HasCondition( COND_CAN_MELEE_ATTACK1) )
		return SCHED_MELEE_ATTACK1;

	if( HasCondition(COND_SHOOTER_POINTING_IMPOSSIBLE) || HasCondition(COND_SHOOTER_ENEMY_TOO_CLOSE) || HasCondition(COND_SHOOTER_ENEMY_TOO_FAR) || HasCondition(COND_SHOOTER_ENEMY_OCCLUDED) || HasCondition(COND_SHOOTER_NOT_POINTED_AT_ENEMY) )
		return SCHED_ESTABLISH_LINE_OF_FIRE;

	if( HasCondition( COND_SEE_ENEMY ) )
	{
		if( HasCondition( COND_SHOOTER_OUTER_RANGE ) && HasCondition( COND_SHOOTER_POINTED_AT_ENEMY ) && RandomInt(0,1) )
		{
			return SCHED_SHOOTER_NPC_SHOOT_ADVANCING;
		}

		if( !HasCondition( COND_SHOOTER_OUTER_RANGE ) && RandomInt( 0, 2 ) )
		{
			return SCHED_SHOOTER_NPC_SHOOT_STRAFING;
		}

		// Keep aiming the NPC's body at the enemy and this should allow the gun to start firing.
		return SCHED_SHOOTER_NPC_AIM_AT_ENEMY;
	}

	return BaseClass::SelectSchedule();
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_AddOnShooterBehavior::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
		case TASK_SHOOTER_GET_ADVANCING_PATH:
			{
				Vector vecToEnemy = GetEnemy()->GetAbsOrigin() - GetAbsOrigin();
				VectorNormalize( vecToEnemy );
				if (!GetNavigator()->SetGoal( GetAbsOrigin() + vecToEnemy * shooter_advance_dist.GetFloat() ))
				{
					TaskFail(FAIL_NO_ROUTE);
				}
				break;
			}

		case TASK_SHOOTER_GET_STRAFING_PATH:
			{
				Vector vecToEnemy = GetEnemy()->GetAbsOrigin() - GetAbsOrigin();
				Vector vecRight;

				QAngle angles;
				VectorAngles( vecToEnemy, angles );
				AngleVectors( angles, NULL, &vecRight, NULL );

				if( RandomInt( 0,1 ) )
				{
					vecRight *= -1;
				}

				if (!GetNavigator()->SetGoal( GetAbsOrigin() + vecRight * shooter_advance_dist.GetFloat() ))
				{
					TaskFail(FAIL_NO_ROUTE);
				}
				break;
			}

		default:
			return BaseClass::StartTask( pTask );
			break;
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_AddOnShooterBehavior::StartTask( int channel, const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_SHOOTER_REST:
		AllShootersRest();
		break;

	case TASK_SHOOTER_IDLE:
		// I am a blocking task
		break;

	case TASK_SHOOTER_SHOOT:
		AllShootersStartShooting();
		break;

	default:
		break;
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAI_AddOnShooterBehavior::RunTask( int channel, const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_SHOOTER_REST:
		if( !AccessShooterAddOn()->IsInRestInterval() )
			TaskComplete( channel );
		break;

	case TASK_SHOOTER_IDLE:
		// I am a blocking task
		break;

	case TASK_SHOOTER_SHOOT:
		if( GetEnemy() != NULL && IsAnyShooterShooting() )
		{
			AllShootersShootAt( GetEnemy() );
		}
		else
		{
			// We have fired all the shots!
			TaskComplete( channel );
		}
		break;

	default:
		break;
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAI_AddOnShooterBehavior::WeaponLOSCondition(const Vector &ownerPos, const Vector &targetPos, bool bSetConditions )
{
	if( ownerPos.DistToSqr(targetPos) < Square( AccessShooterAddOn()->GetMinRange() ) || ownerPos.DistToSqr(targetPos) > Square( AccessShooterAddOn()->GetMaxRange() ) )
	{
		// Don't accept any spots that are too close to enemy or too far from enemy.
		return false;
	}

	if( !AccessShooterAddOn()->CouldPointAtTarget( ownerPos, targetPos ) )
	{
		// If we stood on this spot, we wouldnt' be able to point the addon at the enemy.
		return false;
	}

	if( AccessShooterAddOn()->IsTargetLocationOccluded( targetPos ) )
	{
		return false;
	}

	// Important, now route the WeaponLOSCondition call through InnateWeaponLOSCondition so we don't get kicked out for having no Active Weapon.
	return GetOuter()->InnateWeaponLOSCondition( ownerPos, targetPos, bSetConditions );
}

AI_BEGIN_CUSTOM_SCHEDULE_PROVIDER( CAI_AddOnShooterBehavior )

	DECLARE_CONDITION( COND_SHOOTER_ENEMY_OCCLUDED )
	DECLARE_CONDITION( COND_SHOOTER_NOT_POINTED_AT_ENEMY )
	DECLARE_CONDITION( COND_SHOOTER_POINTED_AT_ENEMY )
	DECLARE_CONDITION( COND_SHOOTER_POINTING_IMPOSSIBLE )
	DECLARE_CONDITION( COND_SHOOTER_ENEMY_TOO_FAR )
	DECLARE_CONDITION( COND_SHOOTER_ENEMY_TOO_CLOSE )
	DECLARE_CONDITION( COND_SHOOTER_OUTER_RANGE )
	DECLARE_CONDITION( COND_SHOOTER_RESTING )
	DECLARE_CONDITION( COND_SHOOTER_CAN_SHOOT )

	DECLARE_TASK( TASK_SHOOTER_SHOOT )
	DECLARE_TASK( TASK_SHOOTER_REST )
	DECLARE_TASK( TASK_SHOOTER_IDLE )
	DECLARE_TASK( TASK_SHOOTER_GET_ADVANCING_PATH )
	DECLARE_TASK( TASK_SHOOTER_GET_STRAFING_PATH )

	DECLARE_INTERACTION( g_interactionAddOnShoot )
	DECLARE_INTERACTION( g_interactionAddOnBeginShooting )

	DEFINE_SCHEDULE
	( 
		SCHED_SHOOTER_NPC_AIM_AT_ENEMY,

		"	Tasks"
		"		TASK_STOP_MOVING					0"
		"		TASK_WAIT_FACE_ENEMY				5"
		"		TASK_WAIT_RANDOM					1"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_SHOOTER_ENEMY_OCCLUDED"
		"		COND_SHOOTER_ENEMY_TOO_FAR"
		"		COND_SHOOTER_ENEMY_TOO_CLOSE"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_SHOOTER_POINTING_IMPOSSIBLE"
	)

	DEFINE_SCHEDULE
	(
		SCHED_SHOOTER_IDLE,

		"	Tasks"
		"		TASK_SHOOTER_IDLE		0"	// Sit and do nothing until interrupted.
		"	Interrupts"
		"		COND_SHOOTER_CAN_SHOOT"
	)

	DEFINE_SCHEDULE
	(
		SCHED_SHOOTER_NPC_SHOOT_ADVANCING,

		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_SHOOTER_NPC_AIM_AT_ENEMY"
		"		TASK_SHOOTER_GET_ADVANCING_PATH		0"
		"		TASK_WALK_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_STOP_MOVING					0"
		"		TASK_WAIT_FACE_ENEMY				1"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_SHOOTER_ENEMY_OCCLUDED"
		"		COND_SHOOTER_ENEMY_TOO_FAR"
		"		COND_SHOOTER_ENEMY_TOO_CLOSE"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_SHOOTER_NOT_POINTED_AT_ENEMY"
		"		COND_SHOOTER_POINTING_IMPOSSIBLE"
	)

	DEFINE_SCHEDULE
	(
	SCHED_SHOOTER_NPC_SHOOT_STRAFING,

	"	Tasks"
	"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_SHOOTER_NPC_AIM_AT_ENEMY"
	"		TASK_SHOOTER_GET_STRAFING_PATH		0"
	"		TASK_RUN_PATH						0"
	"		TASK_WAIT_FOR_MOVEMENT				0"
	"		TASK_STOP_MOVING					0"
	"		TASK_WAIT_FACE_ENEMY				1"
	""
	"	Interrupts"
	"		COND_NEW_ENEMY"
	"		COND_SHOOTER_ENEMY_OCCLUDED"
	"		COND_SHOOTER_ENEMY_TOO_FAR"
	"		COND_SHOOTER_ENEMY_TOO_CLOSE"
	"		COND_CAN_MELEE_ATTACK1"
	"		COND_SHOOTER_NOT_POINTED_AT_ENEMY"
	"		COND_SHOOTER_POINTING_IMPOSSIBLE"
	)

	DEFINE_SCHEDULE
	(
		SCHED_SHOOTER_SHOOT,

		"	Tasks"
		"		TASK_SHOOTER_SHOOT		0"	
		"		TASK_SHOOTER_REST		0"
		"	Interrupts"
		"		COND_SHOOTER_NOT_POINTED_AT_ENEMY"
		"		COND_SHOOTER_ENEMY_TOO_FAR"
		"		COND_SHOOTER_ENEMY_TOO_CLOSE"
		"		COND_SHOOTER_ENEMY_OCCLUDED"
		"		COND_ENEMY_WENT_NULL"
	)

 AI_END_CUSTOM_SCHEDULE_PROVIDER()
