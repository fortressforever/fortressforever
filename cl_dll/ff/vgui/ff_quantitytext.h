/********************************************************************
	created:	2006/02/04
	created:	4:2:2006   15:58
	filename: 	F:\cvs\code\cl_dll\ff\ff_hud_quantityitem.h
	file path:	F:\cvs\code\cl_dll\ff
	file base:	ff_hud_quantityitem
	file ext:	h
	author:		Elmo
	
	purpose:	Customisable Quanitity indicator
*********************************************************************/

#ifndef FF_QUANTITYTEXT_H
#define FF_QUANTITYTEXT_H

#include "cbase.h"

#include <vgui_controls/Panel.h>
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
namespace vgui
{
	class FFQuantityText : public Panel
	{
	private:
		DECLARE_CLASS_SIMPLE( FFQuantityText, Panel );

		float m_flScaleX;
		float m_flScaleY;

		int m_iPositionX;
		int m_iPositionY;

		int m_iPositionOffsetX;
		int m_iPositionOffsetY;

		int m_iAnchorPosition;
		int m_iAnchorWidth;
		int m_iAnchorHeight;

		int m_iAlignHorizontally;
		int m_iAlignVertically;

		bool m_bShow;
		bool m_bDropShadow;

		int m_iSize;

		int m_iColorMode;
		Color m_clrCustom;
		Color m_clrText;

		HFont m_hfText;
		HFont* m_hfFamily;
		
		wchar_t* m_wszText;

		void RecalculateAnchorPosition();
		void RecalculateColor();
		void RecalculateFont();
		
		Color GetRgbColor(
			int iColorMode,
			Color &clrCustom)

	public:
		FFQuantityText(Panel *parent, const char *pElementName);

		void SetTextSize(int iSize);

		void SetTextShadow(bool bHasShadow);

		void SetTextFont(vgui::HFont font);

		void SetTextAnchorPosition(int iAnchorPosition);

		void SetTextAlignmentHorizontal(int iAlignHorizontally);
		void SetTextAlignmentVertical(int iAlignVertically);

		void SetPositionOffsetX(int iPositionOffsetX);
		void SetPositionOffsetY(int iPositionOffsetY);

		void ShowText(bool bShow);

		bool SetCustomColor(Color colour);
		bool SetColorMode(int iColorMode);
		
		void DrawText(
			wchar_t* wszText,
			HFont hfText,
			Color clrText,
			int iPositionX,
			int iPositionY);

		virtual void Paint();
		virtual void OnTick();
	};
}
#endif