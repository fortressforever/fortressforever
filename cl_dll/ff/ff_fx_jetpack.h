#ifndef FF_FX_JETPACK_H
#define FF_FX_JETPACK_H

#include "dlight.h"

class JetpackParticle : public Particle
{
public:
	JetpackParticle( void ) { }

	int				m_Type, m_Appearance;		// Type of particle
	Vector			m_Origin, m_Velocity;					
	float			m_flRoll, m_flRollDelta;	// Rotation
	float			m_Lifetime, m_Dietime;		// To keep track
	float			m_Collisiontime;			// When particle will next hit
	unsigned char	m_uchStartSize, m_uchEndSize;
	Vector			m_HitSurfaceNormal;			// Normal of collision surface

	float			m_flRedness;

	dlight_t		*m_pDLight;					// dynamic light attached to particle
	float			m_fDLightDieTime;			// when the dynamic light dies, so you can avoid trying to access it
	float			m_fDLightStartRadius;		// size of the dynamic light at the beginning
	float			m_fDLightEndRadius;			// size of the dynamic light at the end
};

class CJetpackEmitter : public CSimpleEmitter
{
public:
	DECLARE_CLASS( CJetpackEmitter, CSimpleEmitter );

	static CSmartPtr< CJetpackEmitter > Create( const char *pDebugName, CBaseEntity *pOwner );

	virtual void RenderParticles( CParticleRenderIterator *pIterator );
	virtual void SimulateParticles( CParticleSimulateIterator *pIterator );
	virtual void Update( float flTimeDelta );

	JetpackParticle *AddJetpackParticle( const Vector& vecStart, const Vector& vecForward );

	void ApplyDrag( Vector *F, Vector vecVelocity, float flScale, float flTargetVel );
	void SetDieTime( float flDieTime ) { m_flDieTime = flDieTime; }
	CBaseEntity* GetOwnerEntity() { return m_pOwner; }
	void SetOwnerEntity(CBaseEntity *pOwner) { m_pOwner = pOwner; }

protected:
	CJetpackEmitter( const char *pDebugName );
	virtual ~CJetpackEmitter( void );

private:
	CJetpackEmitter( const CJetpackEmitter & );

	static PMaterialHandle m_hFlameMaterial;

	CBaseEntity *m_pOwner;

	float			m_SpreadSpeed;
	float			m_Speed;
	float			m_StartSize;
	float			m_EndSize;
	float			m_Rate;
	float			m_Lifetime;

	float	m_flDieTime;
	float	m_flNextParticle;
	float	m_flLastParticleDLightTime;
};

#endif // FF_FX_JETPACK_H
