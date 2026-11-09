//========== Copyright © 2005, Valve Corporation, All rights reserved. ========
//
// Purpose:
//
//=============================================================================

#include "cbase.h"

#include "ai_basenpc.h"
#include "ai_utils.h"
#include "ai_addon.h"
#include "monstermaker.h"
#include "ndebugoverlay.h"

#include "memdbgon.h"

ConVar ai_addons_buy_for_free("ai_addons_buy_for_free", "0");

//=============================================================================
//=============================================================================
class CAI_AddonBuilder : public CPointEntity
{
	DECLARE_CLASS( CAI_AddonBuilder, CPointEntity );
	DECLARE_DATADESC();

public:
	void Execute();
	int GetCost( string_t pData );

	void SetBlinkEntity( CBaseEntity *pEntity );


	//---------------------------------
	//Input Handlers
	//---------------------------------
	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );
	void InputExecute( inputdata_t &inputdata );
	
	COutputEvent m_OnCreateNpc;
	COutputEvent m_OnFailedToCreateNpc;
	COutputEvent m_OnCreateAddon;
	COutputEvent m_OnFailedToCreateAddon;



	int DrawDebugTextOverlays(void);

private:
	string_t m_iszClassnameOrTemplate;
	string_t m_iszClassname;
	string_t m_iszNPCName;
	string_t m_iszAddOnName;
	int m_nNpcPoints;
	int m_nAddonPoints;
	bool m_bDisabled;
	int m_iCost;

	EHANDLE	m_hBlinkEntity;
};

class CAI_AddOnBuilderTemplateNPCMaker : public CTemplateNPCMaker
{
public:
	void SetTemplate( string_t iszTemplate )
	{
		m_iszTemplateName = iszTemplate;
		m_nMaxLiveChildren = INT_MAX;
	}
};

BEGIN_DATADESC( CAI_AddonBuilder )
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Execute", InputExecute ),

	DEFINE_OUTPUT( m_OnCreateNpc, "OnCreateNpc" ),
	DEFINE_OUTPUT( m_OnFailedToCreateNpc, "OnFailedToCreateNpc" ),
	DEFINE_OUTPUT( m_OnCreateAddon, "OnCreateAddon" ),
	DEFINE_OUTPUT( m_OnFailedToCreateAddon, "OnFailedToCreateAddon" ),

	DEFINE_FIELD(m_iszClassnameOrTemplate, FIELD_STRING ),
	DEFINE_KEYFIELD(m_iszNPCName, FIELD_STRING, "NPCName" ),
	DEFINE_KEYFIELD(m_iszAddOnName, FIELD_STRING, "AddOnName" ),
	DEFINE_KEYFIELD(m_nNpcPoints, FIELD_INTEGER, "NpcPoints" ),
	DEFINE_KEYFIELD(m_nAddonPoints, FIELD_INTEGER, "AddonPoints" ),
	DEFINE_FIELD(m_iCost, FIELD_INTEGER ),
	DEFINE_FIELD(m_hBlinkEntity, FIELD_EHANDLE ),
	DEFINE_KEYFIELD(m_bDisabled, FIELD_BOOLEAN, "StartDisabled" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS(ai_addon_builder, CAI_AddonBuilder );

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_AddonBuilder::InputEnable( inputdata_t &inputdata )
{
	m_bDisabled = false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_AddonBuilder::InputDisable( inputdata_t &inputdata )
{
	m_bDisabled = true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_AddonBuilder::InputExecute( inputdata_t &inputdata )
{
	if ( !m_bDisabled )
	{
		Execute();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draw any debug text overlays
// Output : Current text offset from the top
//-----------------------------------------------------------------------------
int CAI_AddonBuilder::DrawDebugTextOverlays( void ) 
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT) 
	{
		char tempstr[512];

		// display NpcPoints
		Q_snprintf(tempstr,sizeof(tempstr),"NPC Points: %d", m_nNpcPoints);
		EntityText(text_offset,tempstr,0);
		text_offset++;

		// display AddonPoints
		Q_snprintf(tempstr,sizeof(tempstr),"Addon Points: %d", m_nAddonPoints);
		EntityText(text_offset,tempstr,0);
		text_offset++;
	}
	return text_offset;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_AddonBuilder::SetBlinkEntity( CBaseEntity *pEntity )
{
	if( m_hBlinkEntity )
	{
		m_hBlinkEntity->RemoveEffects( EF_ITEM_BLINK );
	}

	pEntity->AddEffects( EF_ITEM_BLINK );
	m_hBlinkEntity.Set( pEntity );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_AddonBuilder::Execute()
{
	Vector	forward;
	trace_t tr;

	CBasePlayer *pPlayer = AI_GetSinglePlayer();

	pPlayer->EyeVectors( &forward );
	Vector vEyePosition = pPlayer->EyePosition();

	//---------------------------------
	// MASK_SHOT on purpose! So that you don't hit the invisible hulls of the NPCs.
	UTIL_TraceLine( vEyePosition, vEyePosition + forward * MAX_COORD_RANGE, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr );

	if ( !tr.m_pEnt )
	{
		EmitSound( "HL2Player.UseDeny" );
		return;
	}

	CAI_BaseNPC *pNPC = tr.m_pEnt->MyNPCPointer();

	static const char *pszAddOnClassname = "ai_addon_*";
	static const char *pszSoldierClassname = "npc_soldier_*";
	static const char *pszCreationSpawnpointName = "ai_addon_builder_spawnpoint*";

	if ( pNPC )
	{	
		if ( pNPC->NameMatches( m_iszNPCName ) )
		{	
			if ( EntityNamesMatch( pszAddOnClassname, m_iszClassnameOrTemplate ) )
			{
				// Are there enough points in the account to create this addon? Or, is purchasing free?
				if ( ( m_nAddonPoints - m_iCost ) >= 0 || ai_addons_buy_for_free.GetBool() )
				{	
					CAI_AddOn *pAddOn = (CAI_AddOn*)CreateEntityByName( STRING(m_iszClassnameOrTemplate) );

					Assert( pAddOn != NULL );

					pAddOn->Spawn();
					
					bool bSuccessful = pAddOn->Install( pNPC );

					if( bSuccessful )
					{
						Msg("AddOn Successful\n");
						pAddOn->EmitSound( "AddOn.Install" );
						
						// Fire the Create Addon output
						m_OnCreateAddon.FireOutput( this, this );
						
						// The addon was successfully installed so deduct the cost of the addon from the point account if it's not free.
						if( !ai_addons_buy_for_free.GetBool() )
						{
							m_nAddonPoints -= m_iCost;
						}
					}
					else
					{
						Msg("AddOn NOT Successful\n");
						// Fire the failed to create addon output
						EmitSound( "HL2Player.UseDeny" );
						m_OnFailedToCreateAddon.FireOutput( this, this );
					}
				}
				else
				{
					Msg( "You do not have enough points available to install that addon.\n" );
					// Fire the failed to create addon output
					EmitSound( "HL2Player.UseDeny" );
					m_OnFailedToCreateAddon.FireOutput( this, this );
				}
			}
			else
			{
				Msg( "That particular NPC is not a prototype. Please copy from a prototype.\n" );
			}
		}
		else if ( pNPC->ClassMatches( pszSoldierClassname ) )
		{
			m_iszClassnameOrTemplate = pNPC->GetEntityName();
			
			// Get the actual classname of the NPC
			m_iszClassname = AllocPooledString( pNPC->GetClassname() );

			EmitSound( "HL2Player.Use" );
			SetBlinkEntity(pNPC);

			// Get the cost of the NPC
			m_iCost = GetCost( m_iszClassname );
		}
	}
	else
	{
		// Look for addons
		CBaseEntity *pAddOn = gEntList.FindEntityByClassnameNearest( pszAddOnClassname, tr.endpos, 36 );
		if ( pAddOn )
		{
			if ( !( pAddOn->GetParent() && pAddOn->GetParent()->IsNPC() ) )
			{
				m_iszClassnameOrTemplate = pAddOn->m_iClassname;
				
				// Get the actual classname of the addon
				m_iszClassname = pAddOn->m_iClassname;
				
				// Get the cost of the addon
				m_iCost = GetCost( m_iszClassname );

				EmitSound( "HL2Player.Use" );
				SetBlinkEntity( pAddOn );
			}
		}
		else if ( !EntityNamesMatch( pszAddOnClassname, m_iszClassnameOrTemplate ) )
		{
			// Are there enough points in the account to create this NPC?
			if ( ( m_nNpcPoints - m_iCost ) >= 0  || ai_addons_buy_for_free.GetBool() )
			{	
				CBaseEntity *pSpawnPoint;

				pSpawnPoint = gEntList.FindEntityByName( NULL, pszCreationSpawnpointName );

				if ( pSpawnPoint )
				{
					while ( pSpawnPoint )
					{
						if ( pSpawnPoint->GetAbsOrigin().DistToSqr( tr.endpos ) < Square( 64 ) )
						{
							break;
						}
						pSpawnPoint = gEntList.FindEntityByName( pSpawnPoint, pszCreationSpawnpointName );

					}

					if ( !pSpawnPoint )
					{
						EmitSound( "HL2Player.UseDeny" );
						Msg( "Failed to create NPC, spawn points not close enough\n" );
						// Fire the failed to create NPC output
						m_OnFailedToCreateNpc.FireOutput( this, this );
						return;
					}
				}

				CAI_BaseNPC *pNPC = NULL;
				CAI_AddOnBuilderTemplateNPCMaker *pMaker = new CAI_AddOnBuilderTemplateNPCMaker;
				pMaker->SetTemplate( m_iszClassnameOrTemplate );
				
				QAngle vecMakerAngles;
				if ( !pSpawnPoint )
				{
					pMaker->SetAbsOrigin( tr.endpos );
					vecMakerAngles = pPlayer->GetAbsAngles();
					vecMakerAngles.y -= 180.0f;
					pMaker->SetAbsAngles( vecMakerAngles );
				}
				else
				{
					pMaker->SetAbsOrigin( pSpawnPoint->GetAbsOrigin() );
					pMaker->SetAbsAngles( pSpawnPoint->GetAbsAngles() );
				}

				DispatchSpawn( pMaker );
				if ( !pMaker->IsMarkedForDeletion() )
				{
					pNPC = pMaker->MakeNPCFromTemplate();
					UTIL_RemoveImmediate( pMaker );
				}

				if ( pNPC )
				{
					EmitSound( "HL2Player.Use" );
					pNPC->SetName( m_iszNPCName );
					// Fire the Spawned NPC output
					m_OnCreateNpc.FireOutput( this, this );
					// The NPC was successfully spawned so deduct the cost of the NPC from the point account

					if( !ai_addons_buy_for_free.GetBool() )
					{
						m_nNpcPoints -= m_iCost;
					}
				}
				else
				{
					Msg( "Failed to create NPC\n" );
					// Fire the failed to create NPC output
					m_OnFailedToCreateNpc.FireOutput( this, this );
				}
			}
			else
			{
				Msg( "You do not have enough points for that NPC.\n" );
				EmitSound( "HL2Player.UseDeny" );
				// Fire the failed to create npc output
				m_OnFailedToCreateNpc.FireOutput( this, this );
			}
		}
	}
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct CostTableEntry_t
{
	const char *pszClassname;
	int iCost;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CostTableEntry_t g_CostTable[] =
{
	{"npc_soldier_large", 4 },
	{"npc_soldier_mid", 2 },
	{"npc_soldier_fast", 1 },
	
	{"ai_addon_dartgun", 2 },
	{"ai_addon_shield", 2 },
	{"ai_addon_propeller", 2 },
	{"ai_addon_minigun", 2 },
	{"ai_addon_rpg", 2 },
	{"ai_addon_bomb", 2 },
	{"ai_addon_saw", 1 },
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CAI_AddonBuilder::GetCost( string_t pData )
{
	// Iterate through the cost table to find the cost of the classname
	for( int i = 0; i < ARRAYSIZE( g_CostTable ); i++ )
	{
		if( !strcmp( g_CostTable[i].pszClassname, STRING( m_iszClassname ) ) )
		{
			Msg( "The cost of %s is %d.\n", pData, g_CostTable[i].iCost );
			return g_CostTable[i].iCost;
		}
	}
	// The classname was not found in the table so default the cost to 1
	Msg( "The cost of %s is 1.\n", pData );	
	return 1;
}