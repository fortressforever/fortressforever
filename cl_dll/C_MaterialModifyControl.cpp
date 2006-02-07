//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Material Modify control entity.
//
//=============================================================================//

#include "cbase.h"
#include "ProxyEntity.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMaterialVar.h"
#include "iviewrender.h"
#include "texture_group_names.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MATERIAL_MODIFY_STRING_SIZE		255

//------------------------------------------------------------------------------
// FIXME: This really should inherit from something	more lightweight
//------------------------------------------------------------------------------

class C_MaterialModifyControl : public C_BaseEntity
{
public:

	DECLARE_CLASS( C_MaterialModifyControl, C_BaseEntity );

	C_MaterialModifyControl();

	void OnDataChanged( DataUpdateType_t updateType );
	bool ShouldDraw();

	IMaterial *GetMaterial( void )					{ return m_pMaterial; }
	const char *GetMaterialVariableName( void )		{ return m_szMaterialVar; }
	const char *GetMaterialVariableValue( void )	{ return m_szMaterialVarValue; }

	DECLARE_CLIENTCLASS();

private:

	char m_szMaterialName[MATERIAL_MODIFY_STRING_SIZE];
	char m_szMaterialVar[MATERIAL_MODIFY_STRING_SIZE];
	char m_szMaterialVarValue[MATERIAL_MODIFY_STRING_SIZE];

	IMaterial		*m_pMaterial;
};

IMPLEMENT_CLIENTCLASS_DT(C_MaterialModifyControl, DT_MaterialModifyControl, CMaterialModifyControl)
	RecvPropString( RECVINFO( m_szMaterialName ) ),
	RecvPropString( RECVINFO( m_szMaterialVar ) ),
	RecvPropString( RECVINFO( m_szMaterialVarValue ) ),
END_RECV_TABLE()

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
C_MaterialModifyControl::C_MaterialModifyControl()
{
	m_pMaterial = NULL;
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void C_MaterialModifyControl::OnDataChanged( DataUpdateType_t updateType )
{
	if( updateType == DATA_UPDATE_CREATED )
	{
		m_pMaterial = materials->FindMaterial( m_szMaterialName, TEXTURE_GROUP_OTHER );
	}
}

//------------------------------------------------------------------------------
// Purpose: We don't draw.
//------------------------------------------------------------------------------
bool C_MaterialModifyControl::ShouldDraw()
{
	return false;
}

//=============================================================================
//
//
//
class CMaterialModifyProxy : public CEntityMaterialProxy
{
public:
	CMaterialModifyProxy();
	virtual ~CMaterialModifyProxy();
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( C_BaseEntity *pEntity );

private:
	IMaterial *m_pMaterial;
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CMaterialModifyProxy::CMaterialModifyProxy()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CMaterialModifyProxy::~CMaterialModifyProxy()
{
}

bool CMaterialModifyProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	m_pMaterial = pMaterial;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMaterialModifyProxy::OnBind( C_BaseEntity *pEntity )
{
	for ( C_BaseEntity *pChild = pEntity->FirstMoveChild(); pChild; pChild = pChild->NextMovePeer() )
	{
		C_MaterialModifyControl *pControl = dynamic_cast<C_MaterialModifyControl*>( pChild );
		if ( !pControl )
			continue;

		IMaterial *pMaterial = pControl->GetMaterial();
		if( !pMaterial )
			continue;

		if ( pMaterial != m_pMaterial )
			continue;

		bool bFound;
		IMaterialVar *pMaterialVar = pMaterial->FindVar( pControl->GetMaterialVariableName(), &bFound, false );
		if ( !bFound )
			continue;

		if( Q_strcmp( pControl->GetMaterialVariableValue(), "" ) )
		{
			pMaterialVar->SetStringValue( pControl->GetMaterialVariableValue() );
		}
	}
}

EXPOSE_INTERFACE( CMaterialModifyProxy, IMaterialProxy, "MaterialModify" IMATERIAL_PROXY_INTERFACE_VERSION );
