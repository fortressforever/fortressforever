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

// All our weapons (no longer static)
const char *s_WeaponAliasInfo[] = 
{
	"none", 			// FF_WEAPON_NONE

	"crowbar", 			// FF_WEAPON_CROWBAR
	"knife", 			// FF_WEAPON_KNIFE
	"medkit", 			// FF_WEAPON_MEDKIT
	"spanner", 			// FF_WEAPON_SPANNER
	"umbrella", 		// FF_WEAPON_UMBRELLA
	"flag", 			// FF_WEAPON_FLAG

	"shotgun", 			// FF_WEAPON_SHOTGUN
	"supershotgun", 	// FF_WEAPON_SUPERSHOTGUN

	"nailgun", 			// FF_WEAPON_NAILGUN
	"supernailgun", 	// FF_WEAPON_SUPERNAILGUN

	"grenadelauncher", 	// FF_WEAPON_GRENADELAUNCHER
	"pipelauncher", 	// FF_WEAPON_PIPELAUNCHER

	"autorifle", 		// FF_WEAPON_AUTORIFLE
	"sniperrifle", 		// FF_WEAPON_SNIPERRIFLE
	"radiotagrifle", 	// FF_WEAPON_RADIOTAGRIFLE

	"flamethrower", 	// FF_WEAPON_FLAMETHROWER, 
	"incendiarycannon", // FF_WEAPON_INCENDIARYCANNON

	"railgun", 			// FF_WEAPON_RAILGUN

	"tranquiliser", 	// FF_WEAPON_TRANQUILISER

	"assaultcannon", 	// FF_WEAPON_ASSAULTCANNON

	"rpg", 				// FF_WEAPON_RPG

	"deploydispenser", 	// FF_WEAPON_DEPLOYDISPENSER
	"deploysentrygun", 	// FF_WEAPON_DEPLOYSENTRYGUN
	"deploydetpack", 	// FF_WEAPON_DEPLOYDETPACK

	"tommygun",			// FF_WEAPON_TOMMYGUN

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
		{
			if (!Q_stricmp(s_WeaponAliasInfo[i], alias))
				return i;
		}
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
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CFFWeaponBase)
END_PREDICTION_DATA()

#ifdef GAME_DLL
	BEGIN_DATADESC(CFFWeaponBase)
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
	AddSolidFlags(FSOLID_TRIGGER); // Nothing collides with these but it gets touches.

	// All FF weapons fire underwater
	m_bFiresUnderwater = true; 

	m_flNextBuildKill = 0.0f;
}

//----------------------------------------------------------------------------
// Purpose: All weapons use the same silence sound
//----------------------------------------------------------------------------
void CFFWeaponBase::WeaponSound(WeaponSound_t sound_type, float soundtime /* = 0.0f */)
{
#ifdef CLIENT_DLL

	// If we have some sounds from the weapon classname.txt file, play a random one of them
	const char *shootsound = GetWpnData().aShootSounds[sound_type]; 
	if (!shootsound || !shootsound[0])
		return;

	CBroadcastRecipientFilter filter; // this is client side only
	if (!te->CanPredict())
		return;

	CBaseEntity::EmitSound(filter, GetPlayerOwner()->entindex(), shootsound, &GetPlayerOwner()->GetAbsOrigin()); 
#else
	BaseClass::WeaponSound(sound_type, soundtime);
#endif
}

//----------------------------------------------------------------------------
// Purpose: Get CFFPlayer owner
//----------------------------------------------------------------------------
CFFPlayer * CFFWeaponBase::GetPlayerOwner() const
{
	return dynamic_cast<CFFPlayer *> (GetOwner());
}

const char *CFFWeaponBase::GetWorldModel( void ) const
{
#ifdef CLIENT_DLL
	CFFPlayer *pFFPlayer = GetPlayerOwner();
	if(pFFPlayer && pFFPlayer->IsDisguised())
	{
		CFFWeaponBase *pWeapon = pFFPlayer->GetActiveFFWeapon();

		if(!pWeapon)
			return BaseClass::GetWorldModel();

		int iSlot = pWeapon->GetFFWpnData().iSlot;
		int iClass = pFFPlayer->GetDisguisedClass();

		if(pWeapon == this)
		{
			if(pFFPlayer->m_DisguisedWeapons[iClass].szWeaponModel[iSlot][0] != NULL)
				return pFFPlayer->m_DisguisedWeapons[iClass].szWeaponModel[iSlot];
		}
	}
#endif
	return BaseClass::GetWorldModel();
}

int CFFWeaponBase::GetWorldModelIndex( void )
{
	CFFPlayer *pFFPlayer = GetPlayerOwner();
	if(pFFPlayer && pFFPlayer->IsDisguised())
		return modelinfo->GetModelIndex(GetWorldModel()); 
	else
#ifdef CLIENT_DLL
		return BaseClass::GetWorldModelIndex();
#else	//server should never use this code!
		Assert( false );
#endif
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Get script file weapon data
//----------------------------------------------------------------------------
const CFFWeaponInfo &CFFWeaponBase::GetFFWpnData() const
{
	const FileWeaponInfo_t *pWeaponInfo = &GetWpnData();
	const CFFWeaponInfo *pFFInfo;

	#ifdef _DEBUG
		pFFInfo = dynamic_cast<const CFFWeaponInfo *> (pWeaponInfo);
		Assert(pFFInfo);
	#else
		pFFInfo = static_cast<const CFFWeaponInfo *> (pWeaponInfo);
	#endif

	return *pFFInfo;
}

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

//----------------------------------------------------------------------------
// Purpose: Stop selection of weapons while building
//----------------------------------------------------------------------------
bool CFFWeaponBase::CanBeSelected()
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	if (pPlayer->m_bBuilding)
		return false;
	else
		return BaseClass::CanBeSelected();
}

//----------------------------------------------------------------------------
// Purpose: Do the weapon's recoil
//----------------------------------------------------------------------------
void CFFWeaponBase::WeaponRecoil()
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	QAngle angPunch = pPlayer->GetPunchAngle();
	angPunch.x += GetFFWpnData().m_flRecoilAmount;

	pPlayer->ViewPunch(angPunch);
}

//----------------------------------------------------------------------------
// Purpose: Return a weapon's death notice name
//----------------------------------------------------------------------------
char *CFFWeaponBase::GetDeathNoticeName()
{
#ifdef GAME_DLL
	return (char *) STRING(m_iClassname);
#else
	return "GetDeathNoticeName not implemented on client yet";
#endif
}

#ifdef CLIENT_DLL

//----------------------------------------------------------------------------
// Purpose: From HL2MP
//----------------------------------------------------------------------------
void CFFWeaponBase::OnDataChanged(DataUpdateType_t type)
{
	BaseClass::OnDataChanged(type);

	if (GetPredictable() && !ShouldPredict())
		ShutdownPredictable();
}


//----------------------------------------------------------------------------
// Purpose: From HL2MP
//----------------------------------------------------------------------------
bool CFFWeaponBase::ShouldPredict()
{
	if (GetOwner() && GetOwner() == C_BasePlayer::GetLocalPlayer())
		return true;

	return BaseClass::ShouldPredict();
}

#endif

//-----------------------------------------------------------------------------
// Purpose: Primary fire button attack for non-clip weapons
//			For clip weapons, see ff_weapon_baseclip.cpp
//-----------------------------------------------------------------------------
void CFFWeaponBase::PrimaryAttack()
{
	// Only the player fires this way so we can cast
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	if (!pPlayer)
		return;

	// Undisguise
#ifdef GAME_DLL
	pPlayer->ResetDisguise();
#endif

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound(SINGLE);

	pPlayer->DoMuzzleFlash();

	SendWeaponAnim(GetPrimaryAttackActivity());

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifdef GAME_DLL
	int nShots = min(GetFFWpnData().m_iCycleDecrement, pPlayer->GetAmmoCount(m_iPrimaryAmmoType));
	pPlayer->RemoveAmmo(nShots, m_iPrimaryAmmoType);
#endif

	// Fire now
	Fire();

	m_flNextPrimaryAttack = gpGlobals->curtime + GetFFWpnData().m_flCycleTime;

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}

	//Add our view kick in
	pPlayer->ViewPunch(QAngle(-GetFFWpnData().m_flRecoilAmount, 0, 0));
}
