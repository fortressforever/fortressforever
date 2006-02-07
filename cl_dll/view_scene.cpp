//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Responsible for drawing the scene
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "view.h"
#include "iviewrender.h"
#include "view_shared.h"
#include "ivieweffects.h"
#include "iinput.h"
#include "model_types.h"
#include "clientsideeffects.h"
#include "particlemgr.h"
#include "viewrender.h"
#include "iclientmode.h"
#if defined( TF2_CLIENT_DLL )
	#include "ground_line.h"
#endif
#include "voice_status.h"
#include "glow_overlay.h"
#include "materialsystem/imesh.h"
#include "materialsystem/ITexture.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMaterialVar.h"
#include "DetailObjectSystem.h"
#include "tier0/vprof.h"
#include "engine/IVEngineCache.h"
#include "engine/IEngineTrace.h"
#include "engine/ivmodelinfo.h"
#include "view_scene.h"
#include "particles_ez.h"
#include "engine/IStaticPropMgr.h"
#include "engine/ivdebugoverlay.h"
#include "smoke_fog_overlay.h"
#include "c_pixel_visibility.h"
#include "ClientEffectPrecacheSystem.h"
#include "c_rope.h"
#include "c_effects.h"
#include "smoke_fog_overlay.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"

// GR
#include "rendertexture.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern void ComputeCameraVariables( const Vector &vecOrigin, const QAngle &vecAngles, 
	Vector *pVecForward, Vector *pVecRight, Vector *pVecUp, VMatrix *pMatCamInverse );


static ConVar cl_overdraw_test( "cl_overdraw_test", "0", FCVAR_CHEAT | FCVAR_NEVER_AS_STRING );
static ConVar r_eyewaterepsilon( "r_eyewaterepsilon", "7.0f", FCVAR_CHEAT );

// Used to verify frame syncing.
void GenerateOverdrawForTesting()
{
	if ( !cl_overdraw_test.GetInt() )
		return;

	for ( int i=0; i < 40; i++ )
	{
		g_SmokeFogOverlayAlpha = 20 / 255.0;
		DrawSmokeFogOverlay();
	}
	g_SmokeFogOverlayAlpha = 0;	
}


//-----------------------------------------------------------------------------
// Convars related to controlling rendering
//-----------------------------------------------------------------------------
static ConVar cl_maxrenderable_dist("cl_maxrenderable_dist", "3000", FCVAR_CHEAT, "Max distance from the camera at which things will be rendered" );

ConVar r_updaterefracttexture( "r_updaterefracttexture", "1" );

// Matches the version in the engine
static ConVar r_drawopaqueworld( "r_drawopaqueworld", "1", FCVAR_CHEAT );
static ConVar r_drawtranslucentworld( "r_drawtranslucentworld", "1", FCVAR_CHEAT );
static ConVar r_3dsky( "r_3dsky","1", FCVAR_CHEAT, "Enable the rendering of 3d sky boxes" );
static ConVar r_skybox( "r_skybox","1", FCVAR_CHEAT, "Enable the rendering of sky boxes" );
static ConVar r_drawviewmodel( "r_drawviewmodel","1", FCVAR_CHEAT );
static ConVar r_drawtranslucentrenderables( "r_drawtranslucentrenderables", "1", FCVAR_CHEAT );
static ConVar r_drawopaquerenderables( "r_drawopaquerenderables", "1", FCVAR_CHEAT );

// FIXME: This is not static because we needed to turn it off for TF2 playtests
ConVar r_DrawDetailProps( "r_DrawDetailProps", "1", FCVAR_CHEAT );


//-----------------------------------------------------------------------------
// Convars related to fog color
//-----------------------------------------------------------------------------
static ConVar fog_override( "fog_override", "0", FCVAR_CHEAT );
// set any of these to use the maps fog
static ConVar fog_start( "fog_start", "-1" );
static ConVar fog_end( "fog_end", "-1" );
static ConVar fog_color( "fog_color", "-1 -1 -1" );
static ConVar fog_enable( "fog_enable", "1" );
static ConVar fog_startskybox( "fog_startskybox", "-1" );
static ConVar fog_endskybox( "fog_endskybox", "-1" );
static ConVar fog_colorskybox( "fog_colorskybox", "-1 -1 -1" );
static ConVar fog_enableskybox( "fog_enableskybox", "1" );


//-----------------------------------------------------------------------------
// Water-related convars
//-----------------------------------------------------------------------------
static ConVar r_debugcheapwater( "r_debugcheapwater", "0", FCVAR_CHEAT );
static ConVar r_waterforceexpensive( "r_waterforceexpensive", "0" );
static ConVar r_waterforcereflectentities( "r_waterforcereflectentities", "0" );
static ConVar r_WaterDrawRefraction( "r_WaterDrawRefraction", "1", 0, "Enable water refraction" );
static ConVar r_WaterDrawReflection( "r_WaterDrawReflection", "1", 0, "Enable water reflection" );
static ConVar r_ForceWaterLeaf( "r_ForceWaterLeaf", "1", 0, "Enable for optimization to water - considers view in leaf under water for purposes of culling" );
static ConVar mat_drawwater( "mat_drawwater", "1", FCVAR_CHEAT );
static ConVar mat_clipz( "mat_clipz", "1" );


//-----------------------------------------------------------------------------
// debugging
//-----------------------------------------------------------------------------
static ConVar r_visocclusion( "r_visocclusion", "0", FCVAR_CHEAT );
// (the engine owns this cvar).
ConVar mat_wireframe( "mat_wireframe", "0", FCVAR_CHEAT );


//-----------------------------------------------------------------------------
// debugging overlays
//-----------------------------------------------------------------------------
static ConVar cl_drawmaterial( "cl_drawmaterial", "", FCVAR_CHEAT, "Draw a particular material over the frame" );
static ConVar cl_drawshadowtexture( "cl_drawshadowtexture", "0", FCVAR_CHEAT );
static ConVar mat_showwatertextures( "mat_showwatertextures", "0", FCVAR_CHEAT );
static ConVar mat_wateroverlaysize( "mat_wateroverlaysize", "128" );
static ConVar mat_showframebuffertexture( "mat_showframebuffertexture", "0", FCVAR_CHEAT );
static ConVar mat_framebuffercopyoverlaysize( "mat_framebuffercopyoverlaysize", "128" );
static ConVar mat_showcamerarendertarget( "mat_showcamerarendertarget", "0", FCVAR_CHEAT );
static ConVar mat_camerarendertargetoverlaysize( "mat_camerarendertargetoverlaysize", "128", FCVAR_CHEAT );
static ConVar mat_hsv( "mat_hsv", "0" );
static ConVar mat_yuv( "mat_yuv", "0" );


//-----------------------------------------------------------------------------
// Other convars
//-----------------------------------------------------------------------------
// GR - HDR
static ConVar mat_bloom( "mat_bloom", "1" );

ConVar r_DoCovertTransitions("r_DoCovertTransitions", "1", FCVAR_NEVER_AS_STRING );				// internally used by game code and engine to choose when to allow LOD transitions.
ConVar r_TransitionSensitivity("r_TransitionSensitivity", "6", 0, "Controls when LODs are changed. Lower numbers cause more overt LOD transitions.");

static ConVar r_screenfademinsize( "r_screenfademinsize", "0" );
static ConVar r_screenfademaxsize( "r_screenfademaxsize", "0" );


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
static Vector g_vecCurrentRenderOrigin(0,0,0);
static QAngle g_vecCurrentRenderAngles(0,0,0);
static Vector g_vecCurrentVForward(0,0,0), g_vecCurrentVRight(0,0,0), g_vecCurrentVUp(0,0,0);
static VMatrix g_matCurrentCamInverse;
static bool s_bCanAccessCurrentView = false;
IntroData_t *g_pIntroData = NULL;
static bool	g_bRenderingView = false;			// For debugging...
static int g_CurrentViewID = VIEW_NONE;
bool g_bRenderingScreenshot = false;


//-----------------------------------------------------------------------------
// Precache of necessary materials
//-----------------------------------------------------------------------------

#ifdef HL2_CLIENT_DLL
CLIENTEFFECT_REGISTER_BEGIN( PrecacheViewRender )
CLIENTEFFECT_MATERIAL( "scripted/intro_screenspaceeffect" )
CLIENTEFFECT_REGISTER_END()
#endif


//-----------------------------------------------------------------------------
// Accessors to return the current view being rendered
//-----------------------------------------------------------------------------
const Vector &CurrentViewOrigin()
{
	Assert( s_bCanAccessCurrentView );
	return g_vecCurrentRenderOrigin;
}

const QAngle &CurrentViewAngles()
{
	Assert( s_bCanAccessCurrentView );
	return g_vecCurrentRenderAngles;
}

const Vector &CurrentViewForward()
{
	Assert( s_bCanAccessCurrentView );
	return g_vecCurrentVForward;
}

const Vector &CurrentViewRight()
{
	Assert( s_bCanAccessCurrentView );
	return g_vecCurrentVRight;
}

const Vector &CurrentViewUp()
{
	Assert( s_bCanAccessCurrentView );
	return g_vecCurrentVUp;
}

const VMatrix &CurrentWorldToViewMatrix()
{
	Assert( s_bCanAccessCurrentView );
	return g_matCurrentCamInverse;
}


//-----------------------------------------------------------------------------
// Methods to set the current view/guard access to view parameters
//-----------------------------------------------------------------------------
void AllowCurrentViewAccess( bool allow )
{
	s_bCanAccessCurrentView = allow;
}

bool IsCurrentViewAccessAllowed()
{
	return s_bCanAccessCurrentView;
}

void SetupCurrentView( const Vector &vecOrigin, const QAngle &angles, view_id_t viewID )
{
	// Store off view origin and angles
	g_vecCurrentRenderOrigin = vecOrigin;
	g_vecCurrentRenderAngles = angles;

	// Compute the world->main camera transform
	ComputeCameraVariables( vecOrigin, angles, 
		&g_vecCurrentVForward, &g_vecCurrentVRight, &g_vecCurrentVUp, &g_matCurrentCamInverse );

	g_CurrentViewID = viewID;
	s_bCanAccessCurrentView = true;

	// Cache off fade distances
	float flScreenFadeMinSize = r_screenfademinsize.GetFloat();
	float flScreenFadeMaxSize = r_screenfademaxsize.GetFloat();
	modelinfo->SetViewScreenFadeRange( flScreenFadeMinSize, flScreenFadeMaxSize );
}

int CurrentViewID()
{
	return g_CurrentViewID;
}

void FinishCurrentView()
{
	s_bCanAccessCurrentView = false;
}


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CViewRender::CViewRender()
{
	m_AnglesHistoryCounter = 0;
	memset(m_AnglesHistory, 0, sizeof(m_AnglesHistory));
	m_flCheapWaterStartDistance = 0.0f;
	m_flCheapWaterEndDistance = 0.1f;
	m_BaseDrawFlags = m_DrawFlags = 0;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
inline bool CViewRender::ShouldDrawEntities( void )
{
	return ( !m_pDrawEntities || (m_pDrawEntities->GetInt() != 0) );
}


//-----------------------------------------------------------------------------
// Purpose: Check all conditions which would prevent drawing the view model
// Input  : drawViewmodel - 
//			*viewmodel - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CViewRender::ShouldDrawViewModel( bool bDrawViewmodel )
{
	if ( !bDrawViewmodel )
		return false;

	if ( !r_drawviewmodel.GetBool() )
		return false;

	if ( input->CAM_IsThirdPerson() )
		return false;

	if ( !ShouldDrawEntities() )
		return false;

	if ( render->GetViewEntity() > gpGlobals->maxClients )
		return false;

	return true;
}


void CViewRender::DrawRenderablesInList( CUtlVector< IClientRenderable * > &list )
{
	int nCount = list.Count();
	for( int i=0; i < nCount; ++i )
	{
		IClientUnknown *pUnk = list[i]->GetIClientUnknown();
		Assert( pUnk );
		C_BaseEntity *ent = pUnk->GetBaseEntity();
		Assert( ent );

		// Non-view models wanting to render in view model list...
		if ( ent->ShouldDraw() )
		{
			ent->DrawModel( STUDIO_RENDER );
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Actually draw the view model
// Input  : drawViewModel - 
//-----------------------------------------------------------------------------
void CViewRender::DrawViewModels( const CViewSetup &view, bool drawViewmodel )
{
	VPROF( "CViewRender::DrawViewModel" );

	if ( !ShouldDrawViewModel( drawViewmodel ) )
		return;

	// Restore the matrices
	materials->MatrixMode( MATERIAL_PROJECTION );
	materials->PushMatrix();

	// Set up for drawing the view model
	render->SetProjectionMatrix( view.fovViewmodel, view.zNearViewmodel, view.zFarViewmodel );

	// FIXME: Add code to read the current depth range
	float depthmin = 0.0f;
	float depthmax = 1.0f;

	// HACK HACK:  Munge the depth range to prevent view model from poking into walls, etc.
	// Force clipped down range
	materials->DepthRange( 0.0f, 0.1f );

	CUtlVector< IClientRenderable * > opaqueViewModelList( 32 );
	CUtlVector< IClientRenderable * > translucentViewModelList( 32 );

	ClientLeafSystem()->CollateViewModelRenderables( opaqueViewModelList, translucentViewModelList );
	DrawRenderablesInList( opaqueViewModelList );
	DrawRenderablesInList( translucentViewModelList );

	// Reset the depth range to the original values
	materials->DepthRange( depthmin, depthmax );

	// Restore the matrices
	materials->MatrixMode( MATERIAL_PROJECTION );
	materials->PopMatrix();
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEnt - 
// Output : int
//-----------------------------------------------------------------------------
VPlane* CViewRender::GetFrustum()
{
	// The frustum is only valid while in a RenderView call.
	Assert(g_bRenderingView || g_bRenderingCameraView || g_bRenderingScreenshot);	
	return m_Frustum;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CViewRender::ShouldDrawBrushModels( void )
{
	if ( m_pDrawBrushModels && !m_pDrawBrushModels->GetInt() )
		return false;

	return true;
}


//-----------------------------------------------------------------------------
// Sort entities in a back-to-front ordering
//-----------------------------------------------------------------------------
void SortEntities( CRenderList::CEntry *pEntities, int nEntities )
{
	// Don't sort if we only have 1 entity
	if ( nEntities <= 1 )
		return;

	float dists[CRenderList::MAX_GROUP_ENTITIES];

	const Vector &vecRenderOrigin = CurrentViewOrigin();
	const Vector &vecRenderForward = CurrentViewForward();

	// First get a distance for each entity.
	int i;
	for( i=0; i < nEntities; i++ )
	{
		IClientRenderable *pRenderable = pEntities[i].m_pRenderable;

		// Compute the center of the object (needed for translucent brush models)
		Vector boxcenter;
		Vector mins,maxs;
		pRenderable->GetRenderBounds( mins, maxs );
		VectorAdd( mins, maxs, boxcenter );
		VectorMA( pRenderable->GetRenderOrigin(), 0.5f, boxcenter, boxcenter );

		// Compute distance...
		Vector delta;
		VectorSubtract( boxcenter, vecRenderOrigin, delta );
		dists[i] = DotProduct( delta, vecRenderForward );
	}

	// H-sort.
	int stepSize = 4;
	while( stepSize )
	{
		int end = nEntities - stepSize;
		for( i=0; i < end; i += stepSize )
		{
			if( dists[i] > dists[i+stepSize] )
			{
				swap( pEntities[i], pEntities[i+stepSize] );
				swap( dists[i], dists[i+stepSize] );

				if( i == 0 )
				{
					i = -stepSize;
				}
				else
				{
					i -= stepSize << 1;
				}
			}
		}

		stepSize >>= 1;
	}
}


void CViewRender::SetupRenderList( const CViewSetup *pView, ClientWorldListInfo_t& info, CRenderList &renderList )
{
	VPROF( "CViewRender::SetupRenderList" );

	// Clear the list.
	int i;
	for( i=0; i < RENDER_GROUP_COUNT; i++ )
	{
		renderList.m_RenderGroupCounts[i] = 0;
	}

	// Precache information used commonly in CollateRenderables
	SetupRenderInfo_t setupInfo;
	setupInfo.m_nRenderFrame = m_BuildRenderableListsNumber;
	setupInfo.m_nDetailBuildFrame = m_BuildWorldListsNumber;
	setupInfo.m_pRenderList = &renderList;
	setupInfo.m_bDrawDetailObjects = g_pClientMode->ShouldDrawDetailObjects() && r_DrawDetailProps.GetInt();

	if (pView)
	{
		setupInfo.m_vecRenderOrigin = pView->origin;
		setupInfo.m_flRenderDistSq = cl_maxrenderable_dist.GetFloat();
		setupInfo.m_flRenderDistSq  *= setupInfo.m_flRenderDistSq;
	}
	else
	{
		setupInfo.m_flRenderDistSq = 0.0f;
	}

	// Now collate the entities in the leaves.
	if( ShouldDrawEntities() )
	{
		IClientLeafSystem *pClientLeafSystem = ClientLeafSystem();
		for( i=0; i < info.m_LeafCount; i++ )
		{
			int nTranslucent = renderList.m_RenderGroupCounts[RENDER_GROUP_TRANSLUCENT_ENTITY];

			// Add renderables from this leaf...
			pClientLeafSystem->CollateRenderablesInLeaf( info.m_pLeafList[i], i, setupInfo );

			int nNewTranslucent = renderList.m_RenderGroupCounts[RENDER_GROUP_TRANSLUCENT_ENTITY] - nTranslucent;
			if( nNewTranslucent )
			{
				// Sort the new translucent entities.
				SortEntities( &renderList.m_RenderGroups[RENDER_GROUP_TRANSLUCENT_ENTITY][nTranslucent], nNewTranslucent );
			}
		}
	}
}

static void OverlayWaterTexture( void )
{
	float offsetS = ( 0.5f / 256.0f );
	float offsetT = ( 0.5f / 256.0f );
	IMaterial *pMaterial;
	pMaterial = materials->FindMaterial( "debug/debugreflect", TEXTURE_GROUP_OTHER, true );
	if( !IsErrorMaterial( pMaterial ) )
	{
		materials->Bind( pMaterial );
		IMesh* pMesh = materials->GetDynamicMesh( true );

		float w = mat_wateroverlaysize.GetFloat();
		float h = mat_wateroverlaysize.GetFloat();

		CMeshBuilder meshBuilder;
		meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

		meshBuilder.Position3f( 0.0f, 0.0f, 0.0f );
		meshBuilder.TexCoord2f( 0, 0.0f + offsetS, 1.0f + offsetT );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( w, 0.0f, 0.0f );
		meshBuilder.TexCoord2f( 0, 1.0f + offsetS, 1.0f + offsetT );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( w, h, 0.0f );
		meshBuilder.TexCoord2f( 0, 1.0f + offsetS, 0.0f + offsetT );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( 0.0f, h, 0.0f );
		meshBuilder.TexCoord2f( 0, 0.0f + offsetS, 0.0f + offsetT );
		meshBuilder.AdvanceVertex();

		meshBuilder.End();
		pMesh->Draw();
	}

	pMaterial = materials->FindMaterial( "debug/debugrefract", TEXTURE_GROUP_OTHER, true );
	if( !IsErrorMaterial( pMaterial ) )
	{
		materials->Bind( pMaterial );
		IMesh* pMesh = materials->GetDynamicMesh( true );

		float w = mat_wateroverlaysize.GetFloat();
		float h = mat_wateroverlaysize.GetFloat();
		float xoffset = mat_wateroverlaysize.GetFloat();


		CMeshBuilder meshBuilder;
		meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

		meshBuilder.Position3f( xoffset + 0.0f, 0.0f, 0.0f );
		meshBuilder.TexCoord2f( 0, 0.0f + offsetS, 0.0f + offsetT );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( xoffset + w, 0.0f, 0.0f );
		meshBuilder.TexCoord2f( 0, 1.0f + offsetS, 0.0f + offsetT );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( xoffset + w, h, 0.0f );
		meshBuilder.TexCoord2f( 0, 1.0f + offsetS, 1.0f + offsetT );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( xoffset + 0.0f, h, 0.0f );
		meshBuilder.TexCoord2f( 0, 0.0f + offsetS, 1.0f + offsetT );
		meshBuilder.AdvanceVertex();

		meshBuilder.End();
		pMesh->Draw();
	}

}

static void OverlayCameraRenderTarget( void )
{
	float offsetS = ( 0.5f / 256.0f );
	float offsetT = ( 0.5f / 256.0f );
	IMaterial *pMaterial;
	pMaterial = materials->FindMaterial( "debug/debugcamerarendertarget", TEXTURE_GROUP_OTHER, true );
	if( !IsErrorMaterial( pMaterial ) )
	{
		materials->Bind( pMaterial );
		IMesh* pMesh = materials->GetDynamicMesh( true );

		float w = mat_wateroverlaysize.GetFloat();
		float h = mat_wateroverlaysize.GetFloat();

		CMeshBuilder meshBuilder;
		meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

		meshBuilder.Position3f( 0.0f, 0.0f, 0.0f );
		meshBuilder.TexCoord2f( 0, 0.0f + offsetS, 0.0f + offsetT );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( w, 0.0f, 0.0f );
		meshBuilder.TexCoord2f( 0, 1.0f + offsetS, 0.0f + offsetT );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( w, h, 0.0f );
		meshBuilder.TexCoord2f( 0, 1.0f + offsetS, 1.0f + offsetT );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( 0.0f, h, 0.0f );
		meshBuilder.TexCoord2f( 0, 0.0f + offsetS, 1.0f + offsetT );
		meshBuilder.AdvanceVertex();

		meshBuilder.End();
		pMesh->Draw();
	}
}


static void OverlayFrameBufferTexture( void )
{
	float offsetS = ( 0.5f / 256.0f );
	float offsetT = ( 0.5f / 256.0f );
	IMaterial *pMaterial;
	pMaterial = materials->FindMaterial( "debug/debugfbtexture", TEXTURE_GROUP_OTHER, true );
	if( !IsErrorMaterial( pMaterial ) )
	{
		materials->Bind( pMaterial );
		IMesh* pMesh = materials->GetDynamicMesh( true );

		float w = mat_framebuffercopyoverlaysize.GetFloat();
		float h = mat_framebuffercopyoverlaysize.GetFloat();

		CMeshBuilder meshBuilder;
		meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

		meshBuilder.Position3f( 0.0f, 0.0f, 0.0f );
		meshBuilder.TexCoord2f( 0, 0.0f + offsetS, 0.0f + offsetT );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( w, 0.0f, 0.0f );
		meshBuilder.TexCoord2f( 0, 1.0f + offsetS, 0.0f + offsetT );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( w, h, 0.0f );
		meshBuilder.TexCoord2f( 0, 1.0f + offsetS, 1.0f + offsetT );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( 0.0f, h, 0.0f );
		meshBuilder.TexCoord2f( 0, 0.0f + offsetS, 1.0f + offsetT );
		meshBuilder.AdvanceVertex();

		meshBuilder.End();
		pMesh->Draw();
	}
}


void UpdateClientRenderableInPVSStatus()
{
	// Vis for this view should already be setup at this point.

	// For each client-only entity, notify it if it's newly coming into the PVS.
	CUtlLinkedList<CClientEntityList::CPVSNotifyInfo,unsigned short> &theList = ClientEntityList().GetPVSNotifiers();
	FOR_EACH_LL( theList, i )
	{
		CClientEntityList::CPVSNotifyInfo *pInfo = &theList[i];

		if ( pInfo->m_InPVSStatus & INPVS_YES )
		{
			// Ok, this entity already thinks it's in the PVS. No need to notify it.
			// We need to set the INPVS_YES_THISFRAME flag if it's in this frame at all, so we 
			// don't tell the entity it's not in the PVS anymore at the end of the frame.
			if ( !( pInfo->m_InPVSStatus & INPVS_THISFRAME ) )
			{
				if ( g_pClientLeafSystem->IsRenderableInPVS( pInfo->m_pRenderable ) )
				{
					pInfo->m_InPVSStatus |= INPVS_THISFRAME;
				}
			}
		}
		else
		{
			// This entity doesn't think it's in the PVS yet. If it is now in the PVS, let it know.
			if ( g_pClientLeafSystem->IsRenderableInPVS( pInfo->m_pRenderable ) )
			{
				pInfo->m_pNotify->OnPVSStatusChanged( true );
				pInfo->m_InPVSStatus |= INPVS_YES;
				pInfo->m_InPVSStatus |= INPVS_THISFRAME;
			}
		}
	}	
}

extern ConVar cl_leveloverview;
//-----------------------------------------------------------------------------
// Purpose: Builds lists of things to render in the world, called once per view
//-----------------------------------------------------------------------------
void CViewRender::BuildWorldRenderLists( const CViewSetup *pView, 
	ClientWorldListInfo_t& info, bool bUpdateLightmaps, bool bDrawEntities, int iForceViewLeaf )
{
	VPROF_BUDGET( "BuildWorldRenderLists", VPROF_BUDGETGROUP_WORLD_RENDERING );
	
	// Server entities already know which ones are in the PVS, but client-only entities don't.
	// We need to know which client-only entities are in the PVS at this point so the shadow
	// manager can project/occlude their shadows correctly.
	UpdateClientRenderableInPVSStatus();

	++m_BuildWorldListsNumber;
	render->BuildWorldLists( &info, bUpdateLightmaps, iForceViewLeaf );

	if ( bDrawEntities )
	{
		// Now that we have the list of all leaves, regenerate shadows cast
		g_pClientShadowMgr->ComputeShadowTextures( pView, info.m_LeafCount, info.m_pLeafList );

		// Compute the prop opacity based on the view position and fov zoom scale
		float flFactor = 1.0f;
		C_BasePlayer *pLocal = C_BasePlayer::GetLocalPlayer();
		if ( pLocal )
		{
			flFactor = pLocal->GetFOVDistanceAdjustFactor();
		}

		if ( cl_leveloverview.GetFloat() > 0 )
		{
			// disable prop fading
			flFactor = -1;
		}

		// When zoomed in, tweak the opacity to stay visible from further away
		staticpropmgr->ComputePropOpacity( CurrentViewOrigin(), flFactor );

		// Build a list of detail props to render
		DetailObjectSystem()->BuildDetailObjectRenderLists();
	}
}


//-----------------------------------------------------------------------------
// Purpose: Computes the actual world list info based on the render flags
//-----------------------------------------------------------------------------
ClientWorldListInfo_t *CViewRender::ComputeActualWorldListInfo( const ClientWorldListInfo_t& info, int nDrawFlags, ClientWorldListInfo_t& tmpInfo )
{
	// Drawing everything? Just return the world list info as-is 
	int nWaterDrawFlags = nDrawFlags & (DF_RENDER_UNDERWATER | DF_RENDER_ABOVEWATER);
	if ( nWaterDrawFlags == (DF_RENDER_UNDERWATER | DF_RENDER_ABOVEWATER) )
		return const_cast<ClientWorldListInfo_t*>(&info);

	tmpInfo.m_ViewFogVolume = info.m_ViewFogVolume;
	tmpInfo.m_LeafCount = 0;

	// Not drawing anything? Then don't bother with renderable lists
	if ( nWaterDrawFlags == 0 )
		return &tmpInfo;
		
	// Create a sub-list based on the actual leaves being rendered
	bool bRenderingUnderwater = (nWaterDrawFlags & DF_RENDER_UNDERWATER) != 0;
	for ( int i = 0; i < info.m_LeafCount; ++i )
	{
		bool bLeafIsUnderwater = ( info.m_pLeafFogVolume[i] != -1 );
		if ( bRenderingUnderwater == bLeafIsUnderwater )
		{
			tmpInfo.m_pLeafList[ tmpInfo.m_LeafCount ] = info.m_pLeafList[ i ];
			tmpInfo.m_pLeafFogVolume[ tmpInfo.m_LeafCount ] = info.m_pLeafFogVolume[ i ];
			tmpInfo.m_pActualLeafIndex[ tmpInfo.m_LeafCount ] = i;
			++tmpInfo.m_LeafCount;
		}
	}

	return &tmpInfo;
}


//-----------------------------------------------------------------------------
// Purpose: Builds render lists for renderables. Called once for refraction, once for over water
//-----------------------------------------------------------------------------
void CViewRender::BuildRenderableRenderLists( const CViewSetup *pView, ClientWorldListInfo_t& info, CRenderList &renderList )
{
	++m_BuildRenderableListsNumber;

	// For better sorting, find out the leaf *nearest* to the camera
	// and render translucent objects as if they are in that leaf.
	if( ShouldDrawEntities() )
	{
		ClientLeafSystem()->ComputeTranslucentRenderLeaf( 
			info.m_LeafCount, info.m_pLeafList, info.m_pLeafFogVolume, m_BuildRenderableListsNumber );
	}
	
	SetupRenderList( pView, info, renderList );
}


//-----------------------------------------------------------------------------
// Computes draw flags for the engine to build its world surface lists
//-----------------------------------------------------------------------------
static inline unsigned long BuildDrawFlags( bool bDrawSkybox, bool bDrawUnderWater, bool bDrawAboveWater, bool bDrawWaterSurface, bool bClipSkybox )
{
	unsigned long nEngineFlags = 0;

	if ( bDrawSkybox )
	{
		nEngineFlags |= DRAWWORLDLISTS_DRAW_SKYBOX;
	}

	if ( bDrawAboveWater )
	{
		nEngineFlags |= DRAWWORLDLISTS_DRAW_STRICTLYABOVEWATER;
		nEngineFlags |= DRAWWORLDLISTS_DRAW_INTERSECTSWATER;
	}

	if ( bDrawUnderWater )
	{
		nEngineFlags |= DRAWWORLDLISTS_DRAW_STRICTLYUNDERWATER;
		nEngineFlags |= DRAWWORLDLISTS_DRAW_INTERSECTSWATER;
	}

	if ( bDrawWaterSurface )
	{
		nEngineFlags |= DRAWWORLDLISTS_DRAW_WATERSURFACE;
	}

	if( bClipSkybox )
	{
		nEngineFlags |= DRAWWORLDLISTS_DRAW_CLIPSKYBOX;
	}

	return nEngineFlags;
}

void CViewRender::DrawWorld( ClientWorldListInfo_t& info, CRenderList &renderList, int flags, float waterZAdjust )
{
	VPROF_INCREMENT_COUNTER( "RenderWorld", 1 );
	VPROF_BUDGET( "DrawWorld", VPROF_BUDGETGROUP_WORLD_RENDERING );
	if( !r_drawopaqueworld.GetBool() )
	{
		return;
	}

	bool drawSkybox = (flags & DF_DRAWSKYBOX) != 0;
	bool drawUnderWater = (flags & DF_RENDER_UNDERWATER) != 0;
	bool drawAboveWater = (flags & DF_RENDER_ABOVEWATER) != 0;
	bool drawWaterSurface = (flags & DF_RENDER_WATER) != 0;
	bool bClipSkybox = (flags & DF_CLIP_SKYBOX ) != 0;

	unsigned long engineFlags = BuildDrawFlags( drawSkybox, drawUnderWater, drawAboveWater, drawWaterSurface, bClipSkybox );
	if ( flags & DF_MAINTAINWORLDLISTS )
	{
		engineFlags |= DRAWWORLDLISTS_MAINTAIN_RENDER_LISTS;
	}

	int oldDrawFlags = m_DrawFlags;
	m_DrawFlags |= flags;

	render->DrawWorldLists( engineFlags, waterZAdjust );

	m_DrawFlags = oldDrawFlags;
}

static inline void DrawOpaqueRenderable( IClientRenderable *pEnt, bool twoPass )
{
	float color[3];

	pEnt->GetColorModulation( color );
	render->SetColorModulation(	color );


	int flags = STUDIO_RENDER;
	if (twoPass)
	{
		flags |= STUDIO_TWOPASS;
	}

	pEnt->DrawModel( flags );
}


static inline void DrawTranslucentRenderable( IClientRenderable *pEnt, bool twoPass )
{
	// Determine blending amount and tell engine
	float blend = (float)( pEnt->GetFxBlend() / 255.0f );

	// Totally gone
	if ( blend <= 0.0f )
		return;

	// Tell engine
	render->SetBlend( blend );

	float color[3];
	pEnt->GetColorModulation( color );
	render->SetColorModulation(	color );

	int flags = STUDIO_RENDER | STUDIO_TRANSPARENCY;
	if (twoPass)
		flags |= STUDIO_TWOPASS;

#if 0
	Vector mins, maxs;
	pEnt->GetRenderBounds( mins, maxs );
	debugoverlay->AddBoxOverlay( pEnt->GetRenderOrigin(), mins, maxs, pEnt->GetRenderAngles(), 255, 255, 255, 64, 0.01 );
	if ( pEnt->GetModel() )
	{
		const char *pName = modelinfo->GetModelName( pEnt->GetModel() );
		if ( Q_stricmp( pName, "models/props_c17/tv_monitor01_screen.mdl" ) )
		{
			debugoverlay->AddTextOverlay( pEnt->GetRenderOrigin(), 0.01, pName );
		}
	}
#endif

	pEnt->DrawModel( flags );
}


//-----------------------------------------------------------------------------
// Draws all opaque renderables in leaves that were rendered
//-----------------------------------------------------------------------------
void CViewRender::DrawOpaqueRenderables( ClientWorldListInfo_t& info, CRenderList &renderList )
{
	VPROF("CViewRender::DrawOpaqueRenderables");

	if( !r_drawopaquerenderables.GetBool() )
		return;
	
	if( !ShouldDrawEntities() )
		return;

	render->SetBlend( 1 );
	
	// Iterate over all leaves that were visible, and draw opaque things in them.	
	bool bRopeBatch = ( r_ropebatch.GetInt() != 0 );
	if ( bRopeBatch )
	{
		RopeManager()->ResetRenderCache();
	}

	// Iterate over all leaves that were visible, and draw opaque things in them.
	CRenderList::CEntry *pEntities = renderList.m_RenderGroups[RENDER_GROUP_OPAQUE_ENTITY];
	int nOpaque = renderList.m_RenderGroupCounts[RENDER_GROUP_OPAQUE_ENTITY];

	for( int i=0; i < nOpaque; ++i )
	{
		DrawOpaqueRenderable( pEntities[i].m_pRenderable, (pEntities[i].m_TwoPass != 0) );
	}

	if ( bRopeBatch )
	{
		RopeManager()->DrawRenderCache();
	}
}


//-----------------------------------------------------------------------------
// Renders all translucent world + detail objects in a particular set of leaves
//-----------------------------------------------------------------------------
void CViewRender::DrawTranslucentWorldInLeaves( int iCurLeafIndex, int iFinalLeafIndex, ClientWorldListInfo_t &info, int nDrawFlags )
{
	VPROF_BUDGET( "CViewRender::DrawTranslucentWorldInLeaves", VPROF_BUDGETGROUP_WORLD_RENDERING );
	for( ; iCurLeafIndex >= iFinalLeafIndex; iCurLeafIndex-- )
	{
		int nActualLeafIndex = info.m_pActualLeafIndex ? info.m_pActualLeafIndex[ iCurLeafIndex ] : iCurLeafIndex;
		Assert( nActualLeafIndex != INVALID_LEAF_INDEX );
		if ( render->LeafContainsTranslucentSurfaces( nActualLeafIndex, nDrawFlags ) )
		{
			// Now draw the surfaces in this leaf
			render->DrawTranslucentSurfaces( nActualLeafIndex, nDrawFlags );
		}
	}
}


//-----------------------------------------------------------------------------
// Renders all translucent world + detail objects in a particular set of leaves
//-----------------------------------------------------------------------------
void CViewRender::DrawTranslucentWorldAndDetailPropsInLeaves( int iCurLeafIndex, int iFinalLeafIndex,
	ClientWorldListInfo_t &info, int nDrawFlags, int &nDetailLeafCount, LeafIndex_t* pDetailLeafList )
{
	VPROF_BUDGET( "CViewRender::DrawTranslucentWorldAndDetailPropsInLeaves", VPROF_BUDGETGROUP_WORLD_RENDERING );
	for( ; iCurLeafIndex >= iFinalLeafIndex; iCurLeafIndex-- )
	{
		int nActualLeafIndex = info.m_pActualLeafIndex ? info.m_pActualLeafIndex[ iCurLeafIndex ] : iCurLeafIndex;
		Assert( nActualLeafIndex != INVALID_LEAF_INDEX );
		if ( render->LeafContainsTranslucentSurfaces( nActualLeafIndex, nDrawFlags ) )
		{
			// First draw any queued-up detail props from previously visited leaves
			DetailObjectSystem()->RenderTranslucentDetailObjects( CurrentViewOrigin(), CurrentViewForward(), nDetailLeafCount, pDetailLeafList );
			nDetailLeafCount = 0;

			// Now draw the surfaces in this leaf
			render->DrawTranslucentSurfaces( nActualLeafIndex, nDrawFlags );
		}

		// Queue up detail props that existed in this leaf
		if ( ClientLeafSystem()->ShouldDrawDetailObjectsInLeaf( info.m_pLeafList[iCurLeafIndex], m_BuildWorldListsNumber ) )
		{
			pDetailLeafList[nDetailLeafCount] = info.m_pLeafList[iCurLeafIndex];
			++nDetailLeafCount;
		}
	}
}


//-----------------------------------------------------------------------------
// Renders all translucent entities in the render list
//-----------------------------------------------------------------------------
void CViewRender::DrawTranslucentRenderablesNoWorld( CRenderList &renderList, bool bInSkybox )
{
	VPROF( "CViewRender::DrawTranslucentRenderablesNoWorld" );

	if ( !ShouldDrawEntities() || !r_drawtranslucentrenderables.GetBool() )
		return;

	// Draw the particle singletons.
	DrawParticleSingletons( bInSkybox );
	
	CRenderList::CEntry *pEntities = renderList.m_RenderGroups[RENDER_GROUP_TRANSLUCENT_ENTITY];
	int iCurTranslucentEntity = renderList.m_RenderGroupCounts[RENDER_GROUP_TRANSLUCENT_ENTITY] - 1;

	while( iCurTranslucentEntity >= 0 )
	{
		IClientRenderable *pRenderable = pEntities[iCurTranslucentEntity].m_pRenderable;
		if ( pRenderable->UsesFrameBufferTexture() )
		{
			UpdateRefractTexture();
		}
		DrawTranslucentRenderable( pRenderable, (pEntities[iCurTranslucentEntity].m_TwoPass != 0) );
		--iCurTranslucentEntity;
	}

	// Reset the blend state.
	render->SetBlend( 1 );
}


//-----------------------------------------------------------------------------
// Renders all translucent world, entities, and detail objects in a particular set of leaves
//-----------------------------------------------------------------------------
void CViewRender::DrawTranslucentRenderables( ClientWorldListInfo_t& info, CRenderList &renderList, 
	int nFlags, bool bInSkybox )
{
	if ( !r_drawtranslucentworld.GetBool() )
	{
		DrawTranslucentRenderablesNoWorld( renderList, bInSkybox );
		return;
	}

	VPROF( "CViewRender::DrawTranslucentRenderables" );
	int iPrevLeaf = info.m_LeafCount - 1;
	int nDetailLeafCount = 0;
	LeafIndex_t *pDetailLeafList = (LeafIndex_t*)stackalloc( info.m_LeafCount * sizeof(LeafIndex_t) );

	bool bDrawUnderWater = (nFlags & DF_RENDER_UNDERWATER) != 0;
	bool bDrawAboveWater = (nFlags & DF_RENDER_ABOVEWATER) != 0;
	bool bDrawWater = (nFlags & DF_RENDER_WATER) != 0;
	bool bClipSkybox = (nFlags & DF_CLIP_SKYBOX ) != 0;
	unsigned long nDrawFlags = BuildDrawFlags( false, bDrawUnderWater, bDrawAboveWater, bDrawWater, bClipSkybox );

	DetailObjectSystem()->BeginTranslucentDetailRendering();
	
	if( ShouldDrawEntities() && r_drawtranslucentrenderables.GetBool() )
	{
		// Draw the particle singletons.
		DrawParticleSingletons( bInSkybox );
		
		CRenderList::CEntry *pEntities = renderList.m_RenderGroups[RENDER_GROUP_TRANSLUCENT_ENTITY];
		int iCurTranslucentEntity = renderList.m_RenderGroupCounts[RENDER_GROUP_TRANSLUCENT_ENTITY] - 1;

		while( iCurTranslucentEntity >= 0 )
		{
			// Seek the current leaf up to our current translucent-entity leaf.
			int iThisLeaf = pEntities[iCurTranslucentEntity].m_iWorldListInfoLeaf;

			// First draw the translucent parts of the world up to and including those in this leaf
			DrawTranslucentWorldAndDetailPropsInLeaves( iPrevLeaf, iThisLeaf, info, nDrawFlags, nDetailLeafCount, pDetailLeafList );

			// We're traversing the leaf list backwards to get the appropriate sort ordering (back to front)
			iPrevLeaf = iThisLeaf - 1;

			// Draw all the translucent entities with this leaf.
			int nLeaf = info.m_pLeafList[iThisLeaf];
			bool bDrawDetailProps = ClientLeafSystem()->ShouldDrawDetailObjectsInLeaf( nLeaf, m_BuildWorldListsNumber );
			if ( bDrawDetailProps )
			{
				// Draw detail props up to but not including this leaf
				Assert( nDetailLeafCount > 0 ); 
				--nDetailLeafCount;
				Assert( pDetailLeafList[nDetailLeafCount] == nLeaf );
				DetailObjectSystem()->RenderTranslucentDetailObjects( CurrentViewOrigin(), CurrentViewForward(), nDetailLeafCount, pDetailLeafList );

				// Draw translucent renderables in the leaf interspersed with detail props
				while( pEntities[iCurTranslucentEntity].m_iWorldListInfoLeaf == iThisLeaf && iCurTranslucentEntity >= 0 )
				{
					IClientRenderable *pRenderable = pEntities[iCurTranslucentEntity].m_pRenderable;

					// Draw any detail props in this leaf that's farther than the entity
					const Vector &vecRenderOrigin = pRenderable->GetRenderOrigin();
					DetailObjectSystem()->RenderTranslucentDetailObjectsInLeaf( 
						CurrentViewOrigin(), CurrentViewForward(), nLeaf, &vecRenderOrigin );

					if ( pRenderable->UsesFrameBufferTexture() )
					{
						UpdateRefractTexture();
					}

					// Then draw the translucent renderable
					DrawTranslucentRenderable( pRenderable, (pEntities[iCurTranslucentEntity].m_TwoPass != 0) );
					--iCurTranslucentEntity;
				}

				// Draw all remaining props in this leaf
				DetailObjectSystem()->RenderTranslucentDetailObjectsInLeaf( CurrentViewOrigin(), CurrentViewForward(), nLeaf, NULL );
			}
			else
			{
				// Draw queued up detail props (we know that the list of detail leaves won't include this leaf, since ShouldDrawDetailObjectsInLeaf is false)
				// Therefore no fixup on nDetailLeafCount is required as in the above section
 				DetailObjectSystem()->RenderTranslucentDetailObjects( CurrentViewOrigin(), CurrentViewForward(), nDetailLeafCount, pDetailLeafList );
				while( pEntities[iCurTranslucentEntity].m_iWorldListInfoLeaf == iThisLeaf && iCurTranslucentEntity >= 0 )
				{
					IClientRenderable *pRenderable = pEntities[iCurTranslucentEntity].m_pRenderable;
					if ( pRenderable->UsesFrameBufferTexture() )
					{
						UpdateRefractTexture();
					}
					DrawTranslucentRenderable( pRenderable, (pEntities[iCurTranslucentEntity].m_TwoPass != 0) );
					--iCurTranslucentEntity;
				}
			}
			nDetailLeafCount = 0;
		}
	}

	// Draw the rest of the surfaces in world leaves
	DrawTranslucentWorldAndDetailPropsInLeaves( iPrevLeaf, 0, info, nDrawFlags, nDetailLeafCount, pDetailLeafList );

	// Draw any queued-up detail props from previously visited leaves
	DetailObjectSystem()->RenderTranslucentDetailObjects( CurrentViewOrigin(), CurrentViewForward(), nDetailLeafCount, pDetailLeafList );

	// Reset the blend state.
	render->SetBlend( 1 );
}


//-----------------------------------------------------------------------------
// Renders the shadow texture to screen...
//-----------------------------------------------------------------------------
static void RenderMaterial( const char *pMaterialName )
{
	// So it's not in the very top left
	float x = 100.0f, y = 100.0f;
	// float x = 0.0f, y = 0.0f;

	IMaterial *pMaterial = materials->FindMaterial( pMaterialName, TEXTURE_GROUP_OTHER, false );
	if ( !IsErrorMaterial( pMaterial ) )
	{
		materials->Bind( pMaterial );
		IMesh* pMesh = materials->GetDynamicMesh( true );

		CMeshBuilder meshBuilder;
		meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

		meshBuilder.Position3f( x, y, 0.0f );
		meshBuilder.TexCoord2f( 0, 0.0f, 0.0f );
		meshBuilder.Color4ub( 255, 255, 255, 255 );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( x + pMaterial->GetMappingWidth(), y, 0.0f );
		meshBuilder.TexCoord2f( 0, 1.0f, 0.0f );
		meshBuilder.Color4ub( 255, 255, 255, 255 );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( x + pMaterial->GetMappingWidth(), y + pMaterial->GetMappingHeight(), 0.0f );
		meshBuilder.TexCoord2f( 0, 1.0f, 1.0f );
		meshBuilder.Color4ub( 255, 255, 255, 255 );
		meshBuilder.AdvanceVertex();

		meshBuilder.Position3f( x, y + pMaterial->GetMappingHeight(), 0.0f );
		meshBuilder.TexCoord2f( 0, 0.0f, 1.0f );
		meshBuilder.Color4ub( 255, 255, 255, 255 );
		meshBuilder.AdvanceVertex();

		meshBuilder.End();
		pMesh->Draw();
	}
}


//-----------------------------------------------------------------------------
// Purpose: Sets the screen space effect material (can't be done during rendering)
//-----------------------------------------------------------------------------
void CViewRender::SetScreenSpaceEffectMaterial( IMaterial *pMaterial )
{
	m_ScreenSpaceEffectMaterial.Init( pMaterial );
}


//-----------------------------------------------------------------------------
// Purpose: Performs screen space effects, if any
//-----------------------------------------------------------------------------
void CViewRender::PerformScreenSpaceEffects()
{
	VPROF("CViewRender::PerformScreenSpaceEffects()");

	if (m_ScreenSpaceEffectMaterial)
	{
		// First copy the FB off to the offscreen texture
		UpdateScreenEffectTexture( 0 );

		// Now draw the entire screen using the material...
		materials->DrawScreenSpaceQuad( m_ScreenSpaceEffectMaterial );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Sets the screen space effect material (can't be done during rendering)
//-----------------------------------------------------------------------------
void CViewRender::SetScreenOverlayMaterial( IMaterial *pMaterial )
{
	m_ScreenOverlayMaterial.Init( pMaterial );
}

IMaterial *CViewRender::GetScreenOverlayMaterial( )
{
	return m_ScreenOverlayMaterial;
}


//-----------------------------------------------------------------------------
// Purpose: Performs screen space effects, if any
//-----------------------------------------------------------------------------
void CViewRender::PerformScreenOverlay()
{
	VPROF("CViewRender::PerformScreenOverlay()");

	if (m_ScreenOverlayMaterial)
	{
		if ( m_ScreenOverlayMaterial->NeedsFullFrameBufferTexture() )
		{
			// First copy the FB off to the offscreen texture
			UpdateScreenEffectTexture( 0 );

			// Now draw the entire screen using the material...
			materials->DrawScreenSpaceQuad( m_ScreenOverlayMaterial );
		}
		else if( m_ScreenOverlayMaterial->NeedsPowerOfTwoFrameBufferTexture() )
		{
			// First copy the FB off to the offscreen texture
			UpdateRefractTexture();

			// Now draw the entire screen using the material...
			materials->DrawScreenSpaceQuad( m_ScreenOverlayMaterial );
		}
		else
		{
			byte color[4] = { 255, 255, 255, 255 };
			render->ViewDrawFade( color, m_ScreenOverlayMaterial );
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Renders world and all entities, etc.
//-----------------------------------------------------------------------------
void CViewRender::DrawWorldAndEntities( bool bDrawSkybox, const CViewSetup &view )
{
	VPROF("CViewRender::DrawWorldAndEntities");

	ClientWorldListInfo_t info;	
	CRenderList renderList;

	EnableWorldFog();

	int nFlags = DF_RENDER_UNDERWATER | DF_RENDER_ABOVEWATER | DF_RENDER_WATER;
	if (bDrawSkybox)
	{
		nFlags |= DF_DRAWSKYBOX;
	}

	render->ViewSetup3D( &view, m_Frustum );
	BuildWorldRenderLists( &view, info, true, true, -1 );
	BuildRenderableRenderLists( &view, info, renderList );

	// Make sure sound doesn't stutter
	engine->Sound_ExtraUpdate();

	m_DrawFlags = m_BaseDrawFlags;

	DrawWorld( info, renderList, nFlags, 0.0f );
	DrawOpaqueRenderables( info, renderList );
	DrawTranslucentRenderables( info, renderList, nFlags, false );

	m_DrawFlags = 0;

	g_ParticleMgr.DrawBeforeViewModelEffects();
}


//-----------------------------------------------------------------------------
// Computes us some geometry to render the frustum planes
//-----------------------------------------------------------------------------
void CViewRender::ComputeFrustumRenderGeometry( Vector pRenderPoint[8] )
{
	Vector viewPoint = CurrentViewOrigin();

	// Find lines along each of the plane intersections.
	// We know these lines are perpendicular to both plane normals,
	// so we can take the cross product to find them.
	static int edgeIdx[4][2] =
	{
		{ 0, 2 }, { 0, 3 }, { 1, 3 }, { 1, 2 }
	};

	int i;
	Vector edges[4];
	for ( i = 0; i < 4; ++i)
	{
		CrossProduct( GetFrustum()[edgeIdx[i][0]].m_Normal,
			GetFrustum()[edgeIdx[i][1]].m_Normal, edges[i] );
		VectorNormalize( edges[i] );
	}

	// Figure out four near points by intersection lines with the near plane
	// Figure out four far points by intersection with lines against far plane
	for (i = 0; i < 4; ++i)
	{
		float t = (GetFrustum()[4].m_Dist - DotProduct(GetFrustum()[4].m_Normal, viewPoint)) /
			DotProduct(GetFrustum()[4].m_Normal, edges[i]);
		VectorMA( viewPoint, t, edges[i], pRenderPoint[i] );

		/*
		t = (m_FrustumPlanes[5][3] - DotProduct(GetFrustum()[5], viewPoint)) /
			DotProduct(GetFrustum()[5], edges[i]);
		VectorMA( viewPoint, t, edges[i], pRenderPoint[i + 4] );
		*/
		if (t < 0)
		{
			edges[i] *= -1;
		}

		VectorMA( pRenderPoint[i], 200.0, edges[i], pRenderPoint[i + 4] );
	}
}


//-----------------------------------------------------------------------------
// renders the frustum
//-----------------------------------------------------------------------------
void CViewRender::RenderFrustum( )
{
	static int indices[] = 
	{
		0, 1, 1, 2, 2, 3, 3, 0,	// near square
		4, 5, 5, 6, 6, 7, 7, 4,	// far square
		0, 4, 1, 5, 2, 6, 3, 7	// connections between them
	};

	int numIndices = sizeof(indices) / sizeof(int);

	Vector vecFrustumRenderPoint[8];
	ComputeFrustumRenderGeometry( vecFrustumRenderPoint );

	int i;
	for ( i = 0; i < numIndices; i += 2 )
	{
		debugoverlay->AddLineOverlay( vecFrustumRenderPoint[indices[i]],
			vecFrustumRenderPoint[indices[i+1]], 0, 0, 255, 255, 1.0f );
	}
}


//-----------------------------------------------------------------------------
// Draws all the debugging info
//-----------------------------------------------------------------------------
void CViewRender::Draw3DDebuggingInfo( const CViewSetup &view )
{
	VPROF("CViewRender::Draw3DDebuggingInfo");

	// Draw 3d overlays
	render->Draw3DDebugOverlays();

	// Draw the line file used for debugging leaks
	render->DrawLineFile();
	
	// Draw client side effects
	// NOTE: These are not sorted against the rest of the frame
	clienteffects->DrawEffects( gpGlobals->frametime );	

	// Mark the frame as locked down for client fx additions
	SetFXCreationAllowed( false );
}


//-----------------------------------------------------------------------------
// Draws all the debugging info
//-----------------------------------------------------------------------------
void CViewRender::Draw2DDebuggingInfo( const CViewSetup &view )
{
	if ( mat_yuv.GetInt() && (engine->GetDXSupportLevel() >= 80) )
	{
		IMaterial *pMaterial;
		pMaterial = materials->FindMaterial( "debug/yuv", TEXTURE_GROUP_OTHER, true );
		if( !IsErrorMaterial( pMaterial ) )
		{
			// First copy the FB off to the offscreen texture
			UpdateScreenEffectTexture( 0 );
			materials->DrawScreenSpaceQuad( pMaterial );
		}
	}

	if ( mat_hsv.GetInt() && (engine->GetDXSupportLevel() >= 90) )
	{
		IMaterial *pMaterial;
		pMaterial = materials->FindMaterial( "debug/hsv", TEXTURE_GROUP_OTHER, true );
		if( !IsErrorMaterial( pMaterial ) )
		{
			// First copy the FB off to the offscreen texture
			UpdateScreenEffectTexture( 0 );
			materials->DrawScreenSpaceQuad( pMaterial );
		}
	}

	if( mat_bloom.GetInt() > 1 && engine->SupportsHDR() )
	{
		IMaterial *pMaterial = NULL;
		switch( mat_bloom.GetInt() )
		{
		case 2:
			pMaterial = materials->FindMaterial( "debug/showdestalpha", TEXTURE_GROUP_OTHER, true );
			break;
		case 3:
			pMaterial = materials->FindMaterial( "debug/showdestalpha_blurred", TEXTURE_GROUP_OTHER, true );
			break;
		case 4:
			pMaterial = materials->FindMaterial( "debug/showblurredcolor", TEXTURE_GROUP_OTHER, true );
			break;
		case 5:
			pMaterial = materials->FindMaterial( "debug/showdestalphatimescolor_blurred", TEXTURE_GROUP_OTHER, true );
			break;
		default:
			break;
		}
		if( !IsErrorMaterial( pMaterial ) )
		{
			// Since bloom is ON, FB texture is already copied
			materials->DrawScreenSpaceQuad( pMaterial );
		}
	}

	// Draw debugging lightmaps
	render->DrawLightmaps();

	if( cl_drawshadowtexture.GetInt() )
	{
		g_pClientShadowMgr->RenderShadowTexture( view.width, view.height );
	}

	const char *pDrawMaterial = cl_drawmaterial.GetString();
	if( pDrawMaterial && pDrawMaterial[0] )
	{
		RenderMaterial( pDrawMaterial ); 
	}

	if( mat_showwatertextures.GetBool() )
	{
		OverlayWaterTexture();
	}

	if( mat_showcamerarendertarget.GetBool() )
	{
		OverlayCameraRenderTarget();
	}

	if( mat_showframebuffertexture.GetBool() )
	{
		OverlayFrameBufferTexture();
	}
}


//-----------------------------------------------------------------------------
// Purpose: Renders world and all entities, etc.
//-----------------------------------------------------------------------------
void CViewRender::ViewDrawScene( bool drawSkybox, const CViewSetup &view, view_id_t viewID, bool bSetupViewModel, bool bDrawViewModel, int baseDrawFlags )
{
	VPROF( "CViewRender::ViewDrawScene" );

	m_BaseDrawFlags = baseDrawFlags;
	m_DrawFlags = 0;

	SetupCurrentView( view.origin, view.angles, viewID );

	// Invoke pre-render methods
	IGameSystem::PreRenderAllSystems();

	// Start view, clear frame/z buffer if necessary
	SetupVis( view );

	g_ParticleMgr.IncrementFrameCode();

#if !defined( TF2_CLIENT_DLL )
	WaterDrawWorldAndEntities( drawSkybox, view );
#else
	DrawWorldAndEntities( drawSkybox, view );
#endif

	// Disable fog for the rest of the stuff
	DisableFog();

	// UNDONE: Don't do this with masked brush models, they should probably be in a separate list
	// render->DrawMaskEntities()
	
	// Here are the overlays...

	// This is an overlay that goes over everything else
#if defined( TF2_CLIENT_DLL )
	CGroundLine::DrawAllGroundLines();
#endif

	CGlowOverlay::DrawOverlays();
	// issue the pixel visibility tests
	PixelVisibility_EndCurrentView();

	// Draw rain..
	DrawPrecipitation();

	// And here are the screen-space effects
	PerformScreenSpaceEffects();

	// Make sure sound doesn't stutter
	engine->Sound_ExtraUpdate();

	// Debugging info goes over the top
	Draw3DDebuggingInfo( view );

	FinishCurrentView();

	m_DrawFlags = 0;
}



//-----------------------------------------------------------------------------
// Purpose: Returns the fog color to use in rendering the current frame.
//-----------------------------------------------------------------------------
static void GetFogColor( float *pColor )
{
	C_BasePlayer *pbp = C_BasePlayer::GetLocalPlayer();
	if( !pbp )
	{
		return;
	}
	CPlayerLocalData	*local		= &pbp->m_Local;

	const char *fogColorString = fog_color.GetString();
	if( fog_override.GetInt() && fogColorString )
	{
		sscanf( fogColorString, "%f%f%f", pColor, pColor+1, pColor+2 );
	}
	else
	{
		if( local->m_fog.blend )
		{
			//
			// Blend between two fog colors based on viewing angle.
			// The secondary fog color is at 180 degrees to the primary fog color.
			//
			Vector forward;
			AngleVectors(pbp->GetAbsAngles(), &forward);
			
			Vector vNormalized = local->m_fog.dirPrimary;
			VectorNormalize( vNormalized );
			local->m_fog.dirPrimary = vNormalized;

			float flBlendFactor = 0.5 * forward.Dot( local->m_fog.dirPrimary ) + 0.5;

			// FIXME: convert to linear colorspace
			pColor[0] = local->m_fog.colorPrimary.GetR() * flBlendFactor + local->m_fog.colorSecondary.GetR() * ( 1 - flBlendFactor );
			pColor[1] = local->m_fog.colorPrimary.GetG() * flBlendFactor + local->m_fog.colorSecondary.GetG() * ( 1 - flBlendFactor );
			pColor[2] = local->m_fog.colorPrimary.GetB() * flBlendFactor + local->m_fog.colorSecondary.GetB() * ( 1 - flBlendFactor );
		}
		else
		{
			pColor[0] = local->m_fog.colorPrimary.GetR();
			pColor[1] = local->m_fog.colorPrimary.GetG();
			pColor[2] = local->m_fog.colorPrimary.GetB();
		}
	}

	VectorScale( pColor, 1.0f / 255.0f, pColor );
}


static float GetFogStart( void )
{
	C_BasePlayer *pbp = C_BasePlayer::GetLocalPlayer();
	if( !pbp )
	{
		return 0.0f;
	}
	CPlayerLocalData	*local		= &pbp->m_Local;

	if( fog_override.GetInt() )
	{
		if( fog_start.GetFloat() == -1.0f )
		{
			return local->m_fog.start;
		}
		else
		{
			return fog_start.GetFloat();
		}
	}
	else
	{
		return local->m_fog.start;
	}
}

static float GetFogEnd( void )
{
	C_BasePlayer *pbp = C_BasePlayer::GetLocalPlayer();
	if( !pbp )
	{
		return 0.0f;
	}
	CPlayerLocalData	*local		= &pbp->m_Local;

	if( fog_override.GetInt() )
	{
		if( fog_end.GetFloat() == -1.0f )
		{
			return local->m_fog.end;
		}
		else
		{
			return fog_end.GetFloat();
		}
	}
	else
	{
		return local->m_fog.end;
	}
}

static bool GetFogEnable( void )
{
	C_BasePlayer *pbp = C_BasePlayer::GetLocalPlayer();
	if( !pbp )
	{
		return false;
	}
	CPlayerLocalData	*local		= &pbp->m_Local;

	if ( cl_leveloverview.GetFloat() > 0 )
		return false;

	// Ask the clientmode
	if ( g_pClientMode->ShouldDrawFog() == false )
		return false;

	if( fog_override.GetInt() )
	{
		if( fog_enable.GetInt() )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return local->m_fog.enable != false;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Returns the skybox fog color to use in rendering the current frame.
//-----------------------------------------------------------------------------
static void GetSkyboxFogColor( float *pColor )
{
	C_BasePlayer *pbp = C_BasePlayer::GetLocalPlayer();
	if( !pbp )
	{
		return;
	}
	CPlayerLocalData	*local		= &pbp->m_Local;

	const char *fogColorString = fog_colorskybox.GetString();
	if( fog_override.GetInt() && fogColorString )
	{
		sscanf( fogColorString, "%f%f%f", pColor, pColor+1, pColor+2 );
	}
	else
	{
		if( local->m_skybox3d.fog.blend )
		{
			//
			// Blend between two fog colors based on viewing angle.
			// The secondary fog color is at 180 degrees to the primary fog color.
			//
			Vector forward;
			AngleVectors(pbp->GetAbsAngles(), &forward);
			
			Vector vNormalized;
			VectorNormalize(vNormalized);
			local->m_skybox3d.fog.dirPrimary = vNormalized;

			float flBlendFactor = 0.5 * forward.Dot( local->m_skybox3d.fog.dirPrimary ) + 0.5;

			// FIXME: convert to linear colorspace
			pColor[0] = local->m_skybox3d.fog.colorPrimary.GetR() * flBlendFactor + local->m_skybox3d.fog.colorSecondary.GetR() * ( 1 - flBlendFactor );
			pColor[1] = local->m_skybox3d.fog.colorPrimary.GetG() * flBlendFactor + local->m_skybox3d.fog.colorSecondary.GetG() * ( 1 - flBlendFactor );
			pColor[2] = local->m_skybox3d.fog.colorPrimary.GetB() * flBlendFactor + local->m_skybox3d.fog.colorSecondary.GetB() * ( 1 - flBlendFactor );
		}
		else
		{
			pColor[0] = local->m_skybox3d.fog.colorPrimary.GetR();
			pColor[1] = local->m_skybox3d.fog.colorPrimary.GetG();
			pColor[2] = local->m_skybox3d.fog.colorPrimary.GetB();
		}
	}

	VectorScale( pColor, 1.0f / 255.0f, pColor );
}


static float GetSkyboxFogStart( void )
{
	C_BasePlayer *pbp = C_BasePlayer::GetLocalPlayer();
	if( !pbp )
	{
		return 0.0f;
	}
	CPlayerLocalData	*local		= &pbp->m_Local;

	if( fog_override.GetInt() )
	{
		if( fog_startskybox.GetFloat() == -1.0f )
		{
			return local->m_skybox3d.fog.start;
		}
		else
		{
			return fog_startskybox.GetFloat();
		}
	}
	else
	{
		return local->m_skybox3d.fog.start;
	}
}

static float GetSkyboxFogEnd( void )
{
	C_BasePlayer *pbp = C_BasePlayer::GetLocalPlayer();
	if( !pbp )
	{
		return 0.0f;
	}
	CPlayerLocalData	*local		= &pbp->m_Local;

	if( fog_override.GetInt() )
	{
		if( fog_endskybox.GetFloat() == -1.0f )
		{
			return local->m_skybox3d.fog.end;
		}
		else
		{
			return fog_endskybox.GetFloat();
		}
	}
	else
	{
		return local->m_skybox3d.fog.end;
	}
}

static bool GetSkyboxFogEnable( void )
{
	C_BasePlayer *pbp = C_BasePlayer::GetLocalPlayer();
	if( !pbp )
	{
		return false;
	}
	CPlayerLocalData	*local		= &pbp->m_Local;

	if( fog_override.GetInt() )
	{
		if( fog_enableskybox.GetInt() )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return !!local->m_skybox3d.fog.enable;
	}
}

void CViewRender::EnableWorldFog( void )
{
	VPROF("CViewRender::EnableWorldFog");
	if( GetFogEnable() )
	{
		float fogColor[3];
		GetFogColor( fogColor );
		materials->FogMode( MATERIAL_FOG_LINEAR );
		materials->FogColor3fv( fogColor );
		materials->FogStart( GetFogStart() );
		materials->FogEnd( GetFogEnd() );
	}
	else
	{
		materials->FogMode( MATERIAL_FOG_NONE );
	}
}

void CViewRender::Enable3dSkyboxFog( void )
{
	C_BasePlayer *pbp = C_BasePlayer::GetLocalPlayer();
	if( !pbp )
	{
		return;
	}
	CPlayerLocalData	*local		= &pbp->m_Local;

	if( GetSkyboxFogEnable() )
	{
		float fogColor[3];
		GetSkyboxFogColor( fogColor );
		float scale = 1.0f;
		if ( local->m_skybox3d.scale > 0.0f )
		{
			scale = 1.0f / local->m_skybox3d.scale;
		}
		materials->FogMode( MATERIAL_FOG_LINEAR );
		materials->FogColor3fv( fogColor );
		materials->FogStart( GetSkyboxFogStart() * scale );
		materials->FogEnd( GetSkyboxFogEnd() * scale );
	}
	else
	{
		materials->FogMode( MATERIAL_FOG_NONE );
	}
}

void CViewRender::DisableFog( void )
{
	VPROF("CViewRander::DisableFog()");

	materials->FogMode( MATERIAL_FOG_NONE );
}

bool CViewRender::Draw3dSkyboxworld( const CViewSetup &view )
{
	VPROF_BUDGET( "CViewRender::Draw3dSkyboxworld", "3D Skybox" );

	// render the 3D skybox
	if ( !r_3dsky.GetInt() )
		return false;

	// The skybox might not be visible from here
	if ( !engine->IsSkyboxVisibleFromPoint( view.origin ) )
		return false;

	C_BasePlayer *pbp = C_BasePlayer::GetLocalPlayer();

	// No local player object yet...
	if ( !pbp )
		return false;

	CPlayerLocalData* local = &pbp->m_Local;
	if ( local->m_skybox3d.area == 255 )
		return false;

	unsigned char **areabits = render->GetAreaBits();
	unsigned char *savebits;
	unsigned char tmpbits[ 32 ];
	savebits = *areabits;
	memset( tmpbits, 0, sizeof(tmpbits) );
	
	// set the sky area bit
	tmpbits[local->m_skybox3d.area>>3] |= 1 << (local->m_skybox3d.area&7);

	*areabits = tmpbits;
	CViewSetup skyView = view;
	skyView.zNear = 0.5;
	skyView.zFar = 18000;
	skyView.clearDepth = true;
	skyView.clearColor = view.clearColor;

	// scale origin by sky scale
	if ( local->m_skybox3d.scale > 0 )
	{
		float scale = 1.0f/local->m_skybox3d.scale;
		VectorScale( skyView.origin, scale, skyView.origin );
	}
	Enable3dSkyboxFog();
	VectorAdd( skyView.origin, local->m_skybox3d.origin, skyView.origin );
	
	skyView.m_vUnreflectedOrigin = skyView.origin;

	// BUGBUG: Fix this!!!  We shouldn't need to call setup vis for the sky if we're connecting
	// the areas.  We'd have to mark all the clusters in the skybox area in the PVS of any 
	// cluster with sky.  Then we could just connect the areas to do our vis.
	//m_bOverrideVisOrigin could hose us here, so call direct
	render->ViewSetupVis( false, 1, &local->m_skybox3d.origin.Get() );
	render->ViewSetup3D( &skyView, m_Frustum );

	// Store off view origin and angles
	SetupCurrentView( skyView.origin, skyView.angles, VIEW_3DSKY );

	// Invoke pre-render methods
	IGameSystem::PreRenderAllSystems();

	ClientWorldListInfo_t info;
	CRenderList renderList;

	BuildWorldRenderLists( NULL, info, true, true, -1 );
	BuildRenderableRenderLists( NULL, info, renderList );

	int flags = DF_RENDER_UNDERWATER | DF_RENDER_ABOVEWATER | DF_RENDER_WATER;
	if( r_skybox.GetBool() )
	{
		flags |= DF_DRAWSKYBOX;
	}
	
	int oldDrawFlags = m_DrawFlags;
	m_DrawFlags |= flags;
	
	DrawWorld( info, renderList, flags, 0.0f );

	// Iterate over all leaves and render objects in those leaves
	DrawOpaqueRenderables( info, renderList );

	// Iterate over all leaves and render objects in those leaves
	DrawTranslucentRenderables( info, renderList, flags, true );

	DisableFog();

	CGlowOverlay::UpdateSkyOverlays( skyView.zFar );
	
	PixelVisibility_EndCurrentView();

	// restore old area bits
	*areabits = savebits;

	FinishCurrentView();

	m_DrawFlags = oldDrawFlags;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CViewRender::SetupVis( const CViewSetup& view )
{
	VPROF( "CViewRender::SetupVis" );

	if ( m_bOverrideVisOrigin )
	{
		// Pass array or vis origins to merge
		render->ViewSetupVis( m_bForceNoVis, m_nNumVisOrigins, m_rgVisOrigins );
	}
	else
	{
		// Use render origin as vis origin by default
		render->ViewSetupVis( m_bForceNoVis, 1, &view.origin );
	}
}


// GR - HDR
static void DoScreenSpaceBloom( void )
{
	int w, h;
	IMaterial *pMatDownsample;
	IMaterial *pMatBlurX;
	IMaterial *pMatBlurY;
	IMaterial *pMatBloom;

	pMatDownsample = materials->FindMaterial( "dev/downsample", TEXTURE_GROUP_OTHER, true );
	if( IsErrorMaterial( pMatDownsample ) )
		return;
	pMatBlurX = materials->FindMaterial( "dev/blurfilterx", TEXTURE_GROUP_OTHER, true );
	if( IsErrorMaterial( pMatBlurX ) )
		return;
	pMatBlurY = materials->FindMaterial( "dev/blurfiltery", TEXTURE_GROUP_OTHER, true );
	if( IsErrorMaterial( pMatBlurY ) )
		return;
	pMatBloom = materials->FindMaterial( "dev/bloom", TEXTURE_GROUP_OTHER, true );
	if( IsErrorMaterial( pMatBloom ) )
		return;

	int oldX, oldY, oldW, oldH;
	materials->GetViewport( oldX, oldY, oldW, oldH );

	ITexture *pSaveRenderTarget = materials->GetRenderTarget();

	ITexture *pFBTexture = GetFullFrameFrameBufferTexture( 0 );
	ITexture *pTex0 = GetSmallBufferHDR0();
	ITexture *pTex1 = GetSmallBufferHDR1();

	// First copy the FB off to the offscreen texture
	materials->CopyRenderTargetToTexture( pFBTexture );
	
	w = pTex0->GetActualWidth();
	h = pTex0->GetActualHeight();

	// Downsample image
	materials->SetRenderTarget( pTex0 );
	materials->Viewport( 0, 0, w, h );
	materials->DrawScreenSpaceQuad( pMatDownsample );

	// Blur filter pass 1
	materials->SetRenderTarget( pTex1 );
	materials->Viewport( 0, 0, w, h );
	materials->DrawScreenSpaceQuad( pMatBlurX );

	// Blur filter pass 2
	materials->SetRenderTarget( pTex0 );
	materials->Viewport( 0, 0, w, h );
	materials->DrawScreenSpaceQuad( pMatBlurY );

	// Render bloom
	materials->SetRenderTarget( pSaveRenderTarget );
	materials->Viewport( oldX, oldY, oldW, oldH );
	materials->DrawScreenSpaceQuad( pMatBloom );
}

//-----------------------------------------------------------------------------
// Purpose: Renders voice feedback and other sprites attached to players
// Input  : none
//-----------------------------------------------------------------------------
void CViewRender::RenderPlayerSprites()
{
	GetClientVoiceMgr()->DrawHeadLabels();
}

//-----------------------------------------------------------------------------
// Purpose: Renders entire view
// Input  : &view - 
//			drawViewModel - 
//-----------------------------------------------------------------------------
void CViewRender::RenderView( const CViewSetup &view, bool drawViewModel )
{
	VPROF( "CViewRender::RenderView" );

	CViewSetup tmpView = view;
	g_bRenderingView = true;

	// Must be first 
	render->SceneBegin();
		
	bool drawSky = r_skybox.GetBool();

	// if the 3d skybox world is drawn, then don't draw the normal skybox
	if ( Draw3dSkyboxworld( view ) )
	{
		// don't clear the framebuffer, we'll erase the 3d skybox
		tmpView.clearColor = false;
		
		// skip the normal skybox.
		drawSky = false;
	}

	// Force it to clear the framebuffer if they're in solid space.
	if ( !tmpView.clearColor )
	{
		if ( enginetrace->GetPointContents( tmpView.origin ) == CONTENTS_SOLID )
		{
			tmpView.clearColor = true;
		}
	}

	// Render world and all entities, particles, etc.
	if( !g_pIntroData )
	{
		ViewDrawScene( drawSky, tmpView, VIEW_MAIN, true, drawViewModel );
	}
	else
	{
		ViewDrawScene_Intro( tmpView, *g_pIntroData );
	}

	// We can still use the 'current view' stuff set up in ViewDrawScene
	s_bCanAccessCurrentView = true;

	engine->DrawPortals();

	DisableFog();

	// Finish scene
	render->SceneEnd();

	// Draw lightsources if enabled
	render->DrawLights();

	RenderPlayerSprites();

	// Now actually draw the viewmodel
	DrawViewModels( tmpView, drawViewModel );

	PixelVisibility_EndScene();
	g_pClientMode->PostRenderWorld();

	// Draw fade over entire screen if needed
	byte color[4];
	bool blend;
	vieweffects->GetFadeParams( tmpView.context, &color[0], &color[1], &color[2], &color[3], &blend );

	// Draw an overlay to make it even harder to see inside smoke particle systems.
	DrawSmokeFogOverlay();

	// Overlay screen fade on entire screen
	IMaterial* pMaterial = blend ? m_ModulateSingleColor : m_TranslucentSingleColor;
	render->ViewDrawFade( color, pMaterial );
	PerformScreenOverlay();

	// Prevent sound stutter if going slow
	engine->Sound_ExtraUpdate();	

	// GR - HDR
	if( mat_bloom.GetBool() && engine->SupportsHDR() )
	{
		DoScreenSpaceBloom();
	}

	// Draw the 2D graphics
	tmpView.clearColor = false;
	tmpView.clearDepth = false;
	render->ViewSetup2D( &tmpView );

	Draw2DDebuggingInfo( tmpView );

	m_AnglesHistory[m_AnglesHistoryCounter] = tmpView.angles;
	m_AnglesHistoryCounter = (m_AnglesHistoryCounter+1) & ANGLESHISTORY_MASK;
	
	// If the angles are moving fast enough, allow LOD transitions.
	float angleMovementDelta = 0;
	for(int i=0; i < ANGLESHISTORY_SIZE; i++)
	{
		angleMovementDelta += (m_AnglesHistory[(i+1) & ANGLESHISTORY_MASK] - m_AnglesHistory[i]).Length();
	}

	angleMovementDelta /= ANGLESHISTORY_SIZE;
	if(angleMovementDelta > r_TransitionSensitivity.GetFloat())
	{
		r_DoCovertTransitions.SetValue(1);
	}
	else
	{
		r_DoCovertTransitions.SetValue(0);
	}

	g_bRenderingView = false;

	// We can no longer use the 'current view' stuff set up in ViewDrawScene
	s_bCanAccessCurrentView = false;

	// Next frame!
	++m_FrameNumber;

	GenerateOverdrawForTesting();
}


int CViewRender::GetDrawFlags()
{
	return m_DrawFlags;
}


void ViewTransform( const Vector &worldSpace, Vector &viewSpace )
{
	const VMatrix &viewMatrix = engine->WorldToViewMatrix();
	Vector3DMultiplyPosition( viewMatrix, worldSpace, viewSpace );
}


//-----------------------------------------------------------------------------
// Purpose: UNDONE: Clean this up some, handle off-screen vertices
// Input  : *point - 
//			*screen - 
// Output : int
//-----------------------------------------------------------------------------
int ScreenTransform( const Vector& point, Vector& screen )
{
// UNDONE: Clean this up some, handle off-screen vertices
	float w;
	const VMatrix &worldToScreen = engine->WorldToScreenMatrix();

	screen.x = worldToScreen[0][0] * point[0] + worldToScreen[0][1] * point[1] + worldToScreen[0][2] * point[2] + worldToScreen[0][3];
	screen.y = worldToScreen[1][0] * point[0] + worldToScreen[1][1] * point[1] + worldToScreen[1][2] * point[2] + worldToScreen[1][3];
//	z		 = worldToScreen[2][0] * point[0] + worldToScreen[2][1] * point[1] + worldToScreen[2][2] * point[2] + worldToScreen[2][3];
	w		 = worldToScreen[3][0] * point[0] + worldToScreen[3][1] * point[1] + worldToScreen[3][2] * point[2] + worldToScreen[3][3];

	// Just so we have something valid here
	screen.z = 0.0f;

	bool behind;
	if( w < 0.001f )
	{
		behind = true;
		screen.x *= 100000;
		screen.y *= 100000;
	}
	else
	{
		behind = false;
		float invw = 1.0f / w;
		screen.x *= invw;
		screen.y *= invw;
	}

	return behind;
}



//-----------------------------------------------------------------------------
//
// NOTE: Below here is all of the stuff that needs to be done for water rendering
//
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Determines what kind of water we're going to use
//-----------------------------------------------------------------------------
void CViewRender::DetermineWaterRenderInfo( const VisibleFogVolumeInfo_t &fogVolumeInfo, CViewRender::WaterRenderInfo_t &info )
{
	// By default, assume cheap water (even if there's no water in the scene!)
	info.m_bCheapWater = true;
	info.m_bRefract = false;
	info.m_bReflect = false;
	info.m_bReflectEntities = false;
	info.m_bDrawWaterSurface = false;
	info.m_bOpaqueWater = true;

	IMaterial *pWaterMaterial = fogVolumeInfo.m_pFogVolumeMaterial;
	if (( fogVolumeInfo.m_nVisibleFogVolume == -1 ) || !pWaterMaterial )
		return;

	// Use cheap water if mat_drawwater is set
	info.m_bDrawWaterSurface = mat_drawwater.GetBool();
	if ( !info.m_bDrawWaterSurface )
	{
		info.m_bOpaqueWater = false;
		return;
	}

	// Determine if the water surface is opaque or not
	info.m_bOpaqueWater = !pWaterMaterial->IsTranslucent();

	// DX level 70 can't handle anything but cheap water
	if (engine->GetDXSupportLevel() < 80)
		return;

	bool bForceCheap = false;
	bool bForceExpensive = r_waterforceexpensive.GetBool();

	// The material can override the default settings though
	IMaterialVar *pForceCheapVar = pWaterMaterial->FindVar( "$forcecheap", NULL, false );
	IMaterialVar *pForceExpensiveVar = pWaterMaterial->FindVar( "$forceexpensive", NULL, false );
	if ( pForceCheapVar && pForceCheapVar->IsDefined() )
	{
		bForceCheap = ( pForceCheapVar->GetIntValue() != 0 );
		if ( bForceCheap )
		{
			bForceExpensive = false;
		}
	}
	if ( !bForceCheap && pForceExpensiveVar && pForceExpensiveVar->IsDefined() )
	{
		 bForceExpensive = bForceExpensive || ( pForceExpensiveVar->GetIntValue() != 0 );
	}

	bool bDebugCheapWater = r_debugcheapwater.GetBool();
	if( bDebugCheapWater )
	{
		Msg( "Water material: %s dist to water: %f\nforcecheap: %s forceexpensive: %s\n", 
			pWaterMaterial->GetName(), fogVolumeInfo.m_flDistanceToWater, 
			bForceCheap ? "true" : "false", bForceExpensive ? "true" : "false" );
	}

	// Unless expensive water is active, reflections are off.
	bool bLocalReflection;
	if( !bForceExpensive || !r_WaterDrawReflection.GetBool() )
	{
		bLocalReflection = false;
	}
	else
	{
		IMaterialVar *pReflectTextureVar = pWaterMaterial->FindVar( "$reflecttexture", NULL, false );
		bLocalReflection = pReflectTextureVar && (pReflectTextureVar->GetType() == MATERIAL_VAR_TYPE_TEXTURE);
	}

	// FIXME: I disabled cheap water LOD when local specular is specified.
	// There are very few places that appear to actually
	// take advantage of it (places where water is in the PVS, but outside of LOD range).
	// It was 2 hours before code lock, and I had the choice of either doubling fill-rate everywhere
	// by making cheap water lod actually work (the water LOD wasn't actually rendering!!!)
	// or to just always render the reflection + refraction if there's a local specular specified.
	// Note that water LOD *does* work with refract-only water

	// Check if the water is out of the cheap water LOD range; if so, use cheap water
	if ( ( (fogVolumeInfo.m_flDistanceToWater >= m_flCheapWaterEndDistance) && !bLocalReflection ) || bForceCheap )
 		return;

	// Get the material that is for the water surface that is visible and check to see
	// what render targets need to be rendered, if any.
	if ( !r_WaterDrawRefraction.GetBool() )
	{
		info.m_bRefract = false;
	}
	else
	{
		IMaterialVar *pRefractTextureVar = pWaterMaterial->FindVar( "$refracttexture", NULL, false );
		info.m_bRefract = pRefractTextureVar && (pRefractTextureVar->GetType() == MATERIAL_VAR_TYPE_TEXTURE);

		// Refractive water can be seen through
		if ( info.m_bRefract )
		{
			info.m_bOpaqueWater = false;
		}
	}

	info.m_bReflect = bLocalReflection;
	if ( info.m_bReflect )
	{
		if( r_waterforcereflectentities.GetBool() )
		{
			info.m_bReflectEntities = true;
		}
		else
		{
			IMaterialVar *pReflectEntitiesVar = pWaterMaterial->FindVar( "$reflectentities", NULL, false );
			info.m_bReflectEntities = pReflectEntitiesVar && (pReflectEntitiesVar->GetIntValue() != 0);
		}
	}

	info.m_bCheapWater = !info.m_bReflect && !info.m_bRefract;

	if( bDebugCheapWater )
	{
		Warning( "refract: %s reflect: %s\n", info.m_bRefract ? "true" : "false", info.m_bReflect ? "true" : "false" );
	}
}


//-----------------------------------------------------------------------------
// Draws the world and all entities
//-----------------------------------------------------------------------------
void CViewRender::WaterDrawWorldAndEntities( bool bDrawSkybox, const CViewSetup &view )
{
	CEngineCacheCriticalSection cacheCriticalSection( engineCache );

	m_pViewDrawSceneRenderTarget = materials->GetRenderTarget();

	VisibleFogVolumeInfo_t fogVolumeInfo;
 	render->ViewSetup3D( &view, m_Frustum );
	render->GetVisibleFogVolume( view.origin, &fogVolumeInfo );

	WaterRenderInfo_t info;
	DetermineWaterRenderInfo( fogVolumeInfo, info );
	
	if ( info.m_bCheapWater )
	{
		ViewDrawScene_NoWater( bDrawSkybox, view, fogVolumeInfo, info );
		materials->SetRenderTarget( m_pViewDrawSceneRenderTarget );
		return;
	}

	// Blat out the visible fog leaf if we're not going to use it
	if ( !r_ForceWaterLeaf.GetBool() )
	{
		fogVolumeInfo.m_nVisibleFogVolumeLeaf = -1;
	}

	// Force color clears for water texture 
	// It's needed to clear alpha channel of textures for HDR
	CViewSetup tmpView = view;
	tmpView.clearColor = true;

	// We can see water of some sort
	if ( !fogVolumeInfo.m_bEyeInFogVolume )
	{
		ViewDrawScene_EyeAboveWater( bDrawSkybox, tmpView, fogVolumeInfo, info );
	}
	else
	{
		ViewDrawScene_EyeUnderWater( bDrawSkybox, tmpView, fogVolumeInfo, info );
	}

	materials->SetRenderTarget( m_pViewDrawSceneRenderTarget );
}


void CViewRender::SetRenderTargetAndView( CViewSetup &view, float waterHeight, int flags )
{
	float spread = 2.0f;
//	float waterHeight = -64.0f; // test_water
//	float waterHeight = 712.0f; // water2
//	float waterHeight = -1152.0f; // test_water2
	float origWaterHeight = waterHeight;
	if( flags & DF_FUDGE_UP )
	{
		waterHeight += spread;
	}
	else
	{
		waterHeight -= spread;
	}
	if( flags & DF_RENDER_REFRACTION )
	{
		ITexture *pTexture = GetWaterRefractionTexture();
		//pTexture->IncrementReferenceCount();
		materials->SetFogZ( waterHeight );
		materials->SetHeightClipZ( waterHeight );
		Assert( pTexture );
		if( pTexture )
		{
			view.width = pTexture->GetActualWidth();
			view.height = pTexture->GetActualHeight();
			view.m_bUseRenderTargetAspectRatio = true;
			materials->SetRenderTarget( pTexture );
		}
	}
	else if( flags & DF_RENDER_REFLECTION )
	{
		ITexture *pTexture = GetWaterReflectionTexture();
		//pTexture->IncrementReferenceCount();
		Assert( pTexture );
		materials->SetFogZ( waterHeight );
		if( pTexture )
		{
			view.width = pTexture->GetActualWidth();
			view.height = pTexture->GetActualHeight();
			materials->SetRenderTarget( pTexture );
		}
		view.m_bUseRenderTargetAspectRatio = true;
		view.angles[0] = -view.angles[0];
		view.angles[2] = -view.angles[2];
		view.origin[2] -= 2.0f * ( view.origin[2] - (origWaterHeight));
		bool bSoftwareUserClipPlane = g_pMaterialSystemHardwareConfig->UseFastClipping();
		if( bSoftwareUserClipPlane && ( view.origin[2] > waterHeight - r_eyewaterepsilon.GetFloat() ) )
		{
			waterHeight = view.origin[2] + r_eyewaterepsilon.GetFloat();
		}
		materials->SetHeightClipZ( waterHeight );
	}
	else
	{
		materials->SetRenderTarget( m_pViewDrawSceneRenderTarget );
		if( flags & DF_CLIP_Z )
		{
			materials->SetHeightClipZ( waterHeight );
		}
	}
}

void CViewRender::WaterDrawHelper( 
	const CViewSetup &view, 
	ClientWorldListInfo_t &info, 
	CRenderList &renderList, 
	float waterHeight, 
	int flags,
	view_id_t viewID,
	float waterZAdjust,
	int iForceViewLeaf
	)
{
	int savedViewID = g_CurrentViewID;
	g_CurrentViewID = viewID;
	bool bClearDepth = ( flags & DF_CLEARDEPTH ) != 0;
	bool bClearColor = ( flags & DF_CLEARCOLOR ) != 0;

	CViewSetup tmpView;
	const CViewSetup *pView = &view;
	if( bClearColor || bClearDepth )
	{
		tmpView = view;
		tmpView.clearColor = bClearColor;
		tmpView.clearDepth = bClearDepth;
		SetRenderTargetAndView( tmpView, waterHeight, flags );
		render->ViewSetup3D( &tmpView, m_Frustum );
		pView = &tmpView;
	}

	if( flags & DF_BUILDWORLDLISTS )
	{
		bool bDrawEntities = ( flags & DF_DRAW_ENTITITES ) != 0;
		BuildWorldRenderLists( pView, info, bClearDepth, bDrawEntities, iForceViewLeaf );
	}
	
	// Make sure sound doesn't stutter
	engine->Sound_ExtraUpdate();

	if( ( flags & DF_CLIP_Z ) && mat_clipz.GetBool() )
	{
		if( flags & DF_CLIP_BELOW )
		{
			materials->SetHeightClipMode( MATERIAL_HEIGHTCLIPMODE_RENDER_ABOVE_HEIGHT );
		}
		else
		{
			materials->SetHeightClipMode( MATERIAL_HEIGHTCLIPMODE_RENDER_BELOW_HEIGHT );
		}
	}
	else
	{
		materials->SetHeightClipMode( MATERIAL_HEIGHTCLIPMODE_DISABLE );
	}

	if (!bClearDepth)
	{
		flags &= ~DF_DRAWSKYBOX;
	}

	// Update our render view flags.
	m_DrawFlags = flags | m_BaseDrawFlags;

	ITexture *pSaveFrameBufferCopyTexture = materials->GetFrameBufferCopyTexture( 0 );
	if ( engine->GetDXSupportLevel() >= 80 )
	{
		materials->SetFrameBufferCopyTexture( GetPowerOfTwoFrameBufferTexture() );
	}

	DrawWorld( info, renderList, m_DrawFlags, waterZAdjust );

	// FIXME: This may have to be static, not sure if we'll get a stack overflow
	ClientWorldListInfo_t tmpInfo;
	tmpInfo.m_pLeafList = (LeafIndex_t*)stackalloc( info.m_LeafCount * sizeof(LeafIndex_t) );
	tmpInfo.m_pLeafFogVolume = (LeafFogVolume_t*)stackalloc( info.m_LeafCount * sizeof(LeafFogVolume_t) );
	tmpInfo.m_pActualLeafIndex = (LeafIndex_t*)stackalloc( info.m_LeafCount * sizeof(LeafIndex_t) );
	ClientWorldListInfo_t *pInfo = ComputeActualWorldListInfo( info, m_DrawFlags, tmpInfo );

	if ( flags & DF_DRAW_ENTITITES )
	{
		BuildRenderableRenderLists( pView, *pInfo, renderList );
		DrawOpaqueRenderables( *pInfo, renderList );
		DrawTranslucentRenderables( *pInfo, renderList, m_DrawFlags, false );
	}
	else
	{
		// Draw translucent world brushes only, no entities
		DrawTranslucentWorldInLeaves( pInfo->m_LeafCount - 1, 0, *pInfo, m_DrawFlags );
	}
	// issue the pixel visibility tests
	if ( CurrentViewID() != VIEW_MAIN )
	{
		PixelVisibility_EndCurrentView();
	}

	materials->SetFrameBufferCopyTexture( pSaveFrameBufferCopyTexture );
	materials->SetHeightClipMode( MATERIAL_HEIGHTCLIPMODE_DISABLE );

	m_DrawFlags = m_BaseDrawFlags;
	g_CurrentViewID = savedViewID;
}


bool DoesViewPlaneIntersectWater( float waterZ, int leafWaterDataID )
{
	VMatrix viewMatrix, projectionMatrix, viewProjectionMatrix, inverseViewProjectionMatrix;
	materials->GetMatrix( MATERIAL_VIEW, &viewMatrix );
	materials->GetMatrix( MATERIAL_PROJECTION, &projectionMatrix );
	MatrixMultiply( projectionMatrix, viewMatrix, viewProjectionMatrix );
	MatrixInverseGeneral( viewProjectionMatrix, inverseViewProjectionMatrix );

	Vector mins, maxs;
	ClearBounds( mins, maxs );
	Vector testPoint[4];
	testPoint[0].Init( -1.0f, -1.0f, 0.0f );
	testPoint[1].Init( -1.0f, 1.0f, 0.0f );
	testPoint[2].Init( 1.0f, -1.0f, 0.0f );
	testPoint[3].Init( 1.0f, 1.0f, 0.0f );
	int i;
	bool bAbove = false;
	bool bBelow = false;
	float fudge = 7.0f;
	for( i = 0; i < 4; i++ )
	{
		Vector worldPos;
		Vector3DMultiplyPositionProjective( inverseViewProjectionMatrix, testPoint[i], worldPos );
		AddPointToBounds( worldPos, mins, maxs );
//		Warning( "viewplanez: %f waterZ: %f\n", worldPos.z, waterZ );
		if( worldPos.z + fudge > waterZ )
		{
			bAbove = true;
		}
		if( worldPos.z - fudge < waterZ )
		{
			bBelow = true;
		}
	}

	if( !( bAbove && bBelow ) )
	{
		// early out since the near plane doesn't cross the z plane of the water.
		return false;
	}

	Vector vecFudge( fudge, fudge, fudge );
	mins -= vecFudge;
	maxs += vecFudge;
	
	// the near plane does cross the z value for the visible water volume.  Call into
	// the engine to find out if the near plane intersects the water volume.
	return render->DoesBoxIntersectWaterVolume( mins, maxs, leafWaterDataID );
} 

static void CalcWaterEyeAdjustments( const CViewSetup &view, const VisibleFogVolumeInfo_t &fogInfo,
										  float &newWaterHeight, float &waterZAdjust, bool bSoftwareUserClipPlane )
{
	if( !bSoftwareUserClipPlane )
	{
		newWaterHeight = fogInfo.m_flWaterHeight;
		waterZAdjust = 0.0f;
		return;
	}
	
	newWaterHeight = fogInfo.m_flWaterHeight;
	float eyeToWaterZDelta = view.origin[2] - fogInfo.m_flWaterHeight;
	float epsilon = r_eyewaterepsilon.GetFloat();
	waterZAdjust = 0.0f;
	if( fabs( eyeToWaterZDelta ) < epsilon )
	{
		if( eyeToWaterZDelta > 0 )
		{
			newWaterHeight = view.origin[2] - epsilon;
		}
		else
		{
			newWaterHeight = view.origin[2] + epsilon;
		}
		waterZAdjust = newWaterHeight - fogInfo.m_flWaterHeight;
	}

//	Warning( "view.origin[2]: %f newWaterHeight: %f fogInfo.m_flWaterHeight: %f waterZAdjust: %f\n", 
//		( float )view.origin[2], newWaterHeight, fogInfo.m_flWaterHeight, waterZAdjust );
}

//-----------------------------------------------------------------------------
// Draws the scene when the view point is above the level of the water
//-----------------------------------------------------------------------------
void CViewRender::ViewDrawScene_EyeAboveWater( bool bDrawSkybox, const CViewSetup &view,
	const VisibleFogVolumeInfo_t &fogInfo, const WaterRenderInfo_t& waterInfo )
{
	VPROF( "CViewRender::ViewDrawScene_EyeAboveWater" );

	bool bSoftwareUserClipPlane = g_pMaterialSystemHardwareConfig->UseFastClipping();
	
	ClientWorldListInfo_t info;	
	CRenderList renderList;

	float newWaterHeight, waterZAdjust;
	CalcWaterEyeAdjustments( view, fogInfo, newWaterHeight, waterZAdjust, bSoftwareUserClipPlane );

	// eye is outside of water
	
	// render the reflection
	if( waterInfo.m_bReflect )
	{
		// NOTE: Clearing the color is unnecessary since we're drawing the skybox
		// and dest-alpha is never used in the reflection
		int nFlags = DF_RENDER_REFLECTION | DF_CLIP_Z | DF_CLIP_BELOW | 
			DF_RENDER_ABOVEWATER | DF_CLEARDEPTH | DF_BUILDWORLDLISTS;

		// NOTE: This will cause us to draw the 2d skybox in the reflection 
		// (which we want to do instead of drawing the 3d skybox)
		nFlags |= DF_DRAWSKYBOX;

		if( waterInfo.m_bReflectEntities )
		{
			nFlags |= DF_DRAW_ENTITITES;
		}

		// Disable occlusion visualization in reflection
		bool bVisOcclusion = r_visocclusion.GetInt();
		r_visocclusion.SetValue( 0 );

		EnableWorldFog();
		WaterDrawHelper( view, info, renderList, fogInfo.m_flWaterHeight, nFlags, VIEW_REFLECTION, 
			0.0f, fogInfo.m_nVisibleFogVolumeLeaf );
		
		r_visocclusion.SetValue( bVisOcclusion );

		materials->Flush();
	}
	
	unsigned char ucFogColor[3];

	int nFlags;
	bool bViewIntersectsWater = false;
	// render refraction
	int nFirstPassFlags = DF_BUILDWORLDLISTS | DF_CLEARCOLOR;
	if ( waterInfo.m_bRefract )
	{
		nFlags = nFirstPassFlags | DF_RENDER_REFRACTION | DF_CLIP_Z | 
			DF_RENDER_UNDERWATER | DF_FUDGE_UP | 
			DF_DRAW_ENTITITES | DF_MAINTAINWORLDLISTS | DF_CLEARDEPTH;
		nFirstPassFlags = 0;
		
		render->SetFogVolumeState( fogInfo.m_nVisibleFogVolume, true );
		
		materials->GetFogColor( ucFogColor );
		materials->ClearColor4ub( ucFogColor[0], ucFogColor[1], ucFogColor[2], 255 );

		WaterDrawHelper( view, info, renderList, newWaterHeight, nFlags, VIEW_REFRACTION, 
			waterZAdjust, -1 );
		if( !bSoftwareUserClipPlane )
		{
			bViewIntersectsWater = DoesViewPlaneIntersectWater( fogInfo.m_flWaterHeight, fogInfo.m_nVisibleFogVolume );
		}
		materials->ClearColor4ub( 0, 0, 0, 255 );
	}

	materials->Flush();
	
	

	EnableWorldFog();
	
	// render the world
	nFlags = nFirstPassFlags | DF_RENDER_ABOVEWATER | DF_CLEARDEPTH | DF_DRAW_ENTITITES | DF_MAINTAINWORLDLISTS;
	if( bViewIntersectsWater && !bSoftwareUserClipPlane )
	{
		// This is necessary to keep the non-water fogged world from drawing underwater in 
		// the case where we want to partially see into the water.
		nFlags |= DF_CLIP_Z | DF_CLIP_BELOW;
	}

	if ( bDrawSkybox )
	{
		nFlags |= DF_DRAWSKYBOX;
		nFlags &= ~DF_CLEARCOLOR;
	}
	if ( waterInfo.m_bDrawWaterSurface )
	{
		nFlags |= DF_RENDER_WATER;
	}
	if ( !waterInfo.m_bRefract && !waterInfo.m_bOpaqueWater )
	{
		nFlags |= DF_RENDER_UNDERWATER;
	}
	
	WaterDrawHelper( view, info, renderList, newWaterHeight, nFlags, VIEW_MAIN, waterZAdjust, -1 );

	if( waterZAdjust != 0.0f && bSoftwareUserClipPlane && waterInfo.m_bRefract )
	{
		nFlags = DF_RENDER_UNDERWATER;
		WaterDrawHelper( view, info, renderList, newWaterHeight, nFlags, VIEW_MAIN, waterZAdjust, -1 );
	}
	else if( bViewIntersectsWater )
	{
		materials->ClearColor4ub( ucFogColor[0], ucFogColor[1], ucFogColor[2], 255 );
		render->SetFogVolumeState( fogInfo.m_nVisibleFogVolume, true );
		nFlags = DF_RENDER_UNDERWATER | DF_CLIP_Z | DF_DRAW_ENTITITES;
		WaterDrawHelper( view, info, renderList, fogInfo.m_flWaterHeight, nFlags, VIEW_NONE, 0.0f, -1 );
	}
	materials->ClearColor4ub( 0, 0, 0, 255 );
}


//-----------------------------------------------------------------------------
// Draws the scene when the view point is under the level of the water
//-----------------------------------------------------------------------------
void CViewRender::ViewDrawScene_EyeUnderWater( bool bDrawSkybox, const CViewSetup &view, 
	const VisibleFogVolumeInfo_t &fogInfo, const WaterRenderInfo_t& waterInfo )
{
	// FIXME: The 3d skybox shouldn't be drawn when the eye is under water

	VPROF( "CViewRender::ViewDrawScene_EyeUnderWater" );

	bool bSoftwareUserClipPlane = g_pMaterialSystemHardwareConfig->UseFastClipping();

	float newWaterHeight, waterZAdjust;
	CalcWaterEyeAdjustments( view, fogInfo, newWaterHeight, waterZAdjust, bSoftwareUserClipPlane );

	ClientWorldListInfo_t info;	
	CRenderList renderList;

	int nFirstPassFlags = DF_BUILDWORLDLISTS | DF_CLEARCOLOR;

	render->SetFogVolumeState( fogInfo.m_nVisibleFogVolume, true );
	unsigned char ucFogColor[3];
	materials->GetFogColor( ucFogColor );
	materials->ClearColor4ub( ucFogColor[0], ucFogColor[1], ucFogColor[2], 255 );

	// render refraction (out of water)
	if ( waterInfo.m_bRefract )
	{
		int nFlags = nFirstPassFlags | DF_CLIP_Z | 
			DF_CLIP_BELOW | DF_RENDER_ABOVEWATER | 
			DF_DRAW_ENTITITES | DF_MAINTAINWORLDLISTS | DF_CLEARDEPTH;

		// NOTE: This will cause us to draw the 2d skybox in the refraction 
		// (which we want to do instead of drawing the 3d skybox)
		nFlags |= DF_DRAWSKYBOX | DF_CLIP_SKYBOX;
		
		nFirstPassFlags = 0;

		EnableWorldFog();
		WaterDrawHelper( view, info, renderList, newWaterHeight, nFlags, VIEW_REFRACTION, waterZAdjust, -1 );
		ITexture *pTexture = GetWaterRefractionTexture();
		materials->CopyRenderTargetToTexture( pTexture );
	}
	
	// NOTE: We're not drawing the 2d skybox under water since it's assumed to not be visible.

	// render the world underwater
	int nFlags = nFirstPassFlags | DF_MAINTAINWORLDLISTS | DF_FUDGE_UP | DF_RENDER_UNDERWATER | DF_CLEARDEPTH | DF_DRAW_ENTITITES;

	if( !bSoftwareUserClipPlane )
	{
		nFlags |= DF_CLIP_Z;
	}
	if ( waterInfo.m_bDrawWaterSurface )
	{
		nFlags |= DF_RENDER_WATER;
	}
	if ( !waterInfo.m_bRefract && !waterInfo.m_bOpaqueWater )
	{
		nFlags |= DF_RENDER_ABOVEWATER;
	}

	render->SetFogVolumeState( fogInfo.m_nVisibleFogVolume, false );

	WaterDrawHelper( view, info, renderList, newWaterHeight, nFlags, VIEW_MAIN, waterZAdjust, -1 );

	if( waterZAdjust != 0.0f && bSoftwareUserClipPlane && waterInfo.m_bRefract )
	{
		nFlags = DF_RENDER_ABOVEWATER;
		WaterDrawHelper( view, info, renderList, newWaterHeight, nFlags, VIEW_MAIN, waterZAdjust, -1 );
	}

	materials->ClearColor4ub( 0, 0, 0, 255 );
}


//-----------------------------------------------------------------------------
// Draws the scene when there's no water or cheap water
//-----------------------------------------------------------------------------
void CViewRender::ViewDrawScene_NoWater( bool bDrawSkybox, const CViewSetup &view, 
	const VisibleFogVolumeInfo_t &fogInfo, const WaterRenderInfo_t& waterInfo )
{
	VPROF( "CViewRender::ViewDrawScene_NoWater" );

	ClientWorldListInfo_t info;	
	CRenderList renderList;

	int nFlags = DF_CLEARDEPTH | DF_BUILDWORLDLISTS | DF_DRAW_ENTITITES;
	if ( !waterInfo.m_bOpaqueWater )
	{
		nFlags |= DF_RENDER_UNDERWATER | DF_RENDER_ABOVEWATER;
	}
	else
	{
		bool bViewIntersectsWater = DoesViewPlaneIntersectWater( fogInfo.m_flWaterHeight, fogInfo.m_nVisibleFogVolume );
		if( bViewIntersectsWater )
		{
			// have to draw both sides if we can see both.
			nFlags |= DF_RENDER_UNDERWATER | DF_RENDER_ABOVEWATER;
		}
		else if ( fogInfo.m_bEyeInFogVolume )
		{
			nFlags |= DF_RENDER_UNDERWATER;
		}
		else
		{
			nFlags |= DF_RENDER_ABOVEWATER;
		}
	}
	if ( waterInfo.m_bDrawWaterSurface )
	{
		nFlags |= DF_RENDER_WATER;
	}

	if ( !fogInfo.m_bEyeInFogVolume )
	{
		if ( bDrawSkybox )
		{
			nFlags |= DF_DRAWSKYBOX;
		}
		EnableWorldFog();
	}
	else
	{
		nFlags |= DF_CLEARCOLOR;

		render->SetFogVolumeState( fogInfo.m_nVisibleFogVolume, false );

		unsigned char ucFogColor[3];
		materials->GetFogColor( ucFogColor );
		materials->ClearColor4ub( ucFogColor[0], ucFogColor[1], ucFogColor[2], 255 );
	}

	WaterDrawHelper( view, info, renderList, 0.0f, nFlags, VIEW_MAIN, 0.0f, -1 );

	materials->ClearColor4ub( 0, 0, 0, 255 );
}


//-----------------------------------------------------------------------------
// Methods related to controlling the cheap water distance
//-----------------------------------------------------------------------------
void CViewRender::SetCheapWaterStartDistance( float flCheapWaterStartDistance )
{
	m_flCheapWaterStartDistance = flCheapWaterStartDistance;
}

void CViewRender::SetCheapWaterEndDistance( float flCheapWaterEndDistance )
{
	m_flCheapWaterEndDistance = flCheapWaterEndDistance;
}

void CViewRender::GetWaterLODParams( float &flCheapWaterStartDistance, float &flCheapWaterEndDistance )
{
	flCheapWaterStartDistance = m_flCheapWaterStartDistance;
	flCheapWaterEndDistance = m_flCheapWaterEndDistance;
}

static void CheapWaterStart_f( void )
{
	if( engine->Cmd_Argc() == 2 )
	{
		float dist = atof( engine->Cmd_Argv( 1 ) );
		view->SetCheapWaterStartDistance( dist );
	}
	else
	{
		float start, end;
		view->GetWaterLODParams( start, end );
		Warning( "r_cheapwaterstart: %f\n", start );
	}
}

static void CheapWaterEnd_f( void )
{
	if( engine->Cmd_Argc() == 2 )
	{
		float dist = atof( engine->Cmd_Argv( 1 ) );
		view->SetCheapWaterEndDistance( dist );
	}
	else
	{
		float start, end;
		view->GetWaterLODParams( start, end );
		Warning( "r_cheapwaterend: %f\n", end );
	}
}


//-----------------------------------------------------------------------------
// A console command allowing you to draw a material as an overlay
//-----------------------------------------------------------------------------
static void ScreenOverlay_f( void )
{
	if( engine->Cmd_Argc() == 2 )
	{
		if ( !Q_stricmp( "off", engine->Cmd_Argv(1) ) )
		{
			view->SetScreenOverlayMaterial( NULL );
		}
		else
		{
			IMaterial *pMaterial = materials->FindMaterial( engine->Cmd_Argv(1), TEXTURE_GROUP_OTHER, false );
			if ( !IsErrorMaterial( pMaterial ) )
			{
				view->SetScreenOverlayMaterial( pMaterial );
			}
			else
			{
				view->SetScreenOverlayMaterial( NULL );
			}
		}
	}
	else
	{
		IMaterial *pMaterial = view->GetScreenOverlayMaterial();
		Warning( "r_screenoverlay: %s\n", pMaterial ? pMaterial->GetName() : "off" );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &view - 
//			&introData - 
//-----------------------------------------------------------------------------
void CViewRender::ViewDrawScene_Intro( const CViewSetup &view, const IntroData_t &introData )
{
	VPROF( "CViewRender::ViewDrawScene" );

	// -----------------------------------------------------------------------
	// Set the clear color to black since we are going to be adding up things
	// in the frame buffer.
	// -----------------------------------------------------------------------
	materials->ClearColor4ub( 0, 0, 0, 255 );
	
	// -----------------------------------------------------------------------
	// Draw the primary scene and copy it to the first framebuffer texture
	// -----------------------------------------------------------------------	
	CViewSetup playerView = view;
	playerView.origin = introData.m_vecCameraView;
	playerView.m_vUnreflectedOrigin = introData.m_vecCameraView;
	playerView.angles = introData.m_vecCameraViewAngles;
	if ( introData.m_playerViewFOV )
	{
		playerView.fov = ScaleFOVByWidthRatio( introData.m_playerViewFOV, engine->GetScreenAspectRatio() / ( 4.0f / 3.0f ) );
	}
	SetupCurrentView( playerView.origin, playerView.angles, VIEW_INTRO_PLAYER );

	// Invoke pre-render methods
	IGameSystem::PreRenderAllSystems();

	// Start view, clear frame/z buffer if necessary
	playerView.clearColor = true;
	playerView.clearDepth = true;
	SetupVis( playerView );

	if( introData.m_bDrawSecondary )
	{
		// NOTE: We only increment this once since time doesn't move forward.
		g_ParticleMgr.IncrementFrameCode();
	}

	if( introData.m_bDrawPrimary )
	{
		DrawWorldAndEntities( true /* drawSkybox */, playerView );
	}
	else
	{
		materials->ClearBuffers( true, true );
	}
	UpdateScreenEffectTexture( 0 );
	
	// -----------------------------------------------------------------------
	// Draw the secondary scene and copy it to the second framebuffer texture
	// -----------------------------------------------------------------------
	CViewSetup cameraView = view;
	SetupCurrentView( cameraView.origin, cameraView.angles, VIEW_INTRO_CAMERA );

	// Invoke pre-render methods
	IGameSystem::PreRenderAllSystems();

	// Start view, clear frame/z buffer if necessary
	cameraView.clearColor = true;
	cameraView.clearDepth = true;
	SetupVis( cameraView );

	if( introData.m_bDrawSecondary )
	{
		DrawWorldAndEntities( true /* drawSkybox */, cameraView );
	}
	else
	{
		materials->ClearBuffers( true, true );
	}
	UpdateScreenEffectTexture( 1 );

	// -----------------------------------------------------------------------
	// Draw quads on the screen for each screenspace pass.
	// -----------------------------------------------------------------------
	// Find the material that we use to render the overlays
	IMaterial *pOverlayMaterial = materials->FindMaterial( "scripted/intro_screenspaceeffect", TEXTURE_GROUP_OTHER );
	IMaterialVar *pModeVar = pOverlayMaterial->FindVar( "$mode", NULL );
	IMaterialVar *pAlphaVar = pOverlayMaterial->FindVar( "$alpha", NULL );

	materials->ClearBuffers( true, true );
	
	materials->MatrixMode( MATERIAL_VIEW );
	materials->PushMatrix();
	materials->LoadIdentity();

	materials->MatrixMode( MATERIAL_PROJECTION );
	materials->PushMatrix();
	materials->LoadIdentity();
	
	int passID;
	for( passID = 0; passID < introData.m_Passes.Count(); passID++ )
	{
		const IntroDataBlendPass_t& pass = introData.m_Passes[passID];
		if ( pass.m_Alpha == 0 )
			continue;

		// Pick one of the blend modes for the material.
		if( pass.m_BlendMode >= 0 && pass.m_BlendMode < 9  )
		{
			pModeVar->SetIntValue( pass.m_BlendMode );
		}
		else
		{
			Assert(0);
		}
		// Set the alpha value for the material.
		pAlphaVar->SetFloatValue( pass.m_Alpha );
		
		// Draw a quad for this pass.
		materials->DrawScreenSpaceQuad( pOverlayMaterial );
	}
	
	materials->MatrixMode( MATERIAL_VIEW );
	materials->PopMatrix();
	
	materials->MatrixMode( MATERIAL_PROJECTION );
	materials->PopMatrix();
	
	// Draw the starfield
	// FIXME

	// blur?
	
	// Disable fog for the rest of the stuff
	DisableFog();
	
	// Here are the overlays...
	CGlowOverlay::DrawOverlays();
	// issue the pixel visibility tests
	PixelVisibility_EndCurrentView();

	// And here are the screen-space effects
	PerformScreenSpaceEffects();

	// Make sure sound doesn't stutter
	engine->Sound_ExtraUpdate();

	// Debugging info goes over the top
	Draw3DDebuggingInfo( view );

	// Let the particle manager simulate things that haven't been simulated.
	g_ParticleMgr.PostRender();

	FinishCurrentView();
}

static ConCommand r_cheapwaterstart( "r_cheapwaterstart", CheapWaterStart_f );
static ConCommand r_cheapwaterend( "r_cheapwaterend", CheapWaterEnd_f );
static ConCommand r_screenspacematerial( "r_screenoverlay", ScreenOverlay_f );


