//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#if !defined EVENTLOG_H
#define EVENTLOG_H

#ifdef _WIN32
#pragma once
#endif

#include <igameevents.h>
#include <igamesystem.h>

class CEventLog : public IGameEventListener2, public CBaseGameSystem
{
	
public:
	CEventLog();
	virtual ~CEventLog();

public: // IGameEventListener Interface
	
	virtual void FireGameEvent( IGameEvent * event );
	
public: // CBaseGameSystem overrides

	virtual bool Init();
	virtual void Shutdown();

protected:

	virtual bool PrintEvent( IGameEvent * event );
	virtual bool PrintGameEvent( IGameEvent * event );
	virtual bool PrintPlayerEvent( IGameEvent * event );
	virtual bool PrintTeamEvent( IGameEvent * event );
	virtual bool PrintOtherEvent( IGameEvent * event );
};

extern IGameSystem* GameLogSystem();

#endif // EVENTLOG_H
