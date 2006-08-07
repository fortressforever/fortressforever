/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_fx_napalm_emitter.h
/// @author Shawn Smith (L0ki)
/// @date Apr. 30, 2005
/// @brief napalm burst emitter
///
/// Declaration of the napalm burst emitter particle system
/// 
/// Revisions
/// ---------
/// Apr. 30, 2005	L0ki: Initial Creation

#ifndef FF_FX_NAPALM_EMITTER_H
#define FF_FX_NAPALM_EMITTER_H

enum NapalmParticleType
{
	eNapalmParticle,
	eNapalmFlame,
	eHeatwave
};

class NapalmParticle : public Particle
{
public:
	NapalmParticle() {}

	NapalmParticleType m_iType;
	Vector			m_vVelocity;
	float			m_flDieTime;
	float			m_flLifetime;
	unsigned char	m_uchColor[4];
	bool			m_bStartFire;
	bool			m_bReverseSize;
	float			m_flScale;
};

class CNapalmEmitter : public CParticleEffect
{
public:
	DECLARE_CLASS( CNapalmEmitter, CParticleEffect );

	static CSmartPtr<CNapalmEmitter> Create( const char *pDebugName );

	virtual void SimulateParticles	( CParticleSimulateIterator *pIterator );
	virtual void RenderParticles	( CParticleRenderIterator *pIterator );

	NapalmParticle*	AddNapalmParticle( const Vector &vOrigin);

			void	SetGravity(const Vector &vGravity, float flGravityMagnitude);
	inline	Vector	GetGravity(void) { return m_vGravity; }
	inline	float	GetGravityMagnitude(void) { return m_flGravityMagnitude; }
	
	void StartFire(const Vector &pos);

protected:
	CNapalmEmitter( const char *pDebugName );
	virtual			~CNapalmEmitter();

private:
	CNapalmEmitter( const CNapalmEmitter & );

	void ApplyGravity(NapalmParticle *pParticle, float flTimeDelta);

	float m_flNearClipMin;
	float m_flNearClipMax;
	Vector m_vGravity;
	float m_flGravityMagnitude;

	static PMaterialHandle m_hMaterial;
	static PMaterialHandle m_hHeatwaveMaterial;
	static PMaterialHandle m_hFlameMaterial;
};

// Render a quad on the screen where you pass in color and size.
// Normal is random and "flutters"
inline void RenderParticle_ColorSizePerturbNormal(
	ParticleDraw* pDraw,									
	const Vector &pos,
	const Vector &color,
	const float alpha,
	const float size
	)
{
	// Don't render totally transparent particles.
	if( alpha < 0.001f )
		return;

	CMeshBuilder *pBuilder = pDraw->GetMeshBuilder();
	if( !pBuilder )
		return;

	unsigned char ubColor[4];
	ubColor[0] = (unsigned char)RoundFloatToInt( color.x * 254.9f );
	ubColor[1] = (unsigned char)RoundFloatToInt( color.y * 254.9f );
	ubColor[2] = (unsigned char)RoundFloatToInt( color.z * 254.9f );
	ubColor[3] = (unsigned char)RoundFloatToInt( alpha * 254.9f );

	Vector vNorm;

	vNorm.Random( -1.0f, 1.0f );

	// Add the 4 corner vertices.
	pBuilder->Position3f( pos.x-size, pos.y-size, pos.z );
	pBuilder->Color4ubv( ubColor );
	pBuilder->Normal3fv( vNorm.Base() );
	pBuilder->TexCoord2f( 0, 0, 1.0f );
	pBuilder->AdvanceVertex();

	pBuilder->Position3f( pos.x-size, pos.y+size, pos.z );
	pBuilder->Color4ubv( ubColor );
	pBuilder->Normal3fv( vNorm.Base() );
	pBuilder->TexCoord2f( 0, 0, 0 );
	pBuilder->AdvanceVertex();

	pBuilder->Position3f( pos.x+size, pos.y+size, pos.z );
	pBuilder->Color4ubv( ubColor );
	pBuilder->Normal3fv( vNorm.Base() );
	pBuilder->TexCoord2f( 0, 1.0f, 0 );
	pBuilder->AdvanceVertex();

	pBuilder->Position3f( pos.x+size, pos.y-size, pos.z );
	pBuilder->Color4ubv( ubColor );
	pBuilder->Normal3fv( vNorm.Base() );
	pBuilder->TexCoord2f( 0, 1.0f, 1.0f );
	pBuilder->AdvanceVertex();
}

#endif//FF_FX_NAPALM_EMITTER_H