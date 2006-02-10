// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildableobjects_shared.cpp
// @author Patrick O'Leary(Mulchman) 
// @date 06/08/2005
// @brief Shared code for buildable objects
//
// REVISIONS
// ---------
// 06/08/2005, Mulchman: 
//		This file First created
// 22/01/2006, Mirv:
//		Rewritten a lot of this, an instance of this object will now hold all the information
//		needed to put a buildable on the ground(including pre-working out the orientation 
//		and whether or not the orientation is okay, since that is now part of this class) 
//		 (Previously calculating the orientation was left until last and not part of the
//		buildable spot validation).
//		Also now the SG orients away from walls.

#include "cbase.h"
#include "ff_buildableobjects_shared.h"

#ifdef CLIENT_DLL
#else
	//#include "ff_sevtest.h"
#endif

CFFBuildableInfo::CFFBuildableInfo(CFFPlayer *pPlayer, int iBuildObject, float flBuildDist, float flRaiseVal) 
{
	m_BuildResult = BUILD_ERROR;

	m_iBuildObject = iBuildObject;
	m_flBuildDist = flBuildDist;
	m_flRaiseVal = flRaiseVal;
	m_flTestDist = m_flRaiseVal + 24.0f;	// How much further down do we test

	if (!pPlayer) 
	{
		m_BuildResult = BUILD_NOPLAYER;
		return;
	}

	m_pPlayer = pPlayer;

	// Get some info from the player...
	pPlayer->EyeVectors(&m_vecPlayerForward, &m_vecPlayerRight);

	// Level off
	m_vecPlayerForward.z = 0;
	m_vecPlayerRight.z = 0;

	// Normalize
	VectorNormalize(m_vecPlayerForward);
	VectorNormalize(m_vecPlayerRight);

	// Store the players' origin
	m_vecPlayerOrigin = pPlayer->GetAbsOrigin();

	// Get a position in front of the player a certain distance & raise it up
	m_vecBuildAirOrigin = m_vecPlayerOrigin + (m_vecPlayerForward * m_flBuildDist);
	m_vecBuildAirOrigin.z += m_flRaiseVal;

	// Original angle is same as player's
	VectorAngles(m_vecPlayerForward, m_angBuildAirAngles);

	// As default the ground values will be the same as the air ones
	m_vecBuildGroundOrigin = m_vecBuildAirOrigin;
	m_angBuildGroundAngles = m_angBuildAirAngles;

	// If we can't trace the hull
	if (IsGeometryInTheWay()) 
	{
		m_BuildResult = BUILD_NOROOM;
		return;
	}

	// If we're dealing w/ a detpack then we're finished here
	if (m_iBuildObject == FF_BUILD_DETPACK) 
	{
		m_BuildResult = BUILD_ALLOWED;
		return;
	}

	// Test to see if can be oriented
	m_BuildResult = CanOrientToGround();
}

bool CFFBuildableInfo::IsGeometryInTheWay() 
{
	// Get a position at the players origin but raised up a bit
	Vector vecPlayerOriginMod = m_vecPlayerOrigin;
	vecPlayerOriginMod.z += m_flRaiseVal;

	// Get the correct mins/maxs 
	Vector vecMins, vecMaxs;
	switch (m_iBuildObject) 
	{
		case FF_BUILD_DISPENSER: vecMins = FF_DISPENSER_MINS; vecMaxs = FF_DISPENSER_MAXS; break;
		case FF_BUILD_SENTRYGUN: vecMins = FF_SENTRYGUN_MINS; vecMaxs = FF_SENTRYGUN_MAXS; break;
		case FF_BUILD_DETPACK:   vecMins = FF_DETPACK_MINS;   vecMaxs = FF_DETPACK_MAXS;   break;
	}

#ifdef GAME_DLL
#ifdef _DEBUG
	NDebugOverlay::Cross3D(m_vecBuildAirOrigin, vecMins, vecMaxs, 60, 255, 60, false, 5);
#endif
#endif

	// Now, using the mins/maxs, do a trace
	// If the trace does not complete them we hit something and the area is not clear
	trace_t trHull;

	// |-- Mirv: Fixed tracehull: use COLLISION_GROUP_WEAPON so that we don't collide with flags and ammo
	UTIL_TraceHull(m_vecBuildAirOrigin, m_vecBuildAirOrigin, vecMins, vecMaxs, MASK_PLAYERSOLID, m_pPlayer, COLLISION_GROUP_WEAPON, &trHull);
	//UTIL_TraceHull(vecPlayerOriginMod, m_vecBuildAirOrigin, vecMins, vecMaxs, MASK_PLAYERSOLID, m_pPlayer, COLLISION_GROUP_NONE, &trHull);

	// See if we started in a solid
	if (trHull.startsolid) 
		return true;
	// See if the trace completed - if fraction == 1.0 then it completed
	else if (trHull.fraction != 1.0) 
		return true;

	return false;
}

void CFFBuildableInfo::GetBuildError() 
{
	// TODO: Hook into the hint system so hints are displayed
	// and not the devmsg/warning stuff

	if (m_BuildResult == BUILD_ALLOWED) 
	{
		DevMsg("[Buildable Object] Build was a success!\n");
	}
	else
	{
		char szError[512];
		Q_strcpy(szError, "[Buildable Object] ");

		switch (m_BuildResult) 
		{
			case BUILD_NOPLAYER: Q_strcat(szError, "Player pointer was NULL!"); break;
			case BUILD_NOROOM: Q_strcat(szError, "World geometry in the way!"); break;
			case BUILD_TOOSTEEP: Q_strcat(szError, "Ground is too steep!"); break;
			case BUILD_TOOFAR: Q_strcat(szError, "Ground is too far below us!"); break;

			default: Q_strcat(szError, "Generic error!"); break;
		}

		Warning("%s\n", szError);
	}
}

QAngle OrientToVectors(const Vector &vecGroundNormal, const Vector &vecPlayerForward) 
{
	// Get correct forward & right vector
	Vector vecUp = vecGroundNormal;
	Vector vecRight = CrossProduct(vecUp, -vecPlayerForward);
	Vector vecForward = CrossProduct(vecUp, vecRight);

	// Normalise(jic) 
	VectorNormalize(vecForward);
	VectorNormalize(vecRight);
	VectorNormalize(vecUp);

	// Quaternions are the handiest things
	Quaternion q;
	BasisToQuaternion(vecForward, vecRight, vecUp, q);

	// Now for the angles
	QAngle angNew;
	QuaternionAngles(q, angNew);

	return angNew;
}

// Vectors must be	0 1
//					2 3
void ComputeRectangularPlane(const Vector &v0, const Vector &v1, const Vector &v2, const Vector &v3, Vector &vOut) 
{
	Vector vecOne = v0 - v3;
	Vector vecTwo = v2 - v1;

	VectorNormalize(vecOne);
	VectorNormalize(vecTwo);

	vOut = CrossProduct(vecOne, vecTwo);

	VectorNormalize(vOut);
}

Vector GetMiddle(const Vector *v1, const Vector *v2, const Vector *v3 = NULL, const Vector *v4 = NULL) 
{
	float c = 2.0f;
	Vector vecMiddle = *v1 + *v2;

	if (v3) 
	{
		vecMiddle += *v3;
		c++;
	}
	if (v4) 
	{
		vecMiddle += *v4;
		c++;
	}

	vecMiddle /= c;

	return vecMiddle;
}

BuildInfoResult_t CFFBuildableInfo::CanOrientToGround() 
{
	Vector vecNormal, vecGround;

	switch (m_iBuildObject) 
	{
		case FF_BUILD_DISPENSER:
		{
			Vector vecRightAdjWidth = m_vecPlayerRight * FF_BUILD_DISP_HALF_WIDTH;
			Vector vecForwardAdjWidth = m_vecPlayerForward * FF_BUILD_DISP_HALF_WIDTH;

			// Set up each element to reflect a corner of the dispenser
			Vector vecCorners[4];
			vecCorners[0] = m_vecBuildAirOrigin + (vecRightAdjWidth) + (vecForwardAdjWidth); // Front right corner
			vecCorners[1] = m_vecBuildAirOrigin + (vecRightAdjWidth) - (vecForwardAdjWidth); // Back right corner
			vecCorners[2] = m_vecBuildAirOrigin - (vecRightAdjWidth) + (vecForwardAdjWidth); // Front left corner
			vecCorners[3] = m_vecBuildAirOrigin - (vecRightAdjWidth) - (vecForwardAdjWidth); // Back left corner

			// For traces to come
			trace_t tr[4];

			// Loop through and do traces from each corner of the dispenser to the ground
			// The mask is normal PLAYER_SOLID minus PLAYER_CLIP as we don't want to build on those(Bug #0000185: Buildable objects can be built on clips.) 
			// HACK Changed to COLLISION_GROUP_PROJECTILE so it doesn't clip healthpacks (Bug #0000242: SG/Disp when building clips on health pack.)
			for (int i = 0; i < 4; i++) 
			{
				UTIL_TraceLine(vecCorners[i], vecCorners[i] - Vector(0, 0, m_flTestDist), CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE, m_pPlayer, COLLISION_GROUP_PROJECTILE, &tr[i]);

				// Bug #0000246: Dispenser and sg overlap if built on each other
				if (!tr[i].DidHit() || tr[i].m_pEnt->Classify() == CLASS_SENTRYGUN || tr[i].m_pEnt->Classify() == CLASS_DISPENSER) 
					return BUILD_TOOFAR;
			}

			// Make an X shaped vector from our 4 corner position traces and do the cross product
			ComputeRectangularPlane(tr[0].endpos, tr[1].endpos, tr[2].endpos, tr[3].endpos, vecNormal);

			vecGround = GetMiddle(&tr[0].endpos, &tr[1].endpos, &tr[2].endpos, &tr[3].endpos);
		}
		break;

		case FF_BUILD_SENTRYGUN:
		{
			Vector vecRightAdjWidth = m_vecPlayerRight * 18.0f;
			Vector vecForwardAdjWidth = m_vecPlayerForward * 8.0f;

			// Set up these vectors to reflect the position above the ground at each
			// of the feet of the sg where we want to start traces from
			Vector vecFeet[3];
			vecFeet[0] = m_vecBuildAirOrigin - (m_vecPlayerForward * 20.0f);
			vecFeet[1] = m_vecBuildAirOrigin + (vecRightAdjWidth) + (vecForwardAdjWidth);
			vecFeet[2] = m_vecBuildAirOrigin - (vecRightAdjWidth) + (vecForwardAdjWidth);

			// For traces to come
			trace_t tr[3];

			// Loop through and do traces from each corner of the sentry to the ground
			// The mask is normal PLAYER_SOLID minus PLAYER_CLIP and CONTENTS_MOVEABLE as we don't want to build on those(Bug #0000185: Buildable objects can be built on clips.) 
			// HACK Changed to COLLISION_GROUP_PROJECTILE so it doesn't clip healthpacks (Bug #0000242: SG/Disp when building clips on health pack.)
			for (int i = 0; i < 3; i++) 
			{
				UTIL_TraceLine(vecFeet[i], vecFeet[i] - Vector(0, 0, m_flTestDist), CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE, m_pPlayer, COLLISION_GROUP_PROJECTILE, &tr[i]);

				// Bug #0000246: Dispenser and sg overlap if built on each other
				if (!tr[i].DidHit() || tr[i].m_pEnt->Classify() == CLASS_SENTRYGUN || tr[i].m_pEnt->Classify() == CLASS_DISPENSER) 
					return BUILD_TOOFAR;
			}

			float flIntercept;
			ComputeTrianglePlane(tr[0].endpos, tr[1].endpos, tr[2].endpos, vecNormal, flIntercept);

			vecGround = GetMiddle(&tr[0].endpos, &tr[1].endpos, &tr[2].endpos);

			// Trace out in front of us [added an extra 32 units up so its less eager on slopes) 
			trace_t tr_fwd;
			UTIL_TraceLine(m_vecBuildAirOrigin + Vector(0, 0, 32.0f), m_vecBuildAirOrigin + Vector(0, 0, 32.0f) + m_vecPlayerForward * 128.0f, CONTENTS_SOLID, NULL, COLLISION_GROUP_NONE, &tr_fwd);

			// If we hit a wall then we want to face towards us instead
			if (tr_fwd.DidHit()) 
			{
				m_vecPlayerForward *= -1;
				VectorAngles(m_vecPlayerForward, m_angBuildAirAngles);
			}
		}
		break;

		case FF_BUILD_DETPACK:
			m_vecBuildGroundOrigin = m_vecBuildAirOrigin;
			m_angBuildGroundAngles = m_angBuildAirAngles;
			return BUILD_ALLOWED;

	default:
		AssertMsg(0, "Invalid buildable!");
		return BUILD_ERROR;
	}

	float flEpsilon = vecNormal.z / VectorLength(vecNormal);

	if (flEpsilon < 0.95f) 
		return BUILD_TOOSTEEP;

	m_angBuildGroundAngles = OrientToVectors(vecNormal, m_vecPlayerForward);
	m_vecBuildGroundOrigin = vecGround;

	return BUILD_ALLOWED;
}
