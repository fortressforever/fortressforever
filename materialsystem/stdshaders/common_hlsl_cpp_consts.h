//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifdef NV3X
  #define PSHADER_VECT_SCALE 20.0
  #define VSHADER_VECT_SCALE (1.0 / (PSHADER_VECT_SCALE) )
#else
  #define PSHADER_VECT_SCALE 1.0
  #define VSHADER_VECT_SCALE 1.0
#endif

// GR - HDR luminance maps to 0..n range
// IMPORTANT: Keep the same value as in materialsystem_global.h
#define MAX_HDR_OVERBRIGHT 16.0f


