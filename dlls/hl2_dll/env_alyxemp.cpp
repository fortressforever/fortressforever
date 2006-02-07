//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Alyx's EMP effect
//
//=============================================================================//

#include "cbase.h"
#include "env_alyxemp_shared.h"
#include "beam_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	EMP_BEAM_SPRITE	"effects/laser1.vmt"

class CAlyxEmpEffect : public CBaseEntity
{
	DECLARE_CLASS( CAlyxEmpEffect, CBaseEntity );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

public:

	void	InputStartCharge( inputdata_t &inputdata );
	void	InputStartDischarge( inputdata_t &inputdata );
	void	InputStop( inputdata_t &inputdata );
	void	InputSetTargetEnt( inputdata_t &inputdata );

	void	Spawn( void );
	void	Precache( void );
	void	Activate( void );

private:
	
	void	SetTargetEntity( const char *szEntityName );

	CHandle<CBeam>			m_hBeam;
	CHandle<CBaseEntity>	m_hTargetEnt;
	string_t				m_strTargetName;
	int						m_nType;			// What type of effect this is (small, large)

	CNetworkVar( int, m_nState );
	CNetworkVar( float, m_flDuration );
	CNetworkVar( float, m_flStartTime );
};

LINK_ENTITY_TO_CLASS( env_alyxemp, CAlyxEmpEffect );

BEGIN_DATADESC( CAlyxEmpEffect )
	
	DEFINE_KEYFIELD( m_nType,			FIELD_INTEGER,	"Type" ),
	DEFINE_KEYFIELD( m_strTargetName,	FIELD_STRING,	"EndTargetName" ),

	DEFINE_FIELD( m_nState,			FIELD_INTEGER ),
	DEFINE_FIELD( m_flDuration,		FIELD_FLOAT ),
	DEFINE_FIELD( m_flStartTime,	FIELD_TIME ),
	DEFINE_FIELD( m_hTargetEnt,		FIELD_EHANDLE ),
	DEFINE_FIELD( m_hBeam,			FIELD_EHANDLE ),

	DEFINE_INPUTFUNC( FIELD_FLOAT, "StartCharge", InputStartCharge ),
	DEFINE_INPUTFUNC( FIELD_VOID, "StartDischarge", InputStartDischarge ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "Stop", InputStop ),
	DEFINE_INPUTFUNC( FIELD_STRING, "SetTargetEnt", InputSetTargetEnt ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CAlyxEmpEffect, DT_AlyxEmpEffect )
	SendPropInt( SENDINFO(m_nState), 8, SPROP_UNSIGNED),
	SendPropFloat( SENDINFO(m_flDuration), 0, SPROP_NOSCALE),
	SendPropFloat( SENDINFO(m_flStartTime), 0, SPROP_NOSCALE),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAlyxEmpEffect::Spawn( void )
{
	Precache();

	// No model but we still need to force this!
	AddEFlags( EFL_FORCE_CHECK_TRANSMIT );

	// No shadows
	AddEffects( EF_NOSHADOW | EF_NORECEIVESHADOW );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAlyxEmpEffect::Activate( void )
{
	// Start out with a target entity
	SetTargetEntity( STRING(m_strTargetName) );
	
	BaseClass::Activate();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *szEntityName - 
//-----------------------------------------------------------------------------
void CAlyxEmpEffect::SetTargetEntity( const char *szEntityName )
{
	// Find and store off our target entity
	if ( szEntityName && szEntityName[0] )
	{
		m_hTargetEnt = gEntList.FindEntityByName( NULL, szEntityName, this );

		if ( m_hTargetEnt == NULL )
		{
			Assert(0);
			DevMsg( "Unable to find env_alyxemp (%s) target %s!\n", GetEntityName(), szEntityName );
		}
	}
	else
	{
		m_hTargetEnt = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAlyxEmpEffect::Precache( void )
{
	PrecacheModel( EMP_BEAM_SPRITE );

	PrecacheScriptSound( "AlyxEmp.Charge" );
	PrecacheScriptSound( "AlyxEmp.Discharge" );
	PrecacheScriptSound( "AlyxEmp.Stop" );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAlyxEmpEffect::InputStartCharge( inputdata_t &inputdata )
{
	EmitSound( "AlyxEmp.Charge" );

	m_nState = ALYXEMP_STATE_CHARGING;
	m_flDuration = inputdata.value.Float();
	m_flStartTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAlyxEmpEffect::InputStartDischarge( inputdata_t &inputdata )
{
	EmitSound( "AlyxEmp.Discharge" );

	m_nState = ALYXEMP_STATE_DISCHARGING;
	m_flStartTime = gpGlobals->curtime;

	// Beam effects on the target entity!
	if ( !m_hBeam && m_hTargetEnt )
	{
		// Check to store off our view model index
		m_hBeam = CBeam::BeamCreate( EMP_BEAM_SPRITE, 8 );

		if ( m_hBeam != NULL )
		{
			m_hBeam->PointEntInit( m_hTargetEnt->GetAbsOrigin(), this );
			m_hBeam->SetWidth( 4 );
			m_hBeam->SetEndWidth( 8 );
			m_hBeam->SetBrightness( 255 );
			m_hBeam->SetColor( 255, 255, 255 );
			m_hBeam->LiveForTime( 999.0f );
			m_hBeam->RelinkBeam();
			m_hBeam->SetNoise( 16 );
		}

		// End hit
		Vector shotDir = ( GetAbsOrigin() - m_hTargetEnt->GetAbsOrigin() );
		VectorNormalize( shotDir );

		CPVSFilter filter( m_hTargetEnt->GetAbsOrigin() );
		te->GaussExplosion( filter, 0.0f, m_hTargetEnt->GetAbsOrigin() - ( shotDir * 4.0f ), RandomVector(-1.0f, 1.0f), 0 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAlyxEmpEffect::InputStop( inputdata_t &inputdata )
{
	EmitSound( "AlyxEmp.Stop" );

	m_nState = ALYXEMP_STATE_OFF;
	m_flDuration = inputdata.value.Float();
	m_flStartTime = gpGlobals->curtime;

	if ( m_hBeam != NULL )
	{
		UTIL_Remove( m_hBeam );
		m_hBeam = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CAlyxEmpEffect::InputSetTargetEnt( inputdata_t &inputdata )
{
	SetTargetEntity( inputdata.value.String() );
}
