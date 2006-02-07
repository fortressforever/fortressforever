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
#include "tempent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Sprite TE
//-----------------------------------------------------------------------------
class C_TESprite : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TESprite, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TESprite( void );
	virtual			~C_TESprite( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	Vector			m_vecOrigin;
	int				m_nModelIndex;
	float			m_fScale;
	int				m_nBrightness;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TESprite::C_TESprite( void )
{
	m_vecOrigin.Init();
	m_nModelIndex = 0;
	m_fScale = 0;
	m_nBrightness = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TESprite::~C_TESprite( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TESprite::PostDataUpdate( DataUpdateType_t updateType )
{
	float a = ( 1.0 / 255.0 ) * m_nBrightness;
	tempents->TempSprite( m_vecOrigin, vec3_origin, m_fScale, m_nModelIndex, kRenderTransAdd, 0, a, 0, FTENT_SPRANIMATE );
}

void TE_Sprite( IRecipientFilter& filter, float delay,
	const Vector* pos, int modelindex, float size, int brightness )
{
	float a = ( 1.0 / 255.0 ) * brightness;
	tempents->TempSprite( *pos, vec3_origin, size, modelindex, kRenderTransAdd, 0, a, 0, FTENT_SPRANIMATE );

}


IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TESprite, DT_TESprite, CTESprite)
	RecvPropVector( RECVINFO(m_vecOrigin)),
	RecvPropInt( RECVINFO(m_nModelIndex)),
	RecvPropFloat( RECVINFO(m_fScale )),
	RecvPropInt( RECVINFO(m_nBrightness)),
END_RECV_TABLE()
