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

	IMaterialVar *m_pResult;
	char m_szLevelName[256];
	char m_szMaterialPath[1024];
};

bool CProxyMapTexture::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	m_szLevelName[0] = 0;
	m_szMaterialPath[0] = 0;

	bool foundVar;
	m_pResult = pMaterial->FindVar("$baseTexture", &foundVar, true);

	const char *pszPath = pKeyValues->GetString("path");
	Q_strncpy(m_szMaterialPath, pszPath ? pszPath : "", 1023);

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

	char szMaterial[1024];
	Q_snprintf(szMaterial, 1023, "%s/%s", m_szMaterialPath, m_szLevelName + 5);
	szMaterial[strlen(m_szMaterialPath) + i - 5 - 4 + 1] = 0;

	bool foundVar;
	IMaterial *m = materials->FindMaterial(szMaterial, TEXTURE_GROUP_VGUI);

	if (!m || m->IsErrorMaterial())
	{
		Q_snprintf(szMaterial, 1023, "%s/default", m_szMaterialPath);
		m = materials->FindMaterial(szMaterial, TEXTURE_GROUP_VGUI);
	}

	IMaterialVar *mv = m->FindVar("$baseTexture", &foundVar, true);
	m_pResult->SetTextureValue(mv->GetTextureValue());
}

EXPOSE_INTERFACE(CProxyMapTexture, IMaterialProxy, "MapTexture" IMATERIAL_PROXY_INTERFACE_VERSION);
