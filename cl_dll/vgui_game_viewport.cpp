//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "clientmode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//================================================================
// Global startup and shutdown functions for game code in the DLL.
//================================================================

class CViewportClientSystem : public IGameSystem
{
	// Init, shutdown
	bool Init()
	{
		g_pClientMode->Layout();
		return true;
	}

	void Shutdown() {}
	void LevelInitPreEntity() {}
	void LevelInitPostEntity() {}
	void LevelShutdownPreEntity() {}
	void LevelShutdownPostEntity() {}
	void PreRender() {}
	void Update( float frametime ) {}
	void SafeRemoveIfDesired() {}

	virtual void OnSave() {}
	virtual void OnRestore() {}
};



static CViewportClientSystem g_ViewportClientSystem;

IGameSystem *ViewportClientSystem()
{
	return &g_ViewportClientSystem;
}
