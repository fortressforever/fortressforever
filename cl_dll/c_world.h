//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#if !defined( C_WORLD_H )
#define C_WORLD_H
#ifdef _WIN32
#pragma once
#endif

#include "c_baseentity.h"

#if defined( CLIENT_DLL )
#define CWorld C_World
#endif

class C_World : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_World, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	C_World( void );
	~C_World( void );
	
	// Override the factory create/delete functions since the world is a singleton.
	virtual bool Init( int entnum, int iSerialNum );
	virtual void Release();

	virtual void Precache();
	virtual void Spawn();

	// Don't worry about adding the world to the collision list; it's already there
	virtual CollideType_t	ShouldCollide( )	{ return ENTITY_SHOULD_NOT_COLLIDE; }

	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual void PreDataUpdate( DataUpdateType_t updateType );

	inline float GetWaveHeight() const
	{
		return m_flWaveHeight;
	}

public:
	float	m_flWaveHeight;
	Vector	m_WorldMins;
	Vector	m_WorldMaxs;
	bool	m_bStartDark;
	float	m_flMaxOccludeeArea;
	float	m_flMinOccluderArea;
	float	m_flMinPropScreenSpaceWidth;
	float	m_flMaxPropScreenSpaceWidth;

private:
	void	RegisterSharedActivities( void );
};

void ClientWorldFactoryInit();
void ClientWorldFactoryShutdown();
C_World* GetClientWorldEntity();

#endif // C_WORLD_H