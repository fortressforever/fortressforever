//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef IMATERIALSYSTEM_H
#define IMATERIALSYSTEM_H

#ifdef _WIN32
#pragma once
#endif

#define OVERBRIGHT 2.0f
#define OO_OVERBRIGHT ( 1.0f / 2.0f )
#define GAMMA 2.2f
#define TEXGAMMA 2.2f

#include "interface.h"
#include "vector.h"
#include "vector4d.h"
#include "vmatrix.h"
#include "appframework/IAppSystem.h"
#include "imageloader.h"
#include "texture_group_names.h"


//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------
class IMaterial;
class IMesh;
struct MaterialSystem_Config_t;
class VMatrix;
struct matrix3x4_t;
class ITexture;
struct MaterialSystemHardwareIdentifier_t;
class KeyValues;


//-----------------------------------------------------------------------------
// important enumeration
//-----------------------------------------------------------------------------

// NOTE NOTE NOTE!!!!  If you up this, grep for "NEW_INTERFACE" to see if there is anything
// waiting to be enabled during an interface revision.
#define MATERIAL_SYSTEM_INTERFACE_VERSION "VMaterialSystem076"

enum ShaderParamType_t 
{ 
	SHADER_PARAM_TYPE_TEXTURE, 
	SHADER_PARAM_TYPE_INTEGER,
	SHADER_PARAM_TYPE_COLOR,
	SHADER_PARAM_TYPE_VEC2,
	SHADER_PARAM_TYPE_VEC3,
	SHADER_PARAM_TYPE_VEC4,
	SHADER_PARAM_TYPE_ENVMAP,	// obsolete
	SHADER_PARAM_TYPE_FLOAT,
	SHADER_PARAM_TYPE_BOOL,
	SHADER_PARAM_TYPE_FOURCC,
	SHADER_PARAM_TYPE_MATRIX,
	SHADER_PARAM_TYPE_MATERIAL,
	SHADER_PARAM_TYPE_STRING,
};

enum MaterialMatrixMode_t
{
	MATERIAL_VIEW = 0,
	MATERIAL_PROJECTION,

	// Texture matrices
	MATERIAL_TEXTURE0,
	MATERIAL_TEXTURE1,
	MATERIAL_TEXTURE2,
	MATERIAL_TEXTURE3,
	MATERIAL_TEXTURE4,
	MATERIAL_TEXTURE5,
	MATERIAL_TEXTURE6,
	MATERIAL_TEXTURE7,

	MATERIAL_MODEL,

	// FIXME: How do I specify the actual number of matrix modes?
	NUM_MODEL_TRANSFORMS = 53,
	MATERIAL_MODEL_MAX = MATERIAL_MODEL + NUM_MODEL_TRANSFORMS, 

	// Total number of matrices
	NUM_MATRIX_MODES = MATERIAL_MODEL_MAX,

	// Number of texture transforms
	NUM_TEXTURE_TRANSFORMS = MATERIAL_TEXTURE7 - MATERIAL_TEXTURE0 + 1
};

#define MATERIAL_MODEL_MATRIX( _n ) (MaterialMatrixMode_t)(MATERIAL_MODEL + (_n))

enum MaterialPrimitiveType_t 
{ 
	MATERIAL_POINTS			= 0x0,
	MATERIAL_LINES,
	MATERIAL_TRIANGLES,
	MATERIAL_TRIANGLE_STRIP,
	MATERIAL_LINE_STRIP,
	MATERIAL_LINE_LOOP,	// a single line loop
	MATERIAL_POLYGON,	// this is a *single* polygon
	MATERIAL_QUADS,

	// This is used for static meshes that contain multiple types of
	// primitive types.	When calling draw, you'll need to specify
	// a primitive type.
	MATERIAL_HETEROGENOUS
};

enum MaterialPropertyTypes_t
{
	MATERIAL_PROPERTY_NEEDS_LIGHTMAP = 0,					// bool
	MATERIAL_PROPERTY_OPACITY,								// int (enum MaterialPropertyOpacityTypes_t)
	MATERIAL_PROPERTY_REFLECTIVITY,							// vec3_t
	MATERIAL_PROPERTY_NEEDS_BUMPED_LIGHTMAPS				// bool
};

// acceptable property values for MATERIAL_PROPERTY_OPACITY
enum MaterialPropertyOpacityTypes_t
{
	MATERIAL_ALPHATEST = 0,
	MATERIAL_OPAQUE,
	MATERIAL_TRANSLUCENT
};

enum MaterialBufferTypes_t
{
	MATERIAL_FRONT = 0,
	MATERIAL_BACK
};

enum MaterialCullMode_t
{
	MATERIAL_CULLMODE_CCW,	// this culls polygons with counterclockwise winding
	MATERIAL_CULLMODE_CW	// this culls polygons with clockwise winding
};

enum MaterialVertexFormat_t
{
	MATERIAL_VERTEX_FORMAT_MODEL,
	MATERIAL_VERTEX_FORMAT_COLOR,
};

enum MaterialFogMode_t
{
	MATERIAL_FOG_NONE,
	MATERIAL_FOG_LINEAR,
	MATERIAL_FOG_LINEAR_BELOW_FOG_Z,
};

enum MaterialHeightClipMode_t
{
	MATERIAL_HEIGHTCLIPMODE_DISABLE,
	MATERIAL_HEIGHTCLIPMODE_RENDER_ABOVE_HEIGHT,
	MATERIAL_HEIGHTCLIPMODE_RENDER_BELOW_HEIGHT
};

//-----------------------------------------------------------------------------
// Light structure
//-----------------------------------------------------------------------------

enum LightType_t
{
	MATERIAL_LIGHT_DISABLE = 0,
	MATERIAL_LIGHT_POINT,
	MATERIAL_LIGHT_DIRECTIONAL,
	MATERIAL_LIGHT_SPOT,
};

enum LightType_OptimizationFlags_t
{
	LIGHTTYPE_OPTIMIZATIONFLAGS_HAS_ATTENUATION0 = 1,
	LIGHTTYPE_OPTIMIZATIONFLAGS_HAS_ATTENUATION1 = 2,
	LIGHTTYPE_OPTIMIZATIONFLAGS_HAS_ATTENUATION2 = 4,
};

struct LightDesc_t 
{
    LightType_t		m_Type;
	Vector			m_Color;
    Vector	m_Position;
    Vector  m_Direction;
    float   m_Range;
    float   m_Falloff;
    float   m_Attenuation0;
    float   m_Attenuation1;
    float   m_Attenuation2;
    float   m_Theta;
    float   m_Phi;
	// These aren't used by DX8. . used for software lighting.
	float	m_ThetaDot;
	float	m_PhiDot;
	unsigned int	m_Flags;


	LightDesc_t() {}

private:
	// No copy constructors allowed
	LightDesc_t(const LightDesc_t& vOther);
};


//-----------------------------------------------------------------------------
// Standard lightmaps
//-----------------------------------------------------------------------------
enum StandardLightmap_t
{
	MATERIAL_SYSTEM_LIGHTMAP_PAGE_WHITE = -1,
	MATERIAL_SYSTEM_LIGHTMAP_PAGE_WHITE_BUMP = -2
};


struct MaterialSystem_SortInfo_t
{
	IMaterial *material;
	int lightmapPageID;
};


#define MAX_FB_TEXTURES 4

//-----------------------------------------------------------------------------
// Information about each adapter
//-----------------------------------------------------------------------------
enum
{
	MATERIAL_ADAPTER_NAME_LENGTH = 512
};

struct MaterialAdapterInfo_t
{
	char m_pDriverName[MATERIAL_ADAPTER_NAME_LENGTH];
	char m_pDriverDescription[MATERIAL_ADAPTER_NAME_LENGTH];
};

struct Material3DDriverInfo_t
{
	char m_pDriverName[MATERIAL_ADAPTER_NAME_LENGTH];
	char m_pDriverDescription[MATERIAL_ADAPTER_NAME_LENGTH];
	char m_pDriverVersion[MATERIAL_ADAPTER_NAME_LENGTH];
	unsigned int m_VendorID;
	unsigned int m_DeviceID;
	unsigned int m_SubSysID;
	unsigned int m_Revision;
	unsigned int m_WHQLLevel;

};

//-----------------------------------------------------------------------------
// Video mode info..
//-----------------------------------------------------------------------------

struct MaterialVideoMode_t
{
	int m_Width;			// if width and height are 0 and you select 
	int m_Height;			// windowed mode, it'll use the window size
	ImageFormat m_Format;	// use ImageFormats (ignored for windowed mode)
	int m_RefreshRate;		// 0 == default (ignored for windowed mode)
};

// fixme: should move this into something else.
struct FlashlightState_t
{
	Vector m_vecLightDirection; // FLASHLIGHTFIXME: can get this from the matrix
	Vector m_vecLightOrigin;  // FLASHLIGHTFIXME: can get this from the matrix
	float m_NearZ;
	float m_FarZ;
	float m_fHorizontalFOVDegrees;
	float m_fVerticalFOVDegrees;
	float m_fQuadraticAtten;
	float m_fLinearAtten;
	float m_fConstantAtten;
	Vector m_Color;
	ITexture *m_pSpotlightTexture;
	int m_nSpotlightTextureFrame;
};

//-----------------------------------------------------------------------------
// Flags to be used with the Init call
//-----------------------------------------------------------------------------
enum MaterialInitFlags_t
{
	MATERIAL_INIT_REFERENCE_RASTERIZER	= 0x4,
};

//-----------------------------------------------------------------------------
// Flags to specify type of depth buffer used with RT
//-----------------------------------------------------------------------------

// GR - this is to add RT with no depth buffer bound

enum MaterialRenderTargetDepth_t
{
	MATERIAL_RT_DEPTH_SHARED   = 0x0,
	MATERIAL_RT_DEPTH_SEPARATE = 0x1,
	MATERIAL_RT_DEPTH_NONE     = 0x2,
};

//-----------------------------------------------------------------------------
// A function to be called when we need to release all vertex buffers
// NOTE: The restore function will tell the caller if all the vertex formats
// changed so that it can flush caches, etc. if it needs to (for dxlevel support)
//-----------------------------------------------------------------------------
enum RestoreChangeFlags_t
{
	MATERIAL_RESTORE_VERTEX_FORMAT_CHANGED = 0x1,
};


// NOTE: All size modes will force the render target to be smaller than or equal to
// the size of the framebuffer.
enum RenderTargetSizeMode_t
{
	RT_SIZE_NO_CHANGE=0,			// Only allowed for render targets that don't want a depth buffer
									// (because if they have a depth buffer, the render target must be less than or equal to the size of the framebuffer).
	RT_SIZE_DEFAULT=1,				// Don't play with the specified width and height other than making sure it fits in the framebuffer.
	RT_SIZE_PICMIP=2,				// Apply picmip to the render target's width and height.
	RT_SIZE_HDR=3,					// CLAMP_BLUR_IMAGE_WIDTH( frame_buffer_width / 4 )
	RT_SIZE_FULL_FRAME_BUFFER=4		// Same size as frame buffer, or next lower power of 2 if we can't do that.
};


typedef void (*MaterialBufferReleaseFunc_t)( );
typedef void (*MaterialBufferRestoreFunc_t)( int nChangeFlags );	// see RestoreChangeFlags_t
typedef void (*ModeChangeCallbackFunc_t)( void );

typedef int VertexBufferHandle_t;
typedef unsigned short MaterialHandle_t;

typedef unsigned short OcclusionQueryObjectHandle_t;
#define INVALID_OCCLUSION_QUERY_OBJECT_HANDLE ( ( OcclusionQueryObjectHandle_t )~( ( OcclusionQueryObjectHandle_t )0 ) )

class IMaterialProxyFactory;
class ITexture;
class IMaterialSystemHardwareConfig;

class IMaterialSystem : public IAppSystem
{
public:
	// Call this to set an explicit shader version to use 
	// Must be called before Init().
	virtual void				SetShaderAPI( char const *pShaderAPIDLL ) = 0;

	// Must be called before Init(), if you're going to call it at all...
	virtual void				SetAdapter( int nAdapter, int nFlags ) = 0;

	// Call this to initialize the material system
	// returns a method to create interfaces in the shader dll
	virtual CreateInterfaceFn	Init( char const* pShaderAPIDLL, 
									  IMaterialProxyFactory *pMaterialProxyFactory,
									  CreateInterfaceFn fileSystemFactory,
									  CreateInterfaceFn cvarFactory=NULL ) = 0;

//	virtual void				Shutdown( ) = 0;

	virtual IMaterialSystemHardwareConfig *GetHardwareConfig( const char *pVersion, int *returnCode ) = 0;

	// Gets the number of adapters...
	virtual int					GetDisplayAdapterCount() const = 0;

	// Returns info about each adapter
	virtual void				GetDisplayAdapterInfo( int adapter, MaterialAdapterInfo_t& info ) const = 0;

	// Returns the number of modes
	virtual int					GetModeCount( int adapter ) const = 0;

	// Returns mode information..
	virtual void				GetModeInfo( int adapter, int mode, MaterialVideoMode_t& info ) const = 0;

	// Returns the mode info for the current display device
	virtual void				GetDisplayMode( MaterialVideoMode_t& mode ) const = 0;

	// Sets the mode...
	virtual bool				SetMode( void* hwnd, const MaterialSystem_Config_t &config ) = 0;
		
	// Get video card identitier
	virtual const MaterialSystemHardwareIdentifier_t &GetVideoCardIdentifier( void ) const = 0;
	
	// Creates/ destroys a child window
	virtual bool				AddView( void* hwnd ) = 0;
	virtual void				RemoveView( void* hwnd ) = 0;

	// Sets the view
	virtual void				SetView( void* hwnd ) = 0;

	virtual void				Get3DDriverInfo( Material3DDriverInfo_t& info ) const = 0;
	
	// return true if lightmaps need to be redownloaded
	// Call this before rendering each frame with the current config
	// for the material system.
	// Will do whatever is necessary to get the material system into the correct state
	// upon configuration change. .doesn't much else otherwise.
	virtual bool				UpdateConfig( bool bForceUpdate ) = 0;

	// Get the current config for this video card (as last set by UpdateConfig)
	virtual const MaterialSystem_Config_t &GetCurrentConfigForVideoCard() const = 0;

	// Force this to be the config; update all material system convars to match the state
	// return true if lightmaps need to be redownloaded
	virtual bool				OverrideConfig( const MaterialSystem_Config_t &config, bool bForceUpdate ) = 0;

	// This is the interface for knowing what materials are available
	// is to use the following functions to get a list of materials.  The
	// material names will have the full path to the material, and that is the 
	// only way that the directory structure of the materials will be seen through this
	// interface.
	// NOTE:  This is mostly for worldcraft to get a list of materials to put
	// in the "texture" browser.in Worldcraft
	virtual MaterialHandle_t	FirstMaterial() const = 0;

	// returns InvalidMaterial if there isn't another material.
	// WARNING: you must call GetNextMaterial until it returns NULL, 
	// otherwise there will be a memory leak.
	virtual MaterialHandle_t	NextMaterial( MaterialHandle_t h ) const = 0;

	// This is the invalid material
	virtual MaterialHandle_t	InvalidMaterial() const = 0;

	// Returns a particular material
	virtual IMaterial*			GetMaterial( MaterialHandle_t h ) const = 0;

	// Find a material by name.
	// The name of a material is a full path to 
	// the vmt file starting from "hl2/materials" (or equivalent) without
	// a file extension.
	// eg. "dev/dev_bumptest" refers to somethign similar to:
	// "d:/hl2/hl2/materials/dev/dev_bumptest.vmt"
	//
	// Most of the texture groups for pTextureGroupName are listed in texture_group_names.h.
	// 
	// Note: if the material can't be found, this returns a checkerboard material. You can 
	// find out if you have that material by calling IMaterial::IsErrorMaterial().
	// (Or use the global IsErrorMaterial function, which checks if it's null too).
	virtual IMaterial *			FindMaterial( char const* pMaterialName, const char *pTextureGroupName, bool complain = true, const char *pComplainPrefix = NULL ) = 0;

	virtual ITexture *			FindTexture( char const* pTextureName, const char *pTextureGroupName, bool complain = true ) = 0;
	virtual void				BindLocalCubemap( ITexture *pTexture ) = 0;
	
	// pass in an ITexture (that is build with "rendertarget" "1") or
	// pass in NULL for the regular backbuffer.
	virtual void				SetRenderTarget( ITexture *pTexture ) = 0;
	virtual ITexture *			GetRenderTarget( void ) = 0;
	
	virtual void				GetRenderTargetDimensions( int &width, int &height) const = 0;
	virtual void				GetBackBufferDimensions( int &width, int &height) const = 0;
	
	// Get the total number of materials in the system.  These aren't just the used
	// materials, but the complete collection.
	virtual int					GetNumMaterials( ) const = 0;

	// Remove any materials from memory that aren't in use as determined
	// by the IMaterial's reference count.
	virtual void				UncacheUnusedMaterials( bool bRecomputeStateSnapshots = false ) = 0;

	// uncache all materials. .  good for forcing reload of materials.
	virtual void				UncacheAllMaterials( ) = 0;

	// Load any materials into memory that are to be used as determined
	// by the IMaterial's reference count.
	virtual void				CacheUsedMaterials( ) = 0;
	
	// Force all textures to be reloaded from disk.
	virtual void				ReloadTextures( ) = 0;
	
	//
	// lightmap allocation stuff
	//

	// To allocate lightmaps, sort the whole world by material twice.
	// The first time through, call AllocateLightmap for every surface.
	// that has a lightmap.
	// The second time through, call AllocateWhiteLightmap for every 
	// surface that expects to use shaders that expect lightmaps.
	virtual void				BeginLightmapAllocation( ) = 0;
	// returns the sorting id for this surface
	virtual int 				AllocateLightmap( int width, int height, 
		                                          int offsetIntoLightmapPage[2],
												  IMaterial *pMaterial ) = 0;
	// returns the sorting id for this surface
	virtual int					AllocateWhiteLightmap( IMaterial *pMaterial ) = 0;
	virtual void				EndLightmapAllocation( ) = 0;

	// lightmaps are in linear color space
	// lightmapPageID is returned by GetLightmapPageIDForSortID
	// lightmapSize and offsetIntoLightmapPage are returned by AllocateLightmap.
	// You should never call UpdateLightmap for a lightmap allocated through
	// AllocateWhiteLightmap.
	virtual void				UpdateLightmap( int lightmapPageID, int lightmapSize[2],
												int offsetIntoLightmapPage[2], 
												float *pFloatImage, float *pFloatImageBump1,
												float *pFloatImageBump2, float *pFloatImageBump3 ) = 0;
	// Force the lightmaps updated with UpdateLightmap to be sent to the hardware.
	virtual void				FlushLightmaps( ) = 0;

	// fixme: could just be an array of ints for lightmapPageIDs since the material
	// for a surface is already known.
	virtual int					GetNumSortIDs( ) = 0;
//	virtual int					GetLightmapPageIDForSortID( int sortID ) = 0;
	virtual void				GetSortInfo( MaterialSystem_SortInfo_t *sortInfoArray ) = 0;

	virtual void				BeginFrame( ) = 0;
	virtual void				EndFrame( ) = 0;
	
	// Bind a material is current for rendering.
	virtual void				Bind( IMaterial *material, void *proxyData = 0 ) = 0;
	// Bind a lightmap page current for rendering.  You only have to 
	// do this for materials that require lightmaps.
	virtual void				BindLightmapPage( int lightmapPageID ) = 0;

	// inputs are between 0 and 1
	virtual void				DepthRange( float zNear, float zFar ) = 0;

	virtual void				ClearBuffers( bool bClearColor, bool bClearDepth, bool bClearStencil = false ) = 0;

	// read to a unsigned char rgb image.
	virtual void				ReadPixels( int x, int y, int width, int height, unsigned char *data, ImageFormat dstFormat ) = 0;

	// Sets lighting
	virtual void				SetAmbientLight( float r, float g, float b ) = 0;
	virtual void				SetLight( int lightNum, LightDesc_t& desc ) = 0;

	// The faces of the cube are specified in the same order as cubemap textures
	virtual void				SetAmbientLightCube( Vector4D cube[6] ) = 0;
	
	// Blit the backbuffer to the framebuffer texture
	virtual void				CopyRenderTargetToTexture( ITexture *pTexture ) = 0;
	
	// Set the current texture that is a copy of the framebuffer.
	virtual void				SetFrameBufferCopyTexture( ITexture *pTexture, int textureIndex = 0 ) = 0;
	virtual ITexture		   *GetFrameBufferCopyTexture( int textureIndex ) = 0;
	
	// Get the image format of the back buffer. . useful when creating render targets, etc.
	virtual ImageFormat			GetBackBufferFormat() const = 0;
	
	// do we need this?
	virtual void				Flush( bool flushHardware = false ) = 0;

	//
	// end vertex array api
	//

	//
	// Debugging tools
	//
	virtual void				DebugPrintUsedMaterials( const char *pSearchSubString, bool bVerbose ) = 0;
	virtual void				DebugPrintUsedTextures( void ) = 0;
	virtual void				ToggleSuppressMaterial( char const* pMaterialName ) = 0;
	virtual void				ToggleDebugMaterial( char const* pMaterialName ) = 0;

	// matrix api
	virtual void				MatrixMode( MaterialMatrixMode_t matrixMode ) = 0;
	virtual void				PushMatrix( void ) = 0;
	virtual void				PopMatrix( void ) = 0;
	virtual void				LoadMatrix( VMatrix const& matrix ) = 0;
	virtual void				LoadMatrix( matrix3x4_t const& matrix ) = 0;
	virtual void				MultMatrix( VMatrix const& matrix ) = 0;
	virtual void				MultMatrix( matrix3x4_t const& matrix ) = 0;
	virtual void				MultMatrixLocal( VMatrix const& matrix ) = 0;
	virtual void				MultMatrixLocal( matrix3x4_t const& matrix ) = 0;
	virtual void				GetMatrix( MaterialMatrixMode_t matrixMode, VMatrix *matrix ) = 0;
	virtual void				GetMatrix( MaterialMatrixMode_t matrixMode, matrix3x4_t *matrix ) = 0;
	virtual void				LoadIdentity( void ) = 0;
	virtual void				Ortho( double left, double top, double right, double bottom, double zNear, double zFar ) = 0;
	virtual void				PerspectiveX( double fovx, double aspect, double zNear, double zFar ) = 0;
	virtual void				PickMatrix( int x, int y, int width, int height ) = 0;
	virtual void				Rotate( float angle, float x, float y, float z ) = 0;
	virtual void				Translate( float x, float y, float z ) = 0;
	virtual void				Scale( float x, float y, float z ) = 0;
	// end matrix api

	// Sets/gets the viewport
	virtual void				Viewport( int x, int y, int width, int height ) = 0;
	virtual void				GetViewport( int& x, int& y, int& width, int& height ) const = 0;

	// The cull mode
	virtual void				CullMode( MaterialCullMode_t cullMode ) = 0;

	// end matrix api

	// This could easily be extended to a general user clip plane
	virtual void				SetHeightClipMode( MaterialHeightClipMode_t nHeightClipMode ) = 0;
	// garymcthack : fog z is always used for heightclipz for now.
	virtual void				SetHeightClipZ( float z ) = 0;
	
	// Fog methods...
	virtual void				FogMode( MaterialFogMode_t fogMode ) = 0;
	virtual void				FogStart( float fStart ) = 0;
	virtual void				FogEnd( float fEnd ) = 0;
	virtual void				SetFogZ( float fogZ ) = 0;
	virtual MaterialFogMode_t	GetFogMode( void ) = 0;

	virtual void				FogColor3f( float r, float g, float b ) = 0;
	virtual void				FogColor3fv( float const* rgb ) = 0;
	virtual void				FogColor3ub( unsigned char r, unsigned char g, unsigned char b ) = 0;
	virtual void				FogColor3ubv( unsigned char const* rgb ) = 0;

	virtual void				GetFogColor( unsigned char *rgb ) = 0;

	// Sets the number of bones for skinning
	virtual void				SetNumBoneWeights( int numBones ) = 0;

	virtual IMaterialProxyFactory *GetMaterialProxyFactory() = 0;
	
	// Read the page size of an existing lightmap by sort id (returned from AllocateLightmap())
	virtual void				GetLightmapPageSize( int lightmap, int *width, int *height ) const = 0;
	
	/// FIXME: This stuff needs to be cleaned up and abstracted.
	// Stuff that gets exported to the launcher through the engine
	virtual void				SwapBuffers( ) = 0;

	// Use this to spew information about the 3D layer 
	virtual void				SpewDriverInfo() const = 0;

	// Creates/destroys Mesh
	virtual IMesh* CreateStaticMesh( IMaterial* pMaterial, const char *pTextureBudgetGroup, bool bForceTempMesh = false ) = 0;
	virtual IMesh* CreateStaticMesh( MaterialVertexFormat_t fmt, const char *pTextureBudgetGroup, bool bSoftwareVertexShader ) = 0;
	virtual void DestroyStaticMesh( IMesh* mesh ) = 0;

	// Gets the dynamic mesh associated with the currently bound material
	// note that you've got to render the mesh before calling this function 
	// a second time. Clients should *not* call DestroyStaticMesh on the mesh 
	// returned by this call.
	// Use buffered = false if you want to not have the mesh be buffered,
	// but use it instead in the following pattern:
	//		meshBuilder.Begin
	//		meshBuilder.End
	//		Draw partial
	//		Draw partial
	//		Draw partial
	//		meshBuilder.Begin
	//		meshBuilder.End
	//		etc
	// Use Vertex or Index Override to supply a static vertex or index buffer
	// to use in place of the dynamic buffers.
	//
	// If you pass in a material in pAutoBind, it will automatically bind the
	// material. This can be helpful since you must bind the material you're
	// going to use BEFORE calling GetDynamicMesh.
	virtual IMesh* GetDynamicMesh( 
		bool buffered = true, 
		IMesh* pVertexOverride = 0,	
		IMesh* pIndexOverride = 0, 
		IMaterial *pAutoBind = 0 ) = 0;
		
	// Selection mode methods
	virtual int  SelectionMode( bool selectionMode ) = 0;
	virtual void SelectionBuffer( unsigned int* pBuffer, int size ) = 0;
	virtual void ClearSelectionNames( ) = 0;
	virtual void LoadSelectionName( int name ) = 0;
	virtual void PushSelectionName( int name ) = 0;
	virtual void PopSelectionName() = 0;
	
	// Installs a function to be called when we need to release vertex buffers + textures
	virtual void AddReleaseFunc( MaterialBufferReleaseFunc_t func ) = 0;
	virtual void RemoveReleaseFunc( MaterialBufferReleaseFunc_t func ) = 0;

	// Installs a function to be called when we need to restore vertex buffers
	virtual void AddRestoreFunc( MaterialBufferRestoreFunc_t func ) = 0;
	virtual void RemoveRestoreFunc( MaterialBufferRestoreFunc_t func ) = 0;

	// Reloads materials
	virtual void	ReloadMaterials( const char *pSubString = NULL ) = 0;

	virtual void	ResetMaterialLightmapPageInfo() = 0;

	// Sets the Clear Color for ClearBuffer....
	virtual void		ClearColor3ub( unsigned char r, unsigned char g, unsigned char b ) = 0;
	virtual void		ClearColor4ub( unsigned char r, unsigned char g, unsigned char b, unsigned char a ) = 0;

	// Force it to ignore Draw calls.
	virtual void		SetInStubMode( bool bInStubMode ) = 0;

	// Create a procedural material. The keyvalues looks like a VMT file
	virtual IMaterial	*CreateMaterial( const char *pMaterialName, KeyValues *pVMTKeyValues ) = 0;

	// Creates a render target
	// If depth == true, a depth buffer is also allocated. If not, then
	// the screen's depth buffer is used.
	virtual ITexture*	CreateRenderTargetTexture( 
		int w, 
		int h, 
		RenderTargetSizeMode_t sizeMode,	// Controls how size is generated (and regenerated on video mode change).
		ImageFormat format, 
		MaterialRenderTargetDepth_t depth = MATERIAL_RT_DEPTH_SHARED
		) = 0;

	// Creates a procedural texture
	virtual ITexture *CreateProceduralTexture( 
		const char *pTextureName, 
		const char *pTextureGroupName,
		int w, 
		int h, 
		ImageFormat fmt, 
		int nFlags ) = 0;

	// Allows us to override the depth buffer setting of a material
	virtual void	OverrideDepthEnable( bool bEnable, bool bDepthEnable ) = 0;

	// FIXME: This is a hack required for NVidia, can they fix in drivers?
	virtual void	DrawScreenSpaceQuad( IMaterial* pMaterial ) = 0;

	// Release temporary HW memory...
	virtual void	ReleaseTempTextureMemory() = 0;

	// GR - named RT
	virtual ITexture*	CreateNamedRenderTargetTexture( 
		const char *pRTName, 
		int w, 
		int h, 
		RenderTargetSizeMode_t sizeMode,	// Controls how size is generated (and regenerated on video mode change).
		ImageFormat format, 
		MaterialRenderTargetDepth_t depth = MATERIAL_RT_DEPTH_SHARED, 
		bool bClampTexCoords = true, 
		bool bAutoMipMap = false 
		) = 0;

	// For debugging and building recording files. This will stuff a token into the recording file,
	// then someone doing a playback can watch for the token.
	virtual void	SyncToken( const char *pToken ) = 0;

	// FIXME: REMOVE THIS FUNCTION!
	// The only reason why it's not gone is because we're a week from ship when I found the bug in it
	// and everything's tuned to use it.
	// It's returning values which are 2x too big (it's returning sphere diameter x2)
	// Use ComputePixelDiameterOfSphere below in all new code instead.
	virtual float	ComputePixelWidthOfSphere( const Vector& origin, float flRadius ) = 0;

	//
	// Occlusion query support
	//
	
	// Allocate and delete query objects.
	virtual OcclusionQueryObjectHandle_t CreateOcclusionQueryObject( void ) = 0;
	virtual void DestroyOcclusionQueryObject( OcclusionQueryObjectHandle_t ) = 0;

	// Bracket drawing with begin and end so that we can get counts next frame.
	virtual void BeginOcclusionQueryDrawing( OcclusionQueryObjectHandle_t ) = 0;
	virtual void EndOcclusionQueryDrawing( OcclusionQueryObjectHandle_t ) = 0;

	// Get the number of pixels rendered between begin and end on an earlier frame.
	// Calling this in the same frame is a huge perf hit!
	virtual int OcclusionQuery_GetNumPixelsRendered( OcclusionQueryObjectHandle_t ) = 0;

	virtual void SetFlashlightMode( bool bEnable ) = 0;

	virtual void SetFlashlightState( const FlashlightState_t &state, const VMatrix &worldToTexture ) = 0;

	virtual void SetModeChangeCallBack( ModeChangeCallbackFunc_t func ) = 0;

	// Gets *recommended* configuration information associated with the display card, 
	// given a particular dx level to run under. 
	// Use dxlevel 0 to use the recommended dx level.
	// The function returns false if an invalid dxlevel was specified

	// UNDONE: To find out all convars affected by configuration, we'll need to change
	// the dxsupport.pl program to output all column headers into a single keyvalue block
	// and then we would read that in, and send it back to the client
	virtual bool GetRecommendedConfigurationInfo( int nDXLevel, KeyValues * pKeyValues ) = 0;

	// Gets the current height clip mode
	virtual MaterialHeightClipMode_t GetHeightClipMode( ) = 0;

	// This returns the diameter of the sphere in pixels based on 
	// the current model, view, + projection matrices and viewport.
	virtual float	ComputePixelDiameterOfSphere( const Vector& vecAbsOrigin, float flRadius ) = 0;

	// By default, the material system applies the VIEW and PROJECTION matrices	to the user clip
	// planes (which are specified in world space) to generate projection-space user clip planes
	// Occasionally (for the particle system in hl2, for example), we want to override that
	// behavior and explictly specify a ViewProj transform for user clip planes
	virtual void	EnableUserClipTransformOverride( bool bEnable ) = 0;
	virtual void	UserClipTransform( const VMatrix &worldToView ) = 0;

	// Flushes managed textures from the texture cacher
	virtual void	EvictManagedResources() = 0;
};
  
extern IMaterialSystem *materials;

#endif // IMATERIALSYSTEM_H
