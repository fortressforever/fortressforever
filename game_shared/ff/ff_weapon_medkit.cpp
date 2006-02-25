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
#endif

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
	virtual void	SecondaryAttack();

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
}

void CFFWeaponMedkit::Precache()
{
	PrecacheScriptSound("medkit.hit");
	PrecacheScriptSound("medkit.infect");

	BaseClass::Precache();
}

void CFFWeaponMedkit::SecondaryAttack() 
{
	// heal the player to full if cheats are on
	ConVar *sv_cheats = (ConVar *) ConCommandBase::FindCommand("sv_cheats");
	if (sv_cheats) 
	{
		CFFPlayer *pPlayer = ToFFPlayer(GetOwner());
		pPlayer->SetHealth(pPlayer->GetMaxHealth());
		EmitSound("medkit.hit");

		const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();
		m_flNextPrimaryAttack = gpGlobals->curtime + pWeaponInfo.m_flCycleTime;
		m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
	}
}

//----------------------------------------------------------------------------
// Purpose: Implement impact function
//----------------------------------------------------------------------------
void CFFWeaponMedkit::Hit(trace_t &traceHit, Activity nHitActivity) 
{
	DevMsg("[CFFWeaponMedkit] Hit\n");
#ifdef GAME_DLL
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	if (pHitEntity != NULL && pHitEntity->IsPlayer()) 
	{
		CFFPlayer *pTarget = ToFFPlayer(pHitEntity);
		DevMsg("[medkit] hit other player.. team: %d(mine: %d) \n", pPlayer->GetTeamNumber(), pTarget->GetTeamNumber());

		// check if they are allies
		if (g_pGameRules->PlayerRelationship(pPlayer, pTarget) == GR_TEAMMATE) 
		{
			DevMsg("[medkit] Same Team\n");
			// if they are same team, then cure the player
			pTarget->Cure(pPlayer);
			pTarget->Heal(5);		// |-- Mirv: Heal them by 5hp

			EmitSound("medkit.hit");
			return;
		}
		else if (g_pGameRules->FPlayerCanTakeDamage(pPlayer, pTarget)) 
		{
			// otherwise, if they are bad people, then infect them
			pTarget->Infect(pPlayer);
			DevMsg("[medkit] Infected Player\n");
			return;
		}
	}
#endif

	//BaseClass::Hit(traceHit, nHitActivity);
}
