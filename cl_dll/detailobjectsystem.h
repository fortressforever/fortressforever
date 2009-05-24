//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Deals with singleton  
//
// $Revision: $
// $NoKeywords: $
//=============================================================================//

#if !defined( DETAILOBJECTSYSTEM_H )
#define DETAILOBJECTSYSTEM_H
#ifdef _WIN32
#pragma once
#endif

#include "IGameSystem.h"
#include "IClientEntityInternal.h"
#include "engine/IVModelRender.h"
#include "vector.h"
#include "IVRenderView.h"

struct model_t;


//-----------------------------------------------------------------------------
// Responsible for managing detail objects
//-----------------------------------------------------------------------------
abstract_class IDetailObjectSystem : public IGameSystem
{
public:
    // Gets a particular detail object
	virtual IClientRenderable* GetDetailModel( int idx ) = 0;

	// Gets called each view
	virtual void BuildDetailObjectRenderLists( ) = 0;

	// Renders all opaque detail objects in a particular set of leaves
	virtual void RenderOpaqueDetailObjects( int nLeafCount, LeafIndex_t *pLeafList ) = 0;

	// Call this before rendering translucent detail objects
	virtual void BeginTranslucentDetailRendering( ) = 0;

	// Renders all translucent detail objects in a particular set of leaves
	virtual void RenderTranslucentDetailObjects( const Vector &viewOrigin, const Vector &viewForward, int nLeafCount, LeafIndex_t *pLeafList ) = 0;

	// Renders all translucent detail objects in a particular leaf up to a particular point
	virtual void RenderTranslucentDetailObjectsInLeaf( const Vector &viewOrigin, const Vector &viewForward, int nLeaf, const Vector *pVecClosestPoint ) = 0;
};


//-----------------------------------------------------------------------------
// System for dealing with detail objects
//-----------------------------------------------------------------------------
IDetailObjectSystem* DetailObjectSystem();


#endif // DETAILOBJECTSYSTEM_H

