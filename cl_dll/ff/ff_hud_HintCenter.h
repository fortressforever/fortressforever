//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_HintCenter.cpp
//	@author Christopher Boylan (Jiggles)
//	@date 2/8/2007
//	@brief HUD Hint/Message display center
//
//	REVISIONS
//	---------
//	2/08/2007, Jiggles:
//		First created - based significantly off of Mulch's ff_hud_hint.

#ifndef FF_HUD_HINTCENTER_H
#define FF_HUD_HINTCENTER_H

#include "hudelement.h"
#include "iclientmode.h"

//#include "ff_panel.h"

using namespace vgui;

//#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/AnimationController.h>
#include "ff_hud_menu.h"


#define SELECTION_TIMEOUT_THRESHOLD		10.0f	// Seconds
#define SELECTION_FADEOUT_TIME			1.5f
#define HINT_HISTORY					512		// Size of old hint buffer

class CHudHintCenter : public CHudElement, public vgui::Frame
{
private:
	DECLARE_CLASS_SIMPLE( CHudHintCenter, vgui::Frame );

public:
	CHudHintCenter( const char *pElementName );
	~CHudHintCenter( void );	

public:
	void	KeyDown( void );
	void	KeyUp( void );


	//void	MouseMove( float *x, float *y );
	//void	SetMenu( void );
	//void	DoCommand( const char *pszCmd );

	// Stuff we need to know
	CPanelAnimationVar( vgui::HFont, m_hIconFont, "IconFont", "HudHintCenterIcon" );
	CPanelAnimationVar( vgui::HFont, m_hIconFontGlow, "IconFontGlow", "HudHintCenterIconGlow" );
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "HUD_TextSmall" );

	CPanelAnimationVarAliasType( float, text1_xpos, "text1_xpos", "34", "proportional_float" );
	CPanelAnimationVarAliasType( float, text1_ypos, "text1_ypos", "10", "proportional_float" );
	CPanelAnimationVarAliasType( float, text1_wide, "text1_wide", "220", "proportional_float" );
	CPanelAnimationVarAliasType( float, text1_tall, "text1_tall", "40", "proportional_float" );

	CPanelAnimationVarAliasType( float, image1_xpos, "image1_xpos", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, image1_ypos, "image1_ypos", "4", "proportional_float" );

	// The Next/Previous Hint button positions/sizes
	CPanelAnimationVarAliasType( float, NextB_xpos, "NextB_xpos", "235", "proportional_float" );
	CPanelAnimationVarAliasType( float, NextB_ypos, "NextB_ypos", "55", "proportional_float" );
	CPanelAnimationVarAliasType( float, B_wide, "B_wide", "20", "proportional_float" );
	CPanelAnimationVarAliasType( float, B_tall, "B_tall", "10", "proportional_float" );

	CPanelAnimationVarAliasType( float, PrevB_xpos, "PrevB_xpos", "5", "proportional_float" );
	CPanelAnimationVarAliasType( float, PrevB_ypos, "PrevB_ypos", "55", "proportional_float" );



public:
	virtual void Init( void );
	virtual void VidInit( void );	
	virtual bool ShouldDraw( void );
	virtual void Paint( void );
	virtual bool IsVisible( void )		{ return ShouldDraw(); }
	virtual void OnThink();

	virtual void OpenSelection( void );
	virtual void HideSelection( void );


	// Callback function for the "FF_SendHint" user message
	void	MsgFunc_FF_SendHint( bf_read &msg );

	// Manually add a hud hint
	void	AddHudHint( unsigned short hintID, short iNumShow, short iPriority, const char *pszMessage );

private:

	// Maps hint text to the proper key binding
	// --For example: {+duck} to SHIFT (if user has the +duck command bound to the SHIFT key)
	bool TranslateKeyCommand( wchar_t *psHintMessage ); 

	void PerformLayout(); // Resizes text box if user changes resolution

	//MESSAGE_FUNC_PARAMS( OnButtonCommand, "Command", data );
	virtual void OnCommand(const char *command);

	bool			m_bHintCenterVisible;
	CHudTexture		*m_pHudIcon;
	CHudTexture		*m_pHudIconGlow;

	//float		m_flStartTime;	// When the message was recevied
	float		m_flLastHintDuration;	// Duration of the hint
	short		m_iLastHintPriority;		// How important was the last hint?

	RichText	*m_pRichText;				// Stores the hint text for display
	Button		*m_pNextHintButton;			// Click to display the next hint
	Button		*m_pPreviousHintButton;		// Click to display the previous hint

	CUtlVector< struct HintInfo > m_HintVector;  // Stores whether a hint has been shown yet

	CUtlVector< wchar_t * > m_HintStringsVector;   // Stores old hint strings
	int			m_iCurrentHintIndex;		   // Which hint is being shown right now?

	//CPanelAnimationVar( vgui::HFont, m_hNumberFont, "NumberFont", "HudSelectionNumbers" );
	//CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "HudSelectionText" );

	//CPanelAnimationVarAliasType( float, m_flSmallBoxSize, "SmallBoxSize", "32", "proportional_float" );
	//CPanelAnimationVarAliasType( float, m_flLargeBoxWide, "LargeBoxWide", "108", "proportional_float" );
	//CPanelAnimationVarAliasType( float, m_flLargeBoxTall, "LargeBoxTall", "72", "proportional_float" );
	//CPanelAnimationVarAliasType( float, m_flBoxGap, "BoxGap", "12", "proportional_float" );

	//CPanelAnimationVarAliasType( float, m_flSelectionNumberXPos, "SelectionNumberXPos", "4", "proportional_float" );
	//CPanelAnimationVarAliasType( float, m_flSelectionNumberYPos, "SelectionNumberYPos", "4", "proportional_float" );

	CPanelAnimationVarAliasType( float, m_flTextYPos, "TextYPos", "54", "proportional_float" );

	CPanelAnimationVar( float, m_flAlphaOverride, "Alpha", "180" );
	CPanelAnimationVar( float, m_flSelectionAlphaOverride, "SelectionAlpha", "255" );

	CPanelAnimationVar( Color, m_TextColor, "TextColor", "SelectionTextFg" );
	//CPanelAnimationVar( Color, m_NumberColor, "NumberColor", "SelectionNumberFg" );
	//CPanelAnimationVar( Color, m_EmptyBoxColor, "EmptyBoxColor", "SelectionEmptyBoxBg" );
	//CPanelAnimationVar( Color, m_BoxColor, "BoxColor", "SelectionBoxBg" );
	CPanelAnimationVar( Color, m_BGBoxColor, "BGBoxColor", "Dark" );

	CPanelAnimationVar( float, m_flWeaponPickupGrowTime, "SelectionGrowTime", "0.1" );

	CPanelAnimationVar( float, m_flTextScan, "TextScan", "1.0" );

	bool m_bFadingOut;

	float	m_flSelectionTime;

	bool m_bHintKeyHeld;

};

// Global
extern CHudHintCenter *g_pHintCenter;



// Struct to hold the hint's unique ID and Count (# of times to show hint)
struct HintInfo
{
	unsigned short	m_HintID;		// Each hint has a unique enumerated index (see ff_utils.h)
	short			m_ShowCount;	// Number of times the hint will be displayed

	HintInfo( unsigned short HintID, short ShowCount )
	{
		m_HintID = HintID;
		m_ShowCount = ShowCount;
	}

	// Needed for CUtlVector's Find function to work
	bool operator==( const HintInfo ID ) const
	{
		return ( ID.m_HintID == m_HintID );
	}
};



#endif
