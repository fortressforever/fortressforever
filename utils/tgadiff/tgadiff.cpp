//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include <stdlib.h>
#include <stdio.h>
#include "tgaloader.h"
#include "tgawriter.h"

void Usage( void )
{
	printf( "Usage: tgadiff src1.tga src2.tga diff.tga\n" );
	exit( -1 );
}

int main( int argc, char **argv )
{
	if( argc != 4 )
	{
		Usage();
	}
	const char *pSrcFileName1 = argv[1];
	const char *pSrcFileName2 = argv[2];
	const char *pDstFileName = argv[3];

	int width1, height1;
	ImageFormat imageFormat1;
	float gamma1;
	if( !TGALoader::GetInfo( pSrcFileName1, &width1, &height1, &imageFormat1, &gamma1 ) )
	{
		printf( "error loading %s\n", pSrcFileName1 );
		exit( -1 );
	}

	int width2, height2;
	ImageFormat imageFormat2;
	float gamma2;
	if( !TGALoader::GetInfo( pSrcFileName2, &width2, &height2, &imageFormat2, &gamma2 ) )
	{
		printf( "error loading %s\n", pSrcFileName2 );
		exit( -1 );
	}

	if( width1 != width2 || height1 != height2 )
	{
		printf( "image dimensions different (%dx%d!=%dx%d): can't do diff for %s\n", 
			width1, height1, width2, height2, pDstFileName );
		exit( -1 );
	}
#if 0
	// have to allow for different formats for now due to *.txt file screwup.
	if( imageFormat1 != imageFormat2 )
	{
		printf( "image format different (%s!=%s). . can't do diff for %s\n", 
			ImageLoader::GetName( imageFormat1 ), ImageLoader::GetName( imageFormat2 ), pDstFileName );
		exit( -1 );
	}
#endif
	if( gamma1 != gamma2 )
	{
		printf( "image gamma different (%f!=%f). . can't do diff for %s\n", gamma1, gamma2, pDstFileName );
		exit( -1 );
	}

	unsigned char *pImage1Tmp = new unsigned char[ImageLoader::GetMemRequired( width1, height1, imageFormat1, false )];
	unsigned char *pImage2Tmp = new unsigned char[ImageLoader::GetMemRequired( width2, height2, imageFormat2, false )];

	if( !TGALoader::Load( pImage1Tmp, pSrcFileName1, width1, height1, imageFormat1, 2.2f, false ) )
	{
		printf( "error loading %s\n", pSrcFileName1 );
		exit( -1 );
	}
	
	if( !TGALoader::Load( pImage2Tmp, pSrcFileName2, width2, height2, imageFormat2, 2.2f, false ) )
	{
		printf( "error loading %s\n", pSrcFileName2 );
		exit( -1 );
	}

	unsigned char *pImage1 = new unsigned char[ImageLoader::GetMemRequired( width1, height1, IMAGE_FORMAT_ABGR8888, false )];
	unsigned char *pImage2 = new unsigned char[ImageLoader::GetMemRequired( width2, height2, IMAGE_FORMAT_ABGR8888, false )];
	unsigned char *pDiff = new unsigned char[ImageLoader::GetMemRequired( width2, height2, IMAGE_FORMAT_ABGR8888, false )];
	ImageLoader::ConvertImageFormat( pImage1Tmp, imageFormat1, pImage1, IMAGE_FORMAT_ABGR8888, width1, height1, 0, 0 );
	ImageLoader::ConvertImageFormat( pImage2Tmp, imageFormat2, pImage2, IMAGE_FORMAT_ABGR8888, width2, height2, 0, 0 );

	int i;
	int sizeInBytes = ImageLoader::SizeInBytes( IMAGE_FORMAT_ABGR8888 );
	bool isDifferent = false;
	for( i = 0; i < width1 * height1 * sizeInBytes; i++ )
	{
		int d;
		d = pImage2[i] - pImage1[i];
		pDiff[i] = d > 0 ? d : -d;
		if( d != 0 )
		{
			isDifferent = true;
		}
	}

	if( !isDifferent )
	{
		printf( "Files are the same %s %s : not generating %s\n", pSrcFileName1, pSrcFileName2, pDstFileName );
		exit( -1 );
	}
	else
	{
		printf( "Generating diff: %s!\n", pDstFileName );
	}

	ImageFormat dstImageFormat;
	// get rid of this until we get the formats matching
//	if( sizeInBytes == 3 )
//	{
//		dstImageFormat = IMAGE_FORMAT_RGB888;
//	}
//	else
	{
		dstImageFormat = IMAGE_FORMAT_RGBA8888;
	}
	
	if( !TGAWriter::Write( pDiff, pDstFileName, width1, height1, dstImageFormat, dstImageFormat ) )
	{
		printf( "error writing %s\n", pDstFileName );
		exit( -1 );
	}
	
	return 0;	
}
