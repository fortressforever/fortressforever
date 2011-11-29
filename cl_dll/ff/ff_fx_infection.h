// =============== Fortress Forever ===============
// ======== A modification for Half-Life 2 ========
//
// @file d:\ffsrc\cl_dll\ff_fx_infection.h
// @author Patrick O'Leary (Mulchman)
// @date 8/15/2006
// @brief infection particle effects
//
// particle manager for the infection particles
//
// Revisions
// ---------
// 8/15/2006, Mulchman
//		Initial version

#ifndef FF_FX_INFECTION_H
#define FF_FX_INFECTION_H

class InfectionParticle : public Particle
{
public:
	InfectionParticle( void ) { }

	Vector			m_vOrigin;
	Vector			m_vFinalPos;
	Vector			m_vVelocity;
	float			m_flEndPosTime;
	float			m_flDieTime;
	float			m_flLifetime;
	float			m_flAlpha;
	float			m_flSize;
	unsigned char	m_uchColor[3];
};

class CInfectionEmitter : public CSimpleEmitter
{
public:
	DECLARE_CLASS( CInfectionEmitter, CSimpleEmitter );

	static CSmartPtr< CInfectionEmitter > Create( const char *pDebugName );

	virtual void RenderParticles( CParticleRenderIterator *pIterator );
	virtual void SimulateParticles( CParticleSimulateIterator *pIterator );
	virtual void Update( float flTimeDelta );

	InfectionParticle *AddInfectionParticle( const Vector& vecOrigin );

	static PMaterialHandle GetMaterial( void ) { return m_hMaterial; }

	void ApplyDrag( Vector *F, Vector vecVelocity, float flScale, float flTargetVel );
	void SetDieTime( float flDieTime ) { m_flDieTime = flDieTime; }
	void UpdateEmitter( const Vector& vecOrigin, const Vector& vecVelocity )
	{
		m_vecOrigin = vecOrigin;
		m_vecVelocity = vecVelocity;
	}
	void UpdateEmitter( const Vector& vecOrigin, const Vector& vecVelocity, const int& numParticles )
	{
		m_vecOrigin = vecOrigin;
		m_vecVelocity = vecVelocity;
		m_iNumParticles = numParticles;
	}

protected:
	CInfectionEmitter( const char *pDebugName );
	virtual ~CInfectionEmitter( void );

private:
	CInfectionEmitter( const CInfectionEmitter & );

	static PMaterialHandle m_hMaterial;

	Vector	m_vecOrigin;
	Vector	m_vecVelocity;

	float	m_flDieTime;
	float	m_flNextParticle;

	int		m_iNumParticles; // number of particles to add per Update()

};

#endif // FF_FX_INFECTION_H
