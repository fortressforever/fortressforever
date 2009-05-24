/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file d:\ffsrc\cl_dll\ff_fx_concbits.cpp
/// @author Ted Maul (ted_maul)
/// @date 2006/04/12
/// @brief conc particle effects
///
/// particle manager for the conc particles

#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "particles_simple.h"
#include "c_te_effect_dispatch.h"
#include "cliententitylist.h"

#include "ff_fx_concbits.h"

extern ConVar conc_glow_r;
extern ConVar conc_glow_g;
extern ConVar conc_glow_b;
extern ConVar conc_glow_a;

ConVar conc_bitforce("ffdev_conc_bitforce","30000", FCVAR_CHEAT,"Strength of conc bit attractor.");
ConVar conc_moveforce("ffdev_conc_moveforce","0.2", FCVAR_CHEAT,"Strength of conc movement attractor.");

PMaterialHandle CConcBitsEmitter::m_hMaterial = INVALID_MATERIAL_HANDLE;

#define CONC_BITS_MATERIAL "particle/particle_smokegrenade"

CLIENTEFFECT_REGISTER_BEGIN(PrecacheConcBits)
CLIENTEFFECT_MATERIAL(CONC_BITS_MATERIAL)
CLIENTEFFECT_REGISTER_END()

CSmartPtr<CConcBitsEmitter> CConcBitsEmitter::Create(const char *pDebugName)
{
	CConcBitsEmitter *pRet = new CConcBitsEmitter(pDebugName);
	pRet->SetDynamicallyAllocated(true);
	if(m_hMaterial == INVALID_MATERIAL_HANDLE)
		m_hMaterial = pRet->GetPMaterial(CONC_BITS_MATERIAL);
	return pRet;
}

CConcBitsEmitter::CConcBitsEmitter(const char *pDebugName) : CParticleEffect(pDebugName)
{
	m_pDebugName = pDebugName;
}

CConcBitsEmitter::~CConcBitsEmitter()
{
}

ConcBitParticle* CConcBitsEmitter::AddConcBit(const Vector &vOrigin)
{
	ConcBitParticle *pRet = (ConcBitParticle*)AddParticle(sizeof(ConcBitParticle), m_hMaterial, vOrigin);
	if (pRet)
	{
		pRet->m_vOrigin = vOrigin;
		pRet->m_vFinalPos.Init();
		pRet->m_vVelocity.Init();
		pRet->m_flDieTime = 2.0f;
		pRet->m_flLifetime = 0;
		pRet->m_uchColor[0] = 255;
		pRet->m_uchColor[1] = 255;
		pRet->m_uchColor[2] = 255;
		pRet->m_flAlpha = 0.8f;
		pRet->m_flSize = 1.0f;
	}

	return pRet;
}

void CConcBitsEmitter::SimulateParticles(CParticleSimulateIterator *pIterator)
{
	float timeDelta = pIterator->GetTimeDelta();

	ConcBitParticle *pParticle = (ConcBitParticle*)pIterator->GetFirst();
	while(pParticle)
	{
		pParticle->m_flLifetime += timeDelta;

		float end = pParticle->m_flLifetime / pParticle->m_flDieTime;
		float start = 1.0f - end;

		//if (pParticle->m_flLifetime < pParticle->m_flEndPosTime)
		{
			// calculate forces on particle
			// start with acceleration towards the origin
			Vector F(0.0f, 0.0f, 0.0f);

			AddAttractor(&F, pParticle->m_vOrigin, pParticle->m_Pos, conc_bitforce.GetFloat());

			C_BaseEntityIterator iterator;
			CBaseEntity *point = iterator.Next();
			while(point != NULL)
			{
				if((point->GetAbsOrigin() - pParticle->m_vOrigin).IsLengthLessThan(512.0f))
				{
					AddAttractor(&F, point->GetAbsOrigin(), pParticle->m_Pos, conc_moveforce.GetFloat() * point->GetAbsVelocity().LengthSqr());
				}
				point = iterator.Next();
			}

			ApplyDrag(&F, pParticle->m_vVelocity, 1.0f, 400.0f);
            
			pParticle->m_Pos += pParticle->m_vVelocity * timeDelta * 0.5f;
			pParticle->m_vVelocity += F * timeDelta;							// assume mass of 1
			//DevMsg("odist: %f\tF: %f\t v: %f\n", odist, F.Length(), pParticle->m_vVelocity.Length());
			pParticle->m_Pos += pParticle->m_vVelocity * timeDelta * 0.5f;
		}

		pParticle->m_flAlpha = 0.8f * start + 0.0f * end;
		pParticle->m_flSize = 24.0f * start + 128.0f * end;

		if (pParticle->m_flLifetime >= pParticle->m_flDieTime)
			pIterator->RemoveParticle(pParticle);

		pParticle = (ConcBitParticle*)pIterator->GetNext();
	}

}

void CConcBitsEmitter::AddAttractor(Vector *F, Vector apos, Vector ppos, float scale)
{
	Vector dir = (apos - ppos);
	dir.NormalizeInPlace();
	float dist = (apos - ppos).Length();
	if(dist > 0.00001f)
		*F += (scale / (dist/* * dist*/)) * dir;
}

void CConcBitsEmitter::ApplyDrag(Vector *F, Vector vel, float scale, float targetvel)
{
	if(vel.IsLengthLessThan(targetvel))
		return;
	Vector dir = -vel;
	vel.NormalizeInPlace();
	float mag = vel.Length() * scale;
	*F += (dir * mag);
}

void CConcBitsEmitter::RenderParticles(CParticleRenderIterator *pIterator)
{
	const ConcBitParticle *pParticle = (const ConcBitParticle*)pIterator->GetFirst();
	while(pParticle)
	{
		Vector	tPos;

		TransformParticle(ParticleMgr()->GetModelView(), pParticle->m_Pos, tPos);
		float sortKey = (int)tPos.z;

		Vector vColor = Vector(pParticle->m_uchColor[0] / 255.0f,
			pParticle->m_uchColor[1] / 255.0f,
			pParticle->m_uchColor[2] / 255.0f);

		RenderParticle_ColorSize(
			pIterator->GetParticleDraw(),
			tPos,
			vColor,
			pParticle->m_flAlpha,
			pParticle->m_flSize
			);

		pParticle = (const ConcBitParticle*)pIterator->GetNext(sortKey);
	}
}

void FF_FX_ConcBits_Callback(const CEffectData &data)
{
	CSmartPtr<CConcBitsEmitter> concEffect = CConcBitsEmitter::Create("ConcBits");

	const unsigned int num_particles = 32;
	Vector offset(0.0f, 0.0f, 0.0f);

	for(unsigned int i = 0; i < num_particles; i++)
	{
        ConcBitParticle *p =  concEffect->AddConcBit(data.m_vOrigin + offset);
        if(p)
		{
			p->m_vOrigin = data.m_vOrigin + offset;
			p->m_vVelocity = RandomVector(-50.0f, 50.0f);
			p->m_vVelocity.z *= 0.1f;
			p->m_Pos = p->m_vOrigin + RandomVector(-50.0f, 50.0f);
			p->m_Pos.z = p->m_vOrigin.z + RandomFloat(-10.0f, 10.0f);
		}
	}
}

DECLARE_CLIENT_EFFECT("FF_ConcBitsEffect", FF_FX_ConcBits_Callback);