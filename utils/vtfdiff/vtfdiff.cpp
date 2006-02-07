//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "vtf/vtf.h"
#include "UtlBuffer.h"
#include "imageloader.h"
#include "vector.h"
#include <conio.h>

void Usage( void )
{
	printf( "Usage: vtfdiff file1.vtf file2.vtf\n" );
	exit( -1 );
}

void LoadFileIntoBuffer( const char *pFileName, CUtlBuffer &buf )
{
	struct	_stat statBuf;
	if( _stat( pFileName, &statBuf ) != 0 )
	{
		goto error;
	}

	buf.EnsureCapacity( statBuf.st_size );
	FILE *fp;
	fp = fopen( pFileName, "rb" );
	if( !fp )
	{
		goto error;
	}
	fread( buf.Base(), 1, statBuf.st_size, fp );
	fclose( fp );

	return;
error:
	printf( "Can't find file %s\n", pFileName );
	exit( -1 );
}

void PrintFlags( int flags )
{
	if( flags & TEXTUREFLAGS_POINTSAMPLE )
	{
		printf( "POINTSAMPLE|" );
	}
	if( flags & TEXTUREFLAGS_POINTSAMPLE )
	{
		printf( "POINTSAMPLE|" );
	}
	if( flags & TEXTUREFLAGS_TRILINEAR )
	{
		printf( "TRILINEAR" );
	}
	if( flags & TEXTUREFLAGS_CLAMPS )
	{
		printf( "CLAMPS|" );
	}
	if( flags & TEXTUREFLAGS_CLAMPT )
	{
		printf( "CLAMPT|" );
	}
	if( flags & TEXTUREFLAGS_ANISOTROPIC )
	{
		printf( "ANISOTROPIC|" );
	}
	if( flags & TEXTUREFLAGS_HINT_DXT5 )
	{
		printf( "HINT_DXT5|" );
	}
	if( flags & TEXTUREFLAGS_NOCOMPRESS )
	{
		printf( "NOCOMPRESS|" );
	}
	if( flags & TEXTUREFLAGS_NORMAL )
	{
		printf( "NORMAL|" );
	}
	if( flags & TEXTUREFLAGS_NOMIP )
	{
		printf( "NOMIP|" );
	}
	if( flags & TEXTUREFLAGS_NOLOD )
	{
		printf( "NOLOD|" );
	}
	if( flags & TEXTUREFLAGS_MINMIP )
	{
		printf( "MINMIP|" );
	}
	if( flags & TEXTUREFLAGS_PROCEDURAL )
	{
		printf( "PROCEDURAL|" );
	}
	if( flags & TEXTUREFLAGS_ONEBITALPHA )
	{
		printf( "ONEBITALPHA|" );
	}
	if( flags & TEXTUREFLAGS_EIGHTBITALPHA )
	{
		printf( "EIGHTBITALPHA|" );
	}
	if( flags & TEXTUREFLAGS_ENVMAP )
	{
		printf( "ENVMAP|" );
	}
	if( flags & TEXTUREFLAGS_RENDERTARGET )
	{
		printf( "RENDERTARGET|" );
	}
	if( flags & TEXTUREFLAGS_DEPTHRENDERTARGET )
	{
		printf( "DEPTHRENDERTARGET|" );
	}
	if( flags & TEXTUREFLAGS_NODEBUGOVERRIDE )
	{
		printf( "NODEBUGOVERRIDE|" );
	}
	if( flags & TEXTUREFLAGS_SINGLECOPY )
	{
		printf( "SINGLECOPY|" );
	}
}

int main( int argc, char **argv )
{
	if( argc != 3 )
	{
		Usage();
	}

	CUtlBuffer file1;
	CUtlBuffer file2;

	LoadFileIntoBuffer( argv[1], file1 );
	LoadFileIntoBuffer( argv[2], file2 );

	// HACK!  zero out the part that is typically garbage in the vtf file due to alignment of VectorAligned.
	( ( int * )file1.Base() )[7] = 0;
	( ( int * )file1.Base() )[11] = 0;
	( ( int * )file2.Base() )[7] = 0;
	( ( int * )file2.Base() )[11] = 0;

	// zero out the version on both since we really don't care what version the file format is.
	( ( int * )file1.Base() )[2] = 0;	
	( ( int * )file1.Base() )[3] = 0;
	( ( int * )file2.Base() )[2] = 0;	
	( ( int * )file2.Base() )[3] = 0;

	// zero out the startframe. . is it ever used for anything?  I don't think so.
	( ( unsigned char * )file1.Base() )[26] = 0;
	( ( unsigned char * )file1.Base() )[27] = 0;
	( ( unsigned char * )file2.Base() )[26] = 0;
	( ( unsigned char * )file2.Base() )[27] = 0;

	// the header is 63 bytes, so we sometimes end up with an extra byte worth of crap at the end. . makes no sense since
	// we are aligned to the byte, but oh well. . may be from old code.
	( ( unsigned char * )file1.Base() )[63] = 0;
	( ( unsigned char * )file2.Base() )[63] = 0;
	
	IVTFTexture *pTexture1 = CreateVTFTexture();
	IVTFTexture *pTexture2 = CreateVTFTexture();

	bool bMatch = true;

	if( !pTexture1->Unserialize( file1 ) )
	{
		printf( "error loading %s\n", argv[1] );
		exit( -1 );
	}
	if( !pTexture2->Unserialize( file2 ) )
	{
		printf( "error loading %s\n", argv[2] );
		exit( -1 );
	}

	if( pTexture1->Width() != pTexture2->Width() ||
		pTexture1->Height() != pTexture2->Height() )
	{
		printf( "%s dimensions differ: %dx%d != %dx%d\n", 
			argv[1],
			( int )pTexture1->Width(), ( int )pTexture1->Height(),
			( int )pTexture2->Width(), ( int )pTexture2->Height() );
		bMatch = false;
	}

	if( pTexture1->LowResWidth() != pTexture2->LowResWidth() ||
		pTexture1->LowResHeight() != pTexture2->LowResHeight() )
	{
		printf( "%s lowres dimensions differ: %dx%d != %dx%d\n", 
			argv[1],
			( int )pTexture1->LowResWidth(), ( int )pTexture1->LowResHeight(),
			( int )pTexture2->LowResWidth(), ( int )pTexture2->LowResHeight() );
		bMatch = false;
	}

	if( pTexture1->MipCount() != pTexture2->MipCount() )
	{
		printf( "%s differing mipcounts: %d != %d\n", 
			argv[1],
			( int )pTexture1->MipCount(), ( int )pTexture2->MipCount() );
		bMatch = false;
	}

	if( pTexture1->FaceCount() != pTexture2->FaceCount() )
	{
		printf( "%s differing facecount: %d != %d\n", 
			argv[1],
			( int )pTexture1->FaceCount(), ( int )pTexture2->FaceCount() );
		bMatch = false;
	}

	if( pTexture1->FrameCount() != pTexture2->FrameCount() )
	{
		printf( "%s differing framecount: %d != %d\n", 
			argv[1],
			( int )pTexture1->FrameCount(), ( int )pTexture2->FrameCount() );
		bMatch = false;
	}
	
	if( pTexture1->Flags() != pTexture2->Flags() )
	{
		printf( "%s differing flags: \"", 
			argv[1] );
		PrintFlags( pTexture1->Flags() );
		printf( "\" != \"" );
		PrintFlags( pTexture2->Flags() );
		printf( "\"\n" );
		bMatch = false;
	}
	
	if( pTexture1->BumpScale() != pTexture2->BumpScale() )
	{
		printf( "%s differing bumpscale: %f != %f\n", 
			argv[1],
			( float )pTexture1->BumpScale(), ( float )pTexture2->BumpScale() );
		bMatch = false;
	}
	
	if( pTexture1->Format() != pTexture2->Format() )
	{
		printf( "%s differing image format: %s!=%s\n", 
			argv[1],
			ImageLoader::GetName( pTexture1->Format() ),
			ImageLoader::GetName( pTexture2->Format() ) );
		bMatch = false;
	}
	
	if( pTexture1->LowResFormat() != pTexture2->LowResFormat() )
	{
		printf( "%s differing lowres image format: %s!=%s\n", 
			argv[1],
			ImageLoader::GetName( pTexture1->LowResFormat() ),
			ImageLoader::GetName( pTexture2->LowResFormat() ) );
		bMatch = false;
	}

	const Vector &vReflectivity1 = pTexture1->Reflectivity();
	const Vector &vReflectivity2 = pTexture2->Reflectivity();
	if( !VectorsAreEqual( vReflectivity1, vReflectivity2, 0.0001f ) )
	{
		printf( "%s differing reflectivity: [%f,%f,%f]!=[%f,%f,%f]\n", 
			argv[1],
			( float )pTexture1->Reflectivity()[0],
			( float )pTexture1->Reflectivity()[1],
			( float )pTexture1->Reflectivity()[2],
			( float )pTexture2->Reflectivity()[0],
			( float )pTexture2->Reflectivity()[1],
			( float )pTexture2->Reflectivity()[2] );
		bMatch = false;
	}
	
	if( bMatch && ( pTexture1->ComputeTotalSize() == pTexture2->ComputeTotalSize() ) )
	{
		if( memcmp( pTexture1->ImageData(), pTexture2->ImageData(), 
			pTexture1->ComputeTotalSize() ) != 0 )
		{
			printf( "%s data different\n",
				argv[1] );
			int i;
			for( i = 0; i < pTexture1->ComputeTotalSize(); i++ )
			{
				if( pTexture1->ImageData()[i] != pTexture2->ImageData()[i] )
				{
					printf( "offset %d different\n", i );
				}
			}
			bMatch = false;
		}
	}

	if( bMatch )
	{
		if( file1.Size() != file2.Size() )
		{
			printf( "%s differing file sizes\n" );
		}
		else
		{
			if( memcmp( file1.Base(), file2.Base(), file1.Size() ) == 0 )
			{
//				printf( "%s MATCH!\n", argv[1] );
				bMatch = true;
			}
			else
			{
				printf( "%s data mismatch\n", argv[1] );
				int i;
				unsigned char *pData1 = ( unsigned char * )file1.Base();
				unsigned char *pData2 = ( unsigned char * )file2.Base();
				for( i = 0; i < pTexture1->ComputeTotalSize(); i++ )
				{
					if( pData1[i] != pData2[i] )
					{
						printf( "offset %d different (0x%x != 0x%x)\n", i, ( int )pData1[i], ( int )pData2[i] );
					}
				}
				bMatch = false;
			}
		}
	}

	if( bMatch )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
