/********************************************************************
	purpose:	Lua Hud Box
*********************************************************************/

#include "cbase.h"
#include "ff_luabox.h"
#include <vgui/ISurface.h>

namespace vgui
{
	FFLuaBox::FFLuaBox(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
	{
		m_iBorderWidth = 0;
	}
	
	void FFLuaBox::ApplySchemeSettings( vgui::IScheme *pScheme )
	{
		BaseClass::ApplySchemeSettings( pScheme );
	}

	void FFLuaBox::Paint() 
	{
		if(m_iBorderWidth > 0)
		{
			surface()->DrawSetColor( m_BorderColor );
			for( int i = 1; i <= (m_iBorderWidth); ++i )
			{
				surface()->DrawOutlinedRect( 
					m_iBorderWidth - i, 
					m_iBorderWidth - i, 
					GetWide() - m_iBorderWidth + i,
					GetTall() - m_iBorderWidth + i
					);
			}
		}

		surface()->DrawSetColor( m_BoxColor );
		surface()->DrawFilledRect( 
			m_iBorderWidth, 
			m_iBorderWidth, 
			GetWide() - m_iBorderWidth, 
			GetTall() - m_iBorderWidth
			);

		BaseClass::Paint();
	}

}