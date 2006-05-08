//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef FF_SHAREDDEFS_H
#define FF_SHAREDDEFS_H
#ifdef _WIN32
#pragma once
#endif


#define FF_PLAYER_VIEW_OFFSET	Vector( 0, 0, 53.5 )

enum FFPlayerGrenadeState
{
    FF_GREN_NONE,
    FF_GREN_PRIMEONE,
    FF_GREN_PRIMETWO
};

enum FFStatusIconTypes
{
    FF_ICON_CONCUSSION,
    FF_ICON_ONFIRE,
    FF_ICON_TRANQ,
	FF_ICON_CALTROP,
	FF_ICON_LEGSHOT,
	FF_ICON_GAS,
	FF_ICON_INFECTED,
	FF_NUMICONS
};

#endif // FF_SHAREDDEFS_H
