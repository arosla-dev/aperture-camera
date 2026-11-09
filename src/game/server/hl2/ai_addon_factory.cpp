//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "eventqueue.h"
#include "saverestore_utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ODDS_NOT_INITIALIZED -1

//=============================================================================
// Struct that carries the name of an addon and the chance it will appear
//=============================================================================
struct addon_info_t
{
	DECLARE_SIMPLE_DATADESC();

	addon_info_t()
	{
		m_iszName = NULL_STRING;
		m_iOdds   = ODDS_NOT_INITIALIZED;
	}
	string_t	m_iszName;
	int			m_iOdds;
};

BEGIN_SIMPLE_DATADESC( addon_info_t )
	DEFINE_FIELD( m_iszName, FIELD_STRING ),
	DEFINE_FIELD( m_iOdds, FIELD_INTEGER ),
END_DATADESC()

//=============================================================================
//=============================================================================
class CAI_AddonFactory : public CPointEntity, public IEntityListener
{
	DECLARE_CLASS( CAI_AddonFactory, CPointEntity );
	DECLARE_DATADESC();



public:
	//---------------------------------
	//Base Entity Stuff
	//---------------------------------
	void Activate();
	bool KeyValue( const char *szKeyName, const char *szValue );
	void UpdateOnRemove();

	//---------------------------------
	//Input Handlers
	//---------------------------------
	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );

	//---------------------------------
	//Listener Stuff
	//---------------------------------	
	virtual void OnEntitySpawned( CBaseEntity *pEntity );

	//---------------------------------
	//Factory Stuff
	//---------------------------------	
	bool CheckIntegrity();
	void InstallAddOns( CAI_BaseNPC *pNPC );
	bool CheckOdds( int iOdds );
	void EnableFactory();
	void DisableFactory();

private:
	CUtlVector<addon_info_t> m_AddonInfo;
	bool m_bDisabled;
};

BEGIN_DATADESC( CAI_AddonFactory )
	DEFINE_UTLVECTOR( m_AddonInfo, FIELD_EMBEDDED ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_KEYFIELD(m_bDisabled, FIELD_BOOLEAN, "StartDisabled" )

END_DATADESC()

LINK_ENTITY_TO_CLASS(ai_addon_factory, CAI_AddonFactory );

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_AddonFactory::Activate()
{
	BaseClass::Activate();
	
	if ( !m_bDisabled )
	{
		EnableFactory();
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_AddonFactory::UpdateOnRemove( void )
{
	gEntList.RemoveListenerEntity( this );
	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_AddonFactory::InputEnable( inputdata_t &inputdata )
{
	if ( m_bDisabled )
	{
		m_bDisabled = false;
		EnableFactory();
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_AddonFactory::InputDisable( inputdata_t &inputdata )
{ 
	if ( !m_bDisabled )
	{
		DisableFactory();
		m_bDisabled = true;
	}
	
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_AddonFactory::EnableFactory()
{
	if ( !m_bDisabled ) // is enabled
	{		
		if ( CheckIntegrity() )
		{
			gEntList.AddListenerEntity( this );
		}
		else
		{
			Msg("**AddOnFactory Error: Integrity check failed, entity removing self\n");
			UTIL_Remove( this );
		}
	}
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_AddonFactory::DisableFactory()
{
	gEntList.RemoveListenerEntity( this );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CAI_AddonFactory::KeyValue( const char *szKeyName, const char *szValue )
{
	const char *pszOptionKey = "option";
	const char *pszWeightKey = "weight";
	
	bool bHandled = BaseClass::KeyValue( szKeyName, szValue );

	// the keyvalues we're using start with these strings.  We need the length
	// in order to know how far to offset by.
	int iOptionKeyNameLength = V_strlen( pszOptionKey );
	int iWeightKeyNameLength = V_strlen( pszWeightKey );


	if ( !bHandled )
	{
		//Msg("Key:%s Value:%s\n", szKeyName, szValue );

		//FIXME need to check explicitly for the substring we're searching for
		if( !V_strncmp( szKeyName, pszOptionKey, iOptionKeyNameLength ) )
		{
			int iIndex = atoi( &szKeyName[ iOptionKeyNameLength ] );
			
			if( iIndex == 0 )
			{
				Msg( "Malformed key name %s\n", szKeyName );
				return true;
			}

			//Msg( "Found option %d \n", iIndex );
			iIndex--; // normalizing from 1 to n -> 0 to n-1.

			// time to stuff this info into the utlVector

			if ( m_AddonInfo.Count() <= iIndex )
			{
				m_AddonInfo.EnsureCount( iIndex+1 );
			}

			m_AddonInfo[iIndex].m_iszName = AllocPooledString( szValue );			
		}
		//FIXME need to check explicitly for the substring we're searching for
		else if( !V_strncmp( szKeyName, pszWeightKey, iWeightKeyNameLength ) )
		{
			int iIndex = atoi( &szKeyName[ iWeightKeyNameLength ] );

			if( iIndex == 0 )
			{
				Msg( "Malformed key name %s\n", szKeyName );
				return true;
			}

			//Msg( "Found weight %d \n", iIndex );
			iIndex--; // normalizing from 1 to n -> 0 to n-1.

			// time to stuff this info into the utlVector

			if ( m_AddonInfo.Count() <= iIndex )
			{
				m_AddonInfo.EnsureCount( iIndex+1 );
			}

			m_AddonInfo[iIndex].m_iOdds = atoi( szValue );
		}
		else 
		{
			Msg( "%s: Unknown KeyName %s.\n", GetDebugName(), szKeyName );
		}
	}
	return bHandled;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_AddonFactory::OnEntitySpawned( CBaseEntity *pEntity )
{
	//Msg( "%s named %s just spawned.\n", pEntity->GetClassname(), pEntity->GetEntityName() );
	IEntityListener::OnEntitySpawned( pEntity );

	if( pEntity->NameMatches( m_target ) || pEntity->ClassMatches( m_target ) )
	{
		CAI_BaseNPC *pNPC = pEntity->MyNPCPointer();

		if( !pNPC )
		{
			Msg("**AddOnFactory Error: %s is not an NPC!\n", pEntity->GetDebugName() );
			return;
		}

		InstallAddOns( pNPC );
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CAI_AddonFactory::CheckIntegrity()
{
	for( int i=0 ; i<m_AddonInfo.Count() ; i++ )
	{
		if( m_AddonInfo[i].m_iOdds == ODDS_NOT_INITIALIZED )
		{
			Msg("**AddOnFactory Error: Option %d did not specify odds\n", i );
			return false;
		}
		if( m_AddonInfo[i].m_iszName == NULL_STRING )
		{
			Msg("**AddOnFactory Error: Option %d did not specify add on name\n", i );
			return false;
		}
	}

	/* Dump the contents of the addon options 
	for( int i=0 ; i<m_AddonInfo.Count() ; i++ )
	{
		Msg( "Index: %d - %s - Odds: %d\n", i, STRING( m_AddonInfo[i].m_iszName) , m_AddonInfo[i].m_iOdds );
	}*/

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_AddonFactory::InstallAddOns( CAI_BaseNPC *pNPC )
{
	// Iterate the list of addon options, roll dice, and install the winners
	for( int i = 0 ; i < m_AddonInfo.Count() ; i++ )
	{
		if( CheckOdds( m_AddonInfo[i].m_iOdds ) )
		{
			// Use entity I/O to install this addon
			variant_t variant;
			variant.SetString( m_AddonInfo[i].m_iszName );
			g_EventQueue.AddEvent( (CBaseEntity *)pNPC, "CreateAddOn", variant, 0.01f, this, this );
		}
	}
}

//-----------------------------------------------------------------------------
// Throw d100 and return true if result <= iOdds. 0 Always returns false.
//-----------------------------------------------------------------------------
bool CAI_AddonFactory::CheckOdds( int iOdds )
{
	if( iOdds < 0 || iOdds > 100 )
	{
		Msg("**AddOnFactory Error: Odds %d is out of range!\n", iOdds );
		return false;
	}

	// Draw a random number from 1 to 100. 
	int iDice = RandomInt( 1, 100 );

	if( iDice <= iOdds )
		return true;

	return false;
}
