//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

// Mirv (2006/03/11) A lot of this has been rewritten for support of 4 prongs

#include "cbase.h"
#include "hud.h"
#include "text_message.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "view.h"
#include <KeyValues.h>
#include <vgui_controls/AnimationController.h>
#include <vgui/ISurface.h>
#include "vguimatsurface/IMatSystemSurface.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMesh.h"
#include "materialsystem/imaterialvar.h"
#include "ieffects.h"
#include "hudelement.h"
#include "ClientEffectPrecacheSystem.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//-----------------------------------------------------------------------------
// Purpose: HDU Damage indication
//-----------------------------------------------------------------------------
class CHudDamageIndicator : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudDamageIndicator, vgui::Panel);

public:
	CHudDamageIndicator(const char *pElementName);
	void Init();
	void Reset();
	virtual bool ShouldDraw();

	// Handler for our message
	void MsgFunc_Damage(bf_read &msg);

private:
	virtual void Paint();
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	CPanelAnimationVarAliasType(float, m_flMarginX, "dmg_xmargin", "40", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flMarginY, "dmg_ymargin", "40", "proportional_float");

	CPanelAnimationVarAliasType(float, m_flDmgDepth, "dmg_depth", "40", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flDmgOuterLength, "dmg_outerlength", "300", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flDmgInnerLength, "dmg_innerlength", "240", "proportional_float");

	CPanelAnimationVar(Color, m_DmgColorLeft, "DmgColorLeft", "255 0 0 0");
	CPanelAnimationVar(Color, m_DmgColorRight, "DmgColorRight", "255 0 0 0");
	CPanelAnimationVar(Color, m_DmgColorFront, "DmgColorFront", "255 0 0 0");
	CPanelAnimationVar(Color, m_DmgColorBehind, "DmgColorBehind", "255 0 0 0");

	void DrawDamageIndicator(int side);
	void DrawFullscreenDamageIndicator();
	void GetDamagePosition(const Vector &vecDelta, float *flRotation);

	CMaterialReference m_WhiteAdditiveMaterial;
};

DECLARE_HUDELEMENT(CHudDamageIndicator);
DECLARE_HUD_MESSAGE(CHudDamageIndicator, Damage);

enum
{
	DAMAGE_ANY, 
	DAMAGE_LOW, 
	DAMAGE_HIGH, 
};

#define ANGLE_ANY	0.0f
#define DMG_ANY		0

struct DamageAnimation_t
{
	const char *name;
	int bitsDamage;
	float angleMinimum;
	float angleMaximum;
	int damage; 
};

//-----------------------------------------------------------------------------
// Purpose: List of damage animations, finds first that matches criteria
//-----------------------------------------------------------------------------
static DamageAnimation_t g_DamageAnimations[] =
{
	{ "HudTakeDamageDrown", 		DMG_DROWN, 	ANGLE_ANY, 	ANGLE_ANY, 	DAMAGE_ANY }, 
	{ "HudTakeDamagePoison", 		DMG_POISON, ANGLE_ANY, 	ANGLE_ANY, 	DAMAGE_ANY }, 
//	{ "HudTakeDamageBurn", 			DMG_BURN, 	ANGLE_ANY, 	ANGLE_ANY, 	DAMAGE_ANY }, 
	{ "HudTakeDamageRadiation", 	DMG_RADIATION, 	ANGLE_ANY, 	ANGLE_ANY, 	DAMAGE_ANY }, 
	{ "HudTakeDamageRadiation", 	DMG_ACID, 	ANGLE_ANY, 	ANGLE_ANY, 	DAMAGE_ANY }, 

	{ "HudTakeDamageHighLeft", 		DMG_ANY, 	45.0f, 			135.0f, 		DAMAGE_HIGH }, 
	{ "HudTakeDamageHighRight", 	DMG_ANY, 	225.0f, 		315.0f, 		DAMAGE_HIGH }, 
	{ "HudTakeDamageHighBehind", 	DMG_ANY, 	135.0f, 		225.0f, 		DAMAGE_HIGH }, 
	{ "HudTakeDamageHighFront", 	DMG_ANY, 	ANGLE_ANY, 		ANGLE_ANY, 		DAMAGE_HIGH }, 

	{ "HudTakeDamageMidLeft", 		DMG_ANY, 	45.0f, 			135.0f, 		DAMAGE_LOW }, 
	{ "HudTakeDamageMidRight", 	    DMG_ANY, 	225.0f, 		315.0f, 		DAMAGE_LOW }, 
	{ "HudTakeDamageMidBehind", 	DMG_ANY, 	135.0f, 		225.0f, 		DAMAGE_LOW }, 
	{ "HudTakeDamageMidFront", 		DMG_ANY, 	ANGLE_ANY, 		ANGLE_ANY, 		DAMAGE_LOW }, 
	
	{ "HudTakeDamageLeft", 			DMG_ANY, 	45.0f, 			135.0f, 		DAMAGE_ANY }, 
	{ "HudTakeDamageRight", 		DMG_ANY, 	225.0f, 		315.0f, 		DAMAGE_ANY }, 
	{ "HudTakeDamageBehind", 		DMG_ANY, 	135.0f, 		225.0f, 		DAMAGE_ANY }, 

	// fall through to front damage
	{ "HudTakeDamageFront", 		DMG_ANY, 	ANGLE_ANY, 	ANGLE_ANY, 	DAMAGE_ANY }, 
	{ NULL }, 
};


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudDamageIndicator::CHudDamageIndicator(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudDamageIndicator")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_WhiteAdditiveMaterial.Init("vgui/white_additive", TEXTURE_GROUP_VGUI); 
	
	SetHiddenBits(HIDEHUD_HEALTH);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDamageIndicator::Reset()
{
	m_DmgColorLeft[3] = 0;
	m_DmgColorRight[3] = 0;
	m_DmgColorFront[3] = 0;
	m_DmgColorBehind[3] = 0;
}

void CHudDamageIndicator::Init()
{
	HOOK_HUD_MESSAGE(CHudDamageIndicator, Damage);
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudDamageIndicator::ShouldDraw()
{
	if (!CHudElement::ShouldDraw())
		return false;

	if (!m_DmgColorLeft[3] && !m_DmgColorRight[3] && !m_DmgColorFront[3] && !m_DmgColorBehind[3])
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Draws a damage quad
//-----------------------------------------------------------------------------
void CHudDamageIndicator::DrawDamageIndicator(int side)
{
	IMesh *pMesh = materials->GetDynamicMesh(true, NULL, NULL, m_WhiteAdditiveMaterial);

	CMeshBuilder meshBuilder;
	meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);

	int iX, iY, iWidth, iTall;
	GetParent()->GetBounds(iX, iY, iWidth, iTall);

	int x1 = m_flMarginX;
	int x2 = m_flMarginX + m_flDmgDepth;
	int y[4] = { (iTall / 2) - (m_flDmgOuterLength / 2), (iTall / 2) - (m_flDmgInnerLength / 2), (iTall / 2) + (m_flDmgInnerLength / 2), (iTall / 2) + (m_flDmgOuterLength / 2) };
	int alpha[4] = { 0.0f, 1.0f, 1.0f, 0.0f };

	int r, g, b, a;

	if (side == 0)
	{
		r = m_DmgColorLeft[0], g = m_DmgColorLeft[1], b = m_DmgColorLeft[2], a = m_DmgColorLeft[3];

		meshBuilder.Color4ub(r, g, b, a * alpha[0]);
		meshBuilder.TexCoord2f(0, 0, 0);
		meshBuilder.Position3f(x1, y[0], 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub(r, g, b, a * alpha[1]);
		meshBuilder.TexCoord2f(0, 1, 0);
		meshBuilder.Position3f(x2, y[1], 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub(r, g, b, a * alpha[2]);
		meshBuilder.TexCoord2f(0, 1, 1);
		meshBuilder.Position3f(x2, y[2], 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub(r, g, b, a * alpha[3]);
		meshBuilder.TexCoord2f(0, 0, 1);
		meshBuilder.Position3f(x1, y[3], 0);
		meshBuilder.AdvanceVertex();
	}
	else if (side == 1)
	{
		r = m_DmgColorRight[0], g = m_DmgColorRight[1], b = m_DmgColorRight[2], a = m_DmgColorRight[3];

		// realign x coords
		x1 = GetWide() - x1;
		x2 = GetWide() - x2;

		meshBuilder.Color4ub(r, g, b, a * alpha[0]);
		meshBuilder.TexCoord2f(0, 0, 0);
		meshBuilder.Position3f(x1, y[0], 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub(r, g, b, a * alpha[3]);
		meshBuilder.TexCoord2f(0, 0, 1);
		meshBuilder.Position3f(x1, y[3], 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub(r, g, b, a * alpha[2]);
		meshBuilder.TexCoord2f(0, 1, 1);
		meshBuilder.Position3f(x2, y[2], 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub(r, g, b, a * alpha[1]);
		meshBuilder.TexCoord2f(0, 1, 0);
		meshBuilder.Position3f(x2, y[1], 0);
		meshBuilder.AdvanceVertex();

	}
	else if (side == 2)
	{
		r = m_DmgColorFront[0], g = m_DmgColorFront[1], b = m_DmgColorFront[2], a = m_DmgColorFront[3];

		int y1 = m_flMarginY;
		int y2 = m_flMarginY + m_flDmgDepth;

		int x[4] = { (iWidth / 2) - (m_flDmgOuterLength / 2), (iWidth / 2) - (m_flDmgInnerLength / 2), (iWidth / 2) + (m_flDmgInnerLength / 2), (iWidth / 2) + (m_flDmgOuterLength / 2) };
		int alpha[4] = { 0.0f, 1.0f, 1.0f, 0.0f };

		meshBuilder.Color4ub(r, g, b, a * alpha[0]);
		meshBuilder.TexCoord2f(0, 0, 0);
		meshBuilder.Position3f(x[0], y1, 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub(r, g, b, a * alpha[3]);
		meshBuilder.TexCoord2f(0, 1, 0);
		meshBuilder.Position3f(x[3], y1, 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub(r, g, b, a * alpha[2]);
		meshBuilder.TexCoord2f(0, 1, 1);
		meshBuilder.Position3f(x[2], y2, 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub(r, g, b, a * alpha[1]);
		meshBuilder.TexCoord2f(0, 0, 1);
		meshBuilder.Position3f(x[1], y2, 0);
		meshBuilder.AdvanceVertex();
	}
	else if (side == 3)
	{
		r = m_DmgColorBehind[0], g = m_DmgColorBehind[1], b = m_DmgColorBehind[2], a = m_DmgColorBehind[3];

		int y1 = m_flMarginY;
		int y2 = m_flMarginY + m_flDmgDepth;
		int x[4] = { (iWidth / 2) - (m_flDmgOuterLength / 2), (iWidth / 2) - (m_flDmgInnerLength / 2), (iWidth / 2) + (m_flDmgInnerLength / 2), (iWidth / 2) + (m_flDmgOuterLength / 2) };
		int alpha[4] = { 0.0f, 1.0f, 1.0f, 0.0f };

		// realign x coords
		y1 = GetTall() - y1;
		y2 = GetTall() - y2;

		meshBuilder.Color4ub(r, g, b, a * alpha[0]);
		meshBuilder.TexCoord2f(0, 0, 0);
		meshBuilder.Position3f(x[0], y1, 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub(r, g, b, a * alpha[1]);
		meshBuilder.TexCoord2f(0, 1, 0);
		meshBuilder.Position3f(x[1], y2, 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub(r, g, b, a * alpha[2]);
		meshBuilder.TexCoord2f(0, 1, 1);
		meshBuilder.Position3f(x[2], y2, 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub(r, g, b, a * alpha[3]);
		meshBuilder.TexCoord2f(0, 0, 1);
		meshBuilder.Position3f(x[3], y1, 0);
		meshBuilder.AdvanceVertex();
	}

	meshBuilder.End();
	pMesh->Draw();
}

//-----------------------------------------------------------------------------
// Purpose: Paints the damage display
//-----------------------------------------------------------------------------
void CHudDamageIndicator::Paint()
{
	// draw side damage indicators
	DrawDamageIndicator(0);
	DrawDamageIndicator(1);
	DrawDamageIndicator(2);
	DrawDamageIndicator(3);
}

//-----------------------------------------------------------------------------
// Purpose: Message handler for Damage message
//-----------------------------------------------------------------------------
void CHudDamageIndicator::MsgFunc_Damage(bf_read &msg)
{
	int armor = msg.ReadByte();	// armor
	int damageTaken = msg.ReadByte();	// health
	int totalDamageTaken = armor + damageTaken;
	long bitsDamage = msg.ReadLong(); // damage bits

	Vector vecFrom;

	vecFrom.x = msg.ReadFloat();
	vecFrom.y = msg.ReadFloat();
	vecFrom.z = msg.ReadFloat();

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	// player has just died, just run the dead damage animation
	if (pPlayer->GetHealth() <= 0)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HudPlayerDeath");
		return;
	}

	// UNDONE: ignore damage without direction
	// this should never happen, unless it's drowning damage, 
	// or the player is forcibly killed, handled above
	if (vecFrom == vec3_origin && ! (bitsDamage & DMG_DROWN))
	{
		if (totalDamageTaken > 0)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HudTakeDamageAll");
		}
		
		return;
	}

	Vector vecDelta = (vecFrom - MainViewOrigin());
	VectorNormalize(vecDelta);

	int damageType = DAMAGE_ANY;
	if (totalDamageTaken > 60)
	{
		damageType = DAMAGE_HIGH;
	}
	else if (totalDamageTaken > 30)
	{
		damageType = DAMAGE_LOW;
	}
/*
	// if we have no suit, all damage is high
	if (!pPlayer->IsSuitEquipped())
	{
		damageType = DAMAGE_HIGH;
	}*/

	if (totalDamageTaken > 0)
	{
		// see which quandrant the effect is in
		float angle;
		GetDamagePosition(vecDelta, &angle);

		// see which effect to play
		DamageAnimation_t *dmgAnim = g_DamageAnimations;
		for (; dmgAnim->name != NULL; ++dmgAnim)
		{
			// If this anim needs special damage type and this damage doesnt match that type
			if (dmgAnim->bitsDamage && ! (bitsDamage & dmgAnim->bitsDamage)) 
				continue;

			if (dmgAnim->angleMinimum && angle < dmgAnim->angleMinimum)
				continue;

			if (dmgAnim->angleMaximum && angle > dmgAnim->angleMaximum)
				continue;

			// If this anim requires at least a decent amount of damage, and the damage isnt correct
			if (dmgAnim->damage && dmgAnim->damage != damageType)
				continue;

			// we have a match, break
			break;
		}

		if (dmgAnim->name)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence(dmgAnim->name);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Convert a damage position in world units to the screen's units
//-----------------------------------------------------------------------------
void CHudDamageIndicator::GetDamagePosition(const Vector &vecDelta, float *flRotation)
{
	float flRadius = 360.0f;

	// Player Data
	Vector playerPosition = MainViewOrigin();
	QAngle playerAngles = MainViewAngles();

	Vector forward, right, up(0, 0, 1);
	AngleVectors(playerAngles, &forward, NULL, NULL);
	forward.z = 0;
	VectorNormalize(forward);
	CrossProduct(up, forward, right);
	float front = DotProduct(vecDelta, forward);
	float side = DotProduct(vecDelta, right);
	float xpos = flRadius * -side;
	float ypos = flRadius * -front;

	// Get the rotation(yaw)
	*flRotation = atan2(xpos, ypos) + M_PI;
	*flRotation *= 180 / M_PI;

	float yawRadians = - (*flRotation) * M_PI / 180.0f;
	float ca = cos(yawRadians);
	float sa = sin(yawRadians);
				 
	// Rotate it around the circle
	xpos = (int) ((ScreenWidth() / 2) + (flRadius * sa));
	ypos = (int) ((ScreenHeight() / 2) - (flRadius * ca));
}

//-----------------------------------------------------------------------------
// Purpose: hud scheme settings
//-----------------------------------------------------------------------------
void CHudDamageIndicator::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);

	int wide, tall;
	GetHudSize(wide, tall);
	SetSize(wide, tall);
}
