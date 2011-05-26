/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life2 ========
///
/// @file ff_weapon_knife.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF Knife code & class declaration.
///
/// REVISIONS
/// ---------
/// Jan 19, 2005 Mirv: Initial implementation

#include "cbase.h"
#include "ff_weapon_basemelee.h"
#include "ff_utils.h"
#include "ff_gamerules.h"
#ifdef CLIENT_DLL
	#define CFFWeaponKnife C_FFWeaponKnife
#endif

ConVar ffdev_cloakstab_damage("ffdev_cloakstab_damage", "100", FCVAR_REPLICATED );
ConVar ffdev_cloakstab_disable_modifier( "ffdev_cloakstab_disable_modifier", "1.5", FCVAR_REPLICATED );
ConVar ffdev_cloakstab_disable_min( "ffdev_cloakstab_disable_min", "2", FCVAR_REPLICATED );
ConVar ffdev_knife_disable_duration("ffdev_knife_disable_duration", "0.1", FCVAR_REPLICATED );

#define FF_CLOAKSTAB_DISABLE_MOD ffdev_cloakstab_disable_modifier.GetFloat()
#define FF_CLOAKSTAB_DISABLE_MIN ffdev_cloakstab_disable_min.GetFloat()
#define FF_KNIFE_DISABLE_DURATION ffdev_knife_disable_duration.GetFloat()

//=============================================================================
// CFFWeaponKnife
//=============================================================================

class CFFWeaponKnife : public CFFWeaponMeleeBase
{
public:
	DECLARE_CLASS(CFFWeaponKnife, CFFWeaponMeleeBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CFFWeaponKnife();

	virtual bool Deploy();

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_KNIFE; }

private:
	void Hit(trace_t &traceHit, Activity nHitActivity);

	CFFWeaponKnife(const CFFWeaponKnife &);
};

//=============================================================================
// CFFWeaponKnife tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponKnife, DT_FFWeaponKnife) 

BEGIN_NETWORK_TABLE(CFFWeaponKnife, DT_FFWeaponKnife) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponKnife) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_knife, CFFWeaponKnife);
PRECACHE_WEAPON_REGISTER(ff_weapon_knife);

//=============================================================================
// CFFWeaponKnife implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponKnife::CFFWeaponKnife() 
{
}

//----------------------------------------------------------------------------
// Purpose: Play the unholster sound for a knife
//----------------------------------------------------------------------------
bool CFFWeaponKnife::Deploy() 
{
	WeaponSoundLocal(SPECIAL1);

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
void CFFWeaponKnife::Hit(trace_t &traceHit, Activity nHitActivity) 
{
	//DevMsg("[CFFWeaponKnife] Hit\n");

#ifdef GAME_DLL

	//Get the knife owner 
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	//Get the entity it hits
	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	//Valid target and the spy is cloaked
	if ( pHitEntity != NULL && pPlayer->IsCloaked() ) 
	{
		//Now check the type of pHitEntity
		if( pHitEntity->IsPlayer() )
		{
			CFFPlayer *pTarget = ToFFPlayer(pHitEntity);

			//If the player can be dealt damage via cloakstab -GreenMushy
			if (g_pGameRules->FCanTakeDamage(pPlayer, pTarget)) 
			{
				Vector hitDirection;
				pPlayer->EyeVectors(&hitDirection, NULL, NULL);
				VectorNormalize(hitDirection);

				//Adding cloakstab damage modifiers -GreenMushy
				CTakeDamageInfo info(this, pPlayer,(ffdev_cloakstab_damage.GetFloat() * ( gpGlobals->curtime - pPlayer->GetCloakTime() )), DMG_GENERIC);
				info.SetDamageForce(hitDirection * MELEE_IMPACT_FORCE);
				info.SetCustomKill(KILLTYPE_BACKSTAB);

				//Adding the damage position for demoman shield blocks -GreenMushy
				info.SetDamagePosition( traceHit.startpos );
				info.SetImpactPosition( traceHit.endpos );

				pHitEntity->DispatchTraceAttack(info, hitDirection, &traceHit); 
				ApplyMultiDamage();
			}

			//Play the sounds here regardless of friendlyfire, friend or foe -GreenMushy
			EmitSoundShared( "Player.knife_stab" );
			EmitSoundShared( "Player.knife_discharge" );

			//Dont need basehit
			return;
		}
			
		//Check if it is a buildable object
		if( FF_IsBuildableObject( pHitEntity ) )
		{
			CFFBuildableObject *pBuildable = FF_ToBuildableObject( pHitEntity );

			//Sentrygun
			if( FF_IsSentrygun(pBuildable) )
			{
				//Only Disable if it is an ENEMY sentry gun or you could grief
				if( FFGameRules()->IsTeam1AlliedToTeam2( pPlayer->GetTeamNumber(), pBuildable->GetPlayerOwner()->GetTeamNumber() ) == GR_NOTTEAMMATE )
				{
					pBuildable->Disable( clamp( (gpGlobals->curtime - pPlayer->GetCloakTime()) * FF_CLOAKSTAB_DISABLE_MOD, FF_CLOAKSTAB_DISABLE_MIN, 999.0f));
				}
			}

			//Play the sounds here regardless of friendlyfire, friend or foe -GreenMushy
			EmitSoundShared( "Player.knife_stab" );
			EmitSoundShared( "Player.knife_discharge" );

			//Dont need basehit
			return;
		}
	}

	//Special situation where there is a valid target and the spy is NOT cloaked
	if ( pHitEntity != NULL && pPlayer->IsCloaked() == false ) 
	{
		//Check if it is a buildable
		if( FF_IsBuildableObject( pHitEntity ) )
		{
			CFFBuildableObject *pBuildable = FF_ToBuildableObject( pHitEntity );
			
			//Right now we only have sentrygun mini disables
			//Sentrygun
			if( FF_IsSentrygun( pBuildable ))
			{
				//Only disable if it is an enemy or you could grief
				if( FFGameRules()->IsTeam1AlliedToTeam2( pPlayer->GetTeamNumber(), pBuildable->GetPlayerOwner()->GetTeamNumber() ) == GR_NOTTEAMMATE )
					pBuildable->Disable( FF_KNIFE_DISABLE_DURATION );

				//Dont need basehit
				return;
			}
		}
	}
#endif

#ifdef CLIENT_DLL

	//Get the knife owner
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	//Get the entity it hits
	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	//Valid target and the spy is NOT cloaked
	if (pHitEntity != NULL && pPlayer->IsCloaked() == false ) 
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

	BaseClass::Hit(traceHit, nHitActivity);
}
