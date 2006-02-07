//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include <stdio.h>
#include <stdlib.h>
#include <ImageLoader.h>
#include "vstdlib/strtools.h"
#include "mathlib.h"
#include <TGAWriter.h>
#include <TGALoader.h>
#include <math.h>
#include <conio.h>
#include "cmdlib.h"


static bool g_NoPause = false;
static bool g_Quiet = false;

static void Pause( void )
{
	if( !g_NoPause )
	{
		printf( "Hit a key to continue\n" );
		getch();
	}
}

static bool ImageRGBA8888HasAlpha( unsigned char *pImage, int numTexels )
{
	int i;
	for( i = 0; i < numTexels; i++ )
	{
		if( pImage[i*4+3] != 255 )
		{
			return true;
		}
	}
	return false;
}

static bool GetKeyValueFromFP( FILE *fp, char **key, char **val )
{
	static char buf[2048];
	while( !feof( fp ) )
	{
		fgets( buf, 2047, fp );
		char *scan = buf;
		// search for the first quote for the key.
		while( 1 )
		{
			if( *scan == '\"' )
			{
				*key = ++scan;
				break;
			}
			if( *scan == '#' )
			{
				goto next_line; // comment
			}
			if( *scan == '\0' )
			{
				goto next_line; // end of line.
			}
			scan++;
		}
		// read the key until another quote.
		while( 1 )
		{
			if( *scan == '\"' )
			{
				*scan = '\0';
				scan++;
				break;
			}
			if( *scan == '\0' )
			{
				goto next_line;
			}
			scan++;
		}
		// search for the first quote for the value.
		while( 1 )
		{
			if( *scan == '\"' )
			{
				*val = ++scan;
				break;
			}
			if( *scan == '#' )
			{
				goto next_line; // comment
			}
			if( *scan == '\0' )
			{
				goto next_line; // end of line.
			}
			scan++;
		}
		// read the value until another quote.
		while( 1 )
		{
			if( *scan == '\"' )
			{
				*scan = '\0';
				scan++;
				// got a key and a value, so get the hell out of here.
				return true;
			}
			if( *scan == '\0' )
			{
				goto next_line;
			}
			scan++;
		}
next_line:
		;
	}
	return false;
}

static void LoadConfigFile( const char *pFileName, float *bumpScale, int *startFrame, int *endFrame )
{
	FILE *fp;
	fp = fopen( pFileName, "r" );
	if( !fp )
	{
		fprintf( stderr, "Can't open: %s\n", pFileName );
		Pause();
		exit( -1 );
	}

	char *key = NULL;
	char *val = NULL;
	while( GetKeyValueFromFP( fp, &key, &val ) )
	{
		if( stricmp( key, "bumpscale" ) == 0 )
		{
			*bumpScale = atof( val );
		}
		if( stricmp( key, "startframe" ) == 0 )
		{
			*startFrame = atoi( val );
		}
		else if( stricmp( key, "endframe" ) == 0 )
		{
			*endFrame = atoi( val );
		}
	}
	fclose( fp );
}

static void Usage()
{
	fprintf( stderr, "Usage: height2normal [-nopause] [-quiet] tex1_normal.txt tex2_normal.txt . . .\n" );
	fprintf( stderr, "-quiet   : don't print anything out, don't pause for input\n" );
	fprintf( stderr, "-nopause : don't pause for input\n" );
	Pause();
	exit( -1 );
}

void ProcessFiles( const char *pNormalFileNameWithoutExtension,
				   const char *pBaseName,
				   int startFrame, int endFrame,
				   float bumpScale )
{
	static char heightTGAFileName[1024];
	static char normalTGAFileName[1024];
	static char buf[1024];
	bool animated = !( startFrame == -1 || endFrame == -1 );
	int numFrames = endFrame - startFrame + 1;
	int frameID;

	for( frameID = 0; frameID < numFrames; frameID++ )
	{
		if( animated )
		{
			sprintf( normalTGAFileName, "%s%03d.tga", pNormalFileNameWithoutExtension, frameID + startFrame );
		}
		else
		{
			sprintf( normalTGAFileName, "%s.tga", pNormalFileNameWithoutExtension );
		}
		if( !Q_stristr( pNormalFileNameWithoutExtension, "_normal" ) )
		{
			fprintf( stderr, "ERROR: config file name must end in _normal.txt\n" );
			return;
		}

		strcpy( buf, pNormalFileNameWithoutExtension );
		char *tmp = ( char * )Q_stristr( buf, "_normal" );
		Assert( tmp );
		tmp[0] = 0;
		if( animated )
		{
			sprintf( heightTGAFileName, "%s_height%03d.tga", buf, frameID + startFrame );
		}
		else
		{
			sprintf( heightTGAFileName, "%s_height.tga", buf );
		}
		
		enum ImageFormat imageFormat;
		int width, height;
		float sourceGamma;
		if( !TGALoader::GetInfo( heightTGAFileName, &width, &height, &imageFormat, &sourceGamma ) )
		{
			fprintf( stderr, "%s not found\n", heightTGAFileName );
			return;
		}

		int memRequired = ImageLoader::GetMemRequired( width, height, IMAGE_FORMAT_IA88, false );
		unsigned char *pImageIA88 = new unsigned char[memRequired];
		
		TGALoader::Load( pImageIA88, heightTGAFileName, width, height, IMAGE_FORMAT_IA88, sourceGamma, false );

		memRequired = ImageLoader::GetMemRequired( width, height, IMAGE_FORMAT_RGBA8888, false );
		unsigned char *pImageRGBA8888 = new unsigned char[memRequired];
		ImageLoader::ConvertIA88ImageToNormalMapRGBA8888( pImageIA88, width, height, pImageRGBA8888, bumpScale );

		ImageLoader::NormalizeNormalMapRGBA8888( pImageRGBA8888, width * height );
		if( ImageRGBA8888HasAlpha( pImageRGBA8888, width * height ) )
		{
			TGAWriter::Write( pImageRGBA8888, normalTGAFileName, width, height, IMAGE_FORMAT_RGBA8888, IMAGE_FORMAT_RGBA8888 );
		}
		else
		{
			memRequired = ImageLoader::GetMemRequired( width, height, IMAGE_FORMAT_RGB888, false );
			unsigned char *pImageRGB888 = new unsigned char[memRequired];
			ImageLoader::ConvertImageFormat( pImageRGBA8888, IMAGE_FORMAT_RGBA8888, 
				pImageRGB888, IMAGE_FORMAT_RGB888, width, height, 0, 0 );
			TGAWriter::Write( pImageRGB888, normalTGAFileName, width, height, IMAGE_FORMAT_RGB888, IMAGE_FORMAT_RGB888 );
			delete [] pImageRGB888;
		}
		delete [] pImageIA88;
		delete [] pImageRGBA8888;
	}
}

int main( int argc, char **argv )
{
	if( argc < 2 )
	{
		Usage();
	}
	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );

	int i = 1;
	i = 1;
	while( i < argc )
	{
		if( stricmp( argv[i], "-quiet" ) == 0 )
		{
			i++;
			g_Quiet = true;
			g_NoPause = true; // no point in pausing if we aren't going to print anything out.
		}
		if( stricmp( argv[i], "-nopause" ) == 0 )
		{
			i++;
			g_NoPause = true;
		}
		else
		{
			break;
		}
	}
	for( ; i < argc; i++ )
	{
		static char normalFileNameWithoutExtension[1024];
		char *pFileName = argv[i];
		if( !g_Quiet )
		{
			printf( "file: %s\n", pFileName );
		}
		float bumpScale = -1.0f;
		int startFrame = -1;
		int endFrame = -1;
		LoadConfigFile( pFileName, &bumpScale, &startFrame, &endFrame );
		if( bumpScale == -1.0f )
		{
			fprintf( stderr, "Must specify \"bumpscale\" in config file\n" );
			Pause();
			continue;
		}
		if( ( startFrame == -1 && endFrame != -1 ) ||
			( startFrame != -1 && endFrame == -1 ) )
		{
			fprintf( stderr, "ERROR: If you use startframe, you must use endframe, and vice versa.\n" );
			Pause();
			continue;
		}
		if( !g_Quiet )
		{
			printf( "\tbumpscale: %f\n", bumpScale );
		}
		
		Q_StripExtension( pFileName, normalFileNameWithoutExtension, sizeof( normalFileNameWithoutExtension ) );

		const char *pBaseName = normalFileNameWithoutExtension;
		while( (pBaseName >= normalFileNameWithoutExtension) && *pBaseName != '\\' && *pBaseName != '/' )
		{
			pBaseName--;
		}
		pBaseName++;
		
		ProcessFiles( normalFileNameWithoutExtension,
					  pBaseName,
					  startFrame, endFrame,
					  bumpScale );
	}
	return 0;
}
