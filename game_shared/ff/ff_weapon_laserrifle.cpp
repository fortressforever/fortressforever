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
	#define CFFWeaponLaserBeam C_FFWeaponLaserBeam

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

ConVar ffdev_laserrifle_zoomfov("ffdev_laserrifle_zoomfov", "40", FCVAR_REPLICATED, "fov of the rifle zoom (+attack2). smaller value = more zoomed in.");
ConVar ffdev_laserrifle_laserdot_scale("ffdev_laserrifle_laserdot_scale", "0.15", FCVAR_REPLICATED, "Scale of the sniper rifle laser dot");

#ifdef GAME_DLL
ConVar ffdev_laserrifle_dmg("ffdev_laserrifle_dmg", "14", 0, "damage of the laser rifle" );
ConVar ffdev_laserrifle_dmg_bmult("ffdev_laserrifle_dmg_bmult", "1.2", 0, "damage multiplier for buildables" );
ConVar ffdev_laserrifle_showtrace("ffdev_laserrifle_showtrace", "0", FCVAR_CHEAT, "Show laser trace");
ConVar ffdev_laserrifle_slowdownfactor("ffdev_laserrifle_slowdownfactor", "1", 0, "Slowdown factor per damage infliction");
#endif





//=============================================================================
// CFFWeaponLaserBeam
//=============================================================================

//=============================================================================
// CFFWeaponLaserDot
//=============================================================================

class CFFWeaponLaserBeam : public CSprite
{
public:
	DECLARE_CLASS(CFFWeaponLaserBeam, CSprite);

	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

#ifdef CLIENT_DLL
	CFFWeaponLaserBeam();
	virtual void GetRenderBounds(Vector& mins, Vector& maxs);
#endif

	static	CFFWeaponLaserBeam *Create(const Vector &origin, CBaseEntity *pOwner = NULL);

	void	SetLaserPosition(const Vector &origin);

	bool	IsOn() const		{ return m_bIsOn; }

	void	TurnOn() 		{ m_bIsOn = true; }
	void	TurnOff() 		{ m_bIsOn = false; }
	void	Toggle() 		{ m_bIsOn = !m_bIsOn; }

	int		ObjectCaps() { return (BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }

#ifdef CLIENT_DLL

	virtual bool			IsTransparent() { return true; }
	virtual RenderGroup_t	GetRenderGroup() { return RENDER_GROUP_TRANSLUCENT_ENTITY; }
	virtual int				DrawModel(int flags);
	virtual void			OnDataChanged(DataUpdateType_t updateType);
	virtual bool			ShouldDraw() { return (IsEffectActive(EF_NODRAW) ==false); }

	IMaterial	*m_pMaterial;
#endif

	CNetworkVar(float, m_flStartTime);

protected:
	bool				m_bIsOn;

};


//=============================================================================
// CFFWeaponLaserBeam tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponLaserBeam, DT_FFWeaponLaserBeam) 

BEGIN_NETWORK_TABLE(CFFWeaponLaserBeam, DT_FFWeaponLaserBeam) 
#ifdef CLIENT_DLL
	RecvPropFloat(RECVINFO(m_flStartTime)) 
#else
	SendPropFloat(SENDINFO(m_flStartTime)) 
#endif
END_NETWORK_TABLE() 

LINK_ENTITY_TO_CLASS(env_fflaserbeam, CFFWeaponLaserBeam);

BEGIN_DATADESC(CFFWeaponLaserBeam) 
	DEFINE_FIELD(m_bIsOn, 				FIELD_BOOLEAN), 
END_DATADESC() 

//=============================================================================
// CFFWeaponLaserBeam implementation
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Create a laser dot
// Input  : &origin - 
// Output : CFFWeaponLaserBeam
//-----------------------------------------------------------------------------
CFFWeaponLaserBeam *CFFWeaponLaserBeam::Create(const Vector &origin, CBaseEntity *pOwner) 
{
#ifdef GAME_DLL
	CFFWeaponLaserBeam *pLaserDot = (CFFWeaponLaserBeam *) CBaseEntity::Create("env_fflaserbeam", origin, QAngle(0, 0, 0));

	if (pLaserDot == NULL) 
		return NULL;

	pLaserDot->SetRenderMode((RenderMode_t) 9);

	pLaserDot->SetMoveType(MOVETYPE_NONE);
	pLaserDot->AddSolidFlags(FSOLID_NOT_SOLID);
	pLaserDot->AddEffects(EF_NOSHADOW);
	//UTIL_SetSize(pLaserDot, -Vector(4, 4, 4), Vector(4, 4, 4));
	UTIL_SetSize(pLaserDot, vec3_origin, vec3_origin);

	pLaserDot->SetOwnerEntity(pOwner);

	pLaserDot->AddEFlags(EFL_FORCE_CHECK_TRANSMIT);

	pLaserDot->SpriteInit(SNIPER_DOT, origin);
	pLaserDot->SetName(AllocPooledString("TEST"));
	pLaserDot->SetTransparency(kRenderWorldGlow, 255, 255, 255, 255, kRenderFxNoDissipation);
	pLaserDot->SetScale(ffdev_laserrifle_laserdot_scale.GetFloat());
	pLaserDot->SetOwnerEntity(pOwner);
//	pLaserDot->SetContextThink(LaserThink, gpGlobals->curtime + 0.1f, g_pLaserDotThink);
	pLaserDot->SetSimulatedEveryTick(true);


	return pLaserDot;
#else
	return NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Need to recompute the collision bounds to include the player too
//			so that the laser dot is active on the client at the right times
//-----------------------------------------------------------------------------
void CFFWeaponLaserBeam::SetLaserPosition(const Vector &origin) 
{
	SetAbsOrigin(origin);

	CFFPlayer *pOwner = ToFFPlayer(GetOwnerEntity());

	if (!pOwner)
		return;

	Vector vecAbsStart = GetAbsOrigin();
	Vector vecAbsEnd = pOwner->Weapon_ShootPosition();

	Vector vecBeamMin, vecBeamMax;
	VectorMin(vecAbsStart, vecAbsEnd, vecBeamMin);
	VectorMax(vecAbsStart, vecAbsEnd, vecBeamMax);

	SetCollisionBounds(vecBeamMin - GetAbsOrigin(), vecBeamMax - GetAbsOrigin());
}

#ifdef CLIENT_DLL

	//-----------------------------------------------------------------------------
	// Purpose: Constructor, get the right materials
	//-----------------------------------------------------------------------------
	CFFWeaponLaserBeam::CFFWeaponLaserBeam()
	{
		m_pMaterial = materials->FindMaterial("effects/blueblacklargebeam", TEXTURE_GROUP_CLIENT_EFFECTS);
		m_pMaterial->IncrementReferenceCount();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Need to make sure that the render bounds include the source
	//			player too
	//-----------------------------------------------------------------------------
	void CFFWeaponLaserBeam::GetRenderBounds(Vector& mins, Vector& maxs)
	{
		// Hey, this is throwing a C_FFPlayer::ToFFPlayer( NULL ) assert... 
		CFFPlayer *pOwner = ToFFPlayer(GetOwnerEntity());

		if (!pOwner)
		{
			BaseClass::GetRenderBounds(mins, maxs);
			return;
		}

		Vector vecAbsStart = GetAbsOrigin();
		Vector vecAbsEnd = pOwner->Weapon_ShootPosition();

		for (int i = 0; i < 3; ++i)
		{
			if (vecAbsStart[i] < vecAbsEnd[i])
			{
				mins[i] = vecAbsStart[i];
				maxs[i] = vecAbsEnd[i];
			}
			else
			{
				mins[i] = vecAbsEnd[i];
				maxs[i] = vecAbsStart[i];
			}
		}

		Vector vecAbsOrigin = GetAbsOrigin();
		mins -= vecAbsOrigin;
		maxs -= vecAbsOrigin;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Draw our sprite
	//-----------------------------------------------------------------------------
	int CFFWeaponLaserBeam::DrawModel(int flags) 
	{
		//See if we should draw
		if (!IsVisible() || (m_bReadyToDraw == false)) 
			return 0;

		//Must be a sprite
		if (modelinfo->GetModelType(GetModel()) != mod_sprite) 
		{
			assert(0);
			return 0;
		}

		float renderscale;
		Vector vecAttachment, vecDir, endPos;
		bool fDrawDot = true;

		int alpha = clamp(150 + 15 * (gpGlobals->curtime - m_flStartTime), 0, 255);

		CFFPlayer *pOwner = ToFFPlayer(GetOwnerEntity());

		// We're going to predict it using the players' angles
		if (pOwner != NULL && pOwner->IsDormant() == false) 
		{
			// Take the eye position and direction
			vecAttachment = pOwner->Weapon_ShootPosition();
			AngleVectors(pOwner->EyeAngles(), &vecDir);

			trace_t tr;
			UTIL_TraceLine(vecAttachment, vecAttachment + (vecDir * MAX_TRACE_LENGTH), MASK_SHOT, pOwner, COLLISION_GROUP_LASER, &tr);

			// Backup without using the normal (for trackerid: #0000866)
			endPos = tr.endpos - vecDir;

			// Bug #0000376: Sniper dot is drawn on sky brushes
			if (tr.surface.flags & SURF_SKY)
				fDrawDot = false;

			color32 colour = { 255, 0, 0, alpha };
			

			// We still trace from eye position, but we want to draw from the gun muzzle
			Vector vecFalseOrigin;
			QAngle angFalseAngles;
			if ( pOwner->IsLocalPlayer() )
			{
				C_BaseViewModel *pWeapon = pOwner->GetViewModel();
				if ( !pWeapon )
					return 0;
				pWeapon->GetAttachment( pWeapon->LookupAttachment("1"), vecFalseOrigin, angFalseAngles );

			}
			else
			{
				C_FFWeaponBase *pWeapon = pOwner->GetActiveFFWeapon();
				if ( !pWeapon )
					return 0;
				pWeapon->GetAttachment( pWeapon->LookupAttachment("1"), vecFalseOrigin, angFalseAngles );
			}


			FX_DrawLine(vecFalseOrigin, tr.endpos, 1, m_pMaterial, colour);


		}
		else
		{
			// Just use our position if we can't predict it otherwise
			endPos = GetAbsOrigin();
		}

		// Randomly flutter
		//renderscale = 16.0f + random->RandomFloat(-2.0f, 2.0f);	
		renderscale = ffdev_laserrifle_laserdot_scale.GetFloat() + random->RandomFloat(-0.005f, 0.005f);

		if (!fDrawDot)
			return 0;

		//Draw it
		int drawn = DrawSprite(
			this, 
			GetModel(), 
			endPos, 
			GetAbsAngles(), 
			m_flFrame, 				// sprite frame to render
			m_hAttachedToEntity, 	// attach to
			m_nAttachment, 			// attachment point
			GetRenderMode(), 		// rendermode
			m_nRenderFX, 
			alpha, 		// alpha
			m_clrRender->r, 
			m_clrRender->g, 
			m_clrRender->b, 
			renderscale);			// sprite scale

		return drawn;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Setup our sprite reference
	//-----------------------------------------------------------------------------
	void CFFWeaponLaserBeam::OnDataChanged(DataUpdateType_t updateType) 
	{
		BaseClass::OnDataChanged( updateType );

		if (updateType == DATA_UPDATE_CREATED) 
		{
			SetNextClientThink( CLIENT_THINK_ALWAYS );
		}
	}
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
	CHandle<CFFWeaponLaserBeam>	m_hLaserDot;
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
					
					//Editing this block so i can set where blood spawns?
					CTakeDamageInfo info = CTakeDamageInfo( this, pPlayer, ffdev_laserrifle_dmg.GetFloat() /* GetFFWpnData().m_iDamage */, DMG_ENERGYBEAM );
					info.SetDamagePosition( traceHit.startpos );
					info.SetImpactPosition( traceHit.endpos );

					pPlayerTarget->TakeDamage( info );
					float flVelocityFactor = ffdev_laserrifle_slowdownfactor.GetFloat();
					if ( flVelocityFactor != 1.0 && !(pPlayerTarget->GetFlags() & FL_ONGROUND) )
						pPlayerTarget->SetAbsVelocity( Vector(	pPlayerTarget->GetAbsVelocity().x * flVelocityFactor , 
																pPlayerTarget->GetAbsVelocity().y * flVelocityFactor, 
																pPlayerTarget->GetAbsVelocity().z > 0 ? pPlayerTarget->GetAbsVelocity().z * flVelocityFactor : pPlayerTarget->GetAbsVelocity().z));
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

#ifdef GAME_DLL
	if (m_hLaserDot != NULL)
	{
		UTIL_Remove(m_hLaserDot);
		m_hLaserDot = NULL;
	}
#endif

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
		// Set the fov cvar (which we ignore on the client) so that the server is up
		// to date. Not the best way of doing it REALLY
		pPlayer->m_iFOV = m_bZoomed ? ffdev_laserrifle_zoomfov.GetFloat() : 0;
	}

#endif
}

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
	//m_fFireDuration = (pOwner->m_nButtons & IN_ATTACK) ? (m_fFireDuration + gpGlobals->frametime) : 0.0f;

	// Player is holding down fire
	if (pOwner->m_nButtons & IN_ATTACK)
	{
		// Time for the next real fire think
		if( m_flNextPrimaryAttack <= gpGlobals->curtime )
		{
			// Out of ammo
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			{
				WeaponSound(STOP);
#ifdef GAME_DLL
			if (m_hLaserDot != NULL)
			{
				UTIL_Remove(m_hLaserDot);
				m_hLaserDot = NULL;
			}
#endif
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
		if (m_hLaserDot != NULL)
		{
			UTIL_Remove(m_hLaserDot);
			m_hLaserDot = NULL;
		}
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
			m_hLaserDot = CFFWeaponLaserBeam::Create(GetAbsOrigin(), pOwner);
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