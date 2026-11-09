//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

// TODO: npc_demomonster is just here for backwards compatibility with levels that use it.
// It has been replaced with npc_blob, and should be deleted ASAP.

#ifndef NPC_DEMOMONSTER_H
#define NPC_DEMOMONSTER_H

#if defined( _WIN32 )
#pragma once
#endif

#include "npc_surface.h"

// 0 for kens code, 1 for ken's code running in faster vphysics loop, 2 for ilya's new code
#define BLOB_PHYSICS_TEST 0

//-----------------------------------------------------------------------------

class CDemoMonsterParticle : public PhysParticle
{
public:
	CDemoMonsterParticle(){};

	Point3D force, velocity;//, neighbors;
	int neighbor_count;

	int group;
	int prev_group;

	// I believe these three variables aren't used
	int frozen;
	//int temp1;
	int temp2;
};


class CNPC_BlobDemoMonster : public CNPC_Surface
{
	DECLARE_CLASS( CNPC_BlobDemoMonster, CNPC_Surface );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	//DEFINE_CUSTOM_AI();

public:
	void		Spawn( void );
	virtual bool CreateVPhysics();
	virtual void VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );
	int			OnTakeDamage_Alive( const CTakeDamageInfo &info );
	virtual void PostPhysFrame();
	void		DoPhysics();
	void		RunAI( void );
	void		MoveTowardsGoal( void );
	void		CreateArms( const Vector &vecForward );
	void		RepulseNeighbors( void );

	// Input handlers
	void InputMoveToPosition( inputdata_t &inputdata );
	void InputDoArms( inputdata_t &inputdata );
	void InputDoContactZ( inputdata_t &inputdata );

	// Networked variables
	CUtlVector< float > m_flSurfaceV;

	// Non networked variables
	SmartArray<CDemoMonsterParticle, true, MAX_SURFACE_ELEMENTS> m_physParticles;
	int m_nOwnedSlot[ MAX_SURFACE_ELEMENTS ];
	int m_nTargetSlot[ MAX_SURFACE_ELEMENTS ];
	bool m_bFloat[ MAX_SURFACE_ELEMENTS ];
	int m_nArm[ MAX_SURFACE_ELEMENTS ];
	// keep track of the sphere being in contact with a surface
	bool m_bContact[ MAX_SURFACE_ELEMENTS ];
#if(BLOB_PHYSICS_TEST == 2)
	int m_nParent[ MAX_SURFACE_ELEMENTS ];
#endif

	Vector		m_vecGoal;
	Vector		m_vecPrevGoal;

	bool		m_bDoArms;
	bool		m_bDoContactZ;
	EHANDLE		m_hTarget;
	float		m_flSimTime;
};

#endif