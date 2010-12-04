//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_weapon_shield.cpp
//	@author Greg "GreenMushy" Stefanakis
//	@date Nov 28, 2010
//	@brief Prototype demoman shield weapon
//	===============================================

//Known bugs:
//Switching weapons while your shield is active leaves the shield in the world detatched from the player!
//(its kindof fun though)

//Future TODOs:
//Disallow players from using the shield in the air
//im allowing it for now because i dont know how to fully get it working

//Add in CVars so the shield can be alligned differently.  Right now you can only alter the height off the ground
//i secretly think the shield is not perfectly centered and is too far from the player

#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"
#include "in_buttons.h"


#ifdef CLIENT_DLL 
	#define CFFWeaponShield C_FFWeaponShield
	#define CFFShield C_FFShield

	#include "c_ff_player.h"
	#include "ff_hud_chat.h"
#else
	#include "ff_player.h"
#endif

#include "ff_buildableobjects_shared.h"

//The value that the player's speed% is reduced to when actively using the shield
ConVar ffdev_shield_speed( "ffdev_shield_speed", "0.2", FCVAR_REPLICATED | FCVAR_NOTIFY);
#define FF_SHIELD_SPEEDEFFECT ffdev_shield_speed.GetFloat()

//How high off the ground the shield should be
ConVar ffdev_shield_height( "ffdev_shield_height", "16", FCVAR_REPLICATED | FCVAR_NOTIFY);
#define SHIELD_HEIGHT ffdev_shield_height.GetInt()

//Show the "buildable" object that is the physical shield in front of the player
ConVar ffdev_shield_showmodel( "ffdev_shield_showmodel", "0", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define SHIELD_SHOWMODEL ffdev_shield_showmodel.GetBool()

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

	virtual FFWeaponID GetWeaponID( void ) const		{ return FF_WEAPON_SHIELD; }

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

	//If the player is currently owning a shield*, then update its position with this player
	if( pPlayer->GetShield() )
	{
		//Set the location and angle
		pPlayer->GetShield()->SetAbsOrigin( pPlayer->GetAbsOrigin() + Vector(0, 0, SHIELD_HEIGHT));
		pPlayer->GetShield()->SetAbsAngles( pPlayer->GetAbsAngles() );
	}

	//-------------------------
	//Pressing Attack
	//-------------------------
	if ((pPlayer->m_nButtons & IN_ATTACK || pPlayer->m_afButtonPressed & IN_ATTACK) )
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

//Override the deploy call and immediatly start holstering it so its not "active"to the player's viewmodel
bool CFFWeaponShield::Deploy()
{
	BaseClass::Deploy();
	//just return true i guess
	return SendWeaponAnim( ACT_VM_HOLSTER );
}
//----------------------------------------------------------------------------
// Purpose: Handles whatever should be done when they fire(build, aim, etc) 
//----------------------------------------------------------------------------
void CFFWeaponShield::ShieldActive( void ) 
{
	//Get the owner of this weapon
	CFFPlayer *pPlayer = GetPlayerOwner();

	//If no player gtfo
	if( pPlayer == NULL )
		return;

	//Check so it only does this once per click
	if( m_bShieldActive == false )
	{
		m_bShieldActive = true;

		//The weapon anim uses an anim extension that it gets from the weapon script.
		//In the case of the shield, it is defaulted as "crowbar" and the player holds it as such
		//To change it i have to change the animation extension in the weapon data and im doing it manually
		//then calling the weapon animation
		//Im not sure if there is an actual function to do this??
		char * szAnimExt = "railgun";
		Q_strncpy( (char*)(GetFFWpnData().m_szAnimExtension), szAnimExt, sizeof( GetFFWpnData().m_szAnimExtension ) );

		// Send animation
		SendWeaponAnim( ACT_VM_DRAW );

#ifdef CLIENT_DLL
		DevMsg("Shield Active\n");
#endif
	}

#ifdef GAME_DLL

	//If the effect is NOT on, then add it!
	if( pPlayer->IsSpeedEffectSet(SE_SHIELD) == false )
	{
		pPlayer->AddSpeedEffect(SE_SHIELD, 0.1f, FF_SHIELD_SPEEDEFFECT, SEM_BOOLEAN);
	}

	//If the player currently has no shield
	if( pPlayer->GetShield() == NULL )
	{
		//Create the shield in the world
		CFFShield *pShield = (CFFShield *)CBaseEntity::Create( "FF_Shield", pPlayer->GetAbsOrigin() + Vector(0, 0, SHIELD_HEIGHT ), pPlayer->GetAbsAngles(), pPlayer);

		//Check if the world model should show or not-GreenMushy
		if( SHIELD_SHOWMODEL == 0 )
		{
			//I guess this makes it visible or not
			pShield->AddEffects( EF_NODRAW );
		}

		//Not sure if this helps make it solid and use the collision group
		pShield->VPhysicsInitNormal( SOLID_VPHYSICS, pShield->GetSolidFlags(), true );

		//I suppose this spawns the object?
		pShield->Spawn();

		//Set this shield to the buildable collision group so it blocks nearly everything
		pShield->SetCollisionGroup( COLLISION_GROUP_BUILDABLE );

		//Set the shield pointer in player to this newly created object, use GetShield() to find it again
		pPlayer->SetShield( pShield );
	}
#endif
}
//----------------------------------------------------------------------------
// Purpose: Checks validity of ground at this point or whatever
//----------------------------------------------------------------------------
void CFFWeaponShield::ShieldIdle( void ) 
{
	//So this only fires once per click
	if( m_bShieldActive == true )
	{
		m_bShieldActive = false;
		
		//The weapon anim uses an anim extension that it gets from the weapon script.
		//In the case of the shield, it is defaulted as "crowbar" and the player holds it as such
		//To change it i have to change the animation extension in the weapon data and im doing it manually
		//then calling the weapon animation
		//Im not sure if there is an actual function to do this??
		char * szAnimExt = "crowbar";
		Q_strncpy( (char*)(GetFFWpnData().m_szAnimExtension), szAnimExt, sizeof( GetFFWpnData().m_szAnimExtension ) );

		// Send animation
		SendWeaponAnim( ACT_VM_HOLSTER );

#ifdef CLIENT_DLL
		DevMsg("Shield Idle\n");
#endif
	}

#ifdef GAME_DLL
	//Get the owner of this shield
	CFFPlayer *pPlayer = GetPlayerOwner();

	//If there is no player, gtfo
	if( pPlayer == NULL )
		return;

	//If the player is effected by the shield slow, remove it
	if( pPlayer->IsSpeedEffectSet(SE_SHIELD) == true )
	{
		//Remove the effect that was put on earlier
		pPlayer->RemoveSpeedEffect(SE_SHIELD);
	}

	//If the player has a valid shield pointer
	if( pPlayer->GetShield() )
	{
		// Remove entity from game 
		UTIL_Remove( pPlayer->GetShield() );
		//Reset the shield to null for the future
		pPlayer->SetShield(NULL);
	}
#endif
}