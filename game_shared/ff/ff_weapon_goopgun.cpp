/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_goopgun.cpp
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
#include "ff_projectile_goop.h"
#include "shot_manipulator.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponGoopGun C_FFWeaponGoopGun
	#include "c_ff_player.h"
	#include "ff_utils.h"
#else
	#include "omnibot_interface.h"
	#include "ff_player.h"
#endif

ConVar ffdev_goopgun_spread("ffdev_goopgun_spread", "0.1", FCVAR_REPLICATED, "Spread of goops");
ConVar ffdev_goopgun_numprojectiles("ffdev_goopgun_numprojectiles", "12", FCVAR_REPLICATED, "Number of goops per shot");
ConVar ffdev_goopgun_velocity("ffdev_goopgun_velocity", "1000", FCVAR_REPLICATED, "Speed of goops shot");
ConVar ffdev_goopgun_damage("ffdev_goopgun_damage", "6", FCVAR_REPLICATED, "Damage per goop");

#define FF_GOOPGUN_SPREAD ffdev_goopgun_spread.GetFloat()
#define FF_GOOPGUN_NUMPROJECTILES ffdev_goopgun_numprojectiles.GetInt()
#define FF_GOOPGUN_VELOCITY ffdev_goopgun_velocity.GetFloat()
#define FF_GOOPGUN_DAMAGEPERGOOP ffdev_goopgun_damage.GetFloat()

//=============================================================================
// CFFWeaponGoopGun
//=============================================================================

class CFFWeaponGoopGun : public CFFWeaponBaseClip
{
public:
	DECLARE_CLASS(CFFWeaponGoopGun, CFFWeaponBaseClip);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponGoopGun();

	virtual void		Fire();
	virtual bool		SendWeaponAnim(int iActivity);

	virtual FFWeaponID	GetWeaponID() const	{ return FF_WEAPON_GOOPGUN; }

private:
	CFFWeaponGoopGun(const CFFWeaponGoopGun &);
};

//=============================================================================
// CFFWeaponGoopGun tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponGoopGun, DT_FFWeaponGoopGun) 

BEGIN_NETWORK_TABLE(CFFWeaponGoopGun, DT_FFWeaponGoopGun) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponGoopGun) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_goopgun, CFFWeaponGoopGun);
PRECACHE_WEAPON_REGISTER(ff_weapon_goopgun);

//=============================================================================
// CFFWeaponGoopGun implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponGoopGun::CFFWeaponGoopGun() 
{
	m_fIsSwitching = false;
}

//----------------------------------------------------------------------------
// Purpose: Fires a grenade
//----------------------------------------------------------------------------
void CFFWeaponGoopGun::Fire() 
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

	CShotManipulator Manipulator(vForward);

	// shoot all projectiles
	for (int i=0; i<FF_GOOPGUN_NUMPROJECTILES; i++)
	{
		Vector vecDir;
		QAngle shootAngle;

		// If we're firing multiple shots, and the first shot has to be bang on target, ignore spread
		// TODO: Possibly also dot his when m_iShots == 1
		if (i == 0 && FF_GOOPGUN_NUMPROJECTILES > 1)
			vecDir = Manipulator.GetShotDirection();

		// Don't run the biasing code for the player at the moment.
		else
			vecDir = Manipulator.ApplySpread(Vector(FF_GOOPGUN_SPREAD, FF_GOOPGUN_SPREAD, FF_GOOPGUN_SPREAD));
		
		VectorAngles( vecDir, shootAngle );

		CFFProjectileGoop *pGoop = CFFProjectileGoop::CreateGoop(this, vecSrc, shootAngle - QAngle(12.0f, 0.0f, 0.0f), pPlayer, FF_GOOPGUN_DAMAGEPERGOOP, pWeaponInfo.m_iDamageRadius, FF_GOOPGUN_VELOCITY);
		pGoop;
	}
}

//----------------------------------------------------------------------------
// Purpose: Override animations
//----------------------------------------------------------------------------
bool CFFWeaponGoopGun::SendWeaponAnim(int iActivity) 
{
	// Override the animation with a specific one
	switch (iActivity) 
	{
	case ACT_VM_DRAW:
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

	return BaseClass::SendWeaponAnim(iActivity);
}