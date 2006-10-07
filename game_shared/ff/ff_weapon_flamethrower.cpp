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

	#include "ilagcompensationmanager.h"
#endif

ConVar ffdev_flame_bbox("ffdev_flame_bbox", "24.0", FCVAR_REPLICATED, "Flame bbox");

#ifdef GAME_DLL
	ConVar ffdev_flame_showtrace("ffdev_flame_showtrace", "0", FCVAR_NONE, "Show flame trace");
	//ConVar buildable_flame_damage( "ffdev_buildable_flame_dmg", "18", FCVAR_ARCHIVE );
#endif

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
	//virtual void WeaponSound(WeaponSound_t sound_type, float soundtime = 0.0f);

	void EmitFlames(bool fEmit);

	virtual ~CFFWeaponFlamethrower();

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
// Purpose: Destructor, destroy flamejet
//----------------------------------------------------------------------------
CFFWeaponFlamethrower::~CFFWeaponFlamethrower()
{
#ifdef GAME_DLL
	if (m_hFlameJet)
		UTIL_Remove(m_hFlameJet);
#endif
}


//----------------------------------------------------------------------------
// Purpose: Turns on the flame stream, creates it if it doesn't yet exist
//----------------------------------------------------------------------------
void CFFWeaponFlamethrower::Fire()
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	Vector vecForward;
	pPlayer->EyeVectors(&vecForward);

	// Normalize, or we get that weird epsilon assert
	VectorNormalizeFast( vecForward );

	// Push them gently if in air
	if (!pPlayer->GetGroundEntity())
		pPlayer->ApplyAbsVelocityImpulse(vecForward * -12.0f);

#ifdef GAME_DLL
	Vector vecShootPos = pPlayer->Weapon_ShootPosition();

	// If underwater then just innocent bubbles
	if (pPlayer->GetWaterLevel() == 3)
	{
		UTIL_BubbleTrail(vecShootPos, vecShootPos + (vecForward * 64.0), random->RandomInt(5, 20));
		return;
	}

	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation(pPlayer, pPlayer->GetCurrentCommand());

	Vector vecStart = vecShootPos + vecForward * 16.0f;

	// 320 is about how far the flames are drawn on the client
	// 0.4f is the time taken to reach end of flame jet
	// EDIT: Both are 20% longer now
	Vector vecEnd = vecStart + ( vecForward * 384.0f ) - GetAbsVelocity() * 0.48f;

	// Visualise trace
	if (ffdev_flame_showtrace.GetBool())
	{
		NDebugOverlay::Line(vecStart, vecEnd, 255, 255, 0, false, 1.0f);
		
		QAngle angDir;
		VectorAngles(vecForward, angDir);
		NDebugOverlay::SweptBox(vecStart, vecEnd, -Vector( 1.0f, 1.0f, 1.0f ) * ffdev_flame_bbox.GetFloat(), Vector( 1.0f, 1.0f, 1.0f ) * ffdev_flame_bbox.GetFloat(), angDir, 200, 100, 0, 100, 0.1f);
	}
	
	// Changed to this to add some "width" to the shot. How much more expensive is this than traceline???
	trace_t traceHit;
	UTIL_TraceHull( vecStart, vecEnd, -Vector( 1.0f, 1.0f, 1.0f ) * ffdev_flame_bbox.GetFloat(), Vector( 1.0f, 1.0f, 1.0f ) * ffdev_flame_bbox.GetFloat(), MASK_SHOT_HULL | MASK_WATER, pPlayer, COLLISION_GROUP_NONE, &traceHit );

	// Don't hit water
	if( ( traceHit.contents & CONTENTS_WATER ) || ( traceHit.contents & CONTENTS_SLIME ) )
	{
		lagcompensation->FinishLagCompensation(pPlayer);
		return;
	}

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
				// Don't burn a guy who is underwater
				if (traceHit.m_pEnt->IsPlayer() && ( pTarget->GetWaterLevel() < 3 ) )
				{
					pTarget->TakeDamage( CTakeDamageInfo( this, pPlayer, GetFFWpnData().m_iDamage, DMG_BURN ) );
					pTarget->ApplyBurning( pPlayer, 0.5f, 10.0f, BURNTYPE_FLAMETHROWER);
				}
				// TODO: Check water level for dispensers & sentryguns!
				else if( FF_IsDispenser( traceHit.m_pEnt ) )
				{
					CFFDispenser *pDispenser = FF_ToDispenser( traceHit.m_pEnt );
					if( pDispenser && ( pDispenser->GetWaterLevel() <= WL_Waist ) )
						pDispenser->TakeDamage( CTakeDamageInfo( this, pPlayer, 18.0f, DMG_BURN ) );
				}
				else if( FF_IsSentrygun( traceHit.m_pEnt ) )
				{
					CFFSentryGun *pSentrygun = FF_ToSentrygun( traceHit.m_pEnt );
					if( pSentrygun && ( pSentrygun->GetWaterLevel() <= WL_Waist ) )
						pSentrygun->TakeDamage( CTakeDamageInfo( this, pPlayer, 18.0f, DMG_BURN ) );
				}
			}
		}		
	}

	lagcompensation->FinishLagCompensation(pPlayer);
#endif
}

//----------------------------------------------------------------------------
// Purpose: Turns off the flame jet if player changes weapon
//----------------------------------------------------------------------------
bool CFFWeaponFlamethrower::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	EmitFlames(false);

	// Stop any sound effect that may be playing at the time. Holster seems to be called
	// on the client for other people too. It should only be sent by the server for other
	// people's holstering.

#ifdef CLIENT_DLL
	if (GetPlayerOwner() == CBasePlayer::GetLocalPlayer())
#endif
	{
		WeaponSound(STOP);
	}

	return BaseClass::Holster();
}

//----------------------------------------------------------------------------
// Purpose: Play the ignite sound & create the flamejet entity
//----------------------------------------------------------------------------
bool CFFWeaponFlamethrower::Deploy()
{
	// Play the ignite sound
	WeaponSoundLocal(SPECIAL1);

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
		m_hFlameJet->FollowEntity(pPlayer);
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
	// We're using m_flNextSecondaryAttack to make sure we don't draw the flames
	// before we're allowed to fire (set by DefaultDeploy)
	if (fEmit && m_flNextSecondaryAttack > gpGlobals->curtime)
	{
		return;
	}

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

/*
//----------------------------------------------------------------------------
// Purpose: Quick change to override the single sound
//----------------------------------------------------------------------------
void CFFWeaponFlamethrower::WeaponSound(WeaponSound_t sound_type, float soundtime)
{
	if (sound_type != SINGLE)
		BaseClass::WeaponSound(sound_type, soundtime);			
}
*/
