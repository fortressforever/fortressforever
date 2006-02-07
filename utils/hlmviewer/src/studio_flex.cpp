//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include <string.h>
#include <assert.h>

#include "ViewerSettings.h"
#include "StudioModel.h"
#include "TGALoader.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/ITexture.h"
#include "matsyswin.h"
#include "istudiorender.h"

#include "studio_render.h"

Vector	g_viewtarget( 0, 0, 0 );
float g_flexdescweight[MAXSTUDIOFLEXDESC];
float g_flexdescweight2[MAXSTUDIOFLEXDESC];
Vector vright( 1, 0, 0 );
Vector vup( 0, 1, 0 );
Vector r_origin( 0, 0, 0 );


// hack!  Should probably use the gamma stuff in mathlib since we already linking it.
int LocalLinearToTexture( float v )
{
	return pow( v, 1.0f / 2.2f ) * 255;
}

// hack!  Should probably use the gamma stuff in mathlib since we already linking it.
float LocalTextureToLinear( int c )
{
	return pow( c / 255.0, 2.2 );
}



#define sign( a ) (((a) < 0) ? -1 : (((a) > 0) ? 1 : 0 ))


void StudioModel::RunFlexRules(  )
{
	int i, j;

	for (i = 0; i < m_pstudiohdr->numflexrules; i++)
	{
		float stack[32];
		int k = 0;
		mstudioflexrule_t *prule = m_pstudiohdr->pFlexRule( i );

		mstudioflexop_t *pops = prule->iFlexOp( 0 );

		for (j = 0; j < prule->numops; j++)
		{
			switch (pops->op)
			{
			case STUDIO_ADD: stack[k-2] = stack[k-2] + stack[k-1]; k--; break;
			case STUDIO_SUB: stack[k-2] = stack[k-2] - stack[k-1]; k--; break;
			case STUDIO_MUL: stack[k-2] = stack[k-2] * stack[k-1]; k--; break;
			case STUDIO_DIV:
				if (stack[k-1] > 0.0001)
				{
					stack[k-2] = stack[k-2] / stack[k-1];
				}
				else
				{
					stack[k-2] = 0;
				}
				k--; 
				break;
			case STUDIO_CONST: stack[k] = pops->d.value; k++; break;
			case STUDIO_FETCH1: stack[k] = m_flexweight[pops->d.index]; k++; break;
			case STUDIO_FETCH2: stack[k] = g_flexdescweight[pops->d.index]; k++; break;
			}
			pops++;
		}
		g_flexdescweight[prule->flex] = stack[0];
	}
}


int StudioModel::FlexVerts( mstudiomesh_t *pmesh )
{
	// Gotta start at one here, since g_flexages defaults to 0
	static int flextag = 1;

	mstudioflex_t	*pflex = pmesh->pFlex( 0 );
	
	// apply flex weights
	int i, j, n;

	// manadatory to access correct verts
	SetCurrentModel();
	const mstudio_meshvertexdata_t *vertData = pmesh->GetVertexData();

	for (i = 0; i < pmesh->numflexes; i++)
	{
		float w = g_flexdescweight[pflex[i].flexdesc];

		if (w <= pflex[i].target0 || w >= pflex[i].target3)
		{
			// value outside of range
			continue;
		}
		else if (w < pflex[i].target1)
		{
			// 0 to 1 ramp
			w = (w - pflex[i].target0) / (pflex[i].target1 - pflex[i].target0);
		}
		else if (w > pflex[i].target2)
		{
			// 1 to 0 ramp
			w = (pflex[i].target3 - w) / (pflex[i].target3 - pflex[i].target2);
		}
		else
		{
			// plateau
			w = 1.0;
		}

		if (w > -0.001 && w < 0.001)
			continue;

		mstudiovertanim_t *pvanim = pflex[i].pVertanim( 0 );

		for (j = 0; j < pflex[i].numverts; j++)
		{
			n = pvanim[j].index;
			// only flex the indicies that are (still) part of this mesh
			if (n < (int)pmesh->numvertices)
			{
				if (g_flexages[n] < flextag)
				{
					g_flexages[n] = flextag;
					VectorCopy( *vertData->Position(n), g_flexedverts[n] );
					VectorCopy( *vertData->Normal(n), g_flexednorms[n] );
				}
				VectorMA( g_flexedverts[n], w, pvanim[j].delta, g_flexedverts[n] );
				VectorMA( g_flexednorms[n], w, pvanim[j].ndelta, g_flexednorms[n] );
			}
			else
			{
				n = 0;
			}
		}
	}
	return flextag++;
	// Con_DPrintf("\n" );
}


