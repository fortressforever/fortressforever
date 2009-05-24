//====== Copyright � 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef ICLIENTENGINETOOLS_H
#define ICLIENTENGINETOOLS_H
#ifdef _WIN32
#pragma once
#endif

#include "interface.h"
#include "toolframework/itoolentity.h" // HTOOLHANDLE

class KeyValues;

//-----------------------------------------------------------------------------
// Purpose: Exported from engine to client .dll to marshall tool framework calls
// into IToolSystems
//-----------------------------------------------------------------------------
class IClientEngineTools : public IBaseInterface
{
public:
	// Level init, shutdown
	virtual void LevelInitPreEntityAllTools() = 0;
	// entities are created / spawned / precached here
	virtual void LevelInitPostEntityAllTools() = 0;

	virtual void LevelShutdownPreEntityAllTools() = 0;
	// Entities are deleted / released here...
	virtual void LevelShutdownPostEntityAllTools() = 0;

	virtual void PreRenderAllTools() = 0;
	virtual void PostRenderAllTools() = 0;

	virtual void PostToolMessage( HTOOLHANDLE hEntity, KeyValues *msg ) = 0;

	virtual void AdjustEngineViewport( int& x, int& y, int& width, int& height ) = 0;
	virtual bool SetupEngineView( Vector &origin, QAngle &angles, float &fov ) = 0;
	virtual bool SetupEngineMicrophone( Vector &origin, QAngle &angles ) = 0;

	// Paintmode is an enum declared in ienginevgui.h
	virtual void VGui_PreRenderAllTools( int paintMode ) = 0;
	virtual void VGui_PostRenderAllTools( int paintMode ) = 0;

	virtual bool IsThirdPersonCamera( ) = 0;
};

#define VCLIENTENGINETOOLS_INTERFACE_VERSION "VCLIENTENGINETOOLS001"

#endif // ICLIENTENGINETOOLS_H
