/********************************************************************
	created:	2007/08/16
	created:	16:8:2007   9:56
	filename: 	c:\ffsvn\code\cl_dll\ff\ProxyMap.cpp
	file path:	c:\ffsvn\code\cl_dll\ff
	file base:	ProxyMap
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	A proxy for the map texture for loading screens.
*********************************************************************/

#include "cbase.h"
#include "FunctionProxy.h"
#include <KeyValues.h>
#include "materialsystem/IMaterialVar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CProxyMapTexture : public CResultProxy
{
public:
	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	virtual void OnBind(void *pC_BaseEntity);

private:

	char m_szLevelName[256];
	IMaterialVar *m_pResult;
};

bool CProxyMapTexture::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	m_szLevelName[0] = 0;
	bool foundVar;
	const char *pResult = pKeyValues->GetString("resultVar");
	m_pResult = pMaterial->FindVar(pResult, &foundVar, true);
	return true;
}

void CProxyMapTexture::OnBind(void *pC_BaseEntity) 
{
	if (Q_strcmp(m_szLevelName, engine->GetLevelName()) == 0)
		return;

	Q_strcpy(m_szLevelName, engine->GetLevelName());

	int i = Q_strlen(m_szLevelName);

	if (i < 3)
		return;

	char szMaterialPath[128];
	Q_snprintf(szMaterialPath, 127, "vgui/loadingscreens/%s", m_szLevelName + 5);
	szMaterialPath[20 + i - 5 - 4] = 0;

	bool foundVar;
	IMaterial *m = materials->FindMaterial(szMaterialPath, TEXTURE_GROUP_VGUI);

	if (!m)
	{
		m = materials->FindMaterial("vgui/loadingscreens/default", TEXTURE_GROUP_VGUI);
	}

	IMaterialVar *mv = m->FindVar("$baseTexture", &foundVar, true);
	m_pResult->SetTextureValue(mv->GetTextureValue());
}

EXPOSE_INTERFACE(CProxyMapTexture, IMaterialProxy, "MapTexture" IMATERIAL_PROXY_INTERFACE_VERSION);
