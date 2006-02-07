//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: static_prop - don't move, don't animate, don't do anything.
//			physics_prop - move, take damage, but don't animate
//
//=============================================================================//

#include "cbase.h"
#include "props_shared.h"
#include "filesystem.h"
#include "animation.h"
#include <vcollide_parse.h>
#include <bone_setup.h>

#ifdef CLIENT_DLL
#include "gamestringpool.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sv_pushaway_clientside_size( "sv_pushaway_clientside_size", "15", FCVAR_REPLICATED, "Minimum size of pushback objects" );
ConVar props_break_max_pieces( "props_break_max_pieces", "-1", 0, "Maximum prop breakable piece count (-1 = model default)" );

extern bool PropBreakableCapEdictsOnCreateAll(int modelindex, IPhysicsObject *pPhysics, const breakablepropparams_t &params, CBaseEntity *pEntity, int iPrecomputedBreakableCount = -1 );
extern CBaseEntity *BreakModelCreateSingle( CBaseEntity *pOwner, breakmodel_t *pModel, const Vector &position, 
	const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, int nSkin, const breakablepropparams_t &params );


//=============================================================================================================
// UTILITY FUNCS
//=============================================================================================================
//-----------------------------------------------------------------------------
// Purpose: returns the axis index with the greatest size
// Input  : &vec - 
// Output : static int
//-----------------------------------------------------------------------------
static int GreatestAxis( const Vector &vec )
{
	if ( vec.x > vec.y )
	{
		if ( vec.x > vec.z )
			return 0;
		return 2;
	}
	if ( vec.y > vec.z )
		return 1;
	return 2;
}

//-----------------------------------------------------------------------------
// Purpose: returns the axis index with the smallest size
// Input  : &vec - 
// Output : static int
//-----------------------------------------------------------------------------
static int SmallestAxis( const Vector &vec )
{
	if ( vec.x < vec.y )
	{
		if ( vec.x < vec.z )
			return 0;
		return 2;
	}
	if ( vec.y < vec.z )
		return 1;
	return 2;
}

//-----------------------------------------------------------------------------
// Purpose: Rotates a matrix by 90 degrees in the plane of axis0/axis1
// Input  : &matrix - 
//			axis0 - 
//			axis1 - 
// Output : static void
//-----------------------------------------------------------------------------
static void MatrixRot90( matrix3x4_t &matrix, int axis0, int axis1 )
{
	Vector col0, col1;
	MatrixGetColumn( matrix, axis0, col0 );
	MatrixGetColumn( matrix, axis1, col1 );
	MatrixSetColumn( col1, axis0, matrix );
	MatrixSetColumn( -col0, axis1, matrix );
}
 
//-----------------------------------------------------------------------------
// Purpose: Given two symmetric boxes, rotate the coordinate frame by the necessary
//			90 degree rotations to approximately align them
// Input  : *pInOutMatrix - 
//			&boxExtents1 - 
//			&boxExtents2 - 
//-----------------------------------------------------------------------------
static void AlignBoxes( matrix3x4_t *pInOutMatrix, const Vector &boxExtents1, const Vector &boxExtents2 )
{
	int rotCount = 0;
	struct 
	{
		int axis0;
		int axis1;
	} rotations[2];
	Vector ext1 = boxExtents1;
	Vector ext2 = boxExtents2;

	int axis0 = GreatestAxis( ext1 );
	int axis1 = GreatestAxis( ext2 );
	if ( axis0 != axis1 )
	{
		rotations[rotCount].axis0 = axis0;
		rotations[rotCount].axis1 = axis1;
		rotCount++;
		ext2[axis1] = ext2[axis0];
	}
	ext1[axis0] = 0;
	ext2[axis0] = 0;

	axis0 = GreatestAxis(ext1);
	axis1 = GreatestAxis(ext2);
	if ( axis0 != axis1 )
	{
		rotations[rotCount].axis0 = axis0;
		rotations[rotCount].axis1 = axis1;
		rotCount++;
	}

	while ( rotCount > 0 )
	{
		rotCount--;
		MatrixRot90( *pInOutMatrix, rotations[rotCount].axis0, rotations[rotCount].axis1 );
	}
}

//=============================================================================================================
// PROP DATA
//=============================================================================================================
CPropData g_PropDataSystem;

// Parsing details for each of the propdata interactions
struct propdata_interaction_s
{
	const char *pszSectionName;
	const char *pszKeyName;
	const char *pszValue;
};

propdata_interaction_s sPropdataInteractionSections[PROPINTER_NUM_INTERACTIONS] =
{
	{ "physgun_interactions", "onworldimpact", "stick" },		// PROPINTER_PHYSGUN_WORLD_STICK,
	{ "physgun_interactions", "onfirstimpact", "break" },		// PROPINTER_PHYSGUN_FIRST_BREAK,
	{ "physgun_interactions", "onfirstimpact", "paintsplat" },	// PROPINTER_PHYSGUN_FIRST_PAINT,
	{ "physgun_interactions", "onfirstimpact", "impale" },		// PROPINTER_PHYSGUN_FIRST_IMPALE,
	{ "physgun_interactions", "onlaunch", "spin_none" },		// PROPINTER_PHYSGUN_LAUNCH_SPIN_NONE,
	{ "physgun_interactions", "onlaunch", "spin_zaxis" },		// PROPINTER_PHYSGUN_LAUNCH_SPIN_Z,
	{ "physgun_interactions", "onbreak", "explode_fire" },		// PROPINTER_PHYSGUN_BREAK_EXPLODE,
	{ "physgun_interactions", "damage", "none" },				// PROPINTER_PHYSGUN_DAMAGE_NONE,

	{ "fire_interactions", "flammable", "yes" },				// PROPINTER_FIRE_FLAMMABLE,
	{ "fire_interactions", "explosive_resist", "yes" },			// PROPINTER_FIRE_EXPLOSIVE_RESIST,
	{ "fire_interactions", "ignite", "halfhealth" },			// PROPINTER_FIRE_IGNITE_HALFHEALTH,
};

//-----------------------------------------------------------------------------
// Constructor, destructor
//-----------------------------------------------------------------------------
CPropData::CPropData( void )
{
	m_bPropDataLoaded = false;
	m_pKVPropData = NULL;
}

//-----------------------------------------------------------------------------
// Inherited from IAutoServerSystem
//-----------------------------------------------------------------------------
void CPropData::LevelInitPreEntity( void )
{
	m_BreakableChunks.RemoveAll();
	ParsePropDataFile();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropData::Shutdown( void )
{
	if ( m_pKVPropData )
	{
		m_pKVPropData->deleteThis();
	}
}

//-----------------------------------------------------------------------------
// Clear out the stats + their history
//-----------------------------------------------------------------------------
void CPropData::ParsePropDataFile( void )
{
	m_pKVPropData = new KeyValues( "PropDatafile" );
	if ( !m_pKVPropData->LoadFromFile( filesystem, "scripts/propdata.txt" ) )
	{
		m_pKVPropData->deleteThis();
		return;
	}

	m_bPropDataLoaded = true;

	// Now try and parse out the breakable section
	KeyValues *pBreakableSection = m_pKVPropData->FindKey( "BreakableModels" );
	if ( pBreakableSection )
	{
		KeyValues *pChunkSection = pBreakableSection->GetFirstSubKey();
		while ( pChunkSection )
		{
			// Create a new chunk section and add it to our list
			int index = m_BreakableChunks.AddToTail();
			propdata_breakablechunk_t *pBreakableChunk = &m_BreakableChunks[index];
			pBreakableChunk->iszChunkType = AllocPooledString( pChunkSection->GetName() );

			// Read in all the model names
			KeyValues *pModelName = pChunkSection->GetFirstSubKey();
			while ( pModelName )
			{
				const char *pModel = pModelName->GetName();
				string_t pooledName = AllocPooledString( pModel );
				pBreakableChunk->iszChunkModels.AddToTail( pooledName );
				CBaseEntity::PrecacheModel( STRING( pooledName ) );

				pModelName = pModelName->GetNextKey();
			}

			pChunkSection = pChunkSection->GetNextKey();
		}
	}
}	

//-----------------------------------------------------------------------------
// Purpose: Parse a keyvalues section into the prop
//
//			pInteractionSection is a bit of jiggery-pokery to get around the unfortunate
//			fact that the interaction KV sections ("physgun_interactions", "fire_interactions", etc)
//			are OUTSIDE the "prop_data" KV section in the model, but may be contained WITHIN the 
//			specified Base's "prop_data" section (i.e. in propdata.txt)
//-----------------------------------------------------------------------------
int CPropData::ParsePropFromKV( CBaseEntity *pProp, KeyValues *pSection, KeyValues *pInteractionSection )
{
	IBreakableWithPropData *pBreakableInterface = dynamic_cast<IBreakableWithPropData*>(pProp);
	if ( !pBreakableInterface )
		return PARSE_FAILED_BAD_DATA;

	if ( !pBreakableInterface )
		return PARSE_FAILED_BAD_DATA;

	int iBaseResult = PARSE_SUCCEEDED;

	// Do we have a base?
	char const *pszBase = pSection->GetString( "base" );
	if ( pszBase && pszBase[0] )
	{
		iBaseResult = ParsePropFromBase( pProp, pszBase );
		if ( (iBaseResult != PARSE_SUCCEEDED) && (iBaseResult != PARSE_SUCCEEDED_ALLOWED_STATIC) )
			return iBaseResult;
	}

	// Allow overriding of Block LOS
	int iBlockLOS = pSection->GetFloat( "blockLOS", -1 );
	if ( iBlockLOS != -1 )
	{
		pBreakableInterface->SetPropDataBlocksLOS( iBlockLOS != 0 );
	}

	// Set whether AI can walk on this prop
	int iIsWalkable = pSection->GetFloat( "AIWalkable", -1 );
	if ( iIsWalkable != -1 )
	{
		pBreakableInterface->SetPropDataIsAIWalkable( iIsWalkable != 0 );
	}

	// Set custom damage table
	const char *pszTableName;
	if ( pBreakableInterface->GetPhysicsDamageTable() == NULL_STRING )
	{
		pszTableName = pSection->GetString( "damage_table", NULL );
	}
	else
	{
		pszTableName = pSection->GetString( "damage_table", STRING(pBreakableInterface->GetPhysicsDamageTable()) );
	}
	if ( pszTableName && pszTableName[0] )
	{
		pBreakableInterface->SetPhysicsDamageTable( AllocPooledString( pszTableName ) );
	}
	else
	{
		pBreakableInterface->SetPhysicsDamageTable( NULL_STRING );
	}

	// Get multiplayer physics mode if not set by map
	pBreakableInterface->SetPhysicsMode( pSection->GetInt( "physicsmode", 
		pBreakableInterface->GetPhysicsMode() ) );
		
	// Get damage modifiers, but only if they're specified, because our base may have already overridden them.
	pBreakableInterface->SetDmgModBullet( pSection->GetFloat( "dmg.bullets", pBreakableInterface->GetDmgModBullet() ) );
	pBreakableInterface->SetDmgModClub( pSection->GetFloat( "dmg.club", pBreakableInterface->GetDmgModClub() ) );
	pBreakableInterface->SetDmgModExplosive( pSection->GetFloat( "dmg.explosive", pBreakableInterface->GetDmgModExplosive() ) );

	// Get the health (unless this is an override prop)
	if ( !FClassnameIs( pProp, "prop_physics_override" ) && !FClassnameIs( pProp, "prop_dynamic_override" ) )
	{
		pProp->SetHealth( pSection->GetInt( "health", pProp->GetHealth() ) );

		// Explosive?
		pBreakableInterface->SetExplosiveDamage( pSection->GetFloat( "explosive_damage", pBreakableInterface->GetExplosiveDamage() ) );
		pBreakableInterface->SetExplosiveRadius( pSection->GetFloat( "explosive_radius", pBreakableInterface->GetExplosiveRadius() ) );

#ifdef GAME_DLL
		// If we now have health, we're not allowed to ignore physics damage
		if ( pProp->GetHealth() )
		{
			pProp->RemoveSpawnFlags( SF_PHYSPROP_DONT_TAKE_PHYSICS_DAMAGE );
		}
#endif
	}

	const char *pszBreakableModel;
	if ( pBreakableInterface->GetBreakableModel() == NULL_STRING )
	{
		pszBreakableModel = pSection->GetString( "breakable_model", NULL );
	}
	else
	{
		pszBreakableModel = pSection->GetString( "breakable_model", STRING(pBreakableInterface->GetBreakableModel()) );
	}
	if ( pszBreakableModel && pszBreakableModel[0] )
	{
		pBreakableInterface->SetBreakableModel( AllocPooledString( pszBreakableModel ) );
	}
	else
	{
		pBreakableInterface->SetBreakableModel( NULL_STRING );
	}
	pBreakableInterface->SetBreakableSkin( pSection->GetInt( "breakable_skin", pBreakableInterface->GetBreakableSkin() ) );
	pBreakableInterface->SetBreakableCount( pSection->GetInt( "breakable_count", pBreakableInterface->GetBreakableCount() ) );

	// Calculate the maximum size of the breakables this breakable will produce
	Vector vecSize = pProp->CollisionProp()->OBBSize();
	// Throw away the smallest coord
	int iSmallest = SmallestAxis(vecSize);
	vecSize[iSmallest] = 1;
	float flVolume = vecSize.x * vecSize.y * vecSize.z;
	int iMaxSize = floor( flVolume / (32.0*32.0) );
	pBreakableInterface->SetMaxBreakableSize( iMaxSize );

	// Now parse our interactions
	for ( int i = 0; i < PROPINTER_NUM_INTERACTIONS; i++ )
	{
		// If we hit this assert, we have too many interactions for our current storage solution to handle
		Assert( i < 32 );

		propdata_interaction_s *pInteraction = &sPropdataInteractionSections[i];

		KeyValues *pkvCurrentInter = pInteractionSection->FindKey( pInteraction->pszSectionName );
		if ( pkvCurrentInter )
		{
			char const *pszInterBase = pkvCurrentInter->GetString( pInteraction->pszKeyName );
			if ( pszInterBase && pszInterBase[0] && !stricmp( pszInterBase, pInteraction->pszValue ) )
			{
				pBreakableInterface->SetInteraction( (propdata_interactions_t)i );
			}
		}
	}

	// If the base said we're allowed to be static, return that
	if ( iBaseResult == PARSE_SUCCEEDED_ALLOWED_STATIC )
		return PARSE_SUCCEEDED_ALLOWED_STATIC;

	// Otherwise, see if our propdata says we are allowed to be static
	if ( pSection->GetInt( "allowstatic", 0 ) )
		return PARSE_SUCCEEDED_ALLOWED_STATIC;

	return PARSE_SUCCEEDED;
}

//-----------------------------------------------------------------------------
// Purpose: Fill out a prop's with base data parsed from the propdata file
//-----------------------------------------------------------------------------
int CPropData::ParsePropFromBase( CBaseEntity *pProp, const char *pszPropData )
{
	if ( !m_bPropDataLoaded )
		return PARSE_FAILED_NO_DATA;

	IBreakableWithPropData *pBreakableInterface = dynamic_cast<IBreakableWithPropData*>(pProp);
	
	if ( !pBreakableInterface )
	{
		return PARSE_FAILED_BAD_DATA;
	}
	
	// Find the specified propdata
	KeyValues *pSection = m_pKVPropData->FindKey( pszPropData );
	if ( !pSection )
	{
		Warning("%s '%s' has a base specified as '%s', but there is no matching entry in propdata.txt.\n", pProp->GetClassname(), STRING( pProp->GetModelName() ), pszPropData );
		return PARSE_FAILED_BAD_DATA;
	}

	// Store off the first base data for debugging
	if ( pBreakableInterface->GetBasePropData() == NULL_STRING )
	{
		pBreakableInterface->SetBasePropData( AllocPooledString( pszPropData ) );
	}

	return ParsePropFromKV( pProp, pSection, pSection );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CPropData::GetRandomChunkModel( const char *pszBreakableSection, int iMaxSize )
{
	if ( !m_bPropDataLoaded )
		return NULL;

	// Find the right section
	int iCount = m_BreakableChunks.Count();
	int i;
	for ( i = 0; i < iCount; i++ )
	{
		if ( !Q_strncmp( STRING(m_BreakableChunks[i].iszChunkType), pszBreakableSection, strlen(pszBreakableSection) ) )
			break;
	}
	if ( i == iCount )
		return NULL;

	// Now pick a random one and return it
	int iRandom;
	if ( iMaxSize == -1 )
	{
		iRandom = RandomInt( 0, m_BreakableChunks[i].iszChunkModels.Count()-1 );
	}
	else
	{
		// Don't pick anything over the specified size
		iRandom = RandomInt( 0, min(iMaxSize, m_BreakableChunks[i].iszChunkModels.Count()-1) );
	}

	return STRING(m_BreakableChunks[i].iszChunkModels[iRandom]);
}

//-----------------------------------------------------------------------------
// breakable prop functions
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
// list of models to break into

class CBreakParser : public IVPhysicsKeyHandler
{
public:
	CBreakParser( float defaultBurstScale, int defaultCollisionGroup ) 
		: m_defaultBurstScale(defaultBurstScale), m_defaultCollisionGroup(defaultCollisionGroup) {}

	void ParseModelName( breakmodel_t *pModel, const char *pValue )
	{
		char tmp[1024];

		Q_strncpy( tmp, pValue, sizeof(pModel->modelName) );
		if ( Q_strnicmp( tmp, "models/", 7 ) )
		{
			Q_snprintf( pModel->modelName, sizeof(pModel->modelName), "models/%s" , tmp );
		}
		else
		{
			Q_strncpy( pModel->modelName, tmp, sizeof(pModel->modelName));
		}
		int len = Q_strlen(pModel->modelName);
		if ( len < 4 || Q_stricmp( pModel->modelName + (len-4), ".mdl" ) )
		{
			Q_strncat( pModel->modelName, ".mdl", sizeof(pModel->modelName), COPY_ALL_CHARACTERS );
		}
	}
	virtual void ParseKeyValue( void *pData, const char *pKey, const char *pValue )
	{
		breakmodel_t *pModel = (breakmodel_t *)pData;
		if ( !strcmpi( pKey, "model" ) )
		{
			ParseModelName( pModel, pValue );
		}
		else if (!strcmpi( pKey, "ragdoll" ) )
		{
			ParseModelName( pModel, pValue );
			pModel->isRagdoll = true;
		}
		else if ( !strcmpi( pKey, "offset" ) )
		{
			UTIL_StringToVector( pModel->offset.Base(), pValue );
		}
		else if ( !strcmpi( pKey, "health" ) )
		{
			pModel->health = atof(pValue);
		}
		else if ( !strcmpi( pKey, "fadetime" ) )
		{
			pModel->fadeTime = atof(pValue);
			if ( !m_wroteCollisionGroup )
			{
				pModel->collisionGroup = COLLISION_GROUP_DEBRIS;
			}
		}
		else if ( !strcmpi( pKey, "fademindist" ) )
		{
			pModel->fadeMinDist = atof(pValue);
		}
		else if ( !strcmpi( pKey, "fademaxdist" ) )
		{
			pModel->fadeMaxDist = atof(pValue);
		}
		else if ( !strcmpi( pKey, "debris" ) )
		{
			pModel->collisionGroup = atoi(pValue) > 0 ? COLLISION_GROUP_DEBRIS : COLLISION_GROUP_INTERACTIVE;
			m_wroteCollisionGroup = true;
		}
		else if ( !strcmpi( pKey, "burst" ) )
		{
			pModel->burstScale = atof( pValue );
		}
		else if ( !strcmpi( pKey, "placementbone" ) )
		{
			Q_strncpy( pModel->placementName, pValue, sizeof(pModel->placementName) );
			pModel->placementIsBone = true;
		}
		else if ( !strcmpi( pKey, "placementattachment" ) )
		{
			Q_strncpy( pModel->placementName, pValue, sizeof(pModel->placementName) );
			pModel->placementIsBone = false;
		}
	}
	virtual void SetDefaults( void *pData ) 
	{
		breakmodel_t *pModel = (breakmodel_t *)pData;
		pModel->modelName[0] = 0;
		pModel->offset = vec3_origin;
		pModel->health = 1;
		pModel->fadeTime = 20.0f;
		pModel->fadeMinDist = 0.0f;
		pModel->fadeMaxDist = 0.0f;
		pModel->burstScale = m_defaultBurstScale;
		pModel->collisionGroup = m_defaultCollisionGroup;
		pModel->isRagdoll = false;
		pModel->placementName[0] = 0;
		pModel->placementIsBone = false;
		m_wroteCollisionGroup = false;
	}

private:
	int		m_defaultCollisionGroup;
	float	m_defaultBurstScale;
	bool	m_wroteCollisionGroup;
};

void BreakModelList( CUtlVector<breakmodel_t> &list, int modelindex, float defBurstScale, int defCollisionGroup )
{
	vcollide_t *pCollide = modelinfo->GetVCollide( modelindex );
	if ( !pCollide )
		return;

	IVPhysicsKeyParser *pParse = physcollision->VPhysicsKeyParserCreate( pCollide->pKeyValues );
	while ( !pParse->Finished() )
	{
		CBreakParser breakParser( defBurstScale, defCollisionGroup );
		
		const char *pBlock = pParse->GetCurrentBlockName();
		if ( !strcmpi( pBlock, "break" ) )
		{
			int index = list.AddToTail();
			breakmodel_t &breakModel = list[index];
			pParse->ParseCustom( &breakModel, &breakParser );
		}
		else
		{
			pParse->SkipBlock();
		}
	}
	physcollision->VPhysicsKeyParserDestroy( pParse );
}


int GetAutoMultiplayerPhysicsMode( Vector size, float mass )
{
	float volume = size.x * size.y * size.z;

	float minsize = sv_pushaway_clientside_size.GetFloat();

	// if it's too small, client side only
	if ( volume < (minsize*minsize*minsize) )
		return PHYSICS_MULTIPLAYER_CLIENTSIDE;

	// if it's too light, no player pushback
	if ( mass < 8.0 )
		return PHYSICS_MULTIPLAYER_NON_SOLID;

	// full pushbackmode
	return PHYSICS_MULTIPLAYER_SOLID;
}

//-----------------------------------------------------------------------------
// Purpose: Returns a string describing a real-world equivalent mass.
// Input  : flMass - mass in kg
//-----------------------------------------------------------------------------
const char *GetMassEquivalent(float flMass)
{
	static struct
	{
		float flMass;
		char *sz;
	} masstext[] =
	{
		{ 5e-6,		"snowflake" },
		{ 2.5e-3,	"ping-pong ball" },
		{ 5e-3,		"penny" },
		{ 0.05,		"golf ball" },
		{ 0.17,		"billard ball" },
		{ 2,		"bag of sugar" },
		{ 7,		"male cat" },
		{ 10,		"bowling ball" },
		{ 30,		"dog" },
		{ 60,		"cheetah" },
		{ 90,		"adult male human" },
		{ 250,		"refrigerator" },
		{ 600,		"race horse" },
		{ 1000,		"small car" },
		{ 1650,		"medium car" },
		{ 2500,		"large car" },
		{ 6000,		"t-rex" },
		{ 7200,		"elephant" },
		{ 8e4,		"space shuttle" },
		{ 7e5,		"locomotive" },
		{ 9.2e6,	"Eiffel tower" },
		{ 6e24,		"the Earth" },
		{ 7e24,		"really freaking heavy" },
	};

	for (int i = 0; i < sizeof(masstext) / sizeof(masstext[0]) - 1; i++)
	{
		if (flMass < masstext[i].flMass)
		{
			return masstext[i].sz;
		}
	}

	return masstext[ sizeof(masstext) / sizeof(masstext[0]) - 1 ].sz;
}

void PropBreakableCreateAll( int modelindex, IPhysicsObject *pPhysics, const breakablepropparams_t &params, CBaseEntity *pEntity, int iPrecomputedBreakableCount, bool bIgnoreGibLimit )
{
	int iMaxBreakCount = bIgnoreGibLimit ? -1 : props_break_max_pieces.GetInt();
	if ( iMaxBreakCount != -1 )
	{
		if ( iPrecomputedBreakableCount != -1 )
		{
			iPrecomputedBreakableCount = min( iMaxBreakCount, iPrecomputedBreakableCount );
		}
		else
		{
			iPrecomputedBreakableCount = iMaxBreakCount;
		}
	}

#ifdef GAME_DLL
	// On server limit break model creation
	if ( !PropBreakableCapEdictsOnCreateAll(modelindex, pPhysics, params, pEntity, iPrecomputedBreakableCount ) )
	{
		DevMsg( "Failed to create PropBreakable: would exceed MAX_EDICTS\n" );
		return;
	}
#endif
	
	vcollide_t *pCollide = modelinfo->GetVCollide( modelindex );
	if ( !pCollide )
		return;

	int nSkin = 0;
	CBaseEntity *pOwnerEntity = pEntity;
	CBaseAnimating *pOwnerAnim = NULL;
	if ( pPhysics )
	{
		pOwnerEntity = static_cast<CBaseEntity *>(pPhysics->GetGameData());
	}
	if ( pOwnerEntity )
	{
		pOwnerAnim = dynamic_cast<CBaseAnimating*>(pOwnerEntity);
		if ( pOwnerAnim )
		{
			nSkin = pOwnerAnim->m_nSkin;
		}
	}
	matrix3x4_t localToWorld;

	studiohdr_t *pStudioHdr = NULL;
	const model_t *model = modelinfo->GetModel( modelindex );
	if ( model )
	{
		pStudioHdr = static_cast< studiohdr_t * >( modelinfo->GetModelExtraData( model ) );
	}

	Vector parentOrigin = vec3_origin;
	int parentAttachment = 	Studio_FindAttachment( pStudioHdr, "placementOrigin" ) + 1;
	if ( parentAttachment > 0 )
	{
		GetAttachmentLocalSpace( pStudioHdr, parentAttachment-1, localToWorld );
		MatrixGetColumn( localToWorld, 3, parentOrigin );
	}
	else
	{
		AngleMatrix( vec3_angle, localToWorld );
	}
	
	CUtlVector<breakmodel_t> list;

	BreakModelList( list, modelindex, params.defBurstScale, params.defCollisionGroup );

	if ( list.Count() )
	{
		for ( int i = 0; i < list.Count(); i++ )
		{
			int modelIndex = modelinfo->GetModelIndex( list[i].modelName );
			if ( modelIndex <= 0 )
				continue;
			
			if ( ( iPrecomputedBreakableCount != -1 ) && ( i >= iPrecomputedBreakableCount ) )
				break;

			matrix3x4_t matrix;
			AngleMatrix( params.angles, params.origin, matrix );

			studiohdr_t *pStudioHdr = NULL;
			const model_t *model = modelinfo->GetModel( modelIndex );
			if ( model )
			{
				pStudioHdr = static_cast< studiohdr_t * >( modelinfo->GetModelExtraData( model ) );
			}

			Vector position = vec3_origin;
			QAngle angles = params.angles;
			if ( pOwnerAnim && list[i].placementName[0] )
			{
				if ( list[i].placementIsBone )
				{
					int boneIndex = pOwnerAnim->LookupBone( list[i].placementName );
					if ( boneIndex >= 0 )
					{
						pOwnerAnim->GetBonePosition( boneIndex, position, angles );
						AngleMatrix( angles, position, matrix );
					}
				}
				else
				{
					int attachmentIndex = Studio_FindAttachment( pStudioHdr, list[i].placementName ) + 1;
					if ( attachmentIndex > 0 )
					{
						pOwnerAnim->GetAttachment( attachmentIndex, matrix );
						MatrixAngles( matrix, angles );
					}
				}
			}
			else
			{
				int placementIndex = Studio_FindAttachment( pStudioHdr, "placementOrigin" ) + 1;
				Vector placementOrigin = parentOrigin;
				if ( placementIndex > 0 )
				{
					GetAttachmentLocalSpace( pStudioHdr, placementIndex-1, localToWorld );
					MatrixGetColumn( localToWorld, 3, placementOrigin );
					placementOrigin -= parentOrigin;
				}

				VectorTransform( list[i].offset - placementOrigin, matrix, position );
			}
			Vector objectVelocity = params.velocity;

			if (pPhysics)
			{
				pPhysics->GetVelocityAtPoint( position, objectVelocity );
			}

			int nActualSkin = nSkin;
			if ( nActualSkin > pStudioHdr->numskinfamilies )
				nActualSkin = 0;

			CBaseEntity *pBreakable = BreakModelCreateSingle( pOwnerEntity, &list[i], position, angles, objectVelocity, params.angularVelocity, nActualSkin, params );

			// If burst scale is set, this piece should 'burst' away from
			// the origin in addition to travelling in the wished velocity.
			if( pBreakable && ( list[i].burstScale != 0.0 ) )
			{
				Vector vecBurstDir = position - params.origin;

				// If $autocenter wasn't used, try the center of the piece
				if ( vecBurstDir == vec3_origin )
				{
					vecBurstDir = pBreakable->WorldSpaceCenter() - params.origin;
				}

				VectorNormalize( vecBurstDir );

				pBreakable->ApplyAbsVelocityImpulse( vecBurstDir * list[i].burstScale );
			}
		}
	}
	// Then see if the propdata specifies any breakable pieces
	else if ( pEntity )
	{
		IBreakableWithPropData *pBreakableInterface = dynamic_cast<IBreakableWithPropData*>(pEntity);
		if ( pBreakableInterface && pBreakableInterface->GetBreakableModel() != NULL_STRING && pBreakableInterface->GetBreakableCount() )
		{
			breakmodel_t breakModel;

			for ( int i = 0; i < pBreakableInterface->GetBreakableCount(); i++ )
			{
				if ( ( iPrecomputedBreakableCount != -1 ) && ( i >= iPrecomputedBreakableCount ) )
					break;

				Q_strncpy( breakModel.modelName, g_PropDataSystem.GetRandomChunkModel(STRING(pBreakableInterface->GetBreakableModel()), pBreakableInterface->GetMaxBreakableSize()), sizeof(breakModel.modelName) );

				breakModel.health = 1;
				breakModel.fadeTime = RandomFloat(5,10);
				breakModel.fadeMinDist = 0.0f;
				breakModel.fadeMaxDist = 0.0f;
				breakModel.burstScale = params.defBurstScale;
				breakModel.collisionGroup = COLLISION_GROUP_DEBRIS;
				breakModel.isRagdoll = false;
				breakModel.placementName[0] = 0;
				breakModel.placementIsBone = false;

				Vector vecObbSize = pEntity->CollisionProp()->OBBSize();

				// Find a random point on the plane of the original's two largest axis
				int smallestAxis = SmallestAxis( vecObbSize );
				Vector vecMins(0,0,0);
				Vector vecMaxs(1,1,1);
				vecMins[smallestAxis] = 0.5;
				vecMaxs[smallestAxis] = 0.5;
				pEntity->CollisionProp()->RandomPointInBounds( vecMins, vecMaxs, &breakModel.offset );

				// Push all chunks away from the center
				Vector vecBurstDir = breakModel.offset - params.origin;
				VectorNormalize( vecBurstDir );
				Vector vecVelocity = vecBurstDir * params.defBurstScale;

				QAngle vecAngles = pEntity->GetAbsAngles();
				int iSkin = pBreakableInterface->GetBreakableSkin();
				CBaseEntity *pBreakable = BreakModelCreateSingle( pOwnerEntity, &breakModel, breakModel.offset, vecAngles, vecVelocity, vec3_origin/*params.angularVelocity*/, iSkin, params );

				if( pBreakable )
				{
					Vector vecBreakableObbSize = pBreakable->CollisionProp()->OBBSize();

					// Try to align the gibs along the original axis 
					matrix3x4_t matrix;
					AngleMatrix( vecAngles, matrix );
					AlignBoxes( &matrix, vecObbSize, vecBreakableObbSize );
					MatrixAngles( matrix, vecAngles );

					if ( pBreakable->VPhysicsGetObject() )
					{
						Vector pos;
						pBreakable->VPhysicsGetObject()->GetPosition( &pos, NULL );
						pBreakable->VPhysicsGetObject()->SetPosition( pos, vecAngles, true );
					}

					pBreakable->SetAbsAngles( vecAngles );
				}
				else
				{
					DevWarning( "PropBreakableCreateAll: Could not create model %s\n", breakModel.modelName );
				}
			}
		}
	}
}


void PropBreakableCreateAll( int modelindex, IPhysicsObject *pPhysics, const Vector &origin, const QAngle &angles, const Vector &velocity, const AngularImpulse &angularVelocity, float impactEnergyScale, float defBurstScale, int defCollisionGroup, CBaseEntity *pEntity )
{
	breakablepropparams_t params( origin, angles, velocity, angularVelocity );
	params.impactEnergyScale = impactEnergyScale;
	params.defBurstScale = defBurstScale;
	params.defCollisionGroup = defCollisionGroup;
	PropBreakableCreateAll( modelindex, pPhysics, params, pEntity, -1, false );
}
