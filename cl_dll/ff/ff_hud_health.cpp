//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// Health.cpp
//
// implementation of CHudHealth class
//
#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"

#include "iclientmode.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/IVGUI.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

using namespace vgui;

#include "hudelement.h"
#include "hud_numericdisplay.h"

#include "ConVar.h"

#include "c_ff_player.h"
#include "ff_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INIT_HEALTH -1

//-----------------------------------------------------------------------------
// Purpose: Health panel
//-----------------------------------------------------------------------------
class CHudHealth : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudHealth, CHudNumericDisplay );

public:
	CHudHealth( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void OnTick( void );
	virtual void Reset( void );
			void MsgFunc_Damage( bf_read &msg );
			void MsgFunc_PlayerAddHealth( bf_read &msg );
			void UpdateDisplay( void );

private:
	// old variables
	int		m_iHealth;
	int		m_bitsDamage;
};	

DECLARE_HUDELEMENT( CHudHealth );
DECLARE_HUD_MESSAGE( CHudHealth, Damage );
DECLARE_HUD_MESSAGE( CHudHealth, PlayerAddHealth );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHealth::CHudHealth( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay( NULL, "HudHealth" )
{
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED );
	//updating health is fairly important!
	//the only reason we need the tick signal is for when we respawn and get a new health value
	ivgui()->AddTickSignal( GetVPanel(), 250 ); 
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::Init()
{
	HOOK_HUD_MESSAGE( CHudHealth, Damage );
	HOOK_HUD_MESSAGE( CHudHealth, PlayerAddHealth );
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::Reset()
{
	BaseClass::Reset();

	m_iHealth		= INIT_HEALTH;
	m_bitsDamage	= 0;

	SetShouldDisplayValue(false);
	SetDisplayValue( m_iHealth );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::UpdateDisplay()
{
	int iHealth = 0;
	int iMaxHealth = 0;	

	if (!m_pFFPlayer)
		return;

	// Never below zero
	iHealth = max( m_pFFPlayer->GetHealth(), 0 );
	iMaxHealth = m_pFFPlayer->GetMaxHealth();

	// Hullucination
	if (m_pFFPlayer->m_iHallucinationIndex)
	{
		iHealth = m_pFFPlayer->m_iHallucinationIndex * 4;
	}

	// Only update the fade if we've changed health
	if ( iHealth == m_iHealth )
		return;

	// Get a health percentage
	float flHealthPercent = ( float )iHealth / ( float )iMaxHealth;

	// Play appropriate animation whether health has gone up or down
	if( iHealth > m_iHealth )
	// Health went up
	{
		if( flHealthPercent < 0.25f )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HealthIncreaseBelow25" );
		}
		else
		{
			if( flHealthPercent >= 1.0f )
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HealthIncreaseAbove100" );
			else
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HealthIncrease" );
		}		
	}
	else
	// Health went down or didn't change
	{
		if( flHealthPercent < 0.25f )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HealthBelow25" );
		}
	}

	m_iHealth = iHealth;

	SetDisplayValue( m_iHealth );
	SetShouldDisplayValue(true);
}

//TODO remove on tick and fix it so it updates properly all the time using msgfunc
void CHudHealth::OnTick( void )
{
	BaseClass::OnTick();

	UpdateDisplay();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::MsgFunc_Damage( bf_read &msg )
{
	msg.ReadByte(); //waste the armour msg
	int iHealthTaken = msg.ReadByte();

	// Actually took damage?
	if ( iHealthTaken > 0 )
	{
		// start the animation
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HealthDamageTaken" );

		//make the display update instantly when we take health damage
		UpdateDisplay();
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::MsgFunc_PlayerAddHealth( bf_read &msg )
{
	int iHealthAdded = msg.ReadByte();

	// Actually took damage?
	if ( iHealthAdded > 0 )
	{
		//make the display update instantly when we get health
		UpdateDisplay();
	}
}