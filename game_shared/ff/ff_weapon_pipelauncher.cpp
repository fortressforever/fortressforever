/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_pipelauncher.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF pipe launcher code & class declaration.
///
/// REVISIONS
/// ---------
/// Dec 24, 2004 Mirv: First created
/// Jan 16, 2005 Mirv: Moved all repeated code to base class


#include "cbase.h"
#include "ff_weapon_baseclip.h"
#include "ff_projectile_pipebomb.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponPipeLauncher C_FFWeaponPipeLauncher
	#include "c_ff_player.h"
	#include "ff_utils.h"
#else
	#include "ff_player.h"
#endif

//=============================================================================
// CFFWeaponPipeLauncher
//=============================================================================

class CFFWeaponPipeLauncher : public CFFWeaponBaseClip
{
public:
	DECLARE_CLASS(CFFWeaponPipeLauncher, CFFWeaponBaseClip);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponPipeLauncher();

	virtual void		Fire();
	virtual bool		Reload();
	virtual bool		SendWeaponAnim(int iActivity);
	virtual bool		Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual bool		Deploy();

	void				Synchronise();

	virtual FFWeaponID	GetWeaponID() const	{ return FF_WEAPON_PIPELAUNCHER; }

private:
	CFFWeaponPipeLauncher(const CFFWeaponPipeLauncher &);
};

//=============================================================================
// CFFWeaponPipeLauncher tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponPipeLauncher, DT_FFWeaponPipeLauncher) 

BEGIN_NETWORK_TABLE(CFFWeaponPipeLauncher, DT_FFWeaponPipeLauncher) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponPipeLauncher) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_pipelauncher, CFFWeaponPipeLauncher);
PRECACHE_WEAPON_REGISTER(ff_weapon_pipelauncher);

//=============================================================================
// CFFWeaponPipeLauncher implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponPipeLauncher::CFFWeaponPipeLauncher() 
{
	m_fIsSwitching = false;
}

//----------------------------------------------------------------------------
// Purpose: Fire a pipebomb
//----------------------------------------------------------------------------
void CFFWeaponPipeLauncher::Fire() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	Vector	vForward, vRight, vUp;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);

	Vector	vecSrc = pPlayer->Weapon_ShootPosition() + vForward * 16.0f + vRight * 8.0f + vUp * -8.0f;

	QAngle angAiming;
	VectorAngles(pPlayer->GetAutoaimVector(0), angAiming);

	angAiming -= QAngle(12.0f, 0, 0);

	// Bug #0000192: Demoman can stick pipes to walls
	// Mirv got a better idea?
	trace_t tr;
	UTIL_TraceLine( vecSrc, vecSrc + ( vForward * 32 ), MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
	if( tr.startsolid )
		vecSrc += ( vForward * -16.0f );
	else if( tr.fraction != 1.0f )
		vecSrc += ( vForward * -24.0f );

	CFFProjectilePipebomb::CreatePipebomb(this, vecSrc, angAiming, pPlayer, pWeaponInfo.m_iDamage, pWeaponInfo.m_iSpeed);

#ifdef CLIENT_DLL
	// This is so we know how many pipebombs we have out at a time
	pPlayer->GetPipebombCounter()->Increment();
#endif

#ifdef GAME_DLL
	// Store off the time we shot the pipebomb
	pPlayer->SetPipebombShotTime( gpGlobals->curtime );
#endif

	// Synchronise with grenade launcher
	Synchronise();
}

//----------------------------------------------------------------------------
// Purpose: Keep ammo counts the same
//----------------------------------------------------------------------------
bool CFFWeaponPipeLauncher::Reload() 
{
	bool bRet = BaseClass::Reload();

	// Synchronise with grenade launcher
	Synchronise();

	return bRet;
}

//----------------------------------------------------------------------------
// Purpose: Override animations
//----------------------------------------------------------------------------
bool CFFWeaponPipeLauncher::SendWeaponAnim(int iActivity) 
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
bool CFFWeaponPipeLauncher::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	// Let the other weapon know it's animation type
	CFFWeaponBaseClip *pWeapon = dynamic_cast<CFFWeaponBaseClip *> (pSwitchingTo);

	if (pWeapon && pWeapon->GetWeaponID() == FF_WEAPON_GRENADELAUNCHER)
		pWeapon->m_fIsSwitching = true;

	// Synchronise with grenade launcher
	Synchronise();

	return BaseClass::Holster(pSwitchingTo);
}

//----------------------------------------------------------------------------
// Purpose: We share ammo with the grenade launcher, so synchronise
//----------------------------------------------------------------------------
void CFFWeaponPipeLauncher::Synchronise()
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	if( !pPlayer )
		return;

	// We could probably just do GetWeapon(2) 
	for (int i = 0; i < MAX_WEAPONS; i++) 
	{
		CFFWeaponBase *w = dynamic_cast<CFFWeaponBase *> (pPlayer->GetWeapon(i));

		if (w && w->GetWeaponID() == FF_WEAPON_GRENADELAUNCHER) 
			w->m_iClip1 = m_iClip1;
	}
}


//----------------------------------------------------------------------------
// Purpose: Send special hint on launcher deploy
//----------------------------------------------------------------------------
bool CFFWeaponPipeLauncher::Deploy() 
{

#ifdef CLIENT_DLL	
	FF_SendHint( DEMOMAN_PL, "#FF_HINT_DEMOMAN_PL" );
#endif
	
	return BaseClass::Deploy();
}

