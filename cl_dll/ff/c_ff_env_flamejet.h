/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_env_flame.h
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date June 14, 2006
/// @brief Declaration of the flamethrower flame entity
///
/// REVISIONS
/// ---------
/// Jun 14, 2006 Mirv: First created


#ifndef C_FF_ENV_FLAME_H
#define C_FF_ENV_FLAME_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "particle_prototype.h"
#include "particle_util.h"
#include "baseparticleentity.h"
#include "dlight.h"

class C_FFFlameJet : public C_BaseParticleEntity, public IPrototypeAppEffect
{
public:
	DECLARE_CLASS(C_FFFlameJet, C_BaseParticleEntity);
	DECLARE_CLIENTCLASS();

	C_FFFlameJet();
	~C_FFFlameJet();

	//=========================================================================
	// C_FFFlameJetParticle
	//=========================================================================

	class C_FFFlameJetParticle : public Particle
	{
	public:

		int				m_Type, m_Appearance;		// Type of particle
		Vector			m_Origin, m_Velocity;					
		float			m_flRoll, m_flRollDelta;	// Rotation
		float			m_Lifetime, m_Dietime;		// To keep track
		float			m_Collisiontime;			// When particle will next hit
		// something
		unsigned char	m_uchStartSize, m_uchEndSize;
		Vector			m_HitSurfaceNormal;			// Normal of collision surface

		float			m_flRedness;
	};

	int IsEmissive() { return true; /*return (m_spawnflags & SF_EMISSIVE);*/ }

	// C_BaseEntity
public:

	virtual void		OnDataChanged(DataUpdateType_t updateType);

	// IPrototypeAppEffect
public:
	virtual void		Start(CParticleMgr *pParticleMgr, IPrototypeArgAccess *pArgs);
	virtual bool		GetPropEditInfo(RecvTable **ppTable, void **ppObj);


	// IParticleEffect
public:
	virtual void		Update(float fTimeDelta);
	virtual void		RenderParticles(CParticleRenderIterator *pIterator);
	virtual void		SimulateParticles(CParticleSimulateIterator *pIterator);

	// C_FFFlameJet-specific
	bool				FlameEmit(bool fEmit);
	void				Cleanup( void );
	void				KillDynLight( void )
	{
		if( m_pDynLight )
		{
			m_pDynLight->die = gpGlobals->curtime;
			m_pDynLight = NULL;
		}
	}
	virtual void		SetDormant( bool bDormant );

public:

public:

	int				m_fEmit;

	float			m_SpreadSpeed;
	float			m_Speed;
	float			m_StartSize;
	float			m_EndSize;
	float			m_Rate;
	float			m_Lifetime; 

private:

	CParticleMgr		*m_pParticleMgr;
	PMaterialHandle		m_hMaterialFlame, m_hMaterialSmoke;
	TimedEvent			m_ParticleSpawn;

	dlight_t			*m_pDynLight;

private:
	C_FFFlameJet(const C_FFFlameJet &);
};

#endif // C_FF_ENV_FLAME_H
