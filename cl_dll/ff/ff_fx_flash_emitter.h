/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_fx_flash_emitter.h
/// @author Patrick O'Leary (Mulchman)
/// @date 02/26/2006
/// @brief Flash effect emitter
///
/// Declaration of the flash effect emitter particle system
/// 
/// Revisions
/// ---------
///	02/26/2006	Initial version

#ifndef FF_FX_FLASH_EMITTER_H
#define FF_FX_FLASH_EMITTER_H


class FlashParticle : public SimpleParticle
{
public:
	FlashParticle( void ) {}
	
	float	m_flOffset;

};

class CFlashEmitter : public CSimpleEmitter
{
public:
	DECLARE_CLASS( CFlashEmitter, CSimpleEmitter );

	static CSmartPtr< CFlashEmitter > Create( const char *pDebugName );

	virtual void RenderParticles( CParticleRenderIterator *pIterator );
	virtual void SimulateParticles( CParticleSimulateIterator *pIterator ) { CSimpleEmitter::SimulateParticles( pIterator ); }

	FlashParticle *AddFlashParticle( void );

	static PMaterialHandle GetMaterial( void ) { return m_hMaterial; }

protected:
	CFlashEmitter( const char *pDebugName );
	virtual ~CFlashEmitter( void );

private:
	CFlashEmitter( const CFlashEmitter & );

	static PMaterialHandle m_hMaterial;

};

#endif //FF_FX_FLASH_EMITTER_H
