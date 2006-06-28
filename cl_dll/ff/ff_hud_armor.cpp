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

	//CHudTexture	*m_pHudElementTexture;
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

/*	wchar_t *tempString = vgui::localize()->Find("#Valve_Hud_ARMOR");

	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"ARMOR");
	}*/

	SetLabelText(L"");
	SetDisplayValue(m_iArmor);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::VidInit()
{
	Reset();

	// Precache the background texture
	//m_pHudElementTexture = new CHudTexture();
	//m_pHudElementTexture->textureId = surface()->CreateNewTextureID();
	//surface()->DrawSetTextureFile(m_pHudElementTexture->textureId, "vgui/hud_armor", true, false);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::OnThink()
{
	int newArmor = 0;
	C_BasePlayer *baselocal = C_BasePlayer::GetLocalPlayer();
	if (baselocal)
	{
		C_FFPlayer *local = ToFFPlayer(baselocal);
		// Never below zero
		newArmor = max(local->GetArmor(), 0);
		/*
		float fArmorType = local->GetArmorType();
		float fScale = 255.0f/max(fArmorType, 1.0f - fArmorType);
		SetFgColor(Color(fArmorType * fScale, (1.0f - fArmorType) * fScale, 0, GetFgColor().a()));
		*/
	}

	// Only update the fade if we've changed armor
	if ( newArmor == m_iArmor )
	{
		return;
	}

	m_iArmor = newArmor;

	if ( m_iArmor >= 20 )
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ArmorIncrease");
	}
	else if ( m_iArmor > 0 )
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ArmorIncrease");
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthLow");
	}

	SetDisplayValue(m_iArmor);
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
		if ( damageTaken > 0 )
		{
			// start the animation
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ArmorIncrease");
		}
	}
}

void CHudArmor::Paint()
{
	if( C_BasePlayer::GetLocalPlayer() && ( C_BasePlayer::GetLocalPlayer()->GetTeamNumber() < TEAM_BLUE ) )
		return;

	// Bug #0000721: Switching from spectator to a team results in HUD irregularities
	C_FFPlayer *pPlayer = ToFFPlayer( C_BasePlayer::GetLocalPlayer() );
	if( pPlayer && ( ( pPlayer->GetClassSlot() < CLASS_SCOUT ) || ( pPlayer->GetClassSlot() > CLASS_CIVILIAN ) ) )
		return;

	//surface()->DrawSetTexture(m_pHudElementTexture->textureId);
	//surface()->DrawSetColor(255, 255, 255, 255);
	//surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());

	BaseClass::Paint();
}