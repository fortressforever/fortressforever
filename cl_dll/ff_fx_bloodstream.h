/********************************************************************
	created:	2006/08/24
	created:	24:8:2006   14:37
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff_fx_bloodstream.h
	file path:	f:\ff-svn\code\trunk\cl_dll
	file base:	ff_fx_bloodstream
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#ifndef FF_FX_BLOODSTREAM_H
#define FF_FX_BLOODSTREAM_H

class BloodStreamParticle : public Particle
{
public:
	BloodStreamParticle() {}

	Vector			m_vOrigin;
	Vector			m_vFinalPos;
	Vector			m_vVelocity;
	float			m_flDieTime;
	float			m_flLifetime;
	float			m_flAlpha;
	float			m_flSize;
	unsigned char	m_uchColor[3];
};

class CBloodStream : public CParticleEffect
{
public:
	DECLARE_CLASS( CBloodStream, CParticleEffect );

	static CSmartPtr<CBloodStream> Create(C_BaseAnimatingOverlay *pRagdoll, const char *pDebugName);

	virtual void SimulateParticles	( CParticleSimulateIterator *pIterator );
	virtual void RenderParticles	( CParticleRenderIterator *pIterator );

	void AddAttractor(Vector *F, Vector apos, Vector ppos, float scale);
	void ApplyDrag(Vector *F, Vector vel, float scale, float targetvel);

	virtual void	Update( float flTimeDelta );

	BloodStreamParticle*	AddBloodStreamParticle( const Vector &vOrigin);

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
	CBloodStream( const char *pDebugName );
	virtual			~CBloodStream();

private:
	CBloodStream( const CBloodStream & );

	float m_flNearClipMin;
	float m_flNearClipMax;

	Vector	m_vecOrigin;
	Vector	m_vecVelocity;

	float	m_flDieTime;
	float	m_flNextParticle;

	static PMaterialHandle m_hMaterial;

	EHANDLE			m_hEntAttached;
};

#endif