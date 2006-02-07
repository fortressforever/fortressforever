//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/Label.h"
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudHintDisplay : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudHintDisplay, vgui::Panel );

public:
	CHudHintDisplay( const char *pElementName );
	void Init();
	void Reset();
	void MsgFunc_HintText( bf_read &msg );

	bool SetHintText( const char *text );

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnThink();

private:
	CUtlVector<vgui::Label *> m_Labels;
	vgui::HFont m_hSmallFont, m_hLargeFont;
	char m_szHintText[128];

	CPanelAnimationVarAliasType( float, m_iTextX, "text_xpos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_iTextY, "text_ypos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_iTextGapX, "text_xgap", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_iTextGapY, "text_ygap", "8", "proportional_float" );
};

DECLARE_HUDELEMENT( CHudHintDisplay );
DECLARE_HUD_MESSAGE( CHudHintDisplay, HintText );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHintDisplay::CHudHintDisplay( const char *pElementName ) : BaseClass(NULL, "HudHintDisplay"), CHudElement( pElementName )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	SetVisible( false );
	m_szHintText[0] = 0;
	SetAlpha( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintDisplay::Init()
{
	HOOK_HUD_MESSAGE( CHudHintDisplay, HintText );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintDisplay::Reset()
{
	SetHintText( NULL );
	SetAlpha( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHintDisplay::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	m_hSmallFont = pScheme->GetFont( "HudHintTextSmall", true );
	m_hLargeFont = pScheme->GetFont( "HudHintTextLarge", true );

	BaseClass::ApplySchemeSettings( pScheme );
}

//-----------------------------------------------------------------------------
// Purpose: Updates the label color each frame
//-----------------------------------------------------------------------------
void CHudHintDisplay::OnThink()
{
	for (int i = 0; i < m_Labels.Count(); i++)
	{
		m_Labels[i]->SetFgColor(GetFgColor());
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets the hint text, replacing variables as necessary
//-----------------------------------------------------------------------------
bool CHudHintDisplay::SetHintText( const char *text )
{
	// clear the existing text
	for (int i = 0; i < m_Labels.Count(); i++)
	{
		m_Labels[i]->MarkForDeletion();
	}
	m_Labels.RemoveAll();

	if ( !text )
	{
		m_szHintText[0] = 0;
		return false;
	}

	strcpy( m_szHintText, text );

	// look up the text string
	wchar_t *ws = vgui::localize()->Find( text );
	if ( !ws || wcslen(ws) <= 0)
		return false;

	// parse out the text into a label set
	while ( *ws )
	{
		wchar_t token[64];
		bool isVar = false;

		// check for variables
		if ( *ws == '%' )
		{
			isVar = true;
			++ws;
		}

		// parse out the string
		wchar_t *end = wcschr( ws, '%' );
		if ( end )
		{
			wcsncpy( token, ws, end - ws );
			token[end - ws] = 0;
		}
		else
		{
			wcscpy( token, ws );
		}
		ws += wcslen( token );
		if ( isVar )
		{
			// move over the end of the variable
			++ws; 
		}

		// put it in a label
		vgui::Label *label = vgui::SETUP_PANEL(new vgui::Label(this, NULL, token));

		// modify the label if necessary
		if ( isVar )
		{
			label->SetFont( m_hLargeFont );

			// lookup key names
			char binding[64];
			vgui::localize()->ConvertUnicodeToANSI( token, binding, sizeof(binding) );

			const char *key = engine->Key_LookupBinding( *binding == '+' ? binding + 1 : binding );
			if ( !key )
			{
				key = "< not bound >";
			}

			//!! change some key names into better names
			char friendlyName[64];
			Q_snprintf( friendlyName, sizeof(friendlyName), "%s", key );
			Q_strupr( friendlyName );

			// set the new text
			label->SetText( friendlyName );
		}
		else
		{
			label->SetFont( m_hSmallFont );
		}

		label->SetPaintBackgroundEnabled( false );
		label->SetPaintBorderEnabled( false );
		label->SizeToContents();
		label->SetContentAlignment( vgui::Label::a_west );
		label->SetFgColor( GetFgColor() );
		m_Labels.AddToTail( vgui::SETUP_PANEL(label) );
	}

	// find the bounds we need to show
	int widest1 = 0, widest2 = 0;
	{for (int i = 0; i < m_Labels.Count(); i++)
	{
		vgui::Label *label = m_Labels[i];
		
		if (i & 1)
		{
			// help text
			if (label->GetWide() > widest2)
			{
				widest2 = label->GetWide();
			}
		}
		else
		{
			// variable
			if (label->GetWide() > widest1)
			{
				widest1 = label->GetWide();
			}
		}
	}}
	int tallest = vgui::surface()->GetFontTall( m_hLargeFont );

	// position the labels
	int col1_x = m_iTextX;
	int col2_x = m_iTextX + widest1 + m_iTextGapX;
	int col_y = m_iTextY;

	int col_y_extra = ((vgui::surface()->GetFontTall(m_hLargeFont) - vgui::surface()->GetFontTall(m_hSmallFont)) / 2);

	{for (int i = 0; i < m_Labels.Count(); i++)
	{
		vgui::Label *label = m_Labels[i];
		
		if (i & 1)
		{
			label->SetPos( col2_x, col_y + col_y_extra );

			col_y += tallest;
			col_y += m_iTextGapY;
		}
		else
		{
			// variable
			label->SetPos( col1_x, col_y );
		}
	}}

	// move ourselves relative to our start position
	int newWide = col2_x + widest2 + m_iTextX;
	int newTall = col_y;
	int ox, oy;
	GetPos(ox, oy);

	if (IsRightAligned())
	{
		int oldWide = GetWide();
		int diff = newWide - oldWide;
		ox -= diff;
	}

	if (IsBottomAligned())
	{
		int oldTall = GetTall();
		int diff = newTall - oldTall;
		oy -= diff;
	}

	// set the size of the hint panel to fit
	SetPos( ox, oy );
	SetSize( newWide, newTall );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Activates the hint display
//-----------------------------------------------------------------------------
void CHudHintDisplay::MsgFunc_HintText( bf_read &msg )
{
	// how many strings do we receive ?
	int count = msg.ReadByte();

	// here we expect only one string
	if ( count != 1 )
	{
		DevMsg("CHudHintDisplay::MsgFunc_HintText: string count != 1.\n");
		return;
	}

	// read the string
	char szString[2048];
	msg.ReadString( szString, sizeof(szString) );
	

	// make it visible
	if ( SetHintText( szString ) )
	{
		SetVisible( true );
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HintMessageShow" ); 
	}
	else
	{
		// it's being cleared, hide the panel
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HintMessageHide" ); 
	}
}
