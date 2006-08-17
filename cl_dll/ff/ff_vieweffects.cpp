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
// Purpose: A message has been passed in
//			FF_VIEWEFFECT_MAX will reset all view effects
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
	// Purpose: Render the giant eyelids! Do this after the hud
	//-----------------------------------------------------------------------------
	void Render(int type, int width, int height)
	{
		// Don't draw until after the hud
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
			flAmount = 0.2f * sinf(flElapsed);
		}
		else if (flRemaining <= M_PI_2)
		{
			flAmount = 0.2f * sinf(flRemaining);
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
	// Purpose: 
	//-----------------------------------------------------------------------------
	void Reset()
	{
		// Do something else for this
		if (g_pMaterialSystemHardwareConfig->GetDXSupportLevel() < 80)
		{
		}
		else
		{
			KeyValues *pKeys = new KeyValues("keys");
			pKeys->SetFloat("duration", 0.0f);

			g_pScreenSpaceEffects->SetScreenSpaceEffectParams("tranquilizedeffect", pKeys);
			g_pScreenSpaceEffects->EnableScreenSpaceEffect("tranquilizedeffect");
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Receive a message and do stuff
	//-----------------------------------------------------------------------------
	void Message(bf_read &msg)
	{
		m_flStart = gpGlobals->curtime;
		m_flDuration = msg.ReadFloat();

		// Do something else for this
		if (g_pMaterialSystemHardwareConfig->GetDXSupportLevel() < 80)
		{
			Warning("*** FF Error *** Not yet implemented for < dx8!\n");
		}
		else
		{
			KeyValues *pKeys = new KeyValues("keys");
			pKeys->SetFloat("duration", m_flDuration - 0.6f);
			pKeys->SetFloat("delay", 0.3f);

			g_pScreenSpaceEffects->SetScreenSpaceEffectParams("tranquilizedeffect", pKeys);
			g_pScreenSpaceEffects->EnableScreenSpaceEffect("tranquilizedeffect");
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
	// Purpose: 
	//-----------------------------------------------------------------------------
	void Reset()
	{
		// Do something else for this
		if (g_pMaterialSystemHardwareConfig->GetDXSupportLevel() < 80)
		{
		}
		else
		{
			KeyValues *pKeys = new KeyValues("keys");
			pKeys->SetFloat("duration", 0.0f);

			g_pScreenSpaceEffects->SetScreenSpaceEffectParams("infectedeffect", pKeys);
			g_pScreenSpaceEffects->EnableScreenSpaceEffect("infectedeffect");
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Receive a message and do stuff
	//-----------------------------------------------------------------------------
	void Message(bf_read &msg)
	{
		// Do something else for this
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
		}
	}
};

DECLARE_VIEWEFFECT(CFFInfectedViewEffect);
