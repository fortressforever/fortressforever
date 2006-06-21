//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CLIENTSCOREBOARDDIALOG_H
#define CLIENTSCOREBOARDDIALOG_H
#ifdef _WIN32
#pragma once
#endif

// BEG: Added by Mulchman

// NOTE there are several random and small changes in here
// due to wavelength tutorials followed by Mulchman. Be
// careful when diffing/overwriting stuff.

// END: Added by Mulchman

#include <vgui_controls/Frame.h>
#include <cl_dll/iviewport.h>
#include <igameevents.h>

/*
//#define TYPE_NOTEAM			0	// NOTEAM must be zero :)
#define TYPE_UNASSIGNED		0	// unassigned type
#define TYPE_TEAM			1	// a section for a single team	
#define TYPE_SPECTATORS		2	// a section for a spectator group
#define TYPE_BLANK			3
*/

#define TYPE_UNASSIGNED     0   
#define TYPE_TEAM           1   // a section for a single team  
#define TYPE_SPECTATORS     2   // a section for a spectator group
#define TYPE_NOTEAM         0	// NOTEAM must be zero :)

// --> Mirv: Channel images
namespace CHANNEL
{
	enum channelicons
	{
		NONE = 1,
		CHANNELA = 2,
		CHANNELB = 3,
	};
}
// <-- Mirv: Channel images

//-----------------------------------------------------------------------------
// Purpose: Game ScoreBoard
//-----------------------------------------------------------------------------
class CClientScoreBoardDialog : public vgui::Frame, public IViewPortPanel, public IGameEventListener2
{
private:
	DECLARE_CLASS_SIMPLE( CClientScoreBoardDialog, vgui::Frame );

protected:
// column widths at 640
	enum { NAME_WIDTH = 160, CLASS_WIDTH = 60, SCORE_WIDTH = 40, DEATH_WIDTH = 40, PING_WIDTH = 40, VOICE_WIDTH = 30, CHANNEL_WIDTH = 30, FRIENDS_WIDTH = 0 };
	// total = 340

public:
	CClientScoreBoardDialog( IViewPort *pViewPort );
	virtual ~CClientScoreBoardDialog();

	virtual const char *GetName( void ) { return PANEL_SCOREBOARD; }
	virtual void SetData(KeyValues *data) {};
	virtual void Reset();
	virtual void Update();
	virtual bool NeedsUpdate( void );
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
  	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }
 	
	// IGameEventListener interface:
	virtual void FireGameEvent( IGameEvent *event);
			
	virtual void OnCommand( const char *command ); // |-- Mirv: Catch channel changing


protected:
	// functions to override
	virtual bool GetPlayerScoreInfo(int playerIndex, KeyValues *outPlayerInfo);
	virtual void InitScoreboardSections();
	virtual void UpdateTeamInfo();
	virtual void UpdatePlayerInfo();
	
	virtual void AddHeader(); // add the start header of the scoreboard
	virtual int	AddSection(int teamType, int teamNumber); // add a new section header for a team

	// sorts players within a section
	static bool StaticPlayerSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	// BEG: Added by Mulchman
	// finds the player in the scoreboard
	int FindItemIDForPlayerIndex( int playerIndex );
	int FindPlayerIndexForItemID( int iItemID );
	// END: Added by Mulchman

	int m_iNumTeams;

	vgui::SectionedListPanel *m_pPlayerList;
	int				m_iSectionId; // the current section we are entering into

	int s_VoiceImage[5];
	int s_ChannelImage[5];	// |-- Mirv: Channel Images
	int TrackerImage;
	int	m_HLTVSpectators;

	vgui::Button *m_pChannelButton;	// |-- Mirv: Channel button
	vgui::Label	 *m_pMapName;		// |-- Mulch: map name

	void MoveLabelToFront( const char *textEntryName );

private:
	int			m_iPlayerIndexSymbol;
	int			m_iDesiredHeight;
	IViewPort	*m_pViewPort;
	float		m_fNextUpdateTime;

	// BEG: Added by Mulchman for stuff
	int			m_iJumpKey;

	// methods
	void FillScoreBoard();
	void RebuildScoreBoard();

protected:
	int m_iTeamSections[ TEAM_COUNT ];
	int m_iNumPlayersOnTeam[ TEAM_COUNT ];
	int m_iTeamLatency[ TEAM_COUNT ];

	virtual void OnKeyCodePressed( vgui::KeyCode code );

private:
	MESSAGE_FUNC_PARAMS( OnItemSelected, "ItemSelected", data );
	// END: Added by Mulchman for stuff
};


#endif // CLIENTSCOREBOARDDIALOG_H
