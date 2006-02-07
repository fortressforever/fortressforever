/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life2 ========
///
/// @file ff_weapon_crowbar.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF Crowbar code & class declaration.
///
/// REVISIONS
/// ---------
/// Jan 18, 2005 Mirv: Added to project


#include "cbase.h"
#include "ff_weapon_basemelee.h"

#ifdef CLIENT_DLL
	#define CFFWeaponCrowbar C_FFWeaponCrowbar
#endif

//=============================================================================
// CFFWeaponCrowbar
//=============================================================================

class CFFWeaponCrowbar : public CFFWeaponMeleeBase
{
public:
	DECLARE_CLASS(CFFWeaponCrowbar, CFFWeaponMeleeBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CFFWeaponCrowbar();

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_CROWBAR; }

private:

	CFFWeaponCrowbar(const CFFWeaponCrowbar &);
};

//=============================================================================
// CFFWeaponCrowbar tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponCrowbar, DT_FFWeaponCrowbar) 

BEGIN_NETWORK_TABLE(CFFWeaponCrowbar, DT_FFWeaponCrowbar) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponCrowbar) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_crowbar, CFFWeaponCrowbar);
PRECACHE_WEAPON_REGISTER(ff_weapon_crowbar);

//=============================================================================
// CFFWeapon implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponCrowbar::CFFWeaponCrowbar() 
{
}
