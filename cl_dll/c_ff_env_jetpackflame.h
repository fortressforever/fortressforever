#ifndef C_FF_ENV_JETPACKFLAME_H
#define C_FF_ENV_JETPACKFLAME_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "particle_prototype.h"
#include "particle_util.h"
#include "baseparticleentity.h"
#include "dlight.h"
#include "c_ff_env_flamejet.h"

class C_FFJetpackFlame : public C_FFFlameJet
{
public:
	DECLARE_CLASS(C_FFJetpackFlame, C_FFFlameJet);
	DECLARE_CLIENTCLASS();

	C_FFJetpackFlame();
	~C_FFJetpackFlame();

	void Update(float fTimeDelta);
//private:
//		C_FFJetpackFlame(const C_FFJetpackFlame &);
};

#endif // C_FF_ENV_JETPACKFLAME_H
