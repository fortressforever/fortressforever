////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2005-12-25 23:49:22 -0500 (Sun, 25 Dec 2005) $
// $LastChangedRevision: 1092 $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __FUNCTIONS_NAV_H__
#define __FUNCTIONS_NAV_H__

// Title: Functions Navigation

typedef enum
{
	NAV_ERROR_NONE = 0,
	NAV_ERROR_FILENOTFOUND,
	NAV_ERROR_WRONGVERSION,
	NAV_ERROR_NOPATH,
} Nav_Error;

// typedef: Nav_EngineFuncs_t
//		This struct defines all the function pointers that the game will fill in 
//		and give to the interface so that the interface can provide the pathfinding 
//		interface for the bot, as opposed to a bot-side path planner.
typedef struct 
{
	// Function: pfnNavInitialise
	//		This function is responsible for telling the interface to load the navigation for
	//		a particular map. May not be necessary if the game loads them by default.
	Nav_Error (*pfnNavInitialise)(const char *_mapname);

	// Function: pfnNavUpdate
	//		This function allows the nav system to do any processing at regular intervals.
	void (*pfnNavUpdate)();

	// Function: pfnNavShutdown
	//		This function allows the external nav system to shutdown if necessary.
	void (*pfnNavShutdown)();

	// Function: pfnNavPlanPath
	//		This function plans a path from point to point. This function will probably
	//		need to pass a pointer to a bot-side class that the game can call functions on and stuff.
	Nav_Error (*pfnNavPlanPath)(const float *_start, const float *_end);
} Nav_EngineFuncs_t;

#endif