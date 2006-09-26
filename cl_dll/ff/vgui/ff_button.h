/********************************************************************
	created:	2006/09/22
	created:	22:9:2006   20:04
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_button.h
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_button
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#ifndef FF_BUTTON_H
#define FF_BUTTON_H

#include "vgui_controls/Button.h"
#include "keyvalues.h"
#include "ff_menu_panel.h"

namespace vgui
{
	class FFButton : public Button
	{
	public:
		DECLARE_CLASS_SIMPLE(FFButton, Button);

		//-----------------------------------------------------------------------------
		// Purpose: Create the backpanel
		//-----------------------------------------------------------------------------
		FFButton(Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget = NULL, const char *pCmd = NULL) : BaseClass(parent, panelName, text, pActionSignalTarget, pCmd)
		{
			m_pBackPanel = new FFMenuPanel(parent, NULL);

			// Make sure high enough to be clicked
			SetZPos(1);
		}

		//-----------------------------------------------------------------------------
		// Purpose: Get all the colours required and turn off borders.
		//			Make sure we're using a good border size
		//-----------------------------------------------------------------------------
		virtual void ApplySchemeSettings(IScheme *pScheme)
		{
			BaseClass::ApplySchemeSettings(pScheme);

			m_ColorBg = GetSchemeColor("FFButton.BgColor", Color(0, 50, 0, 100), pScheme);
			m_ColorBgArmed = GetSchemeColor("FFButton.BgColorArmed", Color(0, 50, 50, 100), pScheme);
			m_ColorBgPressed = GetSchemeColor("FFButton.BgColorPressed", Color(255, 50, 255, 100), pScheme);
			m_ColorBgDisabled = GetSchemeColor("FFButton.BgColorDisabled", Color(0, 50, 0, 5), pScheme);
			m_ColorFg = GetSchemeColor("FFButton.FgColor", Color(50, 0, 0, 255), pScheme);
			m_ColorFgArmed = GetSchemeColor("FFButton.FgColorArmed", Color(50, 50, 0, 255), pScheme);
			m_ColorFgPressed = GetSchemeColor("FFButton.FgColorPressed", Color(0, 0, 0, 255), pScheme);
			m_ColorFgDisabled = GetSchemeColor("FFButton.FgColorDisabled", Color(50, 0, 0, 5), pScheme);

			SetPaintBackgroundEnabled(false);
			SetPaintBorderEnabled(false);

			m_pBackPanel->SetBorderWidth(scheme()->GetProportionalScaledValue(8));
			m_pBackPanel->SetShadowOffset(scheme()->GetProportionalScaledValue(5));
		}

		//-----------------------------------------------------------------------------
		// Purpose: Make sure the backpanel follows us around
		//-----------------------------------------------------------------------------
		virtual void SetPos(int x, int y)
		{
			m_pBackPanel->SetPos(x, y);
			BaseClass::SetPos(x, y);
		}

		virtual void SetSize(int x, int y)
		{
			m_pBackPanel->SetSize(x, y);
			BaseClass::SetSize(x, y);
		}

		//-----------------------------------------------------------------------------
		// Purpose: Skip the button's background
		//-----------------------------------------------------------------------------
		virtual void Paint()
		{
			Button::BaseClass::Paint();
		}

		//-----------------------------------------------------------------------------
		// Purpose: Make sure the back panel is kept in sync with our visiblity
		//-----------------------------------------------------------------------------
		virtual void SetVisible(bool state)
		{
			m_pBackPanel->SetVisible(state);
			BaseClass::SetVisible(state);
		}

		//-----------------------------------------------------------------------------
		// Purpose: Update the backpanel's status
		//-----------------------------------------------------------------------------
		virtual void OnThink()
		{
			unsigned int iState = (IsEnabled() ? 1 : 0) + (IsVisible() ? 2 : 0) + (IsArmed() ? 4 : 0) + (IsSelected() ? 8 : 0);

			if (m_iLastState != iState)
			{
				if (!IsVisible())
				{
					m_pBackPanel->SetVisible(false);
					return;
				}

				m_pBackPanel->SetVisible(true);

				if (!IsEnabled())
				{
					m_pBackPanel->SetFgColor(m_ColorFgDisabled);
					m_pBackPanel->SetBgColor(m_ColorBgDisabled);
				}
				else if (IsSelected())
				{
					m_pBackPanel->SetFgColor(m_ColorFgPressed);
					m_pBackPanel->SetBgColor(m_ColorBgPressed);
				}
				else if (IsArmed())
				{
					m_pBackPanel->SetFgColor(m_ColorFgArmed);
					m_pBackPanel->SetBgColor(m_ColorBgArmed);
				}
				else
				{
					m_pBackPanel->SetFgColor(m_ColorFg);
					m_pBackPanel->SetBgColor(m_ColorBg);
				}

				m_pBackPanel->SetHideShadow(!IsEnabled());

				m_iLastState = iState;
			}

			BaseClass::OnThink();
		}

		//-----------------------------------------------------------------------------
		// Purpose: Pass on our settings to the frame. This might be a bit scary.
		//-----------------------------------------------------------------------------
		void ApplySettings(KeyValues *inResourceData)
		{
			const char *pszImageIcon = inResourceData->GetString("buttonIcon", NULL);

			if (pszImageIcon)
			{
				SetTextImageIndex(1);
				SetImageAtIndex(0, scheme()->GetImage(pszImageIcon, true), 10);
			}

			BaseClass::ApplySettings(inResourceData);
			m_pBackPanel->ApplySettings(inResourceData);
		}

	private:
		FFMenuPanel *m_pBackPanel;

		unsigned int m_iLastState;

		Color m_ColorBg;
		Color m_ColorBgArmed;
		Color m_ColorBgPressed;
		Color m_ColorBgDisabled;
		Color m_ColorFg;
		Color m_ColorFgArmed;
		Color m_ColorFgPressed;
		Color m_ColorFgDisabled;
	};
}

#endif