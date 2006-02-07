/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_env_flame.h
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date March 11, 2004
/// @brief Declaration of the flamethrower flame entity
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First created


#ifndef FF_ENV_FLAME_H
#define FF_ENV_FLAME_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "te_effect_dispatch.h"

// Declare the flamejet entity for the server-side
class CFlameJet : public CBaseEntity
{
public:
	DECLARE_SERVERCLASS();
	DECLARE_CLASS(CFlameJet, CBaseEntity);

	void Spawn();
	
	void InputToggle(inputdata_t &input);	// Input function for toggling our effect's state

	CNetworkVar(bool, m_bEmit);		// Is it flaming

private:
	DECLARE_DATADESC();
};

#endif // FF_ENV_FLAME_H
