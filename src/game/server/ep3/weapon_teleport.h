//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//


#ifndef EP3_WEAPON_TELEPORT_H
#define EP3_WEAPON_TELEPORT_H

extern void ShootTeleportProjectile( Vector vOrigin, QAngle angles, CBaseEntity *pOwner, CBaseEntity *pGun );
extern CBaseEntity *ShootTeleportProjectileNPC( Vector vOrigin, Vector vTossVelocity, CBaseEntity *pOwner );



#endif // EP3_WEAPON_TELEPORT_H
