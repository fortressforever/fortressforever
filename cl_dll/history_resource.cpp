//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Item pickup history displayed onscreen when items are picked up.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "history_resource.h"
#include "hud_macros.h"
#include <vgui_controls/Controls.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include "iclientmode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

extern ConVar hud_drawhistory_time;

DECLARE_HUDELEMENT( CHudHistoryResource );
DECLARE_HUD_MESSAGE( CHudHistoryResource, ItemPickup );
DECLARE_HUD_MESSAGE( CHudHistoryResource, AmmoDenied );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHistoryResource::CHudHistoryResource( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "HudHistoryResource" )
{	
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	m_bDoNotDraw = true;
	m_wcsAmmoFullMsg[0] = 0;
	SetHiddenBits( HIDEHUD_MISCSTATUS );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHistoryResource::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	SetPaintBackgroundEnabled( false );

	// lookup text to display for ammo full message
	wchar_t *wcs = localize()->Find("#hl2_AmmoFull");
	if (wcs)
	{
		wcsncpy(m_wcsAmmoFullMsg, wcs, sizeof(m_wcsAmmoFullMsg) / sizeof(wchar_t));
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHistoryResource::Init( void )
{
	HOOK_HUD_MESSAGE( CHudHistoryResource, ItemPickup );
	HOOK_HUD_MESSAGE( CHudHistoryResource, AmmoDenied );

	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHistoryResource::Reset( void )
{
	m_PickupHistory.RemoveAll();
	m_iCurrentHistorySlot = 0;
	m_bDoNotDraw = true;
}

//-----------------------------------------------------------------------------
// Purpose: these kept only for hl1-port compatibility
//-----------------------------------------------------------------------------
void CHudHistoryResource::SetHistoryGap( int iNewHistoryGap )
{
}

//-----------------------------------------------------------------------------
// Purpose: adds an element to the history
//-----------------------------------------------------------------------------
void CHudHistoryResource::AddToHistory( C_BaseCombatWeapon *weapon )
{
	// don't draw exhaustable weapons (grenades) since they'll have an ammo pickup icon as well
	if ( weapon->GetWpnData().iFlags & ITEM_FLAG_EXHAUSTIBLE )
		return;

	int iId = weapon->entindex();

	// don't show the same weapon twice
	for ( int i = 0; i < m_PickupHistory.Count(); i++ )
	{
		if ( m_PickupHistory[i].iId == iId )
		{
			// it's already in list
			return;
		}
	}
	
	AddIconToHistory( HISTSLOT_WEAP, iId, weapon, 0, NULL );
}

//-----------------------------------------------------------------------------
// Purpose: Add a new entry to the pickup history
//-----------------------------------------------------------------------------
void CHudHistoryResource::AddToHistory( int iType, int iId, int iCount )
{
	// Ignore adds with no count
	if ( iType == HISTSLOT_AMMO )
	{
		if ( !iCount )
			return;

		// clear out any ammo pickup denied icons, since we can obviously pickup again
		for ( int i = 0; i < m_PickupHistory.Count(); i++ )
		{
			if ( m_PickupHistory[i].type == HISTSLOT_AMMODENIED && m_PickupHistory[i].iId == iId )
			{
				// kill the old entry
				m_PickupHistory[i].DisplayTime = 0.0f;
				// change the pickup to be in this entry
				m_iCurrentHistorySlot = i;
				break;
			}
		}
	}

	AddIconToHistory( iType, iId, NULL, iCount, NULL );
}

//-----------------------------------------------------------------------------
// Purpose: Add a new entry to the pickup history
//-----------------------------------------------------------------------------
void CHudHistoryResource::AddToHistory( int iType, const char *szName, int iCount )
{
	if ( iType != HISTSLOT_ITEM )
		return;

	// Get the item's icon
	CHudTexture *i = gHUD.GetIcon( szName );
	if ( i == NULL )
		return;  

	AddIconToHistory( iType, 1, NULL, iCount, i );
}

//-----------------------------------------------------------------------------
// Purpose: adds a history icon
//-----------------------------------------------------------------------------
void CHudHistoryResource::AddIconToHistory( int iType, int iId, C_BaseCombatWeapon *weapon, int iCount, CHudTexture *icon )
{
	if ( m_bDoNotDraw )
		return;

	// Check to see if the pic would have to be drawn too high. If so, start again from the bottom
	if ( (m_flHistoryGap * m_iCurrentHistorySlot) > GetTall() )
	{
		m_iCurrentHistorySlot = 0;
	}

	// ensure the size 
	m_PickupHistory.EnsureCount(m_iCurrentHistorySlot + 1);

	// default to just writing to the first slot
	HIST_ITEM *freeslot = &m_PickupHistory[m_iCurrentHistorySlot++];

	if ( iType == HISTSLOT_AMMODENIED && freeslot->DisplayTime )
	{
		// don't override existing pickup icons with denied icons
		return;
	}

	freeslot->iId = iId;
	freeslot->icon = icon;
	freeslot->type = iType;
	freeslot->m_hWeapon  = weapon;
	freeslot->iCount = iCount;

	if (iType == HISTSLOT_AMMODENIED)
	{
		freeslot->DisplayTime = gpGlobals->curtime + (hud_drawhistory_time.GetFloat() / 2.0f);
	}
	else
	{
		freeslot->DisplayTime = gpGlobals->curtime + hud_drawhistory_time.GetFloat();
	}
}


//-----------------------------------------------------------------------------
// Purpose: Handle an item pickup event from the server
//-----------------------------------------------------------------------------
void CHudHistoryResource::MsgFunc_ItemPickup( bf_read &msg )
{
	char szName[2048];
	
	msg.ReadString( szName, sizeof(szName) );

	// Add the item to the history
	AddToHistory( HISTSLOT_ITEM, szName );
}

//-----------------------------------------------------------------------------
// Purpose: ammo denied message
//-----------------------------------------------------------------------------
void CHudHistoryResource::MsgFunc_AmmoDenied( bf_read &msg )
{
	int iAmmo = msg.ReadShort();

	// see if there are any existing ammo items of that type
	for ( int i = 0; i < m_PickupHistory.Count(); i++ )
	{
		if ( m_PickupHistory[i].type == HISTSLOT_AMMO && m_PickupHistory[i].iId == iAmmo )
		{
			// it's already in the list as a pickup, ignore
			return;
		}
	}

	// see if there are any denied ammo icons, if so refresh their timer
	{for ( int i = 0; i < m_PickupHistory.Count(); i++ )
	{
		if ( m_PickupHistory[i].type == HISTSLOT_AMMODENIED && m_PickupHistory[i].iId == iAmmo )
		{
			// it's already in the list, refresh
			m_PickupHistory[i].DisplayTime = gpGlobals->curtime + (hud_drawhistory_time.GetFloat() / 2.0f);
			return;
		}
	}}

	// add into the list
	AddToHistory( HISTSLOT_AMMODENIED, iAmmo, 0 );
}

//-----------------------------------------------------------------------------
// Purpose: If there aren't any items in the history, clear it out.
//-----------------------------------------------------------------------------
void CHudHistoryResource::CheckClearHistory( void )
{
	for ( int i = 0; i < m_PickupHistory.Count(); i++ )
	{
		if ( m_PickupHistory[i].type )
			return;
	}

	m_iCurrentHistorySlot = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHudHistoryResource::ShouldDraw( void )
{
	return ( CHudElement::ShouldDraw() );
}

//-----------------------------------------------------------------------------
// Purpose: Draw the pickup history
//-----------------------------------------------------------------------------
void CHudHistoryResource::Paint( void )
{
	if (m_bDoNotDraw)
	{
		// this is to not draw things until the first rendered
		m_bDoNotDraw = false;
		return;
	}

	if ( !m_iCurrentHistorySlot )
		return;

	int wide, tall;
	GetSize( wide, tall );

	for ( int i = 0; i < m_PickupHistory.Count(); i++ )
	{
		if ( m_PickupHistory[i].type )
		{
			m_PickupHistory[i].DisplayTime = min( m_PickupHistory[i].DisplayTime, gpGlobals->curtime + hud_drawhistory_time.GetFloat() );
			if ( m_PickupHistory[i].DisplayTime <= gpGlobals->curtime )
			{  // pic drawing time has expired
				memset( &m_PickupHistory[i], 0, sizeof(HIST_ITEM) );
				CheckClearHistory();
				continue;
			}

			float elapsed = m_PickupHistory[i].DisplayTime - gpGlobals->curtime;
			float scale = elapsed * 80;
			Color clr = gHUD.m_clrNormal;
			clr[3] = min( scale, 255 );

			bool bUseAmmoFullMsg = false;

			// get the icon and number to draw
			const CHudTexture *itemIcon = NULL;
			int iAmount = 0;

			switch ( m_PickupHistory[i].type )
			{
			case HISTSLOT_AMMO:
				{
					itemIcon = gWR.GetAmmoIconFromWeapon( m_PickupHistory[i].iId );
					iAmount = m_PickupHistory[i].iCount;
				}
				break;
			case HISTSLOT_AMMODENIED:
				{
					itemIcon = gWR.GetAmmoIconFromWeapon( m_PickupHistory[i].iId );
					iAmount = 0;
					bUseAmmoFullMsg = true;
					// display as red
					clr = gHUD.m_clrCaution;	
					clr[3] = min( scale, 255 );
				}
				break;

			case HISTSLOT_WEAP:
				{
					C_BaseCombatWeapon *pWeapon = m_PickupHistory[i].m_hWeapon;
					if ( !pWeapon )
						return;

					if ( !pWeapon->HasAmmo() )
					{
						// if the weapon doesn't have ammo, display it as red
						clr = gHUD.m_clrCaution;	
						clr[3] = min( scale, 255 );
					}

					itemIcon = pWeapon->GetSpriteInactive();
				}
				break;
			case HISTSLOT_ITEM:
				{
					if ( !m_PickupHistory[i].iId )
						continue;

					itemIcon = m_PickupHistory[i].icon;
				}
				break;
			default:
				// unknown history type
				Assert( 0 );
				break;
			}

			if ( !itemIcon )
				continue;

			int ypos = tall - (m_flHistoryGap * (i + 1));
			int xpos = wide - itemIcon->Width() - m_flIconInset;

			itemIcon->DrawSelf( xpos, ypos, clr );

			if ( iAmount )
			{
				wchar_t text[16];
				_snwprintf( text, sizeof( text ) / sizeof(wchar_t), L"%i", m_PickupHistory[i].iCount );

				// offset the number to sit properly next to the icon
				ypos -= ( surface()->GetFontTall( m_hNumberFont ) - itemIcon->Height() ) / 2;

				vgui::surface()->DrawSetTextFont( m_hNumberFont );
				vgui::surface()->DrawSetTextColor( clr );
				vgui::surface()->DrawSetTextPos( wide - m_flTextInset, ypos );

				for ( int ch = 0; text[ch] != 0; ch++ )
				{
					vgui::surface()->DrawUnicodeChar( text[ ch ] );
				}
			}
			else if ( bUseAmmoFullMsg )
			{
				// offset the number to sit properly next to the icon
				ypos -= ( surface()->GetFontTall( m_hTextFont ) - itemIcon->Height() ) / 2;

				vgui::surface()->DrawSetTextFont( m_hTextFont );
				vgui::surface()->DrawSetTextColor( clr );
				vgui::surface()->DrawSetTextPos( wide - m_flTextInset, ypos );

				for ( int ch = 0; m_wcsAmmoFullMsg[ch] != 0; ch++ )
				{
					vgui::surface()->DrawUnicodeChar( m_wcsAmmoFullMsg[ ch ] );
				}
			}
		}
	}
}


