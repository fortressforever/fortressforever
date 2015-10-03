//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// ff_hud_armor.cpp
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

#define INIT_ARMOR -1

//-----------------------------------------------------------------------------
// Purpose: Health panel
//-----------------------------------------------------------------------------
class CHudArmor : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudArmor, CHudNumericDisplay );

public:
	CHudArmor( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void OnTick( void );
	virtual void Reset( void );
			void MsgFunc_Damage( bf_read &msg );
			void MsgFunc_PlayerAddArmor( bf_read &msg );
			void UpdateDisplay( void );

private:
	// old variables
	int		m_iArmor;

	int		m_bitsDamage;
};	

DECLARE_HUDELEMENT( CHudArmor );
DECLARE_HUD_MESSAGE( CHudArmor, Damage );
DECLARE_HUD_MESSAGE( CHudArmor, PlayerAddArmor );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudArmor::CHudArmor( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudArmor")
{
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED );
	//updating armor is fairly important!
	//the only reason we need the tick signal is for when we respawn and get a new armor value
	ivgui()->AddTickSignal( GetVPanel(), 250 ); 
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::Init()
{
	HOOK_HUD_MESSAGE( CHudArmor, Damage );
	HOOK_HUD_MESSAGE( CHudArmor, PlayerAddArmor );
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::Reset()
{
	BaseClass::Reset();
	m_iArmor		= INIT_ARMOR;
	m_bitsDamage	= 0;

	SetShouldDisplayValue(false);
	SetDisplayValue(m_iArmor);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::UpdateDisplay()
{
	int iArmor = 0;
	int iMaxArmor = 0;
	
	if (!m_pFFPlayer)
		return;

	// Never below zero
	iArmor = max( m_pFFPlayer->GetArmor(), 0 );
	iMaxArmor = m_pFFPlayer->GetMaxArmor();

	// Hullucination
	if (m_pFFPlayer->m_iHallucinationIndex)
	{
		iArmor = m_pFFPlayer->m_iHallucinationIndex * 4;
	}

	// Only update the fade if we've changed armor
	if ( iArmor == m_iArmor )
		return;

	// Get a health percentage
	float flArmorPercent = ( float )iArmor / ( float )iMaxArmor;

	// Play appropriate animation whether armor has gone up or down
	if( iArmor > m_iArmor )
	// Armor went up
	{
		if( flArmorPercent < 0.25f )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "ArmorIncreaseBelow25" );
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "ArmorIncrease" );
		}		
	}
	else
	// Armor went down or didn't change
	{
		if( flArmorPercent < 0.25f )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "ArmorBelow25" );
		}
	}

	m_iArmor = iArmor;

	SetDisplayValue( m_iArmor );
	SetShouldDisplayValue(true);
}

//TODO remove on tick and fix it so it updates properly all the time using msgfunc
void CHudArmor::OnTick( void )
{
	BaseClass::OnTick();

	UpdateDisplay();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::MsgFunc_Damage( bf_read &msg )
{
	int iArmorTaken = msg.ReadByte();	// armor

	// Actually took damage?
	if ( iArmorTaken > 0 )
	{
		// start the animation
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "ArmorDamageTaken" );

		//make the display update instantly when we take armour damage
		UpdateDisplay();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::MsgFunc_PlayerAddArmor( bf_read &msg )
{
	int iArmorAdded = msg.ReadByte();	// armor

	// Actually took damage?
	if ( iArmorAdded > 0 )
	{
		//make the display update instantly when we get armour
		UpdateDisplay();
	}
}