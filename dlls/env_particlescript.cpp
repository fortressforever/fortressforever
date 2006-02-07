//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "baseanimating.h"
#include "SkyCamera.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// An entity which emits other entities at points 
//-----------------------------------------------------------------------------
class CEnvParticleScript : public CBaseAnimating
{
public:
	DECLARE_CLASS( CEnvParticleScript, CBaseAnimating );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CEnvParticleScript();

	virtual void Precache();
	virtual void Spawn();
	virtual void Activate();
	virtual int  UpdateTransmitState();

	void InputSetSequence( inputdata_t &inputdata );

private:
	CNetworkVar( float, m_flSequenceScale );
};


//-----------------------------------------------------------------------------
// Save/load 
//-----------------------------------------------------------------------------
BEGIN_DATADESC( CEnvParticleScript )

	DEFINE_FIELD( m_flSequenceScale, FIELD_FLOAT ),
	
	// Inputs
	DEFINE_INPUTFUNC( FIELD_STRING, "SetSequence", InputSetSequence ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( env_particlescript, CEnvParticleScript );


//-----------------------------------------------------------------------------
// Datatable
//-----------------------------------------------------------------------------
IMPLEMENT_SERVERCLASS_ST( CEnvParticleScript, DT_EnvParticleScript )
	SendPropFloat(SENDINFO(m_flSequenceScale), 0, SPROP_NOSCALE),
END_SEND_TABLE()


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CEnvParticleScript::CEnvParticleScript()
{
	UseClientSideAnimation();
}


//-----------------------------------------------------------------------------
// Precache
//-----------------------------------------------------------------------------
void CEnvParticleScript::Precache()
{
	BaseClass::Precache();
	PrecacheModel( STRING( GetModelName() ) );
}


//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CEnvParticleScript::Spawn()
{
	Precache();
	BaseClass::Spawn();
	AddEffects( EF_NOSHADOW );
	// We need a model for its animation sequences even though we don't render it
	SetModel( STRING( GetModelName() ) );
}


//-----------------------------------------------------------------------------
// Activate
//-----------------------------------------------------------------------------
void CEnvParticleScript::Activate()
{
	BaseClass::Activate();

	DetectInSkybox();
	CSkyCamera *pCamera = GetEntitySkybox();
	if ( pCamera )
	{
		float flSkyboxScale = pCamera->m_skyboxData.scale;
		if ( flSkyboxScale == 0.0f )
		{
			flSkyboxScale = 1.0f;
		}

		m_flSequenceScale = flSkyboxScale;
	}
	else
	{
		m_flSequenceScale = 1.0f;
	}

	m_flPlaybackRate = 1.0f;
}

//-----------------------------------------------------------------------------
// Should we transmit it to the client?
//-----------------------------------------------------------------------------
int CEnvParticleScript::UpdateTransmitState()
{
	if ( IsEffectActive( EF_NODRAW ) )
	{
		return SetTransmitState( FL_EDICT_DONTSEND );
	}

	if ( IsEFlagSet( EFL_IN_SKYBOX ) )
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

	return SetTransmitState( FL_EDICT_PVSCHECK );
}


//-----------------------------------------------------------------------------
// Purpose: Input that sets the sequence of the entity
//-----------------------------------------------------------------------------
void CEnvParticleScript::InputSetSequence( inputdata_t &inputdata )
{
	if ( inputdata.value.StringID() != NULL_STRING )
	{
		int nSequence = LookupSequence( STRING( inputdata.value.StringID() ) );
		if ( nSequence != ACT_INVALID )
		{
			SetSequence( nSequence );
		}
	}
}
