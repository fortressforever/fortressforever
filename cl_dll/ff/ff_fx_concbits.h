/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file d:\ffsrc\cl_dll\ff_fx_concbits.h
/// @author Ted_Maul (ted_maul)
/// @date 2006/04/12
/// @brief conc particle effects
///
/// particle manager for the conc particles
#ifndef ff_fx_concbits_H
#define ff_fx_concbits_H

class ConcBitParticle : public Particle
{
public:
	ConcBitParticle() {}

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

class CConcBitsEmitter : public CParticleEffect
{
public:
	DECLARE_CLASS(CConcBitsEmitter, CParticleEffect);

	static CSmartPtr<CConcBitsEmitter> Create(const char *pDebugName);

	virtual void SimulateParticles(CParticleSimulateIterator *pIterator);
	virtual void RenderParticles(CParticleRenderIterator *pIterator);

	void AddAttractor(Vector *F, Vector apos, Vector ppos, float scale);
	void ApplyDrag(Vector *F, Vector vel, float scale, float targetvel);

	ConcBitParticle* AddConcBit(const Vector &vOrigin);

protected:
	CConcBitsEmitter(const char *pDebugName);
	virtual ~CConcBitsEmitter();

private:
	CConcBitsEmitter(const CConcBitsEmitter &o);

	static PMaterialHandle m_hMaterial;
};

#endif
