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

//#define RAIL_MODEL "models/projectiles/rail/w_rail.mdl"
#define RAIL_BEAM "effects/rail_beam.vmt"
#define RAIL_GLOW "effects/rail_glow.vmt"

IMPLEMENT_NETWORKCLASS_ALIASED( FFRailEffects, DT_FFRailEffects )

BEGIN_NETWORK_TABLE(CFFRailEffects, DT_FFRailEffects) 
END_NETWORK_TABLE() 

LINK_ENTITY_TO_CLASS( ff_rail_effects, CFFRailEffects );
PRECACHE_REGISTER( ff_rail_effects );

ConVar ffdev_rail_beamlength( "ffdev_rail_beamlength", "512", FCVAR_REPLICATED, "Length of rail beam." );

ConVar ffdev_rail_color_default( "ffdev_rail_color_default", "0.250 0.500 0.750", 0, "Color of default rail (R G B - fraction from 0 to 1)" );
ConVar ffdev_rail_color_bounce1( "ffdev_rail_color_bounce1", "0.125 0.750 0.625", 0, "Color of rail after bouncing once (R G B - fraction from 0 to 1)" );
ConVar ffdev_rail_color_bounce2( "ffdev_rail_color_bounce2", "0.000 1.000 0.500", 0, "Color of rail after bouncing twice (R G B - fraction from 0 to 1)" );

CFFRailEffects::CFFRailEffects()
{
	m_Keyframes.Purge();

	m_flMaxBeamLength = 0.0f;

	m_bTimeToDie = false;
}

void CFFRailEffects::Release()
{
	m_Keyframes.Purge();
	BaseClass::Release();
}

void CFFRailEffects::Precache( void )
{
	//PrecacheModel( RAIL_MODEL );

	PrecacheMaterial( RAIL_BEAM );
	PrecacheMaterial( RAIL_GLOW );

	BaseClass::Precache();
}

void CFFRailEffects::Spawn( void )
{
	m_flMaxBeamLength = ffdev_rail_beamlength.GetFloat();

	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_NONE);

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
	float flRemainingBeamLength = m_flMaxBeamLength;
	for (int i = 1; i < m_Keyframes.Count(); i++)
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

		// FF TODO: draw the glows on their own (er...let them die independently)
		DrawHalo(pMat, vecBeamStart, 2.0f * flRandom, flColor );
	}

	if (m_bTimeToDie)
	{
		m_flMaxBeamLength = (m_flMaxBeamLength - flRemainingBeamLength) - (abs(GetAbsVelocity().Length()) * gpGlobals->frametime);

		if (m_flMaxBeamLength <= 0)
		{
			Remove();
			return 0;
		}
	}

	return BaseClass::DrawModel(flags);
}
