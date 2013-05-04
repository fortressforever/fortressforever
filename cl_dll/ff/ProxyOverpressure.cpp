/********************************************************************
	created:	2006/08/07
	created:	7:8:2006   22:02
	filename: 	f:\ff-svn\code\trunk_current\cl_dll\ff\ProxyOverpressureCharge.cpp
	file path:	f:\ff-svn\code\trunk_current\cl_dll\ff
	file base:	ProxyOverpressureCharge
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
//extern ConVar ffdev_overpressure_delay;
#define OVERPRESSURE_COOLDOWN 16	//ffdev_overpressure_delay.GetFloat()

//-----------------------------------------------------------------------------
// Returns charge of current weapon
//-----------------------------------------------------------------------------
class CProxyOverpressureCharge : public CResultProxy
{
public:
	virtual void OnBind(void *pC_BaseEntity);

private:
	int		m_iModifier;
};

void CProxyOverpressureCharge::OnBind(void *pC_BaseEntity)
{
	if (!pC_BaseEntity)
		return;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayerOrObserverTarget();
	Assert(pPlayer);

	if (!pPlayer)
		return;

	Assert(m_pResult);

	// Use this awful global function for now
	// TODO: Stop using this awful global function
	//float flNextClassSpecificSkill = GetAssaultCannonCharge();
	// gotta take into account the spinup before we start displaying heat.
	float flNextClassSpecificSkill = ( OVERPRESSURE_COOLDOWN - (pPlayer->m_flNextClassSpecificSkill - gpGlobals->curtime) ) / ( OVERPRESSURE_COOLDOWN );
	
	flNextClassSpecificSkill = 100 * clamp( flNextClassSpecificSkill, 0.01f, 1.0f );
	
	SetFloatResult(flNextClassSpecificSkill);
}

EXPOSE_INTERFACE(CProxyOverpressureCharge, IMaterialProxy, "OverpressureCharge" IMATERIAL_PROXY_INTERFACE_VERSION);