/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file teammenu2.h
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date August 15, 2005
/// @brief New team selection menu
///
/// REVISIONS
/// ---------
/// Aug 15, 2005 Mirv: First creation

#ifndef TEAMMENU_H
#define TEAMMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>

#include <cl_dll/iviewport.h>
#include <vgui/KeyCode.h>


class TeamButton;


namespace vgui
{
	class TextEntry;
	class IScheme;
	class FFButton;


	//-----------------------------------------------------------------------------
	// Purpose: displays the team menu
	//-----------------------------------------------------------------------------

	class CTeamMenu : public Frame, public IViewPortPanel
	{
	private:
		DECLARE_CLASS_SIMPLE(CTeamMenu, Frame);

	public:
		CTeamMenu(IViewPort *pViewPort);
		virtual ~CTeamMenu();

		virtual const char *GetName() { return PANEL_TEAM; }
		virtual void SetData(KeyValues *data);
		virtual void Reset();
		virtual void Update();
		virtual void ShowPanel(bool bShow);

		virtual void OnKeyCodePressed(KeyCode code);
		virtual void OnKeyCodeReleased(KeyCode code);

		virtual bool IsVisible() 						{ return BaseClass::IsVisible(); }
  		virtual void SetParent(VPANEL parent) 	{ BaseClass::SetParent(parent); }
		virtual bool NeedsUpdate() 				{ return false; }
		virtual bool HasInputElements() 			{ return true; }

		virtual void ApplySchemeSettings(IScheme *pScheme);


		// both Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
		VPANEL GetVPanel() 					{ return BaseClass::GetVPanel(); }

	private:

		void	UpdateMapDescriptionText();
		void	UpdateServerInfo();
		void	UpdateTeamButtons();

	public:

	protected:	
		// vgui overrides
		virtual void OnCommand(const char *command);

		IViewPort	*m_pViewPort;

		// ServerInfo elements
		RichText		*m_pServerInfoText;

		// MapDescription elements
		Label			*m_pMapDescriptionHead;
		RichText		*m_pMapDescriptionText;

		// ClassSelection elements
		TeamButton		*m_pTeamButtons[4];
		FFButton		*m_pSpectateButton;
		FFButton		*m_pAutoAssignButton;
		
		// Other
		FFButton		*m_pFlythroughButton;

		FFButton		*m_pMapScreenshotButton;			// Click to display the map screenshot
	};
}

#endif // TEAMMENU_H
