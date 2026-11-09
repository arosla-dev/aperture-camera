//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef NPC_FOUNTAIN_H
#define NPC_FOUNTAIN_H

#if defined( _WIN32 )
#pragma once
#endif

#include "npc_surface.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

class CBlobFountainController;

class CNPC_BlobFountain : public CNPC_Surface
{
	DECLARE_CLASS( CNPC_BlobFountain, CNPC_Surface );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	//DEFINE_CUSTOM_AI();

public:
	virtual bool CreateVPhysics();
	virtual void VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );
	virtual void RunAI( void );
	const Vector &GetNozzle()	{ return m_vecStart; };

	void Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular );

#undef USE_BLOBULATOR // TODO (Ilya): Get this working
#ifdef USE_BLOBULATOR
	CLennardJonesForce			m_force;
#endif
	CBlobFountainController		*m_pBlobFountainController;
	IPhysicsMotionController	*m_pMotionController;

	// keep track of the sphere being in contact with a surface
	bool m_bContact[ MAX_SURFACE_ELEMENTS ];
	float m_iContactTime[ MAX_SURFACE_ELEMENTS ];
	float m_fRadius[ MAX_SURFACE_ELEMENTS ];
	int m_iMode[ MAX_SURFACE_ELEMENTS ];
	bool m_bPause;

	// --------------------------------

	// DEFINE_CUSTOM_AI;

private:
};

class CBlobFountainController : public IMotionEvent
{
public:
	DECLARE_SIMPLE_DATADESC();

	CBlobFountainController( CNPC_BlobFountain *pOwner ) { m_pOwner = pOwner; };

	IMotionEvent::simresult_e Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular );

	CNPC_BlobFountain *m_pOwner;
	bool		m_bActive;

	Vector m_vecLennardJonesForce[ MAX_SURFACE_ELEMENTS ];
	float		m_flLennardJonesTime;
};



#endif