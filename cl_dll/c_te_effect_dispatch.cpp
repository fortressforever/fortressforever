//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_basetempentity.h"
#include "networkstringtable_clientdll.h"
#include "effect_dispatch_data.h"
#include "c_te_effect_dispatch.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// CClientEffectRegistration registration
//-----------------------------------------------------------------------------

CClientEffectRegistration *CClientEffectRegistration::s_pHead = NULL;

CClientEffectRegistration::CClientEffectRegistration( const char *pEffectName, ClientEffectCallback fn )
{
	m_pEffectName = pEffectName;
	m_pFunction = fn;
	m_pNext = s_pHead;
	s_pHead = this;
}


//-----------------------------------------------------------------------------
// Purpose: EffectDispatch TE
//-----------------------------------------------------------------------------
class C_TEEffectDispatch : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEEffectDispatch, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TEEffectDispatch( void );
	virtual			~C_TEEffectDispatch( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	CEffectData m_EffectData;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEEffectDispatch::C_TEEffectDispatch( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEEffectDispatch::~C_TEEffectDispatch( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void DispatchEffectToCallback( const char *pEffectName, const CEffectData &m_EffectData )
{
	// Look through all the registered callbacks
	for ( CClientEffectRegistration *pReg = CClientEffectRegistration::s_pHead; pReg; pReg = pReg->m_pNext )
	{
		// If the name matches, call it
		if ( Q_stricmp( pReg->m_pEffectName, pEffectName ) == 0 )
		{
			pReg->m_pFunction( m_EffectData );
			return;
		}
	}

	DevMsg("DispatchEffect: effect '%s' not found on client\n", pEffectName );

}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TEEffectDispatch::PostDataUpdate( DataUpdateType_t updateType )
{
	// Find the effect name.
	const char *pEffectName = g_StringTableEffectDispatch->GetString( m_EffectData.GetEffectNameIndex() );
	if ( pEffectName )
	{
		DispatchEffectToCallback( pEffectName, m_EffectData );
	}
}


IMPLEMENT_CLIENTCLASS_EVENT_DT( C_TEEffectDispatch, DT_TEEffectDispatch, CTEEffectDispatch )
	
	RecvPropDataTable( RECVINFO_DT( m_EffectData ), 0, &REFERENCE_RECV_TABLE( DT_EffectData ) )
			
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Clientside version
//-----------------------------------------------------------------------------
void TE_DispatchEffect( IRecipientFilter& filter, float delay, const Vector &pos, const char *pName, const CEffectData &data )
{
	DispatchEffectToCallback( pName, data );
}

// Client version of dispatch effect, for predicted weapons
void DispatchEffect( const char *pName, const CEffectData &data )
{
	CPASFilter filter( data.m_vOrigin );
	te->DispatchEffect( filter, 0.0, data.m_vOrigin, pName, data );
}
