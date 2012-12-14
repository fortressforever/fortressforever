/********************************************************************
	created:	2006/01/31
	created:	31:1:2006   0:43
	filename: 	f:\cvs\code\cl_dll\ff\ProxyTeam.cpp
	file path:	f:\cvs\code\cl_dll\ff
	file base:	ProxyTeam
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Team ID material proxy
*********************************************************************/

#include "cbase.h"
#include "FunctionProxy.h"
#include "c_ff_player.h"
#include "ff_weapon_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Returns the player team id(from 0 to 4) 
//-----------------------------------------------------------------------------
class CProxyTeam : public CResultProxy
{
public:
	virtual void OnBind(void *pC_BaseEntity);
};

void CProxyTeam::OnBind(void *pC_BaseEntity) 
{
	if (!pC_BaseEntity) 
		return;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayerOrObserverTarget();

	Assert(m_pResult);

	if (pPlayer) 
		SetFloatResult(pPlayer->GetTeamNumber() - (TEAM_BLUE - 1));
	else
		SetFloatResult(0);
}

EXPOSE_INTERFACE(CProxyTeam, IMaterialProxy, "CurrentTeam" IMATERIAL_PROXY_INTERFACE_VERSION);