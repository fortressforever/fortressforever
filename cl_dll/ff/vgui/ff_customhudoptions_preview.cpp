#include "cbase.h"
#include "ff_customhudoptions_preview.h"

#include "vstdlib/icommandline.h"
using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

DEFINE_GAMEUI(CFFCustomHudPreview, CFFCustomHudPreviewPanel, ffcustomhudpreview);

//-----------------------------------------------------------------------------
// Purpose: Display the ff custom hud preview panel
//-----------------------------------------------------------------------------
CON_COMMAND(ff_customhudpreview, NULL)
{
	ffcustomhudpreview->GetPanel()->SetVisible(true);
}
	
CFFCustomHudPreviewPanel::CFFCustomHudPreviewPanel( vgui::VPANEL parent ) : BaseClass( NULL, "FFCustomHudPreviewPanel" )
{
	SetParent(parent);
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme");
	SetScheme( scheme );

	// Centre this panel on the screen for consistency.
	int nWide = GetWide();
	int nTall = GetTall();

	SetPos((ScreenWidth() - nWide) / 2, (ScreenHeight() - nTall) / 2);
	
	/*
	previewPanel = new FFQuantityPanel(this, "PREVIEWPANELLOLZ");
	previewPanel->SetHeaderText(L"wank");
	previewPanel->SetHeaderIconChar(";");
	FFQuantityBar* lol1 = previewPanel->AddChild("wootang");
	lol1->SetAmount(80);
	lol1->SetAmountMax(100);
	lol1->SetLabelText("sperm");
	lol1->SetIconChar(";");
	*/

	//CenterThisPanelOnScreen();//keep in mind, hl2 supports widescreen 
	SetVisible(false); //made visible on command later 

	//Other useful options
	SetSizeable(false);
	LoadControlSettings("resource/ui/FFCustomHudPreview.res");
	SetMoveable(true);
}

void CFFCustomHudPreviewPanel::SetVisible(bool state)
{
	if (state)
	{
		RequestFocus();
		MoveToFront();
	}

	BaseClass::SetVisible(state);
}