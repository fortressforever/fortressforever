/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_fx_cloaksmoke_emitter.h
/// @author Greg Stefanakis (GreenMushy)
/// @date May 27, 2011
/// @brief cloak smoke particle emitter
///
/// Declaration of the cloak smoke emitter particle system
/// 
/// Revisions
/// ---------

#ifndef FF_FX_CLOAKSMOKE_EMITTER_H
#define FF_FX_CLOAKSMOKE_EMITTER_H

class CloakSmokeParticle : public Particle
{
public:
	CloakSmokeParticle() {}

	Vector			m_vOrigin;
	Vector			m_vFinalPos;
	Vector			m_vVelocity;
	float			m_flDieTime;
	float			m_flLifetime;
	float			m_flAlpha;
	float			m_flSize;
	unsigned char	m_uchColor[3];
};

class CCloakSmokeCloud : public CParticleEffect
{
public:
	DECLARE_CLASS( CCloakSmokeCloud, CParticleEffect );

	static CSmartPtr<CCloakSmokeCloud> Create( const char *pDebugName );

	virtual void SimulateParticles	( CParticleSimulateIterator *pIterator );
	virtual void RenderParticles	( CParticleRenderIterator *pIterator );

	void AddAttractor(Vector *F, Vector apos, Vector ppos, float scale);
	void ApplyDrag(Vector *F, Vector vel, float scale, float targetvel);

	virtual void	Update( float flTimeDelta );

	CloakSmokeParticle*	AddCloakSmokeParticle( const Vector &vOrigin);

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
	CCloakSmokeCloud( const char *pDebugName );
	virtual			~CCloakSmokeCloud();

private:
	CCloakSmokeCloud( const CCloakSmokeCloud & );

	float m_flNearClipMin;
	float m_flNearClipMax;

	Vector	m_vecOrigin;
	Vector	m_vecVelocity;

	float	m_flDieTime;
	float	m_flNextParticle;

	bool	m_bInitiated;

	static PMaterialHandle m_hMaterial;
};

#endif//FF_FX_CLOAKSMOKE_EMITTER_H