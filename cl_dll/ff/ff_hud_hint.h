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

//using namespace vgui;

#include <vgui_controls/Panel.h>

// An individual hint. Can contain several words
// and/or tokens for key bindings
class CHint
{
public:
	CHint(bool fActive, const char *pszHint, const char *pszSound, float flStartTime)
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
	// We're only doing one hint at a time now. Yar.
	//CUtlVector< CHint >	m_hHints;

	// TODO: Hard code after tweaking/testing phase(s)
	float		m_flDuration;	// Duration of hints
	float		m_flNextHint;	// Next hint allowed
	float		m_flStarted;	// Start of hint

	bool		m_bActive;

	vgui::RichText	*m_pRichText;

public:

	CHudHint(const char *pElementName);
	virtual ~CHudHint();

	void	Init( void );
	void	VidInit( void );
	void	Paint( void );

	// Callback function for the "FF_HudHint" user message
	void	MsgFunc_FF_HudHint( bf_read &msg );

	// Manually add a hud hint
	void	AddHudHint(byte bType, unsigned short wID, const char *pszMessage, const char *pszSound);
	
};


enum HINT_TYPE
{
	HINT_GENERAL = 0,
	HINT_MAP
};