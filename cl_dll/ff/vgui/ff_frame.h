/********************************************************************
	created:	2006/09/13
	created:	13:9:2006   18:30
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_frame.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_frame
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "ff_menu_panel.h"
#include "KeyValues.h"
#include "vgui_controls/Label.h"

namespace vgui
{

	//-----------------------------------------------------------------------------
	// Purpose: This has been renamed to Section
	//-----------------------------------------------------------------------------
	class Section : public FFMenuPanel
	{
		DECLARE_CLASS_SIMPLE(Section, FFMenuPanel);

	public:
		Section(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
		{
			m_pTitleLabel = new Label(this, NULL, "");
			m_pTitleLabel->SetVisible(false);
		}

		void ApplySchemeSettings(IScheme *pScheme)
		{
			BaseClass::ApplySchemeSettings(pScheme);

	//!!pcdebug		m_pTitleLabel->SetBounds(GetInsetWidth(), GetInsetWidth(), 100, surface()->GetFontTall(hFont));
			m_pTitleLabel->SetBounds(GetInsetWidth(), GetInsetWidth(), BaseClass::GetWide(), m_pTitleLabel->GetTall());
			m_pTitleLabel->SetContentAlignment(Label::a_northwest);
			SetFgColor(GetSchemeColor("HUD_Tone_Default", Color(199, 219, 255, 255), pScheme));
			SetBgColor(GetSchemeColor("HUD_BG_Default", Color(109, 124, 142, 115), pScheme));

			// Frames are always at the back of stuff
			SetZPos(-1);
		}

		void ApplySettings(KeyValues *inResourceData)
		{
			BaseClass::ApplySettings(inResourceData);

			const char *labelText =	inResourceData->GetString("titleText", NULL);

			if (labelText)
			{
				m_pTitleLabel->SetText(labelText);
				m_pTitleLabel->SetVisible(true);
			}
		}

	private:

		Label	*m_pTitleLabel;
	};

	DECLARE_BUILD_FACTORY(Section);

};