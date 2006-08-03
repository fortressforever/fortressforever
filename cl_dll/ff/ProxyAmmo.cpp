/********************************************************************
	created:	2006/01/31
	created:	31:1:2006   0:42
	filename: 	f:\cvs\code\cl_dll\ff\ProxyAmmo.cpp
	file path:	f:\cvs\code\cl_dll\ff
	file base:	ProxyAmmo
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Ammo count material proxy for weapons
*********************************************************************/

#include "cbase.h"
#include "FunctionProxy.h"
#include "c_ff_player.h"
#include "ff_weapon_base.h"
#include "materialsystem/IMaterialVar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Returns ammo in active weapon(from 0 to 200) 
//-----------------------------------------------------------------------------
class CProxyAmmo : public CResultProxy
{
public:
	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	virtual void OnBind(void *pC_BaseEntity);

private:
	int		m_iModifier;
};

bool CProxyAmmo::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	bool bModifier;
	IMaterialVar *pModifier = pMaterial->FindVar("$digitindex", &bModifier, false);

	if (!bModifier)
	{
		m_iModifier = -1;
	}
	else
	{
		m_iModifier = pModifier->GetIntValue();
	}

	return CResultProxy::Init(pMaterial, pKeyValues);
}

void CProxyAmmo::OnBind(void *pC_BaseEntity)
{
	if (!pC_BaseEntity)
		return;

	C_FFPlayer *pPlayer = ToFFPlayer(C_BasePlayer::GetLocalPlayer());
	Assert(pPlayer);

	if (!pPlayer)
		return;

	C_FFWeaponBase *pWeapon = pPlayer->GetActiveFFWeapon();
	Assert(pWeapon);

	if (!pWeapon)
		return;

	Assert(m_pResult);

	int iAmmo;

	// Make sure we report the right ammo
	if (pWeapon->UsesClipsForAmmo1())
	{
		iAmmo = pWeapon->m_iClip1;
	}
	else
	{
		iAmmo = pPlayer->GetAmmoCount(pWeapon->GetPrimaryAmmoType());
	}

	// Make sure we show the correct digit
	if (m_iModifier >= 0)
	{
		int p = pow(10, m_iModifier);
		iAmmo /= p;
		iAmmo -= 10 * (iAmmo / 10);
	}

	if (pPlayer && pPlayer->GetActiveFFWeapon())
	{
		SetFloatResult(iAmmo);
	}
	else
	{
		SetFloatResult(0);
	}
}

EXPOSE_INTERFACE(CProxyAmmo, IMaterialProxy, "CurrentAmmo" IMATERIAL_PROXY_INTERFACE_VERSION);