//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: AI system that makes NPCs verbally respond to game events
//
//=============================================================================//

#ifndef	AI_EVENTRESPONSE_H
#define	AI_EVENTRESPONSE_H

#include "utldict.h"

#define NPCEVENTRESPONSE_DISTANCE_SQR		(768 * 768) // Maximum distance for responding to NPCs
#define NPCEVENTRESPONSE_REFIRE_TIME		15.0		// Time after giving a response before giving any more
#define NPCEVENTRESPONSE_GIVEUP_TIME		4.0			// Time after a response trigger was fired before discarding it without responding

//-----------------------------------------------------------------------------
// Purpose: AI system that makes NPCs verbally respond to game events
//-----------------------------------------------------------------------------
class CNPCEventResponseSystem : public CAutoGameSystemPerFrame
{
public:
	CNPCEventResponseSystem( char const *name ) : CAutoGameSystemPerFrame( name )
	{
	}

	void	LevelInitPreEntity();
	void	FrameUpdatePreEntityThink();
	void	TriggerEvent( const char *pResponse, bool bForce = false );

private:
	float	m_flNextEventPoll;

	struct storedevent_t
	{
		float	flEventTime;
		float	flNextResponseTime;
		bool	bForce;
		bool	bPreventExpiration;
	};

	typedef CUtlDict< storedevent_t, int > EventMap;
	EventMap m_ActiveEvents;
};

CNPCEventResponseSystem	*NPCEventResponse();

#endif // AI_EVENTRESPONSE_H