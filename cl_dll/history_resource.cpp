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
#include "vgui_controls/AnimationController.h"
#include "ammodef.h"
#include "ff_hud_boxes.h"
#include "ff_utils.h"

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
	m_bNeedsDraw = false;
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

// Need these for the texture caching
void FreeHudTextureList(CUtlDict<CHudTexture *, int>& list);
CHudTexture *FindHudTextureInDict(CUtlDict<CHudTexture *, int>& list, const char *psz);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHistoryResource::Reset( void )
{
	m_PickupHistory.RemoveAll();
	m_iCurrentHistorySlot = 0;
	m_bDoNotDraw = true;

	// --> Mirv: Get icon for each generic ammo type
	
	// Open up our dedicated ammo hud file
	CUtlDict<CHudTexture *, int> tempList;
	LoadHudTextures(tempList, "scripts/ff_hud_genericammo", NULL);

	for (int i = 0; i < MAX_AMMO_TYPES; i++)
	{
		Ammo_t *pAmmo = GetAmmoDef()->GetAmmoOfIndex(i);

		if (pAmmo && pAmmo->pName)
		{
			CHudTexture *p = FindHudTextureInDict(tempList, pAmmo->pName);
			if (p)
			{
				m_pHudAmmoTypes[i] = gHUD.AddUnsearchableHudIconToList(*p);
			}
			else
			{
				Warning("Could not find entry for %s ammo in ff_hud_genericammo\n", pAmmo->pName);
			}
		}
	}
	FreeHudTextureList(tempList);
	// <-- Mirv: Get icon for each generic ammo type
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
	return;	// |-- Mirv: Don't show any weapons as pickup icons

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

		/*// HACK HACK HACK Hide the addition of cells...
		if (iId == 4 && iCount == 3)
			return;*/

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

	// Get the item's icon
	CHudTexture *icon = gHUD.GetIcon( FF_GetAmmoName(iId) );

	AddIconToHistory( iType, iId, NULL, iCount, icon );
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
	m_bNeedsDraw = true;

	// Check to see if the pic would have to be drawn too high. If so, start again from the bottom
	if ( (m_flHistoryGap * m_iCurrentHistorySlot) > GetTall() )
	{
		m_iCurrentHistorySlot = 0;
	}

	// If the history resource is appearing, slide the hint message element down
	if ( m_iCurrentHistorySlot == 0 )
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HintMessageLower" ); 
	}

	// --> Mirv: Also limit to 8 icons
	if (m_iCurrentHistorySlot == 8)
	{
		m_iCurrentHistorySlot = 0;
	}
	// <-- Mirv

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
	char szName[1024];
	
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
	for ( int i = 0; i < m_PickupHistory.Count(); i++ )
	{
		if ( m_PickupHistory[i].type == HISTSLOT_AMMODENIED && m_PickupHistory[i].iId == iAmmo )
		{
			// it's already in the list, refresh
			m_PickupHistory[i].DisplayTime = gpGlobals->curtime + (hud_drawhistory_time.GetFloat() / 2.0f);
			m_bNeedsDraw = true;
			return;
		}
	}

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

	// Slide the hint message element back up
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HintMessageRaise" ); 
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudHistoryResource::ShouldDraw( void )
{
	return ( ( m_iCurrentHistorySlot > 0 || m_bNeedsDraw ) && CHudElement::ShouldDraw() );
}

//-----------------------------------------------------------------------------
// Purpose: Draw the pickup history
//-----------------------------------------------------------------------------
void CHudHistoryResource::Paint( void )
{
	if ( m_bDoNotDraw )
	{
		// this is to not draw things until the first rendered
		m_bDoNotDraw = false;
		return;
	}

	// set when drawing should occur
	// will be set if valid drawing does occur
	m_bNeedsDraw = false;

	int wide, tall;
	GetSize( wide, tall );

	for ( int i = 0; i < m_PickupHistory.Count(); i++ )
	{
		if ( m_PickupHistory[i].type )
		{
			m_PickupHistory[i].DisplayTime = min( m_PickupHistory[i].DisplayTime, gpGlobals->curtime + hud_drawhistory_time.GetFloat() );
			if ( m_PickupHistory[i].DisplayTime <= gpGlobals->curtime )
			{  
				// pic drawing time has expired
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
					if (!m_PickupHistory[i].icon)
						itemIcon = gWR.GetAmmoIconFromWeapon( m_PickupHistory[i].iId );
					else
						itemIcon = m_PickupHistory[i].icon;
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

			// --> Mirv: Draw proper icons

			if ( clr[3] )
			{
				// valid drawing will occur
				m_bNeedsDraw = true;
			}

			// We don't have a weapon for this item, so just show a generic one
			if (!itemIcon && m_PickupHistory[i].iId >= 0)
				itemIcon = m_pHudAmmoTypes[m_PickupHistory[i].iId];

			// these will get changed later if there is an icon
			int iconTall = surface()->GetFontTall( m_hNumberFont );
			int iconWide = 0;

			int ypos = tall - (m_flHistoryGap * (i + 1));

			if (itemIcon)
			{
				if (itemIcon->bRenderUsingFont)
				{
					iconTall = itemIcon->Height();
					iconWide = itemIcon->Width();
				}
				else
				{
					float ratio = (float)itemIcon->Width() / (float)itemIcon->Height();
					iconWide = iconTall * ratio;
				}

				int xpos = wide - iconWide - m_flIconInset;

				itemIcon->DrawSelf(xpos, ypos, iconWide, iconTall, clr);
			}
			// <-- Mirv: Draw proper icons

			if ( iAmount )
			{
				wchar_t text[16];
				_snwprintf( text, sizeof( text ) / sizeof(wchar_t), L"%i", m_PickupHistory[i].iCount );

				// offset the number to sit properly next to the icon
				ypos -= ( surface()->GetFontTall( m_hNumberFont ) - iconTall ) / 2;

				vgui::surface()->DrawSetTextFont( m_hNumberFont );
				vgui::surface()->DrawSetTextColor( clr );
				vgui::surface()->DrawSetTextPos( wide - m_flTextInset, ypos );
				vgui::surface()->DrawUnicodeString( text );
			}
			else if ( bUseAmmoFullMsg )
			{
				// offset the number to sit properly next to the icon
				ypos -= ( surface()->GetFontTall( m_hTextFont ) - iconTall ) / 2;

				vgui::surface()->DrawSetTextFont( m_hTextFont );
				vgui::surface()->DrawSetTextColor( clr );
				vgui::surface()->DrawSetTextPos( wide - m_flTextInset, ypos );
				vgui::surface()->DrawUnicodeString( m_wcsAmmoFullMsg );
			}
		}
	}
}


