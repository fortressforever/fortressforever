/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_nailgun.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF nailgun code & class declaration
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First logged
/// Jan 16, 2005 Mirv: Moved all repeated code to base class


#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"
#include "ff_projectile_nail.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponNailgun C_FFWeaponNailgun
	#include "c_ff_player.h"
#else
	#include "omnibot_interface.h"
	#include "ff_player.h"
#endif

//=============================================================================
// CFFWeaponNailgun
//=============================================================================

class CFFWeaponNailgun : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponNailgun, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponNailgun();

	virtual void Fire();

	virtual FFWeaponID GetWeaponID() const { return FF_WEAPON_NAILGUN; }

private:

	CFFWeaponNailgun(const CFFWeaponNailgun &);

};

//=============================================================================
// CFFWeaponNailgun tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponNailgun, DT_FFWeaponNailgun) 

BEGIN_NETWORK_TABLE(CFFWeaponNailgun, DT_FFWeaponNailgun) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponNailgun) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_nailgun, CFFWeaponNailgun);
PRECACHE_WEAPON_REGISTER(ff_weapon_nailgun);

//=============================================================================
// CFFWeaponNailgun implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponNailgun::CFFWeaponNailgun() 
{
}

//----------------------------------------------------------------------------
// Purpose: Fire a nail
//----------------------------------------------------------------------------
void CFFWeaponNailgun::Fire() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	Vector vecShootOrigin = pPlayer->Weapon_ShootPosition();

	Vector vecSpawnOrigin;
	CBaseViewModel *vm = pPlayer->GetViewModel();
	vm->GetAttachment(vm->LookupAttachment("1"), vecSpawnOrigin, QAngle());

	// Trace is to check if the gun is sticking out the other side of a thin object, bullets will spawn at the muzzleflash attachment if it's safe.
	trace_t tr;
	UTIL_TraceLine(vecShootOrigin, vecSpawnOrigin, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);
	if(tr.fraction != 1.0f)
	{
		// Nail spawned on other side of object, force the nail to spawn at the weapon shoot position.
		vecSpawnOrigin = vecShootOrigin;
	}

	// Another trace to get the point the nail will eventually end.
	Vector vecForward;
	pPlayer->EyeVectors(&vecForward);
	UTIL_TraceLine(vecShootOrigin, vecShootOrigin + (vecForward * 32768), MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);


	// TODO: Trace a line from the vecSpawnOrigin to tr.endpos, if something is in the way then set vecSpawnOrigin to vecShootOrigin.
	// TODO: If we moved vecSpawnOrigin to vecShootOrigin in the first trace or this trace, then move the nailgun model so it shoots from that position (sort of like iron sights).
	// -->
	// <--

	// Get the angle the nail should be, this also determines the nails end position (since velocity is set based on angle).
	QAngle nailAngles;
	VectorAngles((tr.endpos - vecSpawnOrigin), nailAngles);

	CFFProjectileNail *pNail = CFFProjectileNail::CreateNail(this, vecSpawnOrigin, nailAngles, pPlayer, pWeaponInfo.m_iDamage, pWeaponInfo.m_iSpeed);
	pNail;

#ifdef GAME_DLL
	Omnibot::Notify_PlayerShoot(pPlayer, Omnibot::TF_WP_NAILGUN, pNail);
#endif
}
