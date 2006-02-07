//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef DISPINFO_LIGHTMAP_RESAMPLE_H
#define DISPINFO_LIGHTMAP_RESAMPLE_H
#ifdef _WIN32
#pragma once
#endif


class CCoreDispInfo;


// Resample vertex alpha into lightmap alpha for displacement surfaces so LOD popping artifacts are 
// less noticeable on the mid-to-high end.
void DispUpdateLightmapAlpha( 
	CCoreDispInfo **ppCoreDispInfos,
	int iDispInfo,		// The dispinfo (and dface_t) to generate the LM alpha for.
	float flPacifierMin,
	float flPacifierMax,	// For pacifier display.
	ddispinfo_t *pDisp,
	int lightmapWidth,
	int lightmapHeight );


#endif // DISPINFO_LIGHTMAP_RESAMPLE_H
