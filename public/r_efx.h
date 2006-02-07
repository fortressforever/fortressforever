//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#if !defined ( EFXH )
#define EFXH
#ifdef _WIN32
#pragma once
#endif

#include "iefx.h"

class IMaterial;
struct dlight_t;

class CVEfx : public IVEfx
{
public:
	int				Draw_DecalIndexFromName	( char *name );
	void			DecalShoot				( int textureIndex, int entity, const model_t *model, const Vector& model_origin, const QAngle& model_angles, const Vector& position, const Vector *saxis, int flags);
	void			DecalColorShoot			( int textureIndex, int entity, const model_t *model, const Vector& model_origin, const QAngle& model_angles, const Vector& position, const Vector *saxis, int flags, const color32 &rgbaColor);
	void			PlayerDecalShoot		( IMaterial *material, void *userdata, int entity, const model_t *model, const Vector& model_origin, const QAngle& model_angles, 
													const Vector& position, const Vector *saxis, int flags, const color32 &rgbaColor );
	dlight_t	*CL_AllocDlight				( int key );
	dlight_t	*CL_AllocElight				( int key );

	// Get a list of the currently-active dynamic lights.
	virtual int CL_GetActiveDLights( dlight_t *pList[MAX_DLIGHTS] );
};

extern CVEfx *g_pEfx;

#endif