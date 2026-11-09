//

#include "cbase.h"

#include "ai_addon.h"
#include "ai_basenpc.h"
#include "ai_behavior.h"
#include "ieffects.h"

#define BRAIN_MODEL "models/addons/brain_tank.mdl"

//---------------------------------------------------------
//---------------------------------------------------------
class CAddOnBrain : public CAI_AddOn
{
public:
	DECLARE_CLASS( CAddOnBrain, CAI_AddOn );
	virtual char *GetAddOnModelName() { return BRAIN_MODEL; }

	//---------------------------------
	// Spawn, etc
	//---------------------------------
	virtual void Spawn();
	virtual void Precache()
	{
		BaseClass::Precache();
	}

	//---------------------------------
	// Appearance, position
	//---------------------------------
	virtual void PickAttachment( CAI_BaseNPC *pHost, char *pchAttachment )
	{
		char szAttachment[ 256 ];

		Q_strcpy( szAttachment, "addon_head" );
		if( IsAddOnAttachmentAvailable(pHost, szAttachment) )
		{
			Q_strcpy( pchAttachment, szAttachment );
			return;
		}
	}

	//---------------------------------
	// Think
	//---------------------------------
	float GetThinkInterval() { return 1.0f; }

	//---------------------------------
	// Install/Remove AddOns
	//---------------------------------
	virtual bool Install( CAI_BaseNPC *pHost, bool bRemoveOnFail = true );

	//DECLARE_DATADESC();
};

//---------------------------------------------------------
//---------------------------------------------------------
class CAI_BrainAddOnBehavior : public CAI_AddOnBehavior<CAddOnBrain>
{
	DECLARE_CLASS( CAI_BrainAddOnBehavior, CAI_SimpleBehavior );

public:
	DECLARE_DATADESC();

	virtual const char *GetName() {	return "Brain"; }
	virtual bool CanSelectSchedule( void ) { return false; }

	CAddOnBrain *AccessShieldAddOn() { return ( m_AddOns.Count() ) ? m_AddOns[0] : NULL; }
};

BEGIN_DATADESC( CAI_BrainAddOnBehavior )
END_DATADESC()


LINK_ENTITY_TO_ADDON_AND_BEHAVIOR( ai_addon_brain, CAddOnBrain, CAI_BrainAddOnBehavior );


//AI_BEGIN_CUSTOM_SCHEDULE_PROVIDER( CAI_BrainAddOnBehavior )
//AI_END_CUSTOM_SCHEDULE_PROVIDER()


//---------------------------------------------------------
//---------------------------------------------------------
void CAddOnBrain::Spawn()
{
	BaseClass::Spawn();
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CAddOnBrain::Install( CAI_BaseNPC *pHost, bool bRemoveOnFail )
{
	if( !BaseClass::Install( pHost, bRemoveOnFail ) )
		return false;

	pHost->CapabilitiesAdd( bits_CAP_DOORS_GROUP );

	return true;
}

