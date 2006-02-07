//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//=============================================================================//

#ifdef _WIN32
#include <windows.h>
#endif
#include "imageloader.h"
#include "basetypes.h"
#include "tier0/dbg.h"
#include <malloc.h>
#include <memory.h>
#include "nvtc.h"
#include "mathlib.h"
#include "vector.h"
#include "utlmemory.h"
#include "vstdlib/strtools.h"

// Should be last include
#include "tier0/memdbgon.h"


//-----------------------------------------------------------------------------
// Various important function types for each color format
//-----------------------------------------------------------------------------

typedef void (*UserFormatToRGBA8888Func_t )( unsigned char *src, unsigned char *dst, int numPixels );
typedef void (*RGBA8888ToUserFormatFunc_t )( unsigned char *src, unsigned char *dst, int numPixels );
typedef void (*GenMipMapLevelFunc_t )( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );

static ImageFormatInfo_t g_ImageFormatInfo[] =
{
	{ "RGBA8888",	4, 8, 8, 8, 8, false }, // IMAGE_FORMAT_RGBA8888,
	{ "ABGR8888",	4, 8, 8, 8, 8, false }, // IMAGE_FORMAT_ABGR8888, 
	{ "RGB888",	3, 8, 8, 8, 0, false }, // IMAGE_FORMAT_RGB888,
	{ "BGR888",	3, 8, 8, 8, 0, false }, // IMAGE_FORMAT_BGR888,
	{ "RGB565",	2, 5, 6, 5, 0, false }, // IMAGE_FORMAT_RGB565, 
	{ "I8",		1, 0, 0, 0, 0, false }, // IMAGE_FORMAT_I8,
	{ "IA88",		2, 0, 0, 0, 8, false }, // IMAGE_FORMAT_IA88
	{ "P8",		1, 0, 0, 0, 0, false }, // IMAGE_FORMAT_P8
	{ "A8",		1, 0, 0, 0, 8, false }, // IMAGE_FORMAT_A8
	{ "RGB888_BLUESCREEN", 3, 8, 8, 8, 0, false },	// IMAGE_FORMAT_RGB888_BLUESCREEN
	{ "BGR888_BLUESCREEN", 3, 8, 8, 8, 0, false },	// IMAGE_FORMAT_BGR888_BLUESCREEN
	{ "ARGB8888",	4, 8, 8, 8, 8, false }, // IMAGE_FORMAT_ARGB8888
	{ "BGRA8888",	4, 8, 8, 8, 8, false }, // IMAGE_FORMAT_BGRA8888
	{ "DXT1",		0, 0, 0, 0, 0, true }, // IMAGE_FORMAT_DXT1
	{ "DXT3",		0, 0, 0, 0, 8, true }, // IMAGE_FORMAT_DXT3
	{ "DXT5",		0, 0, 0, 0, 8, true }, // IMAGE_FORMAT_DXT5
	{ "BGRX8888",	4, 8, 8, 8, 0, false }, // IMAGE_FORMAT_BGRX8888
	{ "BGR565",	2, 5, 6, 5, 0, false }, // IMAGE_FORMAT_BGR565
	{ "BGRX5551",	2, 5, 5, 5, 0, false }, // IMAGE_FORMAT_BGRX5551
	{ "BGRA4444",	2, 4, 4, 4, 4, false },	 // IMAGE_FORMAT_BGRA4444
	{ "DXT1_ONEBITALPHA",		0, 0, 0, 0, 0, true }, // IMAGE_FORMAT_DXT1_ONEBITALPHA
	{ "BGRA5551",	2, 5, 5, 5, 1, false }, // IMAGE_FORMAT_BGRA5551
	{ "UV88",	    2, 8, 8, 0, 0, false }, // IMAGE_FORMAT_UV88
	{ "UVWQ8888",	    4, 8, 8, 8, 8, false }, // IMAGE_FORMAT_UVWQ8899
	{ "RGBA16161616F",	    8, 16, 16, 16, 16, false }, // IMAGE_FORMAT_RGBA16161616F
	{ "RGBA16161616",	    8, 16, 16, 16, 16, false }, // IMAGE_FORMAT_RGBA16161616
	{ "IMAGE_FORMAT_UVLX8888",	    4, 8, 8, 8, 8, false }, // IMAGE_FORMAT_UVLX8899
};


namespace ImageLoader
{

// Color Conversion functions
static void RGBA8888ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToABGR8888( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToRGB888( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToBGR888( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToRGB565( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToI8( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToIA88( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToP8( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToA8( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToRGB888_BLUESCREEN( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToBGR888_BLUESCREEN( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToARGB8888( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToBGRA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToBGRX8888( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToBGR565( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToBGRX5551( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToBGRA5551( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToBGRA4444( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToUV88( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToUVWQ8888( unsigned char *src, unsigned char *dst, int numPixels );
static void RGBA8888ToUVLX8888( unsigned char *src, unsigned char *dst, int numPixels );
//static void RGBA8888ToRGBA16161616F( unsigned char *src, unsigned char *dst, int numPixels );

static void ABGR8888ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void RGB888ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void BGR888ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void RGB565ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void I8ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void IA88ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void P8ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void A8ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void RGB888_BLUESCREENToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void BGR888_BLUESCREENToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void ARGB8888ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void BGRA8888ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void BGRX8888ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void BGR565ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void BGRX5551ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void BGRA5551ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void BGRA4444ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void UV88ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void UVWQ8888ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
static void UVLX8888ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );
//static void RGBA16161616FToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels );

// Mipmap Generation functions
static void GenMipMapLevelRGBA8888( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );
static void GenMipMapLevelABGR8888( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );
static void GenMipMapLevelRGB888( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );
static void GenMipMapLevelBGR888( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );
static void GenMipMapLevelRGB565( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );
static void GenMipMapLevelI8( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );
static void GenMipMapLevelIA88( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );
static void GenMipMapLevelA8( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );
static void GenMipMapLevelARGB8888( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );
static void GenMipMapLevelBGRA8888( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );
static void GenMipMapLevelBGRX8888( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );
static void GenMipMapLevelBGR565( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );
static void GenMipMapLevelBGRX5551( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );
static void GenMipMapLevelBGRA5551( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );
static void GenMipMapLevelBGRA4444( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );
static void GenMipMapLevelUV88( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );
static void GenMipMapLevelUVWQ8888( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );
static void GenMipMapLevelUVLX8888( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );
//static void GenMipMapLevelRGBA16161616F( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight );


static UserFormatToRGBA8888Func_t GetUserFormatToRGBA8888Func_t( ImageFormat srcImageFormat )
{
	switch( srcImageFormat )
	{
	case IMAGE_FORMAT_RGBA8888:
		return RGBA8888ToRGBA8888;
	case IMAGE_FORMAT_ABGR8888:
		return ABGR8888ToRGBA8888;
	case IMAGE_FORMAT_RGB888:
		return RGB888ToRGBA8888;
	case IMAGE_FORMAT_BGR888:
		return BGR888ToRGBA8888;
	case IMAGE_FORMAT_RGB565:
		return NULL;
//			return RGB565ToRGBA8888;
	case IMAGE_FORMAT_I8:
		return I8ToRGBA8888;
	case IMAGE_FORMAT_IA88:
		return IA88ToRGBA8888;
	case IMAGE_FORMAT_P8:
		return NULL;
//			return P8ToRGBA8888;
	case IMAGE_FORMAT_A8:
		return A8ToRGBA8888;
	case IMAGE_FORMAT_RGB888_BLUESCREEN:
		return RGB888_BLUESCREENToRGBA8888;
	case IMAGE_FORMAT_BGR888_BLUESCREEN:
		return BGR888_BLUESCREENToRGBA8888;
	case IMAGE_FORMAT_ARGB8888:
		return ARGB8888ToRGBA8888;
	case IMAGE_FORMAT_BGRA8888:
		return BGRA8888ToRGBA8888;
	case IMAGE_FORMAT_BGRX8888:
		return BGRX8888ToRGBA8888;
	case IMAGE_FORMAT_BGR565:
		return BGR565ToRGBA8888;
	case IMAGE_FORMAT_BGRX5551:
		return BGRX5551ToRGBA8888;
	case IMAGE_FORMAT_BGRA5551:
		return BGRA5551ToRGBA8888;
	case IMAGE_FORMAT_BGRA4444:
		return BGRA4444ToRGBA8888;
	case IMAGE_FORMAT_UV88:
		return UV88ToRGBA8888;
	case IMAGE_FORMAT_UVWQ8888:
		return UVWQ8888ToRGBA8888;
	case IMAGE_FORMAT_UVLX8888:
		return UVLX8888ToRGBA8888;
	case IMAGE_FORMAT_RGBA16161616F:
		return NULL;
		//		return RGBA16161616FToRGBA8888;
	default:
		return NULL;
	}
}

static RGBA8888ToUserFormatFunc_t GetRGBA8888ToUserFormatFunc_t( ImageFormat dstImageFormat )
{
	switch( dstImageFormat )
	{
	case IMAGE_FORMAT_RGBA8888:
		return RGBA8888ToRGBA8888;
	case IMAGE_FORMAT_ABGR8888:
		return RGBA8888ToABGR8888;
	case IMAGE_FORMAT_RGB888:
		return RGBA8888ToRGB888;
	case IMAGE_FORMAT_BGR888:
		return RGBA8888ToBGR888;
	case IMAGE_FORMAT_RGB565:
		return NULL;
//			return RGBA8888ToRGB565;
	case IMAGE_FORMAT_I8:
		return RGBA8888ToI8;
	case IMAGE_FORMAT_IA88:
		return RGBA8888ToIA88;
	case IMAGE_FORMAT_P8:
		return NULL;
//			return RGBA8888ToP8;
	case IMAGE_FORMAT_A8:
		return RGBA8888ToA8;
	case IMAGE_FORMAT_RGB888_BLUESCREEN:
		return RGBA8888ToRGB888_BLUESCREEN;
	case IMAGE_FORMAT_BGR888_BLUESCREEN:
		return RGBA8888ToBGR888_BLUESCREEN;
	case IMAGE_FORMAT_ARGB8888:
		return RGBA8888ToARGB8888;
	case IMAGE_FORMAT_BGRA8888:
		return RGBA8888ToBGRA8888;
	case IMAGE_FORMAT_BGRX8888:
		return RGBA8888ToBGRX8888;
	case IMAGE_FORMAT_BGR565:
		return RGBA8888ToBGR565;
	case IMAGE_FORMAT_BGRX5551:
		return RGBA8888ToBGRX5551;
	case IMAGE_FORMAT_BGRA5551:
		return RGBA8888ToBGRA5551;
	case IMAGE_FORMAT_BGRA4444:
		return RGBA8888ToBGRA4444;
	case IMAGE_FORMAT_UV88:
		return RGBA8888ToUV88;
	case IMAGE_FORMAT_UVWQ8888:
		return RGBA8888ToUVWQ8888;
	case IMAGE_FORMAT_UVLX8888:
		return RGBA8888ToUVLX8888;
	case IMAGE_FORMAT_RGBA16161616F:
		return NULL;
//		return RGBA8888ToRGBA16161616F;
	default:
		return NULL;
	}
}

GenMipMapLevelFunc_t GetGenMipMapLevelFunc( ImageFormat imageFormat )
{
	switch( imageFormat )
	{
	case IMAGE_FORMAT_RGBA8888:
		return &GenMipMapLevelRGBA8888;
	case IMAGE_FORMAT_ABGR8888:
		return &GenMipMapLevelABGR8888;
	case IMAGE_FORMAT_RGB888:
		return &GenMipMapLevelRGB888;
	case IMAGE_FORMAT_BGR888:
		return &GenMipMapLevelBGR888;
	case IMAGE_FORMAT_RGB565:
		return &GenMipMapLevelRGB565;
	case IMAGE_FORMAT_I8:
		return &GenMipMapLevelI8;
	case IMAGE_FORMAT_IA88:
		return &GenMipMapLevelIA88;
	case IMAGE_FORMAT_A8:
		return &GenMipMapLevelA8;
	case IMAGE_FORMAT_RGB888_BLUESCREEN:
		return &GenMipMapLevelRGB888;
	case IMAGE_FORMAT_BGR888_BLUESCREEN:
		return &GenMipMapLevelBGR888;
	case IMAGE_FORMAT_ARGB8888:
		return &GenMipMapLevelARGB8888;
	case IMAGE_FORMAT_BGRA8888:
		return &GenMipMapLevelBGRA8888;
	case IMAGE_FORMAT_BGRX8888:
		return &GenMipMapLevelBGRX8888;
	case IMAGE_FORMAT_BGR565:
		return &GenMipMapLevelBGR565;
	case IMAGE_FORMAT_BGRX5551:
		return &GenMipMapLevelBGRX5551;
	case IMAGE_FORMAT_BGRA5551:
		return &GenMipMapLevelBGRA5551;
	case IMAGE_FORMAT_BGRA4444:
		return &GenMipMapLevelBGRA4444;
	case IMAGE_FORMAT_UV88:
		return &GenMipMapLevelUV88;
	case IMAGE_FORMAT_UVWQ8888:
		return &GenMipMapLevelUVWQ8888;
	case IMAGE_FORMAT_UVLX8888:
		return &GenMipMapLevelUVLX8888;
	case IMAGE_FORMAT_RGBA16161616F:
		return NULL;
//		return &GenMipMapLevelRGBA16161616F;
	default:
		return NULL;
		break;
	}
}

//-----------------------------------------------------------------------------
// Returns info about each image format
//-----------------------------------------------------------------------------

ImageFormatInfo_t const& ImageFormatInfo( ImageFormat fmt )
{
	Assert( fmt < NUM_IMAGE_FORMATS );
	return g_ImageFormatInfo[fmt];
}

static DWORD GetDXTCEncodeType( ImageFormat imageFormat )
{
	switch( imageFormat )
	{
	case IMAGE_FORMAT_DXT1:
		return S3TC_ENCODE_RGB_FULL;
	case IMAGE_FORMAT_DXT1_ONEBITALPHA:
		return S3TC_ENCODE_RGB_FULL | S3TC_ENCODE_RGB_ALPHA_COMPARE;
	case IMAGE_FORMAT_DXT3:
		return S3TC_ENCODE_RGB_FULL | S3TC_ENCODE_ALPHA_EXPLICIT;
	case IMAGE_FORMAT_DXT5:
		return S3TC_ENCODE_RGB_FULL | S3TC_ENCODE_ALPHA_INTERPOLATED;
	default:
		return 0;
	}
}

int GetMemRequired( int width, int height, ImageFormat imageFormat, bool mipmap )
{
	if( !mipmap )
	{
		if( imageFormat == IMAGE_FORMAT_DXT1 ||
			imageFormat == IMAGE_FORMAT_DXT3 ||
			imageFormat == IMAGE_FORMAT_DXT5 )
		{
/*
			DDSURFACEDESC desc;
			memset( &desc, 0, sizeof(desc) );

			DWORD dwEncodeType;
			dwEncodeType = GetDXTCEncodeType( imageFormat );
			desc.dwSize = sizeof( desc );
			desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
			desc.dwWidth = width;
			desc.dwHeight = height;
			return S3TCgetEncodeSize( &desc, dwEncodeType );
*/
			Assert( ( width < 4 ) || !( width % 4 ) );
			Assert( ( height < 4 ) || !( height % 4 ) );
			if( width < 4 && width > 0 )
			{
				width = 4;
			}
			if( height < 4 && height > 0 )
			{
				height = 4;
			}
			int numBlocks = ( width * height ) >> 4;
			switch( imageFormat )
			{
			case IMAGE_FORMAT_DXT1:
				return numBlocks * 8;
				break;
			case IMAGE_FORMAT_DXT3:
			case IMAGE_FORMAT_DXT5:
				return numBlocks * 16;
				break;
			default:
				Assert( 0 );
				return 0;
				break;
			}
		}
		else
		{
			return width * height * SizeInBytes(imageFormat);
		}
	}
	else
	{
		int memSize = 0;

		while( 1 )
		{
			memSize += GetMemRequired( width, height, imageFormat, false );
			if( width == 1 && height == 1 )
			{
				break;
			}
			width >>= 1;
			height >>= 1;
			if( width < 1 )
			{
				width = 1;
			}
			if( height < 1 )
			{
				height = 1;
			}
		}

		return memSize;
	}
}

int GetMipMapLevelByteOffset( int width, int height, ImageFormat imageFormat, int skipMipLevels )
{
	int offset = 0;

	while( skipMipLevels > 0 )
	{
		offset += width * height * SizeInBytes(imageFormat);
		if( width == 1 && height == 1 )
		{
			break;
		}
		width >>= 1;
		height >>= 1;
		if( width < 1 )
		{
			width = 1;
		}
		if( height < 1 )
		{
			height = 1;
		}
		skipMipLevels--;
	}
	return offset;
}

void GetMipMapLevelDimensions( int *width, int *height, int skipMipLevels )
{
	while( skipMipLevels > 0 )
	{
		if( *width == 1 && *height == 1 )
		{
			break;
		}
		*width >>= 1;
		*height >>= 1;
		if( *width < 1 )
		{
			*width = 1;
		}
		if( *height < 1 )
		{
			*height = 1;
		}
		skipMipLevels--;
	}
}

int GetNumMipMapLevels( int width, int height )
{
	if( width < 1 || height < 1 )
	{
		return 0;
	}
	int numMipLevels = 1;
	while( 1 )
	{
		if( width == 1 && height == 1 )
		{
			break;
		}
		width >>= 1;
		height >>= 1;
		if( width < 1 )
		{
			width = 1;
		}
		if( height < 1 )
		{
			height = 1;
		}
		numMipLevels++;
	}
	return numMipLevels;
}

#pragma pack(1)
struct DXTColBlock
{
	WORD col0;
	WORD col1;

	// no bit fields - use bytes
	BYTE row[4];
};

struct DXTColorBGRA8888
{
	unsigned char b;		// change the order of names to change the 
	unsigned char g;		//  order of the output ARGB or BGRA, etc...
	unsigned char r;		//  Last one is MSB, 1st is LSB.
	unsigned char a;
	inline void FromBGRA8888( const DXTColorBGRA8888& in )
	{
		*( DWORD * )this = *( DWORD * )&in;
	}
	inline void AlphaFromBGRA8888( const DXTColorBGRA8888& in )
	{
		a = in.a;
	}
};

struct DXTColorRGBA8888
{
	unsigned char r;		// change the order of names to change the 
	unsigned char g;		//  order of the output ARGB or BGRA, etc...
	unsigned char b;		//  Last one is MSB, 1st is LSB.
	unsigned char a;
	inline void FromBGRA8888( const DXTColorBGRA8888& in )
	{
		r = in.r;
		g = in.g;
		b = in.b;
		a = in.a;
	}
	inline void AlphaFromBGRA8888( const DXTColorBGRA8888& in )
	{
		a = in.a;
	}
};

struct DXTColorRGB888
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	inline void FromBGRA8888( const DXTColorBGRA8888& in )
	{
		r = in.r;
		g = in.g;
		b = in.b;
	}
};

struct DXTColorBGR888
{
	unsigned char b;
	unsigned char g;
	unsigned char r;
	inline void FromBGRA8888( const DXTColorBGRA8888& in )
	{
		r = in.r;
		g = in.g;
		b = in.b;
	}
};

struct DXTColorBGR565
{
	unsigned short b : 5;		// order of names changes
	unsigned short g : 6;		//  byte order of output to 32 bit
	unsigned short r : 5;
	inline void FromBGRA8888( const DXTColorBGRA8888& in )
	{
		r = in.r >> 3;
		g = in.g >> 2;
		b = in.b >> 3;
	}
};

struct DXTColorBGRA5551
{
	unsigned short b : 5;		// order of names changes
	unsigned short g : 5;		//  byte order of output to 32 bit
	unsigned short r : 5;
	unsigned short a : 1;
	inline void FromBGRA8888( const DXTColorBGRA8888& in )
	{
		r = in.r >> 3;
		g = in.g >> 3;
		b = in.b >> 3;
		a = in.a >> 7;
	}
	inline void AlphaFromBGRA8888( const DXTColorBGRA8888& in )
	{
		a = in.a >> 7;
	}
};

struct DXTColorBGRA4444
{
	unsigned short b : 4;		// order of names changes
	unsigned short g : 4;		//  byte order of output to 32 bit
	unsigned short r : 4;
	unsigned short a : 4;
	inline void FromBGRA8888( const DXTColorBGRA8888& in )
	{
		r = in.r >> 4;
		g = in.g >> 4;
		b = in.b >> 4;
		a = in.a >> 4;
	}
	inline void AlphaFromBGRA8888( const DXTColorBGRA8888& in )
	{
		a = in.a >> 4;
	}
};

struct DXTAlphaBlock3BitLinear
{
	BYTE alpha0;
	BYTE alpha1;

	BYTE stuff[6];
};

#pragma pack()

static inline void GetColorBlockColorsBGRA8888( DXTColBlock *pBlock, DXTColorBGRA8888 *col_0, 
											    DXTColorBGRA8888 *col_1, DXTColorBGRA8888 *col_2, 
												DXTColorBGRA8888 *col_3, WORD & wrd  )
{
	DXTColorBGR565 *pCol;

	pCol = (DXTColorBGR565*) & (pBlock->col0 );

	col_0->a = 0xff;
	col_0->r = pCol->r;
	col_0->r <<= 3;				// shift to full precision
	col_0->g = pCol->g;
	col_0->g <<= 2;
	col_0->b = pCol->b;
	col_0->b <<= 3;

	pCol = (DXTColorBGR565*) & (pBlock->col1 );
	col_1->a = 0xff;
	col_1->r = pCol->r;
	col_1->r <<= 3;				// shift to full precision
	col_1->g = pCol->g;
	col_1->g <<= 2;
	col_1->b = pCol->b;
	col_1->b <<= 3;


	if( pBlock->col0 > pBlock->col1 )
	{
		// Four-color block: derive the other two colors.    
		// 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
		// These two bit codes correspond to the 2-bit fields 
		// stored in the 64-bit block.

		wrd = ((WORD)col_0->r * 2 + (WORD)col_1->r )/3;
											// no +1 for rounding
											// as bits have been shifted to 888
		col_2->r = (BYTE)wrd;

		wrd = ((WORD)col_0->g * 2 + (WORD)col_1->g )/3;
		col_2->g = (BYTE)wrd;

		wrd = ((WORD)col_0->b * 2 + (WORD)col_1->b )/3;
		col_2->b = (BYTE)wrd;
		col_2->a = 0xff;

		wrd = ((WORD)col_0->r + (WORD)col_1->r *2 )/3;
		col_3->r = (BYTE)wrd;

		wrd = ((WORD)col_0->g + (WORD)col_1->g *2 )/3;
		col_3->g = (BYTE)wrd;

		wrd = ((WORD)col_0->b + (WORD)col_1->b *2 )/3;
		col_3->b = (BYTE)wrd;
		col_3->a = 0xff;

	}
	else
	{
		// Three-color block: derive the other color.
		// 00 = color_0,  01 = color_1,  10 = color_2,  
		// 11 = transparent.
		// These two bit codes correspond to the 2-bit fields 
		// stored in the 64-bit block. 

		// explicit for each component, unlike some refrasts...????
		
		wrd = ((WORD)col_0->r + (WORD)col_1->r )/2;
		col_2->r = (BYTE)wrd;
		wrd = ((WORD)col_0->g + (WORD)col_1->g )/2;
		col_2->g = (BYTE)wrd;
		wrd = ((WORD)col_0->b + (WORD)col_1->b )/2;
		col_2->b = (BYTE)wrd;
		col_2->a = 0xff;

		col_3->r = 0x00;		// random color to indicate alpha
		col_3->g = 0xff;
		col_3->b = 0xff;
		col_3->a = 0x00;

	}
}			//  Get color block colors (...)

template <class CDestPixel> 
static inline void DecodeColorBlock( CDestPixel *pOutputImage, DXTColBlock *pColorBlock, int width,
					                 DXTColorBGRA8888 *col_0, DXTColorBGRA8888 *col_1, 
					                 DXTColorBGRA8888 *col_2, DXTColorBGRA8888 *col_3 )
{
	// width is width of image in pixels
	DWORD bits;
	int r,n;

	// bit masks = 00000011, 00001100, 00110000, 11000000
	const DWORD masks[] = { 3, 12, 3 << 4, 3 << 6 };
	const int   shift[] = { 0, 2, 4, 6 };

	// r steps through lines in y
	for( r=0; r < 4; r++, pOutputImage += width-4 )	// no width*4 as DWORD ptr inc will *4
	{

		// width * 4 bytes per pixel per line
		// each j dxtc row is 4 lines of pixels

		// n steps through pixels
		for( n=0; n < 4; n++ )
		{
			bits =		pColorBlock->row[r] & masks[n];
			bits >>=	shift[n];

			switch( bits )
			{
			case 0 :
				pOutputImage->FromBGRA8888( *col_0 );
				pOutputImage++;		// increment to next output pixel
				break;
			case 1 :
				pOutputImage->FromBGRA8888( *col_1 );
				pOutputImage++;
				break;
			case 2 :
				pOutputImage->FromBGRA8888( *col_2 );
				pOutputImage++;
				break;
			case 3 :
				pOutputImage->FromBGRA8888( *col_3 );
				pOutputImage++;
				break;
			default:
				Assert( 0 );
				pOutputImage++;
				break;
			}
		}
	}
}

template <class CDestPixel> 
static inline void DecodeAlpha3BitLinear( CDestPixel *pImPos, DXTAlphaBlock3BitLinear *pAlphaBlock,
									      int width )
{
	static BYTE		gBits[4][4];
	static WORD		gAlphas[8];
	static DXTColorBGRA8888	gACol[4][4];

	gAlphas[0] = pAlphaBlock->alpha0;
	gAlphas[1] = pAlphaBlock->alpha1;

	
	// 8-alpha or 6-alpha block?    

	if( gAlphas[0] > gAlphas[1] )
	{
		// 8-alpha block:  derive the other 6 alphas.    
		// 000 = alpha_0, 001 = alpha_1, others are interpolated

		gAlphas[2] = ( 6 * gAlphas[0] +     gAlphas[1]) / 7;	// bit code 010
		gAlphas[3] = ( 5 * gAlphas[0] + 2 * gAlphas[1]) / 7;	// Bit code 011    
		gAlphas[4] = ( 4 * gAlphas[0] + 3 * gAlphas[1]) / 7;	// Bit code 100    
		gAlphas[5] = ( 3 * gAlphas[0] + 4 * gAlphas[1]) / 7;	// Bit code 101
		gAlphas[6] = ( 2 * gAlphas[0] + 5 * gAlphas[1]) / 7;	// Bit code 110    
		gAlphas[7] = (     gAlphas[0] + 6 * gAlphas[1]) / 7;	// Bit code 111
	}    
	else
	{
		// 6-alpha block:  derive the other alphas.    
		// 000 = alpha_0, 001 = alpha_1, others are interpolated

		gAlphas[2] = (4 * gAlphas[0] +     gAlphas[1]) / 5;	// Bit code 010
		gAlphas[3] = (3 * gAlphas[0] + 2 * gAlphas[1]) / 5;	// Bit code 011    
		gAlphas[4] = (2 * gAlphas[0] + 3 * gAlphas[1]) / 5;	// Bit code 100    
		gAlphas[5] = (    gAlphas[0] + 4 * gAlphas[1]) / 5;	// Bit code 101
		gAlphas[6] = 0;										// Bit code 110
		gAlphas[7] = 255;									// Bit code 111
	}


	// Decode 3-bit fields into array of 16 BYTES with same value

	// first two rows of 4 pixels each:
	// pRows = (Alpha3BitRows*) & ( pAlphaBlock->stuff[0] );
	const DWORD mask = 0x00000007;		// bits = 00 00 01 11

	DWORD bits = *( (DWORD*) & ( pAlphaBlock->stuff[0] ));

	gBits[0][0] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[0][1] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[0][2] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[0][3] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[1][0] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[1][1] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[1][2] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[1][3] = (BYTE)( bits & mask );

	// now for last two rows:

	bits = *( (DWORD*) & ( pAlphaBlock->stuff[3] ));		// last 3 bytes

	gBits[2][0] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[2][1] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[2][2] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[2][3] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[3][0] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[3][1] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[3][2] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[3][3] = (BYTE)( bits & mask );

	// decode the codes into alpha values
	int row, pix;
	for( row = 0; row < 4; row++ )
	{
		for( pix=0; pix < 4; pix++ )
		{
			gACol[row][pix].a = (BYTE) gAlphas[ gBits[row][pix] ];

			Assert( gACol[row][pix].r == 0 );
			Assert( gACol[row][pix].g == 0 );
			Assert( gACol[row][pix].b == 0 );
		}
	}

	// Write out alpha values to the image bits
	for( row=0; row < 4; row++, pImPos += width-4 )
	{
		for( pix = 0; pix < 4; pix++ )
		{
			// zero the alpha bits of image pixel
			pImPos->AlphaFromBGRA8888( *(( DXTColorBGRA8888 *) &(gACol[row][pix])) );
			pImPos++;
		}
	}
}

template <class CDestPixel> 
static void ConvertFromDXT1( unsigned char *src, CDestPixel *dst, int width, int height )
{
	Assert( sizeof( DXTColorBGRA8888 ) == 4 );
	Assert( sizeof( DXTColorRGBA8888 ) == 4 );
	Assert( sizeof( DXTColorRGB888 ) == 3 );
	Assert( sizeof( DXTColorBGR888 ) == 3 );
	Assert( sizeof( DXTColorBGR565 ) == 2 );
	Assert( sizeof( DXTColorBGRA5551 ) == 2 );
	Assert( sizeof( DXTColorBGRA4444 ) == 2 );

	int realWidth = 0;
	int realHeight = 0;
	CDestPixel *realDst = NULL;
	// Deal with the case where we have a dimension smaller than 4.
	if( width < 4 || height < 4 )
	{
		realWidth = width;
		realHeight = height;
		// round up to the nearest four
		width = ( width + 3 ) & ~3;
		height = ( height + 3 ) & ~3;
		realDst = dst;
		dst = ( CDestPixel * )_alloca( width * height * sizeof( CDestPixel ) );
		Assert( dst );
	}
	Assert( !( width % 4 ) );
	Assert( !( height % 4 ) );

	int xblocks, yblocks;
	xblocks = width >> 2;
	yblocks = height >> 2;
	CDestPixel *pDstScan = dst;
	DWORD *pSrcScan = ( DWORD * )src;

	DXTColBlock *pBlock;
	DXTColorBGRA8888 col_0, col_1, col_2, col_3;
	WORD wrdDummy;

	int i, j;
	for( j = 0; j < yblocks; j++ )
	{
		// 8 bytes per block
		pBlock = ( DXTColBlock * )( ( unsigned char * )pSrcScan + j * xblocks * 8 );
		for( i=0; i < xblocks; i++, pBlock++ )
		{
			GetColorBlockColorsBGRA8888( pBlock, &col_0, &col_1, &col_2, &col_3, wrdDummy );

			// now decode the color block into the bitmap bits
			// inline func:
			pDstScan = dst + i*4 + j*4*width;
			DecodeColorBlock<CDestPixel>( pDstScan, pBlock, width, &col_0, &col_1,
								          &col_2, &col_3 );
		}
	}
	// Deal with the case where we have a dimension smaller than 4.
	if( realDst )
	{
		int x, y;
		for( y = 0; y < realHeight; y++ )
		{
			for( x = 0; x < realWidth; x++ )
			{
				realDst[x+(y*realWidth)] = dst[x+(y*width)];
			}
		}
	}
}

template <class CDestPixel> 
static void ConvertFromDXT5( unsigned char *src, CDestPixel *dst, int width, int height )
{
	int realWidth = 0;
	int realHeight = 0;
	CDestPixel *realDst = NULL;
	// Deal with the case where we have a dimension smaller than 4.
	if( width < 4 || height < 4 )
	{
		realWidth = width;
		realHeight = height;
		// round up to the nearest four
		width = ( width + 3 ) & ~3;
		height = ( height + 3 ) & ~3;
		realDst = dst;
		dst = ( CDestPixel * )_alloca( width * height * sizeof( CDestPixel ) );
		Assert( dst );
	}
	Assert( !( width % 4 ) );
	Assert( !( height % 4 ) );

	int xblocks, yblocks;
	xblocks = width >> 2;
	yblocks = height >> 2;
	
	CDestPixel *pDstScan = dst;
	DWORD *pSrcScan = ( DWORD * )src;

	DXTColBlock				*pBlock;
	DXTAlphaBlock3BitLinear *pAlphaBlock;

	DXTColorBGRA8888 col_0, col_1, col_2, col_3;
	WORD wrd;

	int i,j;
	for( j=0; j < yblocks; j++ )
	{
		// 8 bytes per block
		// 1 block for alpha, 1 block for color

		pBlock = (DXTColBlock*) ( (unsigned char *)pSrcScan + j * xblocks * 16 );

		for( i=0; i < xblocks; i++, pBlock ++ )
		{
			// inline
			// Get alpha block
			pAlphaBlock = (DXTAlphaBlock3BitLinear*) pBlock;

			// inline func:
			// Get color block & colors
			pBlock++;

			GetColorBlockColorsBGRA8888( pBlock, &col_0, &col_1, &col_2, &col_3, wrd );

			pDstScan = dst + i*4 + j*4*width;

			// Decode the color block into the bitmap bits
			// inline func:
			DecodeColorBlock<CDestPixel>( pDstScan, pBlock, width, &col_0, &col_1,
								          &col_2, &col_3 );

			// Overwrite the previous alpha bits with the alpha block
			//  info
			DecodeAlpha3BitLinear( pDstScan, pAlphaBlock, width );
		}
	}
	// Deal with the case where we have a dimension smaller than 4.
	if( realDst )
	{
		int x, y;
		for( y = 0; y < realHeight; y++ )
		{
			for( x = 0; x < realWidth; x++ )
			{
				realDst[x+(y*realWidth)] = dst[x+(y*width)];
			}
		}
	}
}

template <class CDestPixel> 
static void ConvertFromDXT5IgnoreAlpha( unsigned char *src, CDestPixel *dst, int width, int height )
{
	int realWidth = 0;
	int realHeight = 0;
	CDestPixel *realDst = NULL;
	// Deal with the case where we have a dimension smaller than 4.
	if( width < 4 || height < 4 )
	{
		realWidth = width;
		realHeight = height;
		// round up to the nearest four
		width = ( width + 3 ) & ~3;
		height = ( height + 3 ) & ~3;
		realDst = dst;
		dst = ( CDestPixel * )_alloca( width * height * sizeof( CDestPixel ) );
		Assert( dst );
	}
	Assert( !( width % 4 ) );
	Assert( !( height % 4 ) );

	int xblocks, yblocks;
	xblocks = width >> 2;
	yblocks = height >> 2;
	
	CDestPixel *pDstScan = dst;
	DWORD *pSrcScan = ( DWORD * )src;

	DXTColBlock				*pBlock;

	DXTColorBGRA8888 col_0, col_1, col_2, col_3;
	WORD wrd;

	int i,j;
	for( j=0; j < yblocks; j++ )
	{
		// 8 bytes per block
		// 1 block for alpha, 1 block for color

		pBlock = (DXTColBlock*) ( (unsigned char *)pSrcScan + j * xblocks * 16 );

		for( i=0; i < xblocks; i++, pBlock ++ )
		{
			// inline func:
			// Get color block & colors
			pBlock++;

			GetColorBlockColorsBGRA8888( pBlock, &col_0, &col_1, &col_2, &col_3, wrd );

			pDstScan = dst + i*4 + j*4*width;

			// Decode the color block into the bitmap bits
			// inline func:
			DecodeColorBlock<CDestPixel>( pDstScan, pBlock, width, &col_0, &col_1,
								          &col_2, &col_3 );
		}
	}
	// Deal with the case where we have a dimension smaller than 4.
	if( realDst )
	{
		int x, y;
		for( y = 0; y < realHeight; y++ )
		{
			for( x = 0; x < realWidth; x++ )
			{
				realDst[x+(y*realWidth)] = dst[x+(y*width)];
			}
		}
	}
}

bool ConvertImageFormat( unsigned char *src, ImageFormat srcImageFormat,
 					     unsigned char *dst, ImageFormat dstImageFormat, 
						 int width, int height, int srcStride, int dstStride )
{
	if( ( dstImageFormat == IMAGE_FORMAT_DXT1 ||
		  dstImageFormat == IMAGE_FORMAT_DXT3 ||
		  dstImageFormat == IMAGE_FORMAT_DXT5 ) && 
		  srcImageFormat == dstImageFormat )
	{
		// Fast path for compressed textures . . stride doesn't make as much sense.
//		Assert( srcStride == 0 && dstStride == 0 );

		int memRequired;
		memRequired = GetMemRequired( width, height, srcImageFormat, false );
		memcpy( dst, src, memRequired );
		return true;
	}
#ifndef IMAGE_LOADER_NO_DXTC
	else if( ( srcImageFormat == IMAGE_FORMAT_RGBA8888 ||
			   srcImageFormat == IMAGE_FORMAT_RGB888   ||
			   srcImageFormat == IMAGE_FORMAT_BGRA8888 ||
			   srcImageFormat == IMAGE_FORMAT_BGRX8888 ) &&
			 ( dstImageFormat == IMAGE_FORMAT_DXT1 ||
			   dstImageFormat == IMAGE_FORMAT_DXT3 ||
			   dstImageFormat == IMAGE_FORMAT_DXT5 ) )
	{
		// from rgb(a) to dxtN
		if( srcStride != 0 || dstStride != 0 )
		{
			return false;
		}
		DDSURFACEDESC descIn;
		DDSURFACEDESC descOut;
		memset( &descIn, 0, sizeof(descIn) );
		memset( &descOut, 0, sizeof(descOut) );
		float weight[3] = {0.3086f, 0.6094f, 0.0820f};
		DWORD dwEncodeType = GetDXTCEncodeType( dstImageFormat );
		
		// Setup descIn
		descIn.dwSize = sizeof(descIn);
		descIn.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_LPSURFACE | 
			/*DDSD_PITCH | */ DDSD_PIXELFORMAT;
		descIn.dwWidth = width;
		descIn.dwHeight = height;
		descIn.lPitch = width * SizeInBytes(srcImageFormat);
		descIn.lpSurface = src;
		descIn.ddpfPixelFormat.dwSize = sizeof( DDPIXELFORMAT );
		switch( srcImageFormat )
		{
		case IMAGE_FORMAT_RGBA8888:
			descIn.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
			descIn.ddpfPixelFormat.dwRGBBitCount = 32;
			descIn.ddpfPixelFormat.dwRBitMask = 0x0000ff;
			descIn.ddpfPixelFormat.dwGBitMask = 0x00ff00;
			descIn.ddpfPixelFormat.dwBBitMask = 0xff0000;
			// must set this anyway or S3TC will lock up!!!
			descIn.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;
			break;
		case IMAGE_FORMAT_BGRA8888:
			descIn.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
			descIn.ddpfPixelFormat.dwRGBBitCount = 32;
			descIn.ddpfPixelFormat.dwRBitMask = 0xFF0000;
			descIn.ddpfPixelFormat.dwGBitMask = 0x00ff00;
			descIn.ddpfPixelFormat.dwBBitMask = 0x0000FF;
			// must set this anyway or S3TC will lock up!!!
			descIn.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;
			break;
		case IMAGE_FORMAT_BGRX8888:
			descIn.ddpfPixelFormat.dwFlags = DDPF_RGB;
			descIn.ddpfPixelFormat.dwRGBBitCount = 32;
			descIn.ddpfPixelFormat.dwRBitMask = 0xFF0000;
			descIn.ddpfPixelFormat.dwGBitMask = 0x00ff00;
			descIn.ddpfPixelFormat.dwBBitMask = 0x0000FF;
			// must set this anyway or S3TC will lock up!!!
			descIn.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;
			break;
		case IMAGE_FORMAT_RGB888:
			descIn.ddpfPixelFormat.dwFlags = DDPF_RGB;
			descIn.ddpfPixelFormat.dwRGBBitCount = 24;
			descIn.ddpfPixelFormat.dwRBitMask = 0x0000ff;
			descIn.ddpfPixelFormat.dwGBitMask = 0x00ff00;
			descIn.ddpfPixelFormat.dwBBitMask = 0xff0000;
			descIn.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;
			break;
		default:
			return false;
		}
		
		// Setup descOut
		descOut.dwSize = sizeof( descOut );
		
		// Encode the texture
		S3TCencode( &descIn, NULL, &descOut, dst, dwEncodeType, weight );
		return true;
	}
#endif
	else if( ( dstImageFormat == IMAGE_FORMAT_RGBA8888 ||
			   dstImageFormat == IMAGE_FORMAT_BGRX8888 ||
			   dstImageFormat == IMAGE_FORMAT_BGRA8888 ||
			   dstImageFormat == IMAGE_FORMAT_BGRA4444 ||
			   dstImageFormat == IMAGE_FORMAT_BGRA5551 ||
			   dstImageFormat == IMAGE_FORMAT_BGRX5551 ||
			   dstImageFormat == IMAGE_FORMAT_BGR565 ||
			   dstImageFormat == IMAGE_FORMAT_BGR888 ||
			   dstImageFormat == IMAGE_FORMAT_RGB888 ) &&
			 ( srcImageFormat == IMAGE_FORMAT_DXT1 ||
			   srcImageFormat == IMAGE_FORMAT_DXT3 ||
			   srcImageFormat == IMAGE_FORMAT_DXT5 ) )
	{
		// from dxtN to rgb(a)

		if( srcStride != 0 || dstStride != 0 )
		{
			return false;
		}
		if( srcImageFormat == IMAGE_FORMAT_DXT1 )
		{
			if( dstImageFormat == IMAGE_FORMAT_RGBA8888 )
			{
				ConvertFromDXT1( src, ( DXTColorRGBA8888 * )dst, width, height );
				return true;
			}
			if( dstImageFormat == IMAGE_FORMAT_BGRA8888 ||
			    dstImageFormat == IMAGE_FORMAT_BGRX8888 )
			{
				ConvertFromDXT1( src, ( DXTColorBGRA8888 * )dst, width, height );
				return true;
			}
			if( dstImageFormat == IMAGE_FORMAT_RGB888 )
			{
				ConvertFromDXT1( src, ( DXTColorRGB888 * )dst, width, height );
				return true;
			}
			if( dstImageFormat == IMAGE_FORMAT_BGR888 )
			{
				ConvertFromDXT1( src, ( DXTColorBGR888 * )dst, width, height );
				return true;
			}
			if( dstImageFormat == IMAGE_FORMAT_BGR565 )
			{
				ConvertFromDXT1( src, ( DXTColorBGR565 * )dst, width, height );
				return true;
			}
			if( dstImageFormat == IMAGE_FORMAT_BGRA5551 ||
				dstImageFormat == IMAGE_FORMAT_BGRX5551 )
			{
				ConvertFromDXT1( src, ( DXTColorBGRA5551 * )dst, width, height );
				return true;
			}
			if( dstImageFormat == IMAGE_FORMAT_BGRA4444 )
			{
				ConvertFromDXT1( src, ( DXTColorBGRA4444 * )dst, width, height );
				return true;
			}
		}
		if( srcImageFormat == IMAGE_FORMAT_DXT5 )
		{
			if( dstImageFormat == IMAGE_FORMAT_RGBA8888 )
			{
				ConvertFromDXT5( src, ( DXTColorRGBA8888 * )dst, width, height );
				return true;
			}
			if( dstImageFormat == IMAGE_FORMAT_BGRA8888 ||
			    dstImageFormat == IMAGE_FORMAT_BGRX8888 )
			{
				ConvertFromDXT5( src, ( DXTColorBGRA8888 * )dst, width, height );
				return true;
			}
			if( dstImageFormat == IMAGE_FORMAT_RGB888 )
			{
				ConvertFromDXT5IgnoreAlpha( src, ( DXTColorRGB888 * )dst, width, height );
				return true;
			}
			if( dstImageFormat == IMAGE_FORMAT_BGR888 )
			{
				ConvertFromDXT5IgnoreAlpha( src, ( DXTColorBGR888 * )dst, width, height );
				return true;
			}
			if( dstImageFormat == IMAGE_FORMAT_BGR565 )
			{
				ConvertFromDXT5IgnoreAlpha( src, ( DXTColorBGR565 * )dst, width, height );
				return true;
			}
			if( dstImageFormat == IMAGE_FORMAT_BGRA5551 ||
				dstImageFormat == IMAGE_FORMAT_BGRX5551 )
			{
				ConvertFromDXT5( src, ( DXTColorBGRA5551 * )dst, width, height );
				return true;
			}
			if( dstImageFormat == IMAGE_FORMAT_BGRA4444 )
			{
				ConvertFromDXT5( src, ( DXTColorBGRA4444 * )dst, width, height );
				return true;
			}
		}
		return false;
	}
	else if( dstImageFormat == IMAGE_FORMAT_DXT1 ||
			 dstImageFormat == IMAGE_FORMAT_DXT3 ||
			 dstImageFormat == IMAGE_FORMAT_DXT5 ||
			 srcImageFormat == IMAGE_FORMAT_DXT1 ||
			 srcImageFormat == IMAGE_FORMAT_DXT3 ||
			 srcImageFormat == IMAGE_FORMAT_DXT5 )
	{
		return false;
	}
	else
	{
		// uncompressed textures
		int line;
		int srcPixelSize = SizeInBytes(srcImageFormat);
		int dstPixelSize = SizeInBytes(dstImageFormat);
		
		if( srcStride == 0 )
		{
			srcStride = srcPixelSize * width;
		}
		if( dstStride == 0 )
		{
			dstStride = dstPixelSize * width;
		}
		
		// Fast path...
		if( srcImageFormat == dstImageFormat )
		{
			for( line = 0; line < height; line++ )
			{
				memcpy( dst + line * dstStride, src + line * srcStride, width * srcPixelSize ); 
			}
			return true;
		}
		
		unsigned char lineBufRGBA8888[IMAGE_MAX_DIM*4];
		
		UserFormatToRGBA8888Func_t userFormatToRGBA8888Func;
		RGBA8888ToUserFormatFunc_t RGBA8888ToUserFormatFunc;
		
		userFormatToRGBA8888Func = GetUserFormatToRGBA8888Func_t( srcImageFormat );
		RGBA8888ToUserFormatFunc = GetRGBA8888ToUserFormatFunc_t( dstImageFormat );
		
		if( !lineBufRGBA8888 || !userFormatToRGBA8888Func || !RGBA8888ToUserFormatFunc )
		{
			return false;
		}
		
		for( line = 0; line < height; line++ )
		{
			userFormatToRGBA8888Func( src + line * srcStride, lineBufRGBA8888, width );
			RGBA8888ToUserFormatFunc( lineBufRGBA8888, dst + line * dstStride, width );
		}
		return true;
	}
}


//-----------------------------------------------------------------------------
// Gamma correction
//-----------------------------------------------------------------------------

static void ConstructFloatGammaTable( float* pTable, float srcGamma, float dstGamma )
{
	for( int i = 0; i < 256; i++ )
	{
		pTable[i] = 255.0 * pow( (float)i / 255.0f, srcGamma / dstGamma );
	}
}

void ConstructGammaTable( unsigned char* pTable, float srcGamma, float dstGamma )
{
	int v;
	for( int i = 0; i < 256; i++ )
	{
		double f;
		f = 255.0 * pow( (float)i / 255.0f, srcGamma / dstGamma );
		v = ( int )(f + 0.5f);
		if( v < 0 )
		{
			v = 0;
		}
		else if( v > 255 )
		{
			v = 255;
		}
		pTable[i] = ( unsigned char )v;
	}
}

void GammaCorrectRGBA8888( unsigned char *pSrc, unsigned char* pDst, int width, int height,
						  unsigned char* pGammaTable )
{
	for (int i = 0; i < height; ++i )
	{
		for (int j = 0; j < width; ++j )
		{
			int idx = (i * width + j) * 4;

			// don't gamma correct alpha
			pDst[idx] = pGammaTable[pSrc[idx]];
			pDst[idx+1] = pGammaTable[pSrc[idx+1]];
			pDst[idx+2] = pGammaTable[pSrc[idx+2]];
		}
	}
}

void GammaCorrectRGBA8888( unsigned char *src, unsigned char* dst, int width, int height,
						  float srcGamma, float dstGamma )
{
	if (srcGamma == dstGamma)
	{
		if (src != dst)
			memcpy( dst, src, GetMemRequired( width, height, IMAGE_FORMAT_RGBA8888, false ) );
		return;
	}

	static unsigned char gamma[256];
	static float lastSrcGamma = -1;
	static float lastDstGamma = -1;

	if (lastSrcGamma != srcGamma || lastDstGamma != dstGamma)
	{
		ConstructGammaTable( gamma, srcGamma, dstGamma );
		lastSrcGamma = srcGamma;
		lastDstGamma = dstGamma;
	}

	GammaCorrectRGBA8888( src, dst, width, height, gamma );
}



//-----------------------------------------------------------------------------
// Generate a NICE filter kernel
//-----------------------------------------------------------------------------

static void GenerateNiceFilter( float wratio, float hratio, int kernelDiameter, float* pKernel, float *pInvKernel )
{
	// Compute a kernel...
	int i, j;
	int kernelWidth = kernelDiameter * wratio;
	int kernelHeight = kernelDiameter * hratio;

	// This is a NICE filter
	// sinc pi*x * a box from -3 to 3 * sinc ( pi * x/3)
	// where x is the pixel # in the destination (shrunken) image.
	// only problem here is that the NICE filter has a very large kernel
	// (7x7 x wratio x hratio)
	float dx = 1.0f / (float)wratio;
	float dy = 1.0f / (float)hratio;
	float y = -((float)kernelDiameter - dy) * 0.5f; 
	float total = 0.0f;
	for ( i = 0; i < kernelHeight; ++i )
	{
		float x = -((float)kernelDiameter - dx) * 0.5f; 
		for ( j = 0; j < kernelWidth; ++j )
		{
			float d = sqrt( x * x + y * y );
			if (d > kernelDiameter * 0.5f)
			{
				pKernel[i * kernelWidth + j] = 0.0f;
			}
			else
			{
				float t = M_PI * d;
				if ( t != 0 )
				{
					float sinc = sin( t ) / t;
					float sinc3 = 3.0f * sin( t / 3.0f ) / t;
					pKernel[i * kernelWidth + j] = sinc * sinc3;
				}
				else
				{
					pKernel[i * kernelWidth + j] = 1.0f;
				}
				total += pKernel[i * kernelWidth + j];
			}
			x += dx;
		}
		y += dy;
	}
	
	// normalize
	for ( i = 0; i < kernelHeight; ++i )
	{
		for ( j = 0; j < kernelWidth; ++j )
		{
			int nPixel = i * kernelWidth + j;
			pKernel[nPixel] /= total;
			pInvKernel[nPixel] = wratio * hratio * pKernel[nPixel]; 
		}
	} 
}


//-----------------------------------------------------------------------------
// Resample an image
//-----------------------------------------------------------------------------
static inline unsigned char Clamp( float x )
{
	int idx = (int)(x + 0.5f);
	if (idx < 0) idx = 0;
	else if (idx > 255) idx = 255;
	return idx;
}

inline bool IsPowerOfTwo( int x )
{
	return (x & ( x - 1 )) == 0;
}


struct KernelInfo_t
{
	float *m_pKernel;
	float *m_pInvKernel;
	int m_nWidth;
	int m_nHeight;
	int m_nDiameter;
};

enum KernelType_t
{
	KERNEL_DEFAULT = 0,
	KERNEL_NORMALMAP,
	KERNEL_ALPHATEST,
};

typedef void (*ApplyKernelFunc_t)( const KernelInfo_t &kernel, const ResampleInfo_t &info, int wratio, int hratio, float* gammaToLinear, float *pAlphaResult );

//-----------------------------------------------------------------------------
// Apply Kernel to an image
//-----------------------------------------------------------------------------
template< int type, bool bNiceFilter >
class CKernelWrapper
{
public:
	static void ApplyKernel( const KernelInfo_t &kernel, const ResampleInfo_t &info, int wratio, int hratio, float* gammaToLinear, float *pAlphaResult )
	{
		float invDstGamma = 1.0f / info.m_flDestGamma;

		// Apply the kernel to the image
		int nInitialY = (hratio >> 1) - ((hratio * kernel.m_nDiameter) >> 1);
		int nInitialX = (wratio >> 1) - ((wratio * kernel.m_nDiameter) >> 1);

		float flAlphaThreshhold = (info.m_flAlphaThreshhold >= 0 ) ? 255.0f * info.m_flAlphaThreshhold : 255.0f * 0.4f;
		for ( int i = 0; i < info.m_nDestHeight; ++i )
		{
			int startY = hratio * i + nInitialY;
			int dstPixel = (i * info.m_nDestWidth) << 2;
			for ( int j = 0; j < info.m_nDestWidth; ++j, dstPixel += 4 )
			{
				int startX = wratio * j + nInitialX;
				float total[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
				for ( int k = 0, srcY = startY; k < kernel.m_nHeight; ++k, ++srcY )
				{
					int sy = srcY;
					if ( info.m_nFlags & RESAMPLE_CLAMPT )
					{
						sy = clamp( sy, 0, info.m_nSrcHeight - 1 );
					}
					else
					{
						// This works since srcHeight is a power of two.
						// Even for negative #s!
						sy &= (info.m_nSrcHeight - 1);
					}

					sy *= info.m_nSrcWidth;

					int kernelIdx;
					if ( bNiceFilter )
					{
						kernelIdx = k * kernel.m_nWidth;
					}
					else
					{
						kernelIdx = 0;
					}

					for ( int l = 0, srcX = startX; l < kernel.m_nWidth; ++l, ++srcX, ++kernelIdx )
					{
						int sx = srcX;
						if ( info.m_nFlags & RESAMPLE_CLAMPS )
						{
							sx = clamp( sx, 0, info.m_nSrcWidth - 1 );
						}
						else
						{
							// This works since srcHeight is a power of two.
							// Even for negative #s!
							sx &= (info.m_nSrcWidth - 1);
						}
						
						int srcPixel = (sy + sx) << 2;

						float flKernelFactor;
						if ( bNiceFilter )
						{
							flKernelFactor = kernel.m_pKernel[kernelIdx];
							if ( flKernelFactor == 0.0f )
								continue;
						}
						else
						{
							flKernelFactor = kernel.m_pKernel[0];
						}

						if( type == KERNEL_NORMALMAP )
						{
							total[0] += flKernelFactor * info.m_pSrc[srcPixel + 0];
							total[1] += flKernelFactor * info.m_pSrc[srcPixel + 1];
							total[2] += flKernelFactor * info.m_pSrc[srcPixel + 2];
							total[3] += flKernelFactor * info.m_pSrc[srcPixel + 3];
						}
						else if ( type == KERNEL_ALPHATEST )
						{
							total[0] += flKernelFactor * gammaToLinear[ info.m_pSrc[srcPixel + 0] ];
							total[1] += flKernelFactor * gammaToLinear[ info.m_pSrc[srcPixel + 1] ];
							total[2] += flKernelFactor * gammaToLinear[ info.m_pSrc[srcPixel + 2] ];
							if ( info.m_pSrc[srcPixel + 3] > 192 )
							{
								total[3] += flKernelFactor * 255.0f;
							}
						}
						else
						{
							total[0] += flKernelFactor * gammaToLinear[ info.m_pSrc[srcPixel + 0] ];
							total[1] += flKernelFactor * gammaToLinear[ info.m_pSrc[srcPixel + 1] ];
							total[2] += flKernelFactor * gammaToLinear[ info.m_pSrc[srcPixel + 2] ];
							total[3] += flKernelFactor * info.m_pSrc[srcPixel + 3];
						}
					}
				}

				// NOTE: Can't use a table here, we lose too many bits
				if( type == KERNEL_NORMALMAP )
				{
					info.m_pDest[ dstPixel + 0 ] = 127.0f + ( info.m_flColorScale * ( total[0] - 127.0f ) );
					info.m_pDest[ dstPixel + 1 ] = 127.0f + ( info.m_flColorScale * ( total[1] - 127.0f ) );
					info.m_pDest[ dstPixel + 2 ] = 127.0f + ( info.m_flColorScale * ( total[2] - 127.0f ) );
					info.m_pDest[ dstPixel + 3 ] = Clamp(total[3]);
				}
				else if ( type == KERNEL_ALPHATEST )
				{
					// If there's more than 40% coverage, then keep the pixel (renormalize the color based on coverage)
					float flFactor = info.m_flColorScale / 255.0f;
					float flAlpha = ( total[3] >= flAlphaThreshhold ) ? 255 : 0; 

					info.m_pDest[ dstPixel + 0 ] = (total[0] > 0) ? Clamp( 255.0f * pow( total[0] * flFactor, invDstGamma ) ) : 0;
					info.m_pDest[ dstPixel + 1 ] = (total[1] > 0) ? Clamp( 255.0f * pow( total[1] * flFactor, invDstGamma ) ) : 0;
					info.m_pDest[ dstPixel + 2 ] = (total[2] > 0) ? Clamp( 255.0f * pow( total[2] * flFactor, invDstGamma ) ) : 0;
					info.m_pDest[ dstPixel + 3 ] = Clamp( flAlpha );

					for ( int k = 0, srcY = startY; k < kernel.m_nHeight; ++k, ++srcY )
					{
						// This works since srcHeight is a power of two.
						// Even for negative #s!
						int sy = srcY & (info.m_nSrcHeight - 1);
						sy *= info.m_nSrcWidth;

						int kernelIdx;
						if ( bNiceFilter )
						{
							kernelIdx = k * kernel.m_nWidth;
						}
						else
						{
							kernelIdx = 0;
						}

						for ( int l = 0, srcX = startX; l < kernel.m_nWidth; ++l, ++srcX, ++kernelIdx )
						{
							int sx = srcX & (info.m_nSrcWidth - 1);					
							int srcPixel = sy + sx;

							float flKernelFactor;
							if ( bNiceFilter )
							{
								flKernelFactor = kernel.m_pInvKernel[kernelIdx];
							}
							else
							{
								flKernelFactor = kernel.m_pInvKernel[0];
							}

							pAlphaResult[srcPixel] += flKernelFactor * flAlpha;
						}
					}
				}
				else
				{
					float flFactor = info.m_flColorScale / 255.0f;

					info.m_pDest[ dstPixel + 0 ] = (total[0] > 0) ? Clamp( 255.0f * pow( total[0] * flFactor, invDstGamma ) ) : 0;
					info.m_pDest[ dstPixel + 1 ] = (total[1] > 0) ? Clamp( 255.0f * pow( total[1] * flFactor, invDstGamma ) ) : 0;
					info.m_pDest[ dstPixel + 2 ] = (total[2] > 0) ? Clamp( 255.0f * pow( total[2] * flFactor, invDstGamma ) ) : 0;
					info.m_pDest[ dstPixel + 3 ] = Clamp( total[3] );
				}
			}
		}

		if ( type == KERNEL_ALPHATEST )
		{
			// Find the delta between the alpha + source image
			int i;
			for ( i = 0; i < info.m_nSrcHeight; ++i )
			{
				int dstPixel = i * info.m_nSrcWidth;
				for ( int j = 0; j < info.m_nSrcWidth; ++j, ++dstPixel )
				{
					pAlphaResult[dstPixel] = fabs( pAlphaResult[dstPixel] - info.m_pSrc[dstPixel * 4 + 3] );
				}
			}

			int nInitialY = 0;
			int nInitialX = 0;
			float flAlphaThreshhold = (info.m_flAlphaHiFreqThreshhold >= 0 ) ? 255.0f * info.m_flAlphaHiFreqThreshhold : 255.0f * 0.4f;
			for ( i = 0; i < info.m_nDestHeight; ++i )
			{
				int startY = hratio * i + nInitialY;
				int dstPixel = (i * info.m_nDestWidth) << 2;
				for ( int j = 0; j < info.m_nDestWidth; ++j, dstPixel += 4 )
				{
					if ( info.m_pDest[ dstPixel + 3 ] == 255 )
						continue;

					int startX = wratio * j + nInitialX;
					float flAlphaDelta = 0.0f;
					for ( int k = 0, srcY = startY; k < hratio; ++k, ++srcY )
					{
						// This works since srcHeight is a power of two.
						// Even for negative #s!
						int sy = srcY & (info.m_nSrcHeight - 1);
						sy *= info.m_nSrcWidth;
						for ( int l = 0, srcX = startX; l < wratio; ++l, ++srcX )
						{
							int sx = srcX & (info.m_nSrcWidth - 1);					
							int srcPixel = sy + sx;
							flAlphaDelta += pAlphaResult[srcPixel];
						}
					}

					flAlphaDelta /= (hratio * wratio);
					if ( flAlphaDelta > flAlphaThreshhold )
					{
						info.m_pDest[ dstPixel + 3 ] = 255.0f;
					}
				}
			}
		}
	}
};

typedef CKernelWrapper< KERNEL_DEFAULT, false >		ApplyKernelDefault_t;
typedef CKernelWrapper< KERNEL_NORMALMAP, false >	ApplyKernelNormalmap_t;
typedef CKernelWrapper< KERNEL_ALPHATEST, false >	ApplyKernelAlphatest_t;
typedef CKernelWrapper< KERNEL_DEFAULT, true >		ApplyKernelDefaultNice_t;
typedef CKernelWrapper< KERNEL_NORMALMAP, true >	ApplyKernelNormalmapNice_t;
typedef CKernelWrapper< KERNEL_ALPHATEST, true >	ApplyKernelAlphatestNice_t;

static ApplyKernelFunc_t g_KernelFunc[] =
{
	ApplyKernelDefault_t::ApplyKernel,
	ApplyKernelNormalmap_t::ApplyKernel,
	ApplyKernelAlphatest_t::ApplyKernel,
};

static ApplyKernelFunc_t g_KernelFuncNice[] =
{
	ApplyKernelDefaultNice_t::ApplyKernel,
	ApplyKernelNormalmapNice_t::ApplyKernel,
	ApplyKernelAlphatestNice_t::ApplyKernel,
};

bool ResampleRGBA8888( const ResampleInfo_t& info )
{
	// No resampling needed, just gamma correction
	if ( info.m_nSrcWidth == info.m_nDestWidth && info.m_nSrcHeight == info.m_nDestHeight )
	{
		// Here, we need to gamma convert the source image..
		GammaCorrectRGBA8888( info.m_pSrc, info.m_pDest, info.m_nSrcWidth, info.m_nSrcHeight, info.m_flSrcGamma, info.m_flDestGamma );
		return true;
	}

	// fixme: has to be power of two for now.
	if( !IsPowerOfTwo(info.m_nSrcWidth) || !IsPowerOfTwo(info.m_nSrcHeight) ||
		!IsPowerOfTwo(info.m_nDestWidth) || !IsPowerOfTwo(info.m_nDestHeight) )
	{
		return false;
	}

	// fixme: can only downsample for now.
	if( (info.m_nSrcWidth < info.m_nDestWidth) || (info.m_nSrcHeight < info.m_nDestHeight) )
	{
		return false;
	}

	// Compute gamma tables...
	static float gammaToLinear[256];
	static float lastSrcGamma = -1;

	if (lastSrcGamma != info.m_flSrcGamma)
	{
		ConstructFloatGammaTable( gammaToLinear, info.m_flSrcGamma, 1.0f );
		lastSrcGamma =  info.m_flSrcGamma;
	}

	int wratio = info.m_nSrcWidth / info.m_nDestWidth;
	int hratio = info.m_nSrcHeight / info.m_nDestHeight;
	
	KernelInfo_t kernel;

	float* pTempMemory = 0;
	float* pTempInvMemory = 0;
	static float* kernelCache[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	static float* pInvKernelCache[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	float pKernelMem[1];
	float pInvKernelMem[1];
	if ( info.m_nFlags & RESAMPLE_NICE_FILTER )
	{
		// Kernel size is measured in dst pixels
		kernel.m_nDiameter = 6;

		// Compute a kernel...
		kernel.m_nWidth = kernel.m_nDiameter * wratio;
		kernel.m_nHeight = kernel.m_nDiameter * hratio;

		// Cache the filter....
		int power = -1;

		if (wratio == hratio)
		{
			power = 0;
			int tempWidth = wratio;
			while (tempWidth > 1)
			{
				++power;
				tempWidth >>= 1;
			}

			// Don't cache anything bigger than 512x512
			if (power >= 10)
			{
				power = -1;
			}
		}

		if (power >= 0)
		{
			if (!kernelCache[power])
			{
				kernelCache[power] = new float[kernel.m_nWidth * kernel.m_nHeight];
				pInvKernelCache[power] = new float[kernel.m_nWidth * kernel.m_nHeight];
				GenerateNiceFilter( wratio, hratio, kernel.m_nDiameter, kernelCache[power], pInvKernelCache[power] ); 
			}

			kernel.m_pKernel = kernelCache[power];
			kernel.m_pInvKernel = pInvKernelCache[power];
		}
		else
		{
			// Don't cache non-square kernels
			pTempMemory = new float[kernel.m_nWidth * kernel.m_nHeight];
			pTempInvMemory = new float[kernel.m_nWidth * kernel.m_nHeight];
			GenerateNiceFilter( wratio, hratio, kernel.m_nDiameter, pTempMemory, pTempInvMemory ); 
			kernel.m_pKernel = pTempMemory;
			kernel.m_pInvKernel = pTempInvMemory;
		}
	}
	else
	{
		// Compute a kernel...
		kernel.m_nWidth = wratio;
		kernel.m_nHeight = hratio;

		kernel.m_nDiameter = 1;

		// Simple implementation of a box filter that doesn't block the stack!
		pKernelMem[0] = 1.0f / (float)(kernel.m_nWidth * kernel.m_nHeight);
		pInvKernelMem[0] = 1.0f;
		kernel.m_pKernel = pKernelMem;
		kernel.m_pInvKernel = pInvKernelMem;
	}

	float *pAlphaResult = NULL;
	KernelType_t type;
	if ( info.m_nFlags & RESAMPLE_NORMALMAP )
	{
		type = KERNEL_NORMALMAP;
	}
	else if ( info.m_nFlags & RESAMPLE_ALPHATEST )
	{
		int nSize = info.m_nSrcHeight * info.m_nSrcWidth * sizeof(float);
		pAlphaResult = (float*)malloc( nSize );
		memset( pAlphaResult, 0, nSize );
		type = KERNEL_ALPHATEST;
	}
	else
	{
		type = KERNEL_DEFAULT;
	}

	if ( info.m_nFlags & RESAMPLE_NICE_FILTER )
	{	
		g_KernelFuncNice[type]( kernel, info, wratio, hratio, gammaToLinear, pAlphaResult );
		if (pTempMemory)
		{
			delete[] pTempMemory;
		}
	}
	else
	{
		g_KernelFunc[type]( kernel, info, wratio, hratio, gammaToLinear, pAlphaResult );
	}

	if ( pAlphaResult )
	{
		free( pAlphaResult );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Generates mipmap levels
//-----------------------------------------------------------------------------

void GenerateMipmapLevels( unsigned char* pSrc, unsigned char* pDst, int width,
	int height,	ImageFormat imageFormat, float srcGamma, float dstGamma, int numLevels )
{
	int dstWidth = width;
	int dstHeight = height;

	// temporary storage for the mipmaps
	int tempMem = GetMemRequired( dstWidth, dstHeight, IMAGE_FORMAT_RGBA8888, false );
	CUtlMemory<unsigned char> tmpImage;
	tmpImage.EnsureCapacity( tempMem );

	while( true )
	{
		// This generates a mipmap in RGBA8888, linear space
		ResampleInfo_t info;
		info.m_pSrc = pSrc;
		info.m_pDest = tmpImage.Base();
		info.m_nSrcWidth = width;
		info.m_nSrcHeight = height;
		info.m_nDestWidth = dstWidth;
		info.m_nDestHeight = dstHeight;
		info.m_flSrcGamma = srcGamma;
		info.m_flDestGamma = dstGamma;

		ResampleRGBA8888( info );

		// each mipmap level needs to be color converted separately
		ConvertImageFormat( tmpImage.Base(), IMAGE_FORMAT_RGBA8888,
			pDst, imageFormat, dstWidth, dstHeight, 0, 0 );

		if (numLevels == 0)
		{
			// We're done after we've made the 1x1 mip level
			if (dstWidth == 1 && dstHeight == 1)
				return;
		}
		else
		{
			if (--numLevels <= 0)
				return;
		}

		// Figure out where the next level goes
		int memRequired = ImageLoader::GetMemRequired( dstWidth, dstHeight, imageFormat, false);
		pDst += memRequired;

		// shrink by a factor of 2, but clamp at 1 pixel (non-square textures)
		dstWidth = dstWidth > 1 ? dstWidth >> 1 : 1;
		dstHeight = dstHeight > 1 ? dstHeight >> 1 : 1;
	}
}

void ImageLoader::ConvertIA88ImageToNormalMapRGBA8888( unsigned char *src, int width, 
														int height, unsigned char *dst,
														float bumpScale )
{
	float heightScale = ( 1.0f / 255.0f ) * bumpScale;
	float c, cx, cy;
	float maxDim = ( width > height ) ? width : height;
	float ooMaxDim = 1.0f / maxDim;

	int s, t;
	for( t = 0; t < height; t++ )
	{
		unsigned char *dstPixel = &dst[t * width * 4];
		for( s = 0; s < width; s++ )
		{
			c = src[( t * width + s ) * 2];
			cx = src[( t * width + ((s+1)%width) ) * 2];
			cy = src[( ((t+1)%height) * width + s ) * 2];

			// \Z (out of screen)
			//  \
			//   \
			//    \
			//     \-----------  X
			//     |
			//     |
			//     |
			//     |
			//     |
			//     Y
			
			Vector xVect, yVect, normal;
			xVect[0] = ooMaxDim;
			xVect[1] = 0.0f;
			xVect[2] = (cx - c) * heightScale;

			yVect[0] = 0.0f;
			yVect[1] = ooMaxDim;
			yVect[2] = (cy - c) * heightScale;

			CrossProduct( xVect, yVect, normal );
			VectorNormalize( normal );

			
			/* Repack the normalized vector into an RGB unsigned byte
			vector in the normal map image. */
			dstPixel[0] = ( unsigned char )( 128 + 127*normal[0] );
			dstPixel[1] = ( unsigned char )( 128 + 127*normal[1] );
			dstPixel[2] = ( unsigned char )( 128 + 127*normal[2] );
			dstPixel[3] = src[( ( t * width + s ) * 2 ) + 1];
			dstPixel += 4;
		}
	}
}

void ImageLoader::ConvertNormalMapRGBA8888ToDUDVMapUVWQ8888( unsigned char *src, int width, int height,
										                     unsigned char *dst_ )
{
	unsigned char *lastPixel = src + width * height * 4;
	char *dst = ( char * )dst_; // NOTE: this is signed!!!!

	for( ; src < lastPixel; src += 4, dst += 4 )
	{
		dst[0] = ( char )( ( ( int )src[0] ) - 127 );
		dst[1] = ( char )( ( ( int )src[1] ) - 127 );
		dst[2] = ( char )( ( ( int )src[2] ) - 127 );
		dst[3] = ( char )( ( ( int )src[3] ) - 127 );
	}
}

void ImageLoader::ConvertNormalMapRGBA8888ToDUDVMapUVLX8888( unsigned char *src, int width, int height,
										                     unsigned char *dst_ )
{
	unsigned char *lastPixel = src + width * height * 4;
	char *dst = ( char * )dst_; // NOTE: this is signed!!!!

	for( ; src < lastPixel; src += 4, dst += 4 )
	{
		dst[0] = ( char )( ( ( int )src[0] ) - 127 );
		dst[1] = ( char )( ( ( int )src[1] ) - 127 );

		unsigned char *pUDst = (unsigned char *)dst;
		pUDst[2] = src[3];
		pUDst[3] = 0xFF;
	}
}

void ImageLoader::ConvertNormalMapRGBA8888ToDUDVMapUV88( unsigned char *src, int width, int height,
										                     unsigned char *dst_ )
{
	unsigned char *lastPixel = src + width * height * 4;
	char *dst = ( char * )dst_; // NOTE: this is signed!!!!

	for( ; src < lastPixel; src += 4, dst += 2 )
	{
		dst[0] = ( char )( ( ( int )src[0] ) - 127 );
		dst[1] = ( char )( ( ( int )src[1] ) - 127 );
	}
}

void ImageLoader::NormalizeNormalMapRGBA8888( unsigned char *src, int numTexels )
{
	unsigned char *pixel;
	unsigned char *lastPixel = src + numTexels * 4;
	Vector tmpVect;

	for( pixel = src; pixel < lastPixel; pixel += 4 )
	{
		tmpVect[0] = ( ( float )pixel[0] - 128.0f ) * ( 1.0f / 127.0f );
		tmpVect[1] = ( ( float )pixel[1] - 128.0f ) * ( 1.0f / 127.0f );
		tmpVect[2] = ( ( float )pixel[2] - 128.0f ) * ( 1.0f / 127.0f );

		VectorNormalize( tmpVect );

		pixel[0] = ( unsigned char )( 128 + 127 * tmpVect[0] );
		pixel[1] = ( unsigned char )( 128 + 127 * tmpVect[1] );
		pixel[2] = ( unsigned char )( 128 + 127 * tmpVect[2] );
	}
}


//-----------------------------------------------------------------------------
// Image rotation
//-----------------------------------------------------------------------------

bool RotateImageLeft( unsigned char *src, unsigned char *dst, 
					  int widthHeight, ImageFormat imageFormat )
{
#define SRC(x,y) src[((x)+(y)*widthHeight)*sizeInBytes]
#define DST(x,y) dst[((x)+(y)*widthHeight)*sizeInBytes]
	if( IsCompressed( imageFormat ) )
	{
		return false;
	}

	int x, y;

	unsigned char tmp[4][4];
	int halfWidthHeight = widthHeight >> 1;
	int sizeInBytes = SizeInBytes( imageFormat );
	Assert( sizeInBytes <= 4 && sizeInBytes > 0 );

	for( y = 0; y < halfWidthHeight; y++ )
	{
		for( x = 0; x < halfWidthHeight; x++ )
		{
			memcpy( tmp[0], &SRC( x, y ), sizeInBytes );
			memcpy( tmp[1], &SRC( y, widthHeight-x-1 ), sizeInBytes );
			memcpy( tmp[2], &SRC( widthHeight-x-1, widthHeight-y-1 ), sizeInBytes );
			memcpy( tmp[3], &SRC( widthHeight-y-1, x ), sizeInBytes );
			memcpy( &DST( x, y ),                             tmp[3], sizeInBytes );
			memcpy( &DST( y, widthHeight-x-1 ),               tmp[0], sizeInBytes );
			memcpy( &DST( widthHeight-x-1, widthHeight-y-1 ), tmp[1], sizeInBytes );
			memcpy( &DST( widthHeight-y-1, x ),               tmp[2], sizeInBytes );
		}
	}
#undef SRC
#undef DST
	return true;
}

bool RotateImage180( unsigned char *src, unsigned char *dst, 
					  int widthHeight, ImageFormat imageFormat )
{
	// OPTIMIZE: do this transformation directly.
	if( RotateImageLeft( src, dst, widthHeight, imageFormat ) )
	{
		return RotateImageLeft( dst, dst, widthHeight, imageFormat );
	}
	return false;
}

bool FlipImageVertically( unsigned char *src, unsigned char *dst, 
						  int widthHeight, ImageFormat imageFormat )
{
#define SRC(x,y) src[((x)+(y)*widthHeight)*sizeInBytes]
#define DST(x,y) dst[((x)+(y)*widthHeight)*sizeInBytes]
	if( IsCompressed( imageFormat ) )
	{
		return false;
	}

	int x, y;

	unsigned char tmp[4];
	int halfWidthHeight = widthHeight >> 1;
	int sizeInBytes = SizeInBytes( imageFormat );
	Assert( sizeInBytes <= 4 && sizeInBytes > 0 );

	for( y = 0; y < halfWidthHeight; y++ )
	{
		for( x = 0; x < widthHeight; x++ )
		{
			memcpy( tmp, &SRC( x, y ), sizeInBytes );
			memcpy( &SRC( x, y ), &DST( x, widthHeight - y - 1 ), sizeInBytes );
			memcpy( &SRC( x, widthHeight - y - 1 ), tmp, sizeInBytes );
		}
	}
#undef SRC
#undef DST
	return true;
}

bool FlipImageHorizontally( unsigned char *src, unsigned char *dst, 
						    int widthHeight, ImageFormat imageFormat )
{
#define SRC(x,y) src[((x)+(y)*widthHeight)*sizeInBytes]
#define DST(x,y) dst[((x)+(y)*widthHeight)*sizeInBytes]
	if( IsCompressed( imageFormat ) )
	{
		return false;
	}

	int x, y;

	unsigned char tmp[4];
	int halfWidthHeight = widthHeight >> 1;
	int sizeInBytes = SizeInBytes( imageFormat );
	Assert( sizeInBytes <= 4 && sizeInBytes > 0 );

	for( y = 0; y < widthHeight; y++ )
	{
		for( x = 0; x < halfWidthHeight; x++ )
		{
			memcpy( tmp, &SRC( x, y ), sizeInBytes );
			memcpy( &SRC( x, y ), &DST( widthHeight - x - 1, y ), sizeInBytes );
			memcpy( &SRC( widthHeight - x - 1, y ), tmp, sizeInBytes );
		}
	}
#undef SRC
#undef DST
	return true;
}

//-----------------------------------------------------------------------------
// Image rotation
//-----------------------------------------------------------------------------

bool SwapAxes( unsigned char *src, 
			  int widthHeight, ImageFormat imageFormat )
{
#define SRC(x,y) src[((x)+(y)*widthHeight)*sizeInBytes]
	if( IsCompressed( imageFormat ) )
	{
		return false;
	}

	int x, y;

	unsigned char tmp[4];
	int sizeInBytes = SizeInBytes( imageFormat );
	Assert( sizeInBytes <= 4 && sizeInBytes > 0 );

	for( y = 0; y < widthHeight; y++ )
	{
		for( x = 0; x < y; x++ )
		{
			memcpy( tmp, &SRC( x, y ), sizeInBytes );
			memcpy( &SRC( x, y ), &SRC( y, x ), sizeInBytes );
			memcpy( &SRC( y, x ), tmp, sizeInBytes );
		}
	}
#undef SRC
	return true;
}

bool GenMipLevel( unsigned char *src, unsigned char *dst, ImageFormat imageFormat,
							   int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	GenMipMapLevelFunc_t func;
	
	func = GetGenMipMapLevelFunc( imageFormat );
	if( !func )
	{
		return false;
	}
	func( src, dst, srcWidth, srcHeight, dstWidth, dstHeight );
	return true;
}

void RGBA8888ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	memcpy( dst, src, 4 * numPixels );
}

void RGBA8888ToABGR8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, dst += 4 )
	{
		dst[0] = src[3];
		dst[1] = src[2];
		dst[2] = src[1];
		dst[3] = src[0];
	}
}

void RGBA8888ToRGB888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, dst += 3 )
	{
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
	}
}

void RGBA8888ToBGR888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, dst += 3 )
	{
		dst[0] = src[2];
		dst[1] = src[1];
		dst[2] = src[0];
	}
}

void RGBA8888ToRGB565( unsigned char *src, unsigned char *dst, int numPixels )
{
	Assert( 0 );
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, dst += 2 )
	{
	}
}

void RGBA8888ToI8( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, dst += 1 )
	{
		dst[0] = ( unsigned char )( 0.299f * src[0] + 0.587f * src[1] + 0.114f * src[2] );
	}
}

void RGBA8888ToIA88( unsigned char *src, unsigned char *dst, int numPixels )
{
	// fixme: need to find the proper rgb weighting
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, dst += 2 )
	{
		dst[0] = ( unsigned char )( 0.299f * src[0] + 0.587f * src[1] + 0.114f * src[2] );
		dst[1] = src[3];
	}
}

void RGBA8888ToP8( unsigned char *src, unsigned char *dst, int numPixels )
{
	Assert( 0 );
}

void RGBA8888ToA8( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, dst += 1 )
	{
		dst[0] = src[3];
	}
}

void RGBA8888ToRGB888_BLUESCREEN( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, dst += 3 )
	{
		if( src[3] == 0 )
		{
			dst[0] = 0;
			dst[1] = 0;
			dst[2] = 255;
		}
		else
		{
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
		}
	}
}

void RGBA8888ToBGR888_BLUESCREEN( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, dst += 3 )
	{
		if( src[3] == 0 )
		{
			dst[2] = 0;
			dst[1] = 0;
			dst[0] = 255;
		}
		else
		{
			dst[2] = src[0];
			dst[1] = src[1];
			dst[0] = src[2];
		}
	}
}

void RGBA8888ToARGB8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, dst += 4 )
	{
		dst[0] = src[3];
		dst[1] = src[0];
		dst[2] = src[1];
		dst[3] = src[2];
	}
}

void RGBA8888ToBGRA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, dst += 4 )
	{
		dst[0] = src[2];
		dst[1] = src[1];
		dst[2] = src[0];
		dst[3] = src[3];
	}
}

void RGBA8888ToBGRX8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, dst += 4 )
	{
		dst[0] = src[2];
		dst[1] = src[1];
		dst[2] = src[0];
	}
}

void RGBA8888ToBGR565( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned short* pDstShort = (unsigned short*)dst;
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, pDstShort ++ )
	{
		*pDstShort = ((src[0] >> 3) << 11) |
					 ((src[1] >> 2) << 5) |
					  (src[2] >> 3);
	}
}

void RGBA8888ToBGRX5551( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned short* pDstShort = (unsigned short*)dst;
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, pDstShort ++ )
	{
		*pDstShort = ((src[0] >> 3) << 10) |
					 ((src[1] >> 3) << 5) |
					  (src[2] >> 3);
	}
}

void RGBA8888ToBGRA5551( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned short* pDstShort = (unsigned short*)dst;
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, pDstShort ++ )
	{
		*pDstShort = ((src[0] >> 3) << 10) |
					 ((src[1] >> 3) << 5) |
					  (src[2] >> 3) |
					  (src[3] >> 7) << 15;
	}
}

void RGBA8888ToBGRA4444( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned short* pDstShort = (unsigned short*)dst;
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, pDstShort ++ )
	{
		*pDstShort = ((src[0] >> 4) << 8) |
					 ((src[1] >> 4) << 4) |
					  (src[2] >> 4) |
					 ((src[3] >> 4) << 12);
	}
}

void RGBA8888ToUV88( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, dst += 2 )
	{
		dst[0] = src[0];
		dst[1] = src[1];
	}
}

void RGBA8888ToUVWQ8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	RGBA8888ToRGBA8888( src, dst, numPixels );
}

void RGBA8888ToUVLX8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	RGBA8888ToRGBA8888( src, dst, numPixels );
}

void ABGR8888ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, dst += 4 )
	{
		dst[0] = src[3];
		dst[1] = src[2];
		dst[2] = src[1];
		dst[3] = src[0];
	}
}

void RGB888ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 3;
	for( ; src < endSrc; src += 3, dst += 4 )
	{
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		dst[3] = 255;
	}
}

void BGR888ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 3;
	for( ; src < endSrc; src += 3, dst += 4 )
	{
		dst[0] = src[2];
		dst[1] = src[1];
		dst[2] = src[0];
		dst[3] = 255;
	}
}

void RGB565ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	Assert( 0 );
	unsigned char *endSrc = src + numPixels * 2;
	for( ; src < endSrc; src += 2, dst += 4 )
	{
	}
}

void I8ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels;
	for( ; src < endSrc; src += 1, dst += 4 )
	{
		dst[0] = src[0];
		dst[1] = src[0];
		dst[2] = src[0];
		dst[3] = 255;
	}
}

void IA88ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 2;
	for( ; src < endSrc; src += 2, dst += 4 )
	{
		dst[0] = src[0];
		dst[1] = src[0];
		dst[2] = src[0];
		dst[3] = src[1];
	}
}

void P8ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	Assert( 0 );
}

void A8ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels;
	for( ; src < endSrc; src += 1, dst += 4 )
	{
		dst[0] = 255;
		dst[1] = 255;
		dst[2] = 255;
		dst[3] = src[0];
	}
}

void RGB888_BLUESCREENToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 3;
	for( ; src < endSrc; src += 3, dst += 4 )
	{
		if( src[0] == 0 && src[1] == 0 && src[2] == 255 )
		{
			dst[0] = 0;
			dst[1] = 0;
			dst[2] = 0;
			dst[3] = 0;
		}
		else
		{
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
			dst[3] = 255;
		}
	}
}

void BGR888_BLUESCREENToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 3;
	for( ; src < endSrc; src += 3, dst += 4 )
	{
		if( src[2] == 0 && src[1] == 0 && src[0] == 255 )
		{
			dst[0] = 0;
			dst[1] = 0;
			dst[2] = 0;
			dst[3] = 0;
		}
		else
		{
			dst[2] = src[0];
			dst[1] = src[1];
			dst[0] = src[2];
			dst[3] = 255;
		}
	}
}

void ARGB8888ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, dst += 4 )
	{
		dst[0] = src[1];
		dst[1] = src[2];
		dst[2] = src[3];
		dst[3] = src[0];
	}
}

void BGRA8888ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, dst += 4 )
	{
		dst[0] = src[2];
		dst[1] = src[1];
		dst[2] = src[0];
		dst[3] = src[3];
	}
}

void BGRX8888ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char *endSrc = src + numPixels * 4;
	for( ; src < endSrc; src += 4, dst += 4 )
	{
		dst[0] = src[2];
		dst[1] = src[1];
		dst[2] = src[0];
		dst[3] = 255;
	}
}

void BGR565ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned short* pSrcShort = (unsigned short*)src;
	unsigned short* pEndSrc = pSrcShort + numPixels;
	for( ; pSrcShort < pEndSrc; pSrcShort++, dst += 4 )
	{
		int blue = (*pSrcShort & 0x1F);
		int green = (*pSrcShort >> 5) & 0x3F;
		int red = (*pSrcShort >> 11) & 0x1F;

		// Expand to 8 bits
		dst[0] = (red << 3) | (red >> 2);
		dst[1] = (green << 2) | (green >> 4);
		dst[2] = (blue << 3) | (blue >> 2);
		dst[3] = 255;
	}
}

void BGRX5551ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned short* pSrcShort = (unsigned short*)src;
	unsigned short* pEndSrc = pSrcShort + numPixels;
	for( ; pSrcShort < pEndSrc; pSrcShort++, dst += 4 )
	{
		int blue = (*pSrcShort & 0x1F);
		int green = (*pSrcShort >> 5) & 0x1F;
		int red = (*pSrcShort >> 10) & 0x1F;

		// Expand to 8 bits
		dst[0] = (red << 3) | (red >> 2);
		dst[1] = (green << 3) | (green >> 2);
		dst[2] = (blue << 3) | (blue >> 2);
		dst[3] = 255;
	}
}

void BGRA5551ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned short* pSrcShort = (unsigned short*)src;
	unsigned short* pEndSrc = pSrcShort + numPixels;
	for( ; pSrcShort < pEndSrc; pSrcShort++, dst += 4 )
	{
		int blue = (*pSrcShort & 0x1F);
		int green = (*pSrcShort >> 5) & 0x1F;
		int red = (*pSrcShort >> 10) & 0x1F;
		int alpha = *pSrcShort & ( 1 << 15 );

		// Expand to 8 bits
		dst[0] = (red << 3) | (red >> 2);
		dst[1] = (green << 3) | (green >> 2);
		dst[2] = (blue << 3) | (blue >> 2);
		// garymcthack
		if( alpha )
		{
			dst[3] = 255;
		}
		else
		{
			dst[3] = 0;
		}
	}
}

void BGRA4444ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned short* pSrcShort = (unsigned short*)src;
	unsigned short* pEndSrc = pSrcShort + numPixels;
	for( ; pSrcShort < pEndSrc; pSrcShort++, dst += 4 )
	{
		int blue = (*pSrcShort & 0xF);
		int green = (*pSrcShort >> 4) & 0xF;
		int red = (*pSrcShort >> 8) & 0xF;
		int alpha = (*pSrcShort >> 12) & 0xF;

		// Expand to 8 bits
		// FIXME: shouldn't this be (red << 4) | red?
		dst[0] = (red << 4) | (red >> 4);
		dst[1] = (green << 4) | (green >> 4);
		dst[2] = (blue << 4) | (blue >> 4);
		dst[3] = (alpha << 4) | (alpha >> 4);
	}
}

void UV88ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	unsigned char* pEndSrc = src + numPixels * 2;
	for( ; src < pEndSrc; src += 2, dst += 4 )
	{
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = 0;
		dst[3] = 0;
	}
}

void UVWQ8888ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	RGBA8888ToRGBA8888( src, dst, numPixels );
}

void UVLX8888ToRGBA8888( unsigned char *src, unsigned char *dst, int numPixels )
{
	RGBA8888ToRGBA8888( src, dst, numPixels );
}

// FIXME: these all need to convert to linear space before doing mipmapping!
// FIXME: optimize
void GenMipMapLevelRGBA8888( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	int x, y, i;
#define SRCINDEX(x,y,c) ( 4 * ( (x) + (y) * srcWidth ) + (c) )
#define DSTINDEX(x,y,c) ( 4 * ( (x) + (y) * dstWidth ) + (c) )

	if( ( srcWidth == 1 ) && ( dstWidth == 1 ) )
	{
		for( y = 0; y < dstHeight; y++ )
		{
			for( i = 0; i < 4; i++ )
			{
				dst[DSTINDEX(0,y,i)] = 
					( src[SRCINDEX(0,(y<<1)+0,i)] +
					  src[SRCINDEX(0,(y<<1)+1,i)] ) >> 2;
			}
		}
	}
	else if( ( srcHeight == 1 ) && ( dstHeight == 1 ) )
	{
		for( x = 0; x < dstWidth; x++ )
		{
			for( i = 0; i < 4; i++ )
			{
				dst[DSTINDEX(x,0,i)] = 
					( src[SRCINDEX((x<<1)+0,0,i)] +
					  src[SRCINDEX((x<<1)+1,0,i)] ) >> 2;
			}
		}
	}
	else if ( dstHeight == (srcHeight>>1) && dstWidth == (srcWidth>>1) )
	{
		for( y = 0; y < dstHeight; y++ )
		{
			for( x = 0; x < dstWidth; x++ )
			{
				for( i = 0; i < 4; i++ )
				{
					dst[DSTINDEX(x,y,i)] = 
						( src[SRCINDEX((x<<1)+0,(y<<1)+0,i)] + 
						  src[SRCINDEX((x<<1)+1,(y<<1)+0,i)] + 
						  src[SRCINDEX((x<<1)+1,(y<<1)+1,i)] + 
						  src[SRCINDEX((x<<1)+0,(y<<1)+1,i)] ) >> 2;
				}
			}
		}
	}
	else
	{
		// HACKHACK: Bugly 2x2 filter over arbitrary sized texture
		float dx = (float)srcWidth / (float)dstWidth;
		float dy = (float)srcHeight / (float)dstHeight;

		for ( y = 0 ; y < dstHeight; y++ )
		{
			for( x = 0; x < dstWidth; x++ )
			{
				int u = (int)(x*dx);
				int v = (int)(y*dy);
				for( i = 0; i < 4; i++ )
				{
					dst[DSTINDEX(x,y,i)] = 
						( src[SRCINDEX(u+0,v+0,i)] + 
						  src[SRCINDEX(u+1,v+0,i)] + 
						  src[SRCINDEX(u+1,v+1,i)] + 
						  src[SRCINDEX(u+0,v+1,i)] ) >> 2;
				}
			}
		}
	}

#undef SRCINDEX
#undef DSTINDEX
}

void GenMipMapLevelABGR8888( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	Assert( 0 );
}

void GenMipMapLevelRGB888( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	int x, y, i;
#define SRCINDEX(x,y,c) ( 3 * ( (x) + (y) * srcWidth ) + (c) )
#define DSTINDEX(x,y,c) ( 3 * ( (x) + (y) * dstWidth ) + (c) )

	if( ( srcWidth == 1 ) && ( dstWidth == 1 ) )
	{
		for( y = 0; y < dstHeight; y++ )
		{
			for( i = 0; i < 3; i++ )
			{
				dst[DSTINDEX(0,y,i)] = 
					( src[SRCINDEX(0,(y<<1)+0,i)] +
					  src[SRCINDEX(0,(y<<1)+1,i)] ) >> 2;
			}
		}
	}
	else if( ( srcHeight == 1 ) && ( dstHeight == 1 ) )
	{
		for( x = 0; x < dstWidth; x++ )
		{
			for( i = 0; i < 3; i++ )
			{
				dst[DSTINDEX(x,0,i)] = 
					( src[SRCINDEX((x<<1)+0,0,i)] +
					  src[SRCINDEX((x<<1)+1,0,i)] ) >> 2;
			}
		}
	}
	else
	{
		for( y = 0; y < dstHeight; y++ )
		{
			for( x = 0; x < dstWidth; x++ )
			{
				for( i = 0; i < 3; i++ )
				{
					dst[DSTINDEX(x,y,i)] = 
						( src[SRCINDEX((x<<1)+0,(y<<1)+0,i)] + 
						  src[SRCINDEX((x<<1)+1,(y<<1)+0,i)] + 
						  src[SRCINDEX((x<<1)+1,(y<<1)+1,i)] + 
						  src[SRCINDEX((x<<1)+0,(y<<1)+1,i)] ) >> 2;
				}
			}
		}
	}
#undef SRCINDEX
#undef DSTINDEX
}

void GenMipMapLevelBGR888( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	Assert( 0 );
}

void GenMipMapLevelRGB565( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	Assert( 0 );
}

void GenMipMapLevelI8( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	int x, y;
#define SRCINDEX(x,y) ( ( (x) + (y) * srcWidth ) )
#define DSTINDEX(x,y) ( ( (x) + (y) * dstWidth ) )

	if( ( srcWidth == 1 ) && ( dstWidth == 1 ) )
	{
		for( y = 0; y < dstHeight; y++ )
		{
			dst[DSTINDEX(0,y)] = 
				( src[SRCINDEX(0,(y<<1)+0)] +
				src[SRCINDEX(0,(y<<1)+1)] ) >> 2;
		}
	}
	else if( ( srcHeight == 1 ) && ( dstHeight == 1 ) )
	{
		for( x = 0; x < dstWidth; x++ )
		{
			dst[DSTINDEX(x,0)] = 
				( src[SRCINDEX((x<<1)+0,0)] +
				src[SRCINDEX((x<<1)+1,0)] ) >> 2;
		}
	}
	else if ( dstHeight == (srcHeight>>1) && dstWidth == (srcWidth>>1) )
	{
		for( y = 0; y < dstHeight; y++ )
		{
			for( x = 0; x < dstWidth; x++ )
			{
				dst[DSTINDEX(x,y)] = 
					( src[SRCINDEX((x<<1)+0,(y<<1)+0)] + 
					src[SRCINDEX((x<<1)+1,(y<<1)+0)] + 
					src[SRCINDEX((x<<1)+1,(y<<1)+1)] + 
					src[SRCINDEX((x<<1)+0,(y<<1)+1)] ) >> 2;
			}
		}
	}
	else
	{
		// HACKHACK: Bugly 2x2 filter over arbitrary sized texture
		float dx = (float)srcWidth / (float)dstWidth;
		float dy = (float)srcHeight / (float)dstHeight;

		for ( y = 0 ; y < dstHeight; y++ )
		{
			for( x = 0; x < dstWidth; x++ )
			{
				int u = (int)(x*dx);
				int v = (int)(y*dy);
				dst[DSTINDEX(x,y)] = 
					( src[SRCINDEX(u+0,v+0)] + 
					src[SRCINDEX(u+1,v+0)] + 
					src[SRCINDEX(u+1,v+1)] + 
					src[SRCINDEX(u+0,v+1)] ) >> 2;
			}
		}
	}

#undef SRCINDEX
#undef DSTINDEX
}

void GenMipMapLevelIA88( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	Assert( 0 );
}

void GenMipMapLevelA8( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	Assert( 0 );
}

void GenMipMapLevelARGB8888( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	GenMipMapLevelRGBA8888(src,dst,srcWidth,srcHeight,dstWidth,dstHeight);
}

void GenMipMapLevelBGRA8888( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	GenMipMapLevelRGBA8888(src,dst,srcWidth,srcHeight,dstWidth,dstHeight);
}

void GenMipMapLevelBGRX8888( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	GenMipMapLevelRGBA8888(src,dst,srcWidth,srcHeight,dstWidth,dstHeight);
}

void GenMipMapLevelBGR565( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	Assert( 0 );
}

void GenMipMapLevelBGRX5551( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	Assert( 0 );
}

void GenMipMapLevelBGRA5551( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	Assert( 0 );
}

void GenMipMapLevelBGRA4444( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	Assert( 0 );
}

void GenMipMapLevelUV88( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	int x, y, i;
#define SRCINDEX(x,y,c) ( 2 * ( (x) + (y) * srcWidth ) + (c) )
#define DSTINDEX(x,y,c) ( 2 * ( (x) + (y) * dstWidth ) + (c) )

	if( ( srcWidth == 1 ) && ( dstWidth == 1 ) )
	{
		for( y = 0; y < dstHeight; y++ )
		{
			for( i = 0; i < 2; i++ )
			{
				dst[DSTINDEX(0,y,i)] = 
					( src[SRCINDEX(0,(y<<1)+0,i)] +
					  src[SRCINDEX(0,(y<<1)+1,i)] ) >> 2;
			}
		}
	}
	else if( ( srcHeight == 1 ) && ( dstHeight == 1 ) )
	{
		for( x = 0; x < dstWidth; x++ )
		{
			for( i = 0; i < 2; i++ )
			{
				dst[DSTINDEX(x,0,i)] = 
					( src[SRCINDEX((x<<1)+0,0,i)] +
					  src[SRCINDEX((x<<1)+1,0,i)] ) >> 2;
			}
		}
	}
	else
	{
		for( y = 0; y < dstHeight; y++ )
		{
			for( x = 0; x < dstWidth; x++ )
			{
				for( i = 0; i < 2; i++ )
				{
					dst[DSTINDEX(x,y,i)] = 
						( src[SRCINDEX((x<<1)+0,(y<<1)+0,i)] + 
						  src[SRCINDEX((x<<1)+1,(y<<1)+0,i)] + 
						  src[SRCINDEX((x<<1)+1,(y<<1)+1,i)] + 
						  src[SRCINDEX((x<<1)+0,(y<<1)+1,i)] ) >> 2;
				}
			}
		}
	}
#undef SRCINDEX
#undef DSTINDEX
}

void GenMipMapLevelUVWQ8888( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	GenMipMapLevelRGBA8888( src, dst, srcWidth, srcHeight, dstWidth, dstHeight );
}

void GenMipMapLevelUVLX8888( unsigned char *src, unsigned char *dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight )
{
	GenMipMapLevelRGBA8888( src, dst, srcWidth, srcHeight, dstWidth, dstHeight );
}

} // ImageLoader namespace ends
