/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life2 ========
///
/// @file ff_weapon_umbrella.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF Umbrella code & class declaration.
///
/// REVISIONS
/// ---------
/// Jan 19, 2005 Mirv: Initial implementation


#include "cbase.h"
#include "ff_weapon_basemelee.h"

#ifdef CLIENT_DLL
	#define CFFWeaponUmbrella C_FFWeaponUmbrella
#endif

//=============================================================================
// CFFWeaponUmbrella
//=============================================================================

class CFFWeaponUmbrella : public CFFWeaponMeleeBase
{
public:
	DECLARE_CLASS(CFFWeaponUmbrella, CFFWeaponMeleeBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CFFWeaponUmbrella();

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_UMBRELLA; }

private:

	CFFWeaponUmbrella(const CFFWeaponUmbrella &);
};

//=============================================================================
// CFFWeaponUmbrella tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponUmbrella, DT_FFWeaponUmbrella) 

BEGIN_NETWORK_TABLE(CFFWeaponUmbrella, DT_FFWeaponUmbrella) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponUmbrella) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_umbrella, CFFWeaponUmbrella);
PRECACHE_WEAPON_REGISTER(ff_weapon_umbrella);

//=============================================================================
// CFFWeapon implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponUmbrella::CFFWeaponUmbrella() 
{
}
