#include "cbase.h"
#include "ff_quantitypanel.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>

namespace vgui
{
	void FFQuantityPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
	{
		SetSize(vgui::scheme()->GetProportionalScaledValue(640), vgui::scheme()->GetProportionalScaledValue(480));

		m_ColorBackground = pScheme->GetColor( "HUD_BG_Default", Color(255,0,0,255) );
		m_ColorBorder = pScheme->GetColor( "HUD_Border_Default", Color(255,0,0,255) );
		m_ColorHeaderText = pScheme->GetColor( "HUD_Tone_Bright", Color(255,0,0,255) );
		m_ColorHeaderIcon = m_ColorHeaderText;
		m_ColorText = pScheme->GetColor( "HUD_Tone_Default", Color(255,0,0,255) );
		
		SetPaintBackgroundEnabled(true);
		SetPaintBorderEnabled(true);
		SetPaintEnabled(true);

		SetBgColor(Color(244,244,244,120));
		SetBorderColor(m_ColorBorder);
		SetPaintBackgroundType(1);
		SetBorder(pScheme->GetBorder("ScoreBoardItemBorder"));

		m_hfHeaderText = pScheme->GetFont( "QuantityPanelHeader", IsProportional() );
		m_hfHeaderIcon = pScheme->GetFont( "QuantityPanelHeaderIcon", IsProportional() );
		m_hfText = pScheme->GetFont( "QuantityPanel", IsProportional() );

		Panel::ApplySchemeSettings( pScheme );
	 }
	void FFQuantityPanel::Paint() 
	{
		//Paint Header Text
		vgui::surface()->DrawSetTextFont( m_hfHeaderText );
		vgui::surface()->DrawSetColor( m_ColorHeaderText.r(), m_ColorHeaderText.g(), m_ColorHeaderText.b(), m_ColorHeaderText.a() );
		vgui::surface()->DrawSetTextPos( 40, 10 );
		vgui::surface()->DrawUnicodeString( m_wszHeaderText );

		//Paint Header Icon
		vgui::surface()->DrawSetTextFont( m_hfHeaderIcon );
		vgui::surface()->DrawSetColor( m_ColorHeaderIcon.r(), m_ColorHeaderIcon.g(), m_ColorHeaderIcon.b(), m_ColorHeaderIcon.a() );
		vgui::surface()->DrawSetTextPos( 10, 10 );
		vgui::surface()->DrawUnicodeString( m_wszHeaderIcon );
	}

	void FFQuantityPanel::SetHeaderText(wchar_t *newHeaderText) { wcscpy( m_wszHeaderText, newHeaderText ); }
	void FFQuantityPanel::SetHeaderIconChar(char *newHeaderIconChar)
	{ 					
		char szIconChar[5];
		Q_snprintf( szIconChar, 2, "%s%", newHeaderIconChar );
		vgui::localize()->ConvertANSIToUnicode( szIconChar, m_wszHeaderIcon, sizeof( m_wszHeaderIcon ) );
	}

	void FFQuantityPanel::SetHeaderTextColor( Color newHeaderTextColor ) { m_ColorHeaderText = newHeaderTextColor; }
	void FFQuantityPanel::SetHeaderIconColor( Color newHeaderIconColor ) { m_ColorHeaderIcon = newHeaderIconColor; }
	void FFQuantityPanel::SetBackgroundColor( Color newBackgroundColor ) { m_ColorBackground = newBackgroundColor; }
	void FFQuantityPanel::SetBorderColor( Color newBorderColor ) { m_ColorBorder = newBorderColor; }

	void FFQuantityPanel::SetHeaderTextFont(vgui::HFont newHeaderTextFont) { m_hfHeaderText = newHeaderTextFont; }
	void FFQuantityPanel::SetHeaderIconFont(vgui::HFont newHeaderIconFont) { m_hfHeaderIcon = newHeaderIconFont; }
}