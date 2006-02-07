//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side CTFTeam class
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_FF_TEAM_H
#define C_FF_TEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "c_team.h"
#include "shareddefs.h"

class C_BaseEntity;
class C_BaseObject;
class CBaseTechnology;

//-----------------------------------------------------------------------------
// Purpose: TF's Team manager
//-----------------------------------------------------------------------------
class C_FFTeam : public C_Team
{
	DECLARE_CLASS( C_FFTeam, C_Team );
	DECLARE_CLIENTCLASS();

public:

					C_FFTeam();
	virtual			~C_FFTeam();
};


#endif // C_FF_TEAM_H
