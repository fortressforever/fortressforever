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

// ted - was trying to render the conc effect particle onto a hemisphere, but didn't finish
/*
inline void RenderParticle_ColorSizeSphere(
	ParticleDraw* pDraw,									
	const Vector &pos,
	const Vector &color,
	const float alpha,
	const float size)
{
	// Don't render totally transparent particles.
	if(alpha < 0.001f)
		return;

	CMeshBuilder *pBuilder = pDraw->GetMeshBuilder();
	if( !pBuilder )
		return;

	unsigned char ubColor[4];
	ubColor[0] = (unsigned char)RoundFloatToInt( color.x * 254.9f );
	ubColor[1] = (unsigned char)RoundFloatToInt( color.y * 254.9f );
	ubColor[2] = (unsigned char)RoundFloatToInt( color.z * 254.9f );
	ubColor[3] = (unsigned char)RoundFloatToInt( alpha * 254.9f );

	float umin = pDraw->m_pSubTexture->m_tCoordMins[0];
	float uman = pDraw->m_pSubTexture->m_tCoordMaxs[0];
	float vmin = pDraw->m_pSubTexture->m_tCoordMins[1];
	float vman = pDraw->m_pSubTexture->m_tCoordMaxs[1];

	unsigned int grid = 16;

	for(unsigned int i = 0; i < grid; i++)
	{
		for(unsigned int j = 0; j < grid; j++)
		{
		}
	}

	pBuilder->Position3f( pos.x + (-ca + sa) * size, pos.y + (-sa - ca) * size, pos.z );
	pBuilder->Color4ubv( ubColor );
	pBuilder->TexCoord2f( 0, pDraw->m_pSubTexture->m_tCoordMins[0], pDraw->m_pSubTexture->m_tCoordMaxs[1] );
	pBuilder->AdvanceVertex();

	pBuilder->Position3f( pos.x + (-ca - sa) * size, pos.y + (-sa + ca) * size, pos.z );
	pBuilder->Color4ubv( ubColor );
	pBuilder->TexCoord2f( 0, pDraw->m_pSubTexture->m_tCoordMins[0], pDraw->m_pSubTexture->m_tCoordMins[1] );
	pBuilder->AdvanceVertex();

	pBuilder->Position3f( pos.x + (ca - sa)  * size, pos.y + (sa + ca)  * size, pos.z );
	pBuilder->Color4ubv( ubColor );
	pBuilder->TexCoord2f( 0, pDraw->m_pSubTexture->m_tCoordMaxs[0], pDraw->m_pSubTexture->m_tCoordMins[1] );
	pBuilder->AdvanceVertex();

	pBuilder->Position3f( pos.x + (ca + sa)  * size, pos.y + (sa - ca)  * size, pos.z );
	pBuilder->Color4ubv( ubColor );
	pBuilder->TexCoord2f( 0, pDraw->m_pSubTexture->m_tCoordMaxs[0], pDraw->m_pSubTexture->m_tCoordMaxs[1] );
	pBuilder->AdvanceVertex();
}
*/

#endif//FF_FX_CONC_EMITTER_H
