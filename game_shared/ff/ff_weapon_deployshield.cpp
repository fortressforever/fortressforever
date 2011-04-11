/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life2 ========
///
/// @file ff_weapon_deployshield.cpp
/// @author Greg "Green Mushy" Stefanakis
/// @brief The active shield implementation
///

#include "cbase.h"
#include "ff_weapon_basemelee.h"

#ifdef CLIENT_DLL
	#define CFFWeaponDeployShield C_FFWeaponDeployShield	
	#include "c_ff_player.h"
#else
	#include "ff_player.h"
#endif


//=============================================================================
// CFFWeaponDeployShield
//=============================================================================

class CFFWeaponDeployShield : public CFFWeaponMeleeBase
{
public:
	DECLARE_CLASS(CFFWeaponDeployShield, CFFWeaponMeleeBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CFFWeaponDeployShield();
	
	void ShieldIdle( void );
	virtual bool Deploy();	//need this to override the base deploy()
	virtual bool Holster(); // need this to override the base holster() and remove shield effects

	virtual void ItemPostFrame( void );	//acts as the update function

	//Get the shield bool
	bool GetShieldActive(){ return m_bShieldActive; }

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_DEPLOYSHIELD; }

private:

	CFFWeaponDeployShield(const CFFWeaponDeployShield &);
	bool m_bShieldActive;//used to keep track of active shield to only call animations once per active/deactive

};

//=============================================================================
// CFFWeaponDeployShield tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponDeployShield, DT_FFWeaponDeployShield) 

BEGIN_NETWORK_TABLE(CFFWeaponDeployShield, DT_FFWeaponDeployShield) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponDeployShield) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_deployshield, CFFWeaponDeployShield);
PRECACHE_WEAPON_REGISTER(ff_weapon_deployshield);

//=============================================================================
// CFFWeapon implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponDeployShield::CFFWeaponDeployShield() 
{
	//Start shield holstered
	m_bShieldActive = false;
}

//----------------------------------------------------------------------------
// Purpose: Override the deploy call and immediatly start holstering it so its not "active"to the player's viewmodel
//----------------------------------------------------------------------------
bool CFFWeaponDeployShield::Deploy()
{
	//Start the shield out inactive
	m_bShieldActive = false;
#ifdef CLIENT_DLL
	DevMsg("Active shield deployed.\n");
#endif
	return CFFWeaponMeleeBase::Deploy();
}
//----------------------------------------------------------------------------
// Purpose: Override the holster call to remove shield effects on the player if they switch weapons
//----------------------------------------------------------------------------
bool CFFWeaponDeployShield::Holster()
{
	//Set the shield to idle to do cleanup and reseting stuff
	m_bShieldActive = false;
	ShieldIdle();

#ifdef CLIENT_DLL
	DevMsg("Active shield holstered.\n");
#endif

	//Now do normal holster
	return CFFWeaponMeleeBase::Holster();
}

//-----------------------------------------------------------------------------
// Purpose: A modified ItemPostFrame to allow for different cycledecrements
//-----------------------------------------------------------------------------
void CFFWeaponDeployShield::ItemPostFrame()
{
	//Get the owner
	CFFPlayer *pPlayer = GetPlayerOwner();

	//If there is no player gtfo
	if( !pPlayer )
		return;

	//-------------------------
	//Pressing Attack
	//-------------------------
	//if ((pPlayer->m_nButtons & IN_ATTACK || pPlayer->m_afButtonPressed & IN_ATTACK) )
	//{
	//	ShieldIdle();
	//}

	//-----------------------
	// No buttons down
	//-----------------------
	if ( !( pPlayer->m_nButtons & IN_ATTACK ) /*|| !(pPlayer->GetFlags() & FL_ONGROUND)*/ )
	{
		ShieldIdle();
	}
}

//----------------------------------------------------------------------------
// Purpose: Switches back to the idle shield
//----------------------------------------------------------------------------
void CFFWeaponDeployShield::ShieldIdle( void ) 
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	//If no player gtfo
	if( pPlayer == NULL )
		return;

	//Check so this only gets called once
	if( m_bShieldActive == false )
	{
		m_bShieldActive = true;

#ifdef GAME_DLL
		pPlayer->SetRiotShieldActive(false);

		//If the player is effected by the shield slow, remove it
		if( pPlayer->IsSpeedEffectSet(SE_SHIELD) == true )
		{
			//Remove the effect that was put on earlier
			pPlayer->RemoveSpeedEffect(SE_SHIELD);
		}
#endif

#ifdef CLIENT_DLL
		//Quickswap
		pPlayer->SwapToWeapon(FF_WEAPON_SHIELD);

		DevMsg("Swapping to idle shield.\n");
#endif
	}
}