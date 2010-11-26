//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_statusicons.cpp
//	@author Kevin Hjelden (FryGuy)
//	@date 05/18/2005
//	@brief Client-side Status Icons (conc, gassed, etc)
//
//	REVISIONS
//	---------
//	05/18/2005, FryGuy:
//		Initial copy.
//	15/08/2006, Mirv:
//		Rewritten most of this now.

#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "engine/IVDebugOverlay.h"

using namespace vgui;

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>

#include "c_ff_player.h"
#include "ff_utils.h"

// |-- Mirv: Store all the references to the textures up here now
const char *szIcons[FF_STATUSICON_MAX] = {	"concussion",		// FF_STATUSICON_CONCUSSION
											"infection",		// FF_STATUSICON_INFECTION
											"leginjury",		// FF_STATUSICON_LEGINJURY
											"tranquilized",		// FF_STATUSICON_TRANQUILIZED
											"hallucinations",	// FF_STATUSICON_HALLUCINATIONS
											"burning",			// FF_STATUSICON_BURNING
											"drowning",			// FF_STATUSICON_DROWNING
											"radiation",		// FF_STATUSICON_RADIATION
											"cold",				// FF_STATUSICON_COLD
											"immune",			// FF_STATUSICON_IMMUNE
											"radiotag",			// FF_STATUSICON_RADIOTAG
											"burning1",			// FF_STATUSICON_BURNING1
											"burning2",			// FF_STATUSICON_BURNING2
											"burning3",			// FF_STATUSICON_BURNING3
											"lockedon",			// FF_STATUSICON_LOCKEDON
											"sliding",			// FF_STATUSICON_SLIDING
											"slowmotion",		// FF_STATUSICON_SLOWMOTION
										};

struct statusicon_t
{
	CHudTexture	*pTexture;
	float		m_flStart;
	float		m_flDuration;
};

class CStatusIcons : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CStatusIcons, vgui::Panel );

	statusicon_t sStatusIcons[FF_STATUSICON_MAX];

protected:
	void	CacheTextures( void );

public:

	CStatusIcons( const char *pElementName );

	void	Init( void );
	void	VidInit( void );
	void	Paint( void );
	void	OnTick( void );

	// Callback function for the "StatusIconUpdate" user message
	void	MsgFunc_StatusIconUpdate(bf_read &msg);

	// for specific status icon colors (and to potentially make them pulsate)
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	Color	m_clrBright;
	Color	m_clrDefault;
	Color	m_clrDim;
	Color	m_clrBurning1;
	Color	m_clrBurning2;
	Color	m_clrBurning3;
	Color	m_clrLockedon;

};

DECLARE_HUDELEMENT( CStatusIcons );
DECLARE_HUD_MESSAGE( CStatusIcons, StatusIconUpdate );

static CStatusIcons *g_pStatusIcons = NULL;

CStatusIcons::CStatusIcons( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudStatusIcons" )
{
	// Set our parent window
	SetParent( g_pClientMode->GetViewport( ) );

	// Hide when player is dead
	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	// initialize the status icons
	for (int i = 0; i < FF_STATUSICON_MAX; i++)
	{
		sStatusIcons[i].pTexture = NULL;
		sStatusIcons[i].m_flStart = sStatusIcons[i].m_flDuration = 0.0f;
	}

	vgui::ivgui()->AddTickSignal( GetVPanel() );
};

// Need these for the texture caching
void FreeHudTextureList(CUtlDict<CHudTexture *, int>& list);
CHudTexture *FindHudTextureInDict(CUtlDict<CHudTexture *, int>& list, const char *psz);

//-----------------------------------------------------------------------------
// Purpose: Load all the status icon textures
//-----------------------------------------------------------------------------
void CStatusIcons::CacheTextures( void )
{
	// Open up our dedicated status effects hud file
	CUtlDict<CHudTexture *, int> tempList;
	LoadHudTextures(tempList, "scripts/ff_hud_statusicons", NULL);

	for (int i = 0; i < FF_STATUSICON_MAX; i++)
	{
		CHudTexture *p = FindHudTextureInDict(tempList, szIcons[i]);
		if (p)
		{
			sStatusIcons[i].pTexture = gHUD.AddUnsearchableHudIconToList(*p);
		}
		else
		{
			Warning("Could not find entry for %s status effect in ff_hud_statusicons\n", szIcons[i]);
		}
	}

	FreeHudTextureList(tempList);
}

//-----------------------------------------------------------------------------
// Purpose: Reset all the status icons for map change
//-----------------------------------------------------------------------------
void CStatusIcons::VidInit( void )
{	
	for (int i = 0; i < FF_STATUSICON_MAX; i++)
	{
		sStatusIcons[i].m_flStart = sStatusIcons[i].m_flDuration = 0.0f;
	}
}

void CStatusIcons::Init( void )
{
	// Cache textures
	CacheTextures();

	// Needed for elsewhere
	g_pStatusIcons = this;

	HOOK_HUD_MESSAGE( CStatusIcons, StatusIconUpdate );
}

void CStatusIcons::OnTick( void )
{
/*	int iWide, iTall;
	surface()->GetScreenSize( iWide, iTall );

	int displayed = 0;
	for (int i=0; i<MAX_STATUSICONS; i++) 
	{
		if (m_hIcons[i].enabled) 
		{
			if (((gpGlobals->curtime - m_hIcons[i].start) <= m_hIcons[i].duration) || (m_hIcons[i].duration == -1))
			{
				displayed++;
			} 
			else 
			{
				// no time left.. remove it
				m_hIcons[i].enabled = false;
			}
		}
	}

	// resize the window
	if (displayed) 
	{
		SetVisible( true );
		SetPos(iWide/2 - displayed*20, iTall - 50);
		SetWide( displayed*40 );
		SetTall( 40 );
	} 
	else 
	{
		SetVisible( false );
	}
*/
}

//-----------------------------------------------------------------------------
// Purpose: Set the status of the correct status icon(s)
//-----------------------------------------------------------------------------
void CStatusIcons::MsgFunc_StatusIconUpdate( bf_read &msg )
{
	int iStatusIcon = msg.ReadByte();
	float flDuration = msg.ReadFloat();

	// Invalid
	if (iStatusIcon < 0 || iStatusIcon > FF_STATUSICON_MAX)
	{
		AssertMsg(0, "Invalid status icon");
		return;
	}

	// Mirv: if they select FF_STATUSICON_MAX, set all (useful for removing all)
	if (iStatusIcon == FF_STATUSICON_MAX)
	{
		for (int i = 0; i < FF_STATUSICON_MAX; i++)
		{
			sStatusIcons[i].m_flStart = gpGlobals->curtime;
			sStatusIcons[i].m_flDuration = flDuration;
		}

		return;
	}

	sStatusIcons[iStatusIcon].m_flStart = gpGlobals->curtime;
	sStatusIcons[iStatusIcon].m_flDuration = flDuration;
}

void CStatusIcons::Paint( void )
{
	int iOffset = 0;

	for (int i = 0; i < FF_STATUSICON_MAX; i++)
	{
		statusicon_t &sIcon = sStatusIcons[i];

		float flTimeLeft = sIcon.m_flStart + sIcon.m_flDuration - gpGlobals->curtime;

		// This icon is not active. If the duration is negative then we allow it to
		// stay on as long as it likes.
		if (flTimeLeft <= 0 && sIcon.m_flDuration > -0.1f)
			continue;

		// This icon has no texture, that's pretty bad news
		if (!sIcon.pTexture)
			continue;

		// We're going vertical now
		//sIcon.pTexture->DrawSelf(0, iOffset, gHUD.m_clrNormal);
		// using specific status icon colors now
		if (i == FF_STATUSICON_BURNING1)
			sIcon.pTexture->DrawSelf(0, iOffset, m_clrBurning1 );
		else if (i == FF_STATUSICON_BURNING2)
			sIcon.pTexture->DrawSelf(0, iOffset, m_clrBurning2 );
		else if (i == FF_STATUSICON_BURNING3)
			sIcon.pTexture->DrawSelf(0, iOffset, m_clrBurning3 );
		else if (i == FF_STATUSICON_LOCKEDON)
			sIcon.pTexture->DrawSelf(0, iOffset, m_clrLockedon );
		else
			sIcon.pTexture->DrawSelf(0, iOffset, m_clrDefault );
		// TODO: make the status icons pulsate (and possibly pulsate faster as the end of duration nears)

		iOffset += sIcon.pTexture->Height() + 5.0f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: So we can reset all these from elsewhere (e.g. player death)
//-----------------------------------------------------------------------------
void ClearStatusIcons()
{
	if (!g_pStatusIcons)
	{
		Assert(0);
		return;
	}
    
	for (int i = 0; i < FF_STATUSICON_MAX; i++)
	{
		g_pStatusIcons->sStatusIcons[i].m_flStart = gpGlobals->curtime;
		g_pStatusIcons->sStatusIcons[i].m_flDuration = 0.0f;
	}
}

// Planning to give the status icons a little "pulse" action
void CStatusIcons::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_clrBright = pScheme->GetColor( "HUD_Status_Bright", Color(109, 124, 142, 224));
	m_clrDefault = pScheme->GetColor("HUD_Status_Default", Color(109, 124, 142, 160));
	m_clrDim = pScheme->GetColor("HUD_Status_Dim", Color(109, 124, 142, 96));
	m_clrBurning1 = pScheme->GetColor("HUD_Status_Burning1", Color(255, 255, 0, 96));
	m_clrBurning2 = pScheme->GetColor("HUD_Status_Burning2", Color(255, 96, 0, 96));
	m_clrBurning3 = pScheme->GetColor("HUD_Status_Burning3", Color(255, 0, 0, 96));
	m_clrLockedon = pScheme->GetColor("HUD_Status_Lockedon", Color(100, 32, 255, 255));
}
