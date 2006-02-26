/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_fx_conc_emitter.h
/// @author Paul Peloski (zero)
/// @date Feb 5, 2006
/// @brief conc effect emitter
///
/// Declaration of the conc effect emitter particle system
/// 
/// Revisions
/// ---------
/// Feb 5, 2006	zero: Initial Creation

#ifndef FF_FX_CONC_EMITTER_H
#define FF_FX_CONC_EMITTER_H

class ConcParticle : public SimpleParticle
{
public:
	ConcParticle() {}
	float			m_flOffset;
	float			m_flRefract;
};

class CConcEmitter : public CSimpleEmitter
{
public:
	DECLARE_CLASS(CConcEmitter, CSimpleEmitter);

	static CSmartPtr<CConcEmitter> Create(const char *pDebugName);

	virtual void RenderParticles	 (CParticleRenderIterator *pIterator);
	virtual void SimulateParticles	 (CParticleSimulateIterator *pIterator) { CSimpleEmitter::SimulateParticles(pIterator); };

	ConcParticle * AddConcParticle();

	static PMaterialHandle GetMaterial() { return m_hMaterial; };

protected:
	CConcEmitter(const char *pDebugName);
	virtual ~CConcEmitter();

private:
	CConcEmitter(const CConcEmitter &);

	static PMaterialHandle m_hMaterial;
};

#endif//FF_FX_CONC_EMITTER_H















