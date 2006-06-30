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
#include "in_buttons.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponFlamethrower C_FFWeaponFlamethrower
	#define CFFFlameJet C_FFFlameJet

	#include "c_ff_player.h"
	#include "c_ff_env_flamejet.h"
#else
	#include "ff_player.h"
	#include "ff_env_flamejet.h"
#endif

//=============================================================================
// CFlameJet [mirrored in ff_env_flamejet.cpp]
//=============================================================================



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

	virtual void Fire();
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual bool Deploy();
	virtual void Precache();
	virtual void ItemPostFrame();
	virtual void WeaponSound(WeaponSound_t sound_type, float soundtime = 0.0f);

	void EmitFlames(bool fEmit);

	virtual FFWeaponID GetWeaponID() const { return FF_WEAPON_FLAMETHROWER; }

private:

	CNetworkHandle(CFFFlameJet, m_hFlameJet);
	
	CFFWeaponFlamethrower(const CFFWeaponFlamethrower &);
};

//=============================================================================
// CFFWeaponFlamethrower tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponFlamethrower, DT_FFWeaponFlamethrower)

BEGIN_NETWORK_TABLE(CFFWeaponFlamethrower, DT_FFWeaponFlamethrower)
#ifdef GAME_DLL
	SendPropEHandle(SENDINFO(m_hFlameJet)),
#else
	RecvPropEHandle(RECVINFO(m_hFlameJet)),
#endif
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
	m_hFlameJet = NULL;
}

//----------------------------------------------------------------------------
// Purpose: Turns on the flame stream, creates it if it doesn't yet exist
//----------------------------------------------------------------------------
void CFFWeaponFlamethrower::Fire()
{
#ifdef GAME_DLL
	CFFPlayer *pPlayer = GetPlayerOwner();

	// If underwater then just innocent bubbles
	if (pPlayer->GetWaterLevel() == 3)
	{
		Vector vecForward;
		pPlayer->EyeVectors(&vecForward, NULL, NULL);
		VectorNormalize(vecForward);

		Vector vecShootPos = pPlayer->Weapon_ShootPosition();

		UTIL_BubbleTrail(vecShootPos, vecShootPos + (vecForward * 64.0), random->RandomInt(5, 20));

		return;
	}

	// Just a basic traceline, this isn't very good but I just want to get some damage in	
	Vector vecStart = pPlayer->Weapon_ShootPosition();
	Vector forward;
	trace_t traceHit;

	pPlayer->EyeVectors(&forward, NULL, NULL);

	Vector vecEnd = vecStart + forward * 200.0f;
	UTIL_TraceLine(vecStart, vecEnd, MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &traceHit);

	// We want to hit buildables too
	if (traceHit.m_pEnt) /* && traceHit.m_pEnt->IsPlayer())*/
	{
		CFFPlayer *pTarget = NULL;

		if (traceHit.m_pEnt->IsPlayer())
			pTarget = ToFFPlayer(traceHit.m_pEnt);
		else
		{
			if (traceHit.m_pEnt->Classify() == CLASS_SENTRYGUN)
				pTarget = ToFFPlayer( ( ( CFFSentryGun * )traceHit.m_pEnt )->m_hOwner.Get() );
			else if (traceHit.m_pEnt->Classify() == CLASS_DISPENSER)
				pTarget = ToFFPlayer( ( ( CFFDispenser * )traceHit.m_pEnt )->m_hOwner.Get() );
		}

		if (pTarget)
		{
			// If pTarget can take damage from the flame thrower shooter...
			if (g_pGameRules->FPlayerCanTakeDamage(pTarget, pPlayer))
			{
				CTakeDamageInfo info(this, GetOwnerEntity(), 20, DMG_BURN);

				pTarget->TakeDamage(info);
				pTarget->ApplyBurning(pPlayer, 0.5f);
			}
		}		
	}
#endif
}

//----------------------------------------------------------------------------
// Purpose: Turns off the flame jet if player changes weapon
//----------------------------------------------------------------------------
bool CFFWeaponFlamethrower::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	EmitFlames(false);

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
	if (!m_hFlameJet)
	{
		CFFPlayer *pPlayer = GetPlayerOwner();
		QAngle angAiming;

		VectorAngles(pPlayer->GetAutoaimVector(0), angAiming);
		
		// Create a flamejet emitter
		m_hFlameJet = dynamic_cast<CFFFlameJet *> (CBaseEntity::Create("env_flamejet", pPlayer->Weapon_ShootPosition(), angAiming, this));

		// Should inherit it's angles & position from the player for now
		m_hFlameJet->SetOwnerEntity(pPlayer);
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

//----------------------------------------------------------------------------
// Purpose: Turn flame jet on or off
//----------------------------------------------------------------------------
void CFFWeaponFlamethrower::EmitFlames(bool fEmit)
{
	// Try changing the flamejet. If status has changed, play the correct sound.
	if (m_hFlameJet && m_hFlameJet->FlameEmit(fEmit))
	{
		if (fEmit)
			WeaponSound(BURST);
		else
			WeaponSound(STOP);
	}
}

//====================================================================================
// WEAPON BEHAVIOUR
//====================================================================================
void CFFWeaponFlamethrower::ItemPostFrame()
{
	CFFPlayer *pOwner = ToFFPlayer(GetOwner());

	if (!pOwner)
		return;

	// Keep track of fire duration for anywhere else it may be needed
	m_fFireDuration = (pOwner->m_nButtons & IN_ATTACK) ? (m_fFireDuration + gpGlobals->frametime) : 0.0f;

	// Player is holding down fire
	if (pOwner->m_nButtons & IN_ATTACK)
	{
		// Ensure it can't fire underwater
		if (pOwner->GetWaterLevel() == 3)
			EmitFlames(false);
		else
			EmitFlames(true);

		// Time for the next real fire think
		if (m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
			// Out of ammo
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			{
				HandleFireOnEmpty();
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;
			}

			// This weapon doesn't fire underwater
			//else if (pOwner->GetWaterLevel() == 3)
			//{
			//	WeaponSound(EMPTY);
			//	m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			//	return;
			//}

			// Weapon should be firing now
			else
			{
				// If the firing button was just pressed, reset the firing time
				if (pOwner && pOwner->m_afButtonPressed & IN_ATTACK)
					m_flNextPrimaryAttack = gpGlobals->curtime;

				PrimaryAttack();
			}
		}
	}
	// No buttons down
	else
	{
		EmitFlames(false);
		WeaponIdle();
	}
}

//----------------------------------------------------------------------------
// Purpose: Quick change to override the single sound
//----------------------------------------------------------------------------
void CFFWeaponFlamethrower::WeaponSound(WeaponSound_t sound_type, float soundtime)
{
	if (sound_type != SINGLE)
		BaseClass::WeaponSound(sound_type, soundtime);			
}