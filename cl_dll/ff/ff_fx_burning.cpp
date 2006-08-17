/********************************************************************
	created:	2006/08/16
	created:	16:8:2006   12:16
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\ff_fx_infection.h
	file path:	f:\ff-svn\code\trunk\cl_dll\ff
	file base:	ff_fx_infection
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

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
#include "materialsystem/IMaterialSystemHardwareConfig.h"
#include "ScreenSpaceEffects.h"

	// There's a few too many .h files here!

class CBurningEffect : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CBurningEffect, vgui::Panel);
	
	CBurningEffect(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudDamageIndicator")
	{
		m_flTargetAmount = m_flAmount = 0;

		// Set our parent window
		SetParent(g_pClientMode->GetViewport());

		// Hide when player is dead
		SetHiddenBits(HIDEHUD_PLAYERDEAD);
	}

	virtual void Init();
	virtual void VidInit();
	virtual bool ShouldDraw();
	virtual void Paint();

	void MsgFunc_BurningEffect(bf_read &msg);
	void MsgFunc_InfectedEffect(bf_read &msg);
	void MsgFunc_TranquilizedEffect(bf_read &msg);

private:

	float m_flAmount, m_flTargetAmount;
	CMaterialReference m_WhiteAdditiveMaterial;
};

DECLARE_HUDELEMENT(CBurningEffect);
DECLARE_HUD_MESSAGE(CBurningEffect, BurningEffect);

//-----------------------------------------------------------------------------
// Purpose: Hook the hud message so that it exists & sort the texture
//-----------------------------------------------------------------------------
void CBurningEffect::Init()
{
	HOOK_HUD_MESSAGE(CBurningEffect, BurningEffect);

	m_WhiteAdditiveMaterial.Init("vgui/white_additive", TEXTURE_GROUP_VGUI);
}

//-----------------------------------------------------------------------------
// Purpose: Make sure this is enabled + covers all the screen
//-----------------------------------------------------------------------------
void CBurningEffect::VidInit()
{
	SetPos(0, 0);
	SetWide(GetParent()->GetWide());
	SetTall(GetParent()->GetTall());

	SetEnabled(true);
	SetVisible(true);

	SetPaintEnabled(true);

	m_flAmount = m_flTargetAmount = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Only draw when there's something to show
//-----------------------------------------------------------------------------
bool CBurningEffect::ShouldDraw()
{
	return (m_flAmount > 0.0f || m_flTargetAmount > 0.0f);
}

//-----------------------------------------------------------------------------
// Purpose: Paint two quads on either side of the screen.
//			Make sure we have some linear progression up to the new target
//			amount so that it appears gracefully.
//			TODO: Take into account fps
//-----------------------------------------------------------------------------
void CBurningEffect::Paint()
{
	// ADDED WHILE EYE THING IS HERE
	if (m_flAmount <= 0.0f || m_flTargetAmount <= 0.0f)
		return;

	// Reduce the target amount gently
	m_flTargetAmount -= 0.5f;

	if (m_flTargetAmount < 0.0f)
		m_flTargetAmount = 0.0f;

	// Actual value less than intended, so raise up to meet
	if (m_flAmount < m_flTargetAmount)
	{
		m_flAmount += 1.0f;
		m_flAmount = min(m_flAmount, m_flTargetAmount);
	}
	else
	{
		m_flAmount = m_flTargetAmount;
	}

	IMesh *pMesh = materials->GetDynamicMesh(true, NULL, NULL, m_WhiteAdditiveMaterial);

	CMeshBuilder meshBuilder;
	meshBuilder.Begin(pMesh, MATERIAL_QUADS, 2);
	int r = 255, g = 0, b = 0, a = clamp(m_flAmount, 0, 255);

	//float blah = a / 255.0f;

	float wide = GetWide();
	float lend =  wide / 1.8f;
	float rstart = wide / 2.24f;
	float tall = GetTall();

	// LHS
	meshBuilder.Color4ub(r, g, b, a);
	meshBuilder.TexCoord2f(0, 0, 0);
	meshBuilder.Position3f(0.0f, 0.0f, 0);
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ub(r, g, b, 0);
	meshBuilder.TexCoord2f(0, 1, 0);
	meshBuilder.Position3f(lend, 0.0f, 0);
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ub(r, g, b, 0);
	meshBuilder.TexCoord2f(0, 1, 1);
	meshBuilder.Position3f(lend, tall, 0);
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ub(r, g, b, a);
	meshBuilder.TexCoord2f(0, 0, 1);
	meshBuilder.Position3f(0.0f, tall, 0);
	meshBuilder.AdvanceVertex();

	// RHS
	meshBuilder.Color4ub(r, g, b, 0);
	meshBuilder.TexCoord2f(0, 0, 0);
	meshBuilder.Position3f(rstart, 0.0f, 0);
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ub(r, g, b, a);
	meshBuilder.TexCoord2f(0, 1, 0);
	meshBuilder.Position3f(wide, 0.0f, 0);
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ub(r, g, b, a);
	meshBuilder.TexCoord2f(0, 1, 1);
	meshBuilder.Position3f(wide, tall, 0);
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4ub(r, g, b, 0);
	meshBuilder.TexCoord2f(0, 0, 1);
	meshBuilder.Position3f(rstart, tall, 0);
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();
}

//-----------------------------------------------------------------------------
// Purpose: Add the new burning effect amount
//-----------------------------------------------------------------------------
void CBurningEffect::MsgFunc_BurningEffect(bf_read &msg)
{
	m_flTargetAmount += (float) msg.ReadByte();

	if (m_flTargetAmount > 255.0f)
		m_flTargetAmount = 255.0f;
}
