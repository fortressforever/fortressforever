// =============== Fortress Forever ===============
// ======== A modification for Half-Life 2 ========
//
// @file cl_dll\ff\ff_fx_overpressure.hp
// @author Patrick O'Leary (Mulchman)
// @date 10/6/2006
// @brief Overpressure particle effects
//
// particle manager for the overpressure particles
//
// Revisions
// ---------
// 10/6/2006, Mulchman
//		Initial version

#ifndef FF_FX_OVERPRESSURE_H
#define FF_FX_OVERPRESSURE_H

class OverpressureParticle : public Particle
{
public:
	OverpressureParticle( void ) { }

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

class COverpressureEmitter : public CSimpleEmitter
{
public:
	DECLARE_CLASS( COverpressureEmitter, CSimpleEmitter );

	static CSmartPtr< COverpressureEmitter > Create( const char *pDebugName );

	virtual void RenderParticles( CParticleRenderIterator *pIterator );
	virtual void SimulateParticles( CParticleSimulateIterator *pIterator );

	OverpressureParticle *AddOverpressureParticle( const Vector& vecOrigin );

	static PMaterialHandle GetMaterial( void ) { return m_hMaterial; }

	void ApplyDrag( Vector *F, Vector vecVelocity, float flScale, float flTargetVel );
	void SetDieTime( float flDieTime ) { m_flDieTime = flDieTime; }
	void UpdateEmitter( const Vector& vecOrigin, const Vector& vecVelocity )
	{
		m_vecOrigin = vecOrigin;
		m_vecVelocity = vecVelocity;
	}

protected:
	COverpressureEmitter( const char *pDebugName );
	virtual ~COverpressureEmitter( void );

private:
	COverpressureEmitter( const COverpressureEmitter & );

	static PMaterialHandle m_hMaterial;

	Vector	m_vecOrigin;
	Vector	m_vecVelocity;

	float	m_flDieTime;
	float	m_flNextParticle;

};

#endif // FF_FX_OVERPRESSURE_H
