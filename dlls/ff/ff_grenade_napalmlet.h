#ifndef FF_GRENADE_NAPALMLET_H
#define FF_GRENADE_NAPALMLET_H

#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "ff_grenade_base.h"

#include "ff_utils.h"
#include "EntityFlame.h"

#define NAPALMLET_MODEL "models/gibs/AGIBS.mdl"

#ifdef CLIENT_DLL
	#define CFFGrenadeNapalmlet C_FFGrenadeMirvlet
#endif

#ifdef GAME_DLL
	#include "ff_player.h"
	#include "ff_buildableobjects_shared.h"
	//#include "baseentity.h"
	//#include "ff_entity_system.h"
	//#include "te_effect_dispatch.h"

	//extern short g_sModelIndexFireball;
	//extern short g_sModelIndexWExplosion;
#endif

//=============================================================================
//
// Class CFFGrenadeNapalmlet
//
//=============================================================================
class CFFGrenadeNapalmlet : public CBaseAnimating
{
public:
	DECLARE_CLASS( CFFGrenadeNapalmlet, CBaseAnimating );
	void Precache();
	//const char *GetBounceSound() { return "MirvletGrenade.Bounce"; }


	CFFGrenadeNapalmlet( void ){m_flBurnTime = gpGlobals->curtime + 5.0f; m_flLastBurnCheck = 0; m_flBurnDamage = 15; m_bFlameSwitch = true;}
	//CFFGrenadeNapalmlet( const CFFGrenadeNapalmlet& ){}
	void UpdateOnRemove( void );

	void Spawn();
	void SetBurnRadius( float burnRadius ){ m_flBurnRadius = burnRadius;  }
	void SetBurnTime( float burnTime ){ m_flBurnTime = gpGlobals->curtime + burnTime; if (m_pFlame) m_pFlame->SetLifetime( m_flBurnTime );}
	void SetBurnDamage( float burnDamage ){ m_flBurnDamage = burnDamage; }

	
	void ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity );
	void FlameThink(void);
	DECLARE_DATADESC();
private:
	float m_flBurnTime;
	float m_flLastBurnCheck;
	float m_flBurnRadius;
	float m_flBurnDamage;
	bool m_bFlameSwitch;
	CEntityFlame *m_pFlame;
};

//#ifdef GAME_DLL
//	BEGIN_DATADESC(CFFGrenadeNapalmlet)
//		DEFINE_THINKFUNC( FlameThink ),
//	END_DATADESC()
//#endif



#endif