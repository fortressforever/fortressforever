//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "baseentity.h"
#include "sendproxy.h"
#include "sun_shared.h"
#include "map_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CSun : public CBaseEntity
{
public:
	DECLARE_CLASS( CSun, CBaseEntity );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CSun();

	virtual void	Activate();

	// Input handlers
	void InputTurnOn( inputdata_t &inputdata );
	void InputTurnOff( inputdata_t &inputdata );
	void InputSetColor( inputdata_t &inputdata );

	virtual int UpdateTransmitState();

public:
	CNetworkVector( m_vDirection );
	
	int		m_bUseAngles;
	float	m_flPitch;
	float	m_flYaw;
	
	CNetworkVar( int, m_nSize );
	CNetworkVar( bool, m_bOn );
};

IMPLEMENT_SERVERCLASS_ST_NOBASE( CSun, DT_Sun )
	SendPropInt( SENDINFO(m_clrRender), 32, SPROP_UNSIGNED, SendProxy_Color32ToInt ),
	SendPropVector( SENDINFO(m_vDirection), 0, SPROP_NORMAL ),
	SendPropInt( SENDINFO(m_bOn), 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO(m_nSize), 10, SPROP_UNSIGNED ),
END_SEND_TABLE()


LINK_ENTITY_TO_CLASS( env_sun, CSun );


BEGIN_DATADESC( CSun )

	DEFINE_FIELD( m_vDirection,		FIELD_VECTOR ),
	
	DEFINE_KEYFIELD( m_bUseAngles, FIELD_INTEGER, "use_angles" ),
	DEFINE_KEYFIELD( m_flPitch, FIELD_FLOAT, "pitch" ),
	DEFINE_KEYFIELD( m_flYaw, FIELD_FLOAT, "angle" ),
	DEFINE_KEYFIELD( m_nSize, FIELD_INTEGER, "size" ),

	DEFINE_FIELD( m_bOn, FIELD_BOOLEAN ),

	DEFINE_INPUTFUNC( FIELD_VOID, "TurnOn", InputTurnOn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "TurnOff", InputTurnOff ),
	DEFINE_INPUTFUNC( FIELD_COLOR32, "SetColor", InputSetColor )

END_DATADESC()

CSun::CSun()
{
	m_vDirection.Init( 0, 0, 1 );
	
	m_bUseAngles = false;
	m_flPitch = 0;
	m_flYaw = 0;
	m_nSize = 16;

	m_bOn = true;
	AddEFlags( EFL_FORCE_CHECK_TRANSMIT );
}

void CSun::Activate()
{
	BaseClass::Activate();

	// Find our target.
	if ( m_bUseAngles )
	{
		SetupLightNormalFromProps( GetAbsAngles(), m_flYaw, m_flPitch, m_vDirection.GetForModify() );
		m_vDirection = -m_vDirection.Get();
	}
	else
	{
		CBaseEntity *pEnt = gEntList.FindEntityByName( 0, m_target, NULL );
		if( pEnt )
		{
			Vector vDirection = GetAbsOrigin() - pEnt->GetAbsOrigin();
			VectorNormalize( vDirection );
			m_vDirection = vDirection;
		}
	}

	NetworkStateChanged();
}

void CSun::InputTurnOn( inputdata_t &inputdata )
{
	if( !m_bOn )
	{
		m_bOn = true;
		NetworkStateChanged();
	}
}

void CSun::InputTurnOff( inputdata_t &inputdata )
{
	if ( m_bOn )
	{
		m_bOn = false;
		NetworkStateChanged();
	}
}

void CSun::InputSetColor( inputdata_t &inputdata )
{
	m_clrRender = inputdata.value.Color32();
	NetworkStateChanged();
}

int CSun::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}


