/********************************************************************
	created:	2006/02/04
	created:	4:2:2006   15:58
	file ext:	h
	author:		squeek
	
	purpose:	Lua Hud Box
*********************************************************************/

#ifndef FF_LUABOX_H
#define FF_LUABOX_H

#include "cbase.h"

/*
#include <KeyValues.h>
#include <vgui/ISystem.h>
*/

#include <vgui_controls/Panel.h>
#include <vgui/IVGui.h>

namespace vgui
{
	class FFLuaBox : public Panel
	{
	private:
		DECLARE_CLASS_SIMPLE( FFLuaBox, Panel );
	public:
		FFLuaBox( Panel *parent, const char *pElementName );

		void SetBorderWidth( int iBorderWidth ) { m_iBorderWidth = iBorderWidth; }
		void SetBoxColor( Color BoxColor ) { m_BoxColor = BoxColor; }
		void SetBorderColor( Color BorderColor ) { m_BorderColor = BorderColor; }
		Color GetBoxColor() { return m_BoxColor; }
		Color GetBorderColor() { return m_BorderColor; }
		int GetBorderWidth() { return m_iBorderWidth; }

		virtual void Paint( );

	protected:
		virtual void ApplySchemeSettings( IScheme *pScheme  );

		Color m_BoxColor;
		Color m_BorderColor;
		int m_iBorderWidth;
	};
}
#endif