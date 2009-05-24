//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef FF_BOT_TEMP_H
#define FF_BOT_TEMP_H
#ifdef _WIN32
#pragma once
#endif

#ifdef _DEBUG
// If iTeam or iClass is -1, then a team or class is randomly chosen.
CBasePlayer *BotPutInServer( bool bFrozen, int iTeam, int iClass );

void Bot_RunAll();

#endif // _DEBUG
#endif // FF_BOT_TEMP_H
