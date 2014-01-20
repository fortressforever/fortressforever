/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life2 ========
///
/// @file ff_weapon_medkit.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF Medkit code & class declaration.
///
/// REVISIONS
/// ---------
/// Jan 19, 2005 Mirv: Initial implementation


#include "cbase.h"
#include "ff_weapon_basemelee.h"

#ifdef CLIENT_DLL
	#define CFFWeaponMedkit C_FFWeaponMedkit
#else
	#include "omnibot_interface.h"
#endif

#include "ff_shareddefs.h"
#include "ff_utils.h"

//=============================================================================
// CFFWeaponMedkit
//=============================================================================

class CFFWeaponMedkit : public CFFWeaponMeleeBase
{
public:
	DECLARE_CLASS(CFFWeaponMedkit, CFFWeaponMeleeBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CFFWeaponMedkit();

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_MEDKIT; }

	virtual void	Precache();
	virtual void	WeaponSound(WeaponSound_t sound_type, float soundtime = 0.0f);


private:
	void			Hit(trace_t &traceHit, Activity nHitActivity);
	CFFWeaponMedkit(const CFFWeaponMedkit &);
};

//=============================================================================
// CFFWeaponMedkit tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponMedkit, DT_FFWeaponMedkit) 

BEGIN_NETWORK_TABLE(CFFWeaponMedkit, DT_FFWeaponMedkit) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponMedkit) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_medkit, CFFWeaponMedkit);
PRECACHE_WEAPON_REGISTER(ff_weapon_medkit);

//=============================================================================
// CFFWeaponMedkit implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponMedkit::CFFWeaponMedkit() 
{
	m_flNextSecondaryAttack = 0;
}

void CFFWeaponMedkit::Precache()
{
	PrecacheScriptSound("medkit.hit");
	PrecacheScriptSound("medkit.infect");

	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Implement impact function
//----------------------------------------------------------------------------
void CFFWeaponMedkit::Hit(trace_t &traceHit, Activity nHitActivity) 
{
	//DevMsg("[CFFWeaponMedkit] Hit\n");

	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	if (pHitEntity != NULL && pHitEntity->IsPlayer()) 
	{
		CFFPlayer *pTarget = ToFFPlayer(pHitEntity);
		//DevMsg("[medkit] hit other player.. team: %d(mine: %d) \n", pPlayer->GetTeamNumber(), pTarget->GetTeamNumber());

		// check if they are allies
		if (g_pGameRules->PlayerRelationship(pPlayer, pTarget) == GR_TEAMMATE) 
		{
			//DevMsg("[medkit] Same Team\n");
			// if they are same team, then cure the player

#ifdef GAME_DLL
			const int iBeforeHealth = pPlayer->GetHealth();

			pTarget->Cure(pPlayer);
			pTarget->Heal(pPlayer, 5);		// |-- Mirv: Heal them by 5hp

			const int iAfterHealth = pPlayer->GetHealth();
			Omnibot::Notify_GotMedicHealth(pTarget,pPlayer,iBeforeHealth,iAfterHealth);
			Omnibot::Notify_GaveMedicHealth(pPlayer,pTarget,iBeforeHealth,iAfterHealth);
#endif

			// Heal sound. Add a delay before next sound can be played too
			if (m_flNextSecondaryAttack <= gpGlobals->curtime)
			{
				WeaponSound(SPECIAL1);
				m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;
			}

			return;
		}
		else
		{
			// Everyone takes damage from the medikit hitting them...			
			Vector hitDirection;
			pPlayer->EyeVectors(&hitDirection, NULL, NULL);
			VectorNormalize(hitDirection);

			int bitsDamageType = DMG_CLUB;
			if (FF_IsAirshot( pHitEntity ))
				bitsDamageType |= DMG_AIRSHOT;

			CTakeDamageInfo info(this, GetOwner(), GetFFWpnData().m_iDamage, bitsDamageType);
			info.SetDamageForce(hitDirection * MELEE_IMPACT_FORCE);

			pHitEntity->DispatchTraceAttack(info, hitDirection, &traceHit); 
			ApplyMultiDamage();

			WeaponSound_t wpnSound = SINGLE;

			// Bug #0000510: Medics can infect medics.
			if( pTarget->GetClassSlot() != CLASS_MEDIC )
			{
#ifdef GAME_DLL
				// otherwise, if they are bad people, then infect them
				pTarget->Infect(pPlayer);
#endif
				wpnSound = SPECIAL2;
			}

			// Infect sound. Add a sound before next sound can be played too
			if (m_flNextSecondaryAttack <= gpGlobals->curtime)
			{
				WeaponSound(wpnSound);
				m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;
			}
			
			return;
		}
	}
}

//----------------------------------------------------------------------------
// Purpose: Implement weapon sound delays so that infect + heal can be played fully
//----------------------------------------------------------------------------
void CFFWeaponMedkit::WeaponSound(WeaponSound_t sound_type, float soundtime /* = 0.0f */)
{
	if (m_flNextSecondaryAttack > gpGlobals->curtime)
	{
		//DevMsg("Ignored");
		return;
	}
	
	BaseClass::WeaponSound(sound_type, soundtime);
}