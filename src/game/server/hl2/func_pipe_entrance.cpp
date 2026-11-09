//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "func_pipe_entrance.h"
#include "ai_initutils.h"
#include "ai_dynamiclink.h"


#include "tier0/memdbgon.h"


static CUtlVector<CFunc_Pipe_Entrance *> s_PipeEntrances;


LINK_ENTITY_TO_CLASS( func_pipe_entrance, CFunc_Pipe_Entrance );

BEGIN_DATADESC( CFunc_Pipe_Entrance )
	DEFINE_KEYFIELD( m_PipeSystemName,	FIELD_STRING,	"pipesystemname" ),	
	DEFINE_KEYFIELD( m_fFlowRate, FIELD_FLOAT, "flowrate" ),
	DEFINE_KEYFIELD( m_fFlowTimeStamp, FIELD_FLOAT, "flowtimestamp" ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),
END_DATADESC()


CFunc_Pipe_Entrance::CFunc_Pipe_Entrance( void )
{
	m_iID = s_PipeEntrances.AddToTail( this );
}

CFunc_Pipe_Entrance::~CFunc_Pipe_Entrance( void )
{
	s_PipeEntrances.FindAndRemove( this );
}

//------------------------------------------------------------------------------
// Purpose: Turns on this trigger.
//------------------------------------------------------------------------------
void CFunc_Pipe_Entrance::Enable( void )
{
	m_bDisabled = false;

	if ( VPhysicsGetObject())
	{
		VPhysicsGetObject()->EnableCollisions( true );
	}

	if (!IsSolidFlagSet( FSOLID_TRIGGER ))
	{
		AddSolidFlags( FSOLID_TRIGGER ); 
		PhysicsTouchTriggers();
	}

	for( int i = m_LinkedEntrances.Count(); --i >= 0; )
	{
		CFunc_Pipe_Entrance *pLinkedEntrance = m_LinkedEntrances[ i ].pPipeEntrance;
		if ( !pLinkedEntrance->m_bDisabled )
		{
			// The other entrance is also enabled, so turn on the dynamic link
			CAI_DynamicLink *pDynamicLink = m_LinkedEntrances[ i ].pDynamicLink;
			pDynamicLink->m_nLinkState = LINK_ON;
			pDynamicLink->SetLinkState();
		}
	}
}

//------------------------------------------------------------------------------
// Purpose: Turns off this trigger.
//------------------------------------------------------------------------------
void CFunc_Pipe_Entrance::Disable( void )
{ 
	m_bDisabled = true;

	if ( VPhysicsGetObject())
	{
		VPhysicsGetObject()->EnableCollisions( false );
	}

	if (IsSolidFlagSet(FSOLID_TRIGGER))
	{
		RemoveSolidFlags( FSOLID_TRIGGER ); 
		PhysicsTouchTriggers();
	}

	for( int i = m_LinkedEntrances.Count(); --i >= 0; )
	{
		// Turn off the dynamic link
		CAI_DynamicLink *pDynamicLink = m_LinkedEntrances[ i ].pDynamicLink;
		pDynamicLink->m_nLinkState = LINK_OFF;
		pDynamicLink->SetLinkState();
	}
}

void CFunc_Pipe_Entrance::Spawn( void )
{
	// Fix maps that were created before flow rate could be set
	if ( m_fFlowRate <= 0.0f )
		m_fFlowRate = 20.0f;

	BaseClass::Spawn();
}

void CFunc_Pipe_Entrance::Activate( void )
{
	LinkToEntrances();

	SetSolid( SOLID_VPHYSICS );
	SetSolidFlags( FSOLID_TRIGGER | FSOLID_NOT_SOLID );
	SetMoveType( MOVETYPE_NONE );
	SetModel( STRING( GetModelName() ) );
	AddEffects( EF_NODRAW );

	BaseClass::Activate();
}

void CFunc_Pipe_Entrance::UpdateOnRemove( void )
{
	UnlinkEntrances();
	BaseClass::UpdateOnRemove();
}

void CFunc_Pipe_Entrance::LinkToEntrances( void )
{
	if( m_PipeSystemName.ToCStr() == NULL || m_PipeSystemName.ToCStr()[0] == '\0' )
		return;	//no name, don't link

	for( int i = s_PipeEntrances.Count(); --i >= 0; )
	{
		if( s_PipeEntrances[i] == this )
		{
			// Create a node for this entrance
			CNodeEnt *pNode = static_cast<CNodeEnt*>( CreateEntityByName( "info_node" ) );
			pNode->SetAbsOrigin( GetAbsOrigin() );
			pNode->m_NodeData.nWCNodeID = 10000 + m_iID;
			DispatchSpawn( pNode );

			continue;
		}

		if( s_PipeEntrances[i]->m_PipeSystemName == m_PipeSystemName )
		{
			//entrances are part of the same system, link.
			LinkedEntrance_t linkedEntrance;
			linkedEntrance.pPipeEntrance = s_PipeEntrances[i];

			int iIndex = s_PipeEntrances[i]->GetLinkedEntranceIndex( this );

			if ( iIndex != s_PipeEntrances[i]->m_LinkedEntrances.InvalidIndex() )
			{
				// There's already a link from that pipe entrance to this one, share it's dynamic link!
				LinkedEntrance_t *pLinkedEntrance = &(s_PipeEntrances[i]->m_LinkedEntrances[iIndex]);
				linkedEntrance.pDynamicLink = pLinkedEntrance->pDynamicLink;
			}
			else
			{
				linkedEntrance.pDynamicLink = static_cast<CAI_DynamicLink*>( CreateEntityByName( "info_node_link" ) );
				linkedEntrance.pDynamicLink->m_nSrcEditID = 10000 + m_iID;
				linkedEntrance.pDynamicLink->m_nDestEditID = 10000 + i;
				linkedEntrance.pDynamicLink->m_nLinkType = bits_CAP_MOVE_CRAWL;
				linkedEntrance.pDynamicLink->AddSpawnFlags( bits_TINY_FLUID_HULL );
				linkedEntrance.pDynamicLink->m_nLinkState = LINK_ON;
				DispatchSpawn( linkedEntrance.pDynamicLink );
			}
			
			m_LinkedEntrances.AddToTail( linkedEntrance );
		}
	}
}

void CFunc_Pipe_Entrance::UnlinkEntrances( void )
{
	for( int i = m_LinkedEntrances.Count(); --i >= 0; )
	{
		LinkedEntrance_t *pLinkedEntrance = &(m_LinkedEntrances[i]);

		int iIndex = pLinkedEntrance->pPipeEntrance->GetLinkedEntranceIndex( this );
		pLinkedEntrance->pPipeEntrance->m_LinkedEntrances.Remove( iIndex );
	}
	m_LinkedEntrances.RemoveAll();
}

int CFunc_Pipe_Entrance::GetLinkedEntranceIndex( CFunc_Pipe_Entrance *pPipeEntrance )
{
	for( int i = m_LinkedEntrances.Count(); --i >= 0; )
	{
		LinkedEntrance_t *pLinkedEntrance = &(m_LinkedEntrances[i]);

		if ( pLinkedEntrance->pPipeEntrance == pPipeEntrance )
			return i;
	}

	return m_LinkedEntrances.InvalidIndex();
}

//------------------------------------------------------------------------------
// Purpose: Input handler to turn on this trigger.
//------------------------------------------------------------------------------
void CFunc_Pipe_Entrance::InputEnable( inputdata_t &inputdata )
{ 
	Enable();
}


//------------------------------------------------------------------------------
// Purpose: Input handler to turn off this trigger.
//------------------------------------------------------------------------------
void CFunc_Pipe_Entrance::InputDisable( inputdata_t &inputdata )
{ 
	Disable();
}

//-----------------------------------------------------------------------------
// Purpose: Toggles this trigger between enabled and disabled.
//-----------------------------------------------------------------------------
void CFunc_Pipe_Entrance::InputToggle( inputdata_t &inputdata )
{
	if (IsSolidFlagSet( FSOLID_TRIGGER ))
	{
		RemoveSolidFlags(FSOLID_TRIGGER);
	}
	else
	{
		AddSolidFlags(FSOLID_TRIGGER);
	}

	PhysicsTouchTriggers();
}
