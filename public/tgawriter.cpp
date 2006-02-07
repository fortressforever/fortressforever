//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


// NOTE: define TGAWRITER_USE_FOPEN in your project if you don't want to use IFileSystem!

#include <stdlib.h>
#include <stdio.h>
#include "tier0/dbg.h"
#include <malloc.h>
#ifndef TGAWRITER_USE_FOPEN
#include "filesystem.h"
#endif
#include "tgawriter.h"
#include "utlbuffer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace TGAWriter
{

#ifndef TGAWRITER_USE_FOPEN
static IBaseFileSystem *s_pFileSystem = NULL;
#endif

typedef struct
{
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TGAHeader_t;

#ifndef TGAWRITER_USE_FOPEN
#define fputc myfputc
#define fwrite myfwrite
#endif

static void fputLittleShort( unsigned short s, CUtlBuffer &buffer )
{
	buffer.PutChar( s & 0xff );
	buffer.PutChar( s >> 8 );
}

#ifndef TGAWRITER_USE_FOPEN
static inline void myfputc( unsigned char c, FileHandle_t fileHandle )
{
	s_pFileSystem->Write( &c, 1, fileHandle );
}

static inline void myfwrite( void const *data, int size1, int size2, FileHandle_t fileHandle )
{
	s_pFileSystem->Write( data, size1 * size2, fileHandle );
}
#endif // !TGAWRITER_USE_FOPEN

// FIXME: assumes that we don't need to do gamma correction.
bool WriteToBuffer( unsigned char *pImageData, CUtlBuffer &buffer, int width, int height, 
            enum ImageFormat srcFormat, enum ImageFormat dstFormat )
{
	TGAHeader_t header;
	int x, y;

	// FIXME: need to handle conversion here.
	if( srcFormat != dstFormat )
	{
		return false;
	}

	header.id_length = 0; // comment length
	header.colormap_type = 0; // ???

	// Brian?: FIXME: should use BGR, etc, so that we can just fwrite the data.
	// Gary: TGA doesn't support BGR.
	switch( dstFormat )
	{
	case IMAGE_FORMAT_RGB888:
		header.image_type = 2; // 24/32 bit uncompressed TGA
		header.pixel_size = 24;
		break;
	case IMAGE_FORMAT_RGBA8888:
		header.image_type = 2; // 24/32 bit uncompressed TGA
		header.pixel_size = 32;
		break;
	case IMAGE_FORMAT_I8:
		header.image_type = 1; // 8 bit uncompressed TGA
		header.pixel_size = 8;
		break;
	default:
		return false;
		break;
	}

	header.colormap_index = 0;
	header.colormap_length = 0;
	header.colormap_size = 0;
	// can I flip this so that I can write the image out directly?
	header.x_origin = 0;
	header.y_origin = 0;
	header.width = ( unsigned short )width;
	header.height = ( unsigned short )height;
	header.attributes = 0;

	buffer.PutChar( header.id_length );
	buffer.PutChar( header.colormap_type );
	buffer.PutChar( header.image_type );
	fputLittleShort( header.colormap_index, buffer );
	fputLittleShort( header.colormap_length, buffer );
	buffer.PutChar( header.colormap_size );
	fputLittleShort( header.x_origin, buffer );
	fputLittleShort( header.y_origin, buffer );
	fputLittleShort( header.width, buffer );
	fputLittleShort( header.height, buffer );
	buffer.PutChar( header.pixel_size );
	buffer.PutChar( header.attributes );
	
	unsigned char *pbuf = (unsigned char *)_alloca( width * 4 );
	switch( dstFormat )
	{
	case IMAGE_FORMAT_RGB888:
		for( y = height - 1; y >= 0; y-- )
		{
			int start = y * width * 3;
			int out = 0;
			for( x = 0; x < width; x++ )
			{
				pbuf[out++] = pImageData[start + 2];
				pbuf[out++] = pImageData[start + 1];
				pbuf[out++] = pImageData[start + 0];
				start += 3;
			}
			buffer.Put( pbuf, width * 3 ); // write a line
		}
		break;
	case IMAGE_FORMAT_RGBA8888:
		for( y = height - 1; y >= 0; y-- )
		{
			int start = y * width * 4;
			int out = 0;
			for( x = 0; x < width; x++ )
			{
				pbuf[out++] = pImageData[start + 2];
				pbuf[out++] = pImageData[start + 1];
				pbuf[out++] = pImageData[start + 0];
				pbuf[out++] = pImageData[start + 3];
				start += 4;
			}
			buffer.Put( pbuf, width * 4 ); // write a line
		}
		break;
	// FIXME: some programs don't understand our I8 files. . probably need to put
	// a linear ramp palette in the file.
	case IMAGE_FORMAT_I8:
		for( y = height - 1; y >= 0; y-- )
		{
			buffer.Put( &pImageData[ y * width ], width );
		}
		break;
	default:
		return false;
		break;
	}
	return true;
}



bool Write( unsigned char *pImageData, const char *fileName, int width, int height, 
            enum ImageFormat srcFormat, enum ImageFormat dstFormat )
{
	bool bSuccess = false;

#ifdef TGAWRITER_USE_FOPEN
	FILE *fp;
	fp = fopen( fileName, "wb" );
	if( !fp )
	{
		return false;
	}
#else
	Assert( s_pFileSystem );
	if( !s_pFileSystem )
	{
		return false;
	}
	FileHandle_t fp;
	fp = s_pFileSystem->Open( fileName, "wb" );
#endif

	CUtlBuffer buffer( 0, sizeof(TGAHeader_t) + (width * height * 4), false );
	if ( WriteToBuffer( pImageData, buffer, width, height, srcFormat, dstFormat ) )
	{
#ifdef TGAWRITER_USE_FOPEN
		fwrite( buffer.Base(), buffer.TellPut(), 1, fp );
#else
		s_pFileSystem->Write( buffer.Base(), buffer.TellPut(), fp );
#endif
		bSuccess = true;
	}

#ifdef TGAWRITER_USE_FOPEN
	fclose( fp );
#else
	s_pFileSystem->Close( fp );
#endif
	return bSuccess;
}

//-----------------------------------------------------------------------------
// Set the file system to use for handling TGA files.
//-----------------------------------------------------------------------------
#ifndef TGAWRITER_USE_FOPEN
bool SetFileSystem( CreateInterfaceFn fileSystemFactory )
{
	s_pFileSystem = ( IFileSystem * )fileSystemFactory( BASEFILESYSTEM_INTERFACE_VERSION, NULL );
	if( s_pFileSystem )
	{
		return true;
	}
	else
	{
		return false;
	}
}
#endif

} // end namespace TGAWriter
