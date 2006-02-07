//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "../EventLog.h"
#include "KeyValues.h"

class CFFEventLog : public CEventLog
{
private:
	typedef CEventLog BaseClass;

public:
	virtual ~CFFEventLog() {};

public:
	bool PrintEvent( IGameEvent * event )	// override virtual function
	{
		if ( BaseClass::PrintEvent( event ) )
		{
			return true;
		}
	
		if ( Q_strcmp(event->GetName(), "ff_") == 0 )
		{
			return PrintFFEvent( event );
		}

		return false;
	}

protected:

	bool PrintFFEvent( IGameEvent * event )	// print Mod specific logs
	{
		//const char * name = event->GetName() + Q_strlen("ff_"); // remove prefix
		return false;
	}

};

CFFEventLog g_FFEventLog;

//-----------------------------------------------------------------------------
// Singleton access
//-----------------------------------------------------------------------------
IGameSystem* GameLogSystem()
{
	return &g_FFEventLog;
}

