/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_sniperrifle.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF sniperrifle code.
///
/// REVISIONS
/// ---------
/// Jan 24, 2005 Mirv: First implementation


#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"
#include "in_buttons.h"
#include "Sprite.h"

#ifdef CLIENT_DLL
	#include "fx.h"
	#include "fx_sparks.h"
	#include "fx_line.h"
	#include "model_types.h"
#endif

//#define	RPG_LASER_SPRITE	"sprites/redglow1"

static int g_iBeam, g_iHalo;

#define SNIPER_BEAM		"effects/bluelaser1.vmt"
#define SNIPER_HALO		"sprites/muzzleflash1.vmt"
#define SNIPER_DOT		"sprites/sniperdot_red.vmt"


#ifdef CLIENT_DLL 
	#define CFFWeaponSniperRifle C_FFWeaponSniperRifle
	#define CFFWeaponLaserDot C_FFWeaponLaserDot
	#define CFFWeaponRadioTagRifle C_FFWeaponRadioTagRifle
	#include "c_ff_player.h"
	#include "view.h"

	#include "iviewrender_beams.h"
	#include "beam_shared.h"
	#include "beamdraw.h"
#else
	#include "ff_player.h"
#endif

//=============================================================================
// CFFWeaponLaserDot
//=============================================================================
class CFFWeaponLaserDot : public CSprite
{
public:
	DECLARE_CLASS(CFFWeaponLaserDot, CSprite);

	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

#ifdef CLIENT_DLL
	CFFWeaponLaserDot();
	virtual void GetRenderBounds(Vector& mins, Vector& maxs);
#endif

	static	CFFWeaponLaserDot *Create(const Vector &origin, CBaseEntity *pOwner = NULL);

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
// CFFWeaponLaserDot tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponLaserDot, DT_FFWeaponLaserDot) 

BEGIN_NETWORK_TABLE(CFFWeaponLaserDot, DT_FFWeaponLaserDot) 
#ifdef CLIENT_DLL
	RecvPropFloat(RECVINFO(m_flStartTime)) 
#else
	SendPropFloat(SENDINFO(m_flStartTime)) 
#endif
END_NETWORK_TABLE() 

LINK_ENTITY_TO_CLASS(env_fflaserdot, CFFWeaponLaserDot);

BEGIN_DATADESC(CFFWeaponLaserDot) 
	DEFINE_FIELD(m_bIsOn, 				FIELD_BOOLEAN), 
END_DATADESC() 

//=============================================================================
// CFFWeaponLaserDot implementation
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Create a laser dot
// Input  : &origin - 
// Output : CFFWeaponLaserDot
//-----------------------------------------------------------------------------
CFFWeaponLaserDot *CFFWeaponLaserDot::Create(const Vector &origin, CBaseEntity *pOwner) 
{
#ifdef GAME_DLL
	CFFWeaponLaserDot *pLaserDot = (CFFWeaponLaserDot *) CBaseEntity::Create("env_fflaserdot", origin, QAngle(0, 0, 0));

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
	pLaserDot->SetScale(0.25f);
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
void CFFWeaponLaserDot::SetLaserPosition(const Vector &origin) 
{
	SetAbsOrigin(origin);

	CFFPlayer *pOwner = ToFFPlayer(GetOwnerEntity());

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
	CFFWeaponLaserDot::CFFWeaponLaserDot()
	{
		m_pMaterial = materials->FindMaterial("effects/blueblacklargebeam", TEXTURE_GROUP_CLIENT_EFFECTS);
		m_pMaterial->IncrementReferenceCount();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Need to make sure that the render bounds include the source
	//			player too
	//-----------------------------------------------------------------------------
	void CFFWeaponLaserDot::GetRenderBounds(Vector& mins, Vector& maxs)
	{
		CFFPlayer *pOwner = ToFFPlayer(GetOwnerEntity());

		if (!pOwner)
		{
			return BaseClass::GetRenderBounds(mins, maxs);
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
	int CFFWeaponLaserDot::DrawModel(int flags) 
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

		CFFPlayer *pOwner = dynamic_cast<CFFPlayer *> (GetOwnerEntity());

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

			// Okay so beams. yes.
			if (!pOwner->IsLocalPlayer()) 
			{
				Vector v1 = tr.endpos - tr.startpos;
				Vector v2 = C_BasePlayer::GetLocalPlayer()->EyePosition() - tr.startpos;

				Vector vecCross = v1.Cross(v2);

				// Area of parallelogram
				float flLength = vecCross.Length();
				flLength /= v1.Length();

#define MAX_DIST	60.0f

				if (flLength < MAX_DIST)
				{
					float flVisibility = (MAX_DIST - flLength) / MAX_DIST;
					color32 colour = { 255, 0, 0, alpha * flVisibility };

					FX_DrawLine(tr.startpos, tr.endpos, flVisibility, m_pMaterial, colour);
				}
			}
		}
		else
		{
			// Just use our position if we can't predict it otherwise
			endPos = GetAbsOrigin();
		}

		// Randomly flutter
		//renderscale = 16.0f + random->RandomFloat(-2.0f, 2.0f);	
		renderscale = 0.20f + random->RandomFloat(-0.005f, 0.005f);

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
	void CFFWeaponLaserDot::OnDataChanged(DataUpdateType_t updateType) 
	{
		if (updateType == DATA_UPDATE_CREATED) 
		{
			SetNextClientThink( CLIENT_THINK_ALWAYS );
		}
	}
#endif

//=============================================================================
// CFFWeaponSniperRifle
//=============================================================================

class CFFWeaponSniperRifle : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponSniperRifle, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CFFWeaponSniperRifle();
	~CFFWeaponSniperRifle();

	virtual void Precache();
	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void WeaponIdle();
	virtual void Spawn();
	virtual void Fire();

	void ToggleZoom();

	virtual bool Deploy();
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);

	virtual void ItemPostFrame();
	virtual void ItemBusyFrame();

#ifdef CLIENT_DLL
	virtual float GetFOV();
#endif

	void UpdateLaserPosition();

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_SNIPERRIFLE; }

private:
	bool m_bZoomed;
	bool m_bInFire;

	CFFWeaponSniperRifle(const CFFWeaponSniperRifle &);

	void CheckZoomToggle();
	void CheckFire();

#ifdef GAME_DLL
	CHandle<CFFWeaponLaserDot>	m_hLaserDot;
#endif

	float m_flFireStartTime;

#ifdef CLIENT_DLL
	float m_flZoomTime;
	float m_flNextZoomTime;
#endif
};

//=============================================================================
// CFFWeaponSniperRifle tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponSniperRifle, DT_FFWeaponSniperRifle) 

BEGIN_NETWORK_TABLE(CFFWeaponSniperRifle, DT_FFWeaponSniperRifle) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponSniperRifle) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_sniperrifle, CFFWeaponSniperRifle);
PRECACHE_WEAPON_REGISTER(ff_weapon_sniperrifle);

//=============================================================================
// CFFWeaponSniperRifle implementation
//=============================================================================

CFFWeaponSniperRifle::CFFWeaponSniperRifle() 
{
	m_bZoomed = false;
	m_bInFire = false;
	m_flFireStartTime = 0.0f;

#ifdef CLIENT_DLL
	m_flNextZoomTime = m_flZoomTime = 0;
#endif
}

CFFWeaponSniperRifle::~CFFWeaponSniperRifle() 
{
#ifdef GAME_DLL
	if (m_hLaserDot != NULL) 
	{
		UTIL_Remove(m_hLaserDot);
		m_hLaserDot = NULL;
	}
#endif
}

void CFFWeaponSniperRifle::Precache() 
{
	BaseClass::Precache();

	// Laser dot...
	//PrecacheModel(RPG_LASER_SPRITE);

	PrecacheModel(SNIPER_DOT);
	g_iBeam = PrecacheModel(SNIPER_BEAM);
	g_iHalo = PrecacheModel(SNIPER_HALO);

	PrecacheScriptSound("SniperRifle.zoom_out");
	PrecacheScriptSound("SniperRifle.zoom_in");

	PrecacheScriptSound("Sniper.Hit");
	PrecacheScriptSound("Sniper.Gib");
}

bool CFFWeaponSniperRifle::Deploy() 
{
	m_bZoomed = false;

#ifdef CLIENT_DLL
	m_flNextZoomTime = m_flZoomTime = 0;
#endif

	return BaseClass::Deploy();
}

bool CFFWeaponSniperRifle::Holster(CBaseCombatWeapon *pSwitchingTo) 
{
	m_bZoomed = false;

#ifdef CLIENT_DLL
	m_flNextZoomTime = m_flZoomTime = 0;
	engine->ClientCmd("fov 0\n");
#endif

#ifdef GAME_DLL
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	if (pPlayer == NULL) 
		return BaseClass::Holster(pSwitchingTo);

	if (m_bInFire) 
	{
		// reset the data
		m_bInFire = false;
		const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();
		m_flNextPrimaryAttack = gpGlobals->curtime + pWeaponInfo.m_flCycleTime;

		// remove the laser as it's not needed anymore
		UTIL_Remove(m_hLaserDot);

		// and make the player go the right speed
		pPlayer->RemoveSpeedEffect(SE_SNIPERRIFLE);
	}

	// zoom back out when weapons are changed
	if (m_bZoomed) 
		ToggleZoom();
#endif

	return BaseClass::Holster(pSwitchingTo);
}

void CFFWeaponSniperRifle::PrimaryAttack() 
{
	// Yes stuff will go in here
	//BaseClass::PrimaryAttack();
	// See: ItemBusyFrame / ItemPostFrame
}

void CFFWeaponSniperRifle::SecondaryAttack() 
{
	/*
	DevMsg("CFFWeaponSniperRifle::SecondaryAttack\n");

	BaseClass::SecondaryAttack();

#ifdef GAME_DLL
	ToggleZoom();
#endif

	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
	*/
}

void CFFWeaponSniperRifle::Fire() 
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();	

	float flSniperRifleCharge = clamp(gpGlobals->curtime - m_flFireStartTime, 1, 7);	// Minimum charge is 1.
	//DevMsg("FIRE IN THE HOLE(%.2f multiplier) !\n", flSniperRifleCharge);

	// Does TFC have any sort of autoaiming for sniper rifle?
	// #define WEINER_SNIPER from tf_defs.h suggests perhaps
	// We could do something like the follows
	// (providing we fix ShouldAutoaim()):

	//QAngle angAiming;
	//VectorAngles(pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES), angAiming);

	FX_FireBullets(
		pPlayer->entindex(), 
		pPlayer->Weapon_ShootPosition(), 
		pPlayer->EyeAngles(), 
		GetWeaponID(), 
		Primary_Mode, 
		CBaseEntity::GetPredictionRandomSeed() & 255, 
		pWeaponInfo.m_flBulletSpread, 
		flSniperRifleCharge
		);

	// TODO: Maybe FX_FireBullets is not a good idea

	WeaponSound(SINGLE);
}

//-----------------------------------------------------------------------------
// Purpose: 
//			TODO: Really need to fix this!!!!
//-----------------------------------------------------------------------------
void CFFWeaponSniperRifle::ToggleZoom() 
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
	engine->ClientCmd(m_bZoomed ? "fov 20\n" : "fov 0\n");
#endif
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFWeaponSniperRifle::ItemBusyFrame() 
{
	// Allow zoom toggling even when we're reloading
	CheckZoomToggle();
	//CheckFire();

	// Bug fix for #0000210: When radio tag rifle out of ammo, weapon slot doesn't auto switch to new weapon
	BaseClass::ItemBusyFrame();
}

void CFFWeaponSniperRifle::ItemPostFrame() 
{
	// Allow zoom toggling
	CheckZoomToggle();
	CheckFire();

	// Bug fix for #0000210: When radio tag rifle out of ammo, weapon slot doesn't auto switch to new weapon
	// Added check so this doesn't go crazy.
	CFFPlayer *pPlayer = GetPlayerOwner();

	if (pPlayer && pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0) 
		HandleFireOnEmpty();
}


// ---------------------------------------------------------------------------
void CFFWeaponSniperRifle::CheckFire() 
{
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());
	
	// if we're currently firing, then check to see if we release
	if (m_bInFire) 
	{
		if (! (pPlayer->m_nButtons & IN_ATTACK)) 
		{
			// Make sure we're on the ground
			if (pPlayer->GetFlags() & FL_ONGROUND) 
			{
				// Fire!!
				Fire();

#ifdef GAME_DLL
				// remove the ammo
				pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);
#endif
			}
			else
				ClientPrint(dynamic_cast<CBasePlayer *> (GetOwnerEntity()), HUD_PRINTCENTER, "#FF_MUSTBEONGROUND");

			// reset the data
			m_bInFire = false;
			const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();
			m_flNextPrimaryAttack = gpGlobals->curtime + pWeaponInfo.m_flCycleTime;

			// remove the laser as it's not needed anymore
#ifdef GAME_DLL
			UTIL_Remove(m_hLaserDot);

			// and make the player go the right speed
			pPlayer->RemoveSpeedEffect(SE_SNIPERRIFLE);
#endif
		}
		else
		{
			// we need to update the laser
#ifdef GAME_DLL
			UpdateLaserPosition();
#endif
		}
	}
	else
	{
		if (pPlayer->m_nButtons & IN_ATTACK) 
		{
			// we shouldn't let them fire just yet
			if (m_flNextPrimaryAttack > gpGlobals->curtime) 
				return;

			// make sure they're not going too fast
			if (pPlayer->GetAbsVelocity().LengthSqr() > 400) 
				return;

			// don't allow charge when going up
			if (pPlayer->GetAbsVelocity().z > 0) 
				return;

			// make sure they have ammo
			if (pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0) 
				return;

			// make sure the player is slow
#ifdef GAME_DLL
			pPlayer->AddSpeedEffect(SE_SNIPERRIFLE, 999, .20, SEM_BOOLEAN);

			if (m_hLaserDot.Get()) 
				m_hLaserDot->m_flStartTime = gpGlobals->curtime;
#endif

			// set up us the variables!
			m_bInFire = true;
			m_flFireStartTime = gpGlobals->curtime;
		}
	}
}

void CFFWeaponSniperRifle::CheckZoomToggle() 
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	
	if (pPlayer->m_afButtonPressed & IN_ATTACK2) 
	{
		//DevMsg("[sniper rifle] Toggling Zoom!\n");
		ToggleZoom();
	}
}

void CFFWeaponSniperRifle::Spawn() 
{
	// Okay putting the predicted sprite creation in here doesn't work
	// because it can't find the owner, understandably.
	BaseClass::Spawn();
}

void CFFWeaponSniperRifle::WeaponIdle() 
{
	UpdateLaserPosition();
}

void CFFWeaponSniperRifle::UpdateLaserPosition() 
{
#ifdef GAME_DLL
	CFFPlayer *pPlayer = GetPlayerOwner();

/*
	CBaseViewModel *pBeamEnt = static_cast<CBaseViewModel *> (pOwner->GetViewModel());

	if (m_hLaserBeam == NULL) 
	{
		m_hLaserBeam = CBeam::BeamCreate(RPG_BEAM_SPRITE, 1.0f);

		if (m_hLaserBeam == NULL) 
		{
			// We were unable to create the beam
			Assert(0);
			return;
		}

		m_hLaserBeam->EntsInit(pBeamEnt, pBeamEnt);

		int     startAttachment = LookupAttachment("laser");
		int endAttachment       = LookupAttachment("laser_end");

		m_hLaserBeam->FollowEntity(pBeamEnt);
		m_hLaserBeam->SetStartAttachment(startAttachment);
		m_hLaserBeam->SetEndAttachment(endAttachment);
		m_hLaserBeam->SetNoise(0);
		m_hLaserBeam->SetColor(255, 0, 0);
		m_hLaserBeam->SetScrollRate(0);
		m_hLaserBeam->SetWidth(0.5f);
		m_hLaserBeam->SetEndWidth(0.5f);
		m_hLaserBeam->SetBrightness(128);
		m_hLaserBeam->SetBeamFlags(SF_BEAM_SHADEIN);
	}
	else
	{
		m_hLaserBeam->SetBrightness(128);
	}
*/
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
float CFFWeaponSniperRifle::GetFOV()
{
	C_FFPlayer *pPlayer = GetPlayerOwner();

	if (!pPlayer)
	{
		return NULL;
	}

	float deltaTime = (float) (gpGlobals->tickcount * gpGlobals->interval_per_tick - m_flZoomTime) * 5.0f;

	if (deltaTime < 1.0f)
	{
		// Random negative business
		if (deltaTime < 0)
		{
			return (m_bZoomed ? -1 : 20.0f);
		}

		float flFOV;

		if (m_bZoomed)
		{
			flFOV = SimpleSplineRemapVal(deltaTime, 0.0f, 1.0f, default_fov.GetFloat(), 20.0f);
		}
		else
		{
			flFOV = SimpleSplineRemapVal(deltaTime, 0.0f, 1.0f, 20.0f, default_fov.GetFloat());
		}

		return flFOV;
	}

	return (m_bZoomed ? 20.0f : -1);
}
#endif

//=============================================================================
// CFFWeaponRadioTagRifle
//=============================================================================

class CFFWeaponRadioTagRifle : public CFFWeaponSniperRifle
{
public:
	DECLARE_CLASS(CFFWeaponRadioTagRifle, CFFWeaponSniperRifle);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CFFWeaponRadioTagRifle() {};
	~CFFWeaponRadioTagRifle() {};

	// TODO: Temporary for now...
	/*
	virtual void Fire() 
	{
		CFFPlayer *pPlayer = GetPlayerOwner();
		const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

		FX_FireBullets(
			pPlayer->entindex(), 
			pPlayer->Weapon_ShootPosition(), 
			pPlayer->EyeAngles() + pPlayer->GetPunchAngle(), 
			GetWeaponID(), 
			Primary_Mode, 
			CBaseEntity::GetPredictionRandomSeed() & 255, 
			pWeaponInfo.m_flBulletSpread);
	}
	*/

	/*
	virtual void PrimaryAttack();
	virtual void WeaponIdle();
	virtual void Spawn();

	virtual bool Deploy();

	void UpdateLaserPosition();
	*/

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_RADIOTAGRIFLE; }

private:

	CFFWeaponRadioTagRifle(const CFFWeaponRadioTagRifle &);

	/*
#ifdef GAME_DLL
	CHandle< CFFWeaponLaserDot >	m_hLaserDot;
#endif
	*/

};

//=============================================================================
// CFFWeaponRadioTagRifle tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponRadioTagRifle, DT_FFWeaponRadioTagRifle) 

BEGIN_NETWORK_TABLE(CFFWeaponRadioTagRifle, DT_FFWeaponRadioTagRifle) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponRadioTagRifle) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_radiotagrifle, CFFWeaponRadioTagRifle);
PRECACHE_WEAPON_REGISTER(ff_weapon_radiotagrifle);

//=============================================================================
// CFFWeaponRadioTagRifle implementation
//=============================================================================