/********************************************************************
	created:	2006/01/21
	created:	21:1:2006   20:55
	filename: 	f:\cvs\code\cl_dll\c_ff_item_flag.cpp
	file path:	f:\cvs\code\cl_dll
	file base:	c_ff_item_flag
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "c_ff_player.h"
#include "iinput.h"

class C_FFItemFlag : public C_BaseAnimating
{
public:
	DECLARE_CLASS(C_FFItemFlag, C_BaseAnimating);
	DECLARE_CLIENTCLASS();

	C_FFItemFlag() {};
	~C_FFItemFlag() {};

	virtual void OnDataChanged(DataUpdateType_t updateType);
	virtual int DrawModel(int flags);
	virtual ShadowType_t ShadowCastType( void );

	float m_flThrowTime;
};

IMPLEMENT_CLIENTCLASS_DT(C_FFItemFlag, DT_FFItemFlag, CFFItemFlag) 
	RecvPropFloat(RECVINFO(m_flThrowTime)) 
END_RECV_TABLE() 


void C_FFItemFlag::OnDataChanged(DataUpdateType_t updateType) 
{
	// NOTE: We MUST call the base classes' implementation of this function
	BaseClass::OnDataChanged(updateType);

	if (updateType == DATA_UPDATE_CREATED) 
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}
}

int C_FFItemFlag::DrawModel(int flags) 
{
	// Bug #0000590: You don't see flags you've thrown, or have, while in thirdperson
	// Always draw if thirdperson
	if( input->CAM_IsThirdPerson() )
		return BaseClass::DrawModel( flags );

	if (GetFollowedEntity() == CBasePlayer::GetLocalPlayer()) 
		return 0;

	// Temporary fix until we sort out the interpolation business
	if (m_flThrowTime + 0.2f > gpGlobals->curtime) 
		return 0;

	return BaseClass::DrawModel(flags);
}

// Bug #0000508: Carried objects cast a shadow for the carrying player
ShadowType_t C_FFItemFlag::ShadowCastType( void )
{
	if( GetFollowedEntity() == C_BasePlayer::GetLocalPlayer() )
	{
		if( r_selfshadows.GetInt() )
			return SHADOWS_RENDER_TO_TEXTURE;

		return SHADOWS_NONE;
	}
	else
	{
		return SHADOWS_RENDER_TO_TEXTURE;
	}
}
