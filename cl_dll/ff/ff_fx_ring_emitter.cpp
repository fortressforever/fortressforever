/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_fx_ring_emitter.cpp
/// @author Patrick O'Leary (Mulchman)
/// @date 02/26/2006
/// @brief Ring effect emitter
///
/// Declaration of the ring effect emitter particle system
/// 
/// Revisions
/// ---------
///	02/26/2006	Initial version

#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "particles_simple.h"
#include "ff_fx_ring_emitter.h"
#include "materialsystem/imaterialvar.h"
#include "c_te_effect_dispatch.h"
#include "mathlib.h"

#define RING_EFFECT_MATERIAL "effects/yellowflare"

ConVar ring_on			( "ffdev_ring_on", "1", 0, "Turn the ring effect on or off - 1 or 0." );
ConVar ring_scale		( "ffdev_ring_scale", "20.0", 0, "How big the particles in the rings are." );
ConVar ring_speed		( "ffdev_ring_speed", "0.2", 0, "Duration of the rings effect." );
ConVar ring_magnitude	( "ffdev_ring_magnitude", "1000.0", 0, "Speed the rings expand." );
//ConVar ring_ripples			( "ffdev_ring_ripples", "1", 0, "How many ripples the ring effect has.");
//ConVar ring_ripple_period	( "ffdev_ring_ripple_period", "0.05", 0, "Time between ripples.");

//========================================================================
// Client effect precache table
//========================================================================
CLIENTEFFECT_REGISTER_BEGIN( PrecacheRingEmitter )
	CLIENTEFFECT_MATERIAL( RING_EFFECT_MATERIAL )
CLIENTEFFECT_REGISTER_END()

//========================================================================
// Static material handles
//========================================================================
PMaterialHandle CRingEmitter::m_hMaterial = INVALID_MATERIAL_HANDLE;

//========================================================================
// CRingEmitter constructor
//========================================================================
CRingEmitter::CRingEmitter( const char *pDebugName ) : CSimpleEmitter( pDebugName )
{
	m_pDebugName = pDebugName;
}

//========================================================================
// CRingEmitter destructor
//========================================================================
CRingEmitter::~CRingEmitter()
{
}

//========================================================================
// CRingEmitter::Create
// ----------------------
// Purpose: Creates a new instance of a CRingEmitter object
//========================================================================
CSmartPtr< CRingEmitter > CRingEmitter::Create( const char *pDebugName )
{
	CRingEmitter *pRet = new CRingEmitter( pDebugName );

	pRet->SetDynamicallyAllocated();

	if( m_hMaterial == INVALID_MATERIAL_HANDLE )
		m_hMaterial = pRet->GetPMaterial( RING_EFFECT_MATERIAL );

	return pRet;
}

//========================================================================
// CRingEmitter::RenderParticles
// ----------
// Purpose: Renders all the particles in the system
//========================================================================
void CRingEmitter::RenderParticles( CParticleRenderIterator *pIterator )
{
	if( ring_on.GetInt() == 0 )
		return;

	const RingParticle *pParticle = ( const RingParticle * )pIterator->GetFirst();

	float flLife, flDeath;

	while( pParticle )
	{
		Vector	tPos;

		TransformParticle( g_ParticleMgr.GetModelView(), pParticle->m_Pos, tPos );
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
				flColor, 																		// Alpha
				/*SimpleSplineRemapVal( flLife, 0.0f, flDeath, 0, ring_scale.GetFloat() ), 		// Size*/
				ring_scale.GetFloat(),
				pParticle->m_flRoll
			);
		}

		pParticle = ( const RingParticle * )pIterator->GetNext( sortKey );
	}

}

//========================================================================
// CRingEmitter::AddRingParticle
// ----------
// Purpose: Add a new particle to the system
//========================================================================
RingParticle *CRingEmitter::AddRingParticle( void )
{
	RingParticle *pRet = ( RingParticle * )AddParticle( sizeof( RingParticle ), m_hMaterial, GetSortOrigin() );

	if( pRet )
	{
		pRet->m_Pos = GetSortOrigin();
		pRet->m_vecVelocity.Init();
		pRet->m_flRoll = 0;
		pRet->m_flRollDelta = 0;
		pRet->m_flLifetime = 0;
		pRet->m_flDieTime = ring_speed.GetFloat();
		pRet->m_uchColor[ 0 ] = pRet->m_uchColor[ 1 ] = pRet->m_uchColor[ 2 ] = 0;
		pRet->m_uchStartAlpha = pRet->m_uchEndAlpha = 255;
		pRet->m_uchStartSize = ring_scale.GetFloat();
		pRet->m_uchEndSize = ring_scale.GetFloat();
		pRet->m_iFlags = 0;
		pRet->m_flOffset = 0;
	}

	return pRet;
}

void FF_FX_RingEffect_Callback( const CEffectData &data )
{
	CSmartPtr< CRingEmitter > ringEffect = CRingEmitter::Create( "RingEffect" );

	int iPoints = 20;

	for( int i = 0; i < iPoints; i++ )
	{
		RingParticle *pParticle1 = ringEffect->AddRingParticle();
		RingParticle *pParticle2 = ringEffect->AddRingParticle();
		RingParticle *pParticle3 = ringEffect->AddRingParticle();

		float flX = ( 1 * cos( i * M_PI * 2.0f / iPoints ) );
		float flY = ( 1 * sin( i * M_PI * 2.0f / iPoints ) );

		pParticle1->m_Pos = data.m_vOrigin + Vector( flX, 0, flY );
		pParticle2->m_Pos = data.m_vOrigin + Vector( 0, flX, flY );
		pParticle3->m_Pos = data.m_vOrigin + Vector( flX, flY, 0 );

		// Set velocity to point from the origin outwards
		pParticle1->m_vecVelocity = Vector( flX * ring_magnitude.GetFloat(), 0, flY * ring_magnitude.GetFloat() );
		pParticle2->m_vecVelocity = Vector( 0, flX * ring_magnitude.GetFloat(), flY * ring_magnitude.GetFloat() );
		pParticle3->m_vecVelocity = Vector( flX * ring_magnitude.GetFloat(), flY * ring_magnitude.GetFloat(), 0 );
	}
}

DECLARE_CLIENT_EFFECT( "FF_RingEffect", FF_FX_RingEffect_Callback );
