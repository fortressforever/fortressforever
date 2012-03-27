/********************************************************************
	created:	2006/08/07
	created:	7:8:2006   22:02
	filename: 	f:\ff-svn\code\trunk_current\cl_dll\ff\ProxySlowfield.cpp
	file path:	f:\ff-svn\code\trunk_current\cl_dll\ff
	file base:	ProxySlowfield
	file ext:	cpp
	author:		Ryan "squeek" Liptak
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "FunctionProxy.h"
#include "c_ff_player.h"
#include "ff_grenade_base.h"
#include "ff_weapon_base.h"
#include "materialsystem/IMaterialVar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Yuck
//extern ConVar ffdev_slowfield_radius_outer;
//extern ConVar ffdev_slowfield_radius_inner;
//extern ConVar ffdev_slowfield_radius_power;
#define SLOWFIELD_RADIUS_OUTER 176 //ffdev_slowfield_radius_outer.GetFloat()
#define SLOWFIELD_RADIUS_INNER 64 //ffdev_slowfield_radius_inner.GetFloat()
#define SLOWFIELD_RADIUS_POWER 1 //ffdev_slowfield_radius_power.GetFloat()

//-----------------------------------------------------------------------------
// Returns charge of current weapon
//-----------------------------------------------------------------------------
class CProxySlowfieldSlow : public CResultProxy
{
public:
	virtual void OnBind(void *pC_BaseEntity);

private:
	int		m_iModifier;
};

void CProxySlowfieldSlow::OnBind(void *pC_BaseEntity)
{
	//if (!pC_BaseEntity)
	//	return;

	C_FFPlayer *pPlayer = ToFFPlayer(C_BasePlayer::GetLocalPlayer());
	Assert(pPlayer);

	if (!pPlayer)
		return;

	if (!pPlayer->IsInSlowfield())
		return;

	Assert(m_pResult);

	C_FFGrenadeBase *pSlowfield = pPlayer->GetActiveSlowfield();
	
	if (!pSlowfield)
		return;

	Vector vecDisplacement = pPlayer->GetAbsOrigin() - pSlowfield->GetAbsOrigin();
	float flDistance = vecDisplacement.Length();

	float flResultVar = 1.0f;

	//if we're scaling between outer and inner radius (linear!!)
	//don't allow divide by zero or for inner/outer to be reversed
	if(flDistance > SLOWFIELD_RADIUS_INNER && (SLOWFIELD_RADIUS_OUTER - SLOWFIELD_RADIUS_INNER) > 0.0f)
	{
		flResultVar = 1.0f - (flDistance - SLOWFIELD_RADIUS_INNER)/(SLOWFIELD_RADIUS_OUTER - SLOWFIELD_RADIUS_INNER);
	}

	flResultVar = clamp( flResultVar, 0.0f, 1.0f );
	
	SetFloatResult(flResultVar);
}

EXPOSE_INTERFACE(CProxySlowfieldSlow, IMaterialProxy, "SlowfieldSlow" IMATERIAL_PROXY_INTERFACE_VERSION);