// =============== Fortress Forever ===============
// ======== A modification for Half-Life 2 ========
//
// @file d:\ffsrc\cl_dll\ff_fx_infection.h
// @author Patrick O'Leary (Mulchman)
// @date 8/15/2006
// @brief infection particle effects
//
// particle manager for the infection particles

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

/*
class CInfectionEmitter : public CParticleEffect
{
public:
	DECLARE_CLASS(CInfectionEmitter, CParticleEffect);

	static CSmartPtr<CInfectionEmitter> Create(const char *pDebugName);

	virtual void SimulateParticles(CParticleSimulateIterator *pIterator);
	virtual void RenderParticles(CParticleRenderIterator *pIterator);

	void AddAttractor(Vector *F, Vector apos, Vector ppos, float scale);
	void ApplyDrag(Vector *F, Vector vel, float scale, float targetvel);

	InfectionParticle* AddInfection(const Vector &vOrigin);

protected:
	CInfectionEmitter(const char *pDebugName);
	virtual ~CInfectionEmitter();

private:
	CInfectionEmitter(const CInfectionEmitter &o);

	static PMaterialHandle m_hMaterial;
};
*/

class CInfectionEmitter : public CSimpleEmitter
{
public:
	DECLARE_CLASS( CInfectionEmitter, CSimpleEmitter );

	static CSmartPtr< CInfectionEmitter > Create( const char *pDebugName );

	virtual void RenderParticles( CParticleRenderIterator *pIterator );
	virtual void SimulateParticles( CParticleSimulateIterator *pIterator );

	InfectionParticle *AddInfectionParticle( const Vector& vecOrigin );

	static PMaterialHandle GetMaterial( void ) { return m_hMaterial; }

protected:
	CInfectionEmitter( const char *pDebugName );
	virtual ~CInfectionEmitter( void );

private:
	CInfectionEmitter( const CInfectionEmitter & );

	static PMaterialHandle m_hMaterial;

};

#endif // FF_FX_INFECTION_H
