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
// Purpose: Armor Ricochet TE
//-----------------------------------------------------------------------------
class C_TEMetalSparks : public C_BaseTempEntity
{
public:
	DECLARE_CLIENTCLASS();

					C_TEMetalSparks( void );
	virtual			~C_TEMetalSparks( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

	virtual void	Precache( void );

public:
	Vector			m_vecPos;
	Vector			m_vecDir;

	const struct model_t *m_pModel;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEMetalSparks::C_TEMetalSparks( void )
{
	m_vecPos.Init();
	m_vecDir.Init();
	m_pModel = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEMetalSparks::~C_TEMetalSparks( void )
{
}

void C_TEMetalSparks::Precache( void )
{
	//m_pModel = engine->LoadModel( "sprites/richo1.vmt" );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TEMetalSparks::PostDataUpdate( DataUpdateType_t updateType )
{
	g_pEffects->MetalSparks( m_vecPos, m_vecDir );
}

void TE_MetalSparks( IRecipientFilter& filter, float delay,
	const Vector* pos, const Vector* dir )
{
	g_pEffects->MetalSparks( *pos, *dir );
}

//-----------------------------------------------------------------------------
// Purpose: Armor Ricochet TE
//-----------------------------------------------------------------------------
class C_TEArmorRicochet : public C_TEMetalSparks
{
	DECLARE_CLASS( C_TEArmorRicochet, C_TEMetalSparks );
public:
	DECLARE_CLIENTCLASS();
	virtual void	PostDataUpdate( DataUpdateType_t updateType );
};

//-----------------------------------------------------------------------------
// Purpose: Client side version of API
// Input  : msg_dest - 
//			delay - 
//			origin - 
//			*recipient - 
//			pos - 
//			dir - 
//-----------------------------------------------------------------------------
void TE_ArmorRicochet( IRecipientFilter& filter, float delay,
	const Vector* pos, const Vector* dir )
{
	g_pEffects->Ricochet( *pos, *dir );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TEArmorRicochet::PostDataUpdate( DataUpdateType_t updateType )
{
	g_pEffects->Ricochet( m_vecPos, m_vecDir );
}

// Expose the TE to the engine.
IMPLEMENT_CLIENTCLASS_EVENT( C_TEMetalSparks, DT_TEMetalSparks, CTEMetalSparks );

BEGIN_RECV_TABLE_NOBASE(C_TEMetalSparks, DT_TEMetalSparks)
	RecvPropVector(RECVINFO(m_vecPos)),
	RecvPropVector(RECVINFO(m_vecDir)),
END_RECV_TABLE()

IMPLEMENT_CLIENTCLASS_EVENT( C_TEArmorRicochet, DT_TEArmorRicochet, CTEArmorRicochet );
BEGIN_RECV_TABLE(C_TEArmorRicochet, DT_TEArmorRicochet)
END_RECV_TABLE()
