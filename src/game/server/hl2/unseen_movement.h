//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef UNSEEN_MOVEMENT_H
#define UNSEEN_MOVEMENT_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_node.h"
#include "ai_network.h"
class Vector;

class CFindNodeFilter : public INearestNodeFilter
{
public:
	CFindNodeFilter( NodeType_e type );
	virtual bool IsValid( CAI_Node *pNode );
	virtual bool ShouldContinue();
	NodeType_e m_NodeType;
};

int FindUnseenNodes( CAI_Node **pNodesOut, int iOutArraySize, const Vector &vEyePosition, INearestNodeFilter *pNodeFilter, const Vector &vBBoxExtents );
inline int FindUnseenNodes( CAI_Node **pNodesOut, int iOutArraySize, const Vector &vEyePosition, NodeType_e nodeType, const Vector &vBBoxExtents )
{
	CFindNodeFilter filter( nodeType );
	return FindUnseenNodes( pNodesOut, iOutArraySize, vEyePosition, &filter, vBBoxExtents );
}



#endif // UNSEEN_MOVEMENT_H
