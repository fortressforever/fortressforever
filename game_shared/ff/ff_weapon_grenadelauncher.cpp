/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_grenadelauncher.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF grenade launcher code & class declaration.
///
/// REVISIONS
/// ---------
/// Dec 24, 2004 Mirv: First created
/// Jan 16, 2005 Mirv: Moved all repeated code to base class


#include "cbase.h"
#include "ff_weapon_baseclip.h"
#include "ff_projectile_grenade.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponGrenadeLauncher C_FFWeaponGrenadeLauncher
	#include "c_ff_player.h"
	#include "ff_utils.h"
#else
	#include "omnibot_interface.h"
	#include "ff_player.h"
#endif

ConVar ffdev_bluepipes_syncwithyellows("ffdev_bluepipes_syncwithyellows", "0.0", FCVAR_REPLICATED, "Whether blues and yellows share clip");
#define FFDEV_BLUEPIPES_SYNCWITHYELLOWS ffdev_bluepipes_syncwithyellows.GetBool()


//=============================================================================
// CFFWeaponGrenadeLauncher
//=============================================================================

class CFFWeaponGrenadeLauncher : public CFFWeaponBaseClip
{
public:
	DECLARE_CLASS(CFFWeaponGrenadeLauncher, CFFWeaponBaseClip);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponGrenadeLauncher();

	virtual void		Fire();
	virtual bool		Reload();
	virtual bool		SendWeaponAnim(int iActivity);
	virtual bool		Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual bool		Deploy();

	void				Synchronise();

	virtual FFWeaponID	GetWeaponID() const	{ return FF_WEAPON_GRENADELAUNCHER; }

private:
	CFFWeaponGrenadeLauncher(const CFFWeaponGrenadeLauncher &);
};

//=============================================================================
// CFFWeaponGrenadeLauncher tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponGrenadeLauncher, DT_FFWeaponGrenadeLauncher) 

BEGIN_NETWORK_TABLE(CFFWeaponGrenadeLauncher, DT_FFWeaponGrenadeLauncher) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponGrenadeLauncher) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_grenadelauncher, CFFWeaponGrenadeLauncher);
PRECACHE_WEAPON_REGISTER(ff_weapon_grenadelauncher);

//=============================================================================
// CFFWeaponGrenadeLauncher implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponGrenadeLauncher::CFFWeaponGrenadeLauncher() 
{
	m_fIsSwitching = false;
}

//----------------------------------------------------------------------------
// Purpose: Fires a grenade
//----------------------------------------------------------------------------
void CFFWeaponGrenadeLauncher::Fire() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	Vector	vForward, vRight, vUp;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);

	Vector	vecSrc = pPlayer->Weapon_ShootPosition() + vForward * 16.0f + vRight * 7.0f + vUp * -8.0f;

	// Bug #0000192: Demoman can stick pipes to walls
	// Mirv got a better idea?
	trace_t tr;
	UTIL_TraceLine( vecSrc, vecSrc + ( vForward * 32 ), MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
	if( tr.startsolid )
		vecSrc += ( vForward * -16.0f );
	else if( tr.fraction != 1.0f )
		vecSrc += ( vForward * -24.0f );

	CFFProjectileGrenade *pGrenade = CFFProjectileGrenade::CreateGrenade(this, vecSrc, pPlayer->EyeAngles() - QAngle(12.0f, 0.0f, 0.0f), pPlayer, pWeaponInfo.m_iDamage, pWeaponInfo.m_iDamageRadius, pWeaponInfo.m_iSpeed);
	pGrenade;

#ifdef GAME_DLL
	Omnibot::Notify_PlayerShoot(pPlayer, Omnibot::TF_WP_GRENADE_LAUNCHER, pGrenade);
#endif

	// Synchronise with pipelauncher
	Synchronise();
}

//----------------------------------------------------------------------------
// Purpose: Keep ammo acounts the same
//----------------------------------------------------------------------------
bool CFFWeaponGrenadeLauncher::Reload() 
{
	bool b = BaseClass::Reload();

	// Synchronise with pipelauncher
	Synchronise();

	return b;
}

//----------------------------------------------------------------------------
// Purpose: Override animations
//----------------------------------------------------------------------------
bool CFFWeaponGrenadeLauncher::SendWeaponAnim(int iActivity) 
{
	// If we have some unexpected clip amount, escape quick
	if (m_iClip1 < 0 || m_iClip1 > 6) 
		return BaseClass::SendWeaponAnim(iActivity);

	// Override the animation with a specific one
	switch (iActivity) 
	{
	case ACT_VM_DRAW:
		if (m_fIsSwitching)
			iActivity = ACT_VM_SWITCHDRAW_WITH0 + m_iClip1;
		else
			iActivity = ACT_VM_DRAW_WITH0 + m_iClip1;
		break;

	case ACT_VM_IDLE:
		iActivity = ACT_VM_IDLE_WITH0 + m_iClip1;
		break;

	case ACT_VM_PRIMARYATTACK:
		iActivity = ACT_VM_PRIMARYATTACK_1TO0 + (m_iClip1 - 1);
		break;

	case ACT_VM_RELOAD:
		iActivity = ACT_VM_RELOAD_0TO1 + m_iClip1;
		break;

	case ACT_SHOTGUN_RELOAD_START:
		iActivity = ACT_VM_STARTRELOAD_WITH0 + m_iClip1;
		break;

	case ACT_SHOTGUN_RELOAD_FINISH:
		iActivity = ACT_VM_FINISHRELOAD_WITH1 + (m_iClip1 - 1);
		break;
	}

	// No more switching mode animation
	m_fIsSwitching = false;

	return BaseClass::SendWeaponAnim(iActivity);
}

//----------------------------------------------------------------------------
// Purpose: Play a 'switch' animation when we swap to the grenade launcher
//----------------------------------------------------------------------------
bool CFFWeaponGrenadeLauncher::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	// Let the other weapon know it's animation type
	CFFWeaponBaseClip *pWeapon = dynamic_cast<CFFWeaponBaseClip *> (pSwitchingTo);
	
	if (pWeapon && pWeapon->GetWeaponID() == FF_WEAPON_PIPELAUNCHER)
		pWeapon->m_fIsSwitching = true;

	// Synchronise with pipelauncher
	Synchronise();

	return BaseClass::Holster(pSwitchingTo);
}

//----------------------------------------------------------------------------
// Purpose: We share ammo with the pipelauncher, so synchronise
//----------------------------------------------------------------------------
void CFFWeaponGrenadeLauncher::Synchronise()
{
	if ( FFDEV_BLUEPIPES_SYNCWITHYELLOWS )
	{
		CFFPlayer *pPlayer = GetPlayerOwner();

		// Player was NULL once in an mdmp on build 3101
		if( !pPlayer )
			return;

		// We could probably just do GetWeapon(2) 
		for (int i = 0; i < MAX_WEAPON_SLOTS; i++) 
		{
			CFFWeaponBase *w = dynamic_cast<CFFWeaponBase *> (pPlayer->GetWeapon(i)); // This gets holstered weapon numbers. 0=crowbar, 1=shotgun, (gren launcher is deployed), 2=pipelauncher 3=detpack

			if (w && w->GetWeaponID() == FF_WEAPON_PIPELAUNCHER)
			{
				w->m_iClip1 = m_iClip1;
				break;
			}
		}
	}
}



//----------------------------------------------------------------------------
// Purpose: Send special hint on launcher deploy
//----------------------------------------------------------------------------
bool CFFWeaponGrenadeLauncher::Deploy() 
{

#ifdef CLIENT_DLL	
	FF_SendHint( DEMOMAN_GL, 1, PRIORITY_LOW, "#FF_HINT_DEMOMAN_GL" );
#endif
	
	return BaseClass::Deploy();
}