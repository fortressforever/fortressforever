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
#include "tempent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Glow Sprite TE
//-----------------------------------------------------------------------------
class C_TEGlowSprite : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEGlowSprite, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TEGlowSprite( void );
	virtual			~C_TEGlowSprite( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	Vector			m_vecOrigin;
	int				m_nModelIndex;
	float			m_fScale;
	float			m_fLife;
	int				m_nBrightness;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEGlowSprite::C_TEGlowSprite( void )
{
	m_vecOrigin.Init();
	m_nModelIndex = 0;
	m_fScale = 0;
	m_fLife = 0;
	m_nBrightness = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEGlowSprite::~C_TEGlowSprite( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TEGlowSprite::PostDataUpdate( DataUpdateType_t updateType )
{
	float a = ( 1.0 / 255.0 ) * m_nBrightness;
	C_LocalTempEntity *ent = tempents->TempSprite( m_vecOrigin, vec3_origin, m_fScale, m_nModelIndex, kRenderTransAdd, 0, a, m_fLife, FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP );
	if ( ent )
	{
		ent->bounceFactor = 0.2;
	}
}

void TE_GlowSprite( IRecipientFilter& filter, float delay,
	const Vector* pos, int modelindex, float life, float size, int brightness )
{
	float a = ( 1.0 / 255.0 ) * brightness;
	C_LocalTempEntity *ent = tempents->TempSprite( *pos, vec3_origin, size, modelindex, kRenderTransAdd, 0, a, life, FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP );
	if ( ent )
	{
		ent->bounceFactor = 0.2;
	}
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TEGlowSprite, DT_TEGlowSprite, CTEGlowSprite)
	RecvPropVector( RECVINFO(m_vecOrigin)),
	RecvPropInt( RECVINFO(m_nModelIndex)),
	RecvPropFloat( RECVINFO(m_fScale )),
	RecvPropFloat( RECVINFO(m_fLife )),
	RecvPropInt( RECVINFO(m_nBrightness)),
END_RECV_TABLE()
