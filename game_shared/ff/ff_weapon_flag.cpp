/********************************************************************
	created:	2006/01/21
	created:	21:1:2006   22:22
	filename: 	f:\cvs\code\game_shared\ff\ff_weapon_flag.cpp
	file path:	f:\cvs\code\game_shared\ff
	file base:	ff_weapon_flag
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "ff_weapon_basemelee.h"

#ifdef CLIENT_DLL
	#define CFFWeaponFlag C_FFWeaponFlag
#endif

//=============================================================================
// CFFWeaponFlag
//=============================================================================

class CFFWeaponFlag : public CFFWeaponMeleeBase
{
public:
	DECLARE_CLASS(CFFWeaponFlag, CFFWeaponMeleeBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CFFWeaponFlag();

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_FLAG; }

private:

	CFFWeaponFlag(const CFFWeaponFlag &);
};

//=============================================================================
// CFFWeaponFlag tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponFlag, DT_FFWeaponFlag) 

BEGIN_NETWORK_TABLE(CFFWeaponFlag, DT_FFWeaponFlag) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponFlag) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_flag, CFFWeaponFlag);
PRECACHE_WEAPON_REGISTER(ff_weapon_flag);

//=============================================================================
// CFFWeaponFlag implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponFlag::CFFWeaponFlag() 
{
}
