//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef FUNC_LADDER_H
#define FUNC_LADDER_H
#ifdef _WIN32
#pragma once
#endif

#if defined( CLIENT_DLL )
#define CFuncLadder C_FuncLadder
#define CInfoLadderDismount C_InfoLadderDismount
#endif

class CInfoLadderDismount : public CBaseEntity
{
public:
	DECLARE_CLASS( CInfoLadderDismount, CBaseEntity );
	DECLARE_NETWORKCLASS();

	virtual void DrawDebugGeometryOverlays();
};

typedef CHandle< CInfoLadderDismount > CInfoLadderDismountHandle;

// Spawnflags
#define SF_LADDER_DONTGETON			1			// Set for ladders that are acting as automount points, but not really ladders

//-----------------------------------------------------------------------------
// Purpose: A player-climbable ladder
//-----------------------------------------------------------------------------
class CFuncLadder : public CBaseEntity
{
public:

	DECLARE_CLASS( CFuncLadder, CBaseEntity );
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

	CFuncLadder();
	~CFuncLadder();

	virtual void Spawn();

	virtual void DrawDebugGeometryOverlays(void);

	int					GetDismountCount() const;
	CInfoLadderDismount	*GetDismount( int index );

	void	GetTopPosition( Vector& org );
	void	GetBottomPosition( Vector& org );
	void	ComputeLadderDir( Vector& bottomToTopVec );

	void	SetEndPoints( const Vector& p1, const Vector& p2 );

	void	InputEnable( inputdata_t &inputdata );
	void	InputDisable( inputdata_t &inputdata );

	bool	IsEnabled() const;

	void	PlayerGotOn( CBasePlayer *pPlayer );
	void	PlayerGotOff( CBasePlayer *pPlayer );

	virtual void Activate();

	bool	DontGetOnLadder( void ) const;

	static int GetLadderCount();
	static CFuncLadder *GetLadder( int index );
	static CUtlVector< CFuncLadder * >	s_Ladders;
public:

	void FindNearbyDismountPoints( const Vector& origin, float radius, CUtlVector< CInfoLadderDismountHandle >& list );

private:


	void	SearchForDismountPoints();

	// Movement vector from "bottom" to "top" of ladder
	CNetworkVector( m_vecLadderDir );

	// Dismount points near top/bottom of ladder, precomputed
	CUtlVector< CInfoLadderDismountHandle > m_Dismounts;

	// Endpoints for checking for mount/dismount
	CNetworkVector( m_vecPlayerMountPositionTop );
	CNetworkVector( m_vecPlayerMountPositionBottom );

	bool		m_bDisabled;
	CNetworkVar( bool,	m_bFakeLadder );

#if defined( GAME_DLL )
	//-----------------------------------------------------
	//	Outputs
	//-----------------------------------------------------
	COutputEvent	m_OnPlayerGotOnLadder;
	COutputEvent	m_OnPlayerGotOffLadder;

	virtual int UpdateTransmitState();
#endif
};

inline bool CFuncLadder::IsEnabled() const
{
	return !m_bDisabled;
}

#endif // FUNC_LADDER_H
