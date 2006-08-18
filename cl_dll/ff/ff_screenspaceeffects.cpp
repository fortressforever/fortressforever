/********************************************************************
	created:	2006/08/16
	created:	16:8:2006   18:35
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\ff_screenspaceeffects.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll\ff
	file base:	ff_screenspaceeffects
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
#include "c_baseplayer.h"

#include "ScreenSpaceEffects.h"

//-----------------------------------------------------------------------------
// Purpose: Okay because a lot of effects will be similar, this is now going
//			to act as a base
//-----------------------------------------------------------------------------
class CBaseEffect : public IScreenSpaceEffect
{
public:
	CBaseEffect();
	~CBaseEffect();

	void Init();
	void Shutdown();

	void SetParameters(KeyValues *params);

	void Render(int x, int y, int w, int h);

	void Enable(bool bEnable);
	bool IsEnabled();

protected:
	// Just make sure nobody can instantiate this by itself
	virtual const char *pszEffect() = 0;

	bool				m_bEnable;

	CMaterialReference	m_Material;

	float				m_flStart;
	float				m_flDuration;
	float				m_flDelay;
};

//-----------------------------------------------------------------------------
// Purpose: A tranquilized class
//-----------------------------------------------------------------------------
class CTranquilizedEffect : public CBaseEffect
{
	virtual const char *pszEffect() { return "effects/tranquilized"; }
};

//-----------------------------------------------------------------------------
// Purpose: An infected effect
//-----------------------------------------------------------------------------
class CInfectedEffect : public CBaseEffect
{
	virtual const char *pszEffect() { return "effects/infected"; }
};

ADD_SCREENSPACE_EFFECT(CTranquilizedEffect, tranquilizedeffect);
ADD_SCREENSPACE_EFFECT(CInfectedEffect, infectedeffect)

//-----------------------------------------------------------------------------
// Purpose CTranquilizedEffect constructor
//-----------------------------------------------------------------------------
CBaseEffect::CBaseEffect()
{
	m_flStart = m_flDuration = 0.0f;
}


//------------------------------------------------------------------------------
// Purpose CTranquilizedEffect destructor
//------------------------------------------------------------------------------
CBaseEffect::~CBaseEffect()
{
}


//------------------------------------------------------------------------------
// Purpose: Get the correct texture
//------------------------------------------------------------------------------
void CBaseEffect::Init()
{
	m_Material.Init(pszEffect(), TEXTURE_GROUP_OTHER);

	m_bEnable = false;
}


//------------------------------------------------------------------------------
// Purpose: Shut it all down!
//------------------------------------------------------------------------------
void CBaseEffect::Shutdown()
{
	m_Material.Shutdown();
}

//------------------------------------------------------------------------------
// Purpose: Start the thing going
//------------------------------------------------------------------------------
void CBaseEffect::Enable(bool bEnable)
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
		m_flStart = gpGlobals->curtime + m_flDelay;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseEffect::IsEnabled()
{
	return (m_flStart < gpGlobals->curtime && m_flStart + m_flDuration > gpGlobals->curtime);
}

//------------------------------------------------------------------------------
// Purpose: Get the duration
//------------------------------------------------------------------------------
void CBaseEffect::SetParameters(KeyValues *params)
{
	if (params->GetDataType("duration") == KeyValues::TYPE_FLOAT)
	{
		m_flDuration = params->GetFloat("duration");
		m_flDelay = params->GetFloat("delay");
	}
}

//------------------------------------------------------------------------------
// Purpose: Render the thing
//			To simulate it fading in and out we first apply the effect
//			and then take the before-buffer and re-draw that back on with
//			alpha set.
//------------------------------------------------------------------------------
void CBaseEffect::Render(int x, int y, int w, int h)
{
	// Don't run yet
	if (m_flStart > gpGlobals->curtime)
		return;

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

static ConVar ffdev_blur_minspeed("ffdev_blur_minspeed", "1000");
static ConVar ffdev_blur_rangespeed("ffdev_blur_rangespeed", "600");

//-----------------------------------------------------------------------------
// Purpose: A motion blur for concs
//-----------------------------------------------------------------------------
class CMotionBlur : public IScreenSpaceEffect
{
public:
	CMotionBlur();
	~CMotionBlur() {};

	void Init();
	void Shutdown();

	void SetParameters(KeyValues *params) {}

	void Render(int x, int y, int w, int h);

	void Enable(bool bEnable) {};
	bool IsEnabled() { return true; }

private:

	bool				m_bEnable;
	float				m_flNextSampleTime;

	CTextureReference	m_BlurImage;		
};

ADD_SCREENSPACE_EFFECT(CMotionBlur, motionblur)

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CMotionBlur::CMotionBlur()
{
	m_flNextSampleTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Create a buffer in which to store our samples
//-----------------------------------------------------------------------------
void CMotionBlur::Init()
{
	m_BlurImage.InitRenderTarget(256, 256, RT_SIZE_FULL_FRAME_BUFFER,
		IMAGE_FORMAT_ARGB8888, MATERIAL_RT_DEPTH_NONE, false);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMotionBlur::Shutdown()
{
	m_BlurImage.Shutdown();
}

//-----------------------------------------------------------------------------
// Purpose: Do the blurring effect based on player speed
//-----------------------------------------------------------------------------
void CMotionBlur::Render(int x, int y, int w, int h)
{
	// First of all lets work out how much to do this
	C_BasePlayer *pPlayer = CBasePlayer::GetLocalPlayer();

	// Must be valid, on a team and not no-clipping
	if (!pPlayer || pPlayer->GetTeamNumber() < TEAM_BLUE || pPlayer->GetMoveType() == MOVETYPE_NOCLIP)
		return;

	Vector vecVelocity = pPlayer->GetAbsVelocity();
	float flSpeed = vecVelocity.LengthSqr();

	// Too slow for any blur effect
	if (flSpeed < ffdev_blur_minspeed.GetFloat() * ffdev_blur_minspeed.GetFloat())
	{
		m_flNextSampleTime = 0.0f;
		return;
	}

	flSpeed = FastSqrt(flSpeed);
	flSpeed -= ffdev_blur_minspeed.GetFloat();
	flSpeed /= ffdev_blur_rangespeed.GetFloat();

	flSpeed = clamp(flSpeed, 0.0f, 1.0f);

	Assert(flSpeed > 0.0f);

	IMaterialVar *pVar = NULL;
	bool	bFound;
	IMaterial *pMatScreen = materials->FindMaterial("frontbuffer", TEXTURE_GROUP_OTHER, true);
	ITexture *pOriginalRenderTarget = materials->GetRenderTarget();
	ITexture *pOriginalTexture = NULL;

	// Time to take a sample for our motionblur buffer
	if(gpGlobals->curtime >= m_flNextSampleTime) 
	{
		Rect_t actualRect;

		// Update the full frame buffer texture ready for use
		UpdateScreenEffectTexture(0, x, y, w, h, false, &actualRect);

		pVar = pMatScreen->FindVar("$alpha", &bFound, false);

		if (pVar)
		{
			pVar->SetFloatValue((m_flNextSampleTime == 0.0f ? 1.0f : flSpeed * 0.2f));
		}

		// Draw onto our buffer
		materials->SetRenderTarget(m_BlurImage);
		materials->DrawScreenSpaceQuad(pMatScreen);

		// Next sample time
		m_flNextSampleTime = gpGlobals->curtime + 0.01f;
	}

	// Set the alpha for the screen
	pVar = pMatScreen->FindVar( "$alpha", &bFound, false );

	if (pVar)
	{
		pVar->SetFloatValue(flSpeed * 0.6f);
	}

	// Set the texture to our buffer
	pVar = pMatScreen->FindVar("$basetexture", &bFound, false);

	if (pVar)
	{
		pOriginalTexture = pVar->GetTextureValue();
		pVar->SetTextureValue(m_BlurImage);
	}

	// Now render back to our actual frontbuffer again
	// We're now drawing 
	materials->SetRenderTarget(pOriginalRenderTarget);
	materials->DrawScreenSpaceQuad(pMatScreen);

	// Set back to original place
	if (pVar)
	{
		pVar->SetTextureValue(pOriginalTexture);
	}
}
