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

#define GASGRENADE_MODEL "models/grenades/gas/gas.mdl"
#define GAS_SOUND "GasGrenade.Explode"
#define GAS_EFFECT "GasCloud"

#ifdef CLIENT_DLL
	#define CFFGrenadeGas C_FFGrenadeGas
	#include "c_te_effect_dispatch.h"
	#include "ff_fx_gascloud_emitter.h"
#else
	#include "te_effect_dispatch.h"
	#include "ff_entity_system.h"
#endif

class CFFGrenadeGas : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeGas,CFFGrenadeBase)
	DECLARE_NETWORKCLASS();

	CNetworkVector(m_vInitialVelocity);
	CNetworkVar(int, m_bIsEmitting);

	virtual void Precache();
	virtual float GetShakeAmplitude( void ) { return 0.0f; }	// remove the shake
	virtual float GetGrenadeDamage() { return 0.0f; }
	virtual float GetGrenadeRadius() { return 200.0f; }
	virtual const char *GetBounceSound() { return "GasGrenade.Bounce"; }
	virtual Class_T Classify( void ) { return CLASS_GREN_GAS; }

#ifdef CLIENT_DLL
	CFFGrenadeGas() {}
	CFFGrenadeGas( const CFFGrenadeGas& ) {}

	virtual void ClientThink();
	virtual void OnDataChanged(DataUpdateType_t updateType);

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

LINK_ENTITY_TO_CLASS( gasgrenade, CFFGrenadeGas );
PRECACHE_WEAPON_REGISTER( gasgrenade );

#ifdef GAME_DLL

	int ACT_GAS_IDLE;
	int ACT_GAS_DEPLOY;
	int ACT_GAS_DEPLOY_IDLE;

	void CFFGrenadeGas::Spawn( void )
	{
		//DevMsg("[Grenade Debug] CFFGrenadeGas::Spawn\n");
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
	}

	void CFFGrenadeGas::Explode(trace_t *pTrace, int bitsDamageType)
	{
		//DevMsg("[Grenade Debug] CFFGrenadeGas::Explode\n");
		CFFGrenadeBase::PreExplode( pTrace, GAS_SOUND, GAS_EFFECT );

		// TODO: trigger client side hallucination here
		// TODO: Don't for !FFScriptRunPredicates( this, "onexplode", true ) check

		//CFFGrenadeBase::PostExplode();
	}

	void CFFGrenadeGas::GrenadeThink( void )
	{
		// If the grenade is in a no grenade area, kill it
		if( !FFScriptRunPredicates( this, "onexplode", true ) && ( gpGlobals->curtime > m_flDetonateTime ) )
		{
			// This will remove the gren
			CFFGrenadeBase::PostExplode();
			return;
		}

		// If we're done deploying, deploy idle
		if( m_Activity == ( Activity )ACT_GAS_DEPLOY )
		{
			m_Activity = ( Activity )ACT_GAS_DEPLOY_IDLE;
			m_iSequence = SelectWeightedSequence( m_Activity );
			SetSequence( m_iSequence );

			m_bIsEmitting = 1;
		}

		// Stop the thing from rolling if it starts moving
		if( ( gpGlobals->curtime > m_flOpenTime + 2.0f ) && VPhysicsGetObject() )
		{
			VPhysicsGetObject()->EnableMotion( false );
		}

		// If open and we're not moving and physics aren't enabled, enable physics
		if( ( m_flOpenTime != 0.0f ) && ( GetAbsVelocity() == vec3_origin ) && !VPhysicsGetObject() )
		{
			VPhysicsInitNormal( SOLID_VPHYSICS, GetSolidFlags(), false );
		}

		// Been detonated for 10 secs now, so fade out
		if (gpGlobals->curtime > m_flDetonateTime + 10.0f)
		{
			SetThink(&CBaseGrenade::SUB_FadeOut);
		}		

		// Damage people in here
		if (gpGlobals->curtime > m_flDetonateTime && m_flNextHurt < gpGlobals->curtime)
		{
			m_flNextHurt = gpGlobals->curtime + 0.2f;

			// If we were idling, deploy
			if( m_Activity == ( Activity )ACT_GAS_IDLE )
			{
				// If it's at rest, enable physics
				if( GetAbsVelocity() == vec3_origin )
					VPhysicsInitNormal( SOLID_VPHYSICS, GetSolidFlags(), false );

				m_flOpenTime = gpGlobals->curtime;

				m_Activity = ( Activity )ACT_GAS_DEPLOY;
				m_iSequence = SelectWeightedSequence( m_Activity );
				SetSequence( m_iSequence );

				EmitSound( "GasGrenade.Open" );
			}

			CBaseEntity *pEntity = NULL;
			for( CEntitySphereQuery sphere( GetAbsOrigin(), GetGrenadeRadius() ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
			{
				if( !pEntity )
					continue;

				if( !pEntity->IsPlayer() )
					continue;

				CFFPlayer *pPlayer = ToFFPlayer( pEntity );

				if( !pPlayer || pPlayer->IsObserver() )
					continue;

				if( !g_pGameRules->FPlayerCanTakeDamage( pPlayer, GetOwnerEntity() ) )
					continue;
			
				if( gpGlobals->curtime > pPlayer->m_flLastGassed + 1.0f )
				{
					pPlayer->EmitSound("Player.DrownContinue");	// |-- Mirv: [TODO] Change to something more suitable

					CTakeDamageInfo info(this, GetOwnerEntity(), vec3_origin, GetAbsOrigin(), 1.0f, DMG_DIRECT);
					pPlayer->TakeDamage(info);

					CSingleUserRecipientFilter user((CBasePlayer *)pPlayer);
					user.MakeReliable();

					UserMessageBegin(user, "StatusIconUpdate");
					WRITE_BYTE(FF_ICON_GAS);
					WRITE_FLOAT(15.0);
					MessageEnd();

					// Don't allow this to be accumulative
					pPlayer->m_flLastGassed = gpGlobals->curtime;
				}
			}
		}

		// Animate
		StudioFrameAdvance();

		// Next think straight away
		SetNextThink(gpGlobals->curtime);		

		/*
		// TODO:
		// Let's effervesce underwater
		if( ( GetWaterLevel() != 0 ) && ( m_flOpenTime != 0.0f ) )
		{
			// When motion is disabled GetAbsOrigin() no longer works

			IPhysicsObject *pObject = VPhysicsGetObject();
			if( pObject )
			{
				//Vector vecPos;
				//QAngle vecAng;
				//pObject->GetPosition( &vecPos, &vecAng );
				
				//UTIL_Bubbles( vecPos - Vector( 16, 16, 16 ), vecPos + Vector( 16, 16, 16 ), random->RandomInt( 2, 8 ) );
			}
			else
			{
				UTIL_Bubbles( GetAbsOrigin() - Vector( 16, 16, 16 ), GetAbsOrigin() + Vector( 16, 16, 16 ), random->RandomInt( 2, 8 ) );
			}			
		}
		*/

		// Do underwater grenade movement thinking
		CFFGrenadeBase::WaterThink();		
	}
#else

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