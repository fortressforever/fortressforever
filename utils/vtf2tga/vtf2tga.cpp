//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "mathlib.h"
#include "tgawriter.h"
#include "vstdlib/strtools.h"
#include "vtf/vtf.h"
#include "UtlBuffer.h"
#include "tier0/dbg.h"
#include "vstdlib/ICommandLine.h"

SpewRetval_t VTF2TGAOutputFunc( SpewType_t spewType, char const *pMsg )
{
	printf( pMsg );
	fflush( stdout );

	if (spewType == SPEW_ERROR)
		return SPEW_ABORT;
	return (spewType == SPEW_ASSERT) ? SPEW_DEBUGGER : SPEW_CONTINUE; 
}

static void Usage( void )
{
	Error( "Usage: vtf2tga -i <input vtf> [-o <output tga>] [-mip]\n" );
	exit( -1 );
}

int main( int argc, char **argv )
{
	SpewOutputFunc( VTF2TGAOutputFunc );
	CommandLine()->CreateCmdLine( argc, argv );
	MathLib_Init( 2.2f, 2.2f, 0.0f, 1.0f, false, false, false, false );

	const char *pVTFFileName = CommandLine()->ParmValue("-i" );
	const char *pTGAFileName = CommandLine()->ParmValue("-o" );
	if ( !pVTFFileName )
	{
		Usage();
	}

	if ( !pTGAFileName )
	{
		pTGAFileName = pVTFFileName;
	}

	bool bGenerateMipLevels = CommandLine()->CheckParm("-mip") != NULL;

	char pActualVTFFileName[MAX_PATH];
	Q_strncpy( pActualVTFFileName, pVTFFileName, MAX_PATH );
	if ( !Q_strstr( pActualVTFFileName, ".vtf" ) )
	{
		Q_strcat( pActualVTFFileName, ".vtf" ); 
	}

	FILE *vtfFp = fopen( pActualVTFFileName, "rb" );
	if( !vtfFp )
	{
		Error( "Can't open %s\n", pActualVTFFileName );
		exit( -1 );
	}

	fseek( vtfFp, 0, SEEK_END );
	int srcVTFLength = ftell( vtfFp );
	fseek( vtfFp, 0, SEEK_SET );

	CUtlBuffer buf;
	buf.EnsureCapacity( srcVTFLength );
	fread( buf.Base(), 1, srcVTFLength, vtfFp );
	fclose( vtfFp );

	IVTFTexture *pTex = CreateVTFTexture();
	if (!pTex->Unserialize( buf ))
	{
		Error( "*** Error reading in .VTF file %s\n", pActualVTFFileName );
		exit(-1);
	}
	
	Msg( "vtf width: %d\n", pTex->Width() );
	Msg( "vtf height: %d\n", pTex->Height() );
	Msg( "vtf numFrames: %d\n", pTex->FrameCount() );

	Msg( "TEXTUREFLAGS_POINTSAMPLE=%s\n", ( pTex->Flags() & TEXTUREFLAGS_POINTSAMPLE ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_TRILINEAR=%s\n", ( pTex->Flags() & TEXTUREFLAGS_TRILINEAR ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_CLAMPS=%s\n", ( pTex->Flags() & TEXTUREFLAGS_CLAMPS ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_CLAMPT=%s\n", ( pTex->Flags() & TEXTUREFLAGS_CLAMPT ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_ANISOTROPIC=%s\n", ( pTex->Flags() & TEXTUREFLAGS_ANISOTROPIC ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_HINT_DXT5=%s\n", ( pTex->Flags() & TEXTUREFLAGS_HINT_DXT5 ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_NOCOMPRESS=%s\n", ( pTex->Flags() & TEXTUREFLAGS_NOCOMPRESS ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_NORMAL=%s\n", ( pTex->Flags() & TEXTUREFLAGS_NORMAL ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_NOMIP=%s\n", ( pTex->Flags() & TEXTUREFLAGS_NOMIP ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_NOLOD=%s\n", ( pTex->Flags() & TEXTUREFLAGS_NOLOD ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_MINMIP=%s\n", ( pTex->Flags() & TEXTUREFLAGS_MINMIP ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_PROCEDURAL=%s\n", ( pTex->Flags() & TEXTUREFLAGS_PROCEDURAL ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_ONEBITALPHA=%s\n", ( pTex->Flags() & TEXTUREFLAGS_ONEBITALPHA ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_EIGHTBITALPHA=%s\n", ( pTex->Flags() & TEXTUREFLAGS_EIGHTBITALPHA ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_ENVMAP=%s\n", ( pTex->Flags() & TEXTUREFLAGS_ENVMAP ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_RENDERTARGET=%s\n", ( pTex->Flags() & TEXTUREFLAGS_RENDERTARGET ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_DEPTHRENDERTARGET=%s\n", ( pTex->Flags() & TEXTUREFLAGS_DEPTHRENDERTARGET ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_NODEBUGOVERRIDE=%s\n", ( pTex->Flags() & TEXTUREFLAGS_NODEBUGOVERRIDE ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_SINGLECOPY=%s\n", ( pTex->Flags() & TEXTUREFLAGS_SINGLECOPY ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_ONEOVERMIPLEVELINALPHA=%s\n", ( pTex->Flags() & TEXTUREFLAGS_ONEOVERMIPLEVELINALPHA ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_PREMULTCOLORBYONEOVERMIPLEVEL=%s\n", ( pTex->Flags() & TEXTUREFLAGS_PREMULTCOLORBYONEOVERMIPLEVEL ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_NORMALTODUDV=%s\n", ( pTex->Flags() & TEXTUREFLAGS_NORMALTODUDV ) ? "true" : "false" );
	Msg( "TEXTUREFLAGS_ALPHATESTMIPGENERATION=%s\n", ( pTex->Flags() & TEXTUREFLAGS_ALPHATESTMIPGENERATION ) ? "true" : "false" );
	
	Vector vecReflectivity = pTex->Reflectivity();
	Msg( "vtf reflectivity: %f %f %f\n", vecReflectivity[0], vecReflectivity[1], vecReflectivity[2] );
	Msg( "transparency: " );
	if( pTex->Flags() & TEXTUREFLAGS_EIGHTBITALPHA )
	{
		Msg( "eightbitalpha\n" );
	}
	else if( pTex->Flags() & TEXTUREFLAGS_ONEBITALPHA )
	{
		Msg( "onebitalpha\n" );
	}
	else
	{
		Msg( "noalpha\n" );
	}
	ImageFormat srcFormat = pTex->Format();
	Msg( "vtf format: %s\n", ImageLoader::GetName( srcFormat ) );
		
	int frameNum = 0;

	int iTGANameLen = Q_strlen( pTGAFileName );

	int iFaceCount = pTex->FaceCount();
	bool bIsCubeMap = pTex->IsCubeMap();

	int iLastMipLevel = bGenerateMipLevels ? pTex->MipCount() - 1 : 0;
	for ( int iMipLevel = 0; iMipLevel <= iLastMipLevel; ++iMipLevel )
	{
		for (int iCubeFace = 0; iCubeFace < iFaceCount; ++iCubeFace)
		{
			// Construct output filename
			char *pTempNameBuf = (char *)stackalloc( iTGANameLen + 12 );
			Q_strncpy( pTempNameBuf, pTGAFileName, iTGANameLen + 1 );
			char *pExt = Q_strrchr( pTempNameBuf, '.' );
			if (pExt)
			{
				pExt = 0;
			}

			if (bIsCubeMap)
			{
				static const char *pCubeFaceName[7] = { "rt", "lf", "bk", "ft", "up", "dn", "sph" };
				Q_strcat( pTempNameBuf, pCubeFaceName[iCubeFace] ); 
			}

			if (iLastMipLevel != 0)
			{
				char pTemp[3];
				Q_snprintf( pTemp, 3, "_%d", iMipLevel );
				Q_strcat( pTempNameBuf, pTemp ); 
			}

			Q_strcat( pTempNameBuf, ".tga" ); 

			unsigned char *pSrcImage = pTex->ImageData( 0, iCubeFace, iMipLevel );

			int iWidth, iHeight;
			pTex->ComputeMipLevelDimensions( iMipLevel, &iWidth, &iHeight );

			ImageFormat dstFormat;

			if( ImageLoader::IsTransparent( srcFormat ) )
			{
				dstFormat = IMAGE_FORMAT_BGRA8888;
			}
			else
			{
				dstFormat = IMAGE_FORMAT_BGR888;
			}
		//	dstFormat = IMAGE_FORMAT_RGBA8888;
		//	dstFormat = IMAGE_FORMAT_RGB888;
		//	dstFormat = IMAGE_FORMAT_BGRA8888;
		//	dstFormat = IMAGE_FORMAT_BGR888;
		//	dstFormat = IMAGE_FORMAT_BGRA5551;
		//	dstFormat = IMAGE_FORMAT_BGR565;
		//	dstFormat = IMAGE_FORMAT_BGRA4444;
		//	printf( "dstFormat: %s\n", ImageLoader::GetName( dstFormat ) );
			unsigned char *pDstImage = new unsigned char[ImageLoader::GetMemRequired( iWidth, iHeight, dstFormat, false )];
			if( !ImageLoader::ConvertImageFormat( pSrcImage, srcFormat, 
				pDstImage, dstFormat, iWidth, iHeight, 0, 0 ) )
			{
				Error( "Error converting from %s to %s\n",
					ImageLoader::GetName( srcFormat ), ImageLoader::GetName( dstFormat ) );
				exit( -1 );
			}

			if( ImageLoader::IsTransparent( dstFormat ) && ( dstFormat != IMAGE_FORMAT_RGBA8888 ) )
			{
				unsigned char *tmpImage = pDstImage;
				pDstImage = new unsigned char[ImageLoader::GetMemRequired( iWidth, iHeight, IMAGE_FORMAT_RGBA8888, false )];
				if( !ImageLoader::ConvertImageFormat( tmpImage, dstFormat, pDstImage, IMAGE_FORMAT_RGBA8888,
					iWidth, iHeight, 0, 0 ) )
				{
					Error( "Error converting from %s to %s\n",
						ImageLoader::GetName( dstFormat ), ImageLoader::GetName( IMAGE_FORMAT_RGBA8888 ) );
				}
				dstFormat = IMAGE_FORMAT_RGBA8888;
			}
			else if( !ImageLoader::IsTransparent( dstFormat ) && ( dstFormat != IMAGE_FORMAT_RGB888 ) )
			{
				unsigned char *tmpImage = pDstImage;
				pDstImage = new unsigned char[ImageLoader::GetMemRequired( iWidth, iHeight, IMAGE_FORMAT_RGB888, false )];
				if( !ImageLoader::ConvertImageFormat( tmpImage, dstFormat, pDstImage, IMAGE_FORMAT_RGB888,
					iWidth, iHeight, 0, 0 ) )
				{
					Error( "Error converting from %s to %s\n",
						ImageLoader::GetName( dstFormat ), ImageLoader::GetName( IMAGE_FORMAT_RGB888 ) );
				}
				dstFormat = IMAGE_FORMAT_RGB888;
			}
			
			TGAWriter::Write( pDstImage, pTempNameBuf, iWidth, iHeight,
				dstFormat, dstFormat );
		}
	}

	// leak leak leak leak leak, leak leak, leak leak (Blue Danube)
	return 0;
}