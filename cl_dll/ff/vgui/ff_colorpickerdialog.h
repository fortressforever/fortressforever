#ifndef FF_COLORPICKERDIALOG_H
#define FF_COLORPICKERDIALOG_H

#include "ff_inputslider.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IVGui.h>
#include "KeyValues.h"

namespace vgui
{
	class FFColorPickerDialog : public Frame
	{
	DECLARE_CLASS_SIMPLE(FFColorPickerDialog, Frame);
	public:
		FFColorPickerDialog(Panel *parent, const char *panelName, bool bColorModeIntensity = false);
		
		void RecalculateColorPreview();
		void OnTick( void );

		void OK();
		void Cancel();

		int GetRedComponentValue( );
		int GetGreenComponentValue( );
		int GetBlueComponentValue( );
		int GetAlphaComponentValue( );

		void SetRedComponentValue(int iRed);
		void SetGreenComponentValue(int iGreen);
		void SetBlueComponentValue(int iBlue);
		void SetAlphaComponentValue(int iAlpha);

		void SetValue( int iRed, int iGreen, int iBlue, int iAlpha );
		void GetValue( int &iRed, int &iGreen, int &iBlue, int &iAlpha );
		
		virtual void SetVisible(bool state);

	private:
		Button	*m_pOKButton;
		Button	*m_pCancelButton;

		int m_iTeamColorPreview;
		int m_iRed;
		int m_iGreen;
		int m_iBlue;
		int m_iAlpha;

		CFFInputSlider	*m_pRed;
		CFFInputSlider	*m_pGreen;
		CFFInputSlider	*m_pBlue;
		CFFInputSlider	*m_pAlpha;

		ComboBox		*m_pColorMode;

		Panel *m_pColor;
		ImagePanel *m_pColorBackground;
		
		MESSAGE_FUNC_PARAMS(OnUpdateCombos, "TextChanged", data);
		MESSAGE_FUNC_PARAMS(OnUpdateSliders, "SliderMoved", data);
		MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);
	};
};
#endif