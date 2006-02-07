//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
// updates:
// 1-4-99	fixed file texture load and file read bug

////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "StudioModel.h"
#include "vphysics/constraints.h"
#include "physmesh.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"
#include "ViewerSettings.h"
#include "bone_setup.h"
#include "UtlMemory.h"
#include "mx/mx.h"
#include "cmdlib.h"
#include "IStudioRender.h"
#include "materialsystem/IMaterialSystemHardwareConfig.h"
#include "MDLViewer.h"
#include "optimize.h"

extern IMaterialSystem *g_pMaterialSystem;
extern IMaterialSystemHardwareConfig *g_pMaterialSystemHardwareConfig;
extern char g_appTitle[];
IStudioRender	*StudioModel::m_pStudioRender;
Vector		    *StudioModel::m_AmbientLightColors;

#pragma warning( disable : 4244 ) // double to float


static StudioModel g_studioModel;

// Expose it to the rest of the app
StudioModel *g_pStudioModel = &g_studioModel;
StudioModel *g_pStudioExtraModel[4];

////////////////////////////////////////////////////////////////////////

class CStudioDataCache : public IStudioDataCache
{
public:
	bool VerifyHeaders( studiohdr_t *pStudioHdr );
	vertexFileHeader_t *CacheVertexData( studiohdr_t *pStudioHdr );
	OptimizedModel::FileHeader_t *CacheIndexData( studiohdr_t *pStudioHdr );
//	studiohwdata_t *CacheHWData( studiohdr_t *pStudioHdr );
};

static CStudioDataCache	g_StudioDataCache;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CStudioDataCache, IStudioDataCache, STUDIO_DATA_CACHE_INTERFACE_VERSION, g_StudioDataCache );

static StudioModel *g_pActiveModel;
void StudioModel::SetCurrentModel()
{
	// track the correct model
	g_pActiveModel = this;
}

/*
=================
VerifyHeaders

Minimal presence and header validation, no data loads
Return true if successful, false otherwise.
=================
*/
bool CStudioDataCache::VerifyHeaders( studiohdr_t *pStudioHdr )
{
	vertexFileHeader_t				vertexHdr;
	OptimizedModel::FileHeader_t	vtxHdr;
	char							fileName[260];
	FileHandle_t fp;

	if (!pStudioHdr->numbodyparts)
	{
		// model has no vertex data
		return true;
	}

	// load the VVD file
	// use model name for correct path
	Q_StripExtension( g_pActiveModel->GetFileName(), fileName, sizeof( fileName ) );
	strcat( fileName, ".vvd" );

	// load header only
	if ( (fp = g_pFileSystem->Open( fileName, "rb" )) == NULL)
	{
		return false;
	}
	g_pFileSystem->Read( &vertexHdr, sizeof(vertexFileHeader_t), fp );
	g_pFileSystem->Close( fp );

	// check
	if (( vertexHdr.id != MODEL_VERTEX_FILE_ID ) ||
		( vertexHdr.version != MODEL_VERTEX_FILE_VERSION ) ||
		( vertexHdr.checksum != pStudioHdr->checksum ))
	{
		return false;
	}

	// load the VTX file
	// use model name for correct path
	const char *pExtension;
	if ( g_pMaterialSystemHardwareConfig->GetDXSupportLevel() >= 90 )
	{
		pExtension = ".dx90.vtx";
	}
	else if ( g_pMaterialSystemHardwareConfig->GetDXSupportLevel() >= 80 )
	{
		pExtension = ".dx80.vtx";
	}
	else
	{
		pExtension = ".sw.vtx";
	}
	Q_StripExtension( g_pActiveModel->GetFileName(), fileName, sizeof( fileName ) );
	strcat( fileName, pExtension );

	// load header only
	if( (fp = g_pFileSystem->Open( fileName, "rb" )) == NULL)
	{
		return false;
	}
	g_pFileSystem->Read( &vtxHdr, sizeof(OptimizedModel::FileHeader_t), fp );
	g_pFileSystem->Close( fp );

	// check
	if (( vtxHdr.version != OPTIMIZED_MODEL_FILE_VERSION ) ||
		( vtxHdr.checkSum != pStudioHdr->checksum ))
	{
		return false;
	}

	// valid
	return true;
}

/*
=================
CacheVertexData

Cache model's specified dynamic data
=================
*/

inline void *Align32( void* pAddr )
{
	return (void*)( ((int)pAddr + 0x1F) & (~0x1F));
}

vertexFileHeader_t *CStudioDataCache::CacheVertexData( studiohdr_t *pStudioHdr )
{
	vertexFileHeader_t	*pVvdHdr;
	vertexFileHeader_t	*pNewVvdHdr;
	void				*pAllocBase;
	char				fileName[260];
	FileHandle_t fp;
	int					size;

	Assert( pStudioHdr );

	// get the persisted data
	if (pStudioHdr->pVertexBase)
	{
		// 32-byte align it
		return (vertexFileHeader_t *)Align32( pStudioHdr->pVertexBase );
	}								  

	// load and persist the vertex file
	// use model name for correct path
	Q_StripExtension( g_pActiveModel->GetFileName(), fileName, sizeof( fileName ) );
	strcat( fileName, ".vvd" );

	// load the file
	if ( (fp = g_pFileSystem->Open( fileName, "rb" )) == NULL)
	{
		Error( "Error opening Vertex File '%s'\n", fileName );
	}

	size = g_pFileSystem->Size( fp );
	pAllocBase = malloc( size + 31 );
	if (!pAllocBase)
	{
		g_pFileSystem->Close(fp);
		Error( "Error allocating %d bytes for Vertex File '%s'\n", size, fileName );
	}
	pVvdHdr = (vertexFileHeader_t *)Align32( pAllocBase );

	g_pFileSystem->Read( pVvdHdr, size, fp );
	g_pFileSystem->Close( fp );

	// check header
	if ( pVvdHdr->id != MODEL_VERTEX_FILE_ID )
	{
		Error( "Error Vertex File '%s' id %d should be %d\n", fileName, pVvdHdr->id, MODEL_VERTEX_FILE_ID );
	}
	if ( pVvdHdr->version != MODEL_VERTEX_FILE_VERSION )
	{
		Error( "Error Vertex File '%s' version %d should be %d\n", fileName, pVvdHdr->version, MODEL_VERTEX_FILE_VERSION );
	}
	if ( pVvdHdr->checksum != pStudioHdr->checksum )
	{
		Error( "Error Vertex File '%s' checksum %d should be %d\n", fileName, pVvdHdr->checksum, pStudioHdr->checksum );
	}

	if (pVvdHdr->numFixups)
	{
		// need to perform mesh relocation fixups
		// allocate a new copy
		void *pNewAlloc = malloc( size + 31 );
		if (!pNewAlloc)
		{
			Error( "Error allocating %d bytes for Vertex File '%s'\n", size, fileName );
		}
 		pNewVvdHdr = (vertexFileHeader_t *)Align32( pNewAlloc );

		Studio_LoadVertexes( pVvdHdr, pNewVvdHdr, 0, true );

		// discard original
		free( pAllocBase );

		pAllocBase = pNewAlloc;
		pVvdHdr = pNewVvdHdr;
	}
	
	pStudioHdr->pVertexBase = pAllocBase;
	
	return pVvdHdr;
}

/*
=================
CacheIndexData

Cache model's specified dynamic data
=================
*/
OptimizedModel::FileHeader_t *CStudioDataCache::CacheIndexData( studiohdr_t *pStudioHdr )
{
	OptimizedModel::FileHeader_t	*pVtxHdr;
	char							fileName[260];
	FileHandle_t fp;
	int								size;

	Assert( pStudioHdr );

	// get the persisted data
	if (pStudioHdr->pIndexBase)
	{
		return (OptimizedModel::FileHeader_t *)pStudioHdr->pIndexBase;
	}

	// load and persist the vtx file
	char* pExtension;
	if ( g_pMaterialSystemHardwareConfig->GetDXSupportLevel() >= 90 )
	{
		pExtension = ".dx80.vtx";
	}
	else if ( g_pMaterialSystemHardwareConfig->GetDXSupportLevel() >= 80 )
	{
		pExtension = ".dx80.vtx";
	}
	else
	{
		pExtension = ".sw.vtx";
	}
	// use model name for correct path
	Q_StripExtension( g_pActiveModel->GetFileName(), fileName, sizeof( fileName ) );
	strcat( fileName, pExtension );

	// load the file
	if( (fp = g_pFileSystem->Open( fileName, "rb" )) == NULL)
	{
		//Error( "Error opening Vtx File '%s'\n", fileName );
		return 0;
	}
	size = g_pFileSystem->Size( fp );
	pVtxHdr = (OptimizedModel::FileHeader_t *)malloc( size );
	if (!pVtxHdr)
	{
		g_pFileSystem->Close(fp);
		Error( "Error allocating %d bytes for Vtx File '%s'\n", size, fileName );
	}

	g_pFileSystem->Read( pVtxHdr, size, fp );
	g_pFileSystem->Close( fp );

	// check version
	if ( pVtxHdr->version != OPTIMIZED_MODEL_FILE_VERSION )
	{
		Error( "Error Vtx File '%s' version %d should be %d\n", fileName, pVtxHdr->version, OPTIMIZED_MODEL_FILE_VERSION );
	}

	// verify checksum
	if ( pVtxHdr->checkSum != pStudioHdr->checksum )
	{
		Error( "Error Vtx File '%s' checksum %d should be %d\n", fileName, pVtxHdr->checkSum, pStudioHdr->checksum );
	}

	pStudioHdr->pIndexBase = (void *)pVtxHdr;
	return pVtxHdr;
}

void StudioModel::Init()
{
	// Load up the IStudioRender interface
	extern CreateInterfaceFn g_MaterialSystemClientFactory;
	extern CreateInterfaceFn g_MaterialSystemFactory;

	CSysModule *studioRenderDLL = NULL;

	studioRenderDLL = g_pFullFileSystem->LoadModule( "StudioRender.dll" );
	if( !studioRenderDLL )
	{
//		Msg( mwWarning, "Can't load StudioRender.dll\n" );
		Assert( 0 ); // garymcthack
		return;
	}
	CreateInterfaceFn studioRenderFactory = Sys_GetFactory( studioRenderDLL );
	if (!studioRenderFactory )
	{
//		Msg( mwWarning, "Can't get studio render factory\n" );
		Assert( 0 ); // garymcthack
		return;
	}
	m_pStudioRender = ( IStudioRender * )studioRenderFactory( STUDIO_RENDER_INTERFACE_VERSION, NULL );
	if (!m_pStudioRender)
	{
//		Msg( mwWarning, "Can't get version %s of StudioRender.dll\n", STUDIO_RENDER_INTERFACE_VERSION );
		Assert( 0 ); // garymcthack
		return;
	}

	if( !m_pStudioRender->Init( g_MaterialSystemFactory, g_MaterialSystemClientFactory, g_MaterialSystemClientFactory, Sys_GetFactoryThis() ) )
	{
//		Msg( mwWarning, "Can't initialize StudioRender.dll\n" );
		Assert( 0 ); // garymcthack
		m_pStudioRender = NULL;
	}

	m_AmbientLightColors = new Vector[m_pStudioRender->GetNumAmbientLightSamples()];
	UpdateStudioRenderConfig( g_viewerSettings.renderMode == RM_FLATSHADED ||
			g_viewerSettings.renderMode == RM_SMOOTHSHADED, 
			g_viewerSettings.renderMode == RM_WIREFRAME, 
			g_viewerSettings.showNormals ); // garymcthack - should really only do this once a frame and at init time.
}

void StudioModel::Shutdown( void )
{
	g_pStudioModel->FreeModel();
	if( m_pStudioRender )
	{
		m_pStudioRender->Shutdown();
		m_pStudioRender = NULL;
	}
	delete [] m_AmbientLightColors;

	UnloadGroupFiles();
}


void StudioModel::ReleaseStudioModel()
{
	g_pStudioModel->FreeModel(); 
}

void StudioModel::RestoreStudioModel()
{
	if (g_pStudioModel->LoadModel(g_pStudioModel->m_pModelName))
	{
		g_pStudioModel->PostLoadModel( g_pStudioModel->m_pModelName );
	}
}



//-----------------------------------------------------------------------------
// Purpose: Frees the model data and releases textures from OpenGL.
//-----------------------------------------------------------------------------
void StudioModel::FreeModel ()
{
	g_pCacheHdr = NULL;

	if (!m_pStudioRender)
		return;

	m_pStudioRender->UnloadModel( &m_HardwareData );
	
	if (m_pstudiohdr && m_pstudiohdr->pVertexBase)
		free( m_pstudiohdr->pVertexBase );

	if (m_pstudiohdr && m_pstudiohdr->pIndexBase)
		free( m_pstudiohdr->pIndexBase );

	if (m_pstudiohdr)
		free( m_pstudiohdr );
	m_pstudiohdr = 0;

	int i;
	for (i = 0; i < 32; i++)
	{
		if (m_panimhdr[i])
		{
			free( m_panimhdr[i] );
			m_panimhdr[i] = 0;
		}
	}

#if 0
	// deleting textures
	g_texnum -= 3;
	int textures[MAXSTUDIOSKINS];
	for (i = 0; i < g_texnum; i++)
		textures[i] = i + 3;

	//glDeleteTextures (g_texnum, (const GLuint *) textures);
	g_texnum = 3;
#endif

	memset( &m_HardwareData, 0, sizeof( m_HardwareData ) );

	m_SurfaceProps.Purge();
	// BUG: Jay, when I call this it crashes
	// delete m_pPhysics;
}

void *StudioModel::operator new( size_t stAllocateBlock )
{
	// call into engine to get memory
	Assert( stAllocateBlock != 0 );
	return calloc( 1, stAllocateBlock );
}

void StudioModel::operator delete( void *pMem )
{
#ifdef _DEBUG
	// set the memory to a known value
	int size = _msize( pMem );
	memset( pMem, 0xcd, size );
#endif

	// get the engine to free the memory
	free( pMem );
}

bool StudioModel::LoadModel( const char *modelname )
{
	FileHandle_t fp;
	long size;
	void *buffer;

	if (!modelname)
		return 0;

	// In the case of restore, m_pModelName == modelname
	if (m_pModelName != modelname)
	{
		// Copy over the model name; we'll need it later...
		if (m_pModelName)
			delete[] m_pModelName;
		m_pModelName = new char[strlen(modelname) + 1];
		strcpy( m_pModelName, modelname );
	}

	// load the model
	if( (fp = g_pFullFileSystem->Open( modelname, "rb" )) == NULL)
		return 0;

	size = g_pFullFileSystem->Size( fp );

	buffer = malloc( size );
	if (!buffer)
	{
		g_pFullFileSystem->Close(fp);
		return 0;
	}

	g_pFullFileSystem->Read( buffer, size, fp );
	g_pFullFileSystem->Close( fp );

	byte				*pin;
	studiohdr_t			*phdr;

	pin = (byte *)buffer;
	phdr = (studiohdr_t *)pin;

	Studio_ConvertStudioHdrToNewVersion( phdr );

	if (strncmp ((const char *) buffer, "IDST", 4) &&
		strncmp ((const char *) buffer, "IDAG", 4))
	{
		free (buffer);
		return 0;
	}

	if (!strncmp ((const char *) buffer, "IDAG", 4) && !m_pstudiohdr)
	{
		free (buffer);
		return 0;
	}

	Studio_ConvertStudioHdrToNewVersion( phdr );

	if ( phdr->version != STUDIO_VERSION )
	{
		free( buffer );
		return 0;
	}

	// check the model's peer files
	// manadatory to access correct verts
	SetCurrentModel();
	if (!g_StudioDataCache.VerifyHeaders( (studiohdr_t *)buffer ))
	{
		// early error before model finalizes setup
		// no current support for specific error reason
		free( buffer );
		return 0;
	}

	if (!m_pstudiohdr)
		m_pstudiohdr = (studiohdr_t *)buffer;

	if ( !m_pStudioRender->LoadModel( m_pstudiohdr, &m_HardwareData ) )
	{
		// garymcthack - need to spew an error
//		Msg( mwWarning, "error loading model: %s\n", modelname );
		free( buffer );
		m_pstudiohdr = NULL;
		return false;
	}

	m_pPhysics = LoadPhysics( m_pstudiohdr, modelname );

	// Copy over all of the hitboxes; we may add and remove elements
	m_HitboxSets.RemoveAll();

	int i;
	int s;
	for ( s = 0; s < m_pstudiohdr->numhitboxsets; s++ )
	{
		mstudiohitboxset_t *set = m_pstudiohdr->pHitboxSet( s );
		if ( !set )
			continue;

		m_HitboxSets.AddToTail();

		for ( i = 0; i < set->numhitboxes; ++i )
		{
			mstudiobbox_t *pHit = set->pHitbox(i);
			int nIndex = m_HitboxSets[ s ].AddToTail( );
			m_HitboxSets[s][nIndex] = *set->pHitbox(i);
		}

		// Set the name
		hbsetname_s *n = &m_HitboxSetNames[ m_HitboxSetNames.AddToTail() ];
		strcpy( n->name, set->pszName() );
	}

	// Copy over all of the surface props; we may change them...
	for ( i = 0; i < m_pstudiohdr->numbones; ++i )
	{
		mstudiobone_t* pBone = m_pstudiohdr->pBone(i);

		CUtlSymbol prop( pBone->pszSurfaceProp() );
		m_SurfaceProps.AddToTail( prop );
	}

	m_physPreviewBone = -1;

	bool forceOpaque = (m_pstudiohdr->flags & STUDIOHDR_FLAGS_FORCE_OPAQUE) != 0;
	bool translucentTwoPass = (m_pstudiohdr->flags & STUDIOHDR_FLAGS_TRANSLUCENT_TWOPASS) ? true : false;
	bool vertexLit = false;
	m_bIsTransparent = false;
	m_bHasProxy = false;

	for( int lodID = m_HardwareData.m_RootLOD; lodID < m_HardwareData.m_NumLODs; lodID++ )
	{
		studioloddata_t *pLODData = &m_HardwareData.m_pLODs[lodID];
		for ( i = 0; i < pLODData->numMaterials; ++i )
		{
			if (pLODData->ppMaterials[i]->IsVertexLit())
			{
				vertexLit = true;
			}
			if ((!forceOpaque) && pLODData->ppMaterials[i]->IsTranslucent())
			{
				m_bIsTransparent = true;
				//Msg("Translucent material %s for model %s\n", pLODData->ppMaterials[i]->GetName(), m_pstudiohdr->name );
			}
			if (pLODData->ppMaterials[i]->HasProxy())
			{
				m_bHasProxy = true;
			}
		}
	}

	return true;
}



bool StudioModel::PostLoadModel( const char *modelname )
{
	if (m_pstudiohdr == NULL)
	{
		return(false);
	}

	SetSequence (0);
	SetController (0, 0.0f);
	SetController (1, 0.0f);
	SetController (2, 0.0f);
	SetController (3, 0.0f);
	SetBlendTime( DEFAULT_BLEND_TIME );
	// SetHeadTurn( 1.0f );  // FIXME:!!!

	int n;
	for (n = 0; n < m_pstudiohdr->numbodyparts; n++)
		SetBodygroup (n, 0);

	SetSkin (0);

/*
	Vector mins, maxs;
	ExtractBbox (mins, maxs);
	if (mins[2] < 5.0f)
		m_origin[2] = -mins[2];
*/
	return true;
}

////////////////////////////////////////////////////////////////////////


int StudioModel::GetSequence( )
{
	return m_sequence;
}

int StudioModel::SetSequence( int iSequence )
{
	if ( !m_pstudiohdr )
		return 0;

	if (iSequence < 0)
		return 0;

	if (iSequence > m_pstudiohdr->GetNumSeq())
		return m_sequence;

	m_prevsequence = m_sequence;
	m_sequence = iSequence;
	m_cycle = 0;
	m_sequencetime = 0.0;

	return m_sequence;
}

void StudioModel::ClearOverlaysSequences( void )
{
	ClearAnimationLayers( );
	memset( m_Layer, 0, sizeof( m_Layer ) );
}

void StudioModel::ClearAnimationLayers( void )
{
	m_iActiveLayers = 0;
}

int	StudioModel::GetNewAnimationLayer( int iPriority )
{
	if ( !m_pstudiohdr )
		return 0;

	if ( m_iActiveLayers >= MAXSTUDIOANIMLAYERS )
	{
		Assert( 0 );
		return MAXSTUDIOANIMLAYERS - 1;
	}

	m_Layer[m_iActiveLayers].m_priority = iPriority;

	return m_iActiveLayers++;
}

int StudioModel::SetOverlaySequence( int iLayer, int iSequence, float flWeight )
{
	if ( !m_pstudiohdr )
		return 0;

	if (iSequence < 0)
		return 0;

	if (iLayer < 0 || iLayer >= MAXSTUDIOANIMLAYERS)
	{
		Assert(0);
		return 0;
	}

	if (iSequence > m_pstudiohdr->GetNumSeq())
		return m_Layer[iLayer].m_sequence;

	m_Layer[iLayer].m_sequence = iSequence;
	m_Layer[iLayer].m_weight = flWeight;
	m_Layer[iLayer].m_playbackrate = 1.0;

	return iSequence;
}

float StudioModel::SetOverlayRate( int iLayer, float flCycle, float flPlaybackRate )
{
	if (iLayer >= 0 && iLayer < MAXSTUDIOANIMLAYERS)
	{
		m_Layer[iLayer].m_cycle = flCycle;
		m_Layer[iLayer].m_playbackrate = flPlaybackRate;
	}
	return flCycle;
}


int StudioModel::GetOverlaySequence( int iLayer )
{
	if ( !m_pstudiohdr )
		return 0;

	if (iLayer < 0 || iLayer >= MAXSTUDIOANIMLAYERS)
	{
		Assert(0);
		return 0;
	}

	return m_Layer[iLayer].m_sequence;
}


float StudioModel::GetOverlaySequenceWeight( int iLayer )
{
	if ( !m_pstudiohdr )
		return 0;

	if (iLayer < 0 || iLayer >= MAXSTUDIOANIMLAYERS)
	{
		Assert(0);
		return 0;
	}

	return m_Layer[iLayer].m_weight;
}


int StudioModel::LookupSequence( const char *szSequence )
{
	int i;

	if ( !m_pstudiohdr )
		return -1;

	for (i = 0; i < m_pstudiohdr->GetNumSeq(); i++)
	{
		if (!stricmp( szSequence, m_pstudiohdr->pSeqdesc( i ).pszLabel() ))
		{
			return i;
		}
	}
	return -1;
}

int StudioModel::SetSequence( const char *szSequence )
{
	return SetSequence( LookupSequence( szSequence ) );
}

void StudioModel::StartBlending( void )
{
	// Switch back to old sequence ( this will oscillate between this one and the last one )
	SetSequence( m_prevsequence );
}

void StudioModel::SetBlendTime( float blendtime )
{
	if ( blendtime > 0.0f )
	{
		m_blendtime = blendtime;
	}
}

float StudioModel::GetTransitionAmount( void )
{
	if ( g_viewerSettings.blendSequenceChanges &&
		m_sequencetime < m_blendtime && m_prevsequence != m_sequence )
	{
		float s;
		s = ( m_sequencetime / m_blendtime );
		return s;
	}

	return 0.0f;
}

int StudioModel::LookupFlexController( char *szName )
{
	if (!m_pstudiohdr)
		return false;

	for (int iFlex = 0; iFlex < m_pstudiohdr->numflexcontrollers; iFlex++)
	{
		if (stricmp( szName, m_pstudiohdr->pFlexcontroller( iFlex )->pszName() ) == 0)
		{
			return iFlex;
		}
	}
	return -1;
}


void StudioModel::SetFlexController( char *szName, float flValue )
{
	SetFlexController( LookupFlexController( szName ), flValue );
}

void StudioModel::SetFlexController( int iFlex, float flValue )
{
	if ( !m_pstudiohdr )
		return;

	if (iFlex >= 0 && iFlex < m_pstudiohdr->numflexcontrollers)
	{
		mstudioflexcontroller_t *pflex = m_pstudiohdr->pFlexcontroller(iFlex);

		if (pflex->min != pflex->max)
		{
			flValue = (flValue - pflex->min) / (pflex->max - pflex->min);
		}
		m_flexweight[iFlex] = clamp( flValue, 0.0f, 1.0f );
	}
}


void StudioModel::SetFlexControllerRaw( int iFlex, float flValue )
{
	if ( !m_pstudiohdr )
		return;

	if (iFlex >= 0 && iFlex < m_pstudiohdr->numflexcontrollers)
	{
		mstudioflexcontroller_t *pflex = m_pstudiohdr->pFlexcontroller(iFlex);
		m_flexweight[iFlex] = clamp( flValue, 0.0f, 1.0f );
	}
}

float StudioModel::GetFlexController( char *szName )
{
	return GetFlexController( LookupFlexController( szName ) );
}

float StudioModel::GetFlexController( int iFlex )
{
	if ( !m_pstudiohdr )
		return 0.0f;

	if (iFlex >= 0 && iFlex < m_pstudiohdr->numflexcontrollers)
	{
		mstudioflexcontroller_t *pflex = m_pstudiohdr->pFlexcontroller(iFlex);

		float flValue = m_flexweight[iFlex];

		if (pflex->min != pflex->max)
		{
			flValue = flValue * (pflex->max - pflex->min) + pflex->min;
		}
		return flValue;
	}
	return 0.0;
}


float StudioModel::GetFlexControllerRaw( int iFlex )
{
	if ( !m_pstudiohdr )
		return 0.0f;

	if (iFlex >= 0 && iFlex < m_pstudiohdr->numflexcontrollers)
	{
		mstudioflexcontroller_t *pflex = m_pstudiohdr->pFlexcontroller(iFlex);

		return m_flexweight[iFlex];
	}
	return 0.0;
}

int StudioModel::GetNumLODs() const
{
	return m_pStudioRender->GetNumLODs( m_HardwareData );
}

float StudioModel::GetLODSwitchValue( int lod ) const
{
	return m_pStudioRender->GetLODSwitchValue( m_HardwareData, lod );
}

void StudioModel::SetLODSwitchValue( int lod, float switchValue )
{
	m_pStudioRender->SetLODSwitchValue( m_HardwareData, lod, switchValue );
}

void StudioModel::ExtractBbox( Vector &mins, Vector &maxs )
{
	if ( !m_pstudiohdr )
		return;

	// look for hull
	if (m_pstudiohdr->hull_min.Length() != 0)
	{
		mins = m_pstudiohdr->hull_min;
		maxs = m_pstudiohdr->hull_max;
	}
	// look for view clip
	else if (m_pstudiohdr->view_bbmin.Length() != 0)
	{
		mins = m_pstudiohdr->view_bbmin;
		maxs = m_pstudiohdr->view_bbmax;
	}
	else
	{
		mstudioseqdesc_t &pseqdesc = m_pstudiohdr->pSeqdesc( m_sequence );

		mins = pseqdesc.bbmin;
		maxs = pseqdesc.bbmax;
	}
}



void StudioModel::GetSequenceInfo( int iSequence, float *pflFrameRate, float *pflGroundSpeed )
{
	float t = GetDuration( iSequence );

	if (t > 0)
	{
		*pflFrameRate = 1.0 / t;
	}
	else
	{
		*pflFrameRate = 1.0;
	}
	*pflGroundSpeed = GetGroundSpeed( iSequence );
}

void StudioModel::GetSequenceInfo( float *pflFrameRate, float *pflGroundSpeed )
{
	GetSequenceInfo( m_sequence, pflFrameRate, pflGroundSpeed );
}

float StudioModel::GetFPS( int iSequence )
{
	if ( !m_pstudiohdr )
		return 0.0f;

	return Studio_FPS( m_pstudiohdr, iSequence, m_poseparameter );
}

float StudioModel::GetFPS( void )
{
	return GetFPS( m_sequence );
}

float StudioModel::GetDuration( int iSequence )
{
	if ( !m_pstudiohdr )
		return 0.0f;

	return Studio_Duration( m_pstudiohdr, iSequence, m_poseparameter );
}


int StudioModel::GetNumFrames( int iSequence )
{
	if ( !m_pstudiohdr || iSequence < 0 || iSequence >= m_pstudiohdr->GetNumSeq() )
	{
		return 1;
	}

	return Studio_MaxFrame( m_pstudiohdr, iSequence, m_poseparameter );
}

static int GetSequenceFlags( studiohdr_t *pstudiohdr, int sequence )
{
	if ( !pstudiohdr || 
		sequence < 0 || 
		sequence >= pstudiohdr->GetNumSeq() )
	{
		return 0;
	}

	mstudioseqdesc_t &seqdesc = pstudiohdr->pSeqdesc( sequence );

	return seqdesc.flags;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iSequence - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool StudioModel::GetSequenceLoops( int iSequence )
{
	if ( !m_pstudiohdr )
		return false;

	int flags = GetSequenceFlags( m_pstudiohdr, iSequence );
	bool looping = flags & STUDIO_LOOPING ? true : false;
	return looping;
}

float StudioModel::GetDuration( )
{
	return GetDuration( m_sequence );
}


void StudioModel::GetMovement( float prevcycle[5], Vector &vecPos, QAngle &vecAngles )
{
	vecPos.Init();
	vecAngles.Init();

	if ( !m_pstudiohdr )
		return;

  	// assume that changes < -0.5 are loops....
  	if (m_cycle - prevcycle[0] < -0.5)
  	{
  		prevcycle[0] = prevcycle[0] - 1.0;
  	}

	Studio_SeqMovement( m_pstudiohdr, m_sequence, prevcycle[0], m_cycle, m_poseparameter, vecPos, vecAngles );
	prevcycle[0] = m_cycle;

	int i;
	for (i = 0; i < 4; i++)
	{
		Vector vecTmp;
		QAngle angTmp;

  		if (m_Layer[i].m_cycle - prevcycle[i+1] < -0.5)
  		{
  			prevcycle[i+1] = prevcycle[i+1] - 1.0;
  		}

		if (m_Layer[i].m_weight > 0.0)
		{
			vecTmp.Init();
			angTmp.Init();
			if (Studio_SeqMovement( m_pstudiohdr, m_Layer[i].m_sequence, prevcycle[i+1], m_Layer[i].m_cycle, m_poseparameter, vecTmp, angTmp ))
			{
				vecPos = vecPos * ( 1.0 - m_Layer[i].m_weight ) + vecTmp * m_Layer[i].m_weight;
			}
		}
		prevcycle[i+1] = m_Layer[i].m_cycle;
	}

	return;
}


void StudioModel::GetMovement( int iSequence, float prevCycle, float nextCycle, Vector &vecPos, QAngle &vecAngles )
{
	if ( !m_pstudiohdr )
	{
		vecPos.Init();
		vecAngles.Init();
		return;
	}

	// FIXME: this doesn't consider layers
	Studio_SeqMovement( m_pstudiohdr, iSequence, prevCycle, nextCycle, m_poseparameter, vecPos, vecAngles );

	return;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the ground speed of the specifed sequence.
//-----------------------------------------------------------------------------
float StudioModel::GetGroundSpeed( int iSequence )
{
	Vector vecMove;
	QAngle vecAngles;
	GetMovement( iSequence, 0, 1, vecMove, vecAngles );

	float t = GetDuration( iSequence );

	float flGroundSpeed = 0;
	if (t > 0)
	{
		flGroundSpeed = vecMove.Length() / t;
	}

	return flGroundSpeed;
}



//-----------------------------------------------------------------------------
// Purpose: Returns the ground speed of the current sequence.
//-----------------------------------------------------------------------------
float StudioModel::GetGroundSpeed( void )
{
	return GetGroundSpeed( m_sequence );
}


//-----------------------------------------------------------------------------
// Purpose: Returns the ground speed of the current sequence.
//-----------------------------------------------------------------------------
float StudioModel::GetCurrentVelocity( void )
{
	Vector vecVelocity;

	if (Studio_SeqVelocity( m_pstudiohdr, m_sequence, m_cycle, m_poseparameter, vecVelocity ))
	{
		return vecVelocity.Length();
	}
	return 0;
}


//-----------------------------------------------------------------------------
// Purpose: Returns the the sequence should be hidden or not
//-----------------------------------------------------------------------------
bool StudioModel::IsHidden( int iSequence )
{
	if (m_pstudiohdr->pSeqdesc( iSequence ).flags & STUDIO_HIDDEN)
		return true;

	return false;
}





void StudioModel::GetSeqAnims( int iSequence, mstudioanimdesc_t *panim[4], float *weight )
{
	if (!m_pstudiohdr)
		return;

	Studio_SeqAnims( m_pstudiohdr, iSequence, m_poseparameter, panim, weight );
}

void StudioModel::GetSeqAnims( mstudioanimdesc_t *panim[4], float *weight )
{
	GetSeqAnims( m_sequence, panim, weight );
}


float StudioModel::SetController( int iController, float flValue )
{
	if (!m_pstudiohdr)
		return 0.0f;

	return Studio_SetController( m_pstudiohdr, iController, flValue, m_controller[iController] );
}



int	StudioModel::LookupPoseParameter( char const *szName )
{
	if (!m_pstudiohdr)
		return false;

	for (int iParameter = 0; iParameter < m_pstudiohdr->GetNumPoseParameters(); iParameter++)
	{
		if (stricmp( szName, m_pstudiohdr->pPoseParameter( iParameter ).pszName() ) == 0)
		{
			return iParameter;
		}
	}
	return -1;
}

float StudioModel::SetPoseParameter( char const *szName, float flValue )
{
	return SetPoseParameter( LookupPoseParameter( szName ), flValue );
}

float StudioModel::SetPoseParameter( int iParameter, float flValue )
{
	if (!m_pstudiohdr)
		return 0.0f;

	return Studio_SetPoseParameter( m_pstudiohdr, iParameter, flValue, m_poseparameter[iParameter] );
}

float StudioModel::GetPoseParameter( char const *szName )
{
	return GetPoseParameter( LookupPoseParameter( szName ) );
}

float StudioModel::GetPoseParameter( int iParameter )
{
	if (!m_pstudiohdr)
		return 0.0f;

	return Studio_GetPoseParameter( m_pstudiohdr, iParameter, m_poseparameter[iParameter] );
}

bool StudioModel::GetPoseParameterRange( int iParameter, float *pflMin, float *pflMax )
{
	*pflMin = 0;
	*pflMax = 0;

	if (!m_pstudiohdr)
		return false;

	if (iParameter < 0 || iParameter >= m_pstudiohdr->GetNumPoseParameters())
		return false;

	const mstudioposeparamdesc_t &Pose = m_pstudiohdr->pPoseParameter( iParameter );

	*pflMin = Pose.start;
	*pflMax = Pose.end;

	return true;
}

int StudioModel::LookupAttachment( char const *szName )
{
	if ( !m_pstudiohdr )
		return -1;

	for (int i = 0; i < m_pstudiohdr->GetNumAttachments(); i++)
	{
		if (stricmp( m_pstudiohdr->pAttachment( i ).pszName(), szName ) == 0)
		{
			return i;
		}
	}
	return -1;
}



int StudioModel::SetBodygroup( int iGroup, int iValue )
{
	if (!m_pstudiohdr)
		return 0;

	if (iGroup > m_pstudiohdr->numbodyparts)
		return -1;

	mstudiobodyparts_t *pbodypart = m_pstudiohdr->pBodypart( iGroup );

	int iCurrent = (m_bodynum / pbodypart->base) % pbodypart->nummodels;

	if (iValue >= pbodypart->nummodels)
		return iCurrent;

	m_bodynum = (m_bodynum - (iCurrent * pbodypart->base) + (iValue * pbodypart->base));

	return iValue;
}


int StudioModel::SetSkin( int iValue )
{
	if (!m_pstudiohdr)
		return 0;

	if (iValue >= m_pstudiohdr->numskinfamilies)
	{
		return m_skinnum;
	}

	m_skinnum = iValue;

	return iValue;
}



void StudioModel::scaleMeshes (float scale)
{
	if (!m_pstudiohdr)
		return;

	int i, j, k;

	// manadatory to access correct verts
	SetCurrentModel();

	// scale verts
	int tmp = m_bodynum;
	for (i = 0; i < m_pstudiohdr->numbodyparts; i++)
	{
		mstudiobodyparts_t *pbodypart = m_pstudiohdr->pBodypart( i );
		for (j = 0; j < pbodypart->nummodels; j++)
		{
			SetBodygroup (i, j);
			SetupModel (i);

			const mstudio_modelvertexdata_t *vertData = m_pmodel->GetVertexData();

			for (k = 0; k < m_pmodel->numvertices; k++)
			{
				*vertData->Position(k) *= scale;
			}
		}
	}

	m_bodynum = tmp;

	// scale complex hitboxes
	int hitboxset = g_MDLViewer->GetCurrentHitboxSet();

	mstudiobbox_t *pbboxes = m_pstudiohdr->pHitbox( 0, hitboxset );
	for (i = 0; i < m_pstudiohdr->iHitboxCount( hitboxset ); i++)
	{
		VectorScale (pbboxes[i].bbmin, scale, pbboxes[i].bbmin);
		VectorScale (pbboxes[i].bbmax, scale, pbboxes[i].bbmax);
	}

	// scale bounding boxes
	for (i = 0; i < m_pstudiohdr->GetNumSeq(); i++)
	{
		mstudioseqdesc_t &seqdesc = m_pstudiohdr->pSeqdesc( i );
		VectorScale (seqdesc.bbmin, scale, seqdesc.bbmin);
		VectorScale (seqdesc.bbmax, scale, seqdesc.bbmax);
	}

	// maybe scale exeposition, pivots, attachments
}



void StudioModel::scaleBones (float scale)
{
	if (!m_pstudiohdr)
		return;

	mstudiobone_t *pbones = m_pstudiohdr->pBone( 0 );
	for (int i = 0; i < m_pstudiohdr->numbones; i++)
	{
		pbones[i].pos *= scale;
		pbones[i].posscale *= scale;
	}	
}

int	StudioModel::Physics_GetBoneCount( void )
{
	return m_pPhysics->Count();
}


const char *StudioModel::Physics_GetBoneName( int index ) 
{ 
	CPhysmesh *pmesh = m_pPhysics->GetMesh( index );

	if ( !pmesh )
		return NULL;

	return pmesh->m_boneName;
}


void StudioModel::Physics_GetData( int boneIndex, hlmvsolid_t *psolid, constraint_ragdollparams_t *pConstraint ) const
{
	CPhysmesh *pMesh = m_pPhysics->GetMesh( boneIndex );
	
	if ( !pMesh )
		return;

	if ( psolid )
	{
		memcpy( psolid, &pMesh->m_solid, sizeof(*psolid) );
	}

	if ( pConstraint )
	{
		*pConstraint = pMesh->m_constraint;
	}
}

void StudioModel::Physics_SetData( int boneIndex, const hlmvsolid_t *psolid, const constraint_ragdollparams_t *pConstraint )
{
	CPhysmesh *pMesh = m_pPhysics->GetMesh( boneIndex );
	
	if ( !pMesh )
		return;

	if ( psolid )
	{
		memcpy( &pMesh->m_solid, psolid, sizeof(*psolid) );
	}

	if ( pConstraint )
	{
		pMesh->m_constraint = *pConstraint;
	}
}


float StudioModel::Physics_GetMass( void )
{
	return m_pPhysics->GetMass();
}

void StudioModel::Physics_SetMass( float mass )
{
	m_physMass = mass;
}


char *StudioModel::Physics_DumpQC( void )
{
	return m_pPhysics->DumpQC();
}

const mstudio_modelvertexdata_t *mstudiomodel_t::GetVertexData()
{
	Assert( g_pActiveModel );

	vertexFileHeader_t *pVertexHdr = g_StudioDataCache.CacheVertexData(g_pActiveModel->getStudioHeader());
	
	vertexdata.pVertexData  = (byte *)pVertexHdr + pVertexHdr->vertexDataStart;
	vertexdata.pTangentData = (byte *)pVertexHdr + pVertexHdr->tangentDataStart;

	return &vertexdata;
}

