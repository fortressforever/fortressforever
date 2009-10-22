/********************************************************************
	created:	2006/01/31
	created:	31:1:2006   0:47
	filename: 	f:\cvs\code\cl_dll\ff\ProxyClass.cpp
	file path:	f:\cvs\code\cl_dll\ff
	file base:	ProxyClass
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Class ID material proxy
*********************************************************************/

#include "cbase.h"
#include "FunctionProxy.h"
#include "c_ff_player.h"
#include "ff_weapon_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Returns the player class id(from 0 to 10) 
//-----------------------------------------------------------------------------
class CProxyClass : public CResultProxy
{
public:
	virtual void OnBind(void *pC_BaseEntity);
};

void CProxyClass::OnBind(void *pC_BaseEntity) 
{
	if (!pC_BaseEntity) 
		return;

	C_FFPlayer *pPlayer = ToFFPlayer(C_BasePlayer::GetLocalPlayer());

	Assert(m_pResult);

	if (pPlayer) 
		SetFloatResult(pPlayer->GetClassSlot());
	else
		SetFloatResult(0);
}

EXPOSE_INTERFACE(CProxyClass, IMaterialProxy, "CurrentClass" IMATERIAL_PROXY_INTERFACE_VERSION);