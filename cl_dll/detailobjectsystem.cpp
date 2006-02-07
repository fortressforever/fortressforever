//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Deals with singleton  
//
// $Revision: $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "DetailObjectSystem.h"
#include "GameBspFile.h"
#include "UtlBuffer.h"
#include "view.h"
#include "ClientMode.h"
#include "IViewRender.h"
#include "BSPTreeData.h"
#include "tier0/vprof.h"
#include "engine/ivmodelinfo.h"
#include "materialsystem/IMesh.h"
#include "model_types.h"
#include "env_detail_controller.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------
struct model_t;


ConVar	cl_detaildist( "cl_detaildist", "1200", 0, "Distance at which detail props are no longer visible" );
ConVar	cl_detailfade( "cl_detailfade", "400", 0, "Distance across which detail props fade in" );

//-----------------------------------------------------------------------------
// Detail models
//-----------------------------------------------------------------------------
class CDetailModel : public CBaseStaticModel
{
	DECLARE_CLASS( CDetailModel, CBaseStaticModel );

public:
	CDetailModel();

	// Initialization
	bool Init( int index, const Vector& org, const QAngle& angles, model_t* pModel, 
		ColorRGBExp32 lighting, int lightstyle, unsigned char lightstylecount, int orientation );

	bool InitSprite( int index, const Vector& org, const QAngle& angles, unsigned short nSpriteIndex, 
		ColorRGBExp32 lighting, int lightstyle, unsigned char lightstylecount, int orientation, float flScale );

	void GetColorModulation( float* color );

	// Computes the render angles for screen alignment
	void ComputeAngles( void );

	// Renders the sprite
	void DrawSprite( CMeshBuilder &meshBuilder );

	int GetType() const { return m_Type; }
	unsigned char GetAlpha() const { return m_Alpha; }

	bool IsDetailModelTranslucent();

	// IHandleEntity stubs.
public:
	virtual void SetRefEHandle( const CBaseHandle &handle )	{ Assert( false ); }
	virtual const CBaseHandle& GetRefEHandle() const		{ Assert( false ); return *((CBaseHandle*)0); }

protected:
	unsigned char	m_LightStyleCount;
	unsigned char	m_Orientation;
	unsigned char	m_Type;
	ColorRGBExp32	m_Color;
	unsigned int	m_LightStyle;

	// FIXME: Would be nice to not have to store these (or use a union or something)
	unsigned short	m_nSpriteIndex;
	float			m_flScale;
};


static ConVar mat_fullbright( "mat_fullbright", "0", FCVAR_CHEAT ); // hook into engine's cvars..
extern ConVar r_DrawDetailProps;
extern ConVar mat_wireframe;


//-----------------------------------------------------------------------------
// Dictionary for detail sprites
//-----------------------------------------------------------------------------
struct DetailPropSpriteDict_t
{
	Vector2D	m_UL;		// Coordinate of upper left
	Vector2D	m_LR;		// Coordinate of lower right
	Vector2D	m_TexUL;	// Texcoords of upper left
	Vector2D	m_TexLR;	// Texcoords of lower left
};


//-----------------------------------------------------------------------------
// Responsible for managing detail objects
//-----------------------------------------------------------------------------
class CDetailObjectSystem : public IDetailObjectSystem, public ISpatialLeafEnumerator
{
public:
	// constructor, destructor
	CDetailObjectSystem();
	virtual ~CDetailObjectSystem();

	// Init, shutdown
	virtual bool Init()
	{
		m_flDefaultFadeStart = cl_detailfade.GetFloat();
		m_flDefaultFadeEnd = cl_detaildist.GetFloat();
		return true;
	}
	virtual void Shutdown() {}

	// Level init, shutdown
	virtual void LevelInitPreEntity();
	virtual void LevelInitPostEntity();
	virtual void LevelShutdownPreEntity();
	virtual void LevelShutdownPostEntity();

	virtual void OnSave() {}
	virtual void OnRestore() {}
	virtual void SafeRemoveIfDesired() {}

	// Gets called each frame
	virtual void PreRender( ) {}
	virtual void Update ( float frametime ) {}

    // Gets a particular detail object
	virtual IClientRenderable* GetDetailModel( int idx );

	// Prepares detail for rendering 
	virtual void BuildDetailObjectRenderLists( );

	// Renders all opaque detail objects in a particular set of leaves
	virtual void RenderOpaqueDetailObjects( int nLeafCount, LeafIndex_t *pLeafList );

	// Renders all translucent detail objects in a particular set of leaves
	virtual void RenderTranslucentDetailObjects( const Vector &viewOrigin, const Vector &viewForward, int nLeafCount, LeafIndex_t *pLeafList );

	// Renders all translucent detail objects in a particular leaf up to a particular point
	virtual void RenderTranslucentDetailObjectsInLeaf( const Vector &viewOrigin, const Vector &viewForward, int nLeaf, const Vector *pVecClosestPoint );

	// Call this before rendering translucent detail objects
	virtual void BeginTranslucentDetailRendering( );

	// Method of ISpatialLeafEnumerator
	bool EnumerateLeaf( int leaf, int context );

	DetailPropLightstylesLump_t& DetailLighting( int i ) { return m_DetailLighting[i]; }
	DetailPropSpriteDict_t& DetailSpriteDict( int i ) { return m_DetailSpriteDict[i]; }

private:
	struct DetailModelDict_t
	{
		model_t* m_pModel;
	};

	struct EnumContext_t
	{
		float m_MaxSqDist;
		float m_FadeSqDist;
		float m_FalloffFactor;
		int	m_BuildWorldListNumber;
	};

	struct SortInfo_t
	{
		int m_nIndex;
		float m_flDistance;
	};

	enum
	{
		MAX_SPRITES_PER_LEAF = 4096
	};

	// Unserialization
	void UnserializeModelDict( CUtlBuffer& buf );
	void UnserializeDetailSprites( CUtlBuffer& buf );
	void UnserializeModels( CUtlBuffer& buf );
	void UnserializeModelLighting( CUtlBuffer& buf );

	// Count the number of detail sprites in the leaf list
	int CountSpritesInLeafList( int nLeafCount, LeafIndex_t *pLeafList );

	// Sorts sprites in back-to-front order
	static int SortFunc( const void *arg1, const void *arg2 );
	int SortSpritesBackToFront( int nLeaf, const Vector &viewOrigin, const Vector &viewForward, SortInfo_t *pSortInfo );

	// For fast detail object insertion
	IterationRetval_t EnumElement( int userId, int context );

	CUtlVector<DetailModelDict_t>			m_DetailObjectDict;
	CUtlVector<CDetailModel>				m_DetailObjects;
	CUtlVector<DetailPropSpriteDict_t>		m_DetailSpriteDict;
	CUtlVector<DetailPropLightstylesLump_t>	m_DetailLighting;

	// Necessary to get sprites to batch correctly
	CMaterialReference m_DetailSpriteMaterial;
	CMaterialReference m_DetailWireframeMaterial;

	// State stored off for rendering detail sprites in a single leaf
	int m_nSpriteCount;
	int m_nFirstSprite;
	int m_nSortedLeaf;
	SortInfo_t m_pSortInfo[MAX_SPRITES_PER_LEAF];

	float m_flDefaultFadeStart;
	float m_flDefaultFadeEnd;
};


//-----------------------------------------------------------------------------
// System for dealing with detail objects
//-----------------------------------------------------------------------------
static CDetailObjectSystem s_DetailObjectSystem;

IDetailObjectSystem* DetailObjectSystem()
{
	return &s_DetailObjectSystem;
}



//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------
CBaseStaticModel::CBaseStaticModel() : m_pModel(0), m_Alpha(255)
{
	m_ModelInstance = MODEL_INSTANCE_INVALID;
	m_hRenderHandle = INVALID_CLIENT_RENDER_HANDLE;
}

CBaseStaticModel::~CBaseStaticModel()
{
	if (m_ModelInstance != MODEL_INSTANCE_INVALID)
	{
		modelrender->DestroyInstance( m_ModelInstance );
	}
}


//-----------------------------------------------------------------------------
// Initialization
//-----------------------------------------------------------------------------
bool CBaseStaticModel::Init( int index, const Vector& org, const QAngle& angles, model_t* pModel )
{
	VectorCopy( org, m_Origin );
	VectorCopy( angles, m_Angles );
	m_pModel = pModel;
	m_Alpha = 255;

	return true;
}


//-----------------------------------------------------------------------------
// Data accessors
//-----------------------------------------------------------------------------
const Vector& CBaseStaticModel::GetRenderOrigin( void )
{
	return m_Origin;
}

const QAngle& CBaseStaticModel::GetRenderAngles( void )
{
	return m_Angles;
}

bool CBaseStaticModel::IsTransparent( void )
{
	return (m_Alpha < 255) || modelinfo->IsTranslucent(m_pModel);
}

bool CBaseStaticModel::ShouldDraw()
{
	// Don't draw in commander mode
	return g_pClientMode->ShouldDrawDetailObjects();
}

void CBaseStaticModel::GetRenderBounds( Vector& mins, Vector& maxs )
{
	int nModelType = modelinfo->GetModelType( m_pModel );
	if (nModelType == mod_studio || nModelType == mod_brush)
	{
		modelinfo->GetModelRenderBounds( GetModel(), mins, maxs );
	}
	else
	{
		mins.Init( 0,0,0 );
		maxs.Init( 0,0,0 );
	}
}

IPVSNotify* CBaseStaticModel::GetPVSNotifyInterface()
{
	return NULL;
}

void CBaseStaticModel::GetRenderBoundsWorldspace( Vector& mins, Vector& maxs )
{
	DefaultRenderBoundsWorldspace( this, mins, maxs );
}

bool CBaseStaticModel::ShouldReceiveProjectedTextures( int flags )
{
	return false;
}

bool CBaseStaticModel::UsesFrameBufferTexture()
{
	return false;
}

void CBaseStaticModel::GetShadowRenderBounds( Vector &mins, Vector &maxs, ShadowType_t shadowType )
{
	GetRenderBounds( mins, maxs );
}

ClientShadowHandle_t CBaseStaticModel::GetShadowHandle() const
{
	return CLIENTSHADOW_INVALID_HANDLE;
}

ClientRenderHandle_t& CBaseStaticModel::RenderHandle()
{
	return m_hRenderHandle;
}	


//-----------------------------------------------------------------------------
// Render setup
//-----------------------------------------------------------------------------
bool CBaseStaticModel::SetupBones( matrix3x4_t *pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime )
{
	if (!m_pModel)
		return false;

	// Setup our transform.
	matrix3x4_t parentTransform;
	const QAngle &vRenderAngles = GetRenderAngles();
	const Vector &vRenderOrigin = GetRenderOrigin();
	AngleMatrix( vRenderAngles, parentTransform );
	parentTransform[0][3] = vRenderOrigin.x;
	parentTransform[1][3] = vRenderOrigin.y;
	parentTransform[2][3] = vRenderOrigin.z;

	// Just copy it on down baby
	studiohdr_t *pStudioHdr = modelinfo->GetStudiomodel( m_pModel );
	for (int i = 0; i < pStudioHdr->numbones; i++) 
	{
		MatrixCopy( parentTransform, pBoneToWorldOut[i] );
	}

	return true;
}

void	CBaseStaticModel::SetupWeights( void )
{
}

void	CBaseStaticModel::DoAnimationEvents( void )
{
}


//-----------------------------------------------------------------------------
// Render baby!
//-----------------------------------------------------------------------------
const model_t* CBaseStaticModel::GetModel( ) const
{
	return m_pModel;
}

int CBaseStaticModel::DrawModel( int flags )
{
	if ((m_Alpha == 0) || (!m_pModel))
		return 0;

	int drawn = modelrender->DrawModel( 
		flags, 
		this,
		m_ModelInstance,
		-1,		// no entity index
		m_pModel,
		m_Origin,
		m_Angles,
		0,	// skin
		0,	// body
		0  // hitboxset
		);
	return drawn;
}


//-----------------------------------------------------------------------------
// Determine alpha and blend amount for transparent objects based on render state info
//-----------------------------------------------------------------------------
void CBaseStaticModel::ComputeFxBlend( )
{
	// Do nothing, it's already calculate in our m_Alpha
}

int CBaseStaticModel::GetFxBlend( )
{
	return m_Alpha;
}

void CBaseStaticModel::GetColorModulation( float* color )
{
	color[0] = color[1] = color[2] = 1.0f;
}


//-----------------------------------------------------------------------------
// Returns false if the entity shouldn't be drawn due to LOD.
//-----------------------------------------------------------------------------
bool CBaseStaticModel::LODTest()
{
	return true;
}



//-----------------------------------------------------------------------------
// Detail models stuff
//-----------------------------------------------------------------------------
CDetailModel::CDetailModel() : m_LightStyleCount(0) 
{
	m_Color.r = m_Color.g = m_Color.b = 255;
	m_Color.exponent = 0;
}


//-----------------------------------------------------------------------------
// Initialization
//-----------------------------------------------------------------------------
bool CDetailModel::Init( int index, const Vector& org, const QAngle& angles, 
	model_t* pModel, ColorRGBExp32 lighting, int lightstyle, unsigned char lightstylecount, 
	int orientation)
{
	m_Color = lighting;
	m_LightStyleCount = lightstylecount;
	m_LightStyle = lightstyle; 
	m_Orientation = orientation;
	m_Type = DETAIL_PROP_TYPE_MODEL;
	return CBaseStaticModel::Init( index, org, angles, pModel );
}

bool CDetailModel::InitSprite( int index, const Vector& org, const QAngle& angles, unsigned short nSpriteIndex, 
	ColorRGBExp32 lighting, int lightstyle, unsigned char lightstylecount, int orientation, float flScale )
{
	m_Color = lighting;
	m_LightStyleCount = lightstylecount;
	m_LightStyle = lightstyle; 
	m_Orientation = orientation;
	m_nSpriteIndex = nSpriteIndex;
	m_Type = DETAIL_PROP_TYPE_SPRITE;
	m_flScale = flScale;

	// HACK: Use m_pModel here to indicate flipped-ness horizontally!
	return CBaseStaticModel::Init( index, org, angles, (model_t*)(index & 0x1) );
}


//-----------------------------------------------------------------------------
// Color, alpha modulation
//-----------------------------------------------------------------------------
void CDetailModel::GetColorModulation( float *color )
{
	if (mat_fullbright.GetInt() == 1)
	{
		color[0] = color[1] = color[2] = 1.0f;
		return;
	}

	Vector tmp;
	Vector normal( 1, 0, 0);
	engine->ComputeDynamicLighting( m_Origin, &normal, tmp );

	float val = engine->LightStyleValue( 0 );
	color[0] = tmp[0] + val * TexLightToLinear( m_Color.r, m_Color.exponent );
	color[1] = tmp[1] + val * TexLightToLinear( m_Color.g, m_Color.exponent );
	color[2] = tmp[2] + val * TexLightToLinear( m_Color.b, m_Color.exponent );

	// Add in the lightstyles
	for (int i = 0; i < m_LightStyleCount; ++i)
	{
		DetailPropLightstylesLump_t& lighting = s_DetailObjectSystem.DetailLighting( m_LightStyle + i );
		val = engine->LightStyleValue( lighting.m_Style );
		if (val != 0)
		{
			color[0] += val * TexLightToLinear( lighting.m_Lighting.r, lighting.m_Lighting.exponent ); 
			color[1] += val * TexLightToLinear( lighting.m_Lighting.g, lighting.m_Lighting.exponent ); 
			color[2] += val * TexLightToLinear( lighting.m_Lighting.b, lighting.m_Lighting.exponent ); 
		}
	}

	// Gamma correct....
	engine->LinearToGamma( color, color );
}


//-----------------------------------------------------------------------------
// Is the model itself translucent, regardless of modulation?
//-----------------------------------------------------------------------------
bool CDetailModel::IsDetailModelTranslucent()
{
	// FIXME: This is only true for my first pass of this feature
	if (m_Type == DETAIL_PROP_TYPE_SPRITE)
		return true;

	return modelinfo->IsTranslucent(GetModel());
}


//-----------------------------------------------------------------------------
// Computes the render angles for screen alignment
//-----------------------------------------------------------------------------
void CDetailModel::ComputeAngles( void )
{
	switch( m_Orientation )
	{
	case 0:
		break;

	case 1:
		{
			Vector vecDir;
			VectorSubtract( CurrentViewOrigin(), m_Origin, vecDir );
			VectorAngles( vecDir, m_Angles );
		}
		break;

	case 2:
		{
			Vector vecDir;
			VectorSubtract( CurrentViewOrigin(), m_Origin, vecDir );
			vecDir.z = 0.0f;
			VectorAngles( vecDir, m_Angles );
		}
		break;
	}
}


//-----------------------------------------------------------------------------
// Renders the sprite
//-----------------------------------------------------------------------------
void CDetailModel::DrawSprite( CMeshBuilder &meshBuilder )
{
	Assert( m_Type == DETAIL_PROP_TYPE_SPRITE );

	Vector vecColor;
	GetColorModulation( vecColor.Base() );

	unsigned char color[4];
	color[0] = (unsigned char)(vecColor[0] * 255.0f);
	color[1] = (unsigned char)(vecColor[1] * 255.0f);
	color[2] = (unsigned char)(vecColor[2] * 255.0f);
	color[3] = m_Alpha;

	DetailPropSpriteDict_t &dict = s_DetailObjectSystem.DetailSpriteDict( m_nSpriteIndex );

	Vector vecOrigin, dx, dy;
	AngleVectors( m_Angles, NULL, &dx, &dy );

	Vector2D ul, lr;
	Vector2DMultiply( dict.m_UL, m_flScale, ul );
	Vector2DMultiply( dict.m_LR, m_flScale, lr );

	VectorMA( m_Origin, ul.x, dx, vecOrigin );
	VectorMA( vecOrigin, ul.y, dy, vecOrigin );
	dx *= (lr.x - ul.x);
	dy *= (lr.y - ul.y);

	Vector2D texul, texlr;
	texul = dict.m_TexUL;
	texlr = dict.m_TexLR;

	// What a shameless re-use of bits (m_pModel == 0 when it should be flipped horizontally)
	if ( !m_pModel )
	{
		texul.x = dict.m_TexLR.x;
		texlr.x = dict.m_TexUL.x;
	}

	meshBuilder.Position3fv( vecOrigin.Base() );
	meshBuilder.TexCoord2fv( 0, texul.Base() );
	meshBuilder.Color4ubv( color );
	meshBuilder.AdvanceVertex();

	vecOrigin += dy;
	meshBuilder.Position3fv( vecOrigin.Base() );
	meshBuilder.TexCoord2f( 0, texul.x, texlr.y );
	meshBuilder.Color4ubv( color );
	meshBuilder.AdvanceVertex();

	vecOrigin += dx;
	meshBuilder.Position3fv( vecOrigin.Base() );
	meshBuilder.TexCoord2fv( 0, texlr.Base() );
	meshBuilder.Color4ubv( color );
	meshBuilder.AdvanceVertex();

	vecOrigin -= dy;
	meshBuilder.Position3fv( vecOrigin.Base() );
	meshBuilder.TexCoord2f( 0, texlr.x, texul.y );
	meshBuilder.Color4ubv( color );
	meshBuilder.AdvanceVertex();
}


//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------
CDetailObjectSystem::CDetailObjectSystem() : m_DetailObjects( 0, 1024 ), m_DetailSpriteDict( 0, 32 ), m_DetailObjectDict( 0, 32 )
{
	BuildExponentTable();
}

CDetailObjectSystem::~CDetailObjectSystem()
{
}

	   
//-----------------------------------------------------------------------------
// Level init, shutdown
//-----------------------------------------------------------------------------
void CDetailObjectSystem::LevelInitPreEntity()
{
	// Prepare the translucent detail sprite material; we only have 1!
	m_DetailSpriteMaterial.Init( "detail/detailsprites", TEXTURE_GROUP_OTHER );
	m_DetailWireframeMaterial.Init( "debug/debugspritewireframe", TEXTURE_GROUP_OTHER );

	// Version check
	if (engine->GameLumpVersion( GAMELUMP_DETAIL_PROPS ) < 4)
	{
		Warning("Map uses old detail prop file format.. ignoring detail props\n");
		return;
	}

	// Unserialize
	int size = engine->GameLumpSize( GAMELUMP_DETAIL_PROPS );
	CUtlMemory<unsigned char> fileMemory;
	fileMemory.EnsureCapacity( size );
	if (engine->LoadGameLump( GAMELUMP_DETAIL_PROPS, fileMemory.Base(), size ))
	{
		CUtlBuffer buf( fileMemory.Base(), size );
		UnserializeModelDict( buf );

		switch (engine->GameLumpVersion( GAMELUMP_DETAIL_PROPS ) )
		{
		case 4:
			UnserializeDetailSprites( buf );
			UnserializeModels( buf );
			break;
		}
	}

	size = engine->GameLumpSize( GAMELUMP_DETAIL_PROP_LIGHTING );
	fileMemory.EnsureCapacity( size );
	if (engine->LoadGameLump( GAMELUMP_DETAIL_PROP_LIGHTING, fileMemory.Base(), size ))
	{
		CUtlBuffer buf( fileMemory.Base(), size );
		UnserializeModelLighting( buf );
	}
}

void CDetailObjectSystem::LevelInitPostEntity()
{
	if ( GetDetailController() )
	{
		cl_detailfade.SetValue( min( m_flDefaultFadeStart, GetDetailController()->m_flFadeStartDist ) );
		cl_detaildist.SetValue( min( m_flDefaultFadeEnd, GetDetailController()->m_flFadeEndDist ) );
	}
	else
	{
		// revert to default values if the map doesn't specify
		cl_detailfade.SetValue( m_flDefaultFadeStart );
		cl_detaildist.SetValue( m_flDefaultFadeEnd );
	}
}

void CDetailObjectSystem::LevelShutdownPreEntity()
{
	m_DetailObjects.RemoveAll();
	m_DetailObjectDict.RemoveAll();
	m_DetailSpriteDict.RemoveAll();
	m_DetailLighting.RemoveAll();
}

void CDetailObjectSystem::LevelShutdownPostEntity()
{
	m_DetailSpriteMaterial.Shutdown();
	m_DetailWireframeMaterial.Shutdown();
}


//-----------------------------------------------------------------------------
// Before each view, blat out the stored detail sprite state
//-----------------------------------------------------------------------------
void CDetailObjectSystem::BeginTranslucentDetailRendering( )
{
	m_nSortedLeaf = -1;
	m_nSpriteCount = m_nFirstSprite = 0;
}


//-----------------------------------------------------------------------------
// Gets a particular detail object
//-----------------------------------------------------------------------------
IClientRenderable* CDetailObjectSystem::GetDetailModel( int idx )
{
	// FIXME: This is necessary because we have intermixed models + sprites
	// in a single list (m_DetailObjects)
	if (m_DetailObjects[idx].GetType() != DETAIL_PROP_TYPE_MODEL)
		return NULL;

	return &m_DetailObjects[idx];
}


//-----------------------------------------------------------------------------
// Unserialization
//-----------------------------------------------------------------------------
void CDetailObjectSystem::UnserializeModelDict( CUtlBuffer& buf )
{
	int count = buf.GetInt();
	while ( --count >= 0 )
	{
		DetailObjectDictLump_t lump;
		buf.Get( &lump, sizeof(DetailObjectDictLump_t) );
		
		DetailModelDict_t dict;
		dict.m_pModel = (model_t *)engine->LoadModel( lump.m_Name, true );

		// Don't allow vertex-lit models
		if (modelinfo->IsModelVertexLit(dict.m_pModel))
		{
			Warning("Detail prop model %s is using vertex-lit materials!\nIt must use unlit materials!\n", lump.m_Name );
			dict.m_pModel = (model_t *)engine->LoadModel( "models/error.mdl" );
		}

		m_DetailObjectDict.AddToTail( dict );
	}
}

void CDetailObjectSystem::UnserializeDetailSprites( CUtlBuffer& buf )
{
	int count = buf.GetInt();
	while ( --count >= 0 )
	{
		int i = m_DetailSpriteDict.AddToTail();
		buf.Get( &m_DetailSpriteDict[i], sizeof(DetailSpriteDictLump_t) );
	}
}


void CDetailObjectSystem::UnserializeModelLighting( CUtlBuffer& buf )
{
	int count = buf.GetInt();
	while ( --count >= 0 )
	{
		int i = m_DetailLighting.AddToTail();
		buf.Get( &m_DetailLighting[i], sizeof(DetailPropLightstylesLump_t) );
	}
}


//-----------------------------------------------------------------------------
// Unserialize all models
//-----------------------------------------------------------------------------
void CDetailObjectSystem::UnserializeModels( CUtlBuffer& buf )
{
	int firstDetailObject = 0;
	int detailObjectCount = 0;
	int detailObjectLeaf = -1;

	int count = buf.GetInt();
	while ( --count >= 0 )
	{
		DetailObjectLump_t lump;
		buf.Get( &lump, sizeof(DetailObjectLump_t) );
		
		// We rely on the fact that details objects are sorted by leaf in the
		// bsp file for this
		if ( detailObjectLeaf != lump.m_Leaf )
		{
			if (detailObjectLeaf != -1)
			{
				ClientLeafSystem()->SetDetailObjectsInLeaf( detailObjectLeaf, 
					firstDetailObject, detailObjectCount );
			}

			detailObjectLeaf = lump.m_Leaf;
			firstDetailObject = m_DetailObjects.Count();
			detailObjectCount = 0;
		}

		int newObj = m_DetailObjects.AddToTail();
		if ( lump.m_Type == DETAIL_PROP_TYPE_MODEL )
		{
			m_DetailObjects[newObj].Init( newObj, lump.m_Origin, lump.m_Angles, 
				m_DetailObjectDict[lump.m_DetailModel].m_pModel, lump.m_Lighting,
				lump.m_LightStyles, lump.m_LightStyleCount, lump.m_Orientation );
		}
		else
		{
			m_DetailObjects[newObj].InitSprite( newObj, lump.m_Origin, lump.m_Angles, 
				lump.m_DetailModel, lump.m_Lighting,
				lump.m_LightStyles, lump.m_LightStyleCount, lump.m_Orientation, lump.m_flScale );
		}

		++detailObjectCount;
	}

	if (detailObjectLeaf != -1)
	{
		ClientLeafSystem()->SetDetailObjectsInLeaf( detailObjectLeaf, 
			firstDetailObject, detailObjectCount );
	}
}



//-----------------------------------------------------------------------------
// Renders all opaque detail objects in a particular set of leaves
//-----------------------------------------------------------------------------
void CDetailObjectSystem::RenderOpaqueDetailObjects( int nLeafCount, LeafIndex_t *pLeafList )
{
	// FIXME: Implement!
}


//-----------------------------------------------------------------------------
// Count the number of detail sprites in the leaf list
//-----------------------------------------------------------------------------
int CDetailObjectSystem::CountSpritesInLeafList( int nLeafCount, LeafIndex_t *pLeafList )
{
	VPROF_BUDGET( "CDetailObjectSystem::CountSpritesInLeafList", VPROF_BUDGETGROUP_DETAILPROP_RENDERING );
	int nPropCount = 0;
	int nFirstDetailObject, nDetailObjectCount;
	for ( int i = 0; i < nLeafCount; ++i )
	{
		// FIXME: This actually counts *everything* in the leaf, which is ok for now
		// given how we're using it
		ClientLeafSystem()->GetDetailObjectsInLeaf( pLeafList[i], nFirstDetailObject, nDetailObjectCount );
		nPropCount += nDetailObjectCount;
	}

	return nPropCount;
}


//-----------------------------------------------------------------------------
// Sorts sprites in back-to-front order
//-----------------------------------------------------------------------------
int CDetailObjectSystem::SortFunc( const void *arg1, const void *arg2 )
{
	// Therefore, things that are farther away in front of us (has a greater + distance)
	// need to appear at the front of the list, hence the somewhat misleading code below
	float flDelta = ((SortInfo_t*)arg1)->m_flDistance - ((SortInfo_t*)arg2)->m_flDistance;
	if ( flDelta > 0 )
		return -1;
	if ( flDelta < 0 )
		return 1;
	return 0;
}

int CDetailObjectSystem::SortSpritesBackToFront( int nLeaf, const Vector &viewOrigin, const Vector &viewForward, SortInfo_t *pSortInfo )
{
	VPROF_BUDGET( "CDetailObjectSystem::SortSpritesBackToFront", VPROF_BUDGETGROUP_DETAILPROP_RENDERING );
	int nFirstDetailObject, nDetailObjectCount;
	ClientLeafSystem()->GetDetailObjectsInLeaf( nLeaf, nFirstDetailObject, nDetailObjectCount );

	Vector vecDelta;
	int nCount = 0;
	for ( int j = 0; j < nDetailObjectCount; ++j )
	{
		CDetailModel &model = m_DetailObjects[nFirstDetailObject + j];

		if ( (model.GetType() == DETAIL_PROP_TYPE_MODEL) || (model.GetAlpha() == 0) )
			continue;

		pSortInfo[nCount].m_nIndex = nFirstDetailObject + j;

		// Compute distance from the camera to each object
		VectorSubtract( model.GetRenderOrigin(), viewOrigin, vecDelta );
		pSortInfo[nCount].m_flDistance = vecDelta.LengthSqr(); //DotProduct( viewForward, vecDelta );
		++nCount;
	}

	qsort( pSortInfo, nCount, sizeof(SortInfo_t), SortFunc ); 
	return nCount;
}


//-----------------------------------------------------------------------------
// Renders all translucent detail objects in a particular set of leaves
//-----------------------------------------------------------------------------
#define MAX_BATCH_SIZE 4096

void CDetailObjectSystem::RenderTranslucentDetailObjects( const Vector &viewOrigin, const Vector &viewForward, int nLeafCount, LeafIndex_t *pLeafList )
{
	VPROF_BUDGET( "CDetailObjectSystem::RenderTranslucentDetailObjects", VPROF_BUDGETGROUP_DETAILPROP_RENDERING );
	if (nLeafCount == 0)
		return;

	// We better not have any partially drawn leaf of detail sprites!
	Assert( m_nSpriteCount == m_nFirstSprite );

	// Here, we must draw all detail objects back-to-front
	// FIXME: Cache off a sorted list so we don't have to re-sort every frame

	// Count the total # of detail sprites we possibly could render
	int nSpriteCount = CountSpritesInLeafList( nLeafCount, pLeafList );
	if (nSpriteCount == 0)
		return;

	int nCountToDraw = nSpriteCount;
	if (nCountToDraw > MAX_BATCH_SIZE)
	{
		nCountToDraw = MAX_BATCH_SIZE;
	}

	materials->MatrixMode( MATERIAL_MODEL );
	materials->PushMatrix();
	materials->LoadIdentity();

	IMaterial *pMaterial = m_DetailSpriteMaterial;
	if ( mat_wireframe.GetBool() )
	{
		pMaterial = m_DetailWireframeMaterial;
	}

	CMeshBuilder meshBuilder;
	IMesh *pMesh = materials->GetDynamicMesh( true, NULL, NULL, pMaterial );
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, nCountToDraw );

	int nTotalDrawn = 0;
	int nCountDrawn = 0;
	for ( int i = 0; i < nLeafCount; ++i )
	{
		int nLeaf = pLeafList[i];

		int nFirstDetailObject, nDetailObjectCount;
		ClientLeafSystem()->GetDetailObjectsInLeaf( nLeaf, nFirstDetailObject, nDetailObjectCount );

		// Sort detail sprites in each leaf independently; then render them
		SortInfo_t *pSortInfo = (SortInfo_t *)stackalloc( nDetailObjectCount * sizeof(SortInfo_t) );
		int nCount = SortSpritesBackToFront( nLeaf, viewOrigin, viewForward, pSortInfo );

		for ( int j = 0; j < nCount; ++j )
		{
			CDetailModel &model = m_DetailObjects[pSortInfo[j].m_nIndex];
			model.DrawSprite( meshBuilder );

			++nTotalDrawn;

			// Prevent the batches from getting too large
			if ( ++nCountDrawn >= nCountToDraw )
			{
				meshBuilder.End();
				pMesh->Draw();

				nSpriteCount -= nCountToDraw;
				nCountToDraw = nSpriteCount;
				if (nCountToDraw > MAX_BATCH_SIZE)
				{
					nCountToDraw = MAX_BATCH_SIZE;
				}

				meshBuilder.Begin( pMesh, MATERIAL_QUADS, nCountToDraw );
				nCountDrawn = 0;
			}
		}
	}

	meshBuilder.End();
	pMesh->Draw();

	materials->PopMatrix();
}


//-----------------------------------------------------------------------------
// Renders a subset of the detail objects in a particular leaf (for interleaving with other translucent entities)
//-----------------------------------------------------------------------------
void CDetailObjectSystem::RenderTranslucentDetailObjectsInLeaf( const Vector &viewOrigin, const Vector &viewForward, int nLeaf, const Vector *pVecClosestPoint )
{
	VPROF_BUDGET( "CDetailObjectSystem::RenderTranslucentDetailObjectsInLeaf", VPROF_BUDGETGROUP_DETAILPROP_RENDERING );

	// We may have already sorted this leaf. If not, sort the leaf.
	if ( m_nSortedLeaf != nLeaf )
	{
		m_nSortedLeaf = nLeaf;
		m_nSpriteCount = 0;
		m_nFirstSprite = 0;

		// Count the total # of detail sprites we possibly could render
		LeafIndex_t nLeafIndex = nLeaf;
		int nSpriteCount = CountSpritesInLeafList( 1, &nLeafIndex );
		Assert( nSpriteCount <= MAX_SPRITES_PER_LEAF ); 
		if (nSpriteCount == 0)
			return;

		// Sort detail sprites in each leaf independently; then render them
		m_nSpriteCount = SortSpritesBackToFront( nLeaf, viewOrigin, viewForward, m_pSortInfo );
		Assert( m_nSpriteCount <= nSpriteCount );
	}

	// No more to draw? Bye!
	if ( m_nSpriteCount == m_nFirstSprite )
		return;

	float flMinDistance = 0.0f;
	if ( pVecClosestPoint )
	{
		Vector vecDelta;
		VectorSubtract( *pVecClosestPoint, viewOrigin, vecDelta );
		flMinDistance = vecDelta.LengthSqr();
	}

	if ( m_pSortInfo[m_nFirstSprite].m_flDistance < flMinDistance )
		return;

	materials->MatrixMode( MATERIAL_MODEL );
	materials->PushMatrix();
	materials->LoadIdentity();

	IMaterial *pMaterial = m_DetailSpriteMaterial;
	if ( mat_wireframe.GetBool() )
	{
		pMaterial = m_DetailWireframeMaterial;
	}

	CMeshBuilder meshBuilder;
	IMesh *pMesh = materials->GetDynamicMesh( true, NULL, NULL, pMaterial );
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, m_nSpriteCount - m_nFirstSprite );

	while ( m_nFirstSprite < m_nSpriteCount && m_pSortInfo[m_nFirstSprite].m_flDistance >= flMinDistance )
	{
		CDetailModel &model = m_DetailObjects[m_pSortInfo[m_nFirstSprite].m_nIndex];
		model.DrawSprite( meshBuilder );
		++m_nFirstSprite;
	}
	meshBuilder.End();
	pMesh->Draw();

	materials->PopMatrix();
}


//-----------------------------------------------------------------------------
// Gets called each view
//-----------------------------------------------------------------------------
bool CDetailObjectSystem::EnumerateLeaf( int leaf, int context )
{
	VPROF_BUDGET( "CDetailObjectSystem::EnumerateLeaf", VPROF_BUDGETGROUP_DETAILPROP_RENDERING );
	Vector v;
	int i, firstDetailObject, detailObjectCount;

	EnumContext_t* pCtx = (EnumContext_t*)context;
	ClientLeafSystem()->DrawDetailObjectsInLeaf( leaf, pCtx->m_BuildWorldListNumber, 
		firstDetailObject, detailObjectCount );

	// Compute the translucency. Need to do it now cause we need to
	// know that when we're rendering (opaque stuff is rendered first)
	for ( i = 0; i < detailObjectCount; ++i)
	{
		// Calculate distance (badly)
		CDetailModel& model = m_DetailObjects[firstDetailObject+i];
		VectorSubtract( model.GetRenderOrigin(), CurrentViewOrigin(), v );

		float sqDist = v.LengthSqr();

		if ( sqDist < pCtx->m_MaxSqDist )
		{
			if ((pCtx->m_FadeSqDist > 0) && (sqDist > pCtx->m_FadeSqDist))
			{
				model.SetAlpha( pCtx->m_FalloffFactor * (pCtx->m_MaxSqDist - sqDist ) );
			}
			else
			{
				model.SetAlpha( 255 );
			}

			// Perform screen alignment if necessary.
			model.ComputeAngles();
		}
		else
		{
			model.SetAlpha( 0 );
		}
	}

	return true;
}


//-----------------------------------------------------------------------------
// Gets called each view
//-----------------------------------------------------------------------------
void CDetailObjectSystem::BuildDetailObjectRenderLists( )
{
	VPROF_BUDGET( "CDetailObjectSystem::BuildDetailObjectRenderLists", VPROF_BUDGETGROUP_DETAILPROP_RENDERING );
	
	if (!g_pClientMode->ShouldDrawDetailObjects() || (r_DrawDetailProps.GetInt() == 0))
		return;

	// Don't bother doing any of this if the level doesn't have detail props.
	if ( m_DetailObjects.Count() == 0 )
		return;

	// We need to recompute translucency information for all detail props
	for (int i = m_DetailObjectDict.Size(); --i >= 0; )
	{
		if (modelinfo->ModelHasMaterialProxy( m_DetailObjectDict[i].m_pModel ))
		{
			modelinfo->RecomputeTranslucency( m_DetailObjectDict[i].m_pModel );
		}
	}

	float factor = 1.0f;
	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();
	if ( local )
	{
		factor = local->GetFOVDistanceAdjustFactor();
	}

	// Compute factors to optimize rendering of the detail models
	EnumContext_t ctx;
	ctx.m_MaxSqDist = cl_detaildist.GetFloat() * cl_detaildist.GetFloat();
	ctx.m_FadeSqDist = cl_detaildist.GetFloat() - cl_detailfade.GetFloat();

	ctx.m_MaxSqDist /= factor;
	ctx.m_FadeSqDist /= factor;

	if (ctx.m_FadeSqDist > 0)
	{
		ctx.m_FadeSqDist *= ctx.m_FadeSqDist;
	}
	else 
	{
		ctx.m_FadeSqDist = 0;
	}
	ctx.m_FalloffFactor = 255.0f / (ctx.m_MaxSqDist - ctx.m_FadeSqDist);
	ctx.m_BuildWorldListNumber = view->BuildWorldListsNumber();

	ISpatialQuery* pQuery = engine->GetBSPTreeQuery();
	pQuery->EnumerateLeavesInSphere( CurrentViewOrigin(), 
		cl_detaildist.GetFloat(), this, (int)&ctx );
}
