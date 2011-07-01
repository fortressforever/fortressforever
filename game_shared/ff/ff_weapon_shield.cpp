//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_weapon_shield.cpp
//	@author Greg "GreenMushy" Stefanakis
//	@date Nov 28, 2010
//	@brief Prototype demoman shield weapon ( inactive weapon state )
//	===============================================

#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"
#include "in_buttons.h"


#ifdef CLIENT_DLL 
	#define CFFWeaponShield C_FFWeaponShield

	#include "c_ff_player.h"
	#include "ff_hud_chat.h"
#else
	#include "ff_player.h"
#endif

//The value that the player's speed% is reduced to when actively using the shield
ConVar ffdev_shield_speed( "ffdev_shield_speed", "0.3", FCVAR_REPLICATED | FCVAR_NOTIFY);
#define FF_SHIELD_SPEEDEFFECT ffdev_shield_speed.GetFloat()

//=============================================================================
// CFFWeaponShield
//=============================================================================

class CFFWeaponShield : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponShield, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponShield( void );
#ifdef CLIENT_DLL 
	~CFFWeaponShield( void ){ }
#endif

	void ShieldActive( void );	//Draw shield and start blocking
	void ShieldIdle( void );	//Holster shield and remove block
	virtual void ItemPostFrame( void );	//acts as the update function
	virtual bool Deploy();	//need this to override the base deploy() and put in an extra animation call
	virtual bool Holster(); // need this to override the base holster() and delete the shield in the world

	virtual FFWeaponID GetWeaponID( void ) const		{ return FF_WEAPON_SHIELD; }

	//Get the shield bool
	bool GetShieldActive(){ return m_bShieldActive; }

private:

	CFFWeaponShield(const CFFWeaponShield &);
	bool m_bShieldActive;//used to keep track of active shield to only call animations once per active/deactive

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
// CFFWeaponShield implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponShield::CFFWeaponShield( void ) 
{
	//Start shield holstered
	m_bShieldActive = false;
}

//-----------------------------------------------------------------------------
// Purpose: A modified ItemPostFrame to allow for different cycledecrements
//-----------------------------------------------------------------------------
void CFFWeaponShield::ItemPostFrame()
{
	//Get the owner
	CFFPlayer *pPlayer = GetPlayerOwner();

	//If there is no player gtfo
	if( !pPlayer )
		return;

	//-------------------------
	//Pressing Attack
	//-------------------------
	if ((pPlayer->m_nButtons & IN_ATTACK || pPlayer->m_afButtonPressed & IN_ATTACK) /*&& (pPlayer->GetFlags() & FL_ONGROUND)*/ )
	{
		ShieldActive();
	}

	// -----------------------
	//  No buttons down
	// -----------------------
	if (! ((pPlayer->m_nButtons & IN_ATTACK)))
	{
		ShieldIdle();
	}
}
//----------------------------------------------------------------------------
// Purpose: Override the deploy call and immediatly start holstering it so its not "active"to the player's viewmodel
//----------------------------------------------------------------------------
bool CFFWeaponShield::Deploy()
{
	//Start the shield out inactive
	m_bShieldActive = false;

#ifdef GAME_DLL
	//Get this player's last weapon
	CFFWeaponBase* pLastWeapon = ToFFPlayer(GetOwnerEntity())->GetLastFFWeapon();

	//If the weapon is valid
	if( pLastWeapon != NULL )
	{
		//If the last weapon was the deploy shield, call a different deploy animation
		if( pLastWeapon->GetWeaponID() == FF_WEAPON_DEPLOYSHIELD )
		{
			DevMsg( "Last Weapon was DeployShield!  Doing special anim(server).\n" );
			ToFFPlayer(GetOwnerEntity())->SetLastFFWeapon(NULL);
			//return SendWeaponAnim( ACT_VM_LOWERED_TO_IDLE );
			return DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), ACT_VM_LOWERED_TO_IDLE, (char*)GetAnimPrefix() );
		}
	}
#else
	//Get this player's last weapon
	CFFWeaponBase* pLastWeaponClient = ToFFPlayer(GetOwnerEntity())->GetLastFFWeaponClient();

	//If the weapon is valid
	if( pLastWeaponClient != NULL )
	{
		//If the last weapon was the deploy shield, call a different deploy animation
		if( pLastWeaponClient->GetWeaponID() == FF_WEAPON_DEPLOYSHIELD )
		{
			DevMsg( "Last Weapon was DeployShield!  Doing special anim(client).\n" );
			ToFFPlayer(GetOwnerEntity())->SetLastFFWeaponClient(NULL);
			//return SendWeaponAnim( ACT_VM_LOWERED_TO_IDLE );
			return DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), ACT_VM_LOWERED_TO_IDLE, (char*)GetAnimPrefix() );
		}
	}
#endif

	//Normal deploy if the last weapon wasnt a deployshield
	DevMsg( "Normal Deploy Called(shared)\n" );
	return BaseClass::Deploy();
}

//----------------------------------------------------------------------------
// Purpose: Override the holster call to delete the shield if there is one in the world
//----------------------------------------------------------------------------
bool CFFWeaponShield::Holster()
{
	//Set the shield to idle to do cleanup and reseting stuff
	//ShieldIdle();

	//Clear the last weapon so it will reset appropriatly
#ifdef CLIENT_DLL
	ToFFPlayer(GetOwnerEntity())->SetLastFFWeaponClient(NULL);
#else
	ToFFPlayer(GetOwnerEntity())->SetLastFFWeapon(NULL);
#endif

	//Now do normal holster
	return BaseClass::Holster();
}

//----------------------------------------------------------------------------
// Purpose: Handles whatever should be done when they fire(build, aim, etc) 
//----------------------------------------------------------------------------
void CFFWeaponShield::ShieldActive( void ) 
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
		pPlayer->SetRiotShieldActive(true);

		//If the effect is NOT on, then add it!
		if( pPlayer->IsSpeedEffectSet(SE_SHIELD) == false )
		{
			pPlayer->AddSpeedEffect(SE_SHIELD, 999, FF_SHIELD_SPEEDEFFECT, SEM_BOOLEAN);
		}
#endif

#ifdef CLIENT_DLL
		//Quickswap
		pPlayer->SwapToWeapon(FF_WEAPON_DEPLOYSHIELD);

		DevMsg("Swapping to active shield.\n");
#endif
	}
}
//----------------------------------------------------------------------------
// Purpose: Checks validity of ground at this point or whatever
//----------------------------------------------------------------------------
void CFFWeaponShield::ShieldIdle( void ) 
{
	//This should send an idle animation
	if ( HasWeaponIdleTimeElapsed() )
	{
#ifdef CLIENT_DLL
		DevMsg("Starting Idle Animation.\n");
#endif
		SendWeaponAnim( ACT_VM_IDLE );
	}
}