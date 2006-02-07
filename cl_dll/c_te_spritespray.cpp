//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_basetempentity.h"
#include "c_te_legacytempents.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Sprite Spray TE
//-----------------------------------------------------------------------------
class C_TESpriteSpray : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TESpriteSpray, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TESpriteSpray( void );
	virtual			~C_TESpriteSpray( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	Vector			m_vecOrigin;
	Vector			m_vecDirection;
	int				m_nModelIndex;
	int				m_nSpeed;
	float			m_fNoise;
	int				m_nCount;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TESpriteSpray::C_TESpriteSpray( void )
{
	m_vecOrigin.Init();
	m_vecDirection.Init();
	m_nModelIndex = 0;
	m_fNoise = 0;
	m_nSpeed = 0;
	m_nCount = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TESpriteSpray::~C_TESpriteSpray( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TESpriteSpray::PostDataUpdate( DataUpdateType_t updateType )
{
	tempents->Sprite_Spray( m_vecOrigin, m_vecDirection, m_nModelIndex, m_nCount, m_nSpeed * 0.2, m_fNoise * 100.0 );
}

void TE_SpriteSpray( IRecipientFilter& filter, float delay,
	const Vector* pos, const Vector* dir, int modelindex, int speed, float noise, int count )
{
	tempents->Sprite_Spray( *pos, *dir, modelindex, count, speed * 0.2, noise * 100.0 );
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TESpriteSpray, DT_TESpriteSpray, CTESpriteSpray)
	RecvPropVector( RECVINFO(m_vecOrigin)),
	RecvPropVector( RECVINFO(m_vecDirection)),
	RecvPropInt( RECVINFO(m_nModelIndex)),
	RecvPropFloat( RECVINFO(m_fNoise )),
	RecvPropInt( RECVINFO(m_nCount)),
	RecvPropInt( RECVINFO(m_nSpeed)),
END_RECV_TABLE()
