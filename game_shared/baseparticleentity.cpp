//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "baseparticleentity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( BaseParticleEntity, DT_BaseParticleEntity )

BEGIN_NETWORK_TABLE( CBaseParticleEntity, DT_BaseParticleEntity )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(	CBaseParticleEntity )
END_PREDICTION_DATA()

CBaseParticleEntity::CBaseParticleEntity( void )
{
}

#if !defined( CLIENT_DLL )
int CBaseParticleEntity::UpdateTransmitState( void )
{
	if ( IsEffectActive( EF_NODRAW ) )
		return SetTransmitState( FL_EDICT_DONTSEND );

	if ( IsEFlagSet( EFL_IN_SKYBOX ) )
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

	// cull against PVS
	return SetTransmitState( FL_EDICT_PVSCHECK );
}
#endif

void CBaseParticleEntity::Activate()
{
#if !defined( CLIENT_DLL )
	BaseClass::Activate();
#endif
}	


void CBaseParticleEntity::Think()
{
	Remove( );
}


void CBaseParticleEntity::FollowEntity(CBaseEntity *pEntity)
{
	BaseClass::FollowEntity( pEntity );
	SetLocalOrigin( vec3_origin );
}


void CBaseParticleEntity::SetLifetime(float lifetime)
{
	if(lifetime == -1)
		SetNextThink( TICK_NEVER_THINK );
	else
		SetNextThink( gpGlobals->curtime + lifetime );
}

#if defined( CLIENT_DLL )
const Vector &CBaseParticleEntity::GetSortOrigin()
{
	return GetAbsOrigin();
}

void CBaseParticleEntity::SimulateParticles( CParticleSimulateIterator *pIterator )
{
	// If you derive from CBaseParticleEntity, you must implement simulation and rendering.
	Assert( false );
}

void CBaseParticleEntity::RenderParticles( CParticleRenderIterator *pIterator )
{
	// If you derive from CBaseParticleEntity, you must implement simulation and rendering.
	Assert( false );
}

#endif
