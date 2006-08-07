//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Revision: $
// $NoKeywords: $
//
// This file contains code to allow us to associate client data with bsp leaves.
//
//=============================================================================//

#if !defined( ICLIENTLEAFSYSTEM_H )
#define ICLIENTLEAFSYSTEM_H
#ifdef _WIN32
#pragma once
#endif


#include "client_render_handle.h"


//-----------------------------------------------------------------------------
// Render groups
//-----------------------------------------------------------------------------
enum RenderGroup_t
{
	RENDER_GROUP_OPAQUE_ENTITY = 0,
	RENDER_GROUP_TRANSLUCENT_ENTITY,
	RENDER_GROUP_TWOPASS, // Implied opaque and translucent in two passes
	RENDER_GROUP_VIEW_MODEL_OPAQUE, // solid weapon view models
	RENDER_GROUP_VIEW_MODEL_TRANSLUCENT, // transparent overlays etc
	RENDER_GROUP_OTHER,	// Unclassfied. Won't get drawn.

	RENDER_GROUP_COUNT_VERSION_1,

	RENDER_GROUP_OPAQUE_STATIC,
	RENDER_GROUP_OPAQUE_BRUSH,

	// This one's always gotta be last
	RENDER_GROUP_COUNT
};

#define CLIENTLEAFSYSTEM_INTERFACE_VERSION_1 "ClientLeafSystem001"
#define CLIENTLEAFSYSTEM_INTERFACE_VERSION	"ClientLeafSystem002"


//-----------------------------------------------------------------------------
// The client leaf system
//-----------------------------------------------------------------------------
abstract_class IClientLeafSystemEngine
{
public:
	// Adds and removes renderables from the leaf lists
	// CreateRenderableHandle stores the handle inside pRenderable.
	virtual void CreateRenderableHandle( IClientRenderable* pRenderable, bool bIsStaticProp = false ) = 0;
	virtual void RemoveRenderable( ClientRenderHandle_t handle ) = 0;
	virtual void AddRenderableToLeaves( ClientRenderHandle_t renderable, int nLeafCount, unsigned short *pLeaves ) = 0;
	virtual void ChangeRenderableRenderGroup( ClientRenderHandle_t handle, RenderGroup_t group ) = 0;
};


#endif	// ICLIENTLEAFSYSTEM_H


