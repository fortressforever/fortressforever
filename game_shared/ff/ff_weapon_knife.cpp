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
#ifdef CLIENT_DLL
	#define CFFWeaponKnife C_FFWeaponKnife
#endif

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

ConVar ffdev_knife_cloakstab_damage("ffdev_knife_cloakstab_damage", "100", FCVAR_REPLICATED );

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
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	if (pHitEntity != NULL && pHitEntity->IsPlayer()) 
	{
		CFFPlayer *pTarget = ToFFPlayer(pHitEntity);

		if (g_pGameRules->FCanTakeDamage(pPlayer, pTarget)) 
		{
			if( pPlayer->IsCloaked() )
			{
				//Adding "cloakstab" -GreenMushy

				Vector hitDirection;
				pPlayer->EyeVectors(&hitDirection, NULL, NULL);
				VectorNormalize(hitDirection);

				CTakeDamageInfo info(this, pPlayer,(ffdev_knife_cloakstab_damage.GetFloat() * ( gpGlobals->curtime - pPlayer->GetCloakTime() )), DMG_GENERIC);
				info.SetDamageForce(hitDirection * MELEE_IMPACT_FORCE);
				info.SetCustomKill(KILLTYPE_BACKSTAB);

				pHitEntity->DispatchTraceAttack(info, hitDirection, &traceHit); 
				ApplyMultiDamage();

				return;
			}
		}
	}
#endif

#ifdef CLIENT_DLL
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	if (pHitEntity != NULL && pHitEntity->IsPlayer()) 
	{
		CFFPlayer *pTarget = ToFFPlayer(pHitEntity);

		if (g_pGameRules->FCanTakeDamage(pPlayer, pTarget)) 
		{
			// we scored a hit, so play the knife slash sound
			// adding a "cloakstab" noise -GreenMushy
			if( pPlayer->IsCloaked())
			{
				EmitSoundShared( "Player.knife_stab" );
				EmitSoundShared( "Player.knife_discharge" );
			}
			else
			{
				WeaponSound(SPECIAL2);
			}
		}
	}
#endif

	BaseClass::Hit(traceHit, nHitActivity);
}
