//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Game-specific impact effect hooks
//
//=============================================================================//
#include "cbase.h"
#include "fx_impact.h"
#include "tempent.h"
#include "c_te_effect_dispatch.h"
#include "c_te_legacytempents.h"


//-----------------------------------------------------------------------------
// Purpose: Handle weapon effect callbacks
//-----------------------------------------------------------------------------
void FF_EjectBrass( int shell, const CEffectData &data )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if( !pPlayer )
		return;

	tempents->CSEjectBrass( data.m_vOrigin, data.m_vAngles, data.m_fFlags, shell, pPlayer );
}

void FF_FX_EjectBrass_9mm_Callback( const CEffectData &data )
{
	FF_EjectBrass( CS_SHELL_9MM, data );
}

void FF_FX_EjectBrass_12Gauge_Callback( const CEffectData &data )
{
	FF_EjectBrass( CS_SHELL_12GAUGE, data );
}

void FF_FX_EjectBrass_40mm_Callback( const CEffectData &data )
{
	FF_EjectBrass( FF_SHELL_40MM, data );
}

//extern ConVar ffdev_nail_speed;
#define NAIL_SPEED 2000.0f

void FF_FX_Projectile_Nail_Callback(const CEffectData &data)
{
	tempents->FFProjectile(data.m_vOrigin, data.m_vAngles, NAIL_SPEED, FF_PROJECTILE_NAIL, 

#ifdef GAME_DLL
		data.m_nEntIndex
#else
		data.m_hEntity.GetEntryIndex()
#endif
		);
}

//static ConVar ffdev_nailradial_number("ffdev_nailradial_number", "8");
#define NAILRADIAL_NUMBER 8

void FF_FX_Projectile_Nail_Radial_Callback(const CEffectData &data)
{
	int nNails = NAILRADIAL_NUMBER;

	float flDeltaAngle = 360.0f / nNails;
	QAngle angRadial = QAngle(0.0f, random->RandomFloat(0.0f, flDeltaAngle), 0.0f);
	
	while (nNails-- > 0)
	{
		tempents->FFProjectile(data.m_vOrigin, angRadial, NAIL_SPEED, FF_PROJECTILE_NAIL_NG, 
#ifdef GAME_DLL
			data.m_nEntIndex
#else
			data.m_hEntity.GetEntryIndex()
#endif
			);
		angRadial.y += flDeltaAngle;
	}

}

DECLARE_CLIENT_EFFECT( "EjectBrass_9mm", FF_FX_EjectBrass_9mm_Callback );
DECLARE_CLIENT_EFFECT( "EjectBrass_12Gauge",FF_FX_EjectBrass_12Gauge_Callback );
DECLARE_CLIENT_EFFECT( "EjectBrass_40mm",FF_FX_EjectBrass_40mm_Callback );

DECLARE_CLIENT_EFFECT("Projectile_Nail", FF_FX_Projectile_Nail_Callback);
DECLARE_CLIENT_EFFECT("Projectile_Nail_Radial", FF_FX_Projectile_Nail_Radial_Callback);
