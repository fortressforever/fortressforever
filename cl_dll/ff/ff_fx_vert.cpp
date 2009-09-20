/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file d:\ffsrc\cl_dll\ff\ff_fx_empzap.cpp
/// @author Alan Smithee (ted_maul)
/// @date 2006/05/20
/// @brief EMP lightning effect
///
/// lightning zaps for the EMP explosion effect

#include "cbase.h"
#include "ff_grenade_base.h"
#include "clienteffectprecachesystem.h"
#include "particles_simple.h"

#include "materialsystem/imaterialvar.h"
#include "c_te_effect_dispatch.h"
#include "beam_flags.h"
#include "beam_shared.h"
#include "iviewrender_beams.h"
#include "beamdraw.h"
#include "mathlib.h"

extern int g_iVertRingTexture;

ConVar ffdev_vert_effect_num_rings("ffdev_vert_effect_num_rings", "20", FCVAR_CHEAT);
ConVar ffdev_vert_effect_framerate("ffdev_vert_effect_framerate", "1", FCVAR_CHEAT);
ConVar ffdev_vert_effect_width("ffdev_vert_effect_width", "16", FCVAR_CHEAT);
ConVar ffdev_vert_effect_spread("ffdev_vert_effect_spread", "2550", FCVAR_CHEAT);
ConVar ffdev_vert_effect_amplitude("ffdev_vert_effect_amplitude", "0", FCVAR_CHEAT);
ConVar ffdev_vert_effect_max_life("ffdev_vert_effect_max_life", ".6", FCVAR_CHEAT);
ConVar ffdev_vert_effect_min_life("ffdev_vert_effect_min_life", ".4", FCVAR_CHEAT);
ConVar ffdev_vert_effect_r("ffdev_vert_effect_r", "128", FCVAR_CHEAT);
ConVar ffdev_vert_effect_g("ffdev_vert_effect_g", "255", FCVAR_CHEAT);
ConVar ffdev_vert_effect_b("ffdev_vert_effect_b", "225", FCVAR_CHEAT);
ConVar ffdev_vert_effect_a("ffdev_vert_effect_a", "178", FCVAR_CHEAT);

#define VERT_NUM_RINGS		ffdev_vert_effect_num_rings.GetFloat()
#define VERT_FRAMERATE		ffdev_vert_effect_framerate.GetFloat()
#define VERT_WIDTH			ffdev_vert_effect_width.GetFloat()
#define VERT_SPREAD			ffdev_vert_effect_spread.GetFloat()
#define VERT_AMPLITUDE		ffdev_vert_effect_amplitude.GetFloat()
#define VERT_MAX_LIFE		ffdev_vert_effect_max_life.GetFloat()
#define VERT_MIN_LIFE		ffdev_vert_effect_min_life.GetFloat()
#define VERT_R				ffdev_vert_effect_r.GetFloat()
#define VERT_G				ffdev_vert_effect_g.GetFloat()
#define VERT_B				ffdev_vert_effect_b.GetFloat()
#define VERT_A				ffdev_vert_effect_a.GetFloat()

void FF_FX_Vert_Callback(const CEffectData &data)
{
	// this ring effect is bollocks compared to HL1's TE_BEAMCYLINDER (which doesn't seemed to be accessible in source)
	// I have the electricity effect backed up somewhere and may try that again in future
	for (int i=0; i<VERT_NUM_RINGS; i++)
	{
		CBroadcastRecipientFilter filter;
		te->BeamRingPoint(filter,
			0,						// delay
			data.m_vOrigin + Vector( 0, 0, SimpleSplineRemapVal(i, 0.0f, VERT_NUM_RINGS, -data.m_flRadius/2, data.m_flRadius/2) ),			// origin
			1.0f,					// start radius
			SimpleSplineRemapVal(i, 0.0f, VERT_NUM_RINGS, 10.0f, data.m_flRadius),			// end radius
			g_iVertRingTexture,		// texture index
			0,						// halo index
			0,						// start frame
			VERT_FRAMERATE,						// frame rate
			SimpleSplineRemapVal(i, 0.0f, VERT_NUM_RINGS, VERT_MIN_LIFE, VERT_MAX_LIFE),					// life
			VERT_WIDTH,						// width
			VERT_SPREAD,					// spread (10x end width)
			VERT_AMPLITUDE,					// amplitude
			VERT_R,					// r
			VERT_G,					// g
			VERT_B,					// b
			VERT_A,					// a
			0,						// speed
			FBEAM_FADEOUT | FBEAM_SINENOISE);	// flags
	}
}

DECLARE_CLIENT_EFFECT("FF_Vert", FF_FX_Vert_Callback);