//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: Spawn and use functions for editor-placed triggers.
//
//===========================================================================//

#include "cbase.h"
#include "triggers.h"
#include "portal_player.h"
#include "ai_basenpc.h"
#include "props.h"
#include "vcollide_parse.h"
#include "solidsetdefaults.h"
#include "physics_saverestore.h"

bool g_bAllOnReleasedChainedToBase;

bool UTIL_FizzlePlayerPhotos( CPortal_Player *pPlayer )
{
	// Must have photos to bother
	if ( Photo_Count() == 0 )
		return false;

	CaptureInfo_t captureInfo;

	bool bFizzleOccurred = false;
	// FIXME: Need better accessor
	for ( int i = 0; i < 3; i++ )
	{
		if ( Photo_Get( i, &captureInfo ) )
		{
			bFizzleOccurred = true;

			// Recreate the object
			CBaseEntity *pFizzledObject = UTIL_RestoreCapturedObject( captureInfo, captureInfo.vecOldOrigin, captureInfo.vecOldAngles, captureInfo.nOldScaleLevel );
			if ( pFizzledObject )
			{
				CBaseAnimating *pAnim = pFizzledObject->GetBaseAnimating();
				if ( pAnim )
				{
					pAnim->OnFizzled();
				}
			}
		}
	}

	// FIXME: Temp masking effect
	color32 white = { 255, 255, 255, 255 };
	UTIL_ScreenFade( pPlayer, white, 0.25f, 0.0f, FFADE_IN );

	// Make them all go away!
	pPlayer->StripPhotos();

	return bFizzleOccurred;
}

class CTriggerPhotoEraser : public CBaseTrigger
{
public:
	DECLARE_CLASS( CTriggerPhotoEraser, CBaseTrigger );
	virtual int	ObjectCaps( void ) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	virtual void Spawn( void )
	{
		BaseClass::Spawn();

		// Don't let the camera shoot through us!
		InitTrigger();

		if ( m_bDisabled == false )
		{
			// We want to hit camera traces!
			SetCollisionGroup( COLLISION_GROUP_CAMERA_SOLID );
			RemoveSolidFlags( FSOLID_NOT_SOLID ); // HACK: We want camera traces to hit this!
		}
	}

	virtual void Touch( CBaseEntity *pOther )
	{
		// We only touch players
		if ( pOther == NULL || pOther->IsPlayer() == false )
			return;

		// Must be enabled
		if ( m_bDisabled )
			return;

		CPortal_Player *pPlayer = (CPortal_Player *) ToBasePlayer( pOther );
		
		if ( UTIL_FizzlePlayerPhotos( pPlayer ) )
		{
			// Fire off the output
			m_OnObjectsFizzled.FireOutput( pPlayer, this );
		}
	}
	
	virtual void Enable( void )
	{
		SetCollisionGroup( COLLISION_GROUP_CAMERA_SOLID );
		RemoveSolidFlags( FSOLID_NOT_SOLID ); // HACK: We want camera traces to hit this!
	}

	virtual void Disable( void )
	{
		SetCollisionGroup( COLLISION_GROUP_NONE );
		AddSolidFlags( FSOLID_NOT_SOLID );
	}

	DECLARE_DATADESC();

protected:
	COutputEvent	m_OnObjectsFizzled;
};

BEGIN_DATADESC( CTriggerPhotoEraser )
	DEFINE_OUTPUT( m_OnObjectsFizzled, "OnObjectsFizzled" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( trigger_photo_eraser, CTriggerPhotoEraser );



//-----------------------------------------------------------------------------
// Purpose: Replace the object in the world
//-----------------------------------------------------------------------------

CBaseEntity *UTIL_RestoreCapturedObject( CaptureInfo_t captureInfo, const Vector &vecPoint, const QAngle &vecAngles, int nScaleLevel, CInfoPlacementHelper *pHelper )
{
	CBaseEntity* pEnt = captureInfo.hCapturedEnt;
	if ( pEnt == NULL )
	{
		Assert( pEnt != NULL );
		Warning( "Tried to restore NULL object.  Object was most likely destroyed when in the camera's inventory!\n" );
		return NULL;
	}

	CBaseAnimating* pEntAnimating = (CBaseAnimating*)pEnt;
	Assert ( pEntAnimating );

	if ( !pEntAnimating )
		return NULL;

	Vector vecPlacementOrigin = vecPoint;
	QAngle vecPlacementAngles = vecAngles;

	if ( pHelper )
	{
		vecPlacementOrigin = pHelper->GetTargetOrigin();

		if ( pHelper->ShouldUseHelperAngles() )
		{
			vecPlacementAngles = pHelper->GetTargetAngles();
		}

		pHelper->m_OnObjectPlaced.FireOutput( captureInfo.hCapturedEnt, UTIL_GetLocalPlayer() );
		pHelper->m_ObjectPlacedSize.Set( nScaleLevel, captureInfo.hCapturedEnt, UTIL_GetLocalPlayer()  );
	}

	pEnt->SetStasis( false );
	pEnt->Teleport( &vecPlacementOrigin, &vecPlacementAngles, &captureInfo.vecVelocity );

	

	float flModelScale = 1.0f;
	if ( captureInfo.pPlacementQuery )
	{
		flModelScale = captureInfo.pPlacementQuery->GetScaleForStep( nScaleLevel, &captureInfo );
	}

	CAI_BaseNPC* pNPCPointer = dynamic_cast<CAI_BaseNPC*>(pEnt->MyCombatCharacterPointer());
	if ( pNPCPointer )
	{
		pNPCPointer->SetHullSizeNormal( true );
	}

	// Restore the angular and spatial velocity
	IPhysicsObject *pObject = pEntAnimating->VPhysicsGetObject();
	if ( pObject && pObject->IsMoveable() )
	{
		pObject->SetVelocityInstantaneous( &captureInfo.vecVelocity, &captureInfo.vecAngVelocity );
		pObject->Wake();
	}

	// Find the amount we need to scale to reach our new desired size, starting from our old one
	Assert( pEntAnimating->m_flModelScale > 0 );
	if ( flModelScale != pEntAnimating->GetModelScale() )
	{
		UTIL_CreateScaledPhysObject( pEntAnimating, flModelScale );

		// Let the object know how large it is now
		pEntAnimating->SetModelScale( flModelScale );
		pEntAnimating->SetObjectScaleLevel( nScaleLevel );
	}

	if ( pNPCPointer )
	{
		pNPCPointer->SetHullSizeNormal( true );
	}
	
	g_bAllOnReleasedChainedToBase = false;
	pEntAnimating->OnReleased();

	// NOTE: If you're here, you forgot to chain to the base class in an OnReleased implementation! 
	Assert( g_bAllOnReleasedChainedToBase );
	
	return pEnt;
}