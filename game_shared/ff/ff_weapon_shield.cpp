/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life2 ========
///
/// @file ff_weapon_shield.cpp
/// @author Dexter Haslem
/// @date September 5 2010
/// @brief FF test shield for demoman
///

#include "cbase.h"
#include "ff_weapon_basemelee.h"

#ifdef CLIENT_DLL
	#define CFFWeaponShield C_FFWeaponShield
#endif


#define HOLSTER_LATCH 2.0f

//=============================================================================
// CFFWeaponShield
//=============================================================================

class CFFWeaponShield : public CFFWeaponMeleeBase
{
public:
	DECLARE_CLASS(CFFWeaponShield, CFFWeaponMeleeBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CFFWeaponShield();

	//virtual bool Deploy();
	//virtual bool Holster(CBaseCombatWeapon *pSwitchingTo =0);
	//virtual bool		SendWeaponAnim(int iActivity);
	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_SHIELD; }
	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
private:
	bool m_bWeaponHolstered;
	float m_lastHolsterTime;

	CFFWeaponShield(const CFFWeaponShield &);
};

//=============================================================================
// CFFWeaponShield tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponShield, DT_FFWeaponShield) 

BEGIN_NETWORK_TABLE(CFFWeaponShield, DT_FFWeaponShield) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponShield) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_shield, CFFWeaponShield);
PRECACHE_WEAPON_REGISTER(ff_weapon_shield);

//=============================================================================
// CFFWeapon implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponShield::CFFWeaponShield() 
{
	m_bWeaponHolstered = false;
	m_lastHolsterTime = 0.0f;
}
/*
bool CFFWeaponShield::Deploy()
{
	BaseClass::SendWeaponAnim( ACT_VM_DRAW );
	return BaseClass::Deploy();
}

bool CFFWeaponShield::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	BaseClass::SendWeaponAnim( ACT_VM_HOLSTER );
	return BaseClass::Holster(pSwitchingTo);
}
*/
void CFFWeaponShield::PrimaryAttack()
{
	// do nothing HEH
	//DevMsg("CFFWeaponShield::PrimaryAttack()\n");
}

void CFFWeaponShield::SecondaryAttack()
{
	//BaseClass::Holster();
	//BaseClass::Un
	if ( gpGlobals->curtime > m_lastHolsterTime + HOLSTER_LATCH )
	{
		if( m_bWeaponHolstered )
			BaseClass::SendWeaponAnim( ACT_VM_HOLSTER );
		else
			BaseClass::SendWeaponAnim( ACT_VM_DRAW );

		m_bWeaponHolstered = !m_bWeaponHolstered;
		m_lastHolsterTime = gpGlobals->curtime;
#ifdef CLIENT_DLL
		DevMsg("CFFWeaponShield::SecondaryAttack()- holstered = %s!\n", m_bWeaponHolstered ? "true" : "false" );
#endif
	}
}
/*
bool CFFWeaponShield::SendWeaponAnim(int iActivity)
{
	/*
	if( iActivity == ACT_VM_IDLE )
	{
		if( m_bWeaponHolstered )
			iActivity = ACT_VM_HOLSTER;
		else
			iActivity = ACT_VM_DRAW;		
	}

	return BaseClass::SendWeaponAnim(iActivity);
}*/