/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_laserrifle.cpp
/// @author David "Mervaka" Cook
/// @date 02 April 2010
/// @brief The FF laserrifle code & class declaration
///
/// REVISIONS
/// ---------
/// Apr 02, 2010 Merv: First logged


#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"
#include "in_buttons.h"
#include "ff_weapon_sniperrifle.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponLaserRifle C_FFWeaponLaserRifle
	#define CFFFlameJet C_FFFlameJet

	#include "c_ff_player.h"
	#include "c_ff_env_flamejet.h"

	#include "ff_utils.h"
#else
	#include "omnibot_interface.h"
	#include "ff_player.h"
	#include "ff_env_flamejet.h"
	#include "ff_player.h"
	#include "ilagcompensationmanager.h"
#endif

ConVar ffdev_laserrifle_zoomfov("ffdev_laserrifle_zoomfov", "25", FCVAR_REPLICATED, "fov of the rifle zoom (+attack2). smaller value = more zoomed in.");
ConVar ffdev_laserrifle_dmg("ffdev_laserrifle_dmg", "10", 0, "damage of the laser rifle" );
ConVar ffdev_laserrifle_dmg_bmult("ffdev_laserrifle_dmg_bmult", "1", 0, "damage multiplier for buildables" );

#ifdef GAME_DLL
	ConVar ffdev_laserrifle_showtrace("ffdev_laserrifle_showtrace", "0", FCVAR_CHEAT, "Show flame trace");
	//ConVar buildable_flame_damage( "ffdev_buildable_flame_dmg", "18", FCVAR_ARCHIVE );
#endif

//=============================================================================
// CFFWeaponLaserRifle
//=============================================================================

class CFFWeaponLaserRifle : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponLaserRifle, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponLaserRifle();
	virtual ~CFFWeaponLaserRifle();

	virtual void Fire();
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual bool Deploy();
	virtual void Precache();
	virtual void ItemPostFrame();
	//virtual void WeaponSound(WeaponSound_t sound_type, float soundtime = 0.0f);


	virtual FFWeaponID GetWeaponID() const { return FF_WEAPON_LASERRIFLE; }

	void UpdateLaserPosition();
	void ToggleZoom();

#ifdef CLIENT_DLL
	virtual float GetFOV();
#endif

private:
	bool m_bZoomed;
	void CheckZoomToggle();

#ifdef CLIENT_DLL
	float m_flZoomTime;
	float m_flNextZoomTime;
#endif

#ifdef GAME_DLL
	CHandle<CFFWeaponLaserDot>	m_hLaserDot;
#endif

CFFWeaponLaserRifle(const CFFWeaponLaserRifle &);
};

//=============================================================================
// CFFWeaponLaserRifle tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponLaserRifle, DT_FFWeaponLaserRifle)

BEGIN_NETWORK_TABLE(CFFWeaponLaserRifle, DT_FFWeaponLaserRifle)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CFFWeaponLaserRifle)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(ff_weapon_laserrifle, CFFWeaponLaserRifle);
PRECACHE_WEAPON_REGISTER(ff_weapon_laserrifle);

//=============================================================================
// CFFWeaponLaserRifle implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponLaserRifle::CFFWeaponLaserRifle()
{
	m_bMuzzleFlash = false;
}

//----------------------------------------------------------------------------
// Purpose: Destructor, destroy flamejet
//----------------------------------------------------------------------------
CFFWeaponLaserRifle::~CFFWeaponLaserRifle()
{
#ifdef GAME_DLL
	if (m_hLaserDot != NULL) 
	{
		UTIL_Remove(m_hLaserDot);
		m_hLaserDot = NULL;
	}
#endif
}

//----------------------------------------------------------------------------
// Purpose: Turns on the flame stream, creates it if it doesn't yet exist
//----------------------------------------------------------------------------
void CFFWeaponLaserRifle::Fire()
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	Vector vecForward;
	pPlayer->EyeVectors(&vecForward);

	// Normalize, or we get that weird epsilon assert
	VectorNormalizeFast( vecForward );

//	float flCapSqr = ffdev_laserrifle_boostcap.GetFloat() * ffdev_laserrifle_boostcap.GetFloat();

	Vector vecShootPos = pPlayer->Weapon_ShootPosition();

#ifdef GAME_DLL	

	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation(pPlayer, pPlayer->GetCurrentCommand());

	Vector vecStart = vecShootPos + vecForward * 16.0f;

	// 320 is about how far the flames are drawn on the client
	// 0.4f is the time taken to reach end of flame jet
	// EDIT: Both are 20% longer now
	Vector vecEnd = vecStart + vecForward * MAX_TRACE_LENGTH;

	// Visualise trace
	if (ffdev_laserrifle_showtrace.GetBool())
	{
		NDebugOverlay::Line(vecStart, vecEnd, 255, 255, 0, false, 1.0f);
	}
	
	trace_t traceHit;
	UTIL_TraceLine( vecStart, vecEnd, MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &traceHit );

	if (traceHit.m_pEnt)
	{
		CBaseEntity *pTarget = traceHit.m_pEnt;

		// only interested in players, dispensers & sentry guns
		if ( pTarget->IsPlayer() || pTarget->Classify() == CLASS_DISPENSER || pTarget->Classify() == CLASS_SENTRYGUN )
		{
			// If pTarget can take damage from the flame thrower shooter...
			if ( g_pGameRules->FCanTakeDamage( pTarget, pPlayer ))
			{
				// Don't burn a guy who is underwater
				if (pTarget->IsPlayer() )
				{
					CFFPlayer *pPlayerTarget = dynamic_cast< CFFPlayer* > ( pTarget );

					pPlayerTarget->TakeDamage( CTakeDamageInfo( this, pPlayer, ffdev_laserrifle_dmg.GetFloat() /* GetFFWpnData().m_iDamage */, DMG_ENERGYBEAM ) );
				}
				// TODO: Check water level for dispensers & sentryguns!
				else if( FF_IsDispenser( pTarget ) )
				{
					CFFDispenser *pDispenser = FF_ToDispenser( pTarget );
					pDispenser->TakeDamage( CTakeDamageInfo( this, pPlayer, ffdev_laserrifle_dmg.GetFloat() * ffdev_laserrifle_dmg_bmult.GetFloat(), DMG_ENERGYBEAM ) );
				}
				else if( FF_IsSentrygun( pTarget ) )
				{
					CFFSentryGun *pSentrygun = FF_ToSentrygun( pTarget );
					pSentrygun->TakeDamage( CTakeDamageInfo( this, pPlayer, ffdev_laserrifle_dmg.GetFloat() * ffdev_laserrifle_dmg_bmult.GetFloat(), DMG_ENERGYBEAM ) );
				}
				else /*if( FF_IsManCannon( pTarget ) )*/
				{
					CFFManCannon *pManCannon = FF_ToManCannon( pTarget );
					if( pManCannon )
						pManCannon->TakeDamage( CTakeDamageInfo( this, pPlayer, ffdev_laserrifle_dmg.GetFloat() * ffdev_laserrifle_dmg_bmult.GetFloat(), DMG_ENERGYBEAM ) );
				}
			}
		}		
	}

	lagcompensation->FinishLagCompensation(pPlayer);
#endif

#ifdef GAME_DLL
//	Omnibot::Notify_PlayerShoot(pPlayer, Omnibot::TF_WP_FLAMETHROWER, 0);
#endif
}

//----------------------------------------------------------------------------
// Purpose: Turns off the flame jet if player changes weapon
//----------------------------------------------------------------------------
bool CFFWeaponLaserRifle::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	// Doing it this way stops the s_absQueriesValid assert
#ifdef CLIENT_DLL 
	m_flNextZoomTime = m_flZoomTime = 0;
	engine->ClientCmd("fov 0\n");
	if( GetPlayerOwner() == C_FFPlayer::GetLocalFFPlayer() )
#endif
		WeaponSound( STOP );

	return BaseClass::Holster();
}

//----------------------------------------------------------------------------
// Purpose: Play the ignite sound & create the flamejet entity
//----------------------------------------------------------------------------
bool CFFWeaponLaserRifle::Deploy()
{
	m_bZoomed = false;

#ifdef CLIENT_DLL
	m_flNextZoomTime = m_flZoomTime = 0;
#endif
	
	return BaseClass::Deploy();
}

//----------------------------------------------------------------------------
// Purpose: Precache some extra sounds
//----------------------------------------------------------------------------
void CFFWeaponLaserRifle::Precache()
{
	PrecacheModel(SNIPER_DOT);
	PrecacheModel(SNIPER_BEAM);
	PrecacheScriptSound("laserrifle.loop_shot");
	PrecacheScriptSound("laserrifle.zoom_out");
	PrecacheScriptSound("laserrifle.zoom_in");
	BaseClass::Precache();
}

void CFFWeaponLaserRifle::ToggleZoom() 
{
#ifdef CLIENT_DLL
	if (m_flNextZoomTime > gpGlobals->curtime)
		return;

	m_flNextZoomTime = gpGlobals->curtime + 0.2f;
	m_flZoomTime = gpGlobals->tickcount * gpGlobals->interval_per_tick;

	m_bZoomed = !m_bZoomed;
	
	C_FFPlayer *pPlayer = GetPlayerOwner();

	if (pPlayer)
	{
		CSingleUserRecipientFilter filter(pPlayer);
		EmitSound(filter, pPlayer->entindex(), m_bZoomed ? "SniperRifle.zoom_in" : "SniperRifle.zoom_out");
	}

	// Set the fov cvar (which we ignore on the client) so that the server is up
	// to date. Not the best way of doing it REALLY
	pPlayer->m_iFOV = m_bZoomed ? 25 : 0;
#endif
}
/*
//----------------------------------------------------------------------------
// Purpose: Turn flame jet on or off
//----------------------------------------------------------------------------
void CFFWeaponLaserRifle::EmitFlames(bool bEmit)
{
	// We're using m_flNextSecondaryAttack to make sure we don't draw the flames
	// before we're allowed to fire (set by DefaultDeploy)
	if (bEmit && m_flNextSecondaryAttack > gpGlobals->curtime)
	{
		WeaponSound(STOP);
		return;
	}
	// Spawn the FlameJet if necessary
//	if(m_hFlameJet)
//		m_hFlameJet->FlameEmit(bEmit);
	// If we are going to Emit play the sound, otherwise don't play anything.
	if (bEmit)
		WeaponSound(BURST);
	else
		WeaponSound(STOP);
	return;
}
*/
//====================================================================================
// WEAPON BEHAVIOUR
//====================================================================================
void CFFWeaponLaserRifle::ItemPostFrame()
{
	CheckZoomToggle();
	CFFPlayer *pOwner = ToFFPlayer(GetOwner());

	if (!pOwner)
		return;

	// Keep track of fire duration for anywhere else it may be needed
	m_fFireDuration = (pOwner->m_nButtons & IN_ATTACK) ? (m_fFireDuration + gpGlobals->frametime) : 0.0f;

	// Player is holding down fire
	if (pOwner->m_nButtons & IN_ATTACK)
	{
		// Time for the next real fire think
		if( m_flNextPrimaryAttack <= gpGlobals->curtime )
		{
			// Out of ammo
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			{
				HandleFireOnEmpty();
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;
			}

			// Weapon should be firing now
			else
			{
				// If the firing button was just pressed, reset the firing time
				if (pOwner && pOwner->m_afButtonPressed & IN_ATTACK)
				{
					m_flNextPrimaryAttack = gpGlobals->curtime;
					WeaponSound(BURST);
				}
#ifdef GAME_DLL
				UpdateLaserPosition();
			if (m_hLaserDot.Get()) 
				m_hLaserDot->m_flStartTime = gpGlobals->curtime;
#endif
				PrimaryAttack();
			}
		}
	}
	// No buttons down
	else
	{
		WeaponSound(STOP);
#ifdef GAME_DLL
		UTIL_Remove(m_hLaserDot);
#endif
		WeaponIdle();
	}
}

void CFFWeaponLaserRifle::CheckZoomToggle() 
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	
	if (pPlayer->m_afButtonPressed & IN_ATTACK2) 
	{
		//DevMsg("[sniper rifle] Toggling Zoom!\n");
		ToggleZoom();
	}
}

void CFFWeaponLaserRifle::UpdateLaserPosition() 
{
#ifdef GAME_DLL
	CFFPlayer *pPlayer = GetPlayerOwner();

	// Create the dot if needed
	if (m_hLaserDot == NULL) 
	{
		CBaseCombatCharacter *pOwner = GetOwner();
	
		if (pOwner != NULL) 
		{
			m_hLaserDot = CFFWeaponLaserDot::Create(GetAbsOrigin(), GetOwner());
			m_hLaserDot->TurnOff();

			m_hLaserDot->m_flStartTime = gpGlobals->curtime;

			UpdateLaserPosition();
		}
	}
	else
	{
		Vector	vForward;
		pPlayer->EyeVectors(&vForward);

		trace_t tr;
		UTIL_TraceLine(pPlayer->Weapon_ShootPosition(), pPlayer->Weapon_ShootPosition() + (vForward * 1600.0f), MASK_SOLID|CONTENTS_DEBRIS|CONTENTS_HITBOX, this, COLLISION_GROUP_NONE, &tr);

		// Put dot on the wall that we hit, but pull back a little
		m_hLaserDot->SetLaserPosition(tr.endpos - vForward);
		m_hLaserDot->AddEFlags(EFL_FORCE_CHECK_TRANSMIT);
	}
#endif
}


#ifdef CLIENT_DLL

extern ConVar default_fov;

//-----------------------------------------------------------------------------
// Purpose: Get the weapon's fov
//-----------------------------------------------------------------------------
float CFFWeaponLaserRifle::GetFOV()
{
	C_FFPlayer *pPlayer = GetPlayerOwner();

	if (!pPlayer)
	{
		return -1;
	}

	float deltaTime = (float) (gpGlobals->tickcount * gpGlobals->interval_per_tick - m_flZoomTime) * 5.0f;

	if (deltaTime < 1.0f)
	{
		// Random negative business
		if (deltaTime < 0)
		{
			return (m_bZoomed ? -1 : ffdev_laserrifle_zoomfov.GetFloat());
		}

		float flFOV;

		if (m_bZoomed)
		{
			flFOV = SimpleSplineRemapVal(deltaTime, 0.0f, 1.0f, default_fov.GetFloat(), ffdev_laserrifle_zoomfov.GetFloat());
		}
		else
		{
			flFOV = SimpleSplineRemapVal(deltaTime, 0.0f, 1.0f, ffdev_laserrifle_zoomfov.GetFloat(), default_fov.GetFloat());
		}

		return flFOV;
	}

	return (m_bZoomed ? ffdev_laserrifle_zoomfov.GetFloat() : -1);
}
#endif