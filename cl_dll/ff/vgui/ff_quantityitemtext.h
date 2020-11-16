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

#ifndef FF_QUANTITYITEMTEXT_H
#define FF_QUANTITYITEMTEXT_H

#define QUANTITYITEMTEXTSIZES 15
#define QUANTITYITEMICONSIZES 20

#include "cbase.h"

#include <vgui_controls/Panel.h>
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
namespace vgui
{
	class FFQuantityItemText : public Panel
	{
	private:
		DECLARE_CLASS_SIMPLE( FFQuantityItemText, Panel );
	public:
		FFQuantityItemText( Panel *parent, const char *pElementName );

		void SetText(char *labelText, bool bRecalculatePaintOffset = true );

		void SetSize(int iSize, bool bRecalculatePaintOffset = true );

		void SetShadow( bool bHasShadow );

		void SetFont(vgui::HFont font, bool bRecalculatePaintOffset = true );

		void SetAmountAnchorPosition( int iAmountAnchorPosition, bool bRecalculatePaintOffset = true );
		void SetIconAnchorPosition( int iIconAnchorPosition, bool bRecalculatePaintOffset = true );
		void SetLabelAnchorPosition( int iLabelAnchorPosition, bool bRecalculatePaintOffset = true );

		void SetAmountAlignmentHorizontal( int iAmountAlignHoriz, bool bRecalculatePaintOffset = true );
		void SetIconAlignmentHorizontal( int iIconAlignHoriz, bool bRecalculatePaintOffset = true );
		void SetLabelAlignmentHorizontal( int iLabelAlignHoriz, bool bRecalculatePaintOffset = true );

		void SetAmountAlignmentVertical( int iAmountAlignVert, bool bRecalculatePaintOffset = true );
		void SetIconAlignmentVertical( int iIconAlignVert, bool bRecalculatePaintOffset = true );
		void SetLabelAlignmentVertical( int iLabelAlignVert, bool bRecalculatePaintOffset = true );

		void SetPositionOffsetX( int iPositionOffsetX, bool bRecalculatePaintOffset = true );
		void SetPositionOffsetY( int iPositionOffsetY, bool bRecalculatePaintOffset = true );
		void SetPositionOffset( int iPositionOffsetX, int iPositionOffsetY, bool bRecalculatePaintOffset = true );

		void Show( bool bShow, bool bRecalculatePaintOffset = true );

		void SetColor( Color colour );

		virtual void Paint( );
		virtual void OnTick( );
		
		void SetDisabled(bool bState);
		bool IsDisabled( );
	};
}
#endif