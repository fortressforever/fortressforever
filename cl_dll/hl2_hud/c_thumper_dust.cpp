//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "particlemgr.h"
#include "particle_prototype.h"
#include "particle_util.h"
#include "c_te_particlesystem.h"
#include "fx.h"
#include "fx_quad.h"
#include "c_te_effect_dispatch.h"
#include "view.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	THUMPER_DUST_LIFETIME		2.0f
#define THUMPER_MAX_PARTICLES		32


extern IPhysicsSurfaceProps *physprops;


class ThumperDustEmitter : public CSimpleEmitter
{
public:
	
	ThumperDustEmitter( const char *pDebugName ) : CSimpleEmitter( pDebugName ) {}

	static ThumperDustEmitter *Create( const char *pDebugName )
	{
		return new ThumperDustEmitter( pDebugName );
	}

	void UpdateVelocity( SimpleParticle *pParticle, float timeDelta )
	{
		// Float up when lifetime is half gone.
		pParticle->m_vecVelocity[2] -= ( 8.0f * timeDelta );


		// FIXME: optimize this....
		pParticle->m_vecVelocity *= ExponentialDecay( 0.9, 0.03, timeDelta );
	}

	virtual	float UpdateRoll( SimpleParticle *pParticle, float timeDelta )
	{
		pParticle->m_flRoll += pParticle->m_flRollDelta * timeDelta;
		
		pParticle->m_flRollDelta += pParticle->m_flRollDelta * ( timeDelta * -2.0f );

		//Cap the minimum roll
		if ( fabs( pParticle->m_flRollDelta ) < 0.5f )
		{
			pParticle->m_flRollDelta = ( pParticle->m_flRollDelta > 0.0f ) ? 0.5f : -0.5f;
		}

		return pParticle->m_flRoll;
	}

	virtual float UpdateAlpha( const SimpleParticle *pParticle )
	{
		return (pParticle->m_uchStartAlpha/255.0f) + ( (float)(pParticle->m_uchEndAlpha/255.0f) - (float)(pParticle->m_uchStartAlpha/255.0f) ) * (pParticle->m_flLifetime / pParticle->m_flDieTime);
	}

private:
	ThumperDustEmitter( const ThumperDustEmitter & );
};

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bNewEntity - whether or not to start a new entity
//-----------------------------------------------------------------------------

void FX_ThumperDust( const CEffectData &data )
{
	Vector vecDustColor;
	vecDustColor.x = 0.85f;
	vecDustColor.y = 0.75f;
	vecDustColor.z = 0.52f;

	CSmartPtr<ThumperDustEmitter> pSimple = ThumperDustEmitter::Create( "thumperdust" );
	pSimple->SetSortOrigin( data.m_vOrigin );
	pSimple->SetNearClip( 16, 24 );

	SimpleParticle	*pParticle = NULL;

	Vector	offset;

	int	numPuffs = THUMPER_MAX_PARTICLES;

	PMaterialHandle	hMaterial[2];
	
	hMaterial[0] = pSimple->GetPMaterial("particle/particle_smokegrenade");
	hMaterial[1] = pSimple->GetPMaterial("particle/particle_noisesphere");

	float flTime = data.m_flScale / 128;

	float yaw;
	Vector forward, vRight, vForward;

	vForward = Vector( 0, 1, 0 );
	vRight = Vector( 1, 0, 0 );

	
	Vector vecColor;
	Vector vecFinalColor;
	int i = 0;

	for ( i = 0; i < numPuffs; i++ )
	{
		yaw = ( (float) i / (float) numPuffs ) * 360.0f;
		forward = ( vRight * sin( DEG2RAD( yaw) ) ) + ( vForward * cos( DEG2RAD( yaw ) ) );
		VectorNormalize( forward );

		offset = ( RandomVector( -4.0f, 4.0f ) + data.m_vOrigin ) + ( forward * 128.0f );

		trace_t	tr;
		UTIL_TraceLine( offset + Vector( 0, 0, 128 ), offset + Vector( 0, 0, -128 ), (MASK_SOLID_BRUSHONLY|CONTENTS_WATER), NULL, COLLISION_GROUP_NONE, &tr );


		pParticle = (SimpleParticle *) pSimple->AddParticle( sizeof(SimpleParticle), hMaterial[random->RandomInt(0,1)], tr.endpos );

		if ( pParticle != NULL )
		{	

			pParticle->m_flLifetime		= 0.0f;
			pParticle->m_flDieTime		= flTime + random->RandomFloat( flTime, flTime * 2 );
	
			Vector dir = (tr.endpos - data.m_vOrigin);
			float length = dir.Length();
			VectorNormalize( dir );

			pParticle->m_vecVelocity	= dir * ( length * 2.0f );
			pParticle->m_vecVelocity[2]	= data.m_flScale / 3;

			engine->ComputeLighting( offset, NULL, true, vecColor );
			
			VectorLerp( vecColor, vecDustColor, 0.5, vecColor );

			vecFinalColor = vecColor;

			pParticle->m_uchColor[0]	= vecFinalColor[0]*255;
			pParticle->m_uchColor[1]	= vecFinalColor[1]*255;
			pParticle->m_uchColor[2]	= vecFinalColor[2]*255;

			pParticle->m_uchStartAlpha	= random->RandomInt( 64, 96 );
			pParticle->m_uchEndAlpha	= 0;

			pParticle->m_uchStartSize	= data.m_flScale / ( random->RandomInt( 3, 4 )  + flTime);
			pParticle->m_uchEndSize		= data.m_flScale;

			pParticle->m_flRoll			= random->RandomInt( 0, 360 );
			pParticle->m_flRollDelta	= random->RandomFloat( -8.0f, 8.0f );
		}
	}

	for ( i = 0; i < numPuffs; i++ )
	{
		offset[0] = random->RandomFloat( -data.m_flScale, data.m_flScale );
		offset[1] = random->RandomFloat( -data.m_flScale, data.m_flScale );
		offset[2] = 0;
		offset += data.m_vOrigin;

		trace_t	tr;
		UTIL_TraceLine( offset + Vector( 0, 0, 128 ), offset + Vector( 0, 0, -128 ), (MASK_SOLID_BRUSHONLY|CONTENTS_WATER), NULL, COLLISION_GROUP_NONE, &tr );


		pParticle = (SimpleParticle *) pSimple->AddParticle( sizeof(SimpleParticle), hMaterial[random->RandomInt(0,1)], offset );
	
		if ( pParticle != NULL )
		{			
			pParticle->m_flLifetime		= 0.0f;
			pParticle->m_flDieTime		= flTime + random->RandomFloat( flTime, flTime * 2 );

			Vector dir = Vector( 0, 0, -1 );
			float length = 15;

			pParticle->m_vecVelocity	= dir * ( length * 4.0f );
			pParticle->m_vecVelocity[2]	= data.m_flScale / 4;

			engine->ComputeLighting( offset, NULL, true, vecColor );
			VectorLerp( vecColor, vecDustColor, 0.5, vecColor );
		
			vecFinalColor = vecColor;

			pParticle->m_uchColor[0]	= vecFinalColor[0]*255;
			pParticle->m_uchColor[1]	= vecFinalColor[1]*255;
			pParticle->m_uchColor[2]	= vecFinalColor[2]*255;

			pParticle->m_uchStartAlpha	= random->RandomInt( 64, 96 );
			pParticle->m_uchEndAlpha	= 0;

			pParticle->m_uchStartSize	= data.m_flScale / ( random->RandomInt( 3, 4 )  + flTime);
			pParticle->m_uchEndSize		= data.m_flScale + ( 20 * flTime );

			pParticle->m_flRoll			= random->RandomInt( 0, 360 );
			pParticle->m_flRollDelta	= random->RandomFloat( -4.0f, 4.0f );
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void ThumperDustCallback( const CEffectData &data )
{
	FX_ThumperDust( data );
}

DECLARE_CLIENT_EFFECT( "ThumperDust", ThumperDustCallback );