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

class C_FFInfoScript : public C_BaseAnimating
{
public:
	DECLARE_CLASS(C_FFInfoScript, C_BaseAnimating);
	DECLARE_CLIENTCLASS();

	C_FFInfoScript() {};
	~C_FFInfoScript() {};

	virtual void	OnDataChanged(DataUpdateType_t updateType);
	virtual int		DrawModel(int flags);	
	virtual void	ClientThink( void );
	virtual ShadowType_t ShadowCastType( void );
	virtual Class_T Classify( void ) { return CLASS_INFOSCRIPT; }

protected:
	float m_flThrowTime;

	Vector m_vecOffset;
};

IMPLEMENT_CLIENTCLASS_DT( C_FFInfoScript, DT_FFInfoScript, CFFInfoScript ) 
	RecvPropFloat( RECVINFO( m_flThrowTime ) ),
	RecvPropVector( RECVINFO( m_vecOffset ) ),
END_RECV_TABLE() 


void C_FFInfoScript::OnDataChanged( DataUpdateType_t updateType ) 
{
	// NOTE: We MUST call the base classes' implementation of this function
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED ) 
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

int C_FFInfoScript::DrawModel( int flags ) 
{
	// Bug #0000590: You don't see flags you've thrown, or have, while in thirdperson
	// Always draw if thirdperson
	if( input->CAM_IsThirdPerson() )
		return BaseClass::DrawModel( flags );

	// Flags seem to have changed to using GetOwnerEntity now
	if (GetFollowedEntity() == CBasePlayer::GetLocalPlayer()) 
		return 0;

	// Temporary fix until we sort out the interpolation business
	if (m_flThrowTime + 0.2f > gpGlobals->curtime) 
		return 0;

	return BaseClass::DrawModel(flags);
}

// Bug #0000508: Carried objects cast a shadow for the carrying player
ShadowType_t C_FFInfoScript::ShadowCastType( void )
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

void C_FFInfoScript::ClientThink( void )
{
	/* Yeah...
	// If we have an owner entity we are being carried
	if( GetFollowedEntity() )
	{
		// Move our origin according to GetOwnerEntity()'s GetAbsOrigin()

		// All this "extra" junk is so the object doesn't move
		// when the player looks up or down
		Vector vecForward, vecRight, vecUp;
		GetFollowedEntity()->GetVectors( &vecForward, &vecRight, &vecUp );

		Vector vecOrigin = GetFollowedEntity()->GetAbsOrigin();

		VectorNormalizeFast( vecForward );

		Vector vecForwardPos = vecOrigin + ( vecForward * 96.0f );

		// Put on the same plane
		vecForwardPos.z = vecOrigin.z;
		
		// Get real forward direction now (not just "forward-wherever-crosshair-pointed" direction)
		vecForward = vecForwardPos - vecOrigin;

		// Get real up direction
		vecUp = ( vecOrigin + Vector( 0, 0, 96.0f ) ) - vecOrigin;

		VectorNormalizeFast( vecForward );		
		VectorNormalizeFast( vecUp );

		// Think this is correct to get a right vector
		CrossProduct( vecUp, vecForward, vecRight );

		VectorNormalizeFast( vecRight );

		SetAbsOrigin( vecOrigin + ( vecForward * m_vecOffset.x ) + ( vecRight * m_vecOffset.y ) + ( vecUp * m_vecOffset.z ) );
		SetAbsAngles( QAngle( 0, GetFollowedEntity()->GetAbsAngles().y, 0 ) );
	}
	*/
}
