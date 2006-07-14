/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_fx_grenades.cpp
/// @author Shawn Smith (L0ki)
/// @date Apr. 30, 2005
/// @brief implementations of the grenade effects
///
/// Implementations for all of the grenade effect functions
/// 
/// Revisions
/// ---------
/// Apr. 30, 2005	L0ki: Initial Creation

#include "cbase.h"
#include "beam_flags.h"
#include "c_te_effect_dispatch.h"
#include "fx.h"
#include "ff_fx_grenades.h"
#include "ff_grenade_base.h"
#include "ff_fx_napalm_emitter.h"
#include "ff_fx_gascloud_emitter.h"
#include "smoke_fog_overlay.h"

extern ConVar conc_framerate;
extern ConVar conc_life;
extern ConVar conc_width;
extern ConVar conc_spread;
extern ConVar conc_amplitude;
extern ConVar conc_speed;

extern ConVar nap_burst_scale;
ConVar nap_burst_gravity("ffdev_nap_burst_gravity","-500.0",0,"Gravity magnitude for the napalm burst");
ConVar nap_burst_vel("ffdev_nap_burst_vel","475",0,"Velocity of the napalm particles.");
ConVar nap_burst_vel_z_divisor("ffdev_nap_burst_vel_z_divisor","2.0",0,"How much to divide the linear velocity for napalm particles by for the vertical velocity.");

extern ConVar gas_scale;
ConVar gas_speed("ffdev_gas_speed","0.75",0,"Speed gas particles expand outward.");

void FF_FX_DrawCircle(Vector &center, float radius, int modelindex, float life = 1.0f)
{
	CPVSFilter filter(center);
	Vector forward, right, up;
	QAngle angle = QAngle(0,0,0);
	//IMaterial *pMat = materials->FindMaterial( "sprites/laser", TEXTURE_GROUP_CLIENT_EFFECTS );
	Vector startpos, endpos;

	AngleVectors(angle,&forward,&right,&up);
	VectorNormalize(forward);
	startpos = center + forward * radius;

	for(int iAngle = 0;iAngle<=360;iAngle += 5)
	{
		angle.y = iAngle;
		AngleVectors(angle,&forward,&right,&up);
		endpos = center + forward * radius;
		te->BeamPoints(
			filter,
			0,
			&startpos,
			&endpos,
			modelindex,
			0,
			0,
			2,
			life,
			5.0,
			5.0,
			5.0,
			0,
			255,
			0,
			0,
			255,
			0
			);
		startpos = endpos;
	}
}

void FF_FX_ConcussionExplosion( Vector &origin )
{
	return;
	float radius = 180.0f;
	//FF_FX_DrawCircle(origin,radius,CFFGrenadeBase::m_iShockwaveTexture);
	CBroadcastRecipientFilter filter;
	te->BeamRingPoint(
		filter,
		0,
		origin,
		0,
		radius,
		CFFGrenadeBase::m_iShockwaveTexture ,
		0,
		0,
		conc_framerate.GetInt(),
		conc_life.GetFloat(),
		conc_width.GetFloat(),
		conc_spread.GetFloat(),
		conc_amplitude.GetFloat(),
		255,
		255,
		255,
		100,
		conc_speed.GetFloat(),
		FBEAM_FADEOUT
	);
}

void ConcussionExplosionCallback(const CEffectData &data)
{
	Vector origin = data.m_vOrigin;
	FF_FX_ConcussionExplosion(origin);
}
DECLARE_CLIENT_EFFECT( "ConcussionExplosion", ConcussionExplosionCallback )


void FF_FX_NapalmBurst( Vector &origin )
{
	float radius = 180.0f;
	//FF_FX_DrawCircle(origin,radius,CFFGrenadeBase::m_iShockwaveTexture);
	CSmartPtr<CNapalmEmitter> pEmitter = CNapalmEmitter::Create("NapalmBurst");
	if(pEmitter == NULL)
		return;
	pEmitter->SetSortOrigin(origin);

	pEmitter->SetGravity( Vector(0,0,1), nap_burst_gravity.GetFloat() );

	//float scale = nap_burst_scale.GetFloat();
	NapalmParticle *pParticle = NULL;
	QAngle angle;
	Vector forward, right, up, velocity;

	//psuedo random burst pattern
	for(float r=radius, offset=0; r>0; r -= 45, offset++)
	{
		int iAngleSeed = RandomInt(0,360);
		for(int iAngle = iAngleSeed; iAngle < (iAngleSeed + 360)/*360*/; iAngle += 60)
		{
			pParticle = pEmitter->AddNapalmParticle(origin);
			if(pParticle)
			{
				angle.x = RandomFloat(45,67.5);//67.5f;
				angle.y = (offset*30) + iAngle;
				angle.z = 0;
				AngleVectors(angle, &forward, &right, &up);

				velocity = forward * (nap_burst_vel.GetFloat() * ((r) / radius));
				velocity.z = RandomFloat(100.0f,nap_burst_vel.GetFloat()/nap_burst_vel_z_divisor.GetFloat());
				pParticle->m_vVelocity = velocity;
				if(random->RandomInt(0,1))
					pParticle->m_bReverseSize = true;
				else
					pParticle->m_bReverseSize = false;
			}
		}
	}
}

void NapalmBurstCallback(const CEffectData &data)
{
	Vector origin = data.m_vOrigin;
	FF_FX_NapalmBurst(origin);
}

DECLARE_CLIENT_EFFECT( "NapalmBurst", NapalmBurstCallback )

// Mirv: This is no longer used!! Gas grenade has its own CGasCloud emitter
void FF_FX_GasCloud( Vector &origin )
{
	CSmartPtr<CGasCloud> pGasCloud = CGasCloud::Create("GasCloud");

	if (pGasCloud == NULL)
		return;

	pGasCloud->SetSortOrigin(origin);

	//float scale = gas_scale.GetFloat();
	GasParticle *pParticle = NULL;
	QAngle angle;
	Vector forward, right, up, velocity;

	// TODO: make the gas cloud act like "real" gas, and disperse from the grenade and slowly "fill"
	//		 the area around the grenade
	for (int i = 0; i < 5; i++)
	{
		pParticle = pGasCloud->AddGasParticle(origin);
		
		if(!pParticle)
			return;

		// Pick a random direction
		Vector vecDirection(RandomFloat(-1.0, 1.0f), RandomFloat(-1.0, 1.0f), RandomFloat(0, 2.0f));
		vecDirection.NormalizeInPlace();

		// And a random distance
		Vector vecFinalPos = origin + vecDirection * RandomFloat(50.0f, 200.0f);

		// Go as far as possible
		trace_t tr;
		UTIL_TraceLine(origin, vecFinalPos, MASK_SOLID, NULL, COLLISION_GROUP_DEBRIS, &tr);

		// Takes 5 seconds for a cloud to disperse
		pParticle->m_vVelocity = (tr.endpos - origin) * 0.2f;

		// This is the position we're going to, even though we may not reach it
		pParticle->m_vFinalPos = tr.endpos;
	}
}

void GasCloudCallback(const CEffectData &data)
{
	Vector origin = data.m_vOrigin;
	FF_FX_GasCloud(origin);
}

DECLARE_CLIENT_EFFECT( "GasCloud", GasCloudCallback )

void FF_FX_EmpExplosion( Vector &origin )
{
	return;
	float radius = 180.0f;
	// TODO: something better looking?
	//shockwave
	CBroadcastRecipientFilter filter;
	te->BeamRingPoint(
		filter,
		0,
		origin,
		0,
		radius,
		CFFGrenadeBase::m_iShockwaveTexture ,
		0,
		0,
		conc_framerate.GetInt(),
		conc_life.GetFloat(),
		conc_width.GetFloat(),
		conc_spread.GetFloat(),
		conc_amplitude.GetFloat(),
		255,
		255,
		0,
		100,
		conc_speed.GetFloat(),
		FBEAM_FADEOUT
		);
}

void EmpExplosionCallback(const CEffectData &data)
{
	Vector origin = data.m_vOrigin;
	FF_FX_EmpExplosion(origin);
}

DECLARE_CLIENT_EFFECT( "EmpExplosion", EmpExplosionCallback )