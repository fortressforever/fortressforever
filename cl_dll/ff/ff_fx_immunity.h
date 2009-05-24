// =============== Fortress Forever ===============
// ======== A modification for Half-Life 2 ========
//
// @file cl_dll\ff\ff_fx_immunity.hp
// @author Patrick O'Leary (Mulchman)
// @date 10/6/2006
// @brief Immunity particle effects
//
// particle manager for the immunity particles
//
// Revisions
// ---------
// 10/6/2006, Mulchman
//		Initial version

#ifndef FF_FX_IMMUNITY_H
#define FF_FX_IMMUNITY_H

class ImmunityParticle : public Particle
{
public:
	ImmunityParticle( void ) { }

	Vector			m_vOrigin;
	Vector			m_vFinalPos;
	Vector			m_vVelocity;
	float			m_flEndPosTime;
	float			m_flDieTime;
	float			m_flLifetime;
	float			m_flAlpha;
	float			m_flSize;
	float			m_flRoll;
	float			m_flRollDelta;
	unsigned char	m_uchColor[3];
};

class CImmunityEmitter : public CSimpleEmitter
{
public:
	DECLARE_CLASS( CImmunityEmitter, CSimpleEmitter );

	static CSmartPtr< CImmunityEmitter > Create( const char *pDebugName );

	virtual void RenderParticles( CParticleRenderIterator *pIterator );
	virtual void SimulateParticles( CParticleSimulateIterator *pIterator );
	virtual void Update( float flTimeDelta );

	ImmunityParticle *AddImmunityParticle( const Vector& vecOrigin );

	static PMaterialHandle GetMaterial( void ) { return m_hMaterial; }

	void ApplyDrag( Vector *F, Vector vecVelocity, float flScale, float flTargetVel );
	void SetDieTime( float flDieTime ) { m_flDieTime = flDieTime; }
	void UpdateEmitter( const Vector& vecOrigin, const Vector& vecVelocity )
	{
		m_vecOrigin = vecOrigin;
		m_vecVelocity = vecVelocity;
	}

protected:
	CImmunityEmitter( const char *pDebugName );
	virtual ~CImmunityEmitter( void );

private:
	CImmunityEmitter( const CImmunityEmitter & );

	static PMaterialHandle m_hMaterial;

	Vector	m_vecOrigin;
	Vector	m_vecVelocity;

	float	m_flDieTime;
	float	m_flNextParticle;

};

#endif // FF_FX_IMMUNITY_H
