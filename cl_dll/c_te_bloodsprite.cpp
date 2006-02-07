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
#include "c_te_legacytempents.h"
#include "fx.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern short		g_sModelIndexBloodDrop;	
extern short		g_sModelIndexBloodSpray;

//-----------------------------------------------------------------------------
// Purpose: Blood sprite
//-----------------------------------------------------------------------------
class C_TEBloodSprite : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEBloodSprite, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TEBloodSprite( void );
	virtual			~C_TEBloodSprite( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	Vector			m_vecOrigin;
	Vector			m_vecDirection;
	int				r, g, b, a;
	int				m_nDropModel;
	int				m_nSprayModel;
	int				m_nSize;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEBloodSprite::C_TEBloodSprite( void )
{
	m_vecOrigin.Init();
	m_vecDirection.Init();

	r = g = b = a = 0;
	m_nSize = 0;
	m_nSprayModel = 0;
	m_nDropModel = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEBloodSprite::~C_TEBloodSprite( void )
{
}

void TE_BloodSprite( IRecipientFilter& filter, float delay,
	const Vector* org, const Vector *dir, int r, int g, int b, int a, int size )
{
	Vector	offset = *org + ( (*dir) * 4.0f );

	tempents->BloodSprite( offset, r, g, b, a, g_sModelIndexBloodSpray, g_sModelIndexBloodDrop, size );	
	FX_Blood( offset, (Vector &)*dir, r, g, b, a );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TEBloodSprite::PostDataUpdate( DataUpdateType_t updateType )
{
	Vector	offset = m_vecOrigin + ( m_vecDirection * 4.0f );

	tempents->BloodSprite( offset, r, g, b, a, m_nSprayModel, m_nDropModel, m_nSize );	
	FX_Blood( offset, m_vecDirection, r, g, b, a );
}

// Expose it to the engine.
IMPLEMENT_CLIENTCLASS_EVENT( C_TEBloodSprite, DT_TEBloodSprite, CTEBloodSprite );

BEGIN_RECV_TABLE_NOBASE(C_TEBloodSprite, DT_TEBloodSprite)
	RecvPropVector( RECVINFO(m_vecOrigin)),
	RecvPropVector( RECVINFO(m_vecDirection)),
	RecvPropInt( RECVINFO(r)),
	RecvPropInt( RECVINFO(g)),
	RecvPropInt( RECVINFO(b)),
	RecvPropInt( RECVINFO(a)),
	RecvPropInt( RECVINFO(m_nSprayModel)),
	RecvPropInt( RECVINFO(m_nDropModel)),
	RecvPropInt( RECVINFO(m_nSize)),
END_RECV_TABLE()

