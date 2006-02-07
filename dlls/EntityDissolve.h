//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef ENTITYDISSOLVE_H
#define ENTITYDISSOLVE_H

#ifdef _WIN32
#pragma once
#endif

class CEntityDissolve : public CBaseEntity 
{
public:
	DECLARE_SERVERCLASS();
	DECLARE_CLASS( CEntityDissolve, CBaseEntity );

	CEntityDissolve( void );
	~CEntityDissolve( void );

	static CEntityDissolve	*Create( CBaseEntity *pTarget, const char *pMaterialName, 
		float flStartTime, int nDissolveType = 0, bool *pRagdollCreated = NULL );
	static CEntityDissolve	*Create( CBaseEntity *pTarget, CBaseEntity *pSource );
	
	void	Precache();
	void	Spawn();
	void	AttachToEntity( CBaseEntity *pTarget );
	void	SetStartTime( float flStartTime );

	DECLARE_DATADESC();

protected:
	void	InputDissolve( inputdata_t &inputdata );
	void	DissolveThink( void );
	void	ElectrocuteThink( void );

	CNetworkVar( float, m_flStartTime );
	CNetworkVar( float, m_flFadeInStart );
	CNetworkVar( float, m_flFadeInLength );
	CNetworkVar( float, m_flFadeOutModelStart );
	CNetworkVar( float, m_flFadeOutModelLength );
	CNetworkVar( float, m_flFadeOutStart );
	CNetworkVar( float, m_flFadeOutLength );
	CNetworkVar( int, m_nDissolveType );
};

#endif // ENTITYDISSOLVE_H
