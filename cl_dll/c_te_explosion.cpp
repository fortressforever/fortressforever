//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client explosions
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "tempentity.h"  // FLAGS
#include "c_te_particlesystem.h"
#include "RagdollExplosionEnumerator.h"
#include "glow_overlay.h"
#include "fx_explosion.h"
#include "ClientEffectPrecacheSystem.h"
#include "engine/ivdebugoverlay.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	OLD_EXPLOSION	0


// Enumator class for ragdolls being affected by explosive forces
CRagdollExplosionEnumerator::CRagdollExplosionEnumerator( Vector origin, float radius, float magnitude )
{
	m_vecOrigin		= origin;
	m_flMagnitude	= magnitude;
	m_flRadius		= radius;
}

// Actual work code
IterationRetval_t CRagdollExplosionEnumerator::EnumElement( IHandleEntity *pHandleEntity )
{
	C_BaseEntity *pEnt = ClientEntityList().GetBaseEntityFromHandle( pHandleEntity->GetRefEHandle() );
	
	if ( pEnt == NULL )
		return ITERATION_CONTINUE;

	C_BaseAnimating *pModel = static_cast< C_BaseAnimating * >( pEnt );

	// If the ragdoll was created on this tick, then the forces were already applied on the server
	if ( pModel == NULL || WasRagdollCreatedOnCurrentTick( pEnt ) )
		return ITERATION_CONTINUE;
	
	m_Entities.AddToTail( pEnt );

	return ITERATION_CONTINUE;
}

CRagdollExplosionEnumerator::~CRagdollExplosionEnumerator()
{
	for (int i = 0; i < m_Entities.Count(); i++ )
	{
		C_BaseEntity *pEnt = m_Entities[i];
		C_BaseAnimating *pModel = static_cast< C_BaseAnimating * >( pEnt );

		Vector	position = pEnt->CollisionProp()->GetCollisionOrigin();

		trace_t	tr;
		UTIL_TraceLine( m_vecOrigin, position, MASK_SHOT, NULL, COLLISION_GROUP_NONE, &tr );

		// debugoverlay->AddLineOverlay( m_vecOrigin, position, 0,255,0, true, 18.0 );

		if ( tr.fraction < 1.0f && tr.m_pEnt != pModel )
			continue;	

		Vector	dir		= position - m_vecOrigin;
		float	dist	= VectorNormalize( dir );
		float	force	= m_flMagnitude - ( ( m_flMagnitude / m_flRadius ) * dist );
		
		if ( force <= 1.0f )
			continue;

		dir *= force; // scale force

		// tricky, adjust tr.start so end-start->= force
		tr.startpos = tr.endpos - dir;
		// move expolsion center a bit down, so things fly higher 
		tr.startpos.z -= 32.0f;

		pModel->ImpactTrace( &tr, DMG_BLAST, NULL );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Explosion TE
//-----------------------------------------------------------------------------
class C_TEExplosion : public C_TEParticleSystem
{
public:
	DECLARE_CLASS( C_TEExplosion, C_TEParticleSystem );
	DECLARE_CLIENTCLASS();

					C_TEExplosion( void );
	virtual			~C_TEExplosion( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );
	virtual void RenderParticles( CParticleRenderIterator *pIterator );
	virtual void SimulateParticles( CParticleSimulateIterator *pIterator );

public:
	void			AffectRagdolls( void );

	int				m_nModelIndex;
	float			m_fScale;
	int				m_nFrameRate;
	int				m_nFlags;
	Vector			m_vecNormal;
	char			m_chMaterialType;
	int				m_nRadius;
	int				m_nMagnitude;

	//CParticleCollision	m_ParticleCollision;
	CParticleMgr		*m_pParticleMgr;
	PMaterialHandle		m_MaterialHandle;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEExplosion::C_TEExplosion( void )
{
	m_nModelIndex = 0;
	m_fScale = 0;
	m_nFrameRate = 0;
	m_nFlags = 0;
	m_vecNormal.Init();
	m_chMaterialType = 'C';
	m_nRadius = 0;
	m_nMagnitude = 0;

	m_pParticleMgr		= NULL;
	m_MaterialHandle	= INVALID_MATERIAL_HANDLE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEExplosion::~C_TEExplosion( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TEExplosion::AffectRagdolls( void )
{
	if ( ( m_nRadius == 0 ) || ( m_nMagnitude == 0 ) )
		return;

	CRagdollExplosionEnumerator	ragdollEnum( m_vecOrigin, m_nRadius, m_nMagnitude );
	partition->EnumerateElementsInSphere( PARTITION_CLIENT_RESPONSIVE_EDICTS, m_vecOrigin, m_nRadius, false, &ragdollEnum );
}

//
// CExplosionOverlay
//

bool CExplosionOverlay::Update( void )
{
	m_flLifetime += gpGlobals->frametime;
	
	const float flTotalLifetime = 0.1f;

	if ( m_flLifetime < flTotalLifetime )
	{
		float flColorScale = 1.0f - ( m_flLifetime / flTotalLifetime );

		for( int i=0; i < m_nSprites; i++ )
		{
			m_Sprites[i].m_vColor = m_vBaseColors[i] * flColorScale;
			
			m_Sprites[i].m_flHorzSize += 16.0f * gpGlobals->frametime;
			m_Sprites[i].m_flVertSize += 16.0f * gpGlobals->frametime;
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TEExplosion::PostDataUpdate( DataUpdateType_t updateType )
{
	AffectRagdolls();

	// Filter out a water explosion
	if ( UTIL_PointContents( m_vecOrigin ) & CONTENTS_WATER )
	{
		WaterExplosionEffect().Create( m_vecOrigin, m_nMagnitude, m_fScale, m_nFlags );
		return;
	}

	if ( !( m_nFlags & TE_EXPLFLAG_NOFIREBALL ) )
	{
		if ( CExplosionOverlay *pOverlay = new CExplosionOverlay )
		{
			pOverlay->m_flLifetime	= 0;
			pOverlay->m_vPos		= m_vecOrigin;
			pOverlay->m_nSprites	= 1;
			
			pOverlay->m_vBaseColors[0].Init( 1.0f, 0.9f, 0.7f );

			pOverlay->m_Sprites[0].m_flHorzSize = 0.05f;
			pOverlay->m_Sprites[0].m_flVertSize = pOverlay->m_Sprites[0].m_flHorzSize*0.5f;

			pOverlay->Activate();
		}
	}

	BaseExplosionEffect().Create( m_vecOrigin, m_nMagnitude, m_fScale, m_nFlags );
}

void C_TEExplosion::RenderParticles( CParticleRenderIterator *pIterator )
{
}


void C_TEExplosion::SimulateParticles( CParticleSimulateIterator *pIterator )
{
	pIterator->RemoveAllParticles();
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TEExplosion, DT_TEExplosion, CTEExplosion)
	RecvPropInt( RECVINFO(m_nModelIndex)),
	RecvPropFloat( RECVINFO(m_fScale )),
	RecvPropInt( RECVINFO(m_nFrameRate)),
	RecvPropInt( RECVINFO(m_nFlags)),
	RecvPropVector( RECVINFO(m_vecNormal)),
	RecvPropInt( RECVINFO(m_chMaterialType)),
	RecvPropInt( RECVINFO(m_nRadius)),
	RecvPropInt( RECVINFO(m_nMagnitude)),
END_RECV_TABLE()


void TE_Explosion( IRecipientFilter& filter, float delay,
	const Vector* pos, int modelindex, float scale, int framerate, int flags, int radius, int magnitude, 
	const Vector* normal = NULL, unsigned char materialType = 'C' )
{
	// Major hack to access singleton object for doing this event (simulate receiving network message)
	__g_C_TEExplosion.m_nModelIndex = modelindex;
	__g_C_TEExplosion.m_fScale = scale;
	__g_C_TEExplosion.m_nFrameRate = framerate;
	__g_C_TEExplosion.m_nFlags = flags;
	__g_C_TEExplosion.m_vecNormal = *normal;
	__g_C_TEExplosion.m_chMaterialType = materialType;
	__g_C_TEExplosion.m_nRadius = radius;
	__g_C_TEExplosion.m_nMagnitude = magnitude;

	__g_C_TEExplosion.PostDataUpdate( DATA_UPDATE_CREATED );

}