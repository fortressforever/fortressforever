//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_basetempentity.h"
#include "iefx.h"
#include "fx.h"
#include "decals.h"
#include "materialsystem/IMaterialSystem.h"
#include "filesystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/itexture.h"
#include "ClientEffectPrecacheSystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifndef _XBOX
CLIENTEFFECT_REGISTER_BEGIN( PrecachePlayerDecal )
CLIENTEFFECT_MATERIAL( "decals/playerlogo01" )
#if !defined(HL2_DLL) || defined(HL2MP)
CLIENTEFFECT_MATERIAL( "decals/playerlogo02" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo03" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo04" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo05" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo06" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo07" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo08" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo09" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo10" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo11" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo12" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo13" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo14" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo15" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo16" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo17" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo18" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo19" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo20" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo21" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo22" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo23" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo24" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo25" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo26" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo27" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo28" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo29" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo30" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo31" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo32" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo33" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo34" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo35" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo36" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo37" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo38" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo39" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo40" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo41" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo42" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo43" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo44" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo45" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo46" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo47" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo48" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo49" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo40" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo41" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo42" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo43" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo44" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo45" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo46" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo47" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo48" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo49" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo50" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo51" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo52" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo53" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo54" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo55" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo56" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo57" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo58" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo59" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo60" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo61" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo62" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo63" )
CLIENTEFFECT_MATERIAL( "decals/playerlogo64" )
#endif
CLIENTEFFECT_REGISTER_END()
#endif

//-----------------------------------------------------------------------------
// Purpose: Player Decal TE
//-----------------------------------------------------------------------------
class C_TEPlayerDecal : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEPlayerDecal, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TEPlayerDecal( void );
	virtual			~C_TEPlayerDecal( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

	virtual void	Precache( void );

public:
	int				m_nPlayer;
	Vector			m_vecOrigin;
	int				m_nEntity;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEPlayerDecal::C_TEPlayerDecal( void )
{
	m_nPlayer = 0;
	m_vecOrigin.Init();
	m_nEntity = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEPlayerDecal::~C_TEPlayerDecal( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TEPlayerDecal::Precache( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : filter - 
//			delay - 
//			pos - 
//			player - 
//			entity - 
//-----------------------------------------------------------------------------
void TE_PlayerDecal( IRecipientFilter& filter, float delay,
	const Vector* pos, int player, int entity  )
{
#ifndef _XBOX
	// No valid target?
	C_BaseEntity *ent = cl_entitylist->GetEnt( entity );
	if ( !ent )
		return;

	// Find player logo for shooter
	player_info_t info;
	engine->GetPlayerInfo( player, &info );

	// Doesn't have a logo
	 if ( !info.customFiles[0] )	
		return;

	IMaterial *logo = materials->FindMaterial( VarArgs("decals/playerlogo%2.2d", player), TEXTURE_GROUP_DECAL );
	if ( IsErrorMaterial( logo ) )
		return;

	char logohex[ 16 ];
	Q_binarytohex( (byte *)&info.customFiles[0], sizeof( info.customFiles[0] ), logohex, sizeof( logohex ) );

	// See if logo has been downloaded.
	char texname[ 512 ];
	Q_snprintf( texname, sizeof( texname ), "temp/%s", logohex );
	char fulltexname[ 512 ];
	Q_snprintf( fulltexname, sizeof( fulltexname ), "materials/temp/%s.vtf", logohex );

	if ( !filesystem->FileExists( fulltexname ) )
	{
		char custname[ 512 ];
		Q_snprintf( custname, sizeof( custname ), "downloads/%s.dat", logohex );
		// it may have been downloaded but not copied under materials folder
		if ( !filesystem->FileExists( custname ) )
			return; // not downloaded yet

		// copy from download folder to materials/temp folder
		// this is done since material system can access only materials/*.vtf files

		if ( !engine->CopyFile( custname, fulltexname) )
			return;
	}

	ITexture *texture = materials->FindTexture( texname, TEXTURE_GROUP_DECAL );
	if ( IsErrorTexture( texture ) ) 
	{
		return; // not found 
	}

	color32 rgbaColor = { 255, 255, 255, 255 };
	effects->PlayerDecalShoot( 
		logo, 
		(void *)player,
		entity, 
		ent->GetModel(), 
		ent->GetAbsOrigin(), 
		ent->GetAbsAngles(), 
		*pos, 
		0, 
		0,
		rgbaColor );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TEPlayerDecal::PostDataUpdate( DataUpdateType_t updateType )
{
#ifndef _XBOX
	// Decals disabled?
	if ( !r_decals.GetBool() )
		return;

	CLocalPlayerFilter filter;
	TE_PlayerDecal(  filter, 0.0f, &m_vecOrigin, m_nPlayer, m_nEntity );
#endif
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TEPlayerDecal, DT_TEPlayerDecal, CTEPlayerDecal)
	RecvPropVector( RECVINFO(m_vecOrigin)),
	RecvPropInt( RECVINFO(m_nEntity)),
	RecvPropInt( RECVINFO(m_nPlayer)),
END_RECV_TABLE()
