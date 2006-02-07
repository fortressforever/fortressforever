//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: The application object.
//
//=============================================================================//

#ifndef IHAMMER_H
#define IHAMMER_H

#include "appframework/iappsystem.h"

typedef struct tagMSG MSG;


//-----------------------------------------------------------------------------
// Interface used to drive hammer
//-----------------------------------------------------------------------------
#define INTERFACEVERSION_HAMMER	"Hammer001"
class IHammer : public IAppSystem
{
public:
	virtual bool PreTranslateMessage( MSG * pMsg ) = 0;
	virtual bool IsIdleMessage( MSG * pMsg ) = 0;
	virtual bool OnIdle( long count ) = 0;

	virtual void RunFrame() = 0;
};	

#endif // IHAMMER_H
