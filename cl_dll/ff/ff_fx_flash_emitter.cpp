/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_fx_flash_emitter.cpp
/// @author Patrick O'Leary (Mulchman)
/// @date 02/26/2006
/// @brief Flash effect emitter
///
/// Declaration of the flash effect emitter particle system
/// 
/// Revisions
/// ---------
///	02/26/2006	

#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "particles_simple.h"
#include "ff_fx_flash_emitter.h"
#include "materialsystem/imaterialvar.h"
#include "c_te_effect_dispatch.h"

#define FLASH_EFFECT_MATERIAL "effects/yellowflare"
#define RING_EFFECT_MATERIAL "sprites/lgtning.vmt"

ConVar flash_on				( "ffdev_flash_on", "1", FCVAR_CHEAT, "Turn the flash effect on or off - 1 or 0." );
ConVar flash_scale			( "ffdev_flash_scale", "256.0", FCVAR_CHEAT, "How big the flash effect gets.");
ConVar flash_speed			( "ffdev_flash_speed", "0.3", FCVAR_CHEAT, "Duration of the flash effect.");
ConVar flash_ripples		( "ffdev_flash_ripples", "1", FCVAR_CHEAT, "How many ripples the flash effect has.");
ConVar flash_ripple_period	( "ffdev_flash_ripple_period", "0.5", FCVAR_CHEAT, "Time between ripples.");

//========================================================================
// Client effect precache table
//========================================================================
CLIENTEFFECT_REGISTER_BEGIN( PrecacheFlashEmitter )
	CLIENTEFFECT_MATERIAL( FLASH_EFFECT_MATERIAL )
	CLIENTEFFECT_MATERIAL( RING_EFFECT_MATERIAL )
CLIENTEFFECT_REGISTER_END()

//========================================================================
// Static material handles
//========================================================================
PMaterialHandle CFlashEmitter::m_hMaterial = INVALID_MATERIAL_HANDLE;

//========================================================================
// CFlashEmitter constructor
//========================================================================
CFlashEmitter::CFlashEmitter( const char *pDebugName ) : CSimpleEmitter( pDebugName )
{
	m_pDebugName = pDebugName;
}

//========================================================================
// CFlashEmitter destructor
//========================================================================
CFlashEmitter::~CFlashEmitter()
{
}

//========================================================================
// CFlashEmitter::Create
// ----------------------
// Purpose: Creates a new instance of a CFlashEmitter object
//========================================================================
CSmartPtr< CFlashEmitter > CFlashEmitter::Create( const char *pDebugName )
{
	CFlashEmitter *pRet = new CFlashEmitter( pDebugName );

	pRet->SetDynamicallyAllocated();

	if( m_hMaterial == INVALID_MATERIAL_HANDLE )
		m_hMaterial = pRet->GetPMaterial( FLASH_EFFECT_MATERIAL );

	return pRet;
}

//========================================================================
// CFlashEmitter::RenderParticles
// ----------
// Purpose: Renders all the particles in the system
//========================================================================
void CFlashEmitter::RenderParticles( CParticleRenderIterator *pIterator )
{
	if( flash_on.GetInt() == 0 )
		return;

	const FlashParticle *pParticle = ( const FlashParticle * )pIterator->GetFirst();

	float flLife, flDeath;

	while( pParticle )
	{
		Vector	tPos;

		TransformParticle( ParticleMgr()->GetModelView(), pParticle->m_Pos, tPos );
		float sortKey = ( int )tPos.z;

		flLife = pParticle->m_flLifetime - pParticle->m_flOffset;
		flDeath = pParticle->m_flDieTime - pParticle->m_flOffset;

		if( flLife > 0.0f )
		{
			float flColor = 1.0f;
			Vector vColor = Vector( flColor, flColor, flColor );

			// Render it
			RenderParticle_ColorSizeAngle(
				pIterator->GetParticleDraw(), 
				tPos, 
				vColor, 
				0.5f, 																		// Alpha
				SimpleSplineRemapVal( flLife, 0.0f, flDeath, 0, flash_scale.GetFloat() ), 		// Size
				pParticle->m_flRoll
			);
		}

		pParticle = ( const FlashParticle * )pIterator->GetNext( sortKey );
	}

}

//========================================================================
// CFlashEmitter::AddFlashParticle
// ----------
// Purpose: Add a new particle to the system
//========================================================================
FlashParticle *CFlashEmitter::AddFlashParticle( void )
{
	FlashParticle *pRet = ( FlashParticle * )AddParticle( sizeof( FlashParticle ), m_hMaterial, GetSortOrigin() );

	if( pRet )
	{
		pRet->m_Pos = GetSortOrigin();
		pRet->m_vecVelocity.Init();
		pRet->m_flRoll = 0;
		pRet->m_flRollDelta = 0;
		pRet->m_flLifetime = 0;
		pRet->m_flDieTime = 0;
		pRet->m_uchColor[ 0 ] = pRet->m_uchColor[ 1 ] = pRet->m_uchColor[ 2 ] = 0;
		pRet->m_uchStartAlpha = pRet->m_uchEndAlpha = 255;
		pRet->m_uchStartSize = 0;
		pRet->m_iFlags = 0;
		pRet->m_flOffset = 0;
	}

	return pRet;
}

void FF_FX_FlashEffect_Callback( const CEffectData &data )
{
	CSmartPtr< CFlashEmitter > flashEffect = CFlashEmitter::Create( "FlashEffect" );

	float offset = 0.0f;

	for( int i = 0; i < flash_ripples.GetInt(); i++ )
	{
		FlashParticle *c = flashEffect->AddFlashParticle();

		c->m_flDieTime = flash_speed.GetFloat();
		c->m_Pos = data.m_vOrigin;
		c->m_flOffset = offset;

		offset += flash_ripple_period.GetFloat();
	}
	
	// shock wave
	CBroadcastRecipientFilter filter;
	filter.AddAllPlayers();
	te->BeamRingPoint( 
		filter, 0.0f, data.m_vOrigin,	//origin
		1.0f,							//start radius
		flash_scale.GetFloat(),		//end radius
		modelinfo->GetModelIndex( RING_EFFECT_MATERIAL ),	//texture
		0,								//halo index
		0,								//start frame
		0,								//framerate
		flash_speed.GetFloat(),//life
		16,								//width
		0,								//spread
		0,								//amplitude
		255,							//r
		255,							//g
		255,							//b
		255,							//a
		0,								//speed
		0x00000008
		);
}

DECLARE_CLIENT_EFFECT( "FF_FlashEffect", FF_FX_FlashEffect_Callback );
