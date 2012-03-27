/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file classmenu.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date August 15, 2005
/// @brief New class selection menu
///
/// REVISIONS
/// ---------
/// Aug 15, 2005 Mirv: First creation

#include "cbase.h"
#include "classmenu.h"
#include <networkstringtabledefs.h>
#include <cdll_client_int.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <FileSystem.h>
#include <KeyValues.h>
#include <convar.h>
#include <vgui_controls/ImageList.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/ProgressBar.h>
#include "ff_modelpanel.h"
#include "ff_frame.h"

#include <cl_dll/iviewport.h>

#include "ienginevgui.h"
#include "IGameUIFuncs.h"
#include <igameresources.h>

#include "ff_playerclass_parse.h"
#include "ff_weapon_parse.h"
#include "ff_utils.h"

#include "ff_button.h"

extern IFileSystem **pFilesystem;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
extern INetworkStringTable *g_pStringTableInfoPanel;
extern IGameUIFuncs *gameuifuncs;

// Button names
const char *szClassButtons[] = { "scoutbutton", "sniperbutton", "soldierbutton", 
								 "demomanbutton", "medicbutton", "hwguybutton", 
								 "pyrobutton", "spybutton", "engineerbutton", 
								 "civilianbutton" };

using namespace vgui;

//=============================================================================
// A team button has the following components:
//		A list of icons
//		Text
//=============================================================================
class LoadoutLabel : public Label
{
public:
	DECLARE_CLASS_SIMPLE(LoadoutLabel, Label);

	LoadoutLabel(Panel *parent, const char *panelName, const char *text) : BaseClass(parent, panelName, text)
	{
		RemoveAllIcons();
	}

	void ApplySchemeSettings(IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		//SetTextInset(10, 7);
		SetContentAlignment(a_south);
	}

	void AddIcon(CHudTexture *pIcon)
	{
		if (!pIcon)
			return;
		
		m_Icons.AddToTail(pIcon);
		m_IconFonts.AddToTail(pIcon->hFont);
	}

	void AddIcon(CHudTexture *pIcon, vgui::HFont hFont )
	{
		m_Icons.AddToTail(pIcon);
		m_IconFonts.AddToTail(hFont);
	}
	
	void RemoveAllIcons()
	{
		m_Icons.RemoveAll();
		m_IconFonts.RemoveAll();
	}

	virtual void Paint()
	{
		int gap = 3;
		int totalWidth = 0;
		for(int i=0; i<m_Icons.Count(); i++)
		{
			CHudTexture *pIcon = m_Icons.Element(i);

			if (!pIcon)
				continue;

			vgui::HFont hFont = pIcon->hFont;

			if (m_IconFonts.IsValidIndex(i))
				hFont = m_IconFonts.Element(i);

			int width;
			if (hFont != pIcon->hFont)
			{
				char character = pIcon->cCharacterInFont;
				width = surface()->GetCharacterWidth(hFont, character);
			}
			else
			{
				width = pIcon->Width();
			}

			totalWidth += width;

			if (i>0)
				totalWidth += gap;
		}
		int xpos = (GetWide()/2.0f) - totalWidth/2;
		for(int i=0; i<m_Icons.Count(); i++)
		{
			CHudTexture *pIcon = m_Icons.Element(i);

			if (!pIcon)
				continue;

			vgui::HFont hFont = pIcon->hFont;

			if (m_IconFonts.IsValidIndex(i))
				hFont = m_IconFonts.Element(i);

			int width;
			if (hFont != pIcon->hFont)
			{
				char character = pIcon->cCharacterInFont;

				wchar_t unicode[2];
				swprintf(unicode, L"%c", character);

				surface()->DrawSetTextColor(Color(255, 255, 255, 255));
				surface()->DrawSetTextFont(hFont);
				surface()->DrawSetTextPos(xpos, 0);
				surface()->DrawUnicodeChar(unicode[0]);

				width = surface()->GetCharacterWidth(hFont, character);
			}
			else
			{
				pIcon->DrawSelf(xpos, 0, Color(255,255,255,255));
				width = pIcon->Width();
			}

			xpos += width + gap;
		}

		BaseClass::Paint();
	}

private:

	CUtlVector<CHudTexture *> m_Icons;
	CUtlVector<vgui::HFont> m_IconFonts;
};

//=============================================================================
// A team button has the following components:
//		Text
//		Progress bar
//=============================================================================
class ClassPropertiesLabel : public Label
{
public:
	DECLARE_CLASS_SIMPLE(ClassPropertiesLabel, Label);

	ClassPropertiesLabel(Panel *parent, const char *panelName, const char *text) : BaseClass(parent, panelName, text)
	{
		m_flValue = m_flMax = 0;
		m_pProgressBar = new vgui::ProgressBar( this, "ProgressBar" );
		m_pProgressBar->SetPos( 20, 1 );
		m_pProgressBar->SetSize( GetWide() - 20, GetTall() );
		m_pProgressBar->SetVisible( true );
	}

	void ApplySchemeSettings(IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		//SetTextInset(10, 7);
		SetContentAlignment(a_west);
		m_pProgressBar->SetSize( GetWide() - 100, GetTall() );
		m_pProgressBar->SetPos( 100, 0 );
	}

	void SetMaxValue( float flMaxValue )
	{
		m_flMax = flMaxValue;
	}

	void SetValue( float flValue )
	{
		m_flValue = flValue;
	}

	virtual void Paint()
	{
		m_pProgressBar->SetProgress( clamp( m_flValue / m_flMax, 0.0f, 1.0f ) );
		m_pProgressBar->SetBgColor( Color( 0,0,0, 100 ) );
		m_pProgressBar->SetFgColor( getIntensityColor((int)(m_flValue/m_flMax * 100), 255, 2, 100, 25, 50, 70, 90, 0) );

		BaseClass::Paint();
	}

private:

	float m_flMax;
	float m_flValue;

	vgui::ProgressBar *m_pProgressBar;
};

/////////////////////////////////////////////
// MouseOverButton
/////////////////////////////////////////////
class MouseOverButton : public FFButton
{
	DECLARE_CLASS_SIMPLE(MouseOverButton, FFButton);

public:

	MouseOverButton(Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget = NULL, const char *pCmd = NULL) : BaseClass(parent, panelName, text, pActionSignalTarget, pCmd)
	{
	}

	enum MouseEvent_t
	{
		MOUSE_ENTERED,
		MOUSE_EXITED,
	};

	virtual void OnCursorEntered() 
	{
		BaseClass::OnCursorEntered();

		KeyValues *msg = new KeyValues("MouseOverEvent");
		msg->SetInt("event", MOUSE_ENTERED);
		PostActionSignal(msg);
	}

	virtual void OnCursorExited()
	{
		BaseClass::OnCursorExited();

		KeyValues *msg = new KeyValues("MouseOverEvent");
		msg->SetInt("event", MOUSE_EXITED);
		PostActionSignal(msg);
	}
};

// Mulch: TODO: make this work for pheeeeeeeesh-y
CON_COMMAND( hud_reloadclassmenu, "hud_reloadclassmenu" )
{
	IViewPortPanel *pPanel = gViewPortInterface->FindPanelByName( PANEL_CLASS );

	if( !pPanel )
		return;

	CClassMenu *pClassMenu = dynamic_cast< CClassMenu * >( pPanel );
	if( !pClassMenu )
		return;

	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFileEx( enginevgui->GetPanel( PANEL_CLIENTDLL ), "resource/ClientScheme.res", "HudScheme" );

	pClassMenu->SetScheme( scheme );
	pClassMenu->SetProportional( true );
	pClassMenu->LoadControlSettings( "Resource/UI/ClassMenu.res" );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CClassMenu::CClassMenu(IViewPort *pViewPort) : Frame(NULL, PANEL_CLASS) 
{
	// initialize dialog
	m_pViewPort = pViewPort;

	m_flNextUpdate = 0;

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);

	// hide the system buttons
	SetTitleBarVisible(false);

	// info window about this class
	m_pClassInfo = new RichText(this, "ClassInfo");
	m_pClassInfo->SetVerticalScrollbar(false);
	m_pClassInfo->SetBgColor(Color(0,0,0,50));
	m_pClassInfo->SetPaintBorderEnabled(true);
	m_pClassInfo->SetPaintBackgroundEnabled(true);
	m_pClassInfo->SetPaintBackgroundType(2);

	m_pCancelButton = new FFButton(this, "CancelButton", "#FF_CANCEL");
	m_pRandomButton = new MouseOverButton(this, "RandomButton", "#FF_RANDOM", this, "RandomButton");

	m_pPrimaryGren = new LoadoutLabel(this, "PrimaryGren", "Primary");
	m_pSecondaryGren = new LoadoutLabel(this, "SecondaryGren", "Secondary");

	m_pGrenadesSection = new Section( this, "GrenadesSection" );
	m_pWeaponsSection = new Section( this, "WeaponsSection" );
	m_pClassInfoSection = new Section( this, "ClassInfoSection" );
	m_pClassRoleSection = new Section( this, "ClassRoleSection" );

	m_pClassRole = new Label( this, "ClassRole", "" );
	
	for(int i=0; i<8; i++)
	{
		m_WepSlots[i] = new LoadoutLabel(this, VarArgs("WepSlot%d", i+1), VarArgs("Weapon %d", i+1));
	}

	char *pszButtons[] = { "ScoutButton", "SniperButton", "SoldierButton", "DemomanButton", "MedicButton", "HwguyButton", "PyroButton", "SpyButton", "EngineerButton", "CivilianButton" };

	for (int iClassIndex = 0; iClassIndex < ARRAYSIZE(pszButtons); iClassIndex++)
	{
		m_pClassButtons[iClassIndex] = new MouseOverButton(this, pszButtons[iClassIndex], (const char *) NULL, this, pszButtons[iClassIndex]);
	}

	m_pSpeed = new ClassPropertiesLabel(this, "SpeedLabel", "Speed");
	m_pSpeed->SetMaxValue( 400 - 180 );
	m_pFirepower = new ClassPropertiesLabel(this, "FirepowerLabel", "Firepower");
	m_pFirepower->SetMaxValue( 100 );
	m_pHealth = new ClassPropertiesLabel(this, "HealthLabel", "Health");
	m_pHealth->SetMaxValue( 400 );

	m_pModelView = new PlayerModelPanel(this, "ClassPreview");

	// HACKHACKHACKHACK
	// The pushing and popping in m_pModelView is breaking the rendering of subsequent
	// vgui items. Therefore we are sticking this right to the front so that it is
	// rendered last.
	m_pModelView->SetZPos(50);

	LoadControlSettings("Resource/UI/ClassMenu.res");
	
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CClassMenu::~CClassMenu() 
{
}

//-----------------------------------------------------------------------------
// Purpose: Do whatever command is needed
//-----------------------------------------------------------------------------
void CClassMenu::OnCommand(const char *command) 
{
	if (Q_strcmp(command, "cancel") != 0)
	{
		engine->ClientCmd(VarArgs("class %s", command));
	}

	m_pViewPort->ShowPanel(this, false);

	BaseClass::OnCommand(command);
}

//-----------------------------------------------------------------------------
// Purpose: Nothing
//-----------------------------------------------------------------------------
void CClassMenu::SetData(KeyValues *data) 
{
}

//-----------------------------------------------------------------------------
// Purpose: Show or don't show
//-----------------------------------------------------------------------------
void CClassMenu::ShowPanel(bool bShow) 
{
	if (BaseClass::IsVisible() == bShow) 
		return;

	m_pViewPort->ShowBackGround(bShow);

	if (bShow) 
	{
		Activate();
		SetMouseInputEnabled(true);

		// Update straight away
		Update();

		MoveToFront();
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
		Reset();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Don't need anything yet
//-----------------------------------------------------------------------------
void CClassMenu::Reset() 
{
	SetClassInfoVisible(false);
}

void CClassMenu::SetClassInfoVisible( bool state )
{
	if (state == false)
		m_pModelView->Reset();

	m_pPrimaryGren->SetVisible(state);
	m_pSecondaryGren->SetVisible(state);

	for (int i=0; i<8; i++)
		m_WepSlots[i]->SetVisible(state);

	m_pSpeed->SetVisible(state);
	m_pFirepower->SetVisible(state);
	m_pHealth->SetVisible(state);
	m_pClassInfo->SetVisible(state);
	m_pClassRole->SetVisible(state);
	
	m_pGrenadesSection->SetVisible(state);
	m_pWeaponsSection->SetVisible(state);
	m_pClassInfoSection->SetVisible(state);
	m_pClassRoleSection->SetVisible(state);
}

//-----------------------------------------------------------------------------
// Purpose: Update the menu with everything
//-----------------------------------------------------------------------------
void CClassMenu::Update() 
{
	IGameResources *pGR = GameResources();

	if (!pGR) 
		return;

	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();

	if (pLocalPlayer == NULL)
		return;

	char nSpacesRemaining[10];
	UTIL_GetClassSpaces(pLocalPlayer->GetTeamNumber(), nSpacesRemaining);

	int nOptions = 0;

	for (int iClassIndex = 0; iClassIndex < 10; iClassIndex++) 
	{
		Button *pClassButton = m_pClassButtons[iClassIndex];

		switch (nSpacesRemaining[iClassIndex])
		{
		case -1:
//			pClassButton->SetVisible(false);
//			break;
		case 0:
			pClassButton->SetVisible(true);
			pClassButton->SetEnabled(false);
			break;
		default:
			pClassButton->SetVisible(true);
			pClassButton->SetEnabled(true);
			
			nOptions++;
		}
	}

	// Random button only enabled when there's more than one class
	m_pRandomButton->SetVisible((nOptions > 1));

	// Cancel only visible if they already have a class
	m_pCancelButton->SetVisible((pGR->GetClass(pLocalPlayer->entindex()) != 0));

	m_flNextUpdate = gpGlobals->curtime + 0.2f;
}

//-----------------------------------------------------------------------------
// Purpose: Give them some key control too
//-----------------------------------------------------------------------------
void CClassMenu::OnKeyCodePressed(KeyCode code) 
{
	// Show the scoreboard over this if needed
	if (engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind("showscores")) 
		gViewPortInterface->ShowPanel(PANEL_SCOREBOARD, true);
	
	if (engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind("serverinfo"))
		engine->ClientCmd( "serverinfo" );

	// Support hiding the class menu by hitting your changeclass button again like TFC
	// 0001232: Or if the user presses escape, kill the menu
	if ((engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind("changeclass")) ||
		(engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind("cancelselect"))) 
		gViewPortInterface->ShowPanel(this, false);

	// Support bring the team menu back up if the class menu is showing
	if (engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind("changeteam")) 
	{
		m_pViewPort->ShowPanel(this, false);
		engine->ClientCmd("changeteam");
	}

	BaseClass::OnKeyCodePressed(code);
}

void CClassMenu::OnKeyCodeReleased(KeyCode code)
{
	// Bug #0000524: Scoreboard gets stuck with the class menu up when you first join
	// Hide the scoreboard now
	if (engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind("showscores"))
	{
		gViewPortInterface->ShowPanel(PANEL_SCOREBOARD, false);
	}

	BaseClass::OnKeyCodeReleased(code);
}

//-----------------------------------------------------------------------------
// Purpose: Update the main display
//-----------------------------------------------------------------------------
void CClassMenu::OnMouseOverMessage(KeyValues *data)
{
	Button *pButton = (Button *) data->GetPtr("panel", NULL);

	// Could not determine where this came from
	if (pButton == NULL)
		return;

	// Get the command from this button and parse accordingly
	if (data->GetInt("event") == MouseOverButton::MOUSE_ENTERED)
	{
		UpdateClassInfo(pButton->GetCommand()->GetString("command"));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Load the correct class into the model view
//-----------------------------------------------------------------------------
void CClassMenu::UpdateClassInfo(const char *pszClassName)
{
	if (Q_stricmp(pszClassName, "randompc") == 0)
	{
		SetClassInfoVisible(false);
		return;
	}

	m_pModelView->SetClass(pszClassName);
	SetClassInfoVisible(true);
	
	// First get the class
	CBasePlayer *pLocalPlayer = CBasePlayer::GetLocalPlayer();

	if (pLocalPlayer == NULL)
		return;

	PLAYERCLASS_FILE_INFO_HANDLE hClassInfo;
	bool bReadInfo = ReadPlayerClassDataFromFileForSlot(*pFilesystem, pszClassName, &hClassInfo, g_pGameRules->GetEncryptionKey());

	if (!bReadInfo)
		return;

	const CFFPlayerClassInfo *pClassInfo = GetFilePlayerClassInfoFromHandle(hClassInfo);

	if (!pClassInfo)
		return;

	m_pSpeed->SetValue( pClassInfo->m_iSpeed - 180 );
	m_pHealth->SetValue( pClassInfo->m_iHealth + pClassInfo->m_iMaxArmour );
	m_pFirepower->SetValue( pClassInfo->m_iFirepower );

	m_pClassInfo->SetText( pClassInfo->m_szDescription );

	m_pClassRole->SetText( pClassInfo->m_szRole );

	for (int i=0; i<8; i++)
		m_WepSlots[i]->SetVisible(false);

	for (int i=0; i<pClassInfo->m_iNumWeapons && i<8; i++)
	{
		// Use the last weapon as their primary
		const char *pszWeapon = pClassInfo->m_aWeapons[i];

		// Now load the weapon info
		WEAPON_FILE_INFO_HANDLE hWeaponInfo;
		bReadInfo = ReadWeaponDataFromFileForSlot(*pFilesystem, pszWeapon, &hWeaponInfo, g_pGameRules->GetEncryptionKey());

		if (!bReadInfo)
			continue;

		const CFFWeaponInfo *pWeaponInfo = (CFFWeaponInfo *) GetFileWeaponInfoFromHandle(hWeaponInfo);

		if (!pWeaponInfo)
			continue;

		m_WepSlots[i]->SetText(pWeaponInfo->szPrintName);
		m_WepSlots[i]->RemoveAllIcons();
		m_WepSlots[i]->AddIcon( pWeaponInfo->iconInactive, vgui::scheme()->GetIScheme(GetScheme())->GetFont( "WeaponIconsClassSelect" ) );
		m_WepSlots[i]->SetVisible(true);
	}

	const char *pszPrimaryName = FF_GetPrimaryName( Class_StringToInt( pszClassName ) );
	m_pPrimaryGren->SetVisible(false);

	GRENADE_FILE_INFO_HANDLE hGrenInfo = LookupGrenadeInfoSlot(pszPrimaryName);
	if (hGrenInfo)
	{
		CFFGrenadeInfo *pGrenInfo = GetFileGrenadeInfoFromHandle(hGrenInfo);
		if (pGrenInfo)
		{
			m_pPrimaryGren->RemoveAllIcons();
			m_pPrimaryGren->AddIcon( pGrenInfo->iconAmmo );
			m_pPrimaryGren->SetText( pGrenInfo->szPrintName );
			m_pPrimaryGren->SetVisible(true);
		}
	}

	const char *pszSecondaryName = FF_GetSecondaryName( Class_StringToInt( pszClassName ) );
	m_pSecondaryGren->SetVisible(false);

	hGrenInfo = LookupGrenadeInfoSlot(pszSecondaryName);
	if (hGrenInfo)
	{
		CFFGrenadeInfo *pGrenInfo = GetFileGrenadeInfoFromHandle(hGrenInfo);
		if (pGrenInfo)
		{
			m_pSecondaryGren->RemoveAllIcons();
			m_pSecondaryGren->AddIcon( pGrenInfo->iconAmmo );
			m_pSecondaryGren->SetText( pGrenInfo->szPrintName );
			m_pSecondaryGren->SetVisible(true);
		}
	}
}