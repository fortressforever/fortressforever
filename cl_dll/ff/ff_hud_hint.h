//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_hint.h
//	@author Patrick O'Leary (Mulchman)
//	@date 05/13/2005
//	@brief Hud Hint class - container for all active
//			hud hints - manages them all
//
//	REVISIONS
//	---------
//	05/13/2005, Mulchman: 
//		First created
//
//	07/11/2005, Mulchman:
//		Added client side ability to add hints

#include "cbase.h"
#include "hudelement.h"
#include "iclientmode.h"

using namespace vgui;

#include <vgui_controls/Panel.h>

// An individual hint. Can contain several words
// and/or tokens for key bindings
class CHint
{
public:
	CHint( const char *pszHint, float flStartTime )
		: m_flStartTime( flStartTime )
	{
		// TODO: Parse out the hint into labels
	}

	CUtlVector< vgui::Label * >	m_Labels;

	float	m_flStartTime;

};

// Hut hint class - the manager (kind of)
class CHudHint : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CHudHint, vgui::Panel );

	wchar_t		m_pText[ 256 ];	// Unicode text buffer

	// Vector of all the hints we're showing at the time
	CUtlVector< CHint >	m_hHints;

	// TODO: Hard code after tweaking/testing phase(s)
	float		m_flDuration;	// Duration of hints

public:
	CHudHint( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudHint" ) 
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport( ) );

		SetHiddenBits( 0 );
	}

	~CHudHint( void );

	void	Init( void );
	void	VidInit( void );
	void	Paint( void );

	// Callback function for the "FF_HudHint" user message
	void	MsgFunc_FF_HudHint( bf_read &msg );

	// Manually add a hud hint
	void	AddHudHint( const char *pszMessage );
	
};
