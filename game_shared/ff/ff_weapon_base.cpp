/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_base.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December ##, 2004
/// @brief All weapons derived from here
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation


#include "cbase.h"
#include "in_buttons.h"
#include "takedamageinfo.h"
#include "ff_weapon_base.h"
#include "ammodef.h"
#include "ff_buildableobjects_shared.h"
#include "ff_utils.h"

#ifdef CLIENT_DLL 
	#include "c_ff_player.h"
#else
	#include "ff_player.h"
	#include "eventqueue.h"
#endif

#ifdef CLIENT_DLL
	static ConVar auto_reload("cl_autoreload", "0", 0, "Automatic weapon reload");
#endif

static const char * s_WeaponAliasInfo[] = 
{
	"none", 				// FF_WEAPON_NONE

	"crowbar", 			// FF_WEAPON_CROWBAR
	"knife", 			// FF_WEAPON_KNIFE
	"medkit", 			// FF_WEAPON_MEDKIT
	"spanner", 			// FF_WEAPON_SPANNER
	"umbrella", 			// FF_WEAPON_UMBRELLA
	"flag", 				// FF_WEAPON_FLAG

	"shotgun", 			// FF_WEAPON_SHOTGUN
	"supershotgun", 		// FF_WEAPON_SUPERSHOTGUN

	"nailgun", 			// FF_WEAPON_NAILGUN
	"supernailgun", 		// FF_WEAPON_SUPERNAILGUN

	"grenadelauncher", 	// FF_WEAPON_GRENADELAUNCHER
	"pipelauncher", 		// FF_WEAPON_PIPELAUNCHER

	"autorifle", 		// FF_WEAPON_AUTORIFLE
	"sniperrifle", 		// FF_WEAPON_SNIPERRIFLE
	"radiotagrifle", 	// FF_WEAPON_RADIOTAGRIFLE

	"flamethrower", 		// FF_WEAPON_FLAMETHROWER, 
	"incendiarycannon", 	// FF_WEAPON_INCENDIARYCANNON

	"railgun", 			// FF_WEAPON_RAILGUN

	"tranquiliser", 		// FF_WEAPON_TRANQUILISER

	"assaultcannon", 	// FF_WEAPON_ASSAULTCANNON

	"rpg", 				// FF_WEAPON_RPG

	"deploydispenser", 	// FF_WEAPON_DEPLOYDISPENSER
	"deploysentrygun", 	// FF_WEAPON_DEPLOYSENTRYGUN
	"deploydetpack", 	// FF_WEAPON_DEPLOYDETPACK

	NULL, 				// FF_WEAPON_MAX
};

//=============================================================================
// Global functions
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Given an alias, return the associated weapon ID
//----------------------------------------------------------------------------
int AliasToWeaponID(const char *alias) 
{
	if (alias) 
	{
		for (int i=0; s_WeaponAliasInfo[i] != NULL; ++i) 
			if (!Q_stricmp(s_WeaponAliasInfo[i], alias)) 
				return i;
	}

	return FF_WEAPON_NONE;
}

//----------------------------------------------------------------------------
// Purpose: Given a weapon ID, return its alias
//----------------------------------------------------------------------------
const char *WeaponIDToAlias(int id) 
{
	if ((id >= FF_WEAPON_MAX) || (id < 0)) 
		return NULL;

	return s_WeaponAliasInfo[id];
}

//=============================================================================
// CFFWeaponBase tables.
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponBase, DT_FFWeaponBase) 

BEGIN_NETWORK_TABLE(CFFWeaponBase, DT_FFWeaponBase) 
#ifdef CLIENT_DLL
	RecvPropInt(RECVINFO(m_fInSpecialReload)) 
#else
	// world weapon models have no animations
  	
	// Jerky anim fix
	//SendPropExclude("DT_AnimTimeMustBeFirst", "m_flAnimTime"), 
	//SendPropExclude("DT_BaseAnimating", "m_nSequence"), 

	SendPropInt(SENDINFO(m_fInSpecialReload), 2, SPROP_UNSIGNED) 
#endif
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponBase) 
	// Jerky anim fix	
	//DEFINE_PRED_FIELD(m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK), 
END_PREDICTION_DATA() 

#ifdef GAME_DLL
	BEGIN_DATADESC(CFFWeaponBase) 
		// New weapon Think and Touch Functions go here..
	END_DATADESC() 
#endif

LINK_ENTITY_TO_CLASS(weapon_ff_base, CFFWeaponBase);

//=============================================================================
// CFFWeaponCSBase implementation. 
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponBase::CFFWeaponBase() 
{
	SetPredictionEligible(true);

	// All FF weapons fire underwater
	m_bFiresUnderwater = true; 

	AddSolidFlags(FSOLID_TRIGGER); // Nothing collides with these but it gets touches.

	m_flNextBuildKill = 0.0f;
}

//----------------------------------------------------------------------------
// Purpose: Get script file weapon data
//----------------------------------------------------------------------------
const CFFWeaponInfo &CFFWeaponBase::GetFFWpnData() const
{
	const FileWeaponInfo_t *pWeaponInfo = &GetWpnData();
	const CFFWeaponInfo *pFFInfo;

	#ifdef _DEBUG
		pFFInfo = dynamic_cast< const CFFWeaponInfo * > (pWeaponInfo);
		Assert(pFFInfo);
	#else
		pFFInfo = static_cast< const CFFWeaponInfo * > (pWeaponInfo);
	#endif

	return *pFFInfo;
}

//----------------------------------------------------------------------------
// Purpose: Play an empty clip sound for a weapon
//----------------------------------------------------------------------------
bool CFFWeaponBase::PlayEmptySound() 
{
	CPASAttenuationFilter filter(this);
	filter.UsePredictionRules();

	EmitSound(filter, entindex(), /*"Default.ClipEmpty_Rifle"*/ "Generic.Empty");
	
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Return pointer to weapon's owner
//----------------------------------------------------------------------------
CFFPlayer * CFFWeaponBase::GetPlayerOwner() const
{
	return dynamic_cast< CFFPlayer * > (GetOwner());
}

//----------------------------------------------------------------------------
// Purpose: Weapon's primary attack, reduce ammo / check for empty clip
//			Call Fire() if able to fire
//----------------------------------------------------------------------------
void CFFWeaponBase::PrimaryAttack() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( pPlayer->m_bBuilding )
	{
#ifdef GAME_DLL
		DevMsg( "[Server] PrimaryAttack :: Player is building!\n" );
#else
		DevMsg( "[Client] PrimaryAttack :: Player is building!\n" );
#endif

		if( m_flNextBuildKill < gpGlobals->curtime )
		{
			m_flNextBuildKill = gpGlobals->curtime + 0.5f;

#ifdef GAME_DLL
			switch( pPlayer->m_iCurBuild )
			{
				case FF_BUILD_DETPACK: pPlayer->Command_BuildDetpack(); break;
				case FF_BUILD_DISPENSER: pPlayer->Command_BuildDispenser(); break;
				case FF_BUILD_SENTRYGUN: pPlayer->Command_BuildSentryGun(); break;
			}
#endif
		}

		return;
	}

	pPlayer->m_iShotsFired++;

	// Reset disguise
#ifdef GAME_DLL
	if (pPlayer->IsDisguised())
		pPlayer->ResetDisguise();
#endif

	// Out of ammo?
	if (m_iClip1 <= 0) 
	{
		if (m_iClip1 == 0) 
		{
			Reload();
			PlayEmptySound();
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else if (pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0) 
		{
			// An iClip1 of -1 means that it's not a clip-based weapon
			// So now return if they have no ammo at all
			return;
		}
		
	}

	SendWeaponAnim(ACT_VM_PRIMARYATTACK);

	// Do this server side so there's no mismatch
#ifdef GAME_DLL
	if (m_iClip1 < 0) 
	{
		// Not a clip based weapon, so remove from primary ammo location
		pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);
	}
	else
	{
		// Remove from clip
		m_iClip1 -= pWeaponInfo.m_iCycleDecrement;
	}
#endif

	// Effects:
	pPlayer->SetAnimation(PLAYER_ATTACK1);
	pPlayer->DoMuzzleFlash();

#ifdef CLIENT_DLL
	pPlayer->m_PlayerAnimState->DoAnimationEvent(PLAYERANIMEVENT_FIRE_GUN_PRIMARY);
#endif

	// Cancel any reload
	m_fInSpecialReload = 0;

	// Now do our actual projectile firing code
	Fire();

	if (m_iClip1 <= 0 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0) 
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
	}

	// More effects:
	WeaponSound(SINGLE);
	WeaponRecoil();

	m_flNextPrimaryAttack = gpGlobals->curtime + pWeaponInfo.m_flCycleTime;

	// how long till we start being idle
	if (m_iClip1 != 0 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) != 0) 
		SetWeaponIdleTime(gpGlobals->curtime + /*5.0f */ 0.75f);
	else
		SetWeaponIdleTime(gpGlobals->curtime + 0.75f);
}

//----------------------------------------------------------------------------
// Purpose: Warn if weapon fire function is not implemented
//----------------------------------------------------------------------------
void CFFWeaponBase::Fire() 
{
	// Shouldnt get here!
	Assert(0 && "Weapon is missing a Fire() implementation!\n");
}

//----------------------------------------------------------------------------
// Purpose: Weapon has just been deployed(ie. unholstered) 
//----------------------------------------------------------------------------
bool CFFWeaponBase::Deploy() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();

#ifdef GAME_DLL
	if (pPlayer->IsDisguised())
	{
		// Spies show different models!
		PLAYERCLASS_FILE_INFO_HANDLE classinfo;

		if (ReadPlayerClassDataFromFileForSlot(filesystem, Class_IntToString(pPlayer->GetDisguisedClass()), &classinfo, GetEncryptionKey()))
		{
			const CFFPlayerClassInfo *pPlayerClassInfo = GetFilePlayerClassInfoFromHandle(classinfo);

			if (pPlayerClassInfo)
			{
				const char *DisguiseWeapon = NULL;

				for (int i = 0; i < pPlayerClassInfo->m_iNumWeapons; i++)
				{
					WEAPON_FILE_INFO_HANDLE weaponinfo;

					if (!ReadWeaponDataFromFileForSlot(filesystem, pPlayerClassInfo->m_aWeapons[i], &weaponinfo, GetEncryptionKey()))
						continue;

					const FileWeaponInfo_t *pWeaponInfo = GetFileWeaponInfoFromHandle(weaponinfo);

					if (pWeaponInfo && pWeaponInfo->iSlot <= GetSlot())
						DisguiseWeapon = pPlayerClassInfo->m_aWeapons[i];
				}

				if (DisguiseWeapon)
				{
					PrecacheModel(DisguiseWeapon);	// Just in case
					SetModel(DisguiseWeapon);
					DevMsg("Disguising weapon as %s (%d)\n", DisguiseWeapon, -1);
				}
			}
		}
	}
#endif

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( pPlayer->m_bBuilding )
	{
#ifdef GAME_DLL
		DevMsg( "[Server]" );
#else
		DevMsg( "[Client]" );
#endif
		DevMsg( " Deploy :: Can't deploy when a player is building!\n" );

		return false;
	}

	pPlayer->m_iShotsFired = 0;

	m_fInSpecialReload = 0;

#ifdef CLIENT_DLL
	m_flNextReloadAttempt = 0;
#endif

	return BaseClass::Deploy();
}

//----------------------------------------------------------------------------
// Purpose: Default TFC style reloading
//----------------------------------------------------------------------------
bool CFFWeaponBase::Reload() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	// Cancel the reload now
#ifdef CLIENT_DLL
	engine->ClientCmd("-reload");
#endif

	// Is this necessary? Player could be reloading shotgun then
	// start building but the build holsters the weapon so... (?)
	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( pPlayer->m_bBuilding )
	{
#ifdef GAME_DLL
		DevMsg( "[Server]" );
#else
		DevMsg( "[Client]" );
#endif
		DevMsg( " Deploy :: Can't reload while building!\n" );

		return false;
	}

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 || m_iClip1 == GetMaxClip1() || m_iClip1 < 0) 
		return true;

	// don't reload until recoil is done
	if (m_flNextPrimaryAttack > gpGlobals->curtime) 
		return true;
		
	// check to see if we're ready to reload
	if (m_fInSpecialReload == 0) 
	{
		// animations
		pPlayer->SetAnimation(PLAYER_RELOAD);
		SendWeaponAnim(ACT_SHOTGUN_RELOAD_START);

		// change to start-reload stage
		m_fInSpecialReload = 1;

		// time till we're ready to start the next sequence
		SetWeaponIdleTime(gpGlobals->curtime + pWeaponInfo.m_flPreReloadTime);

		return true;
	}
	// we've started reloading sequence
	else if (m_fInSpecialReload == 1) 
	{
		// has sequence not finished yet
		if (m_flTimeWeaponIdle > gpGlobals->curtime) 
			return true;

		// change to the adding ammo stage
		m_fInSpecialReload = 2;

		// anims
		SendWeaponAnim(ACT_VM_RELOAD);
		WeaponSound(RELOAD);

		SetWeaponIdleTime(gpGlobals->curtime + pWeaponInfo.m_flReloadTime);
	}
	else
	{
		// has sequence not finished yet(Bug #0000145: First rocket in the RPG reload cycle is mis-timed) 
		if (m_flTimeWeaponIdle > gpGlobals->curtime) 
			return true;

#ifdef GAME_DLL
		SendReloadEvents();
#endif
		
		//WeaponSound(RELOAD);		

#ifdef GAME_DLL
		CFFPlayer *pPlayer = GetPlayerOwner();

		// phew finally, remove ammo from stores...
		if (pPlayer) 
			 pPlayer->RemoveAmmo(pWeaponInfo.m_iCycleDecrement, m_iPrimaryAmmoType);

		// ...and add it to the clip
		m_iClip1 += pWeaponInfo.m_iCycleDecrement;
#endif

		// go back to the previous stage of the sequence so it triggers this again
		m_fInSpecialReload = 1;
	}

	return true;
}

//----------------------------------------------------------------------------
// Purpose: Carry on reload sequence or play an idle sequence, depending on state
//----------------------------------------------------------------------------
void CFFWeaponBase::WeaponIdle() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

#ifdef CLIENT_DLL
	// A reloadable weapon with ammo to reload with
	if (!m_fInSpecialReload && m_flNextReloadAttempt < gpGlobals->curtime && m_iClip1 < GetMaxClip1() && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) > 0 && m_iClip1 >= 0) 
	{
		// Auto reloading, a rather hacky way perhaps
		if (auto_reload.GetInt() != 0) 
			engine->ClientCmd("+reload");

		m_flNextReloadAttempt = gpGlobals->curtime + 0.1f;
	}
#endif

	if (m_flTimeWeaponIdle < gpGlobals->curtime) 
	{
		if (m_iClip1 == 0 && m_fInSpecialReload == 0 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType)) 
		{
			Reload();
		}
		else if (m_fInSpecialReload != 0) 
		{
			if (m_iClip1 != pWeaponInfo.iMaxClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType)) 
			{
				Reload();
			}
			else
			{
				// reload debounce has timed out
				SendWeaponAnim(ACT_SHOTGUN_RELOAD_FINISH);
				m_fInSpecialReload = 0;

				WeaponSound(COCK);

				SetWeaponIdleTime(gpGlobals->curtime + 1.5);
			}
		}
		else
		{
			SendWeaponAnim(ACT_VM_IDLE);
		}
	}
}

//----------------------------------------------------------------------------
// Purpose: Jump the view a bit according to the script file
//----------------------------------------------------------------------------
void CFFWeaponBase::WeaponRecoil() 
{
#ifdef GAME_DLL
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	// Update punch angles
	QAngle angle = pPlayer->GetPunchAngle();
	angle.x -= pWeaponInfo.m_flRecoilAmount;
	pPlayer->SetPunchAngle(angle);
#endif
}

#ifdef GAME_DLL

	//----------------------------------------------------------------------------
	// Purpose: Tell other entities to play reloading animation on this player
	//----------------------------------------------------------------------------
	void CFFWeaponBase::SendReloadEvents() 
	{
		CFFPlayer *pPlayer = dynamic_cast< CFFPlayer * > (GetOwner());
		if (!pPlayer) 
			return;

		// Send a message to any clients that have this entity to play the reload.
		CPASFilter filter(pPlayer->GetAbsOrigin());
		filter.RemoveRecipient(pPlayer);

		/*UserMessageBegin(filter, "ReloadEffect");
		WRITE_SHORT(pPlayer->entindex());
		MessageEnd();*/

		// Make the player play his reload animation.
		// ted - temporarily disabled
		// pPlayer->DoAnimationEvent(PLAYERANIMEVENT_RELOAD);
	}

#endif


//-----------------------------------------------------------------------------
// Purpose: OVERRIDDEN from base sdk. Changed SequenceLength to MAX_DEPLOY_TIME for purposes of the first firing
// Input  : *szViewModel - 
//			*szWeaponModel - 
//			iActivity - 
//			*szAnimExt - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CFFWeaponBase::DefaultDeploy(char *szViewModel, char *szWeaponModel, int iActivity, char *szAnimExt) 
{
	// Msg("deploy %s at %f\n", GetClassname(), gpGlobals->curtime);

	// Weapons that don't autoswitch away when they run out of ammo 
	// can still be deployed when they have no ammo.
	//if (!HasAnyAmmo() && AllowsAutoSwitchFrom()) 
	//	return false;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner) 
	{
		// Dead men deploy no weapons
		if (pOwner->IsAlive() == false) 
			return false;

		pOwner->SetAnimationExtension(szAnimExt);

		SetViewModel();
		SendWeaponAnim(iActivity);

		pOwner->SetNextAttack(gpGlobals->curtime + MAX_DEPLOY_TIME);
	}

	// Can't shoot again until we've finished deploying
	m_flNextPrimaryAttack	= gpGlobals->curtime + MAX_DEPLOY_TIME;

	SetWeaponVisible(true);

#ifndef CLIENT_DLL
	// Cancel any pending hide events
	g_EventQueue.CancelEventOn(this, "HideWeapon");
#endif

	return true;
}

// Bug #0000333: Buildable Behavior (non build slot) while building
bool CFFWeaponBase::CanBeSelected( void )
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	if( pPlayer->m_bBuilding )
		return false;
	else
		return BaseClass::CanBeSelected();
}

char *CFFWeaponBase::GetDeathNoticeName()
{
#if !defined( CLIENT_DLL )
	return (char *) STRING(m_iClassname);
#else
	return "GetDeathNoticeName not implemented on client yet";
#endif
}

// Jerky anim fix
#ifdef CLIENT_DLL

void CFFWeaponBase::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( GetPredictable() && !ShouldPredict() )
		ShutdownPredictable();
}


bool CFFWeaponBase::ShouldPredict()
{
	if ( GetOwner() && GetOwner() == C_BasePlayer::GetLocalPlayer() )
		return true;

	return BaseClass::ShouldPredict();
}

#endif