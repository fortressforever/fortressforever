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
#include "ff_shareddefs.h"

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

//ConVar ffdev_knife_backstab_angle("ffdev_knife_backstab_angle", "0.643", FCVAR_FF_FFDEV_REPLICATED);
#define KNIFE_BACKSTAB_ANGLE 0.643

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
			// check to see if we got a backstab

			// get the displacement between the players
			Vector vDisplacement = pTarget->GetAbsOrigin() - pPlayer->GetAbsOrigin();
			vDisplacement.z = 0;
			vDisplacement.NormalizeInPlace();
	
			// get the direction the target is facing
			Vector vFacing;
			AngleVectors(pTarget->GetLocalAngles(), &vFacing);
			vFacing.z = 0;
			vFacing.NormalizeInPlace();

			// see if they are facing the same direction
			float angle = vFacing.Dot(vDisplacement);
			if (angle > KNIFE_BACKSTAB_ANGLE) // cos(45deg) 
			{
				//DevMsg("BACKSTAB!!!!!\n");
				// we get to totally kerplown this guy

				// Mulch: armor doesn't protect against DMG_DIRECT
				Vector hitDirection;
				pPlayer->EyeVectors(&hitDirection, NULL, NULL);
				VectorNormalize(hitDirection);

				int bitsDamageType = DMG_DIRECT;
				if (FF_IsAirshot(pHitEntity))
					bitsDamageType |= DMG_AIRSHOT;

				CTakeDamageInfo info(this, pPlayer, 108, bitsDamageType);
				info.SetDamageForce(hitDirection * MELEE_IMPACT_FORCE);
				info.SetCustomKill(KILLTYPE_BACKSTAB);

				pHitEntity->DispatchTraceAttack(info, hitDirection, &traceHit); 
				ApplyMultiDamage();

				// Is the guy dead? If so then take his clothes because we are cool
				if (pHitEntity->IsPlayer() && !pHitEntity->IsAlive() && pPlayer->IsDisguised())
				{
					CFFPlayer *pVictim = ToFFPlayer(pHitEntity);
					pPlayer->SetDisguise(pVictim->GetTeamNumber(), pVictim->GetClassSlot(), true);

#ifndef CLIENT_DLL
				
					FF_SendHint( pPlayer, SPY_GANKDISGUISE, -1, PRIORITY_NORMAL, "#FF_HINT_SPY_GANKDISGUISE" );
#endif

					CBaseEntity *pRagdoll = pVictim->m_hRagdoll.Get();

					// Ragdoll should die very soon
					if (pRagdoll)
					{
						pRagdoll->SetNextThink(gpGlobals->curtime + 2.5f);
					}
				}
				else
				{
					// He's not dead, so lose our disguise as normal
					pPlayer->ResetDisguise();
				}

				// we don't need to call BaseClass since we already did damage.
				return;
			}

			//DevMsg("Test Backstab: (%.2f, %.2f) dot(%.2f, %.2f) = %.2f\n", vDisplacement.x, vDisplacement.y, vFacing.x, vFacing.y, angle);

		}
	}

	// It wasn't a backstab (which returned early), so remove any disguise here
	pPlayer->ResetDisguise();

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
			WeaponSound(SPECIAL2);
		}
	}
#endif

	BaseClass::Hit(traceHit, nHitActivity);
}
