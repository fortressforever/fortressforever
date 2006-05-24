/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_flamethrower.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date 09 March 2005
/// @brief The FF flamethrower code & class declaration
///
/// REVISIONS
/// ---------
/// Mar 09, 2005 Mirv: First logged


#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"

// For CFlameJet
#include "baseparticleentity.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponFlamethrower C_FFWeaponFlamethrower
	#include "c_ff_player.h"
#else
	#include "ff_player.h"
#endif

//=============================================================================
// CFlameJet [mirrored in ff_env_flamejet.cpp]
//=============================================================================
class CFlameJet : public CBaseParticleEntity
{
public:
	DECLARE_CLASS(CFlameJet, CBaseParticleEntity);
	DECLARE_DATADESC();

	// Modified by Mulchman - added the ifdef to
	// stop the client from complaining(throwing warning) 
	// during compile
#ifdef GAME_DLL
	DECLARE_SERVERCLASS();
#endif

	virtual void	Spawn();

// Stuff from the datatable.
public:
	CNetworkVar(int, m_bEmit);
};


//=============================================================================
// CFFWeaponFlamethrower
//=============================================================================

class CFFWeaponFlamethrower : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponFlamethrower, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponFlamethrower();

	virtual void PrimaryAttack();
	virtual void Fire();
	virtual void WeaponIdle();
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual bool Deploy();
	virtual void Precache();

	// Override the single one(quickest way to do it) 
	virtual void WeaponSound(WeaponSound_t sound_type, float soundtime = 0.0f) 
	{
		if (sound_type != SINGLE) 
			BaseClass::WeaponSound(sound_type, soundtime);			
	}

	// Turn on/off flames
	void TurnOn(bool on, bool force = false) 
	{
		if (m_pFlameJet) 
		{
			if ((m_pFlameJet->m_bEmit != 0) == on && !force)
				return;

			m_pFlameJet->m_bEmit = on;
		}

		if (on) 
			//EmitSound("flamethrower.loop_shot");
			WeaponSound(BURST);
		else
			WeaponSound(STOP);
			//StopSound("flamethrower.loop_shot");
	}


	virtual FFWeaponID GetWeaponID() const { return FF_WEAPON_FLAMETHROWER; }

private:

	CFlameJet *m_pFlameJet;
	bool		m_fFiring;

	CFFWeaponFlamethrower(const CFFWeaponFlamethrower &);

};

//=============================================================================
// CFFWeaponFlamethrower tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponFlamethrower, DT_FFWeaponFlamethrower) 

BEGIN_NETWORK_TABLE(CFFWeaponFlamethrower, DT_FFWeaponFlamethrower) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponFlamethrower) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_flamethrower, CFFWeaponFlamethrower);
PRECACHE_WEAPON_REGISTER(ff_weapon_flamethrower);

//=============================================================================
// CFFWeaponFlamethrower implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponFlamethrower::CFFWeaponFlamethrower() 
{
	m_pFlameJet = NULL;
	m_fFiring = false;
}

//----------------------------------------------------------------------------
// Purpose: Weapon's primary attack, reduce ammo / check for empty clip
//			Call Fire() if able to fire
//----------------------------------------------------------------------------
void CFFWeaponFlamethrower::PrimaryAttack() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	// Mulch: can't fire flamethrower under water or when waist deep in water
	if ((pPlayer->GetWaterLevel() == WL_Eyes) || (pPlayer->GetWaterLevel() == WL_Waist)) 
	{
		// Stop the emissions
		TurnOn(false);

#ifdef GAME_DLL
		Vector vecForward;
		pPlayer->EyeVectors(&vecForward, NULL, NULL);
		VectorNormalize(vecForward);

		Vector vecShootPos = pPlayer->Weapon_ShootPosition();

		UTIL_BubbleTrail(vecShootPos, vecShootPos + (vecForward * 32.0), random->RandomInt(5, 20));
#endif

		return;
	}

	m_fFiring = true;
	
	BaseClass::PrimaryAttack();
}

//----------------------------------------------------------------------------
// Purpose: Turns on the flame stream, creates it if it doesn't yet exist
//----------------------------------------------------------------------------
void CFFWeaponFlamethrower::Fire() 
{
	// Start emitting particles	
	TurnOn(true);

#ifdef GAME_DLL
	CFFPlayer *pPlayer = GetPlayerOwner();

	// Just a basic traceline, this isn't very good but I just want to get some damage in	
	Vector vecStart = pPlayer->Weapon_ShootPosition();
	Vector forward;
	trace_t traceHit;

	pPlayer->EyeVectors(&forward, NULL, NULL);

	Vector vecEnd = vecStart + forward * 200.0f;
	UTIL_TraceLine(vecStart, vecEnd, MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &traceHit);

	if (traceHit.m_pEnt && traceHit.m_pEnt->IsPlayer()) 
	{
		CFFPlayer *pTarget = ToFFPlayer(traceHit.m_pEnt);

		if (g_pGameRules->FPlayerCanTakeDamage(pPlayer, pTarget)) 
		{
			CTakeDamageInfo info(this, GetOwnerEntity(), 20, DMG_BURN);

			pTarget->TakeDamage(info);
			pTarget->ApplyBurning(pPlayer, 0.5f);
		}
	}
#endif
}

//----------------------------------------------------------------------------
// Purpose: Turns off the flame jet if it exists and player not firing
//----------------------------------------------------------------------------
void CFFWeaponFlamethrower::WeaponIdle() 
{
	if (m_fFiring)
	{
		m_fFiring = false;
		TurnOn(false);
	}

	BaseClass::WeaponIdle();
}


//----------------------------------------------------------------------------
// Purpose: Turns off the flame jet if player changes weapon
//----------------------------------------------------------------------------
bool CFFWeaponFlamethrower::Holster(CBaseCombatWeapon *pSwitchingTo) 
{
	TurnOn(false, true);

	return BaseClass::Holster();
}

//----------------------------------------------------------------------------
// Purpose: Play the ignite sound & create the flamejet entity
//----------------------------------------------------------------------------
bool CFFWeaponFlamethrower::Deploy() 
{
	// Play the ignite sound
	WeaponSound(SPECIAL1);

#ifdef GAME_DLL
	// Flamejet entity doesn't exist yet, so make it now
	if (!m_pFlameJet) 
	{
		CFFPlayer *pPlayer = GetPlayerOwner();
		QAngle angAiming;

		VectorAngles(pPlayer->GetAutoaimVector(0), angAiming);
		
		// Create a flamejet emitter
		m_pFlameJet = dynamic_cast<CFlameJet *> (CBaseEntity::Create("env_flamejet", pPlayer->Weapon_ShootPosition(), angAiming, this));

		// Should inherit it's angles & position from the player for now
		//m_pFlameJet->SetParent(pPlayer);
		//m_pFlameJet->SetParent(pPlayer);
		m_pFlameJet->SetOwnerEntity(pPlayer);
	}
#endif
	
	return BaseClass::Deploy();
}

//----------------------------------------------------------------------------
// Purpose: Precache some extra sounds
//----------------------------------------------------------------------------
void CFFWeaponFlamethrower::Precache() 
{
	PrecacheScriptSound("flamethrower.loop_shot");
	BaseClass::Precache();
}
