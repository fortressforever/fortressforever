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
	float			m_flEndPosTime;
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

	GasParticle*	AddGasParticle( const Vector &vOrigin);

protected:
	CGasCloud( const char *pDebugName );
	virtual			~CGasCloud();

private:
	CGasCloud( const CGasCloud & );

	float m_flNearClipMin;
	float m_flNearClipMax;

	static PMaterialHandle m_hMaterial;
};

#endif//FF_FX_GASCLOUD_EMITTER_H