//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef CITADEL_EFFECTS_SHARED_H
#define CITADEL_EFFECTS_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#define	SF_ENERGYCORE_NO_PARTICLES	(1<<0)
#define	SF_ENERGYCORE_START_ON		(1<<1)

enum
{
	ENERGYCORE_STATE_OFF,
	ENERGYCORE_STATE_CHARGING,
	ENERGYCORE_STATE_DISCHARGING,
};

#endif // CITADEL_EFFECTS_SHARED_H
