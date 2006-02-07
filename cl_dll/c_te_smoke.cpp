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
#include "IEffects.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Smoke TE
//-----------------------------------------------------------------------------
class C_TESmoke : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TESmoke, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TESmoke( void );
	virtual			~C_TESmoke( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	Vector			m_vecOrigin;
	int				m_nModelIndex;
	float			m_fScale;
	int				m_nFrameRate;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TESmoke::C_TESmoke( void )
{
	m_vecOrigin.Init();
	m_nModelIndex = 0;
	m_fScale = 0;
	m_nFrameRate = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TESmoke::~C_TESmoke( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TESmoke::PostDataUpdate( DataUpdateType_t updateType )
{
	// The number passed down is 10 times smaller...
	g_pEffects->Smoke( m_vecOrigin, m_nModelIndex, m_fScale * 10.0f, m_nFrameRate );	
}

void TE_Smoke( IRecipientFilter& filter, float delay,
	const Vector* pos, int modelindex, float scale, int framerate )
{
	// The number passed down is 10 times smaller...
	g_pEffects->Smoke( *pos, modelindex, scale * 10.0f, framerate );
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TESmoke, DT_TESmoke, CTESmoke)
	RecvPropVector( RECVINFO(m_vecOrigin)),
	RecvPropInt( RECVINFO(m_nModelIndex)),
	RecvPropFloat( RECVINFO(m_fScale )),
	RecvPropInt( RECVINFO(m_nFrameRate)),
END_RECV_TABLE()
