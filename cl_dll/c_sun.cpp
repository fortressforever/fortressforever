//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_sun.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT_NOBASE( C_Sun, DT_Sun, CSun )
	
	RecvPropInt( RECVINFO(m_clrRender), 0, RecvProxy_IntToColor32 ),
	RecvPropVector( RECVINFO( m_vDirection ) ),
	RecvPropInt( RECVINFO( m_bOn ) ),
	RecvPropInt( RECVINFO( m_nSize ) ),

END_RECV_TABLE()

C_Sun::C_Sun()
{
	m_Overlay.m_bDirectional = true;
	m_Overlay.m_bInSky = true;

	m_GlowOverlay.m_bDirectional = true;
	m_GlowOverlay.m_bInSky = true;
}


C_Sun::~C_Sun()
{
}


void C_Sun::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	// We have to do special setup on our colors because we're tinting an additive material.
	// If we don't have at least one component at full strength, the luminosity of the material
	// will change and that will cause the material to become more translucent  This would be incorrect
	// for the sun, which should always be completely opaque at its core.  Here, we renormalize the
	// components to make sure only hue is altered.

	float maxComponent = max ( m_clrRender->r, max ( m_clrRender->g, m_clrRender->b ) );

	Vector vColor;

	// Re-normalize the color ranges
	if ( maxComponent <= 0.0f )
	{
		// This is an error, set to pure white
		vColor.Init( 1.0f, 1.0f, 1.0f );
	}
	else
	{
		vColor.x = m_clrRender->r / maxComponent;
		vColor.y = m_clrRender->g / maxComponent;
		vColor.z = m_clrRender->b / maxComponent;
	}
	
	// Setup the core overlay
	m_Overlay.m_vDirection = m_vDirection;
	m_Overlay.m_nSprites = 1;

	m_Overlay.m_Sprites[0].m_vColor = vColor;
	m_Overlay.m_Sprites[0].m_flHorzSize = m_nSize * 2.0f;
	m_Overlay.m_Sprites[0].m_flVertSize = m_nSize * 2.0f;

	m_Overlay.m_Sprites[0].m_pMaterial = materials->FindMaterial( "sprites/light_glow02_add_noz", TEXTURE_GROUP_OTHER );
	m_Overlay.m_flProxyRadius = 0.05f; // about 1/20th of the screen

	// Setup the external glow overlay
	m_GlowOverlay.m_vDirection = m_vDirection;
	m_GlowOverlay.m_nSprites = 1;

	m_GlowOverlay.m_Sprites[0].m_vColor = vColor;
	m_GlowOverlay.m_Sprites[0].m_flHorzSize = m_nSize;
	m_GlowOverlay.m_Sprites[0].m_flVertSize = m_nSize;
	m_GlowOverlay.m_Sprites[0].m_pMaterial = materials->FindMaterial( "sprites/light_glow02_add_noz", TEXTURE_GROUP_OTHER );

	// This texture will fade away as the dot between camera and sun changes
	m_GlowOverlay.SetModulateByDot();
	m_GlowOverlay.m_flProxyRadius = 0.05f; // about 1/20th of the screen
	
	// Either activate or deactivate.
	if ( m_bOn )
	{
		m_Overlay.Activate();
		m_GlowOverlay.Activate();
	}
	else
	{
		m_Overlay.Deactivate();
		m_GlowOverlay.Deactivate();
	}
}



