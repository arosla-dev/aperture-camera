//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef FUNC_PIPE_ENTRANCE_H
#define FUNC_PIPE_ENTRANCE_H
#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"
#include "triggers.h"

class CFunc_Pipe_Entrance;
class CAI_DynamicLink;

struct LinkedEntrance_t
{
	CFunc_Pipe_Entrance *pPipeEntrance;
	CAI_DynamicLink		*pDynamicLink;
};

class CFunc_Pipe_Entrance : public CBaseEntity
{
public:
	DECLARE_CLASS( CFunc_Pipe_Entrance, CBaseEntity );
	DECLARE_DATADESC();

	CFunc_Pipe_Entrance( void );
	virtual ~CFunc_Pipe_Entrance( void );
	void LinkToEntrances( void );
	void UnlinkEntrances( void );
	int GetLinkedEntranceIndex( CFunc_Pipe_Entrance *pPipeEntrance );

	void Enable( void );
	void Disable( void );

	virtual void Spawn( void );
	virtual void Activate( void );
	virtual void UpdateOnRemove( void );

	// Input handlers
	virtual void InputEnable( inputdata_t &inputdata );
	virtual void InputDisable( inputdata_t &inputdata );
	virtual void InputToggle( inputdata_t &inputdata );

	bool		m_bDisabled;

	string_t						m_PipeSystemName;
	float							m_fFlowRate;
	CUtlVector<LinkedEntrance_t>	m_LinkedEntrances;
	int								m_iID;
	float							m_fFlowTimeStamp; //last time a particle entered or exited this pipe
};



#endif // FUNC_PIPE_ENTRANCE_H
