// c_ff_achievementmgr.h

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef UTLMAP_H
	#include "utlmap.h"
#endif


// these must match steamworks stats configuration for the appid
#define STAT_FLAG_CAPS_NAME		"STAT_FLAG_CAPS"
#define STAT_FLAG_TOUCHES_NAME	"STAT_FLAG_TOUCHES"

class CFFAchievementManager
{
public:
	CFFAchievementManager( );
	~CFFAchievementManager( );

	void Think();
private:
	bool m_bIsSteamworksLoaded;
};


extern CFFAchievementManager _achievementMgr;
