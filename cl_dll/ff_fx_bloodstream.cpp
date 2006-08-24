/********************************************************************
	created:	2006/08/24
	created:	24:8:2006   14:07
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff_fx_bloodstream.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll
	file base:	ff_fx_bloodstream
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "particles_simple.h"
#include "ff_fx_bloodstream.h"

static ConVar blood_speed_min("ffdev_blood_speed_min", "10.0");
static ConVar blood_speed_max("ffdev_blood_speed_max", "15.0");
static ConVar blood_startsize("ffdev_blood_startsize", "4.0");
static ConVar blood_endsize("ffdev_blood_endsize", "6.0");
static ConVar blood_fluc("ffdev_blood_fluc", "10.0f");

PMaterialHandle CBloodStream::m_hMaterial			= INVALID_MATERIAL_HANDLE;

#define BLOOD_STREAM_MATERIAL		"effects/blood_core"

CLIENTEFFECT_REGISTER_BEGIN(PrecacheBloodStream)
CLIENTEFFECT_MATERIAL(BLOOD_STREAM_MATERIAL)
CLIENTEFFECT_REGISTER_END()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CBloodStream::CBloodStream(const char *pDebugName) : CParticleEffect(pDebugName)
{
	m_pDebugName = pDebugName;

	m_flNearClipMin	= 16.0f;
	m_flNearClipMax	= 64.0f;

	m_flNextParticle = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CBloodStream::~CBloodStream()
{
}

//-----------------------------------------------------------------------------
// Purpose: Create a new blood stream with an owner ragdoll
//-----------------------------------------------------------------------------
CSmartPtr<CBloodStream> CBloodStream::Create(C_BaseAnimatingOverlay *pRagdoll, const char *pDebugName)
{
	CBloodStream *pRet = new CBloodStream(pDebugName);
	pRet->SetDynamicallyAllocated(true);

	if (m_hMaterial == INVALID_MATERIAL_HANDLE)
	{
		m_hMaterial = pRet->GetPMaterial(BLOOD_STREAM_MATERIAL);
	}

	pRet->m_hEntAttached = pRagdoll;

	return pRet;
}

//-----------------------------------------------------------------------------
// Purpose: Create particle with default settings
//-----------------------------------------------------------------------------
BloodStreamParticle * CBloodStream::AddBloodStreamParticle(const Vector &vOrigin)
{
	BloodStreamParticle *pRet = (BloodStreamParticle *) AddParticle(sizeof(BloodStreamParticle), m_hMaterial, vOrigin);

	if (pRet)
	{
		pRet->m_vOrigin = vOrigin;
		pRet->m_vFinalPos.Init();
		pRet->m_vVelocity.Init();
		pRet->m_flDieTime = random->RandomFloat(0.7f, 1.3f);
		pRet->m_flLifetime = 0;
		pRet->m_uchColor[0] = 192;
		pRet->m_uchColor[1] = 32;
		pRet->m_uchColor[2] = 32;
		pRet->m_flAlpha = 0.8f;
		pRet->m_flSize = 2.0f;
	}

	return pRet;
}

//-----------------------------------------------------------------------------
// Purpose: Move the particles with drag & gravity
//-----------------------------------------------------------------------------
void CBloodStream::SimulateParticles(CParticleSimulateIterator *pIterator)
{
	float timeDelta = pIterator->GetTimeDelta();


	BloodStreamParticle *pParticle = (BloodStreamParticle *) pIterator->GetFirst();
	while (pParticle)
	{
		pParticle->m_flLifetime += timeDelta;

		float end = pParticle->m_flLifetime / pParticle->m_flDieTime;
		float start = 1.0f - end;

		Vector F(0.0f, 0.0f, 0.0f);

		//ApplyDrag(&F, pParticle->m_vVelocity, 4.0f, 20.0f);

		pParticle->m_Pos += pParticle->m_vVelocity * timeDelta * 0.5f;
		pParticle->m_vVelocity += F * timeDelta;							// assume mass of 1
		pParticle->m_vVelocity += Vector(0, 0, -0.5f) * pParticle->m_flLifetime;		// gravity
		pParticle->m_Pos += pParticle->m_vVelocity * timeDelta * 0.5f;
		pParticle->m_flAlpha = 0.8f * start + 0.0f * end;
		pParticle->m_flSize = blood_startsize.GetFloat() * start + blood_endsize.GetFloat() * end;

		if (pParticle->m_flLifetime >= pParticle->m_flDieTime)
			pIterator->RemoveParticle(pParticle);

		pParticle = (BloodStreamParticle *) pIterator->GetNext();
	}
}

void CBloodStream::AddAttractor(Vector *F, Vector apos, Vector ppos, float scale)
{
	Vector dir = (apos - ppos);
	dir.NormalizeInPlace();
	float dist = (apos - ppos).Length();
	if (dist > 0.00001f)
		*F += (scale / (dist/* * dist */)) * dir;
}

void CBloodStream::ApplyDrag(Vector *F, Vector vel, float scale, float targetvel)
{
	if (vel.IsLengthLessThan(targetvel))
		return;
	Vector dir = -vel;
	vel.NormalizeInPlace();
	float mag = vel.Length() * scale;
	*F += (dir * mag);
}


//-----------------------------------------------------------------------------
// Purpose: Render. TODO: Stop swapping between vector/char array for colour
//-----------------------------------------------------------------------------
void CBloodStream::RenderParticles(CParticleRenderIterator *pIterator)
{
	const BloodStreamParticle *pParticle = (const BloodStreamParticle *) pIterator->GetFirst();
	while (pParticle)
	{
		//Render
		Vector	tPos;

		TransformParticle(ParticleMgr()->GetModelView(), pParticle->m_Pos, tPos);
		float sortKey = (int) tPos.z;

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

		pParticle = (const BloodStreamParticle *) pIterator->GetNext(sortKey);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Add a new bunch of particles at each attachment point
//-----------------------------------------------------------------------------
void CBloodStream::Update(float flTimeDelta)
{
	// Don't create any more after this has died. Once the remaining ones have
	// been simulated this entity will automatically remove itself
	if (gpGlobals->curtime > m_flDieTime)
		return;

	if (gpGlobals->curtime < m_flNextParticle)
		return;

	m_flNextParticle = gpGlobals->curtime + 0.02f;

	BloodStreamParticle *pParticle = NULL;
	QAngle angle;
	Vector forward, right, up, velocity;

	C_BaseAnimatingOverlay *pRagdoll = (C_BaseAnimatingOverlay *) m_hEntAttached.Get();

	if (!pRagdoll)
		return;

	// Add a particle at each decapitated spot
	for (int i = 1; i <= 5; i++)
	{
		if (pRagdoll->GetBodygroup(i))
		{
			Vector vecOrigin;
			QAngle angDir;
			pRagdoll->GetAttachment(i, vecOrigin, angDir);

			pParticle = AddBloodStreamParticle(vecOrigin);

			if (!pParticle)
				return;

#define BLOOD_FLUCTUATION	blood_fluc.GetFloat()

			AngleVectors(angDir + QAngle(random->RandomFloat(-BLOOD_FLUCTUATION, BLOOD_FLUCTUATION), random->RandomFloat(-BLOOD_FLUCTUATION, BLOOD_FLUCTUATION),random->RandomFloat(-BLOOD_FLUCTUATION, BLOOD_FLUCTUATION)), &vecOrigin);
			pParticle->m_vVelocity = vecOrigin * random->RandomFloat(blood_speed_min.GetFloat(), blood_speed_max.GetFloat());
		}
	}
}