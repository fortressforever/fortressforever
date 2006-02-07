//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef DISPCOLL_COMMON_H
#define DISPCOLL_COMMON_H
#pragma once

#include "trace.h"
#include "builddisp.h"
#include "terrainmod.h"

#define DISPCOLL_TREETRI_SIZE		MAX_DISPTRIS
#define DISPCOLL_DIST_EPSILON		0.03125f
#define DISPCOLL_ROOTNODE_INDEX		0
#define DISPCOLL_INVALID_TRI		-1
#define DISPCOLL_INVALID_FRAC		-99999.9f
#define DISPCOLL_NORMAL_UNDEF		0xffff

extern double g_flDispCollSweepTimer;
extern double g_flDispCollIntersectTimer;
extern double g_flDispCollInCallTimer;

class CDispLeafLink;
class ITerrainMod;

struct RayDispOutput_t
{
	short	ndxVerts[4];	// 3 verts and a pad
	float	u, v;			// the u, v paramters (edgeU = v1 - v0, edgeV = v2 - v0)
	float	dist;			// intersection distance
};

//=============================================================================
//	Displacement Collision Triangle
class CDispCollTri
{

	struct index_t
	{
		union
		{
			struct
			{
				unsigned short uiVert:9;
				unsigned short uiMin:2;
				unsigned short uiMax:2;
			} m_Index;
			
			unsigned short m_IndexDummy;
		};
	};

	index_t				m_TriData[3];
	unsigned short		m_uiCache;				// 1-bit cached, 15-bit index

public:

	Vector				m_vecNormal;			// Triangle normal (plane normal).
	float				m_flDist;				// Triangle plane dist.
	byte				m_ucSignBits;			// Plane test.
	byte				m_ucPlaneType;			// Axial test?
	unsigned short		m_uiFlags;				// Uses 5-bits - maybe look into merging it with something?
	unsigned short		m_uiPad[2];				// Pad to 32-bytes.

	// Creation.
	     CDispCollTri();
	void Init( void );
	void CalcPlane( CUtlVector<Vector> &m_aVerts );
	void FindMinMax( CUtlVector<Vector> &m_aVerts );

	// Triangle data.
	inline void SetVert( int iPos, int iVert )			{ Assert( ( iPos >= 0 ) && ( iPos < 3 ) ); Assert( ( iVert >= 0 ) && ( iVert < ( 1 << 9 ) ) ); m_TriData[iPos].m_Index.uiVert = iVert; }
	inline int  GetVert( int iPos )						{ Assert( ( iPos >= 0 ) && ( iPos < 3 ) ); return m_TriData[iPos].m_Index.uiVert; }
	inline void SetMin( int iAxis, int iMin )			{ Assert( ( iAxis >= 0 ) && ( iAxis < 3 ) ); Assert( ( iMin >= 0 ) && ( iMin < 3 ) ); m_TriData[iAxis].m_Index.uiMin = iMin; }
	inline int  GetMin( int iAxis )						{ Assert( ( iAxis >= 0 ) && ( iAxis < 3 ) ); return m_TriData[iAxis].m_Index.uiMin; }
	inline void SetMax( int iAxis, int iMax )			{ Assert( ( iAxis >= 0 ) && ( iAxis < 3 ) ); Assert( ( iMax >= 0 ) && ( iMax < 3 ) ); m_TriData[iAxis].m_Index.uiMax = iMax; }
	inline int  GetMax( int iAxis )						{ Assert( ( iAxis >= 0 ) && ( iAxis < 3 ) ); return m_TriData[iAxis].m_Index.uiMax; }

	// Cache.
	inline void			  SetCacheIndex( int iCache )	{ Assert( ( iCache >= 0 ) && ( iCache < ( 1 << 15 ) ) ); m_uiCache = iCache; m_uiCache |= 0x8000; } 
	inline unsigned short GetCacheIndex( void )			{ return ( m_uiCache & 0x7fff ); }
	inline bool			  IsCached( void )				{ return ( ( m_uiCache & 0x8000 ) != 0 ); }
	inline void			  ClearCache( void )			{ m_uiCache &= ~0x8000; }
};

//=============================================================================
//	AABB Node
class CDispCollAABBNode
{
public:

	Vector  m_vecBox[2];					// 0 - min, 1 - max
	short	m_iTris[2];
	short	m_iPad;							// Pad to 32-bytes.

		 CDispCollAABBNode();
	void Init( void );	
	inline bool IsLeaf( void )				{ return ( ( m_iTris[0] != DISPCOLL_INVALID_TRI ) || ( m_iTris[1] != DISPCOLL_INVALID_TRI ) ); }
	void GenerateBox( CUtlVector <CDispCollTri> &m_aTris, CUtlVector<Vector> &m_aVerts );
};

//=============================================================================
//	Helper
class CDispCollHelper
{
public:

	float	m_flStartFrac;
	float	m_flEndFrac;
	Vector	m_vecImpactNormal;
	float	m_flImpactDist;
};

//=============================================================================
//	Cache
class CDispCollTriCache
{
public:

	unsigned short m_iCrossX[3];
	unsigned short m_iCrossY[3];
	unsigned short m_iCrossZ[3];
};

//=============================================================================
//
// Displacement Collision Tree Data
//
class CDispCollTree
{
public:

	// Creation/Destruction.
	CDispCollTree();
	~CDispCollTree();
	virtual bool Create( CCoreDispInfo *pDisp );

	// Raycasts.
	bool AABBTree_Ray( const Ray_t &ray, CBaseTrace *pTrace, bool bSide = true );
	bool AABBTree_Ray( const Ray_t &ray, RayDispOutput_t &output );

	// Hull Sweeps.
	bool AABBTree_SweepAABB( const Ray_t &ray, CBaseTrace *pTrace );

	// Hull Intersection.
	bool AABBTree_IntersectAABB( const Ray_t &ray );

	// Point/Box vs. Bounds.
	bool PointInBounds( Vector const &vecBoxCenter, Vector const &vecBoxMin, Vector const &vecBoxMax, bool bPoint );

	// Terrain modification - update the collision model.
	void ApplyTerrainMod( ITerrainMod *pMod );

	// Utility.
	inline void SetPower( int power )								{ m_nPower = power; }
	inline int GetPower( void )										{ return m_nPower; }

	inline int GetWidth( void )										{ return ( ( 1 << m_nPower ) + 1 ); }
	inline int GetHeight( void )									{ return ( ( 1 << m_nPower ) + 1 ); }
	inline int GetSize( void )										{ return ( ( 1 << m_nPower ) + 1 ) * ( ( 1 << m_nPower ) + 1 ); }
	inline int GetTriSize( void )									{ return ( ( 1 << m_nPower ) * ( 1 << m_nPower ) * 2 ); }

//	inline void SetTriFlags( short iTri, unsigned short nFlags )	{ m_aTris[iTri].m_uiFlags = nFlags; }

	inline void GetStabDirection( Vector &vecDir )					{ vecDir = m_vecStabDir; }

	inline void GetBounds( Vector &vecBoxMin, Vector &vecBoxMax )	{ vecBoxMin = m_vecBounds[0]; vecBoxMax = m_vecBounds[1]; }

	inline CDispLeafLink*&	GetLeafLinkHead()						{ return m_pLeafLinkHead; }

	inline int GetContents( void )									{ return m_nContents; }
	inline void SetSurfaceProps( int iProp, short nSurfProp )		{ Assert( ( iProp >= 0 ) && ( iProp < 2 ) ); m_nSurfaceProps[iProp] = nSurfProp; }
	inline short GetSurfaceProps( int iProp )						{ return m_nSurfaceProps[iProp]; }

	inline void SetCheckCount( int nDepth, int nCount )				{ Assert( ( nDepth >= 0 ) && ( nDepth < MAX_CHECK_COUNT_DEPTH ) ); m_nCheckCount[nDepth] = nCount; }
	inline int GetCheckCount( int nDepth ) const					{ Assert( ( nDepth >= 0 ) && ( nDepth < MAX_CHECK_COUNT_DEPTH ) ); return m_nCheckCount[nDepth]; }

	inline unsigned int GetMemorySize( void )						{ return m_nSize; }
	inline unsigned int GetCacheMemorySize( void )					{ return m_nCacheSize; }

public:

	inline int Nodes_GetChild( int iNode, int nDirection );
	inline int Nodes_CalcCount( int nPower );
	inline int Nodes_GetParent( int iNode );
	inline int Nodes_GetLevel( int iNode );
	inline int Nodes_GetIndexFromComponents( int x, int y );

protected:

	bool AABBTree_Create( CCoreDispInfo *pDisp );
	void AABBTree_CopyDispData( CCoreDispInfo *pDisp );
	void AABBTree_CreateLeafs( void );
	void AABBTree_GenerateBoxes( void );
	void AABBTree_CalcBounds( void );

	void AABBTree_BuildTreeTrisSweep_r( const Ray_t &ray, int iNode, CDispCollTri **ppTreeTris, unsigned short &nTriCount );
	void AABBTree_BuildTreeTrisIntersect_r( const Ray_t &ray, int iNode, CDispCollTri **ppTreeTris, unsigned short &nTriCount );

	void AABBTree_TreeTrisRayTest_r( const Ray_t &ray, const Vector &vecInvDelta, int iNode, CBaseTrace *pTrace, bool bSide, CDispCollTri **pImpactTri );
	void AABBTree_TreeTrisRayBarycentricTest_r( const Ray_t &ray, const Vector &vecInvDelta, int iNode, RayDispOutput_t &output, CDispCollTri **pImpactTri );
	void AABBTree_TreeTrisSweepTest_r( const Ray_t &ray, const Vector &vecInvDelta, const Vector &rayDir, int iNode, CBaseTrace *pTrace );

	bool AABBTree_SweepAABBBox( const Ray_t &ray, const Vector &rayDir, CBaseTrace *pTrace );
	void AABBTree_TreeTrisSweepTestBox_r( const Ray_t &ray, const Vector &rayDir, const Vector &vecMin, const Vector &vecMax, int iNode, CBaseTrace *pTrace );

	void AABBTree_TreeTrisIntersectBox_r( const Ray_t &ray, const Vector &vecMin, const Vector &vecMax, int iNode, bool bIntersect );

protected:

	void SweepAABBTriIntersect( const Ray_t ray, const Vector &rayDir, CDispCollTri *pTri, CBaseTrace *pTrace, bool bTestOutside );

	void Cache_TestIt( void );

	void Cache_Create( CDispCollTri *pTri );
	void Cache_Create( CDispCollTri *pTri, int iTri );		// Testing!
	bool Cache_EdgeCrossAxisX( const Vector &vecEdge, const Vector &vecOnEdge, const Vector &vecOffEdge, CDispCollTri *pTri, unsigned short &iPlane );
	bool Cache_EdgeCrossAxisY( const Vector &vecEdge, const Vector &vecOnEdge, const Vector &vecOffEdge, CDispCollTri *pTri, unsigned short &iPlane );
	bool Cache_EdgeCrossAxisZ( const Vector &vecEdge, const Vector &vecOnEdge, const Vector &vecOffEdge, CDispCollTri *pTri, unsigned short &iPlane );

	inline bool FacePlane( const Ray_t &ray, const Vector &rayDir, CDispCollTri *pTri );
	inline bool AxisPlanesXYZ( const Ray_t &ray, const Vector &rayDir, CDispCollTri *pTri );
	inline bool EdgeCrossAxisX( const Ray_t &ray, unsigned short iPlane );
	inline bool EdgeCrossAxisY( const Ray_t &ray, unsigned short iPlane );
	inline bool EdgeCrossAxisZ( const Ray_t &ray, unsigned short iPlane );

	inline bool ResolveRayPlaneIntersect( float flStart, float flEnd, const Vector &vecNormal, float flDist );

	// Utility
	inline void CalcClosestBoxPoint( const Vector &vecPlaneNormal, const Vector &vecBoxStart, const Vector &vecBoxExtents, Vector &vecBoxPoint );
	inline void CalcClosestExtents( const Vector &vecPlaneNormal, const Vector &vecBoxExtents, Vector &vecBoxPoint );
	int AddPlane( const Vector &vecNormal );

protected:

	enum { MAX_CHECK_COUNT_DEPTH = 2 };

	int								m_nPower;								// Size of the displacement ( 2^power + 1 )

	CDispLeafLink					*m_pLeafLinkHead;						// List that links it into the leaves.

	Vector							m_vecSurfPoints[4];						// Base surface points.
	int								m_nContents;								// The displacement surface "contents" (solid, etc...)
	short							m_nSurfaceProps[2];						// Surface properties (save off from texdata for impact responses)

	// Collision data.
	Vector							m_vecStabDir;							// Direction to stab for this displacement surface (is the base face normal)
	Vector							m_vecBounds[2];							// Bounding box of the displacement surface and base face

	int								m_nCheckCount[MAX_CHECK_COUNT_DEPTH];	// Per frame collision flag (so we check only once)

	CUtlVector<Vector>				m_aVerts;								// Displacement verts.
	CUtlVector<CDispCollTri>		m_aTris;								// Displacement triangles.
	CUtlVector<CDispCollAABBNode>	m_aNodes;								// Nodes.
	
	// Cache
	CUtlVector<CDispCollTriCache>	m_aTrisCache;
	CUtlVector<Vector>				m_aEdgePlanes;

	CDispCollHelper					m_Helper;

	unsigned int					m_nSize;
	unsigned int					m_nCacheSize;
};

//-----------------------------------------------------------------------------
// Purpose: get the child node index given the current node index and direction
//          of the child (1 of 4)
//   Input: iNode - current node index
//          nDirection - direction of the child ( [0...3] - SW, SE, NW, NE )
//  Output: int - the index of the child node
//-----------------------------------------------------------------------------
inline int CDispCollTree::Nodes_GetChild( int iNode, int nDirection )
{
	// node range [0...m_NodeCount)
	Assert( iNode >= 0 );
	Assert( iNode < m_aNodes.Count() );

    // ( node index * 4 ) + ( direction + 1 )
    return ( ( iNode << 2 ) + ( nDirection + 1 ) );	
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
inline int CDispCollTree::Nodes_CalcCount( int nPower )
{ 
	Assert( nPower >= 1 );
	Assert( nPower <= 4 );

	return ( ( 1 << ( ( nPower + 1 ) << 1 ) ) / 3 ); 
}

//-----------------------------------------------------------------------------
// Purpose: get the parent node index given the current node
//   Input: iNode - current node index
//  Output: int - the index of the parent node
//-----------------------------------------------------------------------------
inline int CDispCollTree::Nodes_GetParent( int iNode )
{
	// node range [0...m_NodeCount)
	Assert( iNode >= 0 );
	Assert( iNode < m_aNodes.Count() );

	// ( node index - 1 ) / 4
	return ( ( iNode - 1 ) >> 2 );
}

//-----------------------------------------------------------------------------
// Purpose:
// TODO: should make this a function - not a hardcoded set of statements!!!
//-----------------------------------------------------------------------------
inline int CDispCollTree::Nodes_GetLevel( int iNode )
{
	// node range [0...m_NodeCount)
	Assert( iNode >= 0 );
	Assert( iNode < m_aNodes.Count() );

	// level = 2^n + 1
	if ( iNode == 0 )  { return 1; }
	if ( iNode < 5 )   { return 2; }
	if ( iNode < 21 )  { return 3; }
	if ( iNode < 85 )  { return 4; }
	if ( iNode < 341 ) { return 5; }

	return -1;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
inline int CDispCollTree::Nodes_GetIndexFromComponents( int x, int y )
{
	int nIndex = 0;

	// Interleave bits from the x and y values to create the index
	int iShift;
	for( iShift = 0; x != 0; iShift += 2, x >>= 1 )
	{
		nIndex |= ( x & 1 ) << iShift;
	}

	for( iShift = 1; y != 0; iShift += 2, y >>= 1 )
	{
		nIndex |= ( y & 1 ) << iShift;
	}

	return nIndex;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
inline void CDispCollTree::CalcClosestBoxPoint( const Vector &vecPlaneNormal, const Vector &vecBoxStart, 
											    const Vector &vecBoxExtents, Vector &vecBoxPoint )
{
	vecBoxPoint = vecBoxStart;
	( vecPlaneNormal[0] < 0.0f ) ? vecBoxPoint[0] += vecBoxExtents[0] : vecBoxPoint[0] -= vecBoxExtents[0];
	( vecPlaneNormal[1] < 0.0f ) ? vecBoxPoint[1] += vecBoxExtents[1] : vecBoxPoint[1] -= vecBoxExtents[1];
	( vecPlaneNormal[2] < 0.0f ) ? vecBoxPoint[2] += vecBoxExtents[2] : vecBoxPoint[2] -= vecBoxExtents[2];
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
inline void CDispCollTree::CalcClosestExtents( const Vector &vecPlaneNormal, const Vector &vecBoxExtents, 
											   Vector &vecBoxPoint )
{
	( vecPlaneNormal[0] < 0.0f ) ? vecBoxPoint[0] = vecBoxExtents[0] : vecBoxPoint[0] = -vecBoxExtents[0];
	( vecPlaneNormal[1] < 0.0f ) ? vecBoxPoint[1] = vecBoxExtents[1] : vecBoxPoint[1] = -vecBoxExtents[1];
	( vecPlaneNormal[2] < 0.0f ) ? vecBoxPoint[2] = vecBoxExtents[2] : vecBoxPoint[2] = -vecBoxExtents[2];
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
inline bool CDispCollTree::ResolveRayPlaneIntersect( float flStart, float flEnd, const Vector &vecNormal, float flDist )
{
	if( ( flStart > 0.0f ) && ( flEnd > 0.0f ) ) 
		return false; 

	if( ( flStart < 0.0f ) && ( flEnd < 0.0f ) ) 
		return true; 

	if( ( flStart >= 0.0f ) && ( flEnd <= 0.0f ) )
	{
		// Find t - the parametric distance along the trace line.
		float flDenom = flStart - flEnd;
		float t = ( flDenom != 0.0f ) ? ( flStart - DISPCOLL_DIST_EPSILON ) / flDenom : 0.0f;
		if( t > m_Helper.m_flStartFrac )
		{
			m_Helper.m_flStartFrac = t;
			VectorCopy( vecNormal, m_Helper.m_vecImpactNormal );
			m_Helper.m_flImpactDist = flDist;
		}
	}
	else
	{
		// Find t - the parametric distance along the trace line.
		float flDenom = flStart - flEnd;
		float t = ( flDenom != 0.0f ) ? ( flStart + DISPCOLL_DIST_EPSILON ) / flDenom : 0.0f;
		if( t < m_Helper.m_flEndFrac )
		{
			m_Helper.m_flEndFrac = t;
		}	
	}
	
	return true;
}

//=============================================================================
// Global Helper Functions
CDispCollTree *DispCollTrees_Alloc( int count );
void DispCollTrees_Free( CDispCollTree *pTrees );

#endif // DISPCOLL_COMMON_H
