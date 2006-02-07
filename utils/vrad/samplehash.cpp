//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "vrad.h"
#include "lightmap.h"

#define SAMPLEHASH_NUM_BUCKETS			65536
#define SAMPLEHASH_GROW_SIZE			0
#define SAMPLEHASH_INIT_SIZE			0

int samplesAdded = 0;
int patchSamplesAdded = 0;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool SampleData_CompareFunc( SampleData_t const &src1, SampleData_t const &src2 )
{
	return ( ( src1.x == src2.x ) &&
		     ( src1.y == src2.y ) &&
			 ( src1.z == src2.z ) );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned int SampleData_KeyFunc( SampleData_t const &src )
{
	return ( src.x + src.y + src.z );
}


CUtlHash<SampleData_t> g_SampleHashTable( SAMPLEHASH_NUM_BUCKETS, 
										  SAMPLEHASH_GROW_SIZE, 
										  SAMPLEHASH_INIT_SIZE, 
										  SampleData_CompareFunc, SampleData_KeyFunc );



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UtlHashHandle_t SampleData_Find( sample_t *pSample )
{
	SampleData_t sampleData;	
	sampleData.x = ( int )( pSample->pos.x / SAMPLEHASH_VOXEL_SIZE ) * 100;
	sampleData.y = ( int )( pSample->pos.y / SAMPLEHASH_VOXEL_SIZE ) * 10;
	sampleData.z = ( int )( pSample->pos.z / SAMPLEHASH_VOXEL_SIZE );

	return g_SampleHashTable.Find( sampleData );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UtlHashHandle_t SampleData_InsertIntoHashTable( sample_t *pSample, SampleHandle_t sampleHandle )
{
	SampleData_t sampleData;
	sampleData.x = ( int )( pSample->pos.x / SAMPLEHASH_VOXEL_SIZE ) * 100;
	sampleData.y = ( int )( pSample->pos.y / SAMPLEHASH_VOXEL_SIZE ) * 10;
	sampleData.z = ( int )( pSample->pos.z / SAMPLEHASH_VOXEL_SIZE );

	UtlHashHandle_t handle = g_SampleHashTable.AllocEntryFromKey( sampleData );

	SampleData_t *pSampleData = &g_SampleHashTable.Element( handle );
	pSampleData->x = sampleData.x;
	pSampleData->y = sampleData.y;
	pSampleData->z = sampleData.z;
	pSampleData->m_Samples.AddToTail( sampleHandle );

	samplesAdded++;

	return handle;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UtlHashHandle_t SampleData_AddSample( sample_t *pSample, SampleHandle_t sampleHandle )
{

	// find the key -- if it doesn't exist add new sample data to the
	// hash table
	UtlHashHandle_t handle = SampleData_Find( pSample );
	if( handle == g_SampleHashTable.InvalidHandle() )
	{
		handle = SampleData_InsertIntoHashTable( pSample, sampleHandle );
	}
	else
	{
		SampleData_t *pSampleData = &g_SampleHashTable.Element( handle );
		pSampleData->m_Samples.AddToTail( sampleHandle );

		samplesAdded++;
	}

	return handle;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void SampleData_Log( void )
{
	if( g_bLogHashData )
	{
		g_SampleHashTable.Log( "samplehash.txt" );
	}
}


//=============================================================================
//=============================================================================
//
// PatchSample Functions
//
//=============================================================================
//=============================================================================

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool PatchSampleData_CompareFunc( PatchSampleData_t const &src1, PatchSampleData_t const &src2 )
{
	return ( ( src1.x == src2.x ) &&
		     ( src1.y == src2.y ) &&
			 ( src1.z == src2.z ) );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned int PatchSampleData_KeyFunc( PatchSampleData_t const &src )
{
	return ( src.x + src.y + src.z );
}


CUtlHash<PatchSampleData_t>	g_PatchSampleHashTable( SAMPLEHASH_NUM_BUCKETS,
												    SAMPLEHASH_GROW_SIZE,
													SAMPLEHASH_INIT_SIZE,
													PatchSampleData_CompareFunc, PatchSampleData_KeyFunc );

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UtlHashHandle_t PatchSampleData_Find( patch_t *pPatch )
{
	PatchSampleData_t patchData;	
	patchData.x = ( int )( pPatch->origin.x / SAMPLEHASH_VOXEL_SIZE ) * 100;
	patchData.y = ( int )( pPatch->origin.y / SAMPLEHASH_VOXEL_SIZE ) * 10;
	patchData.z = ( int )( pPatch->origin.z / SAMPLEHASH_VOXEL_SIZE );

	return g_PatchSampleHashTable.Find( patchData );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UtlHashHandle_t PatchSampleData_InsertIntoHashTable( patch_t *pPatch, int ndxPatch )
{
	PatchSampleData_t patchData;	
	patchData.x = ( int )( pPatch->origin.x / SAMPLEHASH_VOXEL_SIZE ) * 100;
	patchData.y = ( int )( pPatch->origin.y / SAMPLEHASH_VOXEL_SIZE ) * 10;
	patchData.z = ( int )( pPatch->origin.z / SAMPLEHASH_VOXEL_SIZE );

	UtlHashHandle_t handle = g_PatchSampleHashTable.AllocEntryFromKey( patchData );

	PatchSampleData_t *pPatchData = &g_PatchSampleHashTable.Element( handle );
	pPatchData->x = patchData.x;
	pPatchData->y = patchData.y;
	pPatchData->z = patchData.z;
	pPatchData->m_ndxPatches.AddToTail( ndxPatch );

	patchSamplesAdded++;

	return handle;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UtlHashHandle_t PatchSampleData_AddSample( patch_t *pPatch, int ndxPatch )
{

	// find the key -- if it doesn't exist add new sample data to the
	// hash table
	UtlHashHandle_t handle = PatchSampleData_Find( pPatch );
	if( handle == g_PatchSampleHashTable.InvalidHandle() )
	{
		handle = PatchSampleData_InsertIntoHashTable( pPatch, ndxPatch );
	}
	else
	{
		PatchSampleData_t *pPatchData = &g_PatchSampleHashTable.Element( handle );
		pPatchData->m_ndxPatches.AddToTail( ndxPatch );

		patchSamplesAdded++;
	}

	return handle;
}

