//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef ENTITYFLAME_H
#define ENTITYFLAME_H
#ifdef _WIN32
#pragma once
#endif

#define FLAME_DAMAGE_INTERVAL			0.2f // How often to deal damage.
#define FLAME_DIRECT_DAMAGE_PER_SEC		0 // 5.0f	// |-- Mirv: We seem to be handling dmg ourself
#define FLAME_RADIUS_DAMAGE_PER_SEC		0 // 4.0f	// |-- Mirv: We seem to be handling dmg ourself

#define FLAME_DIRECT_DAMAGE ( FLAME_DIRECT_DAMAGE_PER_SEC * FLAME_DAMAGE_INTERVAL )
#define FLAME_RADIUS_DAMAGE ( FLAME_RADIUS_DAMAGE_PER_SEC * FLAME_DAMAGE_INTERVAL )

#define FLAME_MAX_LIFETIME_ON_DEAD_NPCS	10.0f

class CEntityFlame : public CBaseEntity 
{
public:
	DECLARE_SERVERCLASS();
	DECLARE_CLASS( CEntityFlame, CBaseEntity );

	CEntityFlame( void );

	static CEntityFlame	*Create( CBaseEntity *pTarget, bool useHitboxes = true, float flameSize = 1.0f );

	void	AttachToEntity( CBaseEntity *pTarget );
	void	SetLifetime( float lifetime );
	void	SetUseHitboxes( bool use );

	// Mulchman
	void	Extinguish( void );

	virtual void Precache();
	virtual void UpdateOnRemove();

	void	SetSize( float size ) { m_flSize = size; }

	friend class CFFPlayer;		// |-- Mirv: Bug #0000162: Switching class while on fire, keeps playing burn sound

	DECLARE_DATADESC();

protected:

	void InputIgnite( inputdata_t &inputdata );

	void	FlameThink( void );

	CNetworkHandle( CBaseEntity, m_hEntAttached );		// The entity that we are burning (attached to).

	CNetworkVar( float, m_flSize );
	CNetworkVar( bool, m_bUseHitboxes );

	CNetworkVar( float, m_flLifetime );
	bool	m_bPlayingSound;
};

#endif // ENTITYFLAME_H
