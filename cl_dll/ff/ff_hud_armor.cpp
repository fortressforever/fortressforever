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
#include <vgui/ISystem.h>
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
	virtual void Reset( void );
	virtual void OnThink();
	void MsgFunc_Damage( bf_read &msg );
	virtual void Paint();

private:
	// old variables
	int		m_iArmor;

	int		m_bitsDamage;
};	

DECLARE_HUDELEMENT( CHudArmor );
DECLARE_HUD_MESSAGE( CHudArmor, Damage );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudArmor::CHudArmor( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudArmor")
{
	//SetHiddenBits(HIDEHUD_PLAYERDEAD);
	SetHiddenBits( /*HIDEHUD_HEALTH |*/ HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::Init()
{
	HOOK_HUD_MESSAGE( CHudArmor, Damage );
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::Reset()
{
	m_iArmor		= INIT_ARMOR;
	m_bitsDamage	= 0;

	SetLabelText(L"");
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
void CHudArmor::OnThink()
{
	int newArmor = 0;
	int maxArmor = 0;
	C_BasePlayer *baselocal = C_BasePlayer::GetLocalPlayer();
	if( baselocal )
	{
		C_FFPlayer *local = ToFFPlayer( baselocal );
		// Never below zero
		newArmor = max( local->GetArmor(), 0 );
		maxArmor = local->GetMaxArmor();
	}

	// Only update the fade if we've changed armor
	if ( newArmor == m_iArmor )
	{
		return;
	}

	// Get an armor percentage
	bool bUnder25Perc = ( float )( ( ( float )newArmor / ( float )maxArmor ) * 100 ) < 25;

	// Play appropriate animation whether armor has gone up or down
	if( newArmor > m_iArmor )
	{
		// Armor went up

		if( bUnder25Perc )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "ArmorIncreaseBelow25" );
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "ArmorIncrease" );
		}		
	}
	else
	{
		// Armor went down

		if( bUnder25Perc )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "ArmorBelow25" );
		}
	}

	m_iArmor = newArmor;

	SetDisplayValue( m_iArmor );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::MsgFunc_Damage( bf_read &msg )
{

	int armor = msg.ReadByte();	// armor
	int damageTaken = msg.ReadByte();	// health
	long bitsDamage = msg.ReadLong(); // damage bits
	bitsDamage; // variable still sent but not used

	Vector vecFrom;

	vecFrom.x = msg.ReadBitCoord();
	vecFrom.y = msg.ReadBitCoord();
	vecFrom.z = msg.ReadBitCoord();

	// Actually took damage?
	if ( damageTaken > 0 || armor > 0 )
	{
		if ( armor > 0 )
		{
			// start the animation
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "ArmorDamageTaken" );
		}
	}
}

void CHudArmor::Paint()
{
	if( !engine->IsInGame() )
		return;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if( !pPlayer )
		return;

	if( FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer ) )
		return;

	BaseClass::Paint();
}