//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "shareddefs.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterial.h"
#include "view.h"
#include "iviewrender.h"
#include "view_shared.h"
#include "texture_group_names.h"
#include "vstdlib/icommandline.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_EnvScreenOverlay : public C_BaseEntity
{
	DECLARE_CLASS( C_EnvScreenOverlay, C_BaseEntity );
public:
	DECLARE_CLIENTCLASS();

	void	PreDataUpdate( DataUpdateType_t updateType );
	void	PostDataUpdate( DataUpdateType_t updateType );

	void	HandleOverlaySwitch( void );
	void	StartOverlays( void );
	void	StopOverlays( void );
	void	StartCurrentOverlay( void );
	void	ClientThink( void );

protected:
	char	m_iszOverlayNames[ MAX_SCREEN_OVERLAYS ][255];
	float	m_flOverlayTimes[ MAX_SCREEN_OVERLAYS ];
	float	m_flStartTime;
	int     m_iDesiredOverlay;
	bool	m_bIsActive;
	bool	m_bWasActive;
	int		m_iCachedDesiredOverlay;
	int		m_iCurrentOverlay;
	float	m_flCurrentOverlayTime;
};

IMPLEMENT_CLIENTCLASS_DT( C_EnvScreenOverlay, DT_EnvScreenOverlay, CEnvScreenOverlay )
	RecvPropArray( RecvPropString( RECVINFO( m_iszOverlayNames[0]) ), m_iszOverlayNames ),
	RecvPropArray( RecvPropFloat( RECVINFO( m_flOverlayTimes[0] ) ), m_flOverlayTimes ),
	RecvPropFloat( RECVINFO( m_flStartTime ) ),
	RecvPropInt( RECVINFO( m_iDesiredOverlay ) ),
	RecvPropBool( RECVINFO( m_bIsActive ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_EnvScreenOverlay::PreDataUpdate( DataUpdateType_t updateType )
{
	BaseClass::PreDataUpdate( updateType );

	m_bWasActive = m_bIsActive;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_EnvScreenOverlay::PostDataUpdate( DataUpdateType_t updateType )
{
	BaseClass::PostDataUpdate( updateType );

	// If we have a start time now, start the overlays going
	if ( m_bIsActive && m_flStartTime > 0 && view->GetScreenOverlayMaterial() == NULL )
	{
		StartOverlays();
	}
	
	if ( m_flStartTime == -1 )
	{
		 StopOverlays();
	}

	HandleOverlaySwitch();

	if ( updateType == DATA_UPDATE_CREATED &&
		CommandLine()->FindParm( "-makereslists" ) )
	{
		for ( int i = 0; i < MAX_SCREEN_OVERLAYS; ++i )
		{
			if ( m_iszOverlayNames[ i ] && m_iszOverlayNames[ i ][ 0 ] )
			{
				materials->FindMaterial( m_iszOverlayNames[ i ], TEXTURE_GROUP_CLIENT_EFFECTS, false );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_EnvScreenOverlay::StopOverlays( void )
{
	SetNextClientThink( CLIENT_THINK_NEVER );

	if ( m_bWasActive && !m_bIsActive )
	{
		view->SetScreenOverlayMaterial( NULL );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_EnvScreenOverlay::StartOverlays( void )
{
	m_iCurrentOverlay = 0;
	m_flCurrentOverlayTime = 0;
	m_iCachedDesiredOverlay	= 0;
	SetNextClientThink( CLIENT_THINK_ALWAYS );

	StartCurrentOverlay();
	HandleOverlaySwitch();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_EnvScreenOverlay::HandleOverlaySwitch( void )
{
	if( m_iCachedDesiredOverlay != m_iDesiredOverlay )
	{
		m_iCurrentOverlay = m_iDesiredOverlay;
		m_iCachedDesiredOverlay = m_iDesiredOverlay;
		StartCurrentOverlay();
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_EnvScreenOverlay::StartCurrentOverlay( void )
{
	if ( m_iCurrentOverlay == MAX_SCREEN_OVERLAYS || !m_iszOverlayNames[m_iCurrentOverlay] || !m_iszOverlayNames[m_iCurrentOverlay][0] )
	{
		// Hit the end of our overlays, so stop.
		m_flStartTime = 0;
		StopOverlays();
		return;
	}

	if ( m_flOverlayTimes[m_iCurrentOverlay] == -1 )
		 m_flCurrentOverlayTime = -1;
	else
		 m_flCurrentOverlayTime = gpGlobals->curtime + m_flOverlayTimes[m_iCurrentOverlay];

	// Bring up the current overlay
	IMaterial *pMaterial = materials->FindMaterial( m_iszOverlayNames[m_iCurrentOverlay], TEXTURE_GROUP_CLIENT_EFFECTS, false );
	if ( !IsErrorMaterial( pMaterial ) )
	{
		view->SetScreenOverlayMaterial( pMaterial );
	}
	else
	{
		Warning("env_screenoverlay couldn't find overlay %s.\n", m_iszOverlayNames[m_iCurrentOverlay] );
		StopOverlays();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_EnvScreenOverlay::ClientThink( void )
{
	// If the current overlay's run out, go to the next one
	if ( m_flCurrentOverlayTime != -1 && m_flCurrentOverlayTime < gpGlobals->curtime )
	{
		m_iCurrentOverlay++;
		StartCurrentOverlay();
	}
}
