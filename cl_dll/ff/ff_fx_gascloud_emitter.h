/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_fx_gascloud_emitter.h
/// @author Shawn Smith (L0ki)
/// @date May 8, 2005
/// @brief gas cloud emitter
///
/// Declaration of the gas cloud emitter particle system
/// 
/// Revisions
/// ---------
/// May 8, 2005	L0ki: Initial Creation

#ifndef FF_FX_GASCLOUD_EMITTER_H
#define FF_FX_GASCLOUD_EMITTER_H

class GasParticle : public Particle
{
public:
	GasParticle() {}

	Vector			m_vOrigin;
	Vector			m_vFinalPos;
	Vector			m_vVelocity;
	float			m_flDieTime;
	float			m_flLifetime;
	float			m_flAlpha;
	float			m_flSize;
	unsigned char	m_uchColor[3];
};

class CGasCloud : public CParticleEffect
{
public:
	DECLARE_CLASS( CGasCloud, CParticleEffect );

	static CSmartPtr<CGasCloud> Create( const char *pDebugName );

	virtual void SimulateParticles	( CParticleSimulateIterator *pIterator );
	virtual void RenderParticles	( CParticleRenderIterator *pIterator );

	void AddAttractor(Vector *F, Vector apos, Vector ppos, float scale);
	void ApplyDrag(Vector *F, Vector vel, float scale, float targetvel);

	virtual void	Update( float flTimeDelta );

	GasParticle*	AddGasParticle( const Vector &vOrigin);

	void UpdateEmitter(const Vector &vecOrigin, const Vector &vecVelocity)
	{
		m_vecOrigin = vecOrigin;
		m_vecVelocity = vecVelocity;
	}

	void SetDieTime(float flDieTime)
	{
		m_flDieTime = flDieTime;
	}

protected:
	CGasCloud( const char *pDebugName );
	virtual			~CGasCloud();

private:
	CGasCloud( const CGasCloud & );

	float m_flNearClipMin;
	float m_flNearClipMax;

	Vector	m_vecOrigin;
	Vector	m_vecVelocity;

	float	m_flDieTime;
	float	m_flNextParticle;

	static PMaterialHandle m_hMaterial;
};

#endif//FF_FX_GASCLOUD_EMITTER_H