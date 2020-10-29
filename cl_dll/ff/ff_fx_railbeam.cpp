/********************************************************************
	created:	2006/07/04
	created:	4:7:2006   13:51
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\ff_fx_railbeam.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll\ff
	file base:	ff_fx_railbeam
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Rail beam
*********************************************************************/

#include "cbase.h"
#include "ff_fx_railbeam.h"
#include "c_te_effect_dispatch.h"
#include "ClientEffectPrecacheSystem.h"
#include "view.h"
#include "iinput.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define RAILBEAM_MATERIAL	"effects/railbeam"
#define FIRE_TIME			0.6f

CLIENTEFFECT_REGISTER_BEGIN(PrecacheRailbeamMaterial)
CLIENTEFFECT_MATERIAL(RAILBEAM_MATERIAL)
CLIENTEFFECT_REGISTER_END()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFFXRailBeam::CFFFXRailBeam(const char *name, const Vector &start, const Vector &end, float scale) : CClientSideEffect(name)
{
	assert(materials);
	if (materials == NULL)
		return;

	PrecacheMaterial(RAILBEAM_MATERIAL);

	// Create a material...
	m_pMaterial = materials->FindMaterial(RAILBEAM_MATERIAL, TEXTURE_GROUP_CLIENT_EFFECTS);
	m_pMaterial->IncrementReferenceCount();

	m_vecOrigin			= start;
	m_vecTarget			= end;
	m_flScale			= scale;
	m_flStartTime		= gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CFFFXRailBeam::~CFFFXRailBeam()
{
	Destroy();
}

//-----------------------------------------------------------------------------
// Purpose: Draw rail beam
//-----------------------------------------------------------------------------
void CFFFXRailBeam::Draw(double frametime)
{
	// We actually stay fullbright for half our visible time
	int alpha = 255 * ((FIRE_TIME - (gpGlobals->curtime - m_flStartTime)) / 0.3f);
	alpha = clamp(alpha, 0, 255);

	// Perhaps thickness could depend on power
	color32 colour = { alpha, alpha, alpha, alpha };
	FX_DrawLineFade(m_vecOrigin, m_vecTarget, 3.0f, m_pMaterial, colour, 8.0f);
}

//-----------------------------------------------------------------------------
// Purpose: Is this rail beam still active
//-----------------------------------------------------------------------------
bool CFFFXRailBeam::IsActive()
{
	return (m_flStartTime + FIRE_TIME > gpGlobals->curtime);
}

//-----------------------------------------------------------------------------
// Purpose: Destroy rail beam
//-----------------------------------------------------------------------------
void CFFFXRailBeam::Destroy()
{
	//Release the material
	if (m_pMaterial != NULL)
	{
		m_pMaterial->DecrementReferenceCount();
		m_pMaterial = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Update
//-----------------------------------------------------------------------------
void CFFFXRailBeam::Update(double frametime)
{
}

//-----------------------------------------------------------------------------
// Purpose: Add a rail beam
//-----------------------------------------------------------------------------
void FX_RailBeam(int entindex, const Vector &target, float scale)
{
	C_BasePlayer *pPlayer = UTIL_PlayerByIndex(entindex);

	if (!pPlayer)
	{
		AssertMsg(0, "No player for this rail beam");
		return;
	}

	C_BaseAnimating *pWeapon = NULL;

	// Use the correct weapon model
	if (pPlayer->IsLocalPlayer() && !input->CAM_IsThirdPerson())
		pWeapon = pPlayer->GetViewModel(0);
	else
		pWeapon = pPlayer->GetActiveWeapon();

	QAngle angDirection;
	Vector m_vecStartPosition;

	// Get the attachment (precache this number sometime)
	if (pWeapon)
	{
		int iAttachment = pWeapon->LookupAttachment("1");
		pWeapon->GetAttachment(iAttachment, m_vecStartPosition, angDirection);
	}
	else
		m_vecStartPosition = pPlayer->Weapon_ShootPosition();

	Vector vecDirection;
	AngleVectors(angDirection, &vecDirection);

	// Check that this isn't going through a wall
	trace_t trWall;
	UTIL_TraceLine( pPlayer->GetAbsOrigin(), m_vecStartPosition, MASK_SOLID_BRUSHONLY, pPlayer, COLLISION_GROUP_NONE, &trWall );

	// Yes, going through a wall
	if( trWall.fraction < 1.0f )
		return;

	trace_t tr;
	UTIL_TraceLine(m_vecStartPosition, m_vecStartPosition + (vecDirection * MAX_TRACE_LENGTH), MASK_SHOT, NULL, COLLISION_GROUP_NONE, &tr);

	// Finally create the beam
	CFFFXRailBeam *t = new CFFFXRailBeam("RailBeam", m_vecStartPosition, tr.endpos, scale);
	assert(t);

	//Throw it into the list
	clienteffects->AddEffect(t);
}

//-----------------------------------------------------------------------------
// Purpose: Callback function for a rail beam
//-----------------------------------------------------------------------------
void RailBeamCallback(const CEffectData &data)
{
	FX_RailBeam(
#ifdef GAME_DLL
		data.m_nEntIndex
#else
		data.m_hEntity.GetEntryIndex()
#endif
		, data.m_vOrigin, data.m_flScale);
}

DECLARE_CLIENT_EFFECT("RailBeam", RailBeamCallback);