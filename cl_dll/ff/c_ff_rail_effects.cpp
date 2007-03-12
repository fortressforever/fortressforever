/// =============== Fortress Forever ==============
/// ======== A modifcation for Half-Life 2 ========
///
/// @file ff_rail_effects.cpp
/// @author Jon "trepid_jon" Day
/// @date February 17, 2007
/// @brief The rail effects class
///
/// REVISIONS
/// ---------
///
/// 

#include "cbase.h"
#include "c_ff_rail_effects.h"
#include "ClientEffectPrecacheSystem.h"
#include "beamdraw.h"
#include "model_types.h"

#define RAIL_MODEL "models/projectiles/rail/w_rail.mdl"
#define RAIL_BEAM "effects/rail_beam.vmt"
#define RAIL_GLOW "effects/rail_glow.vmt"

IMPLEMENT_NETWORKCLASS_ALIASED( FFRailEffects, DT_FFRailEffects )

BEGIN_NETWORK_TABLE(CFFRailEffects, DT_FFRailEffects) 
END_NETWORK_TABLE() 

LINK_ENTITY_TO_CLASS( ff_rail_effects, CFFRailEffects );
PRECACHE_REGISTER( ff_rail_effects );

ConVar ffdev_rail_beamlength( "ffdev_rail_beamlength", "512", FCVAR_REPLICATED, "Length of rail beam." );
//ConVar ffdev_rail_dietime( "ffdev_rail_dietime", "1.0", FCVAR_REPLICATED, "Length of rail beam." );
#define RAIL_EFFECTS_DIETIME 1.0f

ConVar ffdev_rail_color_default( "ffdev_rail_color_default", "0.2 0.4 0.6", 0, "Color of default rail (R G B - fraction from 0 to 1)" );
ConVar ffdev_rail_color_bounce1( "ffdev_rail_color_bounce1", "0.2 0.4 0.8", 0, "Color of rail after bouncing once (R G B - fraction from 0 to 1)" );
ConVar ffdev_rail_color_bounce2( "ffdev_rail_color_bounce2", "0.2 0.4 1.0", 0, "Color of rail after bouncing twice (R G B - fraction from 0 to 1)" );

CFFRailEffects::CFFRailEffects()
{
	m_Keyframes.Purge();

	m_bTimeToDie = false;
	m_flDieTimer = 0.0f;
}

void CFFRailEffects::Release()
{
	m_Keyframes.Purge();
	BaseClass::Release();
}

void CFFRailEffects::Precache( void )
{
	PrecacheModel( RAIL_MODEL );
	PrecacheMaterial( RAIL_BEAM );
	PrecacheMaterial( RAIL_GLOW );

	BaseClass::Precache();
}

void CFFRailEffects::Spawn( void )
{
	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_NONE);
	AddSolidFlags(FSOLID_NOT_SOLID);

	ClientEntityList().AddNonNetworkableEntity( this );
	AddToLeafSystem(GetRenderGroup());
}

void CFFRailEffects::GetRenderBounds( Vector& mins, Vector& maxs )
{
	ClearBounds( mins, maxs );

	int iCount = m_Keyframes.Count();
	if (iCount < 1)
		AddPointToBounds( GetAbsOrigin(), mins, maxs );
	else
		for (int i = 0; i < iCount; i++)
			AddPointToBounds( m_Keyframes[i].pos, mins, maxs );

	mins -= GetRenderOrigin();
	maxs -= GetRenderOrigin();
}

int CFFRailEffects::DrawModel( int flags )
{
	float flDieTime = RAIL_EFFECTS_DIETIME; // ffdev_rail_dietime.GetFloat();

	// it's clamped at flDieTime down below when it's updated
	if (m_flDieTimer >= flDieTime)
	{
		Remove();
		return 0;
	}

	float flMaxBeamLength = ffdev_rail_beamlength.GetFloat();

	if (m_bTimeToDie)
	{
		flMaxBeamLength *= clamp(((flDieTime / 10) - m_flDieTimer) / (flDieTime / 10), 0, 1);
		m_flDieTimer = clamp(m_flDieTimer + gpGlobals->frametime, 0.0f, flDieTime);
	}

	float flRemainingBeamLength = flMaxBeamLength;
	for (int i = 1; i < m_Keyframes.Count() && m_flDieTimer < flDieTime; i++)
	{
		//Warning("m_Keyframes[%d] = (%f, %f, %f)\n", i, m_Keyframes[i].x, m_Keyframes[i].y, m_Keyframes[i].z);

		// FF TODO: this will be faster when we get rid of the cvars
		float flColor[3] = { 0, 1, 0 };
		switch (m_Keyframes[i].type)
		{
		case RAIL_KEYFRAME_TYPE_BOUNCE1:
			{
				const char *szColor = ffdev_rail_color_bounce1.GetString();
				if( szColor )
					sscanf( szColor, "%f%f%f", flColor, flColor+1, flColor+2 );
			}
			break;
		case RAIL_KEYFRAME_TYPE_BOUNCE2:
			{
				const char *szColor = ffdev_rail_color_bounce2.GetString();
				if( szColor )
					sscanf( szColor, "%f%f%f", flColor, flColor+1, flColor+2 );
			}
			break;
		case RAIL_KEYFRAME_TYPE_END:
		case RAIL_KEYFRAME_TYPE_START:
		case RAIL_KEYFRAME_TYPE_DEFAULT:
		default:
			{
				const char *szColor = ffdev_rail_color_default.GetString();
				if( szColor )
					sscanf( szColor, "%f%f%f", flColor, flColor+1, flColor+2 );
			}
			break;
		}
		Vector vecColor = Vector(flColor[0], flColor[1], flColor[2]);

		Vector vecBeamStart = m_Keyframes[i-1].pos;
		Vector vecBeamEnd = m_Keyframes[i].pos;

		float flRandom = random->RandomFloat(1.0f, 2.0f);

		if (flRemainingBeamLength > 0)
		{
			Vector vecBeamDelta = vecBeamEnd - vecBeamStart;
			float flBeamLength = abs(clamp(vecBeamDelta.Length(), 0, flRemainingBeamLength));
			VectorNormalizeFast(vecBeamDelta);
			vecBeamDelta *= flBeamLength;
			vecBeamEnd = vecBeamStart + vecBeamDelta;

			flRemainingBeamLength -= flBeamLength;

			Vector vecBeamControl = (vecBeamStart + vecBeamEnd) * 0.5f + Vector(random->RandomFloat(-flRandom, flRandom), random->RandomFloat(-flRandom, flRandom), random->RandomFloat(-flRandom, flRandom));
			float flScrollOffset = gpGlobals->curtime - (int) gpGlobals->curtime;

			IMaterial *pMat = materials->FindMaterial(RAIL_BEAM, TEXTURE_GROUP_CLIENT_EFFECTS);
			materials->Bind(pMat);

			DrawBeamQuadratic(vecBeamStart, vecBeamControl, vecBeamEnd, 1.0f * flRandom, vecColor, flScrollOffset);
		}

		IMaterial *pMat = materials->FindMaterial(RAIL_GLOW, TEXTURE_GROUP_CLIENT_EFFECTS);
		materials->Bind(pMat);

		// fade the glow
		if (m_bTimeToDie)
			for (int j = 0; j < 3; j++)
				if (flColor[j] > 0)
					flColor[j] *= (flDieTime - m_flDieTimer) / flDieTime;

		DrawHalo(pMat, vecBeamStart, 2.0f * flRandom, flColor );
	}

	return BaseClass::DrawModel(flags);
}
