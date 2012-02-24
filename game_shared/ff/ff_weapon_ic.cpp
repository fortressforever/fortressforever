/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_ic.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF Rocket launcher code & class declaration.
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation logged
/// Jan 16, 2005 Mirv: Moved all repeated code to base class


#include "cbase.h"
#include "ff_weapon_baseclip.h"
#include "ff_projectile_incendiaryrocket.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponIC C_FFWeaponIC
	#include "c_ff_player.h"
	#include "ff_utils.h"
#else
	#include "omnibot_interface.h"
	#include "ff_player.h"
#endif

ConVar ffdev_ic_blastpush("ffdev_ic_blastpush", "0", FCVAR_REPLICATED | FCVAR_CHEAT);

//=============================================================================
// CFFWeaponIC
//=============================================================================

class CFFWeaponIC : public CFFWeaponBaseClip
{
public:
	DECLARE_CLASS(CFFWeaponIC, CFFWeaponBaseClip);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponIC();

	virtual void		Fire();
	virtual bool Deploy();
	virtual FFWeaponID	GetWeaponID() const	{ return FF_WEAPON_IC; }

private:
	CFFWeaponIC(const CFFWeaponIC &);
};

//=============================================================================
// CFFWeaponIC tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponIC, DT_FFWeaponIC) 

BEGIN_NETWORK_TABLE(CFFWeaponIC, DT_FFWeaponIC) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponIC) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_ic, CFFWeaponIC);
PRECACHE_WEAPON_REGISTER(ff_weapon_ic);

//=============================================================================
// CFFWeaponIC implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponIC::CFFWeaponIC() 
{
}


//----------------------------------------------------------------------------
// Purpose: Display the IC hint
//----------------------------------------------------------------------------
bool CFFWeaponIC::Deploy()
{
#ifdef CLIENT_DLL
	
	FF_SendHint( PYRO_IC, 1, PRIORITY_LOW, "#FF_HINT_PYRO_IC" );
#endif
	
	return BaseClass::Deploy();
}


//----------------------------------------------------------------------------
// Purpose: Fire a rocket
//----------------------------------------------------------------------------
void CFFWeaponIC::Fire() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
 	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	Vector	vForward, vRight, vUp;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);

	//Vector	vecSrc = pPlayer->Weapon_ShootPosition() + vForward * 8.0f + vRight * 8.0f + vUp * -8.0f;

	//Vector vecSrc = pPlayer->GetAbsOrigin() + vForward * 16.0f + vRight * 4.0f + Vector(0, 1, 20.0f);
	Vector vecSrc = pPlayer->GetAbsOrigin() + vForward * 16.0f + vRight * 8.0f + Vector(0, 1, (pPlayer->GetFlags() & FL_DUCKING) ? 5.0f : 23.0f);

	// 0000936 - added cvar for testing. Keep line below commented out.
	//CFFProjectileIncendiaryRocket::CreateRocket(this, vecSrc, angAiming, pPlayer, pWeaponInfo.m_iDamage, pWeaponInfo.m_iSpeed);
	CFFProjectileIncendiaryRocket *pRocket = CFFProjectileIncendiaryRocket::CreateRocket(this, vecSrc, pPlayer->EyeAngles(), pPlayer, pWeaponInfo.m_iDamage, pWeaponInfo.m_iDamageRadius, pWeaponInfo.m_iSpeed);
	pRocket;

#ifdef GAME_DLL
	Omnibot::Notify_PlayerShoot(pPlayer, Omnibot::TF_WP_NAPALMCANNON, pRocket);
#endif

	// Push player but don't add to upwards force
	// 0000936 - reduce the blast push
	Vector vecImpulse = vForward * (ffdev_ic_blastpush.GetFloat() * -1.0f);

	if (vecImpulse.z > 0)
		vecImpulse.z = 0;

	pPlayer->ApplyAbsVelocityImpulse(vecImpulse);
}
