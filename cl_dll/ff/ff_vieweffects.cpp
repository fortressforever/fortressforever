/********************************************************************
	created:	2006/08/17
	created:	17:8:2006   18:43
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\ff_vieweffects.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll\ff
	file base:	ff_vieweffects
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	View effects for ff
*********************************************************************/

#include "cbase.h"
#include "ff_vieweffects.h"
#include "hud_macros.h"
#include "ff_shareddefs.h"
#include "ScreenSpaceEffects.h"
#include "keyvalues.h"
#include "materialsystem/IMaterialSystemHardwareConfig.h"

void __MsgFunc_FFViewEffect(bf_read &msg);

//-----------------------------------------------------------------------------
// Purpose: A useful function. Give it your current start & duration, a new
//			duration and your fade in / out times and it'll set your new
//			start + duration times without messing up any in-progress effect
//-----------------------------------------------------------------------------
void AddNewDurationFromNow(float &flStart, float &flDuration, float flNewDuration, float flFadeInTime, float flFadeOutTime)
{
	float flElapsed  = gpGlobals->curtime - flStart;
	float flRemaining = flDuration - flElapsed;

	// We're not currently in an effect, either because the last one just
	// finished or the next one is still in the future. Therefore set straight
	// away
	if (flElapsed > flDuration || flStart > gpGlobals->curtime)
	{
		flStart = gpGlobals->curtime;
		flDuration = flNewDuration;
		return;
	}

	// We're in some sort of effect now, make sure that the duration isn't too short
	// so that we don't do an ugly cut.
	if (flNewDuration < flFadeOutTime)
	{
		// We're already fading out, we can't do it any quicker.
		// So escape now.
		if (flRemaining < flFadeOutTime)
			return;

		// Duration is now set to the minimum possible.
		flDuration = flElapsed + flFadeOutTime;

		// If we haven't even finihed fading in, adjust the start time so that we
		// continue fading out from this point
		if (flElapsed < flFadeInTime)
		{
			flStart -= flFadeInTime - flElapsed;
		}

		// Phew!
		return;
	}

	// We're still fading in, therefore we don't want to change the start time.
	// However this means that the duration needs to be increased since it's an
	// earlier starttime than it was expecting.
	if (flElapsed < flFadeInTime)
	{
		flDuration = flNewDuration + flElapsed;
		return;
	}

	// We're in the middle of fading out. We want to start fading in again at
	// the point we're currently at. This means that the new flElapsed should
	// equal the old flRemaining.
	// Since this means we have to put the start time in the past, we should
	// therefore extend the duration by the same amount.
	if (flRemaining < flFadeOutTime)
	{
		flStart = gpGlobals->curtime - flRemaining;
		flDuration = flNewDuration + flRemaining;
		return;
	}

	// We must be within an effect. Keep the start time (there's no real good
	// reason to change it) and adjust the duration accordingly.
	// This is the same as conditional statement 2, so can probably be resolved
	// along with that.
	flDuration = flNewDuration + flElapsed;
}

//=============================================================================
// A manager for our view effects
//=============================================================================
class CFFViewEffectsMgr : public IFFViewEffects
{
public:
	CFFViewEffectsMgr();

	void Init();
	void LevelInit();
	void Reset();
	void Render(int type, int width, int height);
	void Message(bf_read &msg);

	void Add(int iId, IFFViewEffects *pViewEffect);

private:
	IFFViewEffects	*m_pViewEffects[FF_VIEWEFFECT_MAX];
	unsigned int	m_nViewEffects;
};

//-----------------------------------------------------------------------------
// Purpose: Our singleton stuff
//-----------------------------------------------------------------------------
static CFFViewEffectsMgr g_FFViewEffects;
IFFViewEffects *ffvieweffects = (IFFViewEffects *) &g_FFViewEffects;

//-----------------------------------------------------------------------------
// Purpose: This catches all our view effect messages
//-----------------------------------------------------------------------------
void __MsgFunc_FFViewEffect(bf_read &msg)
{
	g_FFViewEffects.Message(msg);
}

//=============================================================================
// Purpose: A base class to allow us to register
//=============================================================================
class CFFBaseViewEffect : public IFFViewEffects
{
public:
	//-----------------------------------------------------------------------------
	// Purpose: Register the view effect with the manager. This is a pretty simple
	//			way of going about this.
	//-----------------------------------------------------------------------------
	CFFBaseViewEffect(FF_View_Effects_t id)
	{
		g_FFViewEffects.Add(id, this);
		m_Id = id;
	}

	// Save some hassle by implementing some member functions.
	// Not Reset or Message though, those have to be handled
	void Init() {}
	void LevelInit() {}
	void Render(int type, int width, int height) {}

	int m_Id;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CFFViewEffectsMgr::CFFViewEffectsMgr()
{
	for (int i = 0; i < FF_VIEWEFFECT_MAX; i++)
	{
		m_pViewEffects[i] = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Hook our message and initialise all our view effects
//-----------------------------------------------------------------------------
void CFFViewEffectsMgr::Init()
{
	HOOK_MESSAGE(FFViewEffect);

	for (unsigned int i = 0; i < m_nViewEffects; i++)
	{
		m_pViewEffects[i]->Init();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFViewEffectsMgr::LevelInit()
{
	for (unsigned int i = 0; i < m_nViewEffects; i++)
	{
		m_pViewEffects[i]->LevelInit();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFViewEffectsMgr::Reset()
{
	for (unsigned int i = 0; i < m_nViewEffects; i++)
	{
		m_pViewEffects[i]->Reset();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFViewEffectsMgr::Render(int type, int width, int height)
{
	for (unsigned int i = 0; i < m_nViewEffects; i++)
	{
		m_pViewEffects[i]->Render(type, width, height);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFViewEffectsMgr::Add(int iId, IFFViewEffects *pViewEffect)
{
	m_pViewEffects[m_nViewEffects++] = pViewEffect;
}

//-----------------------------------------------------------------------------
// Purpose: A message has been passed in. We read the first byte to determine
//			what view effect this is and then pass the msg onto the view effect
//			itself to dissect as needed.
//			FF_VIEWEFFECT_MAX will reset all view effects.
//-----------------------------------------------------------------------------
void CFFViewEffectsMgr::Message(bf_read &msg)
{
	int iId = (int) msg.ReadByte();

	// Reset all
	if (iId == FF_VIEWEFFECT_MAX)
	{
		for (unsigned int i = 0; i < m_nViewEffects; i++)
		{
			m_pViewEffects[i]->Reset();
		}

		return;
	}

	// Invalid
	if (iId < 0 || iId > FF_VIEWEFFECT_MAX)
	{
		Assert(0);
		return;
	}

	// Call the correct one
	for (unsigned int i = 0; i < m_nViewEffects; i++)
	{
		CFFBaseViewEffect *pBaseViewEffect = dynamic_cast<CFFBaseViewEffect *> (m_pViewEffects[i]);

		if (pBaseViewEffect && pBaseViewEffect->m_Id == iId)
		{
			pBaseViewEffect->Message(msg);
			return;
		}
	}

	// The vieweffect either hasn't been initialized properly in its constructor
	// or perhaps hasn't been created with the DECLARE_VIEWEFFECT macro.
	AssertMsg(0, "ViewEffect hasn't been instantiated yet!");
}

//=============================================================================
// Purpose: Tranquilized effect
//=============================================================================
class CFFTranquilizerViewEffect : public CFFBaseViewEffect
{
public:
	//-----------------------------------------------------------------------------
	// Purpose: Register
	//-----------------------------------------------------------------------------
	CFFTranquilizerViewEffect(const char *pName) : CFFBaseViewEffect(FF_VIEWEFFECT_TRANQUILIZED)
	{
	}

	//-----------------------------------------------------------------------------
	// Purpose: Initialise texture
	//-----------------------------------------------------------------------------
	void Init()
	{
		m_hEyeLidMaterial.Init("effects/eyelid.vmt", TEXTURE_GROUP_OTHER);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Reset variables
	//-----------------------------------------------------------------------------
	void LevelInit()
	{
		m_flStart = m_flDuration = 0.0f;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Render the giant eyelids over the hud.
	//-----------------------------------------------------------------------------
	void Render(int type, int width, int height)
	{
		// This draws above the hud
		if (type == VIEWEFFECT_BEFOREHUD)
			return;

		float flElapsed = gpGlobals->curtime - m_flStart;
		float flRemaining = m_flDuration - flElapsed;

		if (flElapsed > m_flDuration)
			return;

		IMesh *pMesh = materials->GetDynamicMesh(true, NULL, NULL, m_hEyeLidMaterial);

		CMeshBuilder meshBuilder;
		meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);

		float flAmount = 0.2f;

#define M_PI_2     1.57079632679489661923
		
		if (flElapsed <= M_PI_2)
		{
			flAmount = 0.05f + 0.15f * sinf(flElapsed);
		}
		else if (flRemaining <= M_PI_2)
		{
			flAmount = 0.05f + 0.15f * sinf(flRemaining);
		}

		float wide = width;
		float tall = height;

		meshBuilder.Color4ub(255, 255, 255, 255);
		meshBuilder.TexCoord2f(0, 0.5f - flAmount, 0.5f - flAmount);
		meshBuilder.Position3f(0.0f, 0.0f, 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub(255, 255, 255, 255);
		meshBuilder.TexCoord2f(0, 0.5f + flAmount, 0.5f - flAmount);
		meshBuilder.Position3f(wide, 0.0f, 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub(255, 255, 255, 255);
		meshBuilder.TexCoord2f(0, 0.5f + flAmount, 0.5f + flAmount);
		meshBuilder.Position3f(wide, tall, 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub(255, 255, 255, 255);
		meshBuilder.TexCoord2f(0, 0.5f - flAmount, 0.5f + flAmount);
		meshBuilder.Position3f(0.0f, tall, 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.End();
		pMesh->Draw();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Need to reset the screenspace effects
	//-----------------------------------------------------------------------------
	void Reset()
	{
		if (g_pMaterialSystemHardwareConfig->GetDXSupportLevel() < 80)
		{
		}
		else
		{
			KeyValues *pKeys = new KeyValues("keys");
			pKeys->SetFloat("duration", 0.0f);

			g_pScreenSpaceEffects->SetScreenSpaceEffectParams("tranquilizedeffect", pKeys);
			g_pScreenSpaceEffects->EnableScreenSpaceEffect("tranquilizedeffect");

			pKeys->deleteThis();
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Expects a DURATION as a FLOAT.
	//			Will also trigger the screenspace effect.
	//-----------------------------------------------------------------------------
	void Message(bf_read &msg)
	{
		//m_flStart = gpGlobals->curtime;
		//m_flDuration = msg.ReadFloat();

		float flNewDuration = msg.ReadFloat();

		AddNewDurationFromNow(m_flStart, m_flDuration, flNewDuration, M_PI_2, M_PI_2);

		if (g_pMaterialSystemHardwareConfig->GetDXSupportLevel() < 80)
		{
			Warning("*** FF Error *** Not yet implemented for < dx8!\n");
		}
		else
		{
			KeyValues *pKeys = new KeyValues("keys");
			pKeys->SetFloat("duration", flNewDuration - 0.3f);
			pKeys->SetFloat("delay", 0.3f);	// Not used atm

			g_pScreenSpaceEffects->SetScreenSpaceEffectParams("tranquilizedeffect", pKeys);
			g_pScreenSpaceEffects->EnableScreenSpaceEffect("tranquilizedeffect");

			pKeys->deleteThis();
		}
	}

private:

	float	m_flStart;
	float	m_flDuration;

	CMaterialReference m_hEyeLidMaterial;
};

DECLARE_VIEWEFFECT(CFFTranquilizerViewEffect);

//=============================================================================
// Purpose: Infected effect
//=============================================================================
class CFFInfectedViewEffect : public CFFBaseViewEffect
{
public:
	//-----------------------------------------------------------------------------
	// Purpose: Register
	//-----------------------------------------------------------------------------
	CFFInfectedViewEffect(const char *pName) : CFFBaseViewEffect(FF_VIEWEFFECT_INFECTED)
	{
	}

	//-----------------------------------------------------------------------------
	// Purpose: Reset screenspace effect
	//-----------------------------------------------------------------------------
	void Reset()
	{
		if (g_pMaterialSystemHardwareConfig->GetDXSupportLevel() < 80)
		{
		}
		else
		{
			KeyValues *pKeys = new KeyValues("keys");
			pKeys->SetFloat("duration", 0.0f);

			g_pScreenSpaceEffects->SetScreenSpaceEffectParams("infectedeffect", pKeys);
			g_pScreenSpaceEffects->EnableScreenSpaceEffect("infectedeffect");

			pKeys->deleteThis();
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Expects a DURATION as a FLOAT.
	//			Will also trigger the screenspace effect.
	//-----------------------------------------------------------------------------
	void Message(bf_read &msg)
	{
		if (g_pMaterialSystemHardwareConfig->GetDXSupportLevel() < 80)
		{
			Warning("*** FF Error *** Not yet implemented for < dx8!\n");
		}
		else
		{
			KeyValues *pKeys = new KeyValues("keys");
			pKeys->SetFloat("duration", msg.ReadFloat());

			g_pScreenSpaceEffects->SetScreenSpaceEffectParams("infectedeffect", pKeys);
			g_pScreenSpaceEffects->EnableScreenSpaceEffect("infectedeffect");

			pKeys->deleteThis();
		}
	}
};

DECLARE_VIEWEFFECT(CFFInfectedViewEffect);

//=============================================================================
// Purpose: Burning vieweffect
//=============================================================================
class CFFBurningEffect : public CFFBaseViewEffect
{
public:
	//-----------------------------------------------------------------------------
	// Purpose: Register
	//-----------------------------------------------------------------------------
	CFFBurningEffect(const char *pName) : CFFBaseViewEffect(FF_VIEWEFFECT_BURNING)
	{
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void Reset()
	{
		m_flAmount = m_flTargetAmount = 0.0f;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Get material set up
	//-----------------------------------------------------------------------------
	void Init()
	{
		m_WhiteAdditiveMaterial.Init("vgui/white_additive", TEXTURE_GROUP_VGUI);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Paint two quads on either side of the screen.
	//			Make sure we have some linear progression up to the new target
	//			amount so that it appears gracefully.
	//			TODO: Take into account fps
	//-----------------------------------------------------------------------------
	void Render(int type, int width, int height)
	{
		// This draws above the hud
		if (type == VIEWEFFECT_BEFOREHUD)
			return;

		// Nothing to do
		if (m_flAmount == 0.0f && m_flTargetAmount == 0.0f)
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

		float wide = (float) width;
		float lend =  wide / 1.8f;
		float rstart = wide / 2.24f;
		float tall = (float) height;

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
	// Purpose: Expects a DAMAGE AMOUNT as a BYTE.
	//			This will be added onto the current damage (which gradually fades)
	//-----------------------------------------------------------------------------
	void Message(bf_read &msg)
	{
		m_flTargetAmount += (float) msg.ReadByte();

		if (m_flTargetAmount > 255.0f)
			m_flTargetAmount = 255.0f;
	}

private:

	float	m_flAmount;
	float	m_flTargetAmount;

	CMaterialReference m_WhiteAdditiveMaterial;
};

DECLARE_VIEWEFFECT(CFFBurningEffect);
