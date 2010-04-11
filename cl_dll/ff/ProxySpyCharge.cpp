/********************************************************************
	created:	2006/08/07
	created:	7:8:2006   22:02
	filename: 	f:\ff-svn\code\trunk_current\cl_dll\ff\ProxySpyCharge.cpp
	file path:	f:\ff-svn\code\trunk_current\cl_dll\ff
	file base:	ProxySpyCharge
	file ext:	cpp
	author:		Ryan "squeek" Liptak
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "FunctionProxy.h"
#include "c_ff_player.h"
#include "ff_weapon_base.h"
#include "materialsystem/IMaterialVar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Yuck
extern ConVar ffdev_cloaktime;

//-----------------------------------------------------------------------------
// Returns charge of current weapon
//-----------------------------------------------------------------------------
class CProxySpyChargeCharge : public CResultProxy
{
public:
	virtual void OnBind(void *pC_BaseEntity);

private:
	int		m_iModifier;
};

void CProxySpyChargeCharge::OnBind(void *pC_BaseEntity)
{
	if (!pC_BaseEntity)
		return;

	C_FFPlayer *pPlayer = ToFFPlayer(C_BasePlayer::GetLocalPlayer());
	Assert(pPlayer);

	if (!pPlayer)
		return;

	Assert(m_pResult);

	float flCloakTime = gpGlobals->curtime - pPlayer->GetCloakTime();
	flCloakTime = 100 * clamp( flCloakTime / ffdev_cloaktime.GetFloat(), 0.01f, 1.0f );
	
	// returns a float from 1-100 (percentage charged)
	SetFloatResult(flCloakTime);
}

EXPOSE_INTERFACE(CProxySpyChargeCharge, IMaterialProxy, "SpyCharge" IMATERIAL_PROXY_INTERFACE_VERSION);