//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

// Particle system entities can derive from this to handle some of the mundane
// functionality of hooking into the engine's entity system.

#ifndef PARTICLE_BASEEFFECT_H
#define PARTICLE_BASEEFFECT_H

#include "predictable_entity.h"
#include "baseentity_shared.h"

#if defined( CLIENT_DLL )
#define CBaseParticleEntity C_BaseParticleEntity

#include "particlemgr.h"

#endif 

class CBaseParticleEntity : public CBaseEntity
#if defined( CLIENT_DLL )
, public IParticleEffect
#endif
{
public:
	DECLARE_CLASS( CBaseParticleEntity, CBaseEntity );
	DECLARE_PREDICTABLE();
	DECLARE_NETWORKCLASS();

	CBaseParticleEntity();

	// CBaseEntity overrides.
public:
#if !defined( CLIENT_DLL )
	virtual int		UpdateTransmitState( void );	
#else
// Default IParticleEffect overrides.
public:

	virtual void SimulateParticles( CParticleSimulateIterator *pIterator );
	virtual void RenderParticles( CParticleRenderIterator *pIterator );
	virtual const Vector & GetSortOrigin();
public:
	CParticleEffectBinding	m_ParticleEffect;
#endif

	virtual void		Activate();
	virtual void		Think();	

#if defined( CLIENT_DLL )
	// NOTE: Ths enclosed particle effect binding will do all the drawing
	virtual bool		ShouldDraw() { return false; }
#endif

public:
	void				FollowEntity(CBaseEntity *pEntity);
	
	// UTIL_Remove will be called after the specified amount of time.
	// If you pass in -1, the entity will never go away automatically.
	void				SetLifetime(float lifetime);

private:
	CBaseParticleEntity( const CBaseParticleEntity & ); // not defined, not accessible
};



#endif


