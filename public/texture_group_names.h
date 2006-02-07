//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef TEXTURE_GROUP_NAMES_H
#define TEXTURE_GROUP_NAMES_H
#ifdef _WIN32
#pragma once
#endif


// These are given to FindMaterial to reference the texture groups that show up on the 
#define TEXTURE_GROUP_LIGHTMAP				"Lightmaps"
#define TEXTURE_GROUP_WORLD					"World textures"
#define TEXTURE_GROUP_MODEL					"Model textures"
#define TEXTURE_GROUP_VGUI					"VGUI textures"
#define TEXTURE_GROUP_PARTICLE				"Particle textures"
#define TEXTURE_GROUP_DECAL					"Decal textures"
#define TEXTURE_GROUP_SKYBOX				"SkyBox textures"
#define TEXTURE_GROUP_CLIENT_EFFECTS		"ClientEffect textures"
#define TEXTURE_GROUP_OTHER					"Other textures"
#define TEXTURE_GROUP_PRECACHED				"Precached" // TODO: assign texture groups to the precached materials
#define TEXTURE_GROUP_CUBE_MAP				"CubeMap textures"
#define TEXTURE_GROUP_RENDER_TARGET			"RenderTargets"
#define TEXTURE_GROUP_UNACCOUNTED			"Unaccounted textures"	// Textures that weren't assigned a texture group.
//#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER	"StaticVertex"
#define TEXTURE_GROUP_STATIC_INDEX_BUFFER	"Static Indices"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_DISP	"Displacement Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_COLOR	"Lighting Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_WORLD	"World Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_MODELS	"Model Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_OTHER	"Other Verts"

// Groups I haven't used yet.
#define TEXTURE_GROUP_DEPTH_BUFFER			"DepthBuffer"
#define TEXTURE_GROUP_VIEW_MODEL			"ViewModel"


#endif // TEXTURE_GROUP_NAMES_H
