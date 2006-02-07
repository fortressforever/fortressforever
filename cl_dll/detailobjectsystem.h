//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
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
// Renderable detail objects
//-----------------------------------------------------------------------------
class CBaseStaticModel : public IClientUnknown, public IClientRenderable
{
public:
	DECLARE_CLASS_NOBASE( CBaseStaticModel );

	// constructor, destructor
	CBaseStaticModel();
	virtual ~CBaseStaticModel();

	// Initialization
	virtual bool Init( int index, const Vector& org, const QAngle& angles, model_t* pModel );
	void SetAlpha( unsigned char alpha ) { m_Alpha = alpha; }


// IClientUnknown overrides.
public:

	virtual IClientUnknown*		GetIClientUnknown()		{ return this; }
	virtual ICollideable*		GetCollideable()		{ return 0; }		// Static props DO implement this.
	virtual IClientNetworkable*	GetClientNetworkable()	{ return 0; }
	virtual IClientRenderable*	GetClientRenderable()	{ return this; }
	virtual IClientEntity*		GetIClientEntity()		{ return 0; }
	virtual C_BaseEntity*		GetBaseEntity()			{ return 0; }
	virtual IClientThinkable*	GetClientThinkable()	{ return 0; }


// IClientRenderable overrides.
public:

	virtual int					GetBody() { return 0; }
	virtual const Vector&		GetRenderOrigin( );
	virtual const QAngle&		GetRenderAngles( );
	virtual bool				ShouldDraw();
	virtual bool				IsTransparent( void );
	virtual const model_t*		GetModel( ) const;
	virtual int					DrawModel( int flags );
	virtual void				ComputeFxBlend( );
	virtual int					GetFxBlend( );
	virtual void				GetColorModulation( float* color );
	virtual bool				LODTest();
	virtual bool				SetupBones( matrix3x4_t *pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime );
	virtual void				SetupWeights( void );
	virtual void				DoAnimationEvents( void );
	virtual void				GetRenderBounds( Vector& mins, Vector& maxs );
	virtual IPVSNotify*			GetPVSNotifyInterface();
	virtual void				GetRenderBoundsWorldspace( Vector& mins, Vector& maxs );
	virtual bool				ShouldReceiveProjectedTextures( int flags );
	virtual bool				GetShadowCastDistance( float *pDist, ShadowType_t shadowType ) const			{ return false; }
	virtual bool				GetShadowCastDirection( Vector *pDirection, ShadowType_t shadowType ) const	{ return false; }
	virtual bool				UsesFrameBufferTexture();

	virtual ClientShadowHandle_t	GetShadowHandle() const;
	virtual ClientRenderHandle_t&	RenderHandle();
	virtual void				GetShadowRenderBounds( Vector &mins, Vector &maxs, ShadowType_t shadowType );

protected:
	Vector	m_Origin;
	QAngle	m_Angles;
	model_t* m_pModel;
	ModelInstanceHandle_t m_ModelInstance;
	unsigned char	m_Alpha;
	ClientRenderHandle_t m_hRenderHandle;
};


//-----------------------------------------------------------------------------
// Responsible for managing detail objects
//-----------------------------------------------------------------------------
class IDetailObjectSystem : public IGameSystem
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

