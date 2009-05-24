/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_fx_ring_emitter.h
/// @author Patrick O'Leary (Mulchman)
/// @date 02/26/2006
/// @brief Ring effect emitter
///
/// Declaration of the ring effect emitter particle system
/// 
/// Revisions
/// ---------
///	02/26/2006	Initial version

#ifndef FF_FX_RING_EMITTER_H
#define FF_FX_RING_EMITTER_H

class RingParticle : public SimpleParticle
{
public:
	RingParticle( void ) {}
	
	float	m_flOffset;

};

class CRingEmitter : public CSimpleEmitter
{
public:
	DECLARE_CLASS( CRingEmitter, CSimpleEmitter );

	static CSmartPtr< CRingEmitter > Create( const char *pDebugName );

	virtual void RenderParticles( CParticleRenderIterator *pIterator );
	virtual void SimulateParticles( CParticleSimulateIterator *pIterator ) { CSimpleEmitter::SimulateParticles( pIterator ); }

	RingParticle *AddRingParticle( void );

	static PMaterialHandle GetMaterial( void ) { return m_hMaterial; }

protected:
	CRingEmitter( const char *pDebugName );
	virtual ~CRingEmitter( void );

private:
	CRingEmitter( const CRingEmitter & );

	static PMaterialHandle m_hMaterial;

};

#endif //FF_FX_RING_EMITTER_H
