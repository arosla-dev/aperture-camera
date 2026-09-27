//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client-side photo with texture bindings
//
//=============================================================================//

#include "cbase.h"
#include "iviewrender.h"
#include "proxyentity.h"
#include "materialsystem/imaterialvar.h"
#include "c_portal_player.h"
#include "imaterialproxydict.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_Photograph : public C_BaseAnimating
{
	DECLARE_CLASS(C_Photograph, C_BaseAnimating);
	DECLARE_CLIENTCLASS();

public:
	const char* GetTextureName(void) { return m_szTextureName; }
	char m_szTextureName[MAX_PATH];

};

IMPLEMENT_CLIENTCLASS_DT(C_Photograph, DT_Photograph, CPhotograph)
RecvPropString(RECVINFO(m_szTextureName)),
END_RECV_TABLE()