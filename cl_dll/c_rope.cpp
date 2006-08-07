//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_rope.h"
#include "beamdraw.h"
#include "view.h"
#include "env_wind_shared.h"
#include "input.h"
#include "rope_helpers.h"
#include "engine/ivmodelinfo.h"
#include "tier0/vprof.h"
#include "c_te_effect_dispatch.h"
#include "collisionutils.h"
#include <KeyValues.h>
#include <bitbuf.h>
#include "utlfixedlinkedlist.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void RecvProxy_RecomputeSprings( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	// Have the regular proxy store the data.
	RecvProxy_Int32ToInt32( pData, pStruct, pOut );

	C_RopeKeyframe *pRope = (C_RopeKeyframe*)pStruct;
	pRope->RecomputeSprings();
}


IMPLEMENT_CLIENTCLASS_DT_NOBASE( C_RopeKeyframe, DT_RopeKeyframe, CRopeKeyframe )
	RecvPropInt( RECVINFO(m_iRopeMaterialModel) ),
	RecvPropEHandle( RECVINFO(m_hStartPoint) ),
	RecvPropEHandle( RECVINFO(m_hEndPoint) ),
	RecvPropInt( RECVINFO(m_iStartAttachment) ),
	RecvPropInt( RECVINFO(m_iEndAttachment) ),

	RecvPropInt( RECVINFO(m_fLockedPoints) ),
	RecvPropInt( RECVINFO(m_Slack), 0, RecvProxy_RecomputeSprings ),
	RecvPropInt( RECVINFO(m_RopeLength), 0, RecvProxy_RecomputeSprings ),
	RecvPropInt( RECVINFO(m_RopeFlags) ),
	RecvPropFloat( RECVINFO(m_TextureScale) ),
	RecvPropInt( RECVINFO(m_nSegments) ),
	RecvPropInt( RECVINFO(m_Subdiv) ),

	RecvPropFloat( RECVINFO(m_Width) ),
	RecvPropFloat( RECVINFO(m_flScrollSpeed) ),
	RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),
	RecvPropInt( RECVINFO_NAME(m_hNetworkMoveParent, moveparent), 0, RecvProxy_IntToMoveParent ),
	
	RecvPropInt( RECVINFO( m_iParentAttachment ) ),
END_RECV_TABLE()

#define ROPE_IMPULSE_SCALE	20
#define ROPE_IMPULSE_DECAY	0.95

static ConVar rope_shake( "rope_shake", "0" );
static ConVar rope_subdiv( "rope_subdiv", "2", 0, "Rope subdivision amount", true, 0, true, MAX_ROPE_SUBDIVS );
static ConVar rope_collide( "rope_collide", "1", 0, "Collide rope with the world" );

static ConVar rope_smooth( "rope_smooth", "1", 0, "Do an antialiasing effect on ropes" );
static ConVar rope_smooth_enlarge( "rope_smooth_enlarge", "1.4", 0, "How much to enlarge ropes in screen space for antialiasing effect" );

static ConVar rope_smooth_minwidth( "rope_smooth_minwidth", "0.3", 0, "When using smoothing, this is the min screenspace width it lets a rope shrink to" );
static ConVar rope_smooth_minalpha( "rope_smooth_minalpha", "0.2", 0, "Alpha for rope antialiasing effect" );

static ConVar rope_smooth_maxalphawidth( "rope_smooth_maxalphawidth", "1.75" );
static ConVar rope_smooth_maxalpha( "rope_smooth_maxalpha", "0.5", 0, "Alpha for rope antialiasing effect" );

static ConVar mat_fullbright( "mat_fullbright", "0", FCVAR_CHEAT ); // get it from the engine
static ConVar r_drawropes( "r_drawropes", "1", FCVAR_CHEAT );
static ConVar r_ropetranslucent( "r_ropetranslucent", "1");

static ConVar rope_wind_dist( "rope_wind_dist", "1000", 0, "Don't use CPU applying small wind gusts to ropes when they're past this distance." );
static ConVar rope_averagelight( "rope_averagelight", "1", 0, "Makes ropes use average of cubemap lighting instead of max intensity." );


static ConVar rope_rendersolid( "rope_rendersolid", "1" );

static ConVar rope_solid_minwidth( "rope_solid_minwidth", "0.6" );
static ConVar rope_solid_maxwidth( "rope_solid_maxwidth", "1" );

static ConVar rope_solid_minalpha( "rope_solid_minalpha", "0.0" );
static ConVar rope_solid_maxalpha( "rope_solid_maxalpha", "1" );


static CCycleCount	g_RopeCollideTicks;
static CCycleCount	g_RopeDrawTicks;
static CCycleCount	g_RopeSimulateTicks;
static int			g_nRopePointsSimulated;

// Active ropes.
CUtlLinkedList<C_RopeKeyframe*, int> g_Ropes;


static Vector	g_FullBright_LightValues[ROPE_MAX_SEGMENTS];
class CFullBrightLightValuesInit
{
public:
	CFullBrightLightValuesInit()
	{
		for( int i=0; i < ROPE_MAX_SEGMENTS; i++ )
			g_FullBright_LightValues[i].Init( 1, 1, 1 );
	}
} g_FullBrightLightValuesInit;

// Precalculated info for rope subdivision.
static float	g_RopeSubdivs[MAX_ROPE_SUBDIVS][MAX_ROPE_SUBDIVS];
class CSubdivInit
{
public:
	CSubdivInit()
	{
		for ( int iSubdiv=0; iSubdiv < MAX_ROPE_SUBDIVS; iSubdiv++ )
		{
			for( int i=0; i < iSubdiv; i++ )
				g_RopeSubdivs[iSubdiv][i] = (float)(i+1) / (iSubdiv+1);
		}
	}
} g_SubdivInit;

//interesting barbed-wire-looking effect
static int		g_nBarbedSubdivs = 3;
static float	g_BarbedSubdivs[MAX_ROPE_SUBDIVS] = {1.5, -0.5, 0.5};

// This can be exposed through the entity if we ever care.
static float g_flLockAmount = 0.1;
static float g_flLockFalloff = 0.3;

//=============================================================================
//
// Rope mananger.
//
struct RopeSegData_t
{
	int			m_nSegmentCount;
	CBeamSeg	m_Segments[MAX_ROPE_SEGMENTS];
	float		m_BackWidths[MAX_ROPE_SEGMENTS];

	// If this is less than rope_solid_minwidth and rope_solid_minalpha is 0, then we can avoid drawing..
	float		m_flMaxBackWidth;
};

class CRopeManager : public IRopeManager
{
public:

	CRopeManager();
	~CRopeManager();

	void ResetRenderCache( void );
	void AddToRenderCache( C_RopeKeyframe *pRope );
	void DrawRenderCache( void );

	void			ResetSegmentCache( int nMaxSegments );
	RopeSegData_t	*GetNextSegmentFromCache( void );

	enum { MAX_ROPE_RENDERCACHE	= 128 };
	
private:

	void RenderNonSolidRopes( IMaterial *pMaterial, int nVertCount, int nIndexCount );
	void RenderSolidRopes( IMaterial *pMaterial, int nVertCount, int nIndexCount, bool bRenderNonSolid );

private:

	struct RopeRenderData_t
	{
		IMaterial			*m_pSolidMaterial;
		IMaterial			*m_pBackMaterial;
		int					m_nCacheCount;
		C_RopeKeyframe		*m_aCache[MAX_ROPE_RENDERCACHE];
	};

	CUtlVector<RopeRenderData_t>	m_aRenderCache;
	int								m_nSegmentCacheCount;
	CUtlVector<RopeSegData_t>		m_aSegmentCache;
};

static CRopeManager s_RopeManager;

IRopeManager *RopeManager()
{
	return &s_RopeManager;
}


inline bool ShouldUseFakeAA( IMaterial *pBackMaterial )
{
	return pBackMaterial && rope_smooth.GetInt() && engine->GetDXSupportLevel() > 70 && !g_pMaterialSystemHardwareConfig->IsAAEnabled();
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CRopeManager::CRopeManager()
{
	m_aRenderCache.Purge();
	m_aSegmentCache.Purge();
	m_nSegmentCacheCount = 0;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CRopeManager::~CRopeManager()
{
	m_aRenderCache.Purge();
	m_aSegmentCache.Purge();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRopeManager::ResetRenderCache( void )
{
	int nRenderCacheCount = m_aRenderCache.Count();
	for ( int iRenderCache = 0; iRenderCache < nRenderCacheCount; ++iRenderCache )
	{
		m_aRenderCache[iRenderCache].m_nCacheCount = 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRopeManager::AddToRenderCache( C_RopeKeyframe *pRope )
{
	if( !pRope->GetSolidMaterial() )
	{
		return;
	}
	
	// Find the current rope list.
	int iRenderCache = 0;
	int nRenderCacheCount = m_aRenderCache.Count();
	for ( ; iRenderCache < nRenderCacheCount; ++iRenderCache )
	{
		if ( ( pRope->GetSolidMaterial() == m_aRenderCache[iRenderCache].m_pSolidMaterial ) &&
			 ( pRope->GetBackMaterial() == m_aRenderCache[iRenderCache].m_pBackMaterial ) )
			break;
	}

	// A full rope list should have been generate in CreateRenderCache
	// If we didn't find one, then allocate the mofo.
	if ( iRenderCache == nRenderCacheCount )
	{
		int iRenderCache = m_aRenderCache.AddToTail();
		m_aRenderCache[iRenderCache].m_pSolidMaterial = pRope->GetSolidMaterial();
		m_aRenderCache[iRenderCache].m_pBackMaterial = pRope->GetBackMaterial();
		m_aRenderCache[iRenderCache].m_nCacheCount = 0;
	}

	if ( m_aRenderCache[iRenderCache].m_nCacheCount >= MAX_ROPE_RENDERCACHE )
	{
		Warning( "CRopeManager::AddToRenderCache count to large for cache!\n" );
		return;
	}

	m_aRenderCache[iRenderCache].m_aCache[m_aRenderCache[iRenderCache].m_nCacheCount] = pRope;
	++m_aRenderCache[iRenderCache].m_nCacheCount;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRopeManager::DrawRenderCache( void )
{
	VPROF_BUDGET( "CRopeManager::DrawRenderCache", VPROF_BUDGETGROUP_ROPES );

	// Check to see if we want to render the ropes.
	if( !r_drawropes.GetBool() )
		return;

	int nRenderCacheCount = m_aRenderCache.Count();
	for ( int iRenderCache = 0; iRenderCache < nRenderCacheCount; ++iRenderCache )
	{
		if ( m_aRenderCache[iRenderCache].m_nCacheCount == 0 )
			continue;

		int nCacheCount = m_aRenderCache[iRenderCache].m_nCacheCount;

		ResetSegmentCache( nCacheCount );

		for ( int iCache = 0; iCache < nCacheCount; ++iCache )
		{
			C_RopeKeyframe *pRope = m_aRenderCache[iRenderCache].m_aCache[iCache];
			if ( pRope )
			{
				RopeSegData_t *pRopeSegment = GetNextSegmentFromCache();
				pRope->BuildRope( pRopeSegment );
			}
		}

		int nVertCount = 0;
		int nIndexCount = 0;
		for ( int iSegmentCache = 0; iSegmentCache < m_nSegmentCacheCount; ++iSegmentCache )
		{
			nVertCount += ( m_aSegmentCache[iSegmentCache].m_nSegmentCount * 2 );
			nIndexCount += ( ( m_aSegmentCache[iSegmentCache].m_nSegmentCount - 1 )  * 6 );
		}

		// Render the non-solid portion of the ropes.
		bool bRenderNonSolid = ShouldUseFakeAA( m_aRenderCache[iRenderCache].m_pBackMaterial );
		if ( bRenderNonSolid )
		{
			RenderNonSolidRopes( m_aRenderCache[iRenderCache].m_pBackMaterial, nVertCount, nIndexCount );
		}

		// Render the solid portion of the ropes.
		if ( rope_rendersolid.GetInt() )
		{
			RenderSolidRopes( m_aRenderCache[iRenderCache].m_pSolidMaterial, nVertCount, nIndexCount, bRenderNonSolid );
		}
	}
	ResetSegmentCache( 0 );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRopeManager::RenderNonSolidRopes( IMaterial *pMaterial, int nVertCount, int nIndexCount )
{
	// Render the solid portion of the ropes.
	CMeshBuilder meshBuilder;
	IMesh *pMesh = materials->GetDynamicMesh( true, NULL, NULL, pMaterial );
	meshBuilder.Begin( pMesh, MATERIAL_TRIANGLES, nVertCount, nIndexCount );

	CBeamSegDraw beamSegment;

	int nVerts = 0;
	for ( int iSegmentCache = 0; iSegmentCache < m_nSegmentCacheCount; ++iSegmentCache )
	{
		int nSegmentCount = m_aSegmentCache[iSegmentCache].m_nSegmentCount;
		beamSegment.Start( nSegmentCount, pMaterial, &meshBuilder, nVerts );
		for ( int iSegment = 0; iSegment < nSegmentCount; ++iSegment )
		{
			beamSegment.NextSeg( &m_aSegmentCache[iSegmentCache].m_Segments[iSegment] );
		}
		beamSegment.End();
		nVerts += ( m_aSegmentCache[iSegmentCache].m_nSegmentCount * 2 );
	}
	
	meshBuilder.End();
	pMesh->Draw();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRopeManager::RenderSolidRopes( IMaterial *pMaterial, int nVertCount, int nIndexCount, bool bRenderNonSolid )
{
	// Render the solid portion of the ropes.
	CMeshBuilder meshBuilder;
	IMesh *pMesh = materials->GetDynamicMesh( true, NULL, NULL, pMaterial );
	meshBuilder.Begin( pMesh, MATERIAL_TRIANGLES, nVertCount, nIndexCount );

	CBeamSegDraw beamSegment;

	if ( bRenderNonSolid )
	{
		int nVerts = 0;
		for ( int iSegmentCache = 0; iSegmentCache < m_nSegmentCacheCount; ++iSegmentCache )
		{
			RopeSegData_t *pSegData = &m_aSegmentCache[iSegmentCache];
			
			// If it's all going to be 0 alpha, then just skip drawing this one.
			if ( rope_solid_minalpha.GetFloat() == 0.0 && pSegData->m_flMaxBackWidth <= rope_solid_minwidth.GetFloat() )
				continue;

			int nSegmentCount = m_aSegmentCache[iSegmentCache].m_nSegmentCount;
			beamSegment.Start( nSegmentCount, pMaterial, &meshBuilder, nVerts );
			for ( int iSegment = 0; iSegment < nSegmentCount; ++iSegment )
			{
				CBeamSeg *pSeg = &m_aSegmentCache[iSegmentCache].m_Segments[iSegment];
				pSeg->m_flWidth = m_aSegmentCache[iSegmentCache].m_BackWidths[iSegment];

				// To avoid aliasing, the "solid" version of the rope on xbox is just "more solid",
				// and it has its own values controlling its alpha.
				pSeg->m_flAlpha = RemapVal( pSeg->m_flWidth, 
					rope_solid_minwidth.GetFloat(),
					rope_solid_maxwidth.GetFloat(),
					rope_solid_minalpha.GetFloat(),
					rope_solid_maxalpha.GetFloat() );
				
				pSeg->m_flAlpha = clamp( pSeg->m_flAlpha, 0.0f, 1.0f );
				
				beamSegment.NextSeg( &m_aSegmentCache[iSegmentCache].m_Segments[iSegment] );
			}
			beamSegment.End();
			nVerts += ( m_aSegmentCache[iSegmentCache].m_nSegmentCount * 2 );
		}
	}
	else
	{
		int nVerts = 0;
		for ( int iSegmentCache = 0; iSegmentCache < m_nSegmentCacheCount; ++iSegmentCache )
		{
			int nSegmentCount = m_aSegmentCache[iSegmentCache].m_nSegmentCount;
			beamSegment.Start( nSegmentCount, pMaterial, &meshBuilder, nVerts );
			for ( int iSegment = 0; iSegment < nSegmentCount; ++iSegment )
			{
				beamSegment.NextSeg( &m_aSegmentCache[iSegmentCache].m_Segments[iSegment] );
			}
			beamSegment.End();
			nVerts += ( m_aSegmentCache[iSegmentCache].m_nSegmentCount * 2 );
		}
	}

	meshBuilder.End();
	pMesh->Draw();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CRopeManager::ResetSegmentCache( int nMaxSegments )
{
	MEM_ALLOC_CREDIT();
	m_nSegmentCacheCount = 0;
	if ( nMaxSegments )
		m_aSegmentCache.EnsureCount( nMaxSegments );
	else
		m_aSegmentCache.Purge();

}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
RopeSegData_t *CRopeManager::GetNextSegmentFromCache( void )
{
	if ( m_nSegmentCacheCount >= m_aSegmentCache.Count() )
	{
		Warning( "CRopeManager::GetNextSegmentFromCache too many segments for cache!\n" );
		return NULL;
	}

	++m_nSegmentCacheCount;
	return &m_aSegmentCache[m_nSegmentCacheCount-1];
}

//=============================================================================

// ------------------------------------------------------------------------------------ //
// Global functions.
// ------------------------------------------------------------------------------------ //

void Rope_ResetCounters()
{
	g_RopeCollideTicks.Init();
	g_RopeDrawTicks.Init();
	g_RopeSimulateTicks.Init();
	g_nRopePointsSimulated = 0;
}


// ------------------------------------------------------------------------------------ //
// This handles the rope shake command.
// ------------------------------------------------------------------------------------ //

void ShakeRopesCallback( const CEffectData &data )
{
	Vector vCenter = data.m_vOrigin;
	float flRadius = data.m_flRadius;
	float flMagnitude = data.m_flMagnitude;

	// Now find any nearby ropes and shake them.
	FOR_EACH_LL( g_Ropes, i )
	{
		C_RopeKeyframe *pRope = g_Ropes[i];
	
		pRope->ShakeRope( vCenter, flRadius, flMagnitude );
	}
}

DECLARE_CLIENT_EFFECT( "ShakeRopes", ShakeRopesCallback );




// ------------------------------------------------------------------------------------ //
// C_RopeKeyframe::CPhysicsDelegate
// ------------------------------------------------------------------------------------ //
#define WIND_FORCE_FACTOR 10

void C_RopeKeyframe::CPhysicsDelegate::GetNodeForces( CSimplePhysics::CNode *pNodes, int iNode, Vector *pAccel )
{
	// Gravity.
	if ( !( m_pKeyframe->GetRopeFlags() & ROPE_NO_GRAVITY ) )
	{
		pAccel->Init( ROPE_GRAVITY );
	}

	if( !m_pKeyframe->m_LinksTouchingSomething[iNode] && m_pKeyframe->m_bApplyWind)
	{
		Vector vecWindVel;
		GetWindspeedAtTime(gpGlobals->curtime, vecWindVel);
		if ( vecWindVel.LengthSqr() > 0 )
		{
			Vector vecWindAccel;
			VectorMA( *pAccel, WIND_FORCE_FACTOR, vecWindVel, *pAccel );
		}
		else
		{
			if (m_pKeyframe->m_flCurrentGustTimer < m_pKeyframe->m_flCurrentGustLifetime )
			{
				float div = m_pKeyframe->m_flCurrentGustTimer / m_pKeyframe->m_flCurrentGustLifetime;
				float scale = 1 - cos( div * M_PI );

				*pAccel += m_pKeyframe->m_vWindDir * scale;
			}
		}
	}

	// HACK.. shake the rope around.
	static float scale=15000;
	if( rope_shake.GetInt() )
	{
		*pAccel += RandomVector( -scale, scale );
	}

	// Apply any instananeous forces and reset
	*pAccel += ROPE_IMPULSE_SCALE * m_pKeyframe->m_flImpulse;
	m_pKeyframe->m_flImpulse *= ROPE_IMPULSE_DECAY;
}


void LockNodeDirection( 
	CSimplePhysics::CNode *pNodes, 
	int parity, 
	int nFalloffNodes,
	float flLockAmount,
	float flLockFalloff,
	const Vector &vIdealDir ) 
{
	for ( int i=0; i < nFalloffNodes; i++ )
	{
		Vector &v0 = pNodes[i*parity].m_vPos;
		Vector &v1 = pNodes[(i+1)*parity].m_vPos;

		Vector vDir = v1 - v0;
		float len = vDir.Length();
		if ( len > 0.0001f )
		{
			vDir /= len;

			Vector vActual;
			VectorLerp( vDir, vIdealDir, flLockAmount, vActual );
			v1 = v0 + vActual * len;

			flLockAmount *= flLockFalloff;
		}
	}
}


void C_RopeKeyframe::CPhysicsDelegate::ApplyConstraints( CSimplePhysics::CNode *pNodes, int nNodes )
{
	VPROF( "CPhysicsDelegate::ApplyConstraints" );

	CTraceFilterWorldOnly traceFilter;

	// Collide with the world.
	if( ((m_pKeyframe->m_RopeFlags & ROPE_COLLIDE) && 
		rope_collide.GetInt()) || 
		(rope_collide.GetInt() == 2) )
	{
		CTimeAdder adder( &g_RopeCollideTicks );

		for( int i=0; i < nNodes; i++ )
		{
			CSimplePhysics::CNode *pNode = &pNodes[i];

			int iIteration;
			int nIterations = 10;
			for( iIteration=0; iIteration < nIterations; iIteration++ )
			{
				trace_t trace;
				UTIL_TraceHull( pNode->m_vPrevPos, pNode->m_vPos, 
					Vector(-2,-2,-2), Vector(2,2,2), MASK_SOLID_BRUSHONLY, &traceFilter, &trace );

				if( trace.fraction == 1 )
					break;

				if( trace.fraction == 0 || trace.allsolid || trace.startsolid )
				{
					m_pKeyframe->m_LinksTouchingSomething[i] = true;
					pNode->m_vPos = pNode->m_vPrevPos;
					break;
				}

				// Apply some friction.
				static float flSlowFactor = 0.3f;
				pNode->m_vPos -= (pNode->m_vPos - pNode->m_vPrevPos) * flSlowFactor;

				// Move it out along the face normal.
				float distBehind = trace.plane.normal.Dot( pNode->m_vPos ) - trace.plane.dist;
				pNode->m_vPos += trace.plane.normal * (-distBehind + 2.2);
				m_pKeyframe->m_LinksTouchingSomething[i] = true;
			}

			if( iIteration == nIterations )
				pNodes[i].m_vPos = pNodes[i].m_vPrevPos;
		}
	}

	// Lock the endpoints.
	QAngle angles;
	if( m_pKeyframe->m_fLockedPoints & ROPE_LOCK_START_POINT )
	{
		m_pKeyframe->GetEndPointAttachment( 0, pNodes[0].m_vPos, angles );
		if (( m_pKeyframe->m_fLockedPoints & ROPE_LOCK_START_DIRECTION ) && (nNodes > 3))
		{
			Vector forward;
			AngleVectors( angles, &forward );

			int parity = 1;
			int nFalloffNodes = min( 2, nNodes - 2 );
			LockNodeDirection( pNodes, parity, nFalloffNodes, g_flLockAmount, g_flLockFalloff, forward );
		}
	}

	if( m_pKeyframe->m_fLockedPoints & ROPE_LOCK_END_POINT )
	{
		m_pKeyframe->GetEndPointAttachment( 1, pNodes[nNodes-1].m_vPos, angles );
		if( m_pKeyframe->m_fLockedPoints & ROPE_LOCK_END_DIRECTION && (nNodes > 3))
		{
			Vector forward;
			AngleVectors( angles, &forward );
			
			int parity = -1;
			int nFalloffNodes = min( 2, nNodes - 2 );
			LockNodeDirection( &pNodes[nNodes-1], parity, nFalloffNodes, g_flLockAmount, g_flLockFalloff, forward );
		}
	}
}


// ------------------------------------------------------------------------------------ //
// C_RopeKeyframe
// ------------------------------------------------------------------------------------ //

C_RopeKeyframe::C_RopeKeyframe()
{
	m_bEndPointAttachmentsDirty = true;
	m_PhysicsDelegate.m_pKeyframe = this;
	m_pMaterial = NULL;
	m_bPhysicsInitted = false;
	m_RopeFlags = 0;
	m_TextureHeight = 1;
	m_hStartPoint = m_hEndPoint = NULL;
	m_iStartAttachment = m_iEndAttachment = 0;
	m_vColorMod.Init( 1, 1, 1 );
	m_nLinksTouchingSomething = 0;
	m_Subdiv = 255; // default to using the cvar
	
	m_fLockedPoints = 0;
	m_fPrevLockedPoints = 0;
	
	m_iForcePointMoveCounter = 0;
	m_flCurScroll = m_flScrollSpeed = 0;
	m_TextureScale = 4;	// 4:1
	m_flImpulse.Init();

	g_Ropes.AddToTail( this );
}


C_RopeKeyframe::~C_RopeKeyframe()
{
	g_Ropes.FindAndRemove( this );
}


C_RopeKeyframe* C_RopeKeyframe::Create(
	C_BaseEntity *pStartEnt,
	C_BaseEntity *pEndEnt,
	int iStartAttachment,
	int iEndAttachment,
	float ropeWidth,
	const char *pMaterialName,
	int numSegments,
	int ropeFlags
	)
{
	C_RopeKeyframe *pRope = new C_RopeKeyframe;

	pRope->InitializeAsClientEntity( NULL, RENDER_GROUP_OPAQUE_ENTITY );
	
	if ( pStartEnt )
	{
		pRope->m_hStartPoint = pStartEnt;
		pRope->m_fLockedPoints |= ROPE_LOCK_START_POINT;
	}

	if ( pEndEnt )
	{
		pRope->m_hEndPoint = pEndEnt;
		pRope->m_fLockedPoints |= ROPE_LOCK_END_POINT;
	}

	pRope->m_iStartAttachment = iStartAttachment;
	pRope->m_iEndAttachment = iEndAttachment;
	pRope->m_Width = ropeWidth;
	pRope->m_nSegments = clamp( numSegments, 2, ROPE_MAX_SEGMENTS );
	pRope->m_RopeFlags = ropeFlags;

	pRope->FinishInit( pMaterialName );
	return pRope;
}


C_RopeKeyframe* C_RopeKeyframe::CreateFromKeyValues( C_BaseAnimating *pEnt, KeyValues *pValues )
{
	C_RopeKeyframe *pRope = C_RopeKeyframe::Create( 
		pEnt,
		pEnt,
		pEnt->LookupAttachment( pValues->GetString( "StartAttachment" ) ),
		pEnt->LookupAttachment( pValues->GetString( "EndAttachment" ) ),
		pValues->GetFloat( "Width", 0.5 ),
		pValues->GetString( "Material" ),
		pValues->GetInt( "NumSegments" ),
		0 );

	if ( pRope )
	{
		if ( pValues->GetInt( "Gravity", 1 ) == 0 )
		{
			pRope->m_RopeFlags |= ROPE_NO_GRAVITY;
		}

		pRope->m_RopeLength = pValues->GetInt( "Length" );
		pRope->m_TextureScale = pValues->GetFloat( "TextureScale", pRope->m_TextureScale );
		pRope->m_Slack = 0;
		pRope->m_RopeFlags |= ROPE_SIMULATE;
	}

	return pRope;
}


int C_RopeKeyframe::GetRopesIntersectingAABB( C_RopeKeyframe **pRopes, int nMaxRopes, const Vector &vAbsMin, const Vector &vAbsMax )
{
	if ( nMaxRopes == 0 )
		return 0;
	
	int nRopes = 0;
	FOR_EACH_LL( g_Ropes, i )
	{
		C_RopeKeyframe *pRope = g_Ropes[i];
	
		Vector v1, v2;
		if ( pRope->GetEndPointPos( 0, v1 ) && pRope->GetEndPointPos( 1, v2 ) )
		{
			if ( IsBoxIntersectingRay( v1, v2-v1, vAbsMin, vAbsMax, 0.1f ) )
			{
				pRopes[nRopes++] = pRope;
				if ( nRopes == nMaxRopes )
					break;
			}
		}
	}

	return nRopes;
}


void C_RopeKeyframe::SetSlack( int slack )
{
	m_Slack = slack;
	RecomputeSprings();
}


void C_RopeKeyframe::SetRopeFlags( int flags )
{
	m_RopeFlags = flags;
	UpdateVisibility();
}

	
int C_RopeKeyframe::GetRopeFlags() const
{
	return m_RopeFlags;
}


void C_RopeKeyframe::SetupHangDistance( float flHangDist )
{
	C_BaseEntity *pEnt1 = m_hStartPoint;
	C_BaseEntity *pEnt2 = m_hEndPoint;
	if ( !pEnt1 || !pEnt2 )
		return;

	QAngle dummyAngles;

	// Calculate starting conditions so we can force it to hang down N inches.
	Vector v1 = pEnt1->GetAbsOrigin();
	pEnt1->GetAttachment( m_iStartAttachment, v1, dummyAngles );
		
	Vector v2 = pEnt2->GetAbsOrigin();
	pEnt2->GetAttachment( m_iEndAttachment, v2, dummyAngles );

	float flSlack, flLen;
	CalcRopeStartingConditions( v1, v2, ROPE_MAX_SEGMENTS, flHangDist, &flLen, &flSlack );

	m_RopeLength = (int)flLen;
	m_Slack = (int)flSlack;

	RecomputeSprings();
}


void C_RopeKeyframe::SetStartEntity( C_BaseEntity *pEnt )
{
	m_hStartPoint = pEnt;
}


void C_RopeKeyframe::SetEndEntity( C_BaseEntity *pEnt )
{
	m_hEndPoint = pEnt;
}


C_BaseEntity* C_RopeKeyframe::GetStartEntity() const
{
	return m_hStartPoint;
}


C_BaseEntity* C_RopeKeyframe::GetEndEntity() const
{
	return m_hEndPoint;
}


CSimplePhysics::IHelper* C_RopeKeyframe::HookPhysics( CSimplePhysics::IHelper *pHook )
{
	m_RopePhysics.SetDelegate( pHook );
	return &m_PhysicsDelegate;
}


void C_RopeKeyframe::SetColorMod( const Vector &vColorMod )
{
	m_vColorMod = vColorMod;
}


void C_RopeKeyframe::RecomputeSprings()
{
	m_RopePhysics.ResetSpringLength(
		(m_RopeLength + m_Slack + ROPESLACK_FUDGEFACTOR) / (m_RopePhysics.NumNodes() - 1) );
}


void C_RopeKeyframe::ShakeRope( const Vector &vCenter, float flRadius, float flMagnitude )
{
	// Sum up whatever it would apply to all of our points.
	for ( int i=0; i < m_nSegments; i++ )
	{
		CSimplePhysics::CNode *pNode = m_RopePhysics.GetNode( i );

		float flDist = (pNode->m_vPos - vCenter).Length();
	
		float flShakeAmount = 1.0f - flDist / flRadius;
		if ( flShakeAmount >= 0 )
		{
			m_flImpulse.z += flShakeAmount * flMagnitude;
		}
	}
}


void C_RopeKeyframe::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	m_bNewDataThisFrame = true;

	if( updateType != DATA_UPDATE_CREATED )
		return;

	// Figure out the material name.
	char str[512];
	const model_t *pModel = modelinfo->GetModel( m_iRopeMaterialModel );
	if ( pModel )
	{
		Q_strncpy( str, modelinfo->GetModelName( pModel ), sizeof( str ) );

		// Get rid of the extension because the material system doesn't want it.
		char *pExt = Q_stristr( str, ".vmt" );
		if ( pExt )
			pExt[0] = 0;
	}
	else
	{
		Q_strncpy( str, "asdf", sizeof( str ) );
	}
	
	FinishInit( str );
}


void C_RopeKeyframe::FinishInit( const char *pMaterialName )
{
	// Get the material from the material system.	
	m_pMaterial = materials->FindMaterial( pMaterialName, TEXTURE_GROUP_OTHER );
	if( m_pMaterial )
		m_TextureHeight = m_pMaterial->GetMappingHeight();
	else
		m_TextureHeight = 1;

	char backName[512];
	Q_snprintf( backName, sizeof( backName ), "%s_back", pMaterialName );
	
	m_pBackMaterial = materials->FindMaterial( backName, TEXTURE_GROUP_OTHER, false );
	if ( IsErrorMaterial( m_pBackMaterial ) )
		m_pBackMaterial = NULL;

	if ( m_pBackMaterial )
		m_pBackMaterial->GetMappingWidth();

	
	// Init rope physics.
	m_nSegments = clamp( m_nSegments, 2, ROPE_MAX_SEGMENTS );
	m_RopePhysics.SetNumNodes( m_nSegments );

	SetCollisionBounds( Vector( -10, -10, -10 ), Vector( 10, 10, 10 ) );

	// We want to think every frame.
	SetNextClientThink( CLIENT_THINK_ALWAYS );
}

void C_RopeKeyframe::RunRopeSimulation( float flSeconds )
{
	// First, forget about links touching things.
	for ( int i=0; i < m_nSegments; i++ )
		m_LinksTouchingSomething[i] = false;

	// Simulate, and it will mark which links touched things.
	m_RopePhysics.Simulate( flSeconds );

	// Now count how many links touched something.
	m_nLinksTouchingSomething = 0;
	for ( int i=0; i < m_nSegments; i++ )
	{
		if ( m_LinksTouchingSomething[i] )
			++m_nLinksTouchingSomething;
	}
}

void C_RopeKeyframe::ClientThink()
{
	// Only recalculate the endpoint attachments once per frame.
	m_bEndPointAttachmentsDirty = true;
	
	if( !r_drawropes.GetBool() )
		return;

	if( !InitRopePhysics() ) // init if not already
		return;

	if( !DetectRestingState( m_bApplyWind ) )
	{
		// Update the simulation.
		CTimeAdder adder( &g_RopeSimulateTicks );
		
		RunRopeSimulation( gpGlobals->frametime );

		g_nRopePointsSimulated += m_RopePhysics.NumNodes();

		m_bNewDataThisFrame = false;

		// Setup a new wind gust?
		m_flCurrentGustTimer += gpGlobals->frametime;
		m_flTimeToNextGust -= gpGlobals->frametime;
		if( m_flTimeToNextGust <= 0 )
		{
			m_vWindDir = RandomVector( -1, 1 );
			VectorNormalize( m_vWindDir );

			static float basicScale = 50;
			m_vWindDir *= basicScale;
			m_vWindDir *= RandomFloat( -1.0f, 1.0f );
			
			m_flCurrentGustTimer = 0;
			m_flCurrentGustLifetime = RandomFloat( 2.0f, 3.0f );

			m_flTimeToNextGust = RandomFloat( 3.0f, 4.0f );
		}

		UpdateBBox();
	}
}


int C_RopeKeyframe::DrawModel( int flags )
{
	VPROF_BUDGET( "C_RopeKeyframe::DrawModel", VPROF_BUDGETGROUP_ROPES );
	if( !InitRopePhysics() )
		return 0;

	if ( !m_bReadyToDraw )
		return 0;

	// Resize the rope
	if( m_RopeFlags & ROPE_RESIZE )
	{
		RecomputeSprings();
	}

	// If our start & end entities have models, but are nodraw, then we don't draw
	if ( m_hStartPoint && m_hStartPoint->IsDormant() && m_hEndPoint && m_hEndPoint->IsDormant() )
	{
		// Check models because rope endpoints are point entities
		if ( m_hStartPoint->GetModelIndex() && m_hEndPoint->GetModelIndex() )
			return 0;
	}

	RopeManager()->AddToRenderCache( this );
	return 1;
}

bool C_RopeKeyframe::ShouldDraw()
{
	if( !r_ropetranslucent.GetBool() ) 
		return false;

	if( !(m_RopeFlags & ROPE_SIMULATE) )
		return false;

	return true;
}

const Vector& C_RopeKeyframe::WorldSpaceCenter( ) const
{
	return GetAbsOrigin();
}

bool C_RopeKeyframe::GetAttachment( int number, Vector &origin, QAngle &angles )
{
	int nNodes = m_RopePhysics.NumNodes();
	if ( (number == ROPE_ATTACHMENT_START_POINT || number == ROPE_ATTACHMENT_END_POINT) && nNodes >= 2 )
	{
		// Now setup the orientation based on the last segment.
		Vector vForward;
		if ( number == ROPE_ATTACHMENT_START_POINT )
		{
			origin = m_RopePhysics.GetNode( 0 )->m_vPredicted;
			vForward = m_RopePhysics.GetNode( 0 )->m_vPredicted - m_RopePhysics.GetNode( 1 )->m_vPredicted;
		}
		else
		{
			origin = m_RopePhysics.GetNode( nNodes-1 )->m_vPredicted;
			vForward = m_RopePhysics.GetNode( nNodes-1 )->m_vPredicted - m_RopePhysics.GetNode( nNodes-2 )->m_vPredicted;
		}
		VectorAngles( vForward, angles );

		return true;
	}
	
	return false;
}

bool C_RopeKeyframe::AnyPointsMoved()
{
	for( int i=0; i < m_RopePhysics.NumNodes(); i++ )
	{
		CSimplePhysics::CNode *pNode = m_RopePhysics.GetNode( i );
		float flMoveDistSqr = (pNode->m_vPos - pNode->m_vPrevPos).LengthSqr();
		if( flMoveDistSqr > 0.03f )
			return true;
	}

	if( --m_iForcePointMoveCounter > 0 )
		return true;

	return false;
}


inline bool C_RopeKeyframe::DidEndPointMove( int iPt )
{
	// If this point isn't locked anyway, just break out.
	if( !( m_fLockedPoints & (1 << iPt) ) )
		return false;

	bool bOld = m_bPrevEndPointPos[iPt];
	Vector vOld = m_vPrevEndPointPos[iPt];

	m_bPrevEndPointPos[iPt] = GetEndPointPos( iPt, m_vPrevEndPointPos[iPt] );
	
	// If it wasn't and isn't attached to anything, don't register a change.
	if( !bOld && !m_bPrevEndPointPos[iPt] )
		return true;

	// Register a change if the endpoint moves.
	if( !VectorsAreEqual( vOld, m_vPrevEndPointPos[iPt], 0.1 ) )
		return true;

	return false;
}


bool C_RopeKeyframe::DetectRestingState( bool &bApplyWind )
{
	bApplyWind = false;

	if( m_fPrevLockedPoints != m_fLockedPoints )
	{
		// Force it to move the points for some number of frames when they get detached or
		// after we get new data. This allows them to accelerate from gravity.
		m_iForcePointMoveCounter = 10; 
		m_fPrevLockedPoints = m_fLockedPoints;
		return false;
	}

	if( m_bNewDataThisFrame )
	{
		// Simulate if anything about us changed this frame, such as our position due to hierarchy.
		// FIXME: this won't work when hierarchy is client side
		return false;
	}

	// Make sure our attachment points haven't moved.
	if( DidEndPointMove( 0 ) || DidEndPointMove( 1 ) )
		return false;

	// See how close we are to the line.
	Vector &vEnd1 = m_RopePhysics.GetFirstNode()->m_vPos;
	Vector &vEnd2 = m_RopePhysics.GetLastNode()->m_vPos;
	
	if ( !( m_RopeFlags & ROPE_NO_WIND ) )
	{
		// Don't apply wind if more than half of the nodes are touching something.
		float flDist1 = CalcDistanceToLineSegment( MainViewOrigin(), vEnd1, vEnd2 );
		if( m_nLinksTouchingSomething < (m_RopePhysics.NumNodes() >> 1) )
			bApplyWind = flDist1 < rope_wind_dist.GetFloat();
	}

	if ( m_flPreviousImpulse != m_flImpulse )
	{
		m_flPreviousImpulse = m_flImpulse;
		return false;
	}

	return !AnyPointsMoved() && !bApplyWind && !rope_shake.GetInt();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_RopeKeyframe::BuildRope( RopeSegData_t *pSegmentData )
{
	if ( !pSegmentData )
		return;

	// Get the lighting values.
	Vector *pLightValues = mat_fullbright.GetInt() ? g_FullBright_LightValues : m_LightValues;

	// Update the rope subdivisions if necessary.
	int nSubdivCount;
	float *pSubdivList;
	UpdateRopeSubdivs( &pSubdivList, &nSubdivCount );

	int nSegmentCount = 0;
	int iPrevNode = 0;
	for( int iNode = 0; iNode < m_RopePhysics.NumNodes(); ++iNode )
	{
		Vector &vecCurrent = m_RopePhysics.GetNode( iNode )->m_vPredicted;
	
		pSegmentData->m_Segments[nSegmentCount].m_vPos = vecCurrent;
		pSegmentData->m_Segments[nSegmentCount].m_vColor = pLightValues[iNode] * m_vColorMod;
		++nSegmentCount;

		if ( ( iNode + 1 ) < m_RopePhysics.NumNodes() )
		{
			// Draw a midpoint to the next segment.
			int iNext = iNode + 1;
			int iNextNext = iNode + 2;
			if ( iNext >= m_RopePhysics.NumNodes() )
			{
				iNext = iNextNext = m_RopePhysics.NumNodes() - 1;
			}
			else if ( iNextNext >= m_RopePhysics.NumNodes() )
			{
				iNextNext = m_RopePhysics.NumNodes() - 1;
			}

			Vector vecColorInc = ( ( pLightValues[iNode+1] - pLightValues[iNode] ) * m_vColorMod ) / ( nSubdivCount + 1 );
			for( int iSubdiv = 0; iSubdiv < nSubdivCount; ++iSubdiv )
			{
				pSegmentData->m_Segments[nSegmentCount].m_vColor = pSegmentData->m_Segments[nSegmentCount-1].m_vColor + vecColorInc;
				Catmull_Rom_Spline( m_RopePhysics.GetNode( iPrevNode )->m_vPredicted, m_RopePhysics.GetNode( iNode )->m_vPredicted,
									m_RopePhysics.GetNode( iNext )->m_vPredicted, m_RopePhysics.GetNode( iNextNext )->m_vPredicted,
									pSubdivList[iSubdiv], pSegmentData->m_Segments[nSegmentCount].m_vPos );
				++nSegmentCount;
				Assert( nSegmentCount <= MAX_ROPE_SEGMENTS );
			}

			iPrevNode = iNode;
		}
	}

	pSegmentData->m_nSegmentCount = nSegmentCount;
	pSegmentData->m_flMaxBackWidth = 0;

	// Figure out texture scale.
	float flPixelsPerInch = 4.0f / m_TextureScale;
	float flTotalTexCoord = flPixelsPerInch * ( m_RopeLength + m_Slack + ROPESLACK_FUDGEFACTOR );
	int nTotalPoints = ( m_RopePhysics.NumNodes() - 1 ) * nSubdivCount + 1;
	float flActualInc = ( flTotalTexCoord / nTotalPoints ) / ( float )m_TextureHeight;

	// First draw a translucent rope underneath the solid rope for an antialiasing effect.
	if ( ShouldUseFakeAA( m_pBackMaterial ) )
	{
		// Compute screen width
		float flScreenWidth = ScreenWidth();
		float flHalfScreenWidth = flScreenWidth / 2.0f;

		float flExtraScreenSpaceWidth = rope_smooth_enlarge.GetFloat();

		float flMinAlpha = rope_smooth_minalpha.GetFloat();
		float flMaxAlpha = rope_smooth_maxalpha.GetFloat();

		float flMinScreenSpaceWidth = rope_smooth_minwidth.GetFloat();
		float flMaxAlphaScreenSpaceWidth = rope_smooth_maxalphawidth.GetFloat();
		
		float flTexCoord = m_flCurScroll;
		for ( int iSegment = 0; iSegment < nSegmentCount; ++iSegment )
		{
			pSegmentData->m_Segments[iSegment].m_flTexCoord = flTexCoord;			

			// Right here, we need to specify a width that will be 1 pixel larger in screen space.
			float zCoord = CurrentViewForward().Dot( pSegmentData->m_Segments[iSegment].m_vPos - CurrentViewOrigin() );
			zCoord = max( zCoord, 0.1f );
							
			float flScreenSpaceWidth = m_Width * flHalfScreenWidth / zCoord;
			if ( flScreenSpaceWidth < flMinScreenSpaceWidth )
			{
				pSegmentData->m_Segments[iSegment].m_flAlpha = flMinAlpha;
				pSegmentData->m_Segments[iSegment].m_flWidth = flMinScreenSpaceWidth * zCoord / flHalfScreenWidth;
				pSegmentData->m_BackWidths[iSegment] = 0.0f;
			}
			else
			{
				if ( flScreenSpaceWidth > flMaxAlphaScreenSpaceWidth )
				{
					pSegmentData->m_Segments[iSegment].m_flAlpha = flMaxAlpha;
				}
				else
				{
					pSegmentData->m_Segments[iSegment].m_flAlpha = RemapVal( flScreenSpaceWidth, flMinScreenSpaceWidth, flMaxAlphaScreenSpaceWidth, flMinAlpha, flMaxAlpha );
				}

				pSegmentData->m_Segments[iSegment].m_flWidth = m_Width;
				pSegmentData->m_BackWidths[iSegment] = m_Width - ( zCoord * flExtraScreenSpaceWidth ) / flScreenWidth;
				if ( pSegmentData->m_BackWidths[iSegment] < 0.0f )
				{
					pSegmentData->m_BackWidths[iSegment] = 0.0f;
				}
				else
				{
					pSegmentData->m_flMaxBackWidth = max( pSegmentData->m_flMaxBackWidth, pSegmentData->m_BackWidths[iSegment] );
				}
			}

			// Get the next texture coordinate.
			flTexCoord += flActualInc;
		}
	}
	else
	{
		float flTexCoord = m_flCurScroll;

		// Build the data with no smoothing.
		for ( int iSegment = 0; iSegment < nSegmentCount; ++iSegment )
		{
			pSegmentData->m_Segments[iSegment].m_flTexCoord = flTexCoord;
			pSegmentData->m_Segments[iSegment].m_flAlpha = 0.3f;
			pSegmentData->m_Segments[iSegment].m_flWidth = m_Width;
			pSegmentData->m_BackWidths[iSegment] = -1.0f;

			// Get the next texture coordinate.
			flTexCoord += flActualInc;
		}
	}
}

void C_RopeKeyframe::UpdateBBox()
{
	Vector &vStart = m_RopePhysics.GetFirstNode()->m_vPos;
	Vector &vEnd   = m_RopePhysics.GetLastNode()->m_vPos;

	Vector mins, maxs;

	VectorMin( vStart, vEnd, mins );
	VectorMax( vStart, vEnd, maxs );

	for( int i=1; i < m_RopePhysics.NumNodes()-1; i++ )
	{
		const Vector &vPos = m_RopePhysics.GetNode(i)->m_vPos;
		AddPointToBounds( vPos, mins, maxs );
	}
	
	mins -= GetAbsOrigin();
	maxs -= GetAbsOrigin();
	SetCollisionBounds( mins, maxs );
}


bool C_RopeKeyframe::InitRopePhysics()
{
	if( !(m_RopeFlags & ROPE_SIMULATE) )
		return 0;

	if( m_bPhysicsInitted )
	{
		return true;
	}

	// Must have both entities to work.
	m_bPrevEndPointPos[0] = GetEndPointPos( 0, m_vPrevEndPointPos[0] );
	if( !m_bPrevEndPointPos[0] )
		return false;

	// They're allowed to not have an end attachment point so the rope can dangle.
	m_bPrevEndPointPos[1] = GetEndPointPos( 1, m_vPrevEndPointPos[1] );
	if( !m_bPrevEndPointPos[1] )
		m_vPrevEndPointPos[1] = m_vPrevEndPointPos[0];

	const Vector &vStart = m_vPrevEndPointPos[0];
	const Vector &vAttached = m_vPrevEndPointPos[1];

	m_RopePhysics.SetupSimulation( 0, &m_PhysicsDelegate );
	RecomputeSprings();
	m_RopePhysics.Restart();

	// Initialize the positions of the nodes.
	for( int i=0; i < m_RopePhysics.NumNodes(); i++ )
	{
		CSimplePhysics::CNode *pNode = m_RopePhysics.GetNode( i );
		float t = (float)i / (m_RopePhysics.NumNodes() - 1);
		
		VectorLerp( vStart, vAttached, t, pNode->m_vPos );
		pNode->m_vPrevPos = pNode->m_vPos;
	}

	// Simulate for a bit to let it sag.
	if ( m_RopeFlags & ROPE_INITIAL_HANG )
	{
		RunRopeSimulation( 5 );
	}

	CalcLightValues();

	// Set our bounds for visibility.
	UpdateBBox();

	m_flTimeToNextGust = RandomFloat( 1.0f, 3.0f );
	m_bPhysicsInitted = true;
	
	return true;
}


bool C_RopeKeyframe::CalculateEndPointAttachment( C_BaseEntity *pEnt, int iAttachment, Vector &vPos, QAngle &angles )
{
	VPROF_BUDGET( "C_RopeKeyframe::CalculateEndPointAttachment", VPROF_BUDGETGROUP_ROPES );

	if( !pEnt )
		return false;

	vPos = pEnt->WorldSpaceCenter( );
	angles = pEnt->GetAbsAngles();

	if ( m_RopeFlags & ROPE_PLAYER_WPN_ATTACH )
	{
		C_BasePlayer *pPlayer = dynamic_cast< C_BasePlayer* >( pEnt );
		if ( pPlayer )
		{
			C_BaseAnimating *pModel = pPlayer->GetRenderedWeaponModel();
			if ( !pModel )
				return false;

			int iAttachment = pModel->LookupAttachment( "buff_attach" );
			return pModel->GetAttachment( iAttachment, vPos, angles );
		}
	}

	if( iAttachment > 0 )
	{
		 if( !pEnt->GetAttachment( iAttachment, vPos, angles ) )
			return false;
	}

	return true;
}

bool C_RopeKeyframe::GetEndPointPos( int iPt, Vector &vPos )
{
	QAngle angle;
	return GetEndPointAttachment( iPt, vPos, angle );
}

bool C_RopeKeyframe::GetEndPointAttachment( int iPt, Vector &vPos, QAngle &angle )
{
	// By caching the results here, we avoid doing this a bunch of times per frame.
	if ( m_bEndPointAttachmentsDirty )
	{
		CalculateEndPointAttachment( m_hStartPoint, m_iStartAttachment, m_vCachedEndPointAttachmentPos[0], m_vCachedEndPointAttachmentAngle[0] );
		CalculateEndPointAttachment( m_hEndPoint, m_iEndAttachment, m_vCachedEndPointAttachmentPos[1], m_vCachedEndPointAttachmentAngle[1] );
		m_bEndPointAttachmentsDirty = false;
	}

	Assert( iPt == 0 || iPt == 1 );
	vPos = m_vCachedEndPointAttachmentPos[iPt];
	angle = m_vCachedEndPointAttachmentAngle[iPt];
	return true;
}

// Look at the global cvar and recalculate rope subdivision data if necessary.
void C_RopeKeyframe::UpdateRopeSubdivs( float **pSubdivs, int *nSubdivs )
{
	if( m_RopeFlags & ROPE_BARBED )
	{
		*pSubdivs = g_BarbedSubdivs;
		*nSubdivs = g_nBarbedSubdivs;
	}
	else
	{
		int subdiv = m_Subdiv;
		if ( subdiv == 255 )
		{
			subdiv = rope_subdiv.GetInt();
		}

		if ( subdiv > MAX_ROPE_SUBDIVS )
			subdiv = MAX_ROPE_SUBDIVS;

		*pSubdivs = g_RopeSubdivs[subdiv];
		*nSubdivs = subdiv;
	}
}


void C_RopeKeyframe::CalcLightValues()
{
	Vector boxColors[6];

	for( int i=0; i < m_RopePhysics.NumNodes(); i++ )
	{
		const Vector &vPos = m_RopePhysics.GetNode(i)->m_vPredicted;
		engine->ComputeLighting( vPos, NULL, true, m_LightValues[i], boxColors );

		if ( !rope_averagelight.GetInt() )
		{
			// The engine averages the lighting across the 6 box faces, but we would rather just get the MAX intensity
			// since we do our own half-lambert lighting in the rope shader to simulate directionality.
			//
			// So here, we take the average of all the incoming light, and scale it to use the max intensity of all the box sides.
			float flMaxIntensity = 0;
			for ( int iSide=0; iSide < 6; iSide++ )
			{
				float flLen = boxColors[iSide].Length();
				flMaxIntensity = max( flMaxIntensity, flLen );
			}

			VectorNormalize( m_LightValues[i] );
			m_LightValues[i] *= flMaxIntensity;
			float flMax = max( m_LightValues[i].x, max( m_LightValues[i].y, m_LightValues[i].z ) );
			if ( flMax > 1 )
				m_LightValues[i] /= flMax;
		}
	}
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void C_RopeKeyframe::ReceiveMessage( int classID, bf_read &msg )
{
	if ( classID != GetClientClass()->m_ClassID )
	{
		// message is for subclass
		BaseClass::ReceiveMessage( classID, msg );
		return;
	}

	// Read instantaneous fore data
	m_flImpulse.x   = msg.ReadFloat();
	m_flImpulse.y   = msg.ReadFloat();
	m_flImpulse.z   = msg.ReadFloat();
}

