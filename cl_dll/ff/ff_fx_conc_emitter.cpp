/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_fx_conc_emitter.cpp
/// @author Paul Peloski (zero)
/// @date Feb 5, 2006
/// @brief conc effect emitter
///
/// Implementation of the conc effect emitter particle system
/// 
/// Revisions
/// ---------
/// Feb 5, 2006	zero: Initial Creation

#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "particles_simple.h"
#include "ff_fx_conc_emitter.h"
#include "materialsystem/imaterialvar.h"
#include "c_te_effect_dispatch.h"

#define CONC_EFFECT_MATERIAL "sprites/concrefract"

ConVar conc_on			 ("ffdev_conc_on", "1", 0, "Turn the conc effect on or off - 1 or 0." );
ConVar conc_scale		 ("ffdev_conc_scale", "512.0", 0, "How big the conc effect gets.");
ConVar conc_refract		 ("ffdev_conc_refract", "0.2", 0, "Refraction amount for conc effect.");
ConVar conc_blur		 ("ffdev_conc_blur", "0", 0, "Blur amount for conc effect.");
ConVar conc_speed		 ("ffdev_conc_speed", "0.5", 0, "Duration of the conc effect.");
ConVar conc_ripples		 ("ffdev_conc_ripples", "1", 0, "How many ripples the conc effect has.");
ConVar conc_ripple_period("ffdev_conc_ripple_period", "0.05", 0, "Time between ripples.");

//========================================================================
// Client effect precache table
//========================================================================
CLIENTEFFECT_REGISTER_BEGIN(PrecacheConcEmitter)
	CLIENTEFFECT_MATERIAL(CONC_EFFECT_MATERIAL)
CLIENTEFFECT_REGISTER_END()

//========================================================================
// Static material handles
//========================================================================
PMaterialHandle CConcEmitter::m_hMaterial = INVALID_MATERIAL_HANDLE;

//========================================================================
// CConcEmitter constructor
//========================================================================
CConcEmitter::CConcEmitter(const char *pDebugName) : CSimpleEmitter(pDebugName)
{
	m_pDebugName = pDebugName;
}

//========================================================================
// CConcEmitter destructor
//========================================================================
CConcEmitter::~CConcEmitter()
{
}

//========================================================================
// CConcEmitter::Create
// ----------------------
// Purpose: Creates a new instance of a CConcEmitter object
//========================================================================
CSmartPtr<CConcEmitter> CConcEmitter::Create(const char *pDebugName)
{
	CConcEmitter *pRet = new CConcEmitter(pDebugName);

	pRet->SetDynamicallyAllocated();

	if (m_hMaterial == INVALID_MATERIAL_HANDLE)
		m_hMaterial = pRet->GetPMaterial(CONC_EFFECT_MATERIAL);

	return pRet;
}

//========================================================================
// CConcEmitter::RenderParticles
// ----------
// Purpose: Renders all the particles in the system
//========================================================================
void CConcEmitter::RenderParticles(CParticleRenderIterator *pIterator)
{
	if( conc_on.GetInt() == 0 )
		return;

	const	ConcParticle *pParticle = (const ConcParticle *) pIterator->GetFirst();
	
	float	flLife, flDeath;

	bool	bFound;

	while (pParticle)
	{
		Vector	tPos;

		TransformParticle(g_ParticleMgr.GetModelView(), pParticle->m_Pos, tPos);
		float sortKey = (int) tPos.z;

		flLife = pParticle->m_flLifetime - pParticle->m_flOffset;
		flDeath = pParticle->m_flDieTime - pParticle->m_flOffset;

		if (flLife > 0.0f)
		{

			IMaterial *pMat = pIterator->GetParticleDraw()->m_pSubTexture->m_pMaterial;

		    if (pMat)
			{
				IMaterialVar *pVar = pMat->FindVar("$refractamount", &bFound, true);

				if (pVar)
					pVar->SetFloatValue(-pParticle->m_flRefract + SimpleSplineRemapVal(flLife, 0.0f, flDeath, 0.001f, pParticle->m_flRefract));

				pVar = pMat->FindVar("$bluramount", &bFound, true);

				if (pVar)
					pVar->SetFloatValue(conc_blur.GetFloat());
			}

			float flColor = 1.0f;// RemapVal(flLife, 0.0f, flDeath, 1.0f, 0.0f);

			Vector vColor = Vector(flColor, flColor, flColor);

			// Render it
			RenderParticle_ColorSizeAngle(
				pIterator->GetParticleDraw(), 
				tPos, 
				vColor, 
				flColor, 																	// Alpha
				SimpleSplineRemapVal(flLife, 0.0f, flDeath, 0, conc_scale.GetFloat()), 		// Size
				pParticle->m_flRoll
				);
		}

		pParticle = (const ConcParticle *) pIterator->GetNext(sortKey);
	}

}

//========================================================================
// CConcEmitter::AddConcParticle
// ----------
// Purpose: Add a new particle to the system
//========================================================================
ConcParticle * CConcEmitter::AddConcParticle()
{
	ConcParticle *pRet = (ConcParticle *) AddParticle(sizeof(ConcParticle), m_hMaterial, GetSortOrigin());

	if (pRet)
	{
		pRet->m_Pos = GetSortOrigin();
		pRet->m_vecVelocity.Init();
		pRet->m_flRoll = 0;
		pRet->m_flRollDelta = 0;
		pRet->m_flLifetime = 0;
		pRet->m_flDieTime = 0;
		pRet->m_uchColor[0] = pRet->m_uchColor[1] = pRet->m_uchColor[2] = 0;
		pRet->m_uchStartAlpha = pRet->m_uchEndAlpha = 255;
		pRet->m_uchStartSize = 0;
		pRet->m_iFlags = 0;
		pRet->m_flOffset = 0;
	}

	return pRet;
}

void FF_FX_ConcussionEffect_Callback(const CEffectData &data)
{
	CSmartPtr<CConcEmitter> concEffect = CConcEmitter::Create("ConcussionEffect");
	
	float offset = 0;

	for (int i = 0; i < conc_ripples.GetInt(); i++)
	{
		ConcParticle *c = concEffect->AddConcParticle();

		c->m_flDieTime = conc_speed.GetFloat();
		c->m_Pos = data.m_vOrigin;
		c->m_flRefract = conc_refract.GetFloat();
		c->m_flOffset = offset;

		offset += conc_ripple_period.GetFloat();
	}
}

DECLARE_CLIENT_EFFECT("FF_ConcussionEffect", FF_FX_ConcussionEffect_Callback);
