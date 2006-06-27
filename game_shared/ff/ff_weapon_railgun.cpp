/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_railgun.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF railgun code & class declaration
///
/// REVISIONS
/// ---------
/// Jan 19 2004 Mirv: First implementation


#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"
#include "ff_projectile_rail.h"
#include "in_buttons.h"

#define TEMP_SPRITE	"sprites/redglow1.vmt"
#define RAIL_BEAM	"effects/blueblacklargebeam"

#ifdef CLIENT_DLL 
	#define CFFWeaponRailgun C_FFWeaponRailgun
	#define CFFRailBeam C_FFRailBeam

	#include "c_ff_player.h"
	#include "fx.h"
	#include "fx_sparks.h"
	#include "fx_line.h"
#else
	#include "ff_player.h"
#endif

//=============================================================================
// CFFRailBeam
//=============================================================================

class CFFRailBeam : public CSprite
{
public:
	DECLARE_CLASS(CFFRailBeam, CSprite);
	DECLARE_NETWORKCLASS(); 

	CFFRailBeam()
	{
		m_flFired = -1.0f;
	}

#ifdef CLIENT_DLL
	virtual bool			IsTransparent() { return true; }
	virtual RenderGroup_t	GetRenderGroup() { return RENDER_GROUP_TRANSLUCENT_ENTITY; }
	virtual bool			ShouldDraw() { return (IsEffectActive(EF_NODRAW) == false); }

	// Returns the bounds relative to the origin (render bounds)
	virtual void GetRenderBounds(Vector& mins, Vector& maxs)
	{
		// nasty temp measure
		ClearBounds(mins, maxs);
		/*AddPointToBounds(m_vecStartPosition, mins, maxs);
		AddPointToBounds(m_vecEndPosition, mins, maxs);
		mins -= GetRenderOrigin();
		maxs -= GetRenderOrigin();*/
	}

	virtual void OnDataChanged( DataUpdateType_t updateType )
	{
		CBaseEntity::OnDataChanged( updateType );
	}

	virtual int	DrawModel(int flags)
	{
		// Only show the beam for a brief time
		if (m_flFired + 0.6f < gpGlobals->curtime)
		{
			m_fJustFired = true;
			return 0;
		}

		// Materials
		IMaterial *pMat = materials->FindMaterial("effects/blueblacklargebeam", TEXTURE_GROUP_CLIENT_EFFECTS);
		materials->Bind(pMat);

		// Update locations n stuff
		if (m_fJustFired)
		{
			m_fJustFired = false;

			C_BaseAnimating *pWeapon = NULL;
			QAngle angDirection;

			C_FFPlayer *pPlayer = ToFFPlayer(GetOwnerEntity());

			if (!pPlayer)
			{
				Assert("Non-FFPlayer firing weapon??");
				return 1;
			}

			// Use the correct weapon model
			if (pPlayer->IsLocalPlayer())
				pWeapon = pPlayer->GetViewModel(0);
			else
				pWeapon = pPlayer->GetActiveWeapon();

			// Get the attachment(precache this number sometime)
			if (pWeapon)
			{
				int iAttachment = pWeapon->LookupAttachment("muzzle");
				pWeapon->GetAttachment(iAttachment, m_vecStartPosition, angDirection);
			}
			else
				AssertMsg(0, "Couldn't get weapon!");

			Vector vecDirection;
			AngleVectors(angDirection, &vecDirection);

			trace_t tr;
			UTIL_TraceLine(m_vecStartPosition, m_vecStartPosition + (vecDirection * MAX_TRACE_LENGTH), MASK_SHOT, NULL, COLLISION_GROUP_NONE, &tr);

			m_vecEndPosition = tr.endpos;
		}

		// We actually stay fullbright for half our visible time
		int alpha = 255 * ((0.6f - (gpGlobals->curtime - m_flFired)) / 0.3f);
		alpha = clamp(alpha, 0, 255);

		// Perhaps thickness could depend on power
		color32 colour = { alpha, alpha, alpha, alpha };
		FX_DrawLineFade(m_vecStartPosition, m_vecEndPosition, 3.0f, pMat, colour, 8.0f);

		return 1;
	}
#endif

public:

#ifdef CLIENT_DLL
	bool m_fJustFired;
	Vector		m_vecStartPosition, m_vecEndPosition;
#endif

	CNetworkVar(float, m_flFired);
};

IMPLEMENT_NETWORKCLASS_ALIASED(FFRailBeam, DT_FFRailBeam)

BEGIN_NETWORK_TABLE(CFFRailBeam, DT_FFRailBeam)
#ifdef CLIENT_DLL
	RecvPropFloat(RECVINFO(m_flFired))
#else
	SendPropFloat(SENDINFO(m_flFired))
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(env_railbeam, CFFRailBeam);

//=============================================================================
// CFFWeaponRailgun
//=============================================================================

class CFFWeaponRailgun : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponRailgun, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponRailgun();

	virtual void	Fire();
	virtual void	ItemPostFrame();
	virtual bool	Deploy();
	virtual void	Precache();

	CNetworkHandle(CFFRailBeam, m_hRailBeam);
	CNetworkVar(float, m_flStartCharge);

	virtual FFWeaponID GetWeaponID() const { return FF_WEAPON_RAILGUN; }

private:

	CFFWeaponRailgun(const CFFWeaponRailgun &);
};

//=============================================================================
// CFFWeaponRailgun tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponRailgun, DT_FFWeaponRailgun)

BEGIN_NETWORK_TABLE(CFFWeaponRailgun, DT_FFWeaponRailgun)
#ifdef GAME_DLL
	SendPropEHandle(SENDINFO(m_hRailBeam)), 
	SendPropTime(SENDINFO(m_flStartCharge)), 
#else
	RecvPropEHandle(RECVINFO(m_hRailBeam)), 
	RecvPropTime(RECVINFO(m_flStartCharge)), 
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CFFWeaponRailgun)
DEFINE_PRED_FIELD_TOL(m_flStartCharge, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE), 
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(ff_weapon_railgun, CFFWeaponRailgun);
PRECACHE_WEAPON_REGISTER(ff_weapon_railgun);

//=============================================================================
// CFFWeaponRailgun implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponRailgun::CFFWeaponRailgun()
{
	m_flStartCharge = -1.0f;
	m_hRailBeam = NULL;
}

//----------------------------------------------------------------------------
// Purpose: Precache stuff
//----------------------------------------------------------------------------
void CFFWeaponRailgun::Precache()
{
	PrecacheModel(TEMP_SPRITE);
	PrecacheModel(RAIL_BEAM);

	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Create the beam entity
//----------------------------------------------------------------------------
bool CFFWeaponRailgun::Deploy()
{
#ifdef GAME_DLL
	// Flamejet entity doesn't exist yet, so make it now
	if (!m_hRailBeam)
	{
		CFFPlayer *pPlayer = GetPlayerOwner();
		QAngle angAiming;

		VectorAngles(pPlayer->GetAutoaimVector(0), angAiming);

		// Create a flamejet emitter
		m_hRailBeam = dynamic_cast<CFFRailBeam *> (CBaseEntity::Create("env_railbeam", pPlayer->Weapon_ShootPosition(), angAiming, pPlayer));

		// A huge bunch of things, possibly not all needed
		//m_hRailBeam->SetRenderMode((RenderMode_t) 9);
		m_hRailBeam->SetMoveType(MOVETYPE_NONE);
		m_hRailBeam->AddSolidFlags(FSOLID_NOT_SOLID);
		m_hRailBeam->AddEffects(EF_NOSHADOW);
		m_hRailBeam->AddEFlags(EFL_FORCE_CHECK_TRANSMIT);
		m_hRailBeam->SpriteInit(TEMP_SPRITE, pPlayer->Weapon_ShootPosition());	// just give it a fake sprite
		//m_hRailBeam->SetName(AllocPooledString("RAILBEAM"));
		//m_hRailBeam->SetTransparency(kRenderWorldGlow, 255, 255, 255, 255, kRenderFxNoDissipation);
		//m_hRailBeam->SetScale(0.25f);
		m_hRailBeam->SetSimulatedEveryTick(true);

		UTIL_SetSize(m_hRailBeam, vec3_origin, vec3_origin);
	}
#endif

	return BaseClass::Deploy();
}

//----------------------------------------------------------------------------
// Purpose: Fire a rail
//----------------------------------------------------------------------------
void CFFWeaponRailgun::Fire()
{
	CFFPlayer *pPlayer = GetPlayerOwner();
	const CFFWeaponInfo &pWeaponInfo = GetFFWpnData();

	Vector	vecForward;
	pPlayer->EyeVectors(&vecForward);

	Vector	vecSrc = pPlayer->Weapon_ShootPosition();

#ifdef CLIENT_DLL
	// For now, fake the bullet source on the client
	C_BaseAnimating *pWeapon;

	// Use the correct weapon model
	if (pPlayer->IsLocalPlayer())
		pWeapon = pPlayer->GetViewModel(0);
	else
		pWeapon = pPlayer->GetActiveWeapon();

	// Get the attachment(precache this number sometime)
	if (pWeapon)
	{
		QAngle angAiming;
		int iAttachment = pWeapon->LookupAttachment("muzzle");
		pWeapon->GetAttachment(iAttachment, vecSrc, angAiming);

		AngleVectors(angAiming, &vecForward);
	}
	else
		AssertMsg(0, "Couldn't get weapon!");
#endif

	QAngle angAiming;
	VectorAngles(pPlayer->GetAutoaimVector(0), angAiming);

	// Trigger the railbeam visual effect
	if (m_hRailBeam)
	{
		m_hRailBeam->SetAbsOrigin(GetAbsOrigin());
		m_hRailBeam->m_flFired = gpGlobals->curtime;
	}

	float flChargeTime = gpGlobals->curtime - m_flStartCharge;

	// Simulate this as a bullet for now
	FireBulletsInfo_t info(1, vecSrc, vecForward, vec3_origin, MAX_TRACE_LENGTH, m_iPrimaryAmmoType);
	info.m_pAttacker = pPlayer;
	info.m_iDamage = pWeaponInfo.m_iDamage + (flChargeTime * 3.0f);
	info.m_iTracerFreq = 0;
	info.m_flDamageForceScale = 1.0f + (flChargeTime * 100.0f);

	pPlayer->FireBullets(info);
}

void CFFWeaponRailgun::ItemPostFrame()
{
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	if (pPlayer && pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0)
		HandleFireOnEmpty();

	// if we're currently firing, then check to see if we release

	if (pPlayer->m_nButtons & IN_ATTACK)
	{
		// Not currently charging
		if (m_flStartCharge < 0)
		{
			// we shouldn't let them fire just yet
			if (m_flNextPrimaryAttack > gpGlobals->curtime)
				return;

			// make sure they have ammo
			if (pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0)
				return;

			m_flStartCharge = gpGlobals->curtime;
		}
	}
	else
	{
		if (m_flStartCharge > 0)
		{
			WeaponSound(SINGLE);

			pPlayer->DoMuzzleFlash();

			SendWeaponAnim(GetPrimaryAttackActivity());

			// player "shoot" animation
			pPlayer->SetAnimation(PLAYER_ATTACK1);

			float flPower = gpGlobals->curtime - m_flStartCharge;
			pPlayer->ViewPunch(-QAngle(1.0f + flPower, 0, 0));

			// Fire!!
			Fire();

			// remove the ammo
#ifdef GAME_DLL
			pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);
#endif

			m_flNextPrimaryAttack = gpGlobals->curtime + GetFFWpnData().m_flCycleTime;
		}

		m_flStartCharge = -1.0f;
	}
}