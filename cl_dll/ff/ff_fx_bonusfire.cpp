#include "cbase.h"
#include "particles_simple.h"
#include "c_te_effect_dispatch.h"

#define BONUSFIRE_HEATWAVE_MATERIAL "sprites/heatwave"
#define BONUSFIRE_FLAME_MATERIAL_1 "sprites/flamelet4"
#define BONUSFIRE_FLAME_MATERIAL_2 "effects/flame"

ConVar ffdev_ic_bonusfire_numparticles_mult("ffdev_ic_bonusfire_numparticles_mult", "5", FCVAR_CHEAT);
#define IC_BONUSFIRE_NUMPARTICLES_MULT ffdev_ic_bonusfire_numparticles_mult.GetFloat()

class CBonusFireEffect : public CEmberEffect
{
public:
	CBonusFireEffect( const char *pDebugName ) : CEmberEffect(pDebugName) {};

	static CSmartPtr<CBonusFireEffect>	Create( const char *pDebugName )
	{
		CBonusFireEffect *pRet = new CBonusFireEffect( pDebugName );
		pRet->SetDynamicallyAllocated( true );
		
		if(m_hHeatwaveMaterial == INVALID_MATERIAL_HANDLE)
			m_hHeatwaveMaterial = pRet->GetPMaterial(BONUSFIRE_HEATWAVE_MATERIAL);
		if(m_hFlameMaterial1 == INVALID_MATERIAL_HANDLE)
			m_hFlameMaterial1 = pRet->GetPMaterial(BONUSFIRE_FLAME_MATERIAL_1);
		if(m_hFlameMaterial2 == INVALID_MATERIAL_HANDLE)
			m_hFlameMaterial2 = pRet->GetPMaterial(BONUSFIRE_FLAME_MATERIAL_2);

		return pRet;
	};
	
	static PMaterialHandle m_hHeatwaveMaterial;
	static PMaterialHandle m_hFlameMaterial1;
	static PMaterialHandle m_hFlameMaterial2;

private:
	CBonusFireEffect( const CBonusFireEffect & ); // not defined, not accessible
};

PMaterialHandle CBonusFireEffect::m_hHeatwaveMaterial = INVALID_MATERIAL_HANDLE;
PMaterialHandle CBonusFireEffect::m_hFlameMaterial1 = INVALID_MATERIAL_HANDLE;
PMaterialHandle CBonusFireEffect::m_hFlameMaterial2 = INVALID_MATERIAL_HANDLE;

void BonusFireCallback( const CEffectData &data )
{
	CSmartPtr<CBonusFireEffect> pEmitter = CBonusFireEffect::Create( "BonusFire" );

	if ( pEmitter == NULL )
		return;

	Vector	projectileOrigin = data.m_vOrigin;
	float	scale = data.m_flScale;
	CBaseEntity *pAffectedEntity = ClientEntityList().GetEnt( data.m_hEntity.GetEntryIndex() );
	Vector affectedOrigin = pAffectedEntity->GetAbsOrigin();
	Vector deltaVec = (affectedOrigin - projectileOrigin);
	VectorNormalize( deltaVec );

	Vector particleVec = Vector(deltaVec.x, deltaVec.y, 1);
	VectorNormalize(particleVec);

	// Set our sort origin to make the system cull properly
	pEmitter->SetSortOrigin( affectedOrigin );

	SimpleParticle *pParticle;

	int numParticles = IC_BONUSFIRE_NUMPARTICLES_MULT * scale;
	int numHeatParticles = numParticles / 3;
	int numOfEachFlameParticle = (numParticles - numHeatParticles) / 2;
	for ( int i = 0; i < numParticles; i++ )
	{
		bool isHeatParticle = i < numHeatParticles;
		PMaterialHandle hMaterial = CBonusFireEffect::m_hHeatwaveMaterial;
		if (i >= numParticles - numOfEachFlameParticle) hMaterial = CBonusFireEffect::m_hFlameMaterial1;
		else if (!isHeatParticle) hMaterial = CBonusFireEffect::m_hFlameMaterial2;

		// offset vertically based on the bounding radius
		float dV = random->RandomFloat(pAffectedEntity->BoundingRadius() / 2.0f, pAffectedEntity->BoundingRadius());
		Vector origin = Vector(affectedOrigin.x, affectedOrigin.y, affectedOrigin.z + dV);

		pParticle = pEmitter->AddSimpleParticle( hMaterial, origin );

		if ( pParticle == NULL )
			return;

		pParticle->m_uchStartSize = (unsigned char) (isHeatParticle ? random->RandomInt(24, 50) : random->RandomInt(10, 15));
		pParticle->m_uchEndSize = random->RandomInt(0, 5);

		pParticle->m_flRoll = random->RandomFloat( 0, 2*M_PI );
		pParticle->m_flRollDelta = random->RandomFloat( -DEG2RAD( 180 ), DEG2RAD( 180 ) );

		pParticle->m_uchColor[0] = 255;	// Red
		pParticle->m_uchColor[1] = 255;	// Green
		pParticle->m_uchColor[2] = 255;	// Blue

		pParticle->m_uchStartAlpha = 100;
		pParticle->m_uchEndAlpha = 255;

		// Find a random speed for the particle
		float speed = random->RandomFloat( 140.0f, 280.0f );

		// Build and set the velocity of the particle
		pParticle->m_vecVelocity = particleVec * speed;

		// Declare our lifetime
		pParticle->m_flDieTime = random->RandomFloat(0.25f, 0.5f);
	}
}

DECLARE_CLIENT_EFFECT( "BonusFire", BonusFireCallback );