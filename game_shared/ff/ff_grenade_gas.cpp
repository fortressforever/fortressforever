/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file ff_grenade_gas.cpp
/// @author Shawn Smith (L0ki)
/// @date Apr. 23, 2005
/// @brief gas grenade class
/// 
/// Implementation of the CFFGrenadeGas class. This is the secondary grenade type for spy.
/// 
/// Revisions
/// ---------
/// Apr. 23, 2005	L0ki: Initial Creation

#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"

#define GASGRENADE_MODEL	"models/grenades/gas/gas.mdl"
#define GAS_SOUND			"GasGrenade.Explode"
#define GAS_EFFECT			"GasCloud"

#ifdef CLIENT_DLL
	#define CFFGrenadeGas C_FFGrenadeGas
	#include "c_te_effect_dispatch.h"
	#include "ff_fx_gascloud_emitter.h"
	#include <igameevents.h>
#else
	#include "te_effect_dispatch.h"
	#include "ff_entity_system.h"
	#include "ai_basenpc.h"
#endif

#define CONCUSSION_SOUND				"ConcussionGrenade.Explode"
#define CONCUSSION_EFFECT_HANDHELD		"FF_ConcussionEffectHandheld" // "ConcussionExplosion"

// 0000819: gas gren radius too large
ConVar ffdev_gasgrenradius("ffdev_gasgrenradius", "280.0", FCVAR_REPLICATED | FCVAR_CHEAT);

class CFFGrenadeGas : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeGas,CFFGrenadeBase)
	DECLARE_NETWORKCLASS();

	// Server spits out a complaint about this not being unsigned
	CNetworkVar( unsigned int, m_bIsEmitting );

	virtual void Precache();
	virtual float GetShakeAmplitude( void ) { return 0.0f; }	// remove the shake
	virtual float GetGrenadeDamage() { return 0.0f; }
	// 0000819: gas gren radius too large
	virtual float GetGrenadeRadius() { return ffdev_gasgrenradius.GetFloat(); }
	virtual const char *GetBounceSound() { return "GasGrenade.Bounce"; }
	virtual Class_T Classify( void ) { return CLASS_GREN_GAS; }

	virtual color32 GetColour() { color32 col = { 20, 168, 20, GREN_ALPHA_DEFAULT }; return col; }

#ifdef CLIENT_DLL
	CFFGrenadeGas() {}
	CFFGrenadeGas( const CFFGrenadeGas& ) {}

	virtual void ClientThink();
	virtual void OnDataChanged(DataUpdateType_t updateType);
	virtual void UpdateOnRemove( void );

	CSmartPtr<CGasCloud>	m_pGasEmitter;

#else
	virtual void Spawn();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);
	virtual void GrenadeThink();

protected:
	int m_iSequence;
	Activity m_Activity;

	float m_flNextHurt;
	float m_flOpenTime;
	float m_flNextPuff;

	Vector	m_vecLastPosition;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeGas, DT_FFGrenadeGas)

BEGIN_NETWORK_TABLE(CFFGrenadeGas, DT_FFGrenadeGas)
#ifdef GAME_DLL
SendPropInt(SENDINFO(m_bIsEmitting), 1, SPROP_UNSIGNED),
#else
RecvPropInt(RECVINFO(m_bIsEmitting)),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( ff_grenade_gas, CFFGrenadeGas );
PRECACHE_WEAPON_REGISTER( ff_grenade_gas );

#ifdef GAME_DLL

	Activity ACT_GAS_IDLE;
	Activity ACT_GAS_DEPLOY;
	Activity ACT_GAS_DEPLOY_IDLE;

	//-----------------------------------------------------------------------------
	// Purpose: Set model, activities
	//-----------------------------------------------------------------------------
	void CFFGrenadeGas::Spawn( void )
	{
		SetModel( GASGRENADE_MODEL );
		BaseClass::Spawn();

		ADD_CUSTOM_ACTIVITY( CFFGrenadeGas, ACT_GAS_IDLE );
		ADD_CUSTOM_ACTIVITY( CFFGrenadeGas, ACT_GAS_DEPLOY );
		ADD_CUSTOM_ACTIVITY( CFFGrenadeGas, ACT_GAS_DEPLOY_IDLE );

		m_Activity = ( Activity )ACT_GAS_IDLE;
		m_iSequence = SelectWeightedSequence( m_Activity );
		SetSequence( m_iSequence );		

		m_flNextHurt = 0;
		m_flOpenTime = 0.0f;
		m_flNextPuff = 0.0f;

		m_bIsEmitting = 0;

		m_vecLastPosition = vec3_origin;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Currently all the gas logic is tied up in GrenadeThink
	//-----------------------------------------------------------------------------
	void CFFGrenadeGas::Explode(trace_t *pTrace, int bitsDamageType)
	{
		UTIL_Remove(this);
	}

	// Jiggles: I commented out the enabling of vphysics in an attempt to "fix" (avoid)
	//			1552: Gas nades can make detpacks clip through map
	void CFFGrenadeGas::GrenadeThink( void )
	{
		// If we're done deploying, deploy idle
		if( m_Activity == ACT_GAS_DEPLOY )
		{
			m_Activity = ACT_GAS_DEPLOY_IDLE;
			m_iSequence = SelectWeightedSequence( m_Activity );
			SetSequence( m_iSequence );

			m_bIsEmitting = 1;
		}

		// Stop the thing from rolling if it starts moving
		//if( ( gpGlobals->curtime > m_flOpenTime + 2.0f ) && VPhysicsGetObject() )
		//{
		//	IPhysicsObject *pObject = VPhysicsGetObject();
		//	
		//	Vector vecVelocity;
		//	AngularImpulse angImpulse;
		//	pObject->GetVelocity( &vecVelocity, &angImpulse );

		//	if( vecVelocity.IsZero() || angImpulse.IsZero() )
		//		pObject->EnableMotion( false );
		//}

		// If open and we're not moving and physics aren't enabled, enable physics
		//if( ( m_flOpenTime != 0.0f ) && ( GetAbsVelocity() == vec3_origin ) && !VPhysicsGetObject() )
		//{
		//	VPhysicsInitNormal( SOLID_VPHYSICS, GetSolidFlags(), false );
		//}

		// Been detonated for 10 secs now, so fade out
		if (gpGlobals->curtime > m_flDetonateTime + 0.0f)
		{
			SetThink(&CBaseGrenade::SUB_FadeOut);
		}		

		// Damage people in here
		if (gpGlobals->curtime > m_flDetonateTime && m_flNextHurt < gpGlobals->curtime)
		{
			// If the grenade is in a no grenade area, kill it
			// This is now only run when the grenade has changed position
			if (m_vecLastPosition != GetAbsOrigin())
			{
				if( !FFScriptRunPredicates( this, "onexplode", true ) && ( gpGlobals->curtime > m_flDetonateTime ) )
				{
					UTIL_Remove(this);
					return;
				}

				m_vecLastPosition = GetAbsOrigin();
			}

			m_flNextHurt = gpGlobals->curtime + 0.2f;

			// If we were idling, deploy
			if( m_Activity == ACT_GAS_IDLE )
			{
				// If it's at rest, enable physics
				//if( GetAbsVelocity() == vec3_origin )
				//	VPhysicsInitNormal( SOLID_VPHYSICS, GetSolidFlags(), false );

				m_flOpenTime = gpGlobals->curtime;

				m_Activity = ACT_GAS_DEPLOY;
				m_iSequence = SelectWeightedSequence( m_Activity );
				SetSequence( m_iSequence );

				//EmitSound(GAS_SOUND);
				EmitSound(CONCUSSION_SOUND);
				CEffectData data;
				data.m_vOrigin = GetAbsOrigin();
				data.m_flScale = 1.0f;
				data.m_flRadius = GetGrenadeRadius();
				DispatchEffect(CONCUSSION_EFFECT_HANDHELD, data);
				UTIL_Remove(this);
			}

			CBaseEntity *pEntity = NULL;
			for( CEntitySphereQuery sphere( GetAbsOrigin(), GetGrenadeRadius() ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
			{
				if( !pEntity )
					continue;

				if( !pEntity->IsPlayer() )
					continue;

				CFFPlayer *pPlayer = ToFFPlayer( pEntity );
				CFFPlayer *pGasser = ToFFPlayer( GetOwnerEntity() );

				if( !pPlayer || pPlayer->IsObserver() || !pGasser)
					continue;

				if( !g_pGameRules->FCanTakeDamage( pPlayer, GetOwnerEntity() ) )
					continue;

				pPlayer->Gas(7.5f, 7.5f, pGasser);
			}
		}

		// Animate
		StudioFrameAdvance();

		// Next think straight away
		SetNextThink(gpGlobals->curtime);

		// Now do rest of think
		//BaseClass::GrenadeThink();
	}
#else

	//-----------------------------------------------------------------------------
	// Purpose: Stop gas emitter
	//-----------------------------------------------------------------------------
	void CFFGrenadeGas::UpdateOnRemove( void )
	{
		if( m_bIsEmitting )
		{
			if( !!m_pGasEmitter )
			{
				m_pGasEmitter->SetDieTime( 0.0f );
				m_pGasEmitter = NULL;
			}
		}

		BaseClass::UpdateOnRemove();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Emit gas.
	//-----------------------------------------------------------------------------
	void CFFGrenadeGas::ClientThink()
	{
		if (m_bIsEmitting)
		{
			if (!m_pGasEmitter)
			{
				// Grenade deals damage for 10 seconds, gas lasts for 5 seconds
				// So we can die 5 seconds before
				m_pGasEmitter = CGasCloud::Create("GasCloud");
				m_pGasEmitter->SetDieTime(gpGlobals->curtime + 5.0f);
			}

			if (!!m_pGasEmitter)
			{
				m_pGasEmitter->UpdateEmitter(GetAbsOrigin(), GetAbsVelocity());
			}
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Called when data changes on the server
	//-----------------------------------------------------------------------------
	void CFFGrenadeGas::OnDataChanged(DataUpdateType_t updateType)
	{
		// NOTE: We MUST call the base classes' implementation of this function
		BaseClass::OnDataChanged(updateType);

		// Setup our entity's particle system on creation
		if (updateType == DATA_UPDATE_CREATED)
		{
			// Call our ClientThink() function once every client frame
			SetNextClientThink(CLIENT_THINK_ALWAYS);
			m_pGasEmitter = NULL;
		}
	}

#endif

void CFFGrenadeGas::Precache()
{
	//DevMsg("[Grenade Debug] CFFGrenadeGas::Precache\n");
	PrecacheModel( GASGRENADE_MODEL );
	PrecacheScriptSound( GAS_SOUND );
	PrecacheScriptSound( "GasGrenade.Open" );
	BaseClass::Precache();
}