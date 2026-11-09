//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//====================================================================================//

#ifndef ADDON_BASESHOOTER_H
#define ADDON_BASESHOOTER_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "ai_behavior.h"
#include "ai_addon.h"


extern int g_interactionAddOnShoot;
extern int g_interactionAddOnBeginShooting;

//=========================================================
// Most 'gun' addons will derive from this addon so that 
// they have a relationship with the AddOnShooterBehavior
//=========================================================
class CAI_AddOnBaseShooter:public CAI_AddOn
{
	DECLARE_CLASS( CAI_AddOnBaseShooter, CAI_AddOn );

public:
	//------------------------------------
	//------------------------------------
	virtual void GatherConditions();

	virtual bool Install( CAI_BaseNPC *pHost, bool bRemoveOnFail = true );
	virtual void Remove();

	//---------------------------------
	// Appearance, position
	//---------------------------------
	virtual void PickAttachment( CAI_BaseNPC *pHost, char *pchAttachment )
	{
		char szAttachment[ 256 ];

		Q_strcpy( szAttachment, "addon_baseshooter" );
		if( IsAddOnAttachmentAvailable(pHost, szAttachment) )
		{
			Q_strcpy( pchAttachment, szAttachment );
		}
	}

	//------------------------------------
	// Pure virtual parameters
	//------------------------------------
	virtual float	GetMinRange() = 0;
	virtual float	GetMaxRange() = 0;
	virtual float	GetMaxDeflection() = 0;
	virtual float	GetRateOfFire() = 0;		// Expressed as rounds-per-second
	virtual float	GetRestInterval() = 0;

	//------------------------------------
	//------------------------------------
	virtual Vector	GetMuzzlePos() = 0;

	//------------------------------------
	// Evaluators
	//------------------------------------
	virtual bool	IsTargetTooFar( CBaseEntity *pTarget );
	virtual bool	IsTargetTooClose( CBaseEntity *pTarget );
	virtual bool	IsTargetLocationOccluded( const Vector &vecTarget );
	virtual bool	IsTargetOccluded( CBaseEntity *pTarget );
	virtual bool	IsPointedAtTarget( CBaseEntity *pTarget, bool bCheck2DOnly = false );
	virtual bool    CouldPointAtTarget( Vector vecLocation, Vector vecTarget );

	//------------------------------------
	// Shooting
	//------------------------------------
	bool IsShooting() { return m_nBurstShotsRemaining > 0; }
	virtual void	StartShootingBurst( bool bPreDelay = false );
	virtual void	AdvanceBurst();
	virtual void	ShootAt( CBaseEntity *pTarget )	{ Msg("POW!\n"); }
	virtual void	StopShooting() { m_nBurstShotsRemaining = 0; }

	//------------------------------------
	// Timing
	//------------------------------------
	void	RestForSeconds( float flSeconds )	{ m_flTimeDoneResting = gpGlobals->curtime + flSeconds; }
	bool	IsInRestInterval()	{ return m_flTimeDoneResting > gpGlobals->curtime; }

public:
	float	m_flTimeDoneResting;		// If in the future, the shooter is in a rest interval and may not shoot.
	int		m_nBurstShotsRemaining;		// The number of shots remaining in the current burst. (queued-up shots, basically).
	float	m_flTimeNextShot;			// The time at which we should fire our next shot.

	DECLARE_DATADESC();
};

//=========================================================
// The behavior associated with most 'gun' addons will 
// derive from this behavior.
//=========================================================
class CAI_AddOnShooterBehavior : public CAI_AddOnBehavior<CAI_AddOnBaseShooter>
{
	DECLARE_CLASS( CAI_AddOnShooterBehavior, CAI_SimpleBehavior );

public:
	CAI_AddOnShooterBehavior();
	~CAI_AddOnShooterBehavior();

	DECLARE_DATADESC();

	virtual const char *GetName() {	return "AddonBaseShooter"; }
	virtual void BuildScheduleTestBits();
	virtual void GatherConditions();
	virtual void GatherConditionsNotActive();
	void GatherAllConditions();

	virtual int SelectSchedule( int channel );
	virtual int SelectSchedule();
	virtual void StartTask( const Task_t *pTask );
	virtual void StartTask( int channel, const Task_t *pTask );
	virtual void RunTask( int channel, const Task_t *pTask );

	virtual bool WeaponLOSCondition(const Vector &ownerPos, const Vector &targetPos, bool bSetConditions );

	CAI_AddOnBaseShooter *AccessShooterAddOnByIndex( int index );
	void AllShootersStartShooting();
	void AllShootersShootAt( CBaseEntity *pTarget );
	void AllShootersStopShooting();
	void AllShootersRest();
	bool IsAnyShooterShooting();

	//-----------------------------------------------------
	// Schedules
	//-----------------------------------------------------
	enum
	{
		SCHED_SHOOTER_NPC_AIM_AT_ENEMY = BaseClass::NEXT_SCHEDULE,
		SCHED_SHOOTER_NPC_SHOOT_ADVANCING,
		SCHED_SHOOTER_NPC_SHOOT_STRAFING,
		SCHED_SHOOTER_IDLE,
		SCHED_SHOOTER_SHOOT,
		NEXT_SCHEDULE,
	};

	//-----------------------------------------------------
	// Conditions
	//-----------------------------------------------------
	enum
	{
		COND_SHOOTER_ENEMY_OCCLUDED = BaseClass::NEXT_CONDITION,		// Barrel of the shooter add-on cannot 'see' the enemy.
		COND_SHOOTER_POINTED_AT_ENEMY,									// Enemy is within valid deflection range of the shooter add-on.
		COND_SHOOTER_POINTING_IMPOSSIBLE,								// Enemy is within deflection range in YAW, but not pitch.
		COND_SHOOTER_NOT_POINTED_AT_ENEMY,								// Enemy is NOT ''      ''
		COND_SHOOTER_ENEMY_TOO_CLOSE,									// Enemy is nearer than MinRange
		COND_SHOOTER_ENEMY_TOO_FAR,										// Enemy is farther than MaxRange
		COND_SHOOTER_RESTING,											// Shooter is in a rest interval.
		COND_SHOOTER_CAN_SHOOT,											// We're on!!!
		COND_SHOOTER_OUTER_RANGE,										// My dist to enemy is > MaxRange / 2
		NEXT_CONDITION,
	};

	//-----------------------------------------------------
	// Tasks
	//-----------------------------------------------------
	enum
	{
		TASK_SHOOTER_SHOOT = BaseClass::NEXT_TASK,
		TASK_SHOOTER_REST, // Pause for rest interval between groups of shots
		TASK_SHOOTER_IDLE, // Blocking wait, expects a condition to interrupt any schedule containing this task
		TASK_SHOOTER_GET_ADVANCING_PATH,
		TASK_SHOOTER_GET_STRAFING_PATH,
		NEXT_TASK,
	};

public:
	CAI_AddOnBaseShooter *AccessShooterAddOn() { return ( m_AddOns.Count() ) ? m_AddOns[0] : NULL; }

public:
	DEFINE_CUSTOM_SCHEDULE_PROVIDER;

};

#endif ADDON_BASESHOOTER_H
