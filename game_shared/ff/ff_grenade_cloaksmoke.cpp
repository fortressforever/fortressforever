/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file ff_grenade_CloakSmoke.cpp
/// @author Shawn Smith (L0ki)
/// @date Apr. 23, 2005
/// @brief CloakSmoke grenade class
/// 
/// Implementation of the CFFGrenadeCloakSmoke class. This is the secondary grenade type for engineer
/// 
/// Revisions
/// ---------
/// Apr. 23, 2005	L0ki: Initial creation

#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"

#ifdef CLIENT_DLL
	#define CFFGrenadeCloakSmoke C_FFGrenadeCloakSmoke

	#include "c_te_effect_dispatch.h"
	#include "ff_fx_cloaksmoke_emitter.h"
	#include <igameevents.h>
#else
	#include "te_effect_dispatch.h"
	#include "ff_entity_system.h"
	#include "ai_basenpc.h"
	#include "ff_player.h"
#endif

#define CLOAKSMOKE_MODEL	"models/grenades/gas/gas.mdl"
#define CLOAKSMOKE_SOUND	"GasGrenade.Explode"
#define CLOAKSMOKE_EFFECT	"GasCloud"

ConVar ffdev_cloaksmoke_gren_lifetime("ffdev_cloaksmoke_gren_lifetime", "10.0f", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define FFDEV_CLOAKSMOKE_GREN_LIFETIME ffdev_cloaksmoke_gren_lifetime.GetFloat()

ConVar ffdev_cloaksmoke_radius_outer("ffdev_cloaksmoke_radius_outer", "256.0", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define FFDEV_CLOAKSMOKE_RADIUS_OUTER ffdev_cloaksmoke_radius_outer.GetFloat()

ConVar ffdev_cloaksmoke_radius_inner("ffdev_cloaksmoke_radius_inner", "192.0", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define FFDEV_CLOAKSMOKE_RADIUS_INNER ffdev_cloaksmoke_radius_inner.GetFloat()

class CFFGrenadeCloakSmoke : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeCloakSmoke,CFFGrenadeBase);
	DECLARE_NETWORKCLASS();

	// Server spits out a complaint about this not being unsigned
	CNetworkVar( unsigned int, m_bIsEmitting );

	virtual void Precache();
	virtual float GetShakeAmplitude( void ) { return 0.0f; }	// remove the shake
	virtual float GetGrenadeDamage() { return 0.0f; }
	virtual float GetGrenadeRadius() { return FFDEV_CLOAKSMOKE_RADIUS_OUTER; }
	virtual const char *GetBounceSound() { return "GasGrenade.Bounce"; }
	virtual Class_T Classify( void ) { return CLASS_GREN_CLOAKSMOKE; }

	virtual color32 GetColour() { color32 col = { 255, 255, 255, GREN_ALPHA_DEFAULT }; return col; }

#ifdef CLIENT_DLL
	CFFGrenadeCloakSmoke() {}
	CFFGrenadeCloakSmoke( const CFFGrenadeCloakSmoke& ) {}

	virtual void ClientThink();
	virtual void OnDataChanged(DataUpdateType_t updateType);
	virtual void UpdateOnRemove( void );

	CSmartPtr<CCloakSmokeCloud>	m_pSmokeEmitter;
#else
	virtual void Spawn();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);
	virtual void GrenadeThink();

protected:
	int m_iSequence;
	Activity m_Activity;

	float m_flOpenTime;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeCloakSmoke, DT_FFGrenadeCloakSmoke)

BEGIN_NETWORK_TABLE(CFFGrenadeCloakSmoke, DT_FFGrenadeCloakSmoke)
#ifdef GAME_DLL
SendPropInt(SENDINFO(m_bIsEmitting), 1, SPROP_UNSIGNED),
#else
RecvPropInt(RECVINFO(m_bIsEmitting)),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( ff_grenade_CloakSmoke, CFFGrenadeCloakSmoke );
PRECACHE_WEAPON_REGISTER( ff_grenade_CloakSmoke );

#ifdef GAME_DLL

	extern Activity ACT_GAS_IDLE;
	extern Activity ACT_GAS_DEPLOY;
	extern Activity ACT_GAS_DEPLOY_IDLE;

	//-----------------------------------------------------------------------------
	// Purpose: Various flags, models
	//-----------------------------------------------------------------------------
	void CFFGrenadeCloakSmoke::Spawn( void )
	{
		SetModel( CLOAKSMOKE_MODEL );
		BaseClass::Spawn();	

		ADD_CUSTOM_ACTIVITY( CFFGrenadeCloakSmoke, ACT_GAS_IDLE );
		ADD_CUSTOM_ACTIVITY( CFFGrenadeCloakSmoke, ACT_GAS_DEPLOY );
		ADD_CUSTOM_ACTIVITY( CFFGrenadeCloakSmoke, ACT_GAS_DEPLOY_IDLE );

		m_Activity = ( Activity )ACT_GAS_IDLE;
		m_iSequence = SelectWeightedSequence( m_Activity );
		SetSequence( m_iSequence );		

		m_flOpenTime = 0.0f;
		m_bIsEmitting = 0;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Emit a ring that blows things up
	//-----------------------------------------------------------------------------
	void CFFGrenadeCloakSmoke::Explode(trace_t *pTrace, int bitsDamageType)
	{
		UTIL_Remove(this);
	}

	//----------------------------------------------------------------------------
	// Purpose: Fire explosion sound early
	//----------------------------------------------------------------------------
	void CFFGrenadeCloakSmoke::GrenadeThink( void )
	{
		// If we're done deploying, deploy idle
		if( m_Activity == ACT_GAS_DEPLOY )
		{
			m_Activity = ACT_GAS_DEPLOY_IDLE;
			m_iSequence = SelectWeightedSequence( m_Activity );
			SetSequence( m_iSequence );

			m_bIsEmitting = 1;
		}

		if (gpGlobals->curtime > m_flDetonateTime + FFDEV_CLOAKSMOKE_GREN_LIFETIME)
		{
			SetThink(&CBaseGrenade::SUB_FadeOut);
		}

		if ( gpGlobals->curtime > m_flDetonateTime )
		{
			// If we were idling, deploy
			if( m_Activity == ACT_GAS_IDLE )
			{
				m_flOpenTime = gpGlobals->curtime;

				m_Activity = ACT_GAS_DEPLOY;
				m_iSequence = SelectWeightedSequence( m_Activity );
				SetSequence( m_iSequence );

				EmitSound(CLOAKSMOKE_SOUND);
			}

			CBaseEntity *pEntity = NULL;
			for( CEntitySphereQuery sphere( GetAbsOrigin(), FFDEV_CLOAKSMOKE_RADIUS_OUTER ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
			{
				if( !pEntity )
					continue;

				if( !pEntity->IsPlayer() )
					continue;

				CFFPlayer *pPlayer = ToFFPlayer( pEntity );

				if( !pPlayer || pPlayer->IsObserver() )
					continue;

				if( pPlayer->GetTeamNumber() != ToFFPlayer(GetOwnerEntity())->GetTeamNumber() )
					continue;

				//Get the player's distance from the gren origin
				Vector vDisplacement = ( pPlayer->GetAbsOrigin() - GetAbsOrigin() );

				//figure the length
				float fDist = vDisplacement.Length();

				//If the player is out
				if( fDist <= FFDEV_CLOAKSMOKE_RADIUS_INNER )
				{
					pPlayer->CloakSmoke();
				}
				else
				{
					pPlayer->RemoveCloakSmoke();
				}
			}
		}

		// Animate
		StudioFrameAdvance();

		// Next think straight away
		SetNextThink(gpGlobals->curtime);
	}
#else

	//-----------------------------------------------------------------------------
	// Purpose: Stop gas emitter
	//-----------------------------------------------------------------------------
	void CFFGrenadeCloakSmoke::UpdateOnRemove( void )
	{
		if( m_bIsEmitting )
		{
			if( !!m_pSmokeEmitter )
			{
				m_pSmokeEmitter->SetDieTime( 0.0f );
				m_pSmokeEmitter = NULL;
			}
		}

		BaseClass::UpdateOnRemove();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Emit gas.
	//-----------------------------------------------------------------------------
	void CFFGrenadeCloakSmoke::ClientThink()
	{
		if (m_bIsEmitting)
		{
			if (!m_pSmokeEmitter)
			{
				// Specify the emitter and how long it should emit particles
				m_pSmokeEmitter = CCloakSmokeCloud::Create("GasCloud");
				m_pSmokeEmitter->SetDieTime(gpGlobals->curtime + FFDEV_CLOAKSMOKE_GREN_LIFETIME);
			}

			if (!!m_pSmokeEmitter)
			{
				m_pSmokeEmitter->UpdateEmitter(GetAbsOrigin(), GetAbsVelocity());
			}
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Called when data changes on the server
	//-----------------------------------------------------------------------------
	void CFFGrenadeCloakSmoke::OnDataChanged(DataUpdateType_t updateType)
	{
		// NOTE: We MUST call the base classes' implementation of this function
		BaseClass::OnDataChanged(updateType);

		// Setup our entity's particle system on creation
		if (updateType == DATA_UPDATE_CREATED)
		{
			// Call our ClientThink() function once every client frame
			SetNextClientThink(CLIENT_THINK_ALWAYS);
			m_pSmokeEmitter = NULL;
		}
	}

#endif

void CFFGrenadeCloakSmoke::Precache()
{
	PrecacheModel( CLOAKSMOKE_MODEL );
	PrecacheScriptSound( CLOAKSMOKE_SOUND );
	PrecacheScriptSound( "GasGrenade.Open" );
	BaseClass::Precache();
}