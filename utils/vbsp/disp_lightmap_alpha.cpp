//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <assert.h>
#include "vbsp.h"
#include "bspfile.h"
#include "iscratchpad3d.h"
#include "pacifier.h"
#include "utllinkedlist.h"
#include "builddisp.h"


#define MAX_DISP_NEIGHBORS	(CCoreDispSurface::MAX_CORNER_NEIGHBOR_COUNT + 4)	// sides+corners


#define MAX_LM_CLIPPED_TRIS	16


class LMTriVert
{
public:
	LMTriVert	operator+(LMTriVert const &other) const
	{
		LMTriVert ret;
		ret.m_Pos[0] = m_Pos[0]+other.m_Pos[0];
		ret.m_Pos[1] = m_Pos[1]+other.m_Pos[1];
		ret.m_Alpha = m_Alpha+other.m_Alpha;
		return ret;
	}
	LMTriVert	operator-(LMTriVert const &other) const
	{
		LMTriVert ret;
		ret.m_Pos[0] = m_Pos[0]-other.m_Pos[0];
		ret.m_Pos[1] = m_Pos[1]-other.m_Pos[1];
		ret.m_Alpha = m_Alpha-other.m_Alpha;
		return ret;
	}
	LMTriVert	operator*(float val) const
	{
		LMTriVert ret;
		ret.m_Pos[0] = m_Pos[0]*val;
		ret.m_Pos[1] = m_Pos[1]*val;
		ret.m_Alpha = m_Alpha*val;
		return ret;
	}

	Vector2D	m_Pos;	// Lightmap texture coordinates.
	float		m_Alpha;
};


class LMTri
{
public:
	LMTriVert	m_Verts[3];
	Vector2D	m_BBoxMin;
	Vector2D	m_BBoxMax;
};


class CTriSpace
{
public:
	void						SuckTrisOutOfList( CUtlLinkedList<LMTri, int> &tris, const Vector2D &vMin, const Vector2D &vMax )
	{
		m_BBoxMin = vMin;
		m_BBoxMax = vMax;

		// Add all the tris that are fully inside this box.
		int iNext;
		for ( int i=tris.Head(); i != tris.InvalidIndex(); i = iNext )
		{
			iNext = tris.Next( i );
			LMTri *pTri = &tris[i];

			if ( pTri->m_BBoxMin.x >= m_BBoxMin.x && pTri->m_BBoxMax.x <= m_BBoxMax.x &&
				pTri->m_BBoxMin.y >= m_BBoxMin.y && pTri->m_BBoxMax.y <= m_BBoxMax.y )
			{
				m_Tris.AddToTail( *pTri );
				tris.Remove( i );
			}
		}
	}

public:
	Vector2D					m_BBoxMin;
	Vector2D					m_BBoxMax;
	CUtlLinkedList<LMTri, int>	m_Tris;
};


class LMBox
{
public:
	float	left, top, right, bottom;
};


float CalcTriAlpha( LMTri *pTri, float &flArea )
{
	float vecs[2][2] = 
	{
		{pTri->m_Verts[2].m_Pos[0] - pTri->m_Verts[0].m_Pos[0], pTri->m_Verts[2].m_Pos[1] - pTri->m_Verts[0].m_Pos[1]},
		{pTri->m_Verts[1].m_Pos[0] - pTri->m_Verts[0].m_Pos[0], pTri->m_Verts[1].m_Pos[1] - pTri->m_Verts[0].m_Pos[1]}
	};

	// Area is half the magnitude of the cross product of the two edge vectors.
	flArea = (float)fabs( vecs[0][0]*vecs[1][1] - vecs[0][1]*vecs[1][0] ) / 2.0f;
	return flArea * (pTri->m_Verts[0].m_Alpha + pTri->m_Verts[1].m_Alpha + pTri->m_Verts[2].m_Alpha) / 3.0f;
}


#define MAX_CLIPPED_LM_TRI_VERTS	16

int ClipToPlane( float nx, float ny, float d, LMTriVert *pVerts, int nVerts, LMTriVert out[MAX_CLIPPED_LM_TRI_VERTS] )
{
	int nOutVerts = 0;
	int iPrev = nVerts - 1;
	float prevDot = nx*pVerts[iPrev].m_Pos[0] + ny*pVerts[iPrev].m_Pos[1] + d;
	int prevSide = prevDot >= 0;

	for( int iCur=0; iCur < nVerts; iCur++ )
	{
		float curDot = nx*pVerts[iCur].m_Pos[0] + ny*pVerts[iCur].m_Pos[1] + d;
		int curSide = curDot >= 0;
		
		if( prevSide )
		{
			out[nOutVerts] = pVerts[iPrev];
			
			if( ++nOutVerts >= MAX_CLIPPED_LM_TRI_VERTS )
				break;
		}

		if( curSide != prevSide )
		{
			// N(a+bt) + d = 0
			// t = (-d - Na) / Nb
			float a[2] = {pVerts[iPrev].m_Pos[0], pVerts[iPrev].m_Pos[1]};
			float b[2] = {pVerts[iCur].m_Pos[0] - a[0], pVerts[iCur].m_Pos[1] - a[1]};
			float t = (-d - (nx*a[0] + ny*a[1])) / (nx*b[0] + ny*b[1]);
			
			out[nOutVerts] = pVerts[iPrev] + (pVerts[iCur] - pVerts[iPrev]) * t;

			if( ++nOutVerts >= MAX_CLIPPED_LM_TRI_VERTS )
				break;
		}

		iPrev = iCur;
		prevSide = curSide;
	}

	return nOutVerts;
}


// Clip the LMTri into the specified box. Returns the number of output triangles.
int ClipLMTriIntoBox( LMTri *pTri, LMBox *pBox, LMTri *pOut, int nMaxOut )
{
	static LMTriVert verts[2][MAX_CLIPPED_LM_TRI_VERTS];
	int iCur = 0;
	int nVerts = 3;

	// Trivial reject...
	if( pTri->m_BBoxMin.x >= pBox->right || pTri->m_BBoxMax.x <= pBox->left ||
		pTri->m_BBoxMin.y >= pBox->top || pTri->m_BBoxMax.y <= pBox->bottom )
	{
		return 0;
	}
	
	nVerts = ClipToPlane( 1,0,-pBox->left, pTri->m_Verts, nVerts, verts[iCur] );
	iCur = !iCur;

	nVerts = ClipToPlane( -1,0,pBox->right, verts[!iCur], nVerts, verts[iCur] );
	iCur = !iCur;

	nVerts = ClipToPlane( 0,1,-pBox->bottom, verts[!iCur], nVerts, verts[iCur] );
	iCur = !iCur;

	nVerts = ClipToPlane( 0,-1,pBox->top, verts[!iCur], nVerts, verts[iCur] );
	
	// Triangulate the output.
	for( int i=0; i < min(nVerts-2, nMaxOut); i++ )
	{
		pOut[i].m_Verts[0] = verts[iCur][0];
		pOut[i].m_Verts[1] = verts[iCur][i+1];
		pOut[i].m_Verts[2] = verts[iCur][i+2];
	}

	return min(nVerts-2, nMaxOut);
}


void UpdateLMTriBBox( LMTri *pTri )
{
	pTri->m_BBoxMin.Init( 10000000.0f, 10000000.0f );
	pTri->m_BBoxMax.Init( -10000000.0f, -10000000.0f );

	for( int i=0; i < 3; i++ )
	{
		pTri->m_BBoxMin = pTri->m_BBoxMin.Min( pTri->m_Verts[i].m_Pos );
		pTri->m_BBoxMax = pTri->m_BBoxMax.Max( pTri->m_Verts[i].m_Pos );
	}
}


Vector V2DTo3D( Vector2D const &v )
{
	return Vector( v.x, v.y, 0 );
}		



void SetLMTriVert( LMTriVert *pVert, CCoreDispInfo *pCoreDispInfo, int index )
{
	Vector2D lmCoords;
	pCoreDispInfo->GetLuxelCoord( 0, index, lmCoords );

	pVert->m_Pos[0] = lmCoords[0];
	pVert->m_Pos[1] = lmCoords[1];
	pVert->m_Alpha = pCoreDispInfo->GetAlpha( index ) / 255.01f;
}


// Sees if pSourceFace[iVert] matches any of pMainFace's verts. If so, returns the vert
// index on pMainFace. If not, returns -1.
int MapVert( CCoreDispInfo *pSourceFace, int iVert, CCoreDispInfo *pMainFace )
{
	Vector vSource;
	pSourceFace->GetSurface()->GetPoint( iVert, vSource );
	
	for( int i=0; i < 4; i++ )
	{
		Vector vMain;
		pMainFace->GetSurface()->GetPoint( i, vMain );
		if( vSource.DistToSqr( vMain ) < 0.01f )
			return i;
	}

	return -1;
}


// If the face normals are close enough together, add the tris from iSourceFace to the list.
// Returns the number of triangles added.
void AddDispTris( CCoreDispInfo **ppCoreDispInfos, int iMainFace, int iSourceFace, CUtlLinkedList<LMTri, int> &tris )
{
	CCoreDispInfo *pMainFace = ppCoreDispInfos[ iMainFace ];
	CCoreDispInfo *pSourceFace = ppCoreDispInfos[ iSourceFace ];

	// Store off the old source face coords.
	Vector2D oldCoords[4];
	if( iMainFace != iSourceFace )
	{
		// Assuming it's a neighbor, we need to extrapolate the lightmap texture coordinates
		// from the new face onto this one.
		pSourceFace->GetSurface()->GetLuxelCoords( 0, oldCoords );

		Vector2D mainCoords[4];
		pMainFace->GetSurface()->GetLuxelCoords( 0, mainCoords );

		Vector2D sfCoords[4];
		bool bFound = false;
		int i;
		for( i=0; i < 4; i++ )
		{
			int map = -1;
			// If this edge has a source or a dest that connects to the main surface,
			// then extrapolate the main surface's tcoords to it.
			map = MapVert( pMainFace, i, pSourceFace );
			int nextMap = MapVert( pMainFace, (i+1)%4, pSourceFace );
			if( map != -1 && nextMap != -1 )
			{
				if( nextMap == (map+1)%4 )
					return;

				int iNext = (i+1) % 4;
				int iPrev = (i+3) % 4;
				sfCoords[map] = mainCoords[i];				// Set the matching verts.
				sfCoords[nextMap] = mainCoords[iNext];
				sfCoords[(map+1)%4] = mainCoords[i] * 2 - mainCoords[iPrev];	// Extrapolate from (i-1, i) to (map+1)
				sfCoords[(map+2)%4] = mainCoords[iPrev] +	// Extrapolate the remaining vertex.
					(mainCoords[i] - mainCoords[iPrev])*2 +
					(mainCoords[(iPrev+3)%4] - mainCoords[iPrev]);
				bFound = true;
				
				break;
			}
		}
		
		// Ok, these two only share a corner. They need to be mapped differently.
		if( !bFound )
		{
			for( i=0; i < 4; i++ )
			{
				int map = MapVert( pMainFace, i, pSourceFace );
				if( map != -1 )
				{
					sfCoords[map] = mainCoords[i];
					sfCoords[(map+1)%4] = mainCoords[i] * 2 - mainCoords[(i+1)%4];
					sfCoords[(map+3)%4] = mainCoords[i] * 2 - mainCoords[(i+3)%4];
					sfCoords[(map+2)%4] = mainCoords[i] * 3 - mainCoords[(i+3)%4] - mainCoords[(i+1)%4];
					bFound = true;
					break;
				}
			}
		}
		assert( bFound );

		// Set the new source face coords.
		pSourceFace->GetSurface()->SetLuxelCoords( 0, sfCoords );
		pSourceFace->CalcDispSurfCoords( true, 0 );
	}



	// Now add pSourceFace's coords.
	int rowcolSize = (1 << pSourceFace->GetPower()) + 1;
	int nQuadsPerDim = rowcolSize - 1;
	for( int triY=0; triY < nQuadsPerDim; triY++ )
	{
		for( int triX=0; triX < nQuadsPerDim; triX++ )
		{
			LMTri newTri;
			newTri.m_BBoxMin.Init();
			newTri.m_BBoxMax.Init();	// avoid assert.

			SetLMTriVert( &newTri.m_Verts[0], pSourceFace, (triY+0)*rowcolSize + (triX+0) );
			SetLMTriVert( &newTri.m_Verts[1], pSourceFace, (triY+1)*rowcolSize + (triX+0) );
			SetLMTriVert( &newTri.m_Verts[2], pSourceFace, (triY+0)*rowcolSize + (triX+1) );
			tris.AddToTail( newTri );

			SetLMTriVert( &newTri.m_Verts[0], pSourceFace, (triY+0)*rowcolSize + (triX+1) );
			SetLMTriVert( &newTri.m_Verts[1], pSourceFace, (triY+1)*rowcolSize + (triX+0) );
			SetLMTriVert( &newTri.m_Verts[2], pSourceFace, (triY+1)*rowcolSize + (triX+1) );
			tris.AddToTail( newTri );
		}
	}

	if( iMainFace != iSourceFace )
	{
		// Restore pSourceFace's coords.
		pSourceFace->GetSurface()->SetLuxelCoords( 0, oldCoords );
		pSourceFace->CalcDispSurfCoords( true, 0 );
	}
}


void DispUpdateLightmapAlpha( 
	CCoreDispInfo **ppCoreDispInfos,
	int iFace,		// The dispinfo (and dface_t) to generate the LM alpha for.
	float flPacifierMin,
	float flPacifierMax,	// For pacifier display.
	ddispinfo_t *pDisp,
	int lightmapWidth,
	int lightmapHeight )
{
	CUtlLinkedList<LMTri, int> tris;
	LMTri clippedTris[MAX_LM_CLIPPED_TRIS];


	// Add all the tris from neighbors and corner neighbors.
	CCoreDispInfo *pCoreDispInfo = ppCoreDispInfos[ iFace ];
	CCoreDispSurface *pSurface = pCoreDispInfo->GetSurface();

	AddDispTris( ppCoreDispInfos, iFace, iFace, tris );
	CUtlVector<int> neighbors;

	int i;
	for( i=0; i < 4; i++ )
	{
		// Add anything connected to this corner.
		for( int iCornerNeighbor=0; iCornerNeighbor < pSurface->GetCornerNeighborCount(i); iCornerNeighbor++ )
		{
			int iNeighbor = pSurface->GetCornerNeighbor(i, iCornerNeighbor);
			if( neighbors.Find( iNeighbor ) == -1 )
			{
				neighbors.AddToTail( iNeighbor );
				AddDispTris( ppCoreDispInfos, iFace, iNeighbor, tris );
			}
		}
	}


	// Update all the bboxes for the tris.
	Vector2D vFullMin( 1e24, 1e24 ), vFullMax( -1e24, -1e24 );
	FOR_EACH_LL( tris, j )
	{
		UpdateLMTriBBox( &tris[j] );
		
		vFullMin = vFullMin.Min( tris[j].m_BBoxMin );
		vFullMax = vFullMax.Max( tris[j].m_BBoxMax );
	}


	// Now cut the space into 4 sections and categorize the tris to speed it up a little.
	CTriSpace spaces[5];	// 01
							// 23  and 4=tris that are in multiple spaces.
	
	Vector2D vCenter = (vFullMin + vFullMax) * 0.5f;
	spaces[0].SuckTrisOutOfList( tris, Vector2D( vFullMin.x, vFullMax.y ), Vector2D( vCenter.x,  vFullMax.y ) );
	spaces[1].SuckTrisOutOfList( tris, Vector2D( vCenter.x,  vCenter.y ),  Vector2D( vFullMax.x, vFullMax.y ) );
	spaces[2].SuckTrisOutOfList( tris, Vector2D( vFullMin.x, vFullMin.y ), Vector2D( vCenter.x,  vCenter.y ) );
	spaces[3].SuckTrisOutOfList( tris, Vector2D( vCenter.x, vFullMin.y ), Vector2D( vFullMax.x,  vCenter.y ) );
	spaces[4].SuckTrisOutOfList( tris, vFullMin, vFullMax ); // All the leftover tris go in here.
	Assert( tris.Count() == 0 );	// Should have categorized them all!

	// For each lightmap pixel, construct a box around it in lightmap coordinate space and clip all the
	// triangles into the box. Then add each triangle's alpha * its area then divide by the total area of 
	// the box to wind up with the alpha value.
	LMBox box;
	for( int lightmapX=0; lightmapX <= lightmapWidth; lightmapX++ )
	{
		box.left = lightmapX - 0.5f;
		box.right = lightmapX + 0.5f;

		for( int lightmapY=0; lightmapY <= lightmapHeight; lightmapY++ )
		{
			// Construct a box in lightmap texture coordinate space.
			box.top = lightmapY + 0.5f;
			box.bottom = lightmapY - 0.5f;

			// Clip all the triangles into the box.
			float flTotalAlpha = 0, flTotalClippedArea = 0;
			for ( int iSpace=0; iSpace < 5; iSpace++ )
			{
				CTriSpace *pSpace = &spaces[iSpace];

				// Trivial-reject?
				if ( box.left >= pSpace->m_BBoxMax.x || box.right <= pSpace->m_BBoxMin.x ||
					box.top <= pSpace->m_BBoxMin.y || box.bottom >= pSpace->m_BBoxMax.y )
				{
					continue;
				}

				FOR_EACH_LL( pSpace->m_Tris, iTri )
				{
					int nClippedTris = ClipLMTriIntoBox( &pSpace->m_Tris[iTri], &box, clippedTris, MAX_LM_CLIPPED_TRIS );

					// Add their alpha...
					for( int iClippedTri=0; iClippedTri < nClippedTris; iClippedTri++ )
					{
						float flArea;
						flTotalAlpha += CalcTriAlpha( &clippedTris[iClippedTri], flArea );
						flTotalClippedArea += flArea;
					}
				}
			}

			float flBoxArea = (box.right - box.left) * (box.top - box.bottom);

			// If the lightmap pixel was partway off the edge of the surface, the alpha
			// contribution of its triangles will be diminished. This scales up their 
			// contribution as though they covered the total area of the box.
			float flClipScale;
			if( flTotalClippedArea > 0.000001f )
				flClipScale  = flBoxArea / flTotalClippedArea;
			else
				flClipScale = 0;

			// use 1-factor so that if we only have 1 texture, lightmap alpha = 1
			// needed for translucent displacements
			float flAlpha = 1.0f - (flTotalAlpha / flBoxArea) * flClipScale;

			int offset = lightmapY * (lightmapWidth+1) + lightmapX;
			g_DispLightmapAlpha[ pDisp->m_iLightmapAlphaStart + offset ] = (byte)max( min( flAlpha * 255.999f, 255.999f ), 0 );
		}

		UpdatePacifier( Lerp( (float)lightmapX / lightmapWidth, flPacifierMin, flPacifierMax ) );
	}
}

