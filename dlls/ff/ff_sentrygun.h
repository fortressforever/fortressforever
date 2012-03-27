// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_sentrygun.h
// @author Patrick O'Leary (Mulchman)
// @date 12/28/2005
// @brief SentryGun class
//
// REVISIONS
// ---------
//	12/28/2005, Mulchman: 
//		First created
//
//	05/09/2005, Mulchman:
//		Tons of little updates here and there
//
//	05/17/2005, Mulchman:
//		Starting to make it animated and such
//
//	06/01/2005, Mulchman:
//		Noticed I had dates wrong... *cough* and
//		still working on making the SG animated
//		and such.
//
//	06/08/2005, Mulchman:
//		Decided the SG needs to inherit from the
//		AI base class and not the buildable class.
//		Some easy stuff will give it the same base
//		buildable attributes while inheriting all
//		of the AI stuff that it so desperately needs!
//
//	05/10/2006, Mulchman:
//		Cleaned this up A LOT. SG still doesn't factor in radiotagged targets, though.

#ifndef FF_SENTRYGUN_H
#define FF_SENTRYGUN_H

#ifdef _WIN32
#pragma once
#endif

#include "ff_buildableobjects_shared.h"

#define SG_BC_YAW			"aim_yaw"
#define SG_BC_PITCH			"aim_pitch"
#define SG_BC_BARREL_ROTATE	"barrel_rotate"
//#define	SG_RANGE			1200
#define	SG_MAX_WAIT			5
#define SG_SHORT_WAIT		2.0		// Used for FAST_RETIRE spawnflag
#define	SG_PING_TIME		10.0f	// LPB!!

#define SG_MAX_PITCH		90.0f
#define SG_MIN_PITCH		-85.0f//-90.0f
#define SG_MIN_ANIMATED_PITCH		-30.0f//-33.0f (caes: changed to -30 so animation doesn't go up again at end)
#define SG_SCAN_HALFWIDTH	30.0f//40.0f

//extern ConVar sg_health_lvl1;
//#define SG_HEALTH_LEVEL1	sg_health_lvl1.GetInt()
//extern ConVar sg_health_lvl2;
//#define SG_HEALTH_LEVEL2	sg_health_lvl2.GetInt()
//extern ConVar sg_health_lvl3;
//#define SG_HEALTH_LEVEL3	sg_health_lvl3.GetInt()


#endif // FF_SENTRYGUN_H
