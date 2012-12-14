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
#include "clienteffectprecachesystem.h"
#include "in_buttons.h"
#include "c_ff_player.h"
#include "ff_utils.h"

#include "ScreenSpaceEffects.h"

CLIENTEFFECT_REGISTER_BEGIN(PrecacheFrontbuffer)
CLIENTEFFECT_MATERIAL("frontbuffer.vmt")
CLIENTEFFECT_REGISTER_END()

#define M_PI_2     1.57079632679489661923

extern void AddNewDurationFromNow(float &flStart, float &flDuration, float flNewDuration, float flFadeInTime, float flFadeOutTime);

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

	float				m_flNextDelay;
	float				m_flNextDuration;
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

//-----------------------------------------------------------------------------
// Purpose: A gassed effect
//-----------------------------------------------------------------------------
class CGassedEffect : public CBaseEffect
{
	virtual const char *pszEffect() { return "effects/gassed"; }
};

ADD_SCREENSPACE_EFFECT(CTranquilizedEffect, tranquilizedeffect);
ADD_SCREENSPACE_EFFECT(CInfectedEffect, infectedeffect)
ADD_SCREENSPACE_EFFECT(CGassedEffect, gassedeffect)

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
		AddNewDurationFromNow(m_flStart, m_flDuration, m_flNextDuration, M_PI_2, M_PI_2);
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
		m_flNextDuration = max(0, params->GetFloat("duration"));
		m_flNextDelay = params->GetFloat("delay");	// Not used atm
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
	if (flElapsed < M_PI_2 || flRemaining < M_PI_2)
	{
		pMatScreen = materials->FindMaterial("frontbuffer.vmt", TEXTURE_GROUP_OTHER, true);

		flInverseAlpha = (flElapsed < M_PI_2 ? flElapsed : flRemaining) / M_PI_2;

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

static ConVar cl_dynamicblur_ac("cl_dynamicblur_ac", "0", FCVAR_ARCHIVE, "Enable/disable assault cannon blur effect");

static ConVar cl_dynamicblur("cl_dynamicblur", "1", FCVAR_ARCHIVE, "Enable/disable speed-based blur effect");
static ConVar cl_dynamicblur_min("cl_dynamicblur_min", "400", FCVAR_ARCHIVE, "The minimum player speed required before the blur effect is applied");
static ConVar cl_dynamicblur_range("cl_dynamicblur_range", "50", FCVAR_ARCHIVE);

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
	void VidInit();

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

float g_flAccumulativeAccel = 0.0f;
float g_flLastSpeed = 0.0f;
float g_flLastBlurTime = 0.0f;

//-----------------------------------------------------------------------------
// Purpose: Reset on new map
//-----------------------------------------------------------------------------
void CMotionBlur::VidInit()
{
	g_flAccumulativeAccel = 0.0f;
	g_flLastBlurTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Create a buffer in which to store our samples
//-----------------------------------------------------------------------------
void CMotionBlur::Init()
{
	g_flAccumulativeAccel = 0.0f;
	g_flLastBlurTime = 0.0f;

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

extern float GetAssaultCannonCharge();

//-----------------------------------------------------------------------------
// Purpose: Do the blurring effect based on player speed
//-----------------------------------------------------------------------------
void CMotionBlur::Render(int x, int y, int w, int h)
{
	// First of all lets work out how much to do this
	C_BasePlayer *pPlayer = C_FFPlayer::GetLocalFFPlayerOrObserverTarget();

	// Must be valid, on a team and not no-clipping and alive
	if (!cl_dynamicblur.GetBool() || !pPlayer || pPlayer->GetTeamNumber() < TEAM_BLUE || !pPlayer->IsAlive() || pPlayer->GetMoveType() == MOVETYPE_NOCLIP)
		return;

	// Don't do anything here if the frametime is broken
	if (gpGlobals->frametime == 0)
		return;

	Vector vecVelocity = pPlayer->GetAbsVelocity();
	float flSpeed = vecVelocity.Length();

	// Acceleration is change in speed over time :)
	float flAcceleration = (flSpeed - g_flLastSpeed) / gpGlobals->frametime;
	g_flLastSpeed = flSpeed;

	// Only care about acceleration, not decceleration
	if (flAcceleration > 0.0f)
 		g_flAccumulativeAccel += fabs(flAcceleration) * gpGlobals->frametime;

	// Manage our accumulative acceleration a bit
	g_flAccumulativeAccel -= gpGlobals->frametime * 2500.0f;

	// This shouldn't be needed any more now that we're testing frametime, but i'll leave it in anyway.
	if (g_flAccumulativeAccel != g_flAccumulativeAccel)
		g_flAccumulativeAccel = 0.0f;
	g_flAccumulativeAccel = clamp(g_flAccumulativeAccel, 0.0f, 10000.0f);

	// Calculate the normalised blue now
	float flBlur = (g_flAccumulativeAccel - cl_dynamicblur_min.GetFloat()) / cl_dynamicblur_range.GetFloat();

	if (pPlayer->GetFlags() & FL_ONGROUND)
		flBlur = 0.0f;
	
	// The assault cannon also blurs things when firing enough
	// Take the largest blur from the two sources
	if (cl_dynamicblur_ac.GetBool())
	{
		float flCharge = GetAssaultCannonCharge();
		if (flCharge > 50.0f)
		{
			float flACBlur = (flCharge - 50.0f) / 100.0f;
			flBlur = max(flBlur, flACBlur);
		}
	}

	// We're going too slow for a blur effect and we've faded off our last blur
	if (gpGlobals->curtime > g_flLastBlurTime + 0.75f && flBlur <= 0.0f)
	{
		m_flNextSampleTime = 0.0f;
		return;
	}

	bool bFadeOff = flBlur <= 0.0f;
	flBlur = clamp(flBlur, 0.05f, 1.0f);

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

		// 100% opacity if we haven't yet got a previous sample

		float flAlpha = (m_flNextSampleTime == 0.0f ? 1.0f : flBlur * 0.2f);
		pVar = pMatScreen->FindVar("$alpha", &bFound, false);

		if (pVar)
			pVar->SetFloatValue(flAlpha);

		// Draw onto our buffer
		materials->SetRenderTarget(m_BlurImage);
		materials->DrawScreenSpaceQuad(pMatScreen);

		// Next sample time
		m_flNextSampleTime = gpGlobals->curtime + 0.01f;

		if (!bFadeOff)
			g_flLastBlurTime = gpGlobals->curtime;
	}

	// Set the alpha for the screen
	pVar = pMatScreen->FindVar( "$alpha", &bFound, false );

	if (pVar)
	{
		pVar->SetFloatValue(flBlur * 0.6f);
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

//-----------------------------------------------------------------------------
// Purpose: A cloaked effect
//-----------------------------------------------------------------------------
class CCloakedEffect : public CBaseEffect
{
	virtual void Render( int x, int y, int w, int h );
	virtual bool IsEnabled( void );
	virtual const char *pszEffect() { return "effects/cloak_view"; }
};

//Commenting out the effect on ur screen when u cloak -GreenMushy
//ADD_SCREENSPACE_EFFECT( CCloakedEffect, cloakedeffect );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCloakedEffect::Render( int x, int y, int w, int h )
{
	if( !IsEnabled() )
		return;

	m_flDuration = gpGlobals->curtime + 5.0f;

	CBaseEffect::Render( x, y, w, h );
}

//-----------------------------------------------------------------------------
// Purpose: This effect only enabled when player is cloaked
//-----------------------------------------------------------------------------
bool CCloakedEffect::IsEnabled( void )
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayerOrObserverTarget();

	if( !pPlayer )
		return false;

	// don't bother with screen effect if in thirdperson
	return ( pPlayer->IsInSlowfield() && ( !pPlayer->IsLocalPlayer() || ( pPlayer->IsLocalPlayer() && !pPlayer->ShouldDrawLocalPlayer() ) ) );
}


//-----------------------------------------------------------------------------
// Purpose: A slowfielded effect
//-----------------------------------------------------------------------------
class CSlowedEffect : public CBaseEffect
{
	virtual void Render( int x, int y, int w, int h );
	virtual bool IsEnabled( void );
	virtual const char *pszEffect() { return "effects/slowfield_view"; }
};

ADD_SCREENSPACE_EFFECT( CSlowedEffect, slowedeffect );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSlowedEffect::Render( int x, int y, int w, int h )
{
	if( !IsEnabled() )
		return;

	m_flDuration = gpGlobals->curtime + 5.0f;
	
	IMaterialVar *pVar = NULL;
	bool bFound = false;

	pVar = m_Material->FindVar("$refracttint", &bFound, false);

	if (pVar)
	{
		C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayerOrObserverTarget();

		if (!pPlayer)
			return;

		C_FFGrenadeBase *pSlowfield = pPlayer->GetActiveSlowfield();
		
		Vector vColor;
		switch (pSlowfield->GetTeamNumber())
		{
		case FF_TEAM_BLUE:
			vColor = Vector(.56, .72, .9);
			break;
		case FF_TEAM_RED:
			vColor = Vector(.9, .56, .56);
			break;
		case FF_TEAM_YELLOW:
			vColor = Vector(.9, .83, .56);
			break;
		case FF_TEAM_GREEN:
			vColor = Vector(.6, .9, .56);
			break;
		default:
			vColor = Vector(.6, .55, .37);
			break;
		}

		//Msg("VarType: %i\n", pVar->GetType());
		pVar->SetVecValue(vColor.x, vColor.y, vColor.z);
	}

	CBaseEffect::Render( x, y, w, h );
}

//-----------------------------------------------------------------------------
// Purpose: This effect only enabled when player is cloaked
//-----------------------------------------------------------------------------
bool CSlowedEffect::IsEnabled( void )
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayerOrObserverTarget();

	if( !pPlayer )
		return false;

	// don't bother with screen effect if in thirdperson
	return ( pPlayer->IsInSlowfield() && ( !pPlayer->IsLocalPlayer() || ( pPlayer->IsLocalPlayer() && !pPlayer->ShouldDrawLocalPlayer() ) ) );
}
