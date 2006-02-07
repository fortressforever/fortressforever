//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Entities that capture the player's UI and move it into game design
//			as outputs.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "entitylist.h"
#include "util.h"
#include "physics.h"
#include "entityoutput.h"
#include "player.h"
#include "in_buttons.h"
#include "basecombatweapon.h"
#include "baseviewmodel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//----------------------------------------------------------------
// Spawn flags
//----------------------------------------------------------------
#define  SF_GAMEUI_FREEZE_PLAYER		32	
#define  SF_GAMEUI_HIDE_WEAPON			64	
#define  SF_GAMEUI_USE_DEACTIVATES		128
#define  SF_GAMEUI_JUMP_DEACTIVATES		256


class CGameUI : public CBaseEntity
{
public:
	DECLARE_CLASS( CGameUI, CBaseEntity );

	DECLARE_DATADESC();

	// Input handlers
	void InputDeactivate( inputdata_t &inputdata );
	void InputActivate( inputdata_t &inputdata );

	void Think( void );
	void Deactivate( CBaseEntity *pActivator );

	float				m_flFieldOfView;
	CHandle<CBaseCombatWeapon>	m_hSaveWeapon;

	COutputEvent		m_playerOn;
	COutputEvent		m_playerOff;

	COutputEvent		m_pressedMoveLeft;
	COutputEvent		m_pressedMoveRight;
	COutputEvent		m_pressedForward;
	COutputEvent		m_pressedBack;
	COutputEvent		m_pressedAttack;
	COutputEvent		m_pressedAttack2;

	COutputFloat		m_xaxis;
	COutputFloat		m_yaxis;
	COutputFloat		m_attackaxis;
	COutputFloat		m_attack2axis;

	bool				m_bForceUpdate;

	CHandle<CBasePlayer>	m_player;
};


BEGIN_DATADESC( CGameUI )

	DEFINE_KEYFIELD( m_flFieldOfView, FIELD_FLOAT, "FieldOfView" ),
	DEFINE_FIELD( m_hSaveWeapon, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bForceUpdate, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_player, FIELD_EHANDLE ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Deactivate", InputDeactivate ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Activate", InputActivate ),

	DEFINE_OUTPUT( m_playerOn, "PlayerOn" ),
	DEFINE_OUTPUT( m_playerOff, "PlayerOff" ),

	DEFINE_OUTPUT( m_pressedMoveLeft, "PressedMoveLeft" ),
	DEFINE_OUTPUT( m_pressedMoveRight, "PressedMoveRight" ),
	DEFINE_OUTPUT( m_pressedForward, "PressedForward" ),
	DEFINE_OUTPUT( m_pressedBack, "PressedBack" ),
	DEFINE_OUTPUT( m_pressedAttack, "PressedAttack" ),
	DEFINE_OUTPUT( m_pressedAttack2, "PressedAttack2" ),

	DEFINE_OUTPUT( m_xaxis, "XAxis" ),
	DEFINE_OUTPUT( m_yaxis, "YAxis" ),
	DEFINE_OUTPUT( m_attackaxis, "AttackAxis" ),
	DEFINE_OUTPUT( m_attack2axis, "Attack2Axis" ),

END_DATADESC()


LINK_ENTITY_TO_CLASS( game_ui, CGameUI );
	

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameUI::InputDeactivate( inputdata_t &inputdata )
{
	Deactivate( inputdata.pActivator );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameUI::Deactivate( CBaseEntity *pActivator )
{
	CBasePlayer *pPlayer = m_player;

	// If deactivated by the player using me
	if ( pPlayer == pActivator )
	{
		// Re-enable player motion
		if (FBitSet(m_spawnflags, SF_GAMEUI_FREEZE_PLAYER))
		{
			m_player->RemoveFlag( FL_ATCONTROLS );
		}

		// Restore weapons
		if (FBitSet(m_spawnflags, SF_GAMEUI_HIDE_WEAPON))
		{
			// Turn the hud back on
			pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_WEAPONSELECTION;

			if (m_hSaveWeapon.Get())
			{
				m_player->Weapon_Switch( m_hSaveWeapon.Get() );
				m_hSaveWeapon = NULL;
			}

			if ( pPlayer->GetActiveWeapon() )
			{
				pPlayer->GetActiveWeapon()->Deploy();
			}
		}

		m_playerOff.FireOutput( pPlayer, this, 0 );

		// clear out the axis controls
		m_xaxis.Set( 0, pPlayer, this );
		m_yaxis.Set( 0, pPlayer, this );
		m_attackaxis.Set( 0, pPlayer, this );
		m_attack2axis.Set( 0, pPlayer, this );

		m_player = NULL;
		SetNextThink( TICK_NEVER_THINK );
	}
}


//------------------------------------------------------------------------------
// Purpose :
//------------------------------------------------------------------------------
void CGameUI::InputActivate( inputdata_t &inputdata )
{
	// If already in use, ignore activatation by others
	if (( m_player.Get() != NULL ) && ( m_player.Get() != inputdata.pActivator ))
	{
		return;
	}

	if ( inputdata.pActivator && inputdata.pActivator->IsPlayer() )
	{
		m_player = (CBasePlayer *)inputdata.pActivator;
		m_playerOn.FireOutput( inputdata.pActivator, this, 0 );

		// Turn the hud off
		SetNextThink( gpGlobals->curtime );

		// Disable player motion
		if (FBitSet(m_spawnflags, SF_GAMEUI_FREEZE_PLAYER))
		{
			m_player->AddFlag( FL_ATCONTROLS );
		}

		if (FBitSet(m_spawnflags, SF_GAMEUI_HIDE_WEAPON))
		{
			m_player->m_Local.m_iHideHUD |= HIDEHUD_WEAPONSELECTION;

			if ( m_player->GetActiveWeapon() )
			{
				m_hSaveWeapon = m_player->GetActiveWeapon();

				m_player->GetActiveWeapon()->Holster();
				m_player->ClearActiveWeapon();
				m_player->HideViewModels();
			}
		}
	}

	m_bForceUpdate = true;
}


//------------------------------------------------------------------------------
// Purpose: Samples the player's inputs and fires outputs based on what buttons
//			are currently held down.
//------------------------------------------------------------------------------
void CGameUI::Think( void )
{
	CBasePlayer *pPlayer = m_player;

	// If player is gone, stop thinking
	if (pPlayer == NULL)
	{
		SetNextThink( TICK_NEVER_THINK );
		return;
	}

	// ------------------------------------------------
	// Check that toucher is facing the UI within
	// the field of view tolerance.  If not disconnect
	// ------------------------------------------------
	if (m_flFieldOfView > -1)
	{
		Vector vPlayerFacing;
		pPlayer->EyeVectors( &vPlayerFacing );
		Vector vPlayerToUI =  GetAbsOrigin() - pPlayer->WorldSpaceCenter();
		VectorNormalize(vPlayerToUI);

		float flDotPr = DotProduct(vPlayerFacing,vPlayerToUI);
		if (flDotPr < m_flFieldOfView)
		{
			Deactivate( pPlayer );
			return;
		}
	}

	pPlayer->AddFlag( FL_ONTRAIN );
	SetNextThink( gpGlobals->curtime );

	// BUGBUG: m_afButtonPressed is broken - check the player.cpp code!!!
	
	//
	// Deactivate if they jump or press +use.
	// FIXME: prevent the use from going through in player.cpp
	if ((( pPlayer->m_afButtonPressed & IN_USE ) && ( m_spawnflags & SF_GAMEUI_USE_DEACTIVATES )) ||
		(( pPlayer->m_afButtonPressed & IN_JUMP ) && ( m_spawnflags & SF_GAMEUI_JUMP_DEACTIVATES )))
	{
		Deactivate( pPlayer );
		return;
	}

	if ( pPlayer->m_afButtonPressed & IN_MOVERIGHT )
	{
		m_pressedMoveRight.FireOutput( pPlayer, this, 0 );
	}
	if ( pPlayer->m_afButtonPressed & IN_MOVELEFT )
	{
		m_pressedMoveLeft.FireOutput( pPlayer, this, 0 );
	}

	if ( pPlayer->m_afButtonPressed & IN_FORWARD )
	{
		m_pressedForward.FireOutput( pPlayer, this, 0 );
	}
	if ( pPlayer->m_afButtonPressed & IN_BACK )
	{
		m_pressedBack.FireOutput( pPlayer, this, 0 );
	}

	if ( pPlayer->m_afButtonPressed & IN_ATTACK )
	{
		m_pressedAttack.FireOutput( pPlayer, this, 0 );
	}

	if ( pPlayer->m_afButtonPressed & IN_ATTACK2 )
	{
		m_pressedAttack2.FireOutput( pPlayer, this, 0 );
	}

	float x = 0, y = 0, attack = 0, attack2 = 0;
	if ( pPlayer->m_nButtons & IN_MOVERIGHT )
	{
		x = 1;
	}
	else if ( pPlayer->m_nButtons & IN_MOVELEFT )
	{
		x = -1;
	}

	if ( pPlayer->m_nButtons & IN_FORWARD )
	{
		y = 1;
	}
	else if ( pPlayer->m_nButtons & IN_BACK )
	{
		y = -1;
	}

	if ( pPlayer->m_nButtons & IN_ATTACK )
	{
		attack = 1;
	}

	if ( pPlayer->m_nButtons & IN_ATTACK2 )
	{
		attack2 = 1;
	}

	//
	// Fire the analog outputs if they changed.
	//
	if ( m_bForceUpdate || ( m_xaxis.Get() != x ) )
	{
		m_xaxis.Set( x, pPlayer, this );
	}

	if ( m_bForceUpdate || ( m_yaxis.Get() != y ) )
	{
		m_yaxis.Set( y, pPlayer, this );
	}

	if ( m_bForceUpdate || ( m_attackaxis.Get() != attack ) )
	{
		m_attackaxis.Set( attack, pPlayer, this );
	}

	if ( m_bForceUpdate || ( m_attack2axis.Get() != attack2 ) )
	{
		m_attack2axis.Set( attack2, pPlayer, this );
	}

	m_bForceUpdate = false;
}
