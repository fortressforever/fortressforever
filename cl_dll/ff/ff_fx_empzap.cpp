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

#include "ff_fx_empzap.h"

#include "materialsystem/imaterialvar.h"
#include "c_te_effect_dispatch.h"
#include "beam_flags.h"
#include "beam_shared.h"
#include "iviewrender_beams.h"
#include "beamdraw.h"
#include "mathlib.h"

/*
class CViewRenderBeams
{
public:
	Beam_t *CreateGenericBeam( BeamInfo_t &beamInfo );
	void CViewRenderBeams::SetBeamAttributes( Beam_t *pBeam, const BeamInfo_t &beamInfo );
};
*/

void FF_FX_EmpZap_Callback(const CEffectData &data)
{
	// this ring effect is bollocks compared to HL1's TE_BEAMCYLINDER (which doesn't seemed to be accessible in source)
	// I have the electricity effect backed up somewhere and may try that again in future
	CBroadcastRecipientFilter filter;
	te->BeamRingPoint(filter, 0, data.m_vOrigin, 1.0f, 240.0f, CFFGrenadeBase::m_iRingTexture, 0, 0, 1, 0.2f, 32, 0, 0, 255, 255, 128, 196, 0, FBEAM_FADEOUT | FBEAM_SINENOISE);
	// undone - didn't work
	/*
	BeamInfo_t beaminfo;
	beaminfo.m_nModelIndex = CFFGrenadeBase::m_iRingTexture;
	beaminfo.m_nHaloIndex = 0;
	beaminfo.m_flHaloScale = 0.0f;
	beaminfo.m_flLife = 0.2f;
	beaminfo.m_flWidth = 32.0f;
	beaminfo.m_flEndWidth = 32.0f;
	beaminfo.m_flFadeLength = 0.0f;
	beaminfo.m_flSpeed = 0.0f;
	beaminfo.m_flRed = 1.0f;
	beaminfo.m_flGreen = 1.0f;
	beaminfo.m_flBlue = 0.5f;
	beaminfo.m_flBrightness = 1.0f;
	beaminfo.m_flAmplitude = 1.0f;
	beaminfo.m_vecCenter = data.m_vOrigin;
	beaminfo.m_flStartRadius = 1.0f;
	beaminfo.m_flEndRadius = 240.0f;
	beaminfo.m_nFlags = FBEAM_FADEOUT;
	beaminfo.m_vecStart = beaminfo.m_vecCenter;
	beaminfo.m_vecEnd = beaminfo.m_vecCenter;

	Beam_t *pBeam = ((CViewRenderBeams*)beams)->CreateGenericBeam(beaminfo);
	if(!pBeam)
	{
		Warning("[EMP effect] Ugly, hacky, never-going-to-work beam creation failed (surprise surprise!)");
		return;
	}

	pBeam->type = TE_BEAMRINGPOINT;
	pBeam->start_radius = beaminfo.m_flStartRadius;
	pBeam->end_radius = beaminfo.m_flEndRadius;
	pBeam->attachment[2] = beaminfo.m_vecCenter;
	((CViewRenderBeams*)beams)->SetBeamAttributes(pBeam, beaminfo);
	*/
}

DECLARE_CLIENT_EFFECT("FF_EmpZap", FF_FX_EmpZap_Callback);