//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "unseen_movement.h"
#include "ai_network.h"
#include "ai_networkmanager.h"
#include "ai_node.h"
#include "ai_link.h"
#include "ai_basenpc.h"
#include "datacache/imdlcache.h"

#include "tier0/memdbgon.h"


CFindNodeFilter::CFindNodeFilter( NodeType_e type ) : 
m_NodeType( type )
{

}

bool CFindNodeFilter::IsValid( CAI_Node *pNode )
{
	return pNode->GetType() == m_NodeType;
}

bool CFindNodeFilter::ShouldContinue()
{ 
	return true;
}


struct AINodePath_t
{
	CAI_Node *pNode;
	float fPathLengthSqr;
};

inline bool PointIsVisible( const Vector &vEye, const Vector &vTest, unsigned char *pvs, size_t pvsSize )
{
	if ( !engine->CheckOriginInPVS( vTest, pvs, pvsSize ) )
	{
		return false;
	}
	else 
	{
		trace_t tr;
		UTIL_TraceLine( vEye, vTest, MASK_VISIBLE, NULL, COLLISION_GROUP_NONE, &tr );
		if( tr.fraction != 1.0f )
			return false;
	}

	return true;
}

inline bool BoxIsVisible( const Vector &vEye, const Vector &vTest, const Vector &vBoxExtents, unsigned char *pvs, size_t pvsSize )
{
	if ( !engine->CheckBoxInPVS( vTest - vBoxExtents, vTest + vBoxExtents, pvs, pvsSize ) )
	{
		return false;
	}
	else 
	{
		//box is in PVS. Test 8 corners and center for visibility.
		trace_t tr;
		UTIL_TraceLine( vEye, vTest, MASK_VISIBLE, NULL, COLLISION_GROUP_NONE, &tr );
		if( tr.fraction == 1.0f )
			return true;

		for( int i = 0; i != 8; ++i )
		{
			Vector vTemp = vTest;
			vTemp.x += (i & (1<<0)) ? vBoxExtents.x : -vBoxExtents.x;
			vTemp.y += (i & (1<<1)) ? vBoxExtents.y : -vBoxExtents.y;
			vTemp.z += (i & (1<<2)) ? vBoxExtents.z : -vBoxExtents.z;
			UTIL_TraceLine( vEye, vTemp, MASK_VISIBLE, NULL, COLLISION_GROUP_NONE, &tr );
			if( tr.fraction == 1.0f )
				return true;
		}
		
		return false;
	}
}

void Recursive_FindUnseenNodes( const Vector &vEye, const Vector &vBoxExtents, CAI_Network *pNetwork, CAI_Node *pCurNode, bool *pVisitedNodes, float fCrawledLengthSqr, AINodePath_t *pFoundPaths, int &iFoundPaths, unsigned char *pvs, size_t iPvsSize )
{
	pVisitedNodes[pCurNode->GetId()] = true;

	if ( !BoxIsVisible( vEye, vBoxExtents, pCurNode->GetOrigin(), pvs, iPvsSize ) )
	{
		//NDebugOverlay::Box( pCurNode->GetOrigin(), -vBoxExtents, vBoxExtents, 0, 255, 0, 100, 5.0f );
		pFoundPaths[iFoundPaths].fPathLengthSqr = fCrawledLengthSqr;
		pFoundPaths[iFoundPaths].pNode = pCurNode;
		++iFoundPaths;
		return;
	}

	//NDebugOverlay::Box( pCurNode->GetOrigin(), -vBoxExtents, vBoxExtents, 0, 0, 255, 50, 5.0f );

	int iLinks = pCurNode->NumLinks();
	pCurNode->ShuffleLinks();
	for( int i = 0; i != iLinks; ++i )
	{
		CAI_Link *pLink = pCurNode->GetShuffeledLink( i );
		CAI_Node *pNewNode = pNetwork->GetNode( pLink->m_iDestID );
		if( pNewNode == pCurNode )
			pNewNode = pNetwork->GetNode( pLink->m_iSrcID );

		if( pVisitedNodes[pNewNode->GetId()] )
			continue; //already visited that node

		// NOTE: this math is only a rough approximation to route length ( A^2 + B^2 != (A+B)^2 )
		float fNewCrawledLengthSqr = (pNewNode->GetOrigin() - pCurNode->GetOrigin()).LengthSqr() + fCrawledLengthSqr;

		Recursive_FindUnseenNodes( vEye, vBoxExtents, pNetwork, pNewNode, pVisitedNodes, fNewCrawledLengthSqr, pFoundPaths, iFoundPaths, pvs, iPvsSize ); 
	}
}

int FindUnseenNodes( CAI_Node **pNodesOut, int iOutArraySize, const Vector &vEyePosition, INearestNodeFilter *pNodeFilter, const Vector &vBBoxExtents )
{
	CAI_Network *pNetwork = g_pAINetworkManager->GetNetwork();
	
	int iNearestNode = pNetwork->NearestNodeToPoint( NULL, vEyePosition, false, pNodeFilter );
	if( iNearestNode == NO_NODE )
	{
		Warning( "Nearest node not found\n" );
		return 0;
	}

	CAI_Node *pStartNode = pNetwork->GetNode( iNearestNode );
	if( pStartNode == NULL )
		return 0;

	int iPlayerEyeCluster = engine->GetClusterForOrigin( vEyePosition );
	unsigned char pvs[MAX_MAP_CLUSTERS/8];
	engine->GetPVSForCluster( iPlayerEyeCluster, sizeof( pvs ), pvs );

	int iNumNetworkNodes = pNetwork->NumNodes();
	bool *pAlreadyVisitedNodes = (bool *)stackalloc( sizeof( bool ) * iNumNetworkNodes );
	memset( pAlreadyVisitedNodes, 0, sizeof( bool ) * iNumNetworkNodes );

	AINodePath_t *pFoundPaths = (AINodePath_t *)stackalloc( sizeof( AINodePath_t ) * iNumNetworkNodes );
	int iFoundPaths = 0;

	Recursive_FindUnseenNodes( vEyePosition, 
								vBBoxExtents, 
								pNetwork, 
								pStartNode, 
								pAlreadyVisitedNodes, 
								(pStartNode->GetOrigin() - vEyePosition).LengthSqr(), 
								pFoundPaths, 
								iFoundPaths, 
								pvs, 
								sizeof( pvs ) );

	if( iFoundPaths == 0 )
	{
		Warning( "No unseen nodes found\n" );
		return 0;
	}

	if( iOutArraySize > iFoundPaths )
		iOutArraySize = iFoundPaths;

	//TODO: Sort by path length
	/*float fShortestPath = pFoundPaths[0].fPathLengthSqr;
	CAI_Node *pShortestNode = pFoundPaths[0].pNode;

	for( int i = 1; i != iFoundPaths; ++i )
	{
		if( pFoundPaths[i].fPathLengthSqr < fShortestPath )
		{
			fShortestPath = pFoundPaths[i].fPathLengthSqr;
			pShortestNode = pFoundPaths[i].pNode;
		}
	}*/

	for( int i = 0; i != iOutArraySize; ++i )
	{
		pNodesOut[i] = pFoundPaths[i].pNode;
	}

	return iOutArraySize;
}

void CC_Unseen_Label( const CCommand &args )
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	Vector vPlayerEye = pPlayer->EyePosition();

	size_t iPossibleNodes = g_pAINetworkManager->GetNetwork()->NumNodes();
	CAI_Node **pPossibleNodes = (CAI_Node **)stackalloc( sizeof(CAI_Node *) * iPossibleNodes );
	Vector vBBox( 10.0f, 10.0f, 10.0f );
	Vector vNBBox = -vBBox;

	int iFoundNodes = FindUnseenNodes( pPossibleNodes, iPossibleNodes, vPlayerEye, NODE_GROUND, vBBox );

	for( int i = 0; i != iFoundNodes; ++i )
	{
		NDebugOverlay::Box( pPossibleNodes[i]->GetOrigin(), vNBBox, vBBox, 0, 255, 0, 255, 5.0f );
	}	
}

static ConCommand unseen_label("unseen_label", CC_Unseen_Label, "Label areas that the player cannot see.", FCVAR_CHEAT);


void CC_NPC_Create_Hidden( const CCommand &args )
{
	MDLCACHE_CRITICAL_SECTION();

	bool allowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );


	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	Vector vPlayerEye = pPlayer->EyePosition();

	CAI_BaseNPC *baseNPC = dynamic_cast< CAI_BaseNPC * >( CreateEntityByName(args[1]) );
	if (baseNPC)
	{
		DispatchSpawn(baseNPC);
		Vector vExtents = (baseNPC->GetHullMaxs() - baseNPC->GetHullMins()) / 2.0f;

		size_t iPossibleNodes = g_pAINetworkManager->GetNetwork()->NumNodes();
		CAI_Node **pPossibleNodes = (CAI_Node **)stackalloc( sizeof(CAI_Node *) * iPossibleNodes );
		
		int iFoundNodes = FindUnseenNodes( pPossibleNodes, iPossibleNodes, vPlayerEye, NODE_GROUND, vExtents );

		if( iFoundNodes == 0 )
		{
			UTIL_Remove( baseNPC );
			return;
		}

		Vector vLocation = pPossibleNodes[RandomInt(0, iFoundNodes - 1)]->GetOrigin();
		baseNPC->Teleport( &vLocation, NULL, NULL );

		UTIL_FindClosestPassableSpace( baseNPC, Vector( 0.0f, 0.0f, 1.0f ), baseNPC->GetAITraceMask() );
	}	

	CBaseEntity::SetAllowPrecache( allowPrecache );
}

static ConCommand npc_create_hidden("npc_create_hidden", CC_NPC_Create_Hidden, "Spawn the NPC type in a nearby location the player can't see.", FCVAR_CHEAT);
