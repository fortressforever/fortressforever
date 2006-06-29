/********************************************************************
created:	2005/12/25
created:	25:12:2005   2:06
filename: 	f:\cvs\code\stats\ff_statdefs.h
file path:	f:\cvs\code\stats
file base:	ff_statdefs
file ext:	h
author:		Gavin "Mirvin_Monkey" Bramhill

purpose:	
*********************************************************************/

// Temporary fix for some linux issues
#undef STAT_MAX
#undef TIMER_MAX

// Must update ff_statslog.cpp too
enum StatisticType
{
	STAT_KILLS = 0,			// CFFPlayer::Event_Killed [ff_player.cpp]
	STAT_TEAMKILLS,			// CFFPlayer::Event_Killed [ff_player.cpp]
	STAT_DEATHS,			// CFFPlayer::Event_Killed [ff_player.cpp]
	STAT_ROUNDWINS, 
	STAT_ROUNDDRAWS, 
	STAT_ROUNDLOSSES, 
	STAT_SCORE, 
	STAT_TEAMFOR, 
	STAT_TEAMAGAINST, 

	STAT_HEALS,				// CFFPlayer::Heal [ff_player.cpp]
	STAT_CRITICALHEALS,		// CFFPlayer::Heal [ff_player.cpp]
	STAT_HPHEALED,			// CFFPlayer::Heal [ff_player.cpp]
	STAT_CURES,				// CFFPlayer::Cure [ff_player.cpp]
	STAT_INFECTIONS,		// CFFPlayer::Infect [ff_player.cpp]
	STAT_INFECTIONSPREADS, 
	STAT_INFECTIONKILLS, 
	STAT_CONCJUMPS, 
	STAT_CONCDISTANCE, 
	STAT_HANGTIME, 

	STAT_MAX
};

enum TimerType
{
	TIMER_PLAYED = 0, 

	TIMER_MAX
};
