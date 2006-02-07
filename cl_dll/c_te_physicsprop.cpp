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
class C_TEPhysicsProp : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEPhysicsProp, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TEPhysicsProp( void );
	virtual			~C_TEPhysicsProp( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	Vector			m_vecOrigin;
	QAngle			m_angRotation;
	Vector			m_vecVelocity;
	int				m_nModelIndex;
	int				m_nFlags;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEPhysicsProp::C_TEPhysicsProp( void )
{
	m_vecOrigin.Init();
	m_angRotation.Init();
	m_vecVelocity.Init();
	m_nModelIndex		= 0;
	m_nFlags			= 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEPhysicsProp::~C_TEPhysicsProp( void )
{
}

void TE_PhysicsProp( IRecipientFilter& filter, float delay,
	int modelindex, const Vector& pos, const QAngle &angles, const Vector& vel, bool breakmodel )
{
	tempents->PhysicsProp( modelindex, pos, angles, vel, breakmodel );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TEPhysicsProp::PostDataUpdate( DataUpdateType_t updateType )
{
	tempents->PhysicsProp( m_nModelIndex, m_vecOrigin, m_angRotation, m_vecVelocity, m_nFlags );
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TEPhysicsProp, DT_TEPhysicsProp, CTEPhysicsProp)
	RecvPropVector( RECVINFO(m_vecOrigin)),
	RecvPropFloat( RECVINFO( m_angRotation[0] ) ),
	RecvPropFloat( RECVINFO( m_angRotation[1] ) ),
	RecvPropFloat( RECVINFO( m_angRotation[2] ) ),
	RecvPropVector( RECVINFO(m_vecVelocity)),
	RecvPropInt( RECVINFO(m_nModelIndex)),
	RecvPropInt( RECVINFO(m_nFlags)),
END_RECV_TABLE()

