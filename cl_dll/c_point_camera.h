//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_POINTCAMERA_H
#define C_POINTCAMERA_H
#ifdef _WIN32
#pragma once
#endif

#include "c_baseentity.h"
#include "basetypes.h"

class C_PointCamera : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_PointCamera, C_BaseEntity );
	DECLARE_CLIENTCLASS();

public:
	C_PointCamera();
	~C_PointCamera();

	bool IsActive();
	
	// C_BaseEntity.
	virtual bool	ShouldDraw();

	float			GetFOV();
	float			GetResolution();
	bool			IsFogEnabled();
	void			GetFogColor( unsigned char &r, unsigned char &g, unsigned char &b );
	float			GetFogStart();
	float			GetFogEnd();
	bool			UseScreenAspectRatio() const { return m_bUseScreenAspectRatio; }
	// Bug #0000390: multiple render targets for cameras
	string_t		GetRenderTarget( void ) { return m_szRenderTarget; }

private:
	float m_FOV;
	float m_Resolution;
	bool m_bFogEnable;
	color32 m_FogColor;
	float m_flFogStart;
	float m_flFogEnd;
	bool m_bActive;
	bool m_bUseScreenAspectRatio;
	// Bug #0000390: multiple render targets for cameras
	char m_szRenderTarget[256];

public:
	C_PointCamera	*m_pNext;
};

C_PointCamera *GetPointCameraList();

#endif // C_POINTCAMERA_H
