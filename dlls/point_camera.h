//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CAMERA_H
#define CAMERA_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CPointCamera : public CBaseEntity
{
public:
	DECLARE_CLASS( CPointCamera, CBaseEntity );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	CPointCamera();
	~CPointCamera();

	void Spawn( void );

	// Tell the client that this camera needs to be rendered
	void SetActive( bool bActive );
	int  UpdateTransmitState(void);

	void ChangeFOVThink( void );

	// Bug #0000390: multiple render targets for cameras
	string_t GetRenderTarget( void ) { return m_szRenderTarget.Get(); }

	void InputChangeFOV( inputdata_t &inputdata );
	void InputSetOnAndTurnOthersOff( inputdata_t &inputdata );
	void InputSetOn( inputdata_t &inputdata );
	void InputSetOff( inputdata_t &inputdata );

private:
	float m_TargetFOV;
	float m_DegreesPerSecond;

	CNetworkVar( float, m_FOV );
	CNetworkVar( float, m_Resolution );
	CNetworkVar( bool, m_bFogEnable );
	CNetworkColor32( m_FogColor );
	CNetworkVar( float, m_flFogStart );
	CNetworkVar( float, m_flFogEnd );
	CNetworkVar( bool, m_bActive );
	CNetworkVar( bool, m_bUseScreenAspectRatio );
	// Bug #0000390: multiple render targets for cameras
	CNetworkVar( string_t, m_szRenderTarget );

	// Allows the mapmaker to control whether a camera is active or not
	bool	m_bIsOn;

public:
	CPointCamera	*m_pNext;
};

CPointCamera *GetPointCameraList();
#endif // CAMERA_H
