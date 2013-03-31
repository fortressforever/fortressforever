/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life2 ========

#include "cbase.h"
#include "ff_utils.h"
#include "ff_weapon_electricknife.h"

#ifdef CLIENT_DLL
	#define CFFWeaponElectricKnife C_FFWeaponElectricKnife
#endif

ConVar ffdev_cloakstab_damage("ffdev_cloakstab_damage", "200", FCVAR_REPLICATED );
#define FFDEV_CLOAKSTAB_DAMAGE ffdev_cloakstab_damage.GetFloat()
ConVar ffdev_cloakstab_damage_buildables_max("ffdev_cloakstab_damage_buildables_max", "130", FCVAR_REPLICATED );
#define FFDEV_CLOAKSTAB_DAMAGE_BUILDABLES_MAX ffdev_cloakstab_damage_buildables_max.GetFloat()
ConVar ffdev_cloakstab_damage_buildables_min("ffdev_cloakstab_damage_buildables_min", "20", FCVAR_REPLICATED );
#define FFDEV_CLOAKSTAB_DAMAGE_BUILDABLES_MIN ffdev_cloakstab_damage_buildables_min.GetFloat() // FFDEV_ELECTRICKNIFE?
ConVar ffdev_cloakstab_disable_maxtime( "ffdev_cloakstab_disable_maxtime", "2.5", FCVAR_REPLICATED );
#define FFDEV_CLOAKSTAB_DISABLE_MAXTIME ffdev_cloakstab_disable_maxtime.GetFloat()
ConVar ffdev_cloakstab_disable_mintime( "ffdev_cloakstab_disable_mintime", "0.5", FCVAR_REPLICATED );
#define FFDEV_CLOAKSTAB_DISABLE_MINTIME ffdev_cloakstab_disable_mintime.GetFloat()
ConVar ffdev_cloakstab_maxpushforce("ffdev_cloakstab_maxpushforce", "20", FCVAR_REPLICATED );
#define FFDEV_CLOAKSTAB_MAXPUSHFORCE ffdev_cloakstab_maxpushforce.GetFloat()
ConVar ffdev_cloakstab_minpushforce("ffdev_cloakstab_minpushforce", "5", FCVAR_REPLICATED );
#define FFDEV_CLOAKSTAB_MINPUSHFORCE ffdev_cloakstab_minpushforce.GetFloat()
ConVar ffdev_cloakstab_pushforce_up("ffdev_cloakstab_pushforce_up", "0.8", FCVAR_REPLICATED );
#define FFDEV_CLOAKSTAB_PUSHFORCE_UP ffdev_cloakstab_pushforce_up.GetFloat()


ConVar ffdev_electricknife_uncharged_disable_duration("ffdev_electricknife_uncharged_disable_duration", "0.1", FCVAR_REPLICATED );
#define FFDEV_ELECTRICKNIFE_UNCHARGED_DISABLE_DURATION ffdev_electricknife_uncharged_disable_duration.GetFloat()
ConVar ffdev_electricknife_cooldown_time("ffdev_electricknife_cooldown_time", "7", FCVAR_REPLICATED, "Time in seconds you have to wait until you can electrify again" );
#define FFDEV_ELECTRICKNIFE_COOLDOWN_TIME ffdev_electricknife_cooldown_time.GetFloat()
ConVar ffdev_spy_nextcloak( "ffdev_cloakcooldown", "7.0", FCVAR_REPLICATED, "Time in seconds you have to wait until you can cloak again" );
ConVar ffdev_electricknife_electrify_duration("ffdev_electricknife_electrify_duration", "3", FCVAR_REPLICATED ); // Time the electrify lasts when activated
#define FFDEV_ELECTRICKNIFE_ELECTRIFY_DURATION ffdev_electricknife_electrify_duration.GetFloat()
ConVar ffdev_electricknife_speedboost( "ffdev_electricknife_speedboost", "1.5", FCVAR_REPLICATED );
#define FFDEV_ELECTRICKNIFE_SPEEDBOOST ffdev_electricknife_speedboost.GetFloat()

ConVar ffdev_electricknife_damage("ffdev_electricknife_damage", "60", FCVAR_REPLICATED );
#define FFDEV_ELECTRICKNIFE_DAMAGE ffdev_electricknife_damage.GetFloat()

//=============================================================================
// CFFWeaponElectricKnife tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponElectricKnife, DT_FFWeaponElectricKnife) 

BEGIN_NETWORK_TABLE(CFFWeaponElectricKnife, DT_FFWeaponElectricKnife) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponElectricKnife) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_electricknife, CFFWeaponElectricKnife);
PRECACHE_WEAPON_REGISTER(ff_weapon_electricknife);

//=============================================================================
// CFFWeaponElectricKnife implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponElectricKnife::CFFWeaponElectricKnife() 
{
	m_flStartElectrifyTime = 0.0f;
	m_flStartRechargeTime = 0.0f;
	pOwner = ToFFPlayer(GetOwner()); // TODO: Check this actually gets the owner player and doesnt do something dumb like this doesnt work during construction
}

//----------------------------------------------------------------------------
// Purpose: Play the unholster sound for a knife
//----------------------------------------------------------------------------
bool CFFWeaponElectricKnife::Deploy() 
{
	WeaponSoundLocal(SPECIAL1);
	pOwner = ToFFPlayer(GetOwner()); // TODO: Check this actually gets the owner player and doesnt do something dumb like this doesnt work during construction

	// A quick hint test
#ifdef CLIENT_DLL
	//FF_HudHint(HINT_GENERAL, 54, "Hi there. You seem to have drawn a knife. The knife can backstab and stuff like that - it's awesome!");
	//Msg( "hudhints sayz: %d\n", hudhints.GetInt() );
	
	FF_SendHint( SPY_KNIFE, 1, PRIORITY_LOW, "#FF_HINT_SPY_KNIFE" );
#endif
	
	return BaseClass::Deploy();
}

//----------------------------------------------------------------------------
// Purpose: Implement impact function
//----------------------------------------------------------------------------
void CFFWeaponElectricKnife::Hit(trace_t &traceHit, Activity nHitActivity) 
{	
	// Do we want to discharge the knife when we swing here?

	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	if ( pHitEntity == NULL ) // Didn't hit an entity, might have hit walls/floors etc
	{
		BaseClass::Hit(traceHit, nHitActivity); // Draw stab marks on walls, play generic melee sounds etc
		return;
	}

	if ( !IsHitEntitySpecialCase(pHitEntity) ) // Hit an entity we don't care about, e.g. jump pads detpacks 
	{
		// This will deal base melee damage as specified in the weapon script,
		// take care that this may be different to FFDEV_CLOAKSTAB_DAMAGE_BUILDABLES_MIN etc
		BaseClass::Hit(traceHit, nHitActivity);  
		return;
	}

	//#ifdef GAME_DLL
	if ( IsElectrified() ) 
	{
		ChargedStabEntity(traceHit, pHitEntity);
	}
	else
	{
		UnchargedStabEntity(traceHit, pHitEntity);
	}
	//#endif

	#ifdef CLIENT_DLL
	//Valid target and the spy is NOT cloaked
	if (IsElectrified() == false ) 
	{
		//Non-cloak stabs make slash noises on enemy players
		//Use the basehit to make noises for other things like sentryguns -GreenMushy
		if( pHitEntity->IsPlayer() )
		{
			WeaponSound(SPECIAL2);
			return;
		}
	}
	#endif
}

bool CFFWeaponElectricKnife::IsHitEntitySpecialCase(CBaseEntity *pHitEntity)
{
	if( pHitEntity->IsPlayer() )
		return true;
	if( FF_IsSentrygun(pHitEntity) )
		return true;
	if( FF_IsDispenser( pHitEntity ) )
		return true;
	
	return false;
}

void CFFWeaponElectricKnife::ChargedStabPlayer(trace_t &traceHit, CFFPlayer *pTarget)
{
	//EmitSoundShared( "Player.knife_stab" ); //Play the sounds here regardless of friendlyfire, friend or foe -GreenMushy
	EmitSoundShared( "electricknife.discharge" );

	if (!g_pGameRules->FCanTakeDamage(pOwner, pTarget)) 
	{
		return;
	}

	Vector hitDirection;
	pOwner->EyeVectors(&hitDirection, NULL, NULL);
	VectorNormalize(hitDirection);
	CTakeDamageInfo info(this, pOwner, GetChargedPlayerDamage(), DMG_GENERIC);
	info.SetCustomKill(KILLTYPE_BACKSTAB);

	info.SetDamageForce( GetChargedPlayerForce(hitDirection) ); // Push players further
	pTarget->DispatchTraceAttack(info, hitDirection, &traceHit); 
	ApplyMultiDamage();
}

void CFFWeaponElectricKnife::ChargedStabSentryGun(trace_t &traceHit, CFFSentryGun *pSentryGun)
{
	FF_DevMsg("");
	DevMsg("ChargedStabEntity! Sentry gun. Electrify percent was %f\n", GetElectrifyPercent());
	if( FFGameRules()->IsTeam1AlliedToTeam2( pOwner->GetTeamNumber(), pSentryGun->GetPlayerOwner()->GetTeamNumber() ) != GR_NOTTEAMMATE )
	{
		return;
	}

#ifdef GAME_DLL
	pSentryGun->Disable( GetChargedSentryGunDisableTime() );
#endif

	Vector hitDirection;
	pOwner->EyeVectors(&hitDirection, NULL, NULL);
	VectorNormalize(hitDirection);
	CTakeDamageInfo info(this, pOwner, GetChargedSentryGunDamage(), DMG_GENERIC);
	info.SetCustomKill(KILLTYPE_BACKSTAB);

	pSentryGun->DispatchTraceAttack(info, hitDirection, &traceHit); 
	ApplyMultiDamage();
}

void CFFWeaponElectricKnife::ChargedStabEntity(trace_t &traceHit, CBaseEntity *pHitEntity)
{
	FF_DevMsg("");
	DevMsg("ChargedStabEntity! Electrify percent was %f\n", GetElectrifyPercent());
	if( pHitEntity->IsPlayer() )
	{
		ChargedStabPlayer(traceHit, ToFFPlayer(pHitEntity) );
	}
	else if( FF_IsBuildableObject( pHitEntity ) )
	{
		FF_DevMsg("");
		DevMsg("ChargedStabEntity! Buildable. Electrify percent was %f\n", GetElectrifyPercent());
		CFFBuildableObject *pBuildable = FF_ToBuildableObject( pHitEntity );

		if( FF_IsSentrygun(pBuildable) )
		{
			ChargedStabSentryGun(traceHit, (CFFSentryGun *) pBuildable);
		}
		else if( FF_IsDispenser( pBuildable ) )
		{
			#ifdef GAME_DLL
			//if( FFGameRules()->IsTeam1AlliedToTeam2( pOwner->GetTeamNumber(), pBuildable->GetPlayerOwner()->GetTeamNumber() ) == GR_NOTTEAMMATE )
			//{
				CFFDispenser *pDispenser = FF_ToDispenser( pBuildable );
				if( pDispenser ) // not sure why dispensers are using different damage code, TakeDamage rather than DispatchTraceAttack + ApplyMultiDamage? DispatchTraceAttack does the clientside tracers i think
					pDispenser->TakeDamage( CTakeDamageInfo( this, pOwner, Lerp( GetElectrifyPercent(), FFDEV_CLOAKSTAB_DAMAGE_BUILDABLES_MIN, FFDEV_CLOAKSTAB_DAMAGE_BUILDABLES_MAX), DMG_GENERIC ) );
			//}
			#endif
		}

		//Play the sounds here regardless of friendlyfire, friend or foe -GreenMushy
		//EmitSoundShared( "Player.knife_stab" );
		EmitSoundShared( "electricknife.discharge" );
	}
}

void CFFWeaponElectricKnife::UnchargedStabEntity(trace_t &traceHit, CBaseEntity *pHitEntity)
{
	CFFBuildableObject *pBuildable = FF_ToBuildableObject( pHitEntity );
	if( pBuildable == NULL )
	{
		return;
	}
	
	if( FF_IsSentrygun( pBuildable ))
	{
		if( FFGameRules()->IsTeam1AlliedToTeam2( pOwner->GetTeamNumber(), pBuildable->GetPlayerOwner()->GetTeamNumber() ) != GR_NOTTEAMMATE )
		{
			return;
		}

#ifdef GAME_DLL
		CFFSentryGun *pSentryGun = (CFFSentryGun *) pBuildable;
		if ( FFDEV_ELECTRICKNIFE_UNCHARGED_DISABLE_DURATION > 0 )
			pSentryGun->Disable( FFDEV_ELECTRICKNIFE_UNCHARGED_DISABLE_DURATION );

		if( pSentryGun )
			pSentryGun->TakeDamage( CTakeDamageInfo( this, pOwner, FFDEV_CLOAKSTAB_DAMAGE_BUILDABLES_MIN , DMG_GENERIC ) );
#endif // Any reason we cant client predict the SG taking damage here? Use DispatchTraceAttack?
	}
}

void CFFWeaponElectricKnife::TryStartCharging()
{
	FF_DevMsg("Trying to start charging!\n");

	if (IsElectrified())
	{
		FF_DevMsg("Still electrified, so wont charge.\n");
		return;
	}
	if (!IsRecharged())
	{
		FF_DevMsg("Not recharged, so wont charge.\n");
		return;
	}

	FF_DevMsg("Charging!\n");
	m_flStartElectrifyTime = gpGlobals->curtime;

	EmitSoundShared( "electricknife.charge" );
#ifdef GAME_DLL // Speed effects on yourself aren't predicted, see RecalculateSpeed() 
	pOwner->AddSpeedEffect( SE_ELECTRICKNIFE, FFDEV_ELECTRICKNIFE_ELECTRIFY_DURATION, FFDEV_ELECTRICKNIFE_SPEEDBOOST, SEM_BOOLEAN );
#endif
}

bool CFFWeaponElectricKnife::IsRecharged()
{
	return GetCooldownPercent() == 1.0f;
}

float CFFWeaponElectricKnife::GetCooldownPercent()
{
	if (IsElectrified())
		return 0.0f;

	if (FFDEV_ELECTRICKNIFE_COOLDOWN_TIME <= 0.0f)
		return 1.0f;

	float rechargePercent = (gpGlobals->curtime - m_flStartRechargeTime) / FFDEV_ELECTRICKNIFE_COOLDOWN_TIME;

	if (rechargePercent > 1.0f)
		return 1.0f;

	return rechargePercent;
}

float CFFWeaponElectricKnife::GetElectrifyPercent()
{
	if (IsElectrified() == false)
		return 0.0f;

	if (FFDEV_ELECTRICKNIFE_ELECTRIFY_DURATION <= 0.0f)
		return 0.0f;

	float electrifyPercent = (gpGlobals->curtime - m_flStartElectrifyTime) / FFDEV_ELECTRICKNIFE_ELECTRIFY_DURATION;

	return electrifyPercent;
}

//----------------------------------------------------------------------------
// Purpose: When holstered we need to stop any sounds + remove speed effects
//----------------------------------------------------------------------------
bool CFFWeaponElectricKnife::Holster(CBaseCombatWeapon *pSwitchingTo) 
{
	FF_DevMsg("");
	DevMsg("Holster.\n");
	CancelElectrify();
	return BaseClass::Holster(pSwitchingTo);
}

void CFFWeaponElectricKnife::PrimaryAttack()
{
	FF_DevMsg("");
	DevMsg("Primary attack - Cancelling electrify! Electrify percent was %f\n", GetElectrifyPercent());
	BaseClass::PrimaryAttack(); // hit stuff first
	CancelElectrify(); // Then reset the charge
}

void CFFWeaponElectricKnife::ItemPostFrame()
{
	if (GetElectrifyPercent() > 1.0f)
	{
		FF_DevMsg("");
		DevMsg("Postframe - electrify over 100%, cancelling.\n");
		CancelElectrify();
	}

	BaseClass::ItemPostFrame();
}

void CFFWeaponElectricKnife::CancelElectrify()
{
	FF_DevMsg("");
	DevMsg("Cancelling electrify! Electrify percent was %f\n", GetElectrifyPercent());
	
	if (!IsElectrified())
		return;

	m_flStartElectrifyTime = 0.0f;
	m_flStartRechargeTime = gpGlobals->curtime;

	EmitSoundShared( "Player.Cloak_End" );
	StopSound( "electricknife.charge" );
	//StopSound( "Player.Cloak" );

#ifdef GAME_DLL // Should this also be clientside for predicted immediate speed removal?

		//Removes the cloak speed effect -GreenMushy
		pOwner->RemoveSpeedEffect( SE_ELECTRICKNIFE );
		
		// Fire an event.
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "uncloaked" ); //TODO: new event type
		if( pEvent )
		{
			pEvent->SetInt( "userid", pOwner->GetUserID() );
			gameeventmanager->FireEvent( pEvent, true );
		}
#endif
}

bool CFFWeaponElectricKnife::IsElectrified()
{
	//DevMsg("IsElectrified, m_flStartElectrifyTime : %f\n", m_flStartElectrifyTime);
	if (m_flStartElectrifyTime == 0.0f)
	{
		return false;
	}

	return true;
}

float CFFWeaponElectricKnife::GetChargedPlayerDamage()
{
	return Lerp( GetElectrifyPercent(), FFDEV_ELECTRICKNIFE_DAMAGE, FFDEV_CLOAKSTAB_DAMAGE);
}

Vector CFFWeaponElectricKnife::GetChargedPlayerForce(Vector hitDirection)
{
	return (hitDirection + Vector(0, 0, FFDEV_CLOAKSTAB_PUSHFORCE_UP)) * MELEE_IMPACT_FORCE * Lerp( GetElectrifyPercent(), FFDEV_CLOAKSTAB_MINPUSHFORCE, FFDEV_CLOAKSTAB_MAXPUSHFORCE);
}

float CFFWeaponElectricKnife::GetChargedSentryGunDamage()
{
	return Lerp( GetElectrifyPercent(), FFDEV_CLOAKSTAB_DAMAGE_BUILDABLES_MIN, FFDEV_CLOAKSTAB_DAMAGE_BUILDABLES_MAX);
}

float CFFWeaponElectricKnife::GetChargedSentryGunDisableTime()
{
	return Lerp( GetElectrifyPercent(), FFDEV_CLOAKSTAB_DISABLE_MINTIME, FFDEV_CLOAKSTAB_DISABLE_MAXTIME);
}
