#ifndef FF_ENV_JETPACKFLAME_H
#define FF_ENV_JETPACKFLAME_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "te_effect_dispatch.h"
#include "baseparticleentity.h"
#include "ff_env_flamejet.h"

class CFFJetpackFlame : public CFFFlameJet
{
public:
	DECLARE_CLASS(CFFJetpackFlame, CFFFlameJet);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
/*
	virtual void	Spawn();
	bool			FlameEmit(bool bEmit);
	virtual void	UpdateOnRemove( void );

	// Stuff from the datatable.
public:
	CNetworkVar(unsigned int, m_bEmit);
*/
};

#endif // FF_ENV_JETPACKFLAME_H
