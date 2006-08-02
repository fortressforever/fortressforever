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

// An info_ff_script has 3 position states
enum FF_ScriptPosState_e
{
	PS_RETURNED = 0,
	PS_CARRIED = 1,
	PS_DROPPED = 2,
	PS_REMOVED = 3
};

// An info_ff_script has 3 goal states
enum FF_ScriptGoalState_e
{
	GS_ACTIVE = 0,
	GS_INACTIVE = 1,
	GS_REMOVED = 2
};

class C_FFInfoScript : public C_BaseAnimating
{
public:
	DECLARE_CLASS(C_FFInfoScript, C_BaseAnimating);
	DECLARE_CLIENTCLASS();

	C_FFInfoScript() { m_iShadow = 1; }
	~C_FFInfoScript() {};

	virtual void	OnDataChanged(DataUpdateType_t updateType);
	virtual bool	ShouldDraw( void );
	virtual int		DrawModel(int flags);	
	virtual void	ClientThink( void );
	virtual ShadowType_t ShadowCastType( void );
	virtual Class_T Classify( void ) { return CLASS_INFOSCRIPT; }

	virtual bool	IsPlayer( void ) { return false; }
	virtual bool	BlocksLOS( void ) { return false; }
	virtual bool	IsAlive( void ) { return false; }

	//virtual int		ShouldTransmit( const CCheckTransmitInfo *pInfo );

	// An info_ff_script's position state
	virtual bool	IsCarried( void );
	virtual bool	IsDropped( void );
	virtual bool	IsReturned( void );

	// An info_ff_script's goal state
	virtual bool	IsActive( void );
	virtual bool	IsInactive( void );
	virtual bool	IsRemoved( void );


protected:
	float m_flThrowTime;

	Vector m_vecOffset;
	int m_iShadow;

	CNetworkVar( int, m_iGoalState );
	CNetworkVar( int, m_iPosState );
};

IMPLEMENT_CLIENTCLASS_DT( C_FFInfoScript, DT_FFInfoScript, CFFInfoScript ) 
	RecvPropFloat( RECVINFO( m_flThrowTime ) ),
	RecvPropVector( RECVINFO( m_vecOffset ) ),
	RecvPropInt( RECVINFO( m_iGoalState ) ),
	RecvPropInt( RECVINFO( m_iPosState ) ),
	RecvPropInt( RECVINFO( m_iShadow ) ),
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
	if( !ShouldDraw() )
		return 0;

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
	if( !m_iShadow )
		return SHADOWS_NONE;

	if( !ShouldDraw() )
		return SHADOWS_NONE;

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

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_FFInfoScript::ShouldDraw( void )
{
	return !IsRemoved();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
//int C_FFInfoScript::ShouldTransmit( const CCheckTransmitInfo *pInfo )
//{
	// Force sending even if no model. By default objects
	// without a model aren't sent to the client. And,
	// sometimes we don't want a model.
//	return FL_EDICT_ALWAYS;
//}

//-----------------------------------------------------------------------------
// Purpose: Is this info_ff_script currently carried?
//-----------------------------------------------------------------------------
bool C_FFInfoScript::IsCarried( void )
{
	return m_iPosState == PS_CARRIED;
}

//-----------------------------------------------------------------------------
// Purpose: Is this info_ff_script currently dropped?
//-----------------------------------------------------------------------------
bool C_FFInfoScript::IsDropped( void )
{
	return m_iPosState == PS_DROPPED;
}

//-----------------------------------------------------------------------------
// Purpose: Is this info_ff_script currently returned?
//-----------------------------------------------------------------------------
bool C_FFInfoScript::IsReturned( void )
{
	return m_iPosState == PS_RETURNED;
}

//-----------------------------------------------------------------------------
// Purpose: Is this info_ff_script "active"?
//-----------------------------------------------------------------------------
bool C_FFInfoScript::IsActive( void )
{
	return m_iGoalState == GS_ACTIVE;
}

//-----------------------------------------------------------------------------
// Purpose: Is this info_ff_script "inactive"?
//-----------------------------------------------------------------------------
bool C_FFInfoScript::IsInactive( void )
{
	return m_iGoalState == GS_INACTIVE;
}

//-----------------------------------------------------------------------------
// Purpose: Is this info_ff_script "removed"?
//-----------------------------------------------------------------------------
bool C_FFInfoScript::IsRemoved( void )
{
	return ( m_iGoalState == GS_REMOVED ) || ( m_iPosState == PS_REMOVED );
}
