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

	C_FFInfoScript() { m_iShadow = 0; m_iHasModel = 0; }
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
	unsigned int m_iShadow;
	unsigned int m_iHasModel;

	CNetworkVar( int, m_iGoalState );
	CNetworkVar( int, m_iPosState );
};

IMPLEMENT_CLIENTCLASS_DT( C_FFInfoScript, DT_FFInfoScript, CFFInfoScript ) 
	RecvPropFloat( RECVINFO( m_flThrowTime ) ),
	RecvPropVector( RECVINFO( m_vecOffset ) ),
	RecvPropInt( RECVINFO( m_iGoalState ) ),
	RecvPropInt( RECVINFO( m_iPosState ) ),
	RecvPropInt( RECVINFO( m_iShadow ) ),
	RecvPropInt( RECVINFO( m_iHasModel ) ),
END_RECV_TABLE() 


void C_FFInfoScript::OnDataChanged( DataUpdateType_t updateType ) 
{
	// NOTE: We MUST call the base classes' implementation of this function
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED ) 
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
	
	UpdateVisibility();
}

int C_FFInfoScript::DrawModel( int flags ) 
{
	if( !m_iHasModel )
		return 0;

	if( !ShouldDraw() )
		return 0;

	// Bug #0000590: You don't see flags you've thrown, or have, while in thirdperson
	// Always draw if thirdperson
	if( input->CAM_IsThirdPerson() )
		return BaseClass::DrawModel( flags );

	// Bug #0001660: Flag visible in first-person spectator mode
	// Don't draw the FFInfoScript if we're in-eye spectating the same player the flag is hitched to |---> Defrag
	CBasePlayer *pLocalPlayer = C_FFPlayer::GetLocalFFPlayerOrObserverTarget();

	if (pLocalPlayer && pLocalPlayer == GetFollowedEntity())
		return 0;

	// Temporary fix until we sort out the interpolation business
	if (m_flThrowTime + 0.2f > gpGlobals->curtime) 
		return 0;

	return BaseClass::DrawModel(flags);
}

// Bug #0000508: Carried objects cast a shadow for the carrying player
ShadowType_t C_FFInfoScript::ShadowCastType( void )
{
	if( !m_iHasModel )
		return SHADOWS_NONE;

	if( !m_iShadow )
		return SHADOWS_NONE;

	if( !ShouldDraw() )
		return SHADOWS_NONE;

	if( GetFollowedEntity() == C_FFPlayer::GetLocalFFPlayerOrObserverTarget() )
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
	// Adjust offset relative to player
	C_FFPlayer *pPlayer = ToFFPlayer( GetFollowedEntity() );
	if( pPlayer && !m_vecOffset.IsZero() )
	{
		// Adjust origin based on player's origin

		Vector vecOrigin, vecForward, vecRight, vecUp;
		vecOrigin = pPlayer->GetFeetOrigin();

		pPlayer->EyeVectors( &vecForward, &vecRight );

		// Level off
		vecForward.z = 0;
		VectorNormalize( vecForward );

		// Get straight up vector
		vecUp = Vector( 0, 0, 1.0f );

		// Get right vector (think this is correct, otherwise swap the shits)
		vecRight = CrossProduct( vecUp, vecForward );

		VectorNormalize( vecRight );

		bool bBalls;
		bBalls = false;

		Vector vecBallOrigin = vecOrigin + ( vecForward * m_vecOffset.x ) + ( vecRight * m_vecOffset.y ) + ( vecUp * m_vecOffset.z );

		SetAbsOrigin( vecBallOrigin );
		SetAbsAngles( QAngle( 0, GetFollowedEntity()->GetAbsAngles().y, 0 ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_FFInfoScript::ShouldDraw( void )
{
	return !IsRemoved();
}

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
	return ( m_iGoalState.Get() == GS_REMOVED ) || ( m_iPosState.Get() == PS_REMOVED );
}
