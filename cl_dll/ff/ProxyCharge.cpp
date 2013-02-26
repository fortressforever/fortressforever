/********************************************************************
	created:	2006/08/07
	created:	7:8:2006   22:02
	filename: 	f:\ff-svn\code\trunk_current\cl_dll\ff\ProxyCharge.cpp
	file path:	f:\ff-svn\code\trunk_current\cl_dll\ff
	file base:	ProxyCharge
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
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
extern float GetAssaultCannonCharge();

//-----------------------------------------------------------------------------
// Returns charge of current weapon
//-----------------------------------------------------------------------------
class CProxyCharge : public CResultProxy
{
public:
	virtual void OnBind(void *pC_BaseEntity);

private:
	int		m_iModifier;
};

void CProxyCharge::OnBind(void *pC_BaseEntity)
{
	if (!pC_BaseEntity)
		return;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayerOrObserverTarget();
	Assert(pPlayer);

	if (!pPlayer)
		return;

	C_FFWeaponBase *pWeapon = pPlayer->GetActiveFFWeapon();
	Assert(pWeapon);

	if (!pWeapon || pWeapon->GetWeaponID() != FF_WEAPON_ASSAULTCANNON)
		return;

	Assert(m_pResult);

	// Use this awful global function for now
	// TODO: Stop using this awful global function
	float flCharge = GetAssaultCannonCharge();
	
	SetFloatResult(flCharge);
}

EXPOSE_INTERFACE(CProxyCharge, IMaterialProxy, "CurrentCharge" IMATERIAL_PROXY_INTERFACE_VERSION);