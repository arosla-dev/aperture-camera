//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef NPC_BLOB_PARTICLE_H
#define NPC_BLOB_PARTICLE_H

#if defined( _WIN32 )
#pragma once
#endif

#include "../common/blobulator/physics/physparticle.h"
#include "../common/blobulator/smartarray.h"

enum BlobParticleType
{
	// NOTE: The order here matters for transformation purposes.
	ROOT_PARTICLE = 0,
	CORE_PARTICLE,
	STICKY_PARTICLE,
	SKIN_PARTICLE,
	TENTACLE_PARTICLE,
	UNDEFINED_PARTICLE,

	NUM_PARTICLE_TYPES,
};

class CBlobParticle;

struct BlobNeighbor_t
{
	float dist;
	CBlobParticle* particle;
	Vector vec;
};

class IPhysicsObject;
class CBlobGroup;

class CBlobParticle
{

public:
	CBlobParticle();

	BlobParticleType m_type;
	CBlobGroup* m_group;
	PhysParticle m_physParticle;
	IPhysicsObject* m_vphysParticle; // pointer to the particle in the physics engine
	CBlobParticle* m_parent;
	CBlobParticle* m_pgoal;
	//CBlobParticle* m_treeParent;
	//CBlobParticle* m_oldTreeParent;

	Vector m_position;
	Vector m_velocity;
	Vector m_force;
	Vector m_vecMoveGoal;
	bool m_bMovingTowardFinalGoal;

	float m_mass;

	// These four probably need to be removed at some point
	int m_targetSlot;
	int m_ownedSlot;
	int m_nArm;
	int m_nParent;

	Vector m_surface;
	float m_support; // a value from 0 to 1 indicating how much support the particle has from neighbors.
	float m_contact;
	float m_fLastContactCountdown;
	float m_fLastNonStraggler;
	Vector m_smoothedVelocity; // Used to find 'straggler' particles (stuck, not moving towards their goal)

	void *m_temp; // we do a lot of neighbor processing. Use this to store temp data without knowing indices

	//CUtlVector<BlobNeighbor_t> m_neighbors;
	SmartArray<BlobNeighbor_t> m_neighbors;
	CUtlVector<Vector> m_surfaces;
	CUtlVector<float> m_intercepts;
	CUtlVector<float> m_lastContactTime;
	int m_nWaypointTargetIndex;
	Vector m_vecPrevWaypointPosition;

	float m_flDeathTime; // Used for dying particles (BLOB_PARTICLE_DEATH)
	float m_flBirthTime; // Used for regenerating particles (BLOB_PARTICLE_BIRTH)

	bool m_isInContact;
	unsigned int m_flags;

	bool isSticky() { return m_type == STICKY_PARTICLE; }
};

class CBlobGroup
{

public:
	CBlobGroup();
	~CBlobGroup() { };

	CBlobParticle* m_root;

	bool m_bIsTentacle;

	CUtlVector<CBlobParticle*> m_members;
	CUtlVector<CBlobParticle*> m_particlesByType[NUM_PARTICLE_TYPES]; // TODO (Ilya): Do constructors and destructors get called here?
	CUtlVector<Vector> m_waypoints;
};

enum BlobTentacleState
{
	BLOB_TENTACLE_STATE_NONE = 0,

	BLOB_TENTACLE_STATE_EXTRUDE,
	BLOB_TENTACLE_STATE_IDLE,
	BLOB_TENTACLE_STATE_LUNGE,
	BLOB_TENTACLE_STATE_RECEDE,
	BLOB_TENTACLE_STATE_HAUL,
	BLOB_TENTACLE_STATE_SWIPE,
	BLOB_TENTACLE_STATE_SPIT_EXTRUDE,
	BLOB_TENTACLE_STATE_SPIT_HOLD,
};

enum BlobTentacleAction
{
	BLOB_TENTACLE_ACTION_NONE = 0,

	BLOB_TENTACLE_ACTION_ATTACK_FRONT,
	BLOB_TENTACLE_ACTION_ATTACK_LEFT,
	BLOB_TENTACLE_ACTION_ATTACK_RIGHT,
	BLOB_TENTACLE_ACTION_SWIPE,
	BLOB_TENTACLE_ACTION_SPIT,
};

class CBlobTentacle
{
	DECLARE_DATADESC();

public:
	CBlobTentacle();
	~CBlobTentacle() { };

	BlobTentacleAction	m_nAction;			// The action this tentacle is performing
	EHANDLE				m_hAnimationEnt;	// The entity from which this tentacle's joint animations are sampled

	int					m_nPhase;			// Current state: BLOB_TENTACLE_STATE_NONE (inactive), TASK_BLOB_TENTACLE_[EXTRUDE|WAIT|LUNGE|RECEDE|HAUL]
	float				m_fPhaseStart;		// When the current phase started
	float				m_fPhaseDuration;	// How long the current phase should last
	bool				m_bPhaseIncomplete; // Override to say this phase is incomplete (even if m_fDuration seconds have elapsed)
	bool				m_bSwipeLeftToRight;
	bool				m_bHasSwooshed;		// Whether it's emitted a swoosh sound during the lunge phase
	float				m_fSpeedMultiplier;	// A multiplier for this tentacle's timing, relative to the default values (2.0 means it moves twice as fast)

	CBlobParticle		*m_pRoot;			// The particle which serves as the root of the tentacle
	Vector				m_vecDirection;		// The direction of the tentacle, away from the base
	Vector				m_vecTarget;		// The location of the tentacle's target (e.g. the player), relative to the tentacle's base. May be deliberately out-of-date.
	Vector				m_vecLastValidSpitPos;		// The last valid location of the tentacle for spit purposes
	Vector				m_vecEyePosition;	// Where should the eye position come from?
	float				m_fLength;			// How long is this tentacle?
	float				m_fLastLength;		// How long is this tentacle?
	Vector				m_vecLastDirection;	// 

	EHANDLE				m_hTargetEnt;		// The entity this tentacle is targeting
	Vector				m_vecTargetOffset;	// Offset from the target entity where the tentacle should strike

	CUtlVector< EHANDLE > m_SwipeHitTargets;
	CBlobGroup *		m_blobGroup;		// The group dedicated to this tentacle
};

#endif
