//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Material modify control entity.
//
//=============================================================================//

#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//------------------------------------------------------------------------------
// FIXME: This really should inherit from something	more lightweight.
//------------------------------------------------------------------------------

#define MATERIAL_MODIFY_STRING_SIZE		255

class CMaterialModifyControl : public CBaseEntity
{
public:

	DECLARE_CLASS( CMaterialModifyControl, CBaseEntity );

	CMaterialModifyControl();

	void Spawn( void );
	bool KeyValue( const char *szKeyName, const char *szValue );
	int UpdateTransmitState();
	int ShouldTransmit( const CCheckTransmitInfo *pInfo );

	void SetMaterialVar( inputdata_t &inputdata );
	void SetMaterialVarToCurrentTime( inputdata_t &inputdata );

	virtual int	ObjectCaps( void ) { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

private:
	CNetworkString( m_szMaterialName, MATERIAL_MODIFY_STRING_SIZE );
	CNetworkString( m_szMaterialVar, MATERIAL_MODIFY_STRING_SIZE );
	CNetworkString( m_szMaterialVarValue, MATERIAL_MODIFY_STRING_SIZE );
};

LINK_ENTITY_TO_CLASS(material_modify_control, CMaterialModifyControl);

BEGIN_DATADESC( CMaterialModifyControl )
	// Variables.
	DEFINE_AUTO_ARRAY( m_szMaterialName, FIELD_CHARACTER ),
	DEFINE_AUTO_ARRAY( m_szMaterialVar, FIELD_CHARACTER ),
	DEFINE_AUTO_ARRAY( m_szMaterialVarValue, FIELD_CHARACTER ),
	// Inputs.
	DEFINE_INPUTFUNC( FIELD_STRING, "SetMaterialVar", SetMaterialVar ),
	DEFINE_INPUTFUNC( FIELD_VOID, "SetMaterialVarToCurrentTime", SetMaterialVarToCurrentTime ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CMaterialModifyControl, DT_MaterialModifyControl)
	SendPropString( SENDINFO( m_szMaterialName ) ),
	SendPropString( SENDINFO( m_szMaterialVar ) ),
	SendPropString( SENDINFO( m_szMaterialVarValue ) ),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CMaterialModifyControl::CMaterialModifyControl()
{
}

//------------------------------------------------------------------------------
// Purpose :
//------------------------------------------------------------------------------
void CMaterialModifyControl::Spawn( void )
{
	Precache();
	SetSolid( SOLID_NONE );
}

//------------------------------------------------------------------------------
// Purpose :
//------------------------------------------------------------------------------
bool CMaterialModifyControl::KeyValue( const char *szKeyName, const char *szValue )
{
	if ( FStrEq( szKeyName, "materialName" ) )
	{
		Q_strncpy( m_szMaterialName.GetForModify(), szValue, MATERIAL_MODIFY_STRING_SIZE );
		return true;
	}

	if ( FStrEq( szKeyName, "materialVar" ) )
	{
		Q_strncpy( m_szMaterialVar.GetForModify(), szValue, MATERIAL_MODIFY_STRING_SIZE );
		return true;
	}

	return BaseClass::KeyValue( szKeyName, szValue );
}

//------------------------------------------------------------------------------
// Purpose : Send even though we don't have a model.
//------------------------------------------------------------------------------
int CMaterialModifyControl::UpdateTransmitState()
{
	// ALWAYS transmit to all clients.
	return SetTransmitState( FL_EDICT_FULLCHECK );
}


//-----------------------------------------------------------------------------
// Send if the parent is being sent:
//-----------------------------------------------------------------------------
int CMaterialModifyControl::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	CBaseEntity *pEnt = GetMoveParent();
	if ( pEnt )
		return pEnt->ShouldTransmit( pInfo );
	
	return FL_EDICT_DONTSEND;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMaterialModifyControl::SetMaterialVar( inputdata_t &inputdata )
{
	Q_strncpy( m_szMaterialVarValue.GetForModify(), inputdata.value.String(), MATERIAL_MODIFY_STRING_SIZE );
}

void CMaterialModifyControl::SetMaterialVarToCurrentTime( inputdata_t &inputdata )
{
	char temp[32];
	Q_snprintf( temp, 32, "%f", gpGlobals->curtime );
	Q_strncpy( m_szMaterialVarValue.GetForModify(), temp, MATERIAL_MODIFY_STRING_SIZE );
}
