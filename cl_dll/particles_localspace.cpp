//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Particles which are simulated locally to some space (attachment, bone, etc)
//
//=============================================================================//

#include "cbase.h"
#include "particles_simple.h"
#include "particles_localspace.h"

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CLocalSpaceEmitter::CLocalSpaceEmitter( const char *pDebugName ) :
	CSimpleEmitter( pDebugName )
{
}


inline const matrix3x4_t& CLocalSpaceEmitter::GetTransformMatrix() const
{
	return m_ParticleEffect.GetLocalSpaceTransform();
}


//-----------------------------------------------------------------------------
// Purpose: Creates a local space emitter
//-----------------------------------------------------------------------------
CSmartPtr<CLocalSpaceEmitter> CLocalSpaceEmitter::Create( const char *pDebugName, int entIndex, int nAttachment, int fFlags )
{
	CLocalSpaceEmitter *pRet = new CLocalSpaceEmitter( pDebugName );
	pRet->SetDynamicallyAllocated( true );
	pRet->m_nEntIndex = entIndex;
	pRet->m_nAttachment = nAttachment;
	pRet->m_fFlags = fFlags;
	
	pRet->SetupTransformMatrix();

	return pRet;
}

//-----------------------------------------------------------------------------
// Purpose: Used to build the transformation matrix for this frame
//-----------------------------------------------------------------------------
void CLocalSpaceEmitter::Update( float flTimeDelta )
{
	SetupTransformMatrix();
}

extern void FormatViewModelAttachment( Vector &vOrigin, bool bInverse );


void CLocalSpaceEmitter::SimulateParticles( CParticleSimulateIterator *pIterator )
{
	float timeDelta = pIterator->GetTimeDelta();

	SimpleParticle *pParticle = (SimpleParticle*)pIterator->GetFirst();
	while ( pParticle )
	{
		// Update velocity
		UpdateVelocity( pParticle, timeDelta );
		pParticle->m_Pos += pParticle->m_vecVelocity * timeDelta;

		// Should this particle die?
		pParticle->m_flLifetime += timeDelta;
		UpdateRoll( pParticle, timeDelta );

		// If we're dead, we're done
		if ( pParticle->m_flLifetime >= pParticle->m_flDieTime )
			pIterator->RemoveParticle( pParticle );
	
		pParticle = (SimpleParticle*)pIterator->GetNext();
	}
}


void CLocalSpaceEmitter::RenderParticles( CParticleRenderIterator *pIterator )
{
	const matrix3x4_t &mLocalToWorld = GetTransformMatrix();
	const VMatrix &mModelView = g_ParticleMgr.GetModelView();

	const SimpleParticle *pParticle = (const SimpleParticle *)pIterator->GetFirst();
	while ( pParticle )
	{
		// Transform it
		Vector screenPos, worldPos;
		VectorTransform( pParticle->m_Pos, mLocalToWorld, worldPos );
		
		// Correct viewmodel squashing
		if ( m_fFlags & FLE_VIEWMODEL )
		{
			FormatViewModelAttachment( worldPos, false );
		}

		TransformParticle( mModelView, worldPos, screenPos );
		
		float sortKey = (int) screenPos.z;

		// Render it
		RenderParticle_ColorSizeAngle(
			pIterator->GetParticleDraw(),
			screenPos,
			UpdateColor( pParticle ),
			UpdateAlpha( pParticle ) * GetAlphaDistanceFade( screenPos, m_flNearClipMin, m_flNearClipMax ),
			UpdateScale( pParticle ),
			pParticle->m_flRoll 
			);

		pParticle = (const SimpleParticle *)pIterator->GetNext( sortKey );
	}
}



//-----------------------------------------------------------------------------
// Purpose: Create the matrix by which we'll transform the particle's local 
//			space into world space, via the attachment's transform
//-----------------------------------------------------------------------------
void CLocalSpaceEmitter::SetupTransformMatrix( void )
{
	C_BaseEntity *pEnt = ClientEntityList().GetEnt( m_nEntIndex );
	
	if ( pEnt )
	{
		Vector origin;
		QAngle angles;

		if ( pEnt->GetAttachment( m_nAttachment, origin, angles ) == false )
		{
			// This attachment is bogus!
			Assert(0);
		}
	
		// Tell the particle effect so it knows
		matrix3x4_t mat;
		AngleMatrix( angles, origin, mat );
		m_ParticleEffect.SetLocalSpaceTransform( mat );
		SetSortOrigin( origin );
	}

	// We preapply the local transform because we need to squash it for viewmodel FOV.
	m_ParticleEffect.SetAutoApplyLocalTransform( false );
}

