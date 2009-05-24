//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements visual effects entities: sprites, beams, bubbles, etc.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "engine/IEngineSound.h"
#include "baseentity.h"
#include "entityoutput.h"
#include "recipientfilter.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CEnvHudHint : public CPointEntity
{
public:
	DECLARE_CLASS( CEnvHudHint, CPointEntity );

	void	Spawn( void );
	void	Precache( void );

private:
	void InputShowHudHint( inputdata_t &inputdata );
	void InputHideHudHint( inputdata_t &inputdata );
	string_t m_iszMessage;
	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( env_hudhint, CEnvHudHint );

BEGIN_DATADESC( CEnvHudHint )

	DEFINE_KEYFIELD( m_iszMessage, FIELD_STRING, "message" ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ShowHudHint", InputShowHudHint ),
	DEFINE_INPUTFUNC( FIELD_VOID, "HideHudHint", InputHideHudHint ),

END_DATADESC()



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvHudHint::Spawn( void )
{
	Precache();

	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NONE );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvHudHint::Precache( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for showing the message and/or playing the sound.
//-----------------------------------------------------------------------------
void CEnvHudHint::InputShowHudHint( inputdata_t &inputdata )
{
	CBaseEntity *pPlayer = NULL;

	if ( inputdata.pActivator && inputdata.pActivator->IsPlayer() )
	{
		pPlayer = inputdata.pActivator;
	}
	else
	{
		pPlayer = UTIL_GetLocalPlayer();
	}

	if ( pPlayer )
	{
		if ( !pPlayer || !pPlayer->IsNetClient() )
			return;

		CSingleUserRecipientFilter user( (CBasePlayer *)pPlayer );
		user.MakeReliable();
		UserMessageBegin( user, "HintText" );
			WRITE_BYTE( 1 );	// one message
			WRITE_STRING( STRING(m_iszMessage) );
		MessageEnd();
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CEnvHudHint::InputHideHudHint( inputdata_t &inputdata )
{
	CBaseEntity *pPlayer = NULL;

	if ( inputdata.pActivator && inputdata.pActivator->IsPlayer() )
	{
		pPlayer = inputdata.pActivator;
	}
	else
	{
		pPlayer = UTIL_GetLocalPlayer();
	}

	if ( pPlayer )
	{
		if ( !pPlayer || !pPlayer->IsNetClient() )
			return;

		CSingleUserRecipientFilter user( (CBasePlayer *)pPlayer );
		user.MakeReliable();
		UserMessageBegin( user, "HintText" );
		WRITE_BYTE( 1 );	// one message
		WRITE_STRING( STRING(NULL_STRING) );
		MessageEnd();
	}
}
