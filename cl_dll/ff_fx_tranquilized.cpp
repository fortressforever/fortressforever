/********************************************************************
	created:	2006/08/16
	created:	16:8:2006   18:35
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff_fx_tranquilized.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll
	file base:	ff_fx_tranquilized
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#include "cbase.h"

#include "keyvalues.h"
#include "cdll_client_int.h"
#include "view_scene.h"
#include "viewrender.h"
#include "vstdlib/icommandline.h"
#include "materialsystem/IMesh.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMaterialSystemHardwareConfig.h"
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/IColorCorrection.h"

#include "ScreenSpaceEffects.h"

class CTranquilizedEffect : public IScreenSpaceEffect
{
public:
	CTranquilizedEffect();
	~CTranquilizedEffect();

	void Init();
	void Shutdown();

	void SetParameters(KeyValues *params);

	void Render(int x, int y, int w, int h);

	void Enable(bool bEnable);
	bool IsEnabled();

	virtual const char *pszEffect() { return "effects/tranquilized"; }

private:

	bool				m_bEnable;

	CMaterialReference	m_Material;

	bool				m_bSplitScreen;

	float				m_flStart;
	float				m_flDuration;
};

class CInfectedEffect : public CTranquilizedEffect
{
	virtual const char *pszEffect() { return "effects/infected"; }
};

ADD_SCREENSPACE_EFFECT(CTranquilizedEffect, tranquilizedeffect);
ADD_SCREENSPACE_EFFECT(CInfectedEffect, infectedeffect)

//-----------------------------------------------------------------------------
// Purpose CTranquilizedEffect constructor
//-----------------------------------------------------------------------------
CTranquilizedEffect::CTranquilizedEffect()
{
	m_bSplitScreen = false;
	m_flStart = m_flDuration = 0.0f;
}


//------------------------------------------------------------------------------
// Purpose CTranquilizedEffect destructor
//------------------------------------------------------------------------------
CTranquilizedEffect::~CTranquilizedEffect()
{
}


//------------------------------------------------------------------------------
// Purpose: Get the correct texture
//------------------------------------------------------------------------------
void CTranquilizedEffect::Init()
{
	m_Material.Init(pszEffect(), TEXTURE_GROUP_OTHER);

	m_bEnable = false;
}


//------------------------------------------------------------------------------
// Purpose: Shut it all down!
//------------------------------------------------------------------------------
void CTranquilizedEffect::Shutdown()
{
	m_Material.Shutdown();
}

//------------------------------------------------------------------------------
// Purpose: Start the thing going
//------------------------------------------------------------------------------
void CTranquilizedEffect::Enable(bool bEnable)
{
	// This shouldn't happen because earlier a different render route should
	// have been taken
	if (g_pMaterialSystemHardwareConfig->GetDXSupportLevel() < 70 && bEnable)
	{
		Assert(0);
		bEnable = false;
	}
	else
	{
		m_bEnable = bEnable;
	}

	if (m_bEnable)
	{
		m_flStart = gpGlobals->curtime;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTranquilizedEffect::IsEnabled()
{
	return (m_flStart + m_flDuration > gpGlobals->curtime);
}

//------------------------------------------------------------------------------
// Purpose: Get the duration
//------------------------------------------------------------------------------
void CTranquilizedEffect::SetParameters(KeyValues *params)
{
	if (params->GetDataType("duration") == KeyValues::TYPE_FLOAT)
	{
		m_flDuration = params->GetFloat("duration");
	}
}

//------------------------------------------------------------------------------
// Purpose: Render the thing
//			To simulate it fading in and out we first apply the effect
//			and then take the before-buffer and re-draw that back on with
//			alpha set.
//------------------------------------------------------------------------------
void CTranquilizedEffect::Render(int x, int y, int w, int h)
{
	float flElapsed = gpGlobals->curtime - m_flStart;

	if (flElapsed > m_flDuration)
		return;

	float flRemaining = m_flDuration - flElapsed;

	// We shouldn't get here if we can't support this render route
	if (g_pMaterialSystemHardwareConfig->GetDXSupportLevel() < 70)
	{
		Assert(0);
		return;
	}

	static bool bFirstFrame = true;

	if (bFirstFrame)
	{
		bFirstFrame = false;
		return;
	}

	Rect_t actualRect;

	// Update the full frame buffer texture ready for use
	UpdateScreenEffectTexture(0, x, y, w, h, false, &actualRect);

	// Now lets start using it
	ITexture *pTexture = GetFullFrameFrameBufferTexture(0);

	IMaterial *pMatScreen = NULL;
	float flInverseAlpha = 0;

	// We want to fade in and out
	// frontbuffer.vmt is a quick and simple way of getting the.. frontbuffer
	if (flElapsed < 0.5f || flRemaining < 0.5f)
	{
		pMatScreen = materials->FindMaterial("frontbuffer.vmt", TEXTURE_GROUP_OTHER, true);

		flInverseAlpha = (flElapsed < 0.5f ? flElapsed : flRemaining) * 2.0f;

		flInverseAlpha = 1.0f - clamp(flInverseAlpha, 0.0f, 1.0f);
	}

	// Draw the shader stuff on top of the buffer
	materials->DrawScreenSpaceRectangle(m_Material, x, y, w, h, 
		actualRect.x, actualRect.y, actualRect.x+actualRect.width-1, actualRect.y+actualRect.height-1, 
		pTexture->GetActualWidth(), pTexture->GetActualHeight());

	// If there is some inverse alpha then re-draw the unmodified front buffer back on again
	// This way we can simulate the effect thing fading in or out
	if (flInverseAlpha > 0.0f)
	{
		bool bFound;
		IMaterialVar *pVar = pMatScreen->FindVar("$alpha", &bFound, true);

		if (pVar)
			pVar->SetFloatValue(flInverseAlpha);

		materials->DrawScreenSpaceRectangle(pMatScreen, x, y, w, h, 
			actualRect.x, actualRect.y, actualRect.x+actualRect.width-1, actualRect.y+actualRect.height-1, 
			pTexture->GetActualWidth(), pTexture->GetActualHeight());
	}
}