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

#include <vgui_controls/Frame.h>
#include <cl_dll/iviewport.h>
#include <igameevents.h>
#include "FFSectionedListPanel.h"

#define TYPE_UNASSIGNED     0   
#define TYPE_TEAM           1   // a section for a single team  
#define TYPE_SPECTATORS     2   // a section for a spectator group
#define TYPE_BLANK			3	// a blank section
#define TYPE_HEADER			4	// the main header
#define TYPE_NOTEAM         0	// NOTEAM must be zero :)

#define SCOREBOARD_NUMSECTIONS 8 // 6 teams, 1 blank section, 1 header section

namespace vgui
{
	#define SectionedListPanel FFSectionedListPanel
	class FFSectionedListPanel;
}

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

struct ScoreboardSection_s
{
	int	m_iTeam;
	int m_iLatency;
	int m_iNumPlayers;
	int m_iScore;
	float m_flLastScored;

	ScoreboardSection_s( void )
	{
		m_iTeam = -1;
		m_iLatency = 0;
		m_iNumPlayers = 0;
		m_iScore = 0;
		m_flLastScored = 0.0f;
	}

	ScoreboardSection_s &operator=( const ScoreboardSection_s& rhs )
	{
		m_iTeam = rhs.m_iTeam;
		m_iLatency = rhs.m_iLatency;
		m_iNumPlayers = rhs.m_iNumPlayers;
		m_iScore = rhs.m_iScore;
		m_flLastScored = rhs.m_flLastScored;

		return *this;
	}
};

//-----------------------------------------------------------------------------
// Purpose: Game ScoreBoard
//-----------------------------------------------------------------------------
class CClientScoreBoardDialog : public vgui::Frame, public IViewPortPanel, public IGameEventListener2
{
private:
	DECLARE_CLASS_SIMPLE( CClientScoreBoardDialog, vgui::Frame );

protected:
// column widths at 640
	enum {      NAME_WIDTH = 140, 
		        CLASS_WIDTH = 60, 
		   FORTPOINTS_WIDTH = 60, 
		        SCORE_WIDTH = 35,  
		        DEATH_WIDTH = 70, 
				ASSIST_WIDTH = 35,
		         PING_WIDTH = 30, 
		        VOICE_WIDTH = 30, 
		       CHANNEL_WIDTH = 0, 
		       FRIENDS_WIDTH = 0 };
	// total			   = 425  

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
	virtual void InitScoreboardSections( void );
	virtual void UpdatePlayerInfo( void );
	
	// Add sections to the scoreboard
	virtual void AddHeader( void );
	virtual int  AddSection( int iType, int iSection );
	virtual void UpdateHeaders( void );

	// sorts players within a section
	static bool StaticPlayerSortFunc_Score( vgui::SectionedListPanel *list, int itemID1, int itemID2 );
	static bool StaticPlayerSortFunc_Name( vgui::SectionedListPanel *list, int itemID1, int itemID2 );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	void PaintBackground();

	// BEG: Added by Mulchman
	// finds the player in the scoreboard
	int FindItemIDForPlayerIndex( int playerIndex );
	int FindPlayerIndexForItemID( int iItemID );
	// END: Added by Mulchman

	//int m_iNumTeams;

	vgui::SectionedListPanel *m_pPlayerList;

	int s_VoiceImage[5];
	int s_ChannelImage[5];	// |-- Mirv: Channel Images
	int TrackerImage;
	int	m_HLTVSpectators;

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
	void FillScoreBoard( void );
	bool NeedToSortTeams( void ) const;
	int  FindSectionByTeam( int iTeam ) const;

protected:
	ScoreboardSection_s m_hSections[ SCOREBOARD_NUMSECTIONS ];

	virtual void OnKeyCodePressed( vgui::KeyCode code );

private:
	MESSAGE_FUNC_PARAMS( OnItemSelected, "ItemSelected", data );
	// END: Added by Mulchman for stuff
};


#endif // CLIENTSCOREBOARDDIALOG_H
