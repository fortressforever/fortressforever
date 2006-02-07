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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Returns ammo in active weapon(from 0 to 200) 
//-----------------------------------------------------------------------------
class CProxyAmmo : public CResultProxy
{
public:
	virtual void OnBind(void *pC_BaseEntity);
};

void CProxyAmmo::OnBind(void *pC_BaseEntity) 
{
	if (!pC_BaseEntity) 
		return;

	C_FFPlayer *pPlayer = ToFFPlayer(C_BasePlayer::GetLocalPlayer());

	Assert(m_pResult);

	if (pPlayer && pPlayer->GetActiveFFWeapon()) 
		SetFloatResult(pPlayer->GetActiveFFWeapon()->m_iClip1);
	else
		SetFloatResult(0);
}

EXPOSE_INTERFACE(CProxyAmmo, IMaterialProxy, "CurrentAmmo" IMATERIAL_PROXY_INTERFACE_VERSION);