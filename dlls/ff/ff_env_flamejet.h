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
#include "baseparticleentity.h"

class CFFFlameJet : public CBaseParticleEntity
{
public:
	DECLARE_CLASS(CFFFlameJet, CBaseParticleEntity);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void	Spawn();
	bool			FlameEmit(bool fEmit);
	virtual void	UpdateOnRemove( void );

	// Stuff from the datatable.
public:
	CNetworkVar(unsigned int, m_fEmit);
};

#endif // FF_ENV_FLAME_H
