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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Breakable Model TE
//-----------------------------------------------------------------------------
class C_TEBreakModel : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEBreakModel, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TEBreakModel( void );
	virtual			~C_TEBreakModel( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	Vector			m_vecOrigin;
	QAngle			m_angRotation;
	Vector			m_vecSize;
	Vector			m_vecVelocity;
	int				m_nRandomization;
	int				m_nModelIndex;
	int				m_nCount;
	float			m_fTime;
	int				m_nFlags;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEBreakModel::C_TEBreakModel( void )
{
	m_vecOrigin.Init();
	m_angRotation.Init();
	m_vecSize.Init();
	m_vecVelocity.Init();
	m_nModelIndex		= 0;
	m_nRandomization	= 0;
	m_nCount			= 0;
	m_fTime				= 0.0;
	m_nFlags			= 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEBreakModel::~C_TEBreakModel( void )
{
}

void TE_BreakModel( IRecipientFilter& filter, float delay,
	const Vector& pos, const QAngle &angles, const Vector& size, const Vector& vel, 
	int modelindex, int randomization, int count, float time, int flags )
{
	tempents->BreakModel( pos, angles, size, vel, randomization, time, count, modelindex, flags );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TEBreakModel::PostDataUpdate( DataUpdateType_t updateType )
{
	tempents->BreakModel( m_vecOrigin, m_angRotation, m_vecSize, m_vecVelocity,
		m_nRandomization, m_fTime, m_nCount, m_nModelIndex, m_nFlags );
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TEBreakModel, DT_TEBreakModel, CTEBreakModel)
	RecvPropVector( RECVINFO(m_vecOrigin)),
	RecvPropFloat( RECVINFO( m_angRotation[0] ) ),
	RecvPropFloat( RECVINFO( m_angRotation[1] ) ),
	RecvPropFloat( RECVINFO( m_angRotation[2] ) ),
	RecvPropVector( RECVINFO(m_vecSize)),
	RecvPropVector( RECVINFO(m_vecVelocity)),
	RecvPropInt( RECVINFO(m_nModelIndex)),
	RecvPropInt( RECVINFO(m_nRandomization)),
	RecvPropInt( RECVINFO(m_nCount)),
	RecvPropFloat( RECVINFO(m_fTime)),
	RecvPropInt( RECVINFO(m_nFlags)),
END_RECV_TABLE()

