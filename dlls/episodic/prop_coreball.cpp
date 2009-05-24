//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Big pulsating ball inside the core of the citadel
//
//=============================================================================//

#include "cbase.h"
#include "baseentity.h"

#define COREBALL_MODEL "models/props_combine/coreball.mdl"

class CPropCoreBall : public CBaseAnimating
{
public:
	DECLARE_CLASS( CPropCoreBall, CBaseAnimating );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CPropCoreBall();

	virtual void Spawn( void );
	virtual void Precache( void );

	CNetworkVar( float, m_flScaleX );
	CNetworkVar( float, m_flScaleY );
	CNetworkVar( float, m_flScaleZ );

	CNetworkVar( float, m_flLerpTimeX );
	CNetworkVar( float, m_flLerpTimeY );
	CNetworkVar( float, m_flLerpTimeZ );

	CNetworkVar( float, m_flGoalTimeX );
	CNetworkVar( float, m_flGoalTimeY );
	CNetworkVar( float, m_flGoalTimeZ );

	void InputSetScaleX( inputdata_t &inputdata );
	void InputSetScaleY( inputdata_t &inputdata );
	void InputSetScaleZ( inputdata_t &inputdata );
};

LINK_ENTITY_TO_CLASS( prop_coreball, CPropCoreBall );

BEGIN_DATADESC( CPropCoreBall )
	DEFINE_INPUTFUNC( FIELD_VECTOR, "SetScaleX", InputSetScaleX ),
	DEFINE_INPUTFUNC( FIELD_VECTOR, "SetScaleY", InputSetScaleY ),
	DEFINE_INPUTFUNC( FIELD_VECTOR, "SetScaleZ", InputSetScaleZ ),

	DEFINE_FIELD( m_flScaleX, FIELD_FLOAT ),
	DEFINE_FIELD( m_flScaleY, FIELD_FLOAT ),
	DEFINE_FIELD( m_flScaleZ, FIELD_FLOAT ),

	DEFINE_FIELD( m_flLerpTimeX, FIELD_FLOAT ),
	DEFINE_FIELD( m_flLerpTimeY, FIELD_FLOAT ),
	DEFINE_FIELD( m_flLerpTimeZ, FIELD_FLOAT ),

	DEFINE_FIELD( m_flGoalTimeX, FIELD_FLOAT ),
	DEFINE_FIELD( m_flGoalTimeY, FIELD_FLOAT ),
	DEFINE_FIELD( m_flGoalTimeZ, FIELD_FLOAT ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CPropCoreBall, DT_PropCoreBall )
	SendPropFloat( SENDINFO(m_flScaleX), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO(m_flScaleY), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO(m_flScaleZ), 0, SPROP_NOSCALE ),

	SendPropFloat( SENDINFO(m_flLerpTimeX), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO(m_flLerpTimeY), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO(m_flLerpTimeZ), 0, SPROP_NOSCALE ),

	SendPropFloat( SENDINFO(m_flGoalTimeX), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO(m_flGoalTimeY), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO(m_flGoalTimeZ), 0, SPROP_NOSCALE ),
END_SEND_TABLE()

CPropCoreBall::CPropCoreBall( void )
{
	m_flScaleX = 1.0f;
	m_flScaleY = 1.0f;
	m_flScaleZ = 1.0f;

	UseClientSideAnimation();
}

void CPropCoreBall::Spawn( void )
{
	Precache();
	SetModel( COREBALL_MODEL );

	SetMoveType( MOVETYPE_NONE );

	BaseClass::Spawn();

	AddEffects( EF_NOSHADOW );

	SetSequence( 0 );
	SetPlaybackRate( 1.0f );
}

void CPropCoreBall::Precache( void )
{
	BaseClass::Precache();
	PrecacheModel( COREBALL_MODEL );
}

void CPropCoreBall::InputSetScaleX( inputdata_t &inputdata )
{
	Vector vecScale;
	inputdata.value.Vector3D( vecScale );

	m_flScaleX = vecScale.x;
	m_flLerpTimeX = vecScale.y;
	m_flGoalTimeX = gpGlobals->curtime;
}

void CPropCoreBall::InputSetScaleY( inputdata_t &inputdata )
{
	Vector vecScale;
	inputdata.value.Vector3D( vecScale );

	m_flScaleY = vecScale.x;
	m_flLerpTimeY = vecScale.y;
	m_flGoalTimeY = gpGlobals->curtime;
}

void CPropCoreBall::InputSetScaleZ( inputdata_t &inputdata )
{
	Vector vecScale;
	inputdata.value.Vector3D( vecScale );

	m_flScaleZ = vecScale.x;
	m_flLerpTimeZ = vecScale.y;
	m_flGoalTimeZ = gpGlobals->curtime;
}