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
#include "particle_parse.h"
#include "grenade_frag.h"
#include "grenade_ar2.h"
#include "smoke_trail.h"
#include "beam_flags.h"

ConVar mortar_speed("mortar_speed", "100" );


#define MORTAR_MODEL "models/addons/mortar.mdl"
#define SHELL_GRAVITY 0.5f

class CDrunkenMortar : public CGrenadeAR2
{
public:
	DECLARE_CLASS(CDrunkenMortar,CGrenadeAR2);

	void Precache(void)
	{
		BaseClass::Precache();
		PrecacheParticleSystem("hunter_flechette_trail");
		PrecacheParticleSystem("dart_trail");
	}

	void Spawn();
	void DrunkenThink();
	void SetTarget( CBaseEntity *pTarget )	{ m_hTarget.Set(pTarget); }

	float		m_flTimeNextCourseChange;
	EHANDLE		m_hTarget;
	int			m_iCourseChange;
	bool		m_bFalling;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(drunken_mortar,CDrunkenMortar);

BEGIN_DATADESC(CDrunkenMortar)
	DEFINE_FIELD( m_flTimeNextCourseChange, FIELD_TIME ),
	DEFINE_FIELD( m_hTarget, FIELD_EHANDLE ),
	DEFINE_FIELD( m_iCourseChange, FIELD_INTEGER ),
	DEFINE_FIELD( m_bFalling, FIELD_BOOLEAN ),
	DEFINE_THINKFUNC(DrunkenThink),
END_DATADESC()

void CDrunkenMortar::Spawn()
{
	BaseClass::Spawn();
	SetThink( &CDrunkenMortar::DrunkenThink );
	SetNextThink( gpGlobals->curtime + 0.1f );

	if( m_hSmokeTrail )
	{
		UTIL_Remove( m_hSmokeTrail );
		m_hSmokeTrail.Set( NULL );
	}

	DispatchParticleEffect( "hunter_flechette_trail", PATTACH_ABSORIGIN_FOLLOW, this );

	SetGravity( SHELL_GRAVITY );

	m_flTimeNextCourseChange = gpGlobals->curtime + RandomFloat( 0, 1 );
}

#define MORTAR_SPEED	600
void CDrunkenMortar::DrunkenThink()
{
	SetNextThink( gpGlobals->curtime + 0.1f );

	if( GetAbsVelocity().z > 0 )
	{
		// Only guided as we fall!
		return;
	}
	else
	{
		if( !m_bFalling )
		{
			EmitSound( "Addon_Mortar.Fall" );
			DispatchParticleEffect( "hunter_flechette_trail", PATTACH_ABSORIGIN_FOLLOW, this );
		}

		m_bFalling = true;
		SetGravity( RandomFloat( 0.75f, 2.0f) );
	}

/*
	Vector vecNewVelocity;

	if( gpGlobals->curtime > m_flTimeNextCourseChange && m_hTarget != NULL )
	{
		if( m_hTarget->GetAbsOrigin().z > GetAbsOrigin().z )
		{
			// Do not change course to chase an enemy who has somehow gotten above me.
			return;
		}

		m_flTimeNextCourseChange = gpGlobals->curtime + RandomFloat( 0.3f, 0.5f );

		if( m_iCourseChange % 2 == 0 )
		{
			Vector vecDir = m_hTarget->WorldSpaceCenter() - GetAbsOrigin();
			VectorNormalize( vecDir );
			vecNewVelocity = vecDir * MORTAR_SPEED;
		}
		else
		{
			Vector vecDir = m_hTarget->WorldSpaceCenter() - GetAbsOrigin();

			vecDir.x *= RandomFloat( -0.95f, 0.95f );
			vecDir.y *= RandomFloat( -0.95f, 0.95f );

			VectorNormalize( vecDir );
			vecNewVelocity = vecDir * MORTAR_SPEED;
		}

		SetAbsVelocity( vecNewVelocity );
		m_iCourseChange++;
	}
*/
}

//---------------------------------------------------------
//---------------------------------------------------------
class CAddOnMortar : public CAI_AddOn
{
public:
	DECLARE_CLASS( CAddOnMortar, CAI_AddOn );
	virtual char *GetAddOnModelName() { return MORTAR_MODEL; }

	//---------------------------------
	// Spawn, etc
	//---------------------------------
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
	virtual bool Install( CAI_BaseNPC *pHost, bool bRemoveOnFail )
	{
		if( BaseClass::Install( pHost, bRemoveOnFail ) )
		{
			pHost->CapabilitiesRemove( bits_CAP_INNATE_MELEE_ATTACK1 );
			return true;
		}

		return false;
	}

	virtual void PickAttachment( CAI_BaseNPC *pHost, char *pchAttachment )
	{
		char szAttachment[ 256 ];

		Q_strcpy( szAttachment, "addon_front" );
		if( IsAddOnAttachmentAvailable(pHost, szAttachment) )
		{
			Q_strcpy( pchAttachment, szAttachment );
		}
	}

	virtual Vector GetAttachOffset( QAngle &attachmentAngles );

	//---------------------------------
	// Functions specific to this addon
	//---------------------------------
	Vector GetMuzzlePos()	{ return GetAbsOrigin() + Vector( 0, 0, 16 ); }

	//-------------------------------
	// Fields specific to this addon
	//-------------------------------
	int m_iSpriteTexture;

	//-------------------------------
	// Schedules/Tasks/Conditions
	//-------------------------------
	enum 
	{
		SCHED_MORTAR_IDLE = NEXT_SCHEDULE,
		SCHED_MORTAR_ATTEMPT_ATTACK,
	};

	enum 
	{
		TASK_MORTAR_SHOOT = NEXT_TASK,
	};

	enum 
	{
		COND_MORTAR_NONE = NEXT_CONDITION,
	};


	DECLARE_DATADESC();
	DEFINE_AGENT();
};

//---------------------------------------------------------
//---------------------------------------------------------
class CAI_MortarAddOnBehavior : public CAI_AddOnBehavior<CAddOnMortar>
{
	DECLARE_CLASS( CAI_MortarAddOnBehavior, CAI_SimpleBehavior );

public:
	DECLARE_DATADESC();

	virtual const char *GetName() {	return "mortar"; }
	virtual bool CanSelectSchedule() { return false; }
};

LINK_ENTITY_TO_ADDON_AND_BEHAVIOR( ai_addon_mortar, CAddOnMortar, CAI_MortarAddOnBehavior );

BEGIN_DATADESC(CAddOnMortar)
END_DATADESC()

BEGIN_DATADESC(CAI_MortarAddOnBehavior)
END_DATADESC()


//---------------------------------------------------------
//---------------------------------------------------------
int CAddOnMortar::SelectSchedule()
{
	if( GetHostEnemy() != NULL )
	{
		return SCHED_MORTAR_ATTEMPT_ATTACK;
	}

	return SCHED_MORTAR_IDLE;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnMortar::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_MORTAR_SHOOT:
		break;

	default:
		CAI_AddOn::StartTask( pTask );
		break;
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnMortar::RunTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_MORTAR_SHOOT:
		{
			if( !GetHostEnemy() )
			{
				TaskFail("Host has no enemy");
				break;
			}

			const int VOLLEY_SIZE = 3;
			Vector vecTrajectory = VecCheckThrow( GetNPCHost(), GetMuzzlePos(), GetHostEnemy()->WorldSpaceCenter(), mortar_speed.GetFloat(), SHELL_GRAVITY );

			if( vecTrajectory != vec3_origin )
			{
				AngularImpulse angVelocity;
				QAngle angles;

				VectorAngles( vecTrajectory, angles );

				CBroadcastRecipientFilter filter2;
				te->BeamRingPoint( filter2, 0, GetAbsOrigin(),	//origin
					8.0f,	//start radius
					64.0f,		//end radius
					m_iSpriteTexture, //texture
					0,			//halo index
					0,			//start frame
					2,			//framerate
					0.5f,		//life
					32,			//width
					0,			//spread
					0,			//amplitude
					255,	//r
					255,	//g
					225,	//b
					200,	//a
					0,		//speed
					FBEAM_FADEOUT
					);

				EmitSound( "Addon_Mortar.Fire" );

				for( int i = 0 ; i < VOLLEY_SIZE ; i++ )
				{
					CDrunkenMortar *pShell = (CDrunkenMortar*)Create( "drunken_mortar", GetMuzzlePos() + Vector( 0, 0, i*6.0f), angles, GetNPCHost() );
					Vector vecSillyTrajectory = vecTrajectory;
					vecSillyTrajectory.x += RandomFloat( -35.f, 35.0f );
					vecSillyTrajectory.y += RandomFloat( -35.f, 35.0f );
					vecSillyTrajectory.z += RandomFloat( -40.f, 0.0f );
					pShell->SetAbsVelocity( vecSillyTrajectory );
					pShell->SetLocalAngularVelocity( RandomAngle( -400, 400 ) );
					pShell->SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE ); 
					pShell->SetThrower( GetNPCHost() );
					pShell->SetDamage( 20 );
					pShell->SetGravity( SHELL_GRAVITY );
					pShell->SetTarget( GetHostEnemy() );
				}

				TaskComplete();
			}
		}
		break;

	default:
		CAI_AddOn::RunTask( pTask );
		break;
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnMortar::Precache()
{
	PrecacheScriptSound( "Addon_Mortar.Fire" );
	PrecacheScriptSound( "Addon_Mortar.Fall" );

	m_iSpriteTexture = PrecacheModel( "sprites/physbeam.vmt" );
	BaseClass::Precache();
}

//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnMortar::GatherConditions()
{
	BaseClass::GatherConditions();

	// What if we're aren't attached to a host?
	if( GetNPCHost() )
	{

	}
}

//---------------------------------------------------------
//---------------------------------------------------------
Vector CAddOnMortar::GetAttachOffset( QAngle &attachmentAngles )
{
	Assert( GetNPCHost() );
	return BaseClass::GetAttachOffset( attachmentAngles );

	Vector vecUp;
	AngleVectors( attachmentAngles, NULL, NULL, &vecUp );
	return vecUp * 3.0f;
}

AI_BEGIN_AGENT(CAddOnMortar)

	//DECLARE_CONDITION( COND_HOST_JUMPING )

	DECLARE_TASK( TASK_MORTAR_SHOOT )

	DEFINE_SCHEDULE
	(
		SCHED_MORTAR_IDLE,
		"	Tasks"
		"		TASK_ADDON_WAIT			1"
		"	"
		"	Interrupts"
	)
		
	DEFINE_SCHEDULE
	(
		SCHED_MORTAR_ATTEMPT_ATTACK,
		"	Tasks"
		"		TASK_MORTAR_SHOOT			0"
		"		TASK_ADDON_WAIT				15"
		"	Interrupts"
	)

AI_END_AGENT()

