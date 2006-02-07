//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Muzzle flash temp ent
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_basetempentity.h"
#include "IEffects.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: User Tracer TE
//-----------------------------------------------------------------------------
class C_TEMuzzleFlash : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEMuzzleFlash, C_BaseTempEntity );
	
	DECLARE_CLIENTCLASS();

					C_TEMuzzleFlash( void );
	virtual			~C_TEMuzzleFlash( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	Vector		m_vecOrigin;
	QAngle		m_vecAngles;
	float		m_flScale;
	int			m_nType;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEMuzzleFlash::C_TEMuzzleFlash( void )
{
	m_vecOrigin.Init();
	m_vecAngles.Init();
	m_flScale	= 1.0f;
	m_nType		= 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEMuzzleFlash::~C_TEMuzzleFlash( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TEMuzzleFlash::PostDataUpdate( DataUpdateType_t updateType )
{
	//FIXME: Index is incorrect
	g_pEffects->MuzzleFlash( m_vecOrigin, m_vecAngles, m_flScale, m_nType );	
}

void TE_MuzzleFlash( IRecipientFilter& filter, float delay,
	const Vector &start, const QAngle &angles, float scale, int type )
{
	g_pEffects->MuzzleFlash( start, angles, scale, 0 );	
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TEMuzzleFlash, DT_TEMuzzleFlash, CTEMuzzleFlash)
	RecvPropVector( RECVINFO(m_vecOrigin)),
	RecvPropVector( RECVINFO(m_vecAngles)),
	RecvPropFloat( RECVINFO(m_flScale)),
	RecvPropInt( RECVINFO(m_nType)),
END_RECV_TABLE()
