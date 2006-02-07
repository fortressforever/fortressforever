//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef PIXELWRITER_H
#define PIXELWRITER_H

#include "imageloader.h"
#include "tier0/dbg.h"

//-----------------------------------------------------------------------------
// Color writing class 
//-----------------------------------------------------------------------------

class CPixelWriter
{
public:
	FORCEINLINE void SetPixelMemory( ImageFormat format, void* pMemory, int stride );
	FORCEINLINE void Seek( int x, int y );
	FORCEINLINE void* SkipBytes( int n );
	FORCEINLINE void SkipPixels( int n );	
	FORCEINLINE void WritePixel( int r, int g, int b, int a = 255 );
	FORCEINLINE void WritePixelNoAdvance( int r, int g, int b, int a = 255 );
	FORCEINLINE void WritePixelSigned( int r, int g, int b, int a = 255 );
	FORCEINLINE void WritePixelNoAdvanceSigned( int r, int g, int b, int a = 255 );
	FORCEINLINE void ReadPixelNoAdvance( int &r, int &g, int &b, int &a );
	FORCEINLINE unsigned char GetPixelSize() { return m_Size; }	
private:
	unsigned char* m_pBase;
	unsigned char* m_pBits;
	unsigned short m_BytesPerRow;
	unsigned char m_Size;
	signed char m_RShift;
	signed char m_GShift;
	signed char m_BShift;
	signed char m_AShift;
	unsigned char m_RMask;
	unsigned char m_GMask;
	unsigned char m_BMask;
	unsigned char m_AMask;
};

FORCEINLINE void CPixelWriter::SetPixelMemory( ImageFormat format, void* pMemory, int stride )
{
	m_pBits = (unsigned char*)pMemory;
	m_pBase = m_pBits;
	m_BytesPerRow = (unsigned short)stride;
	switch(format)
	{
	case IMAGE_FORMAT_BGRA8888: // NOTE! : the low order bits are first in this naming convention.
		m_Size = 4;
		m_RShift = 16;
		m_GShift = 8;
		m_BShift = 0;
		m_AShift = 24;
		m_RMask = 0xFF;
		m_GMask = 0xFF;
		m_BMask = 0xFF;
		m_AMask = 0xFF;
		break;

	case IMAGE_FORMAT_BGRX8888:
		m_Size = 4;
		m_RShift = 16;
		m_GShift = 8;
		m_BShift = 0;
		m_AShift = 24;
		m_RMask = 0xFF;
		m_GMask = 0xFF;
		m_BMask = 0xFF;
		m_AMask = 0x00;
		break;

	case IMAGE_FORMAT_BGRA4444:
		m_Size = 2;
		m_RShift = 4;
		m_GShift = 0;
		m_BShift = -4;
		m_AShift = 8;
		m_RMask = 0xF0;
		m_GMask = 0xF0;
		m_BMask = 0xF0;
		m_AMask = 0xF0;
		break;

	case IMAGE_FORMAT_BGR888:
		m_Size = 3;
		m_RShift = 16;
		m_GShift = 8;
		m_BShift = 0;
		m_AShift = 0;
		m_RMask = 0xFF;
		m_GMask = 0xFF;
		m_BMask = 0xFF;
		m_AMask = 0x00;
		break;

	case IMAGE_FORMAT_BGR565:
		m_Size = 2;
		m_RShift = 8;
		m_GShift = 3;
		m_BShift = -3;
		m_AShift = 0;
		m_RMask = 0xF8;
		m_GMask = 0xFC;
		m_BMask = 0xF8;
		m_AMask = 0x00;
		break;

	case IMAGE_FORMAT_BGRA5551:
	case IMAGE_FORMAT_BGRX5551:
		m_Size = 2;
		m_RShift = 7;
		m_GShift = 2;
		m_BShift = -3;
		m_AShift = 8;
		m_RMask = 0xF8;
		m_GMask = 0xF8;
		m_BMask = 0xF8;
		m_AMask = 0x80;
		break;

	// GR - alpha format for HDR support
	case IMAGE_FORMAT_A8:
		m_Size = 1;
		m_RShift = 0;
		m_GShift = 0;
		m_BShift = 0;
		m_AShift = 0;
		m_RMask = 0x00;
		m_GMask = 0x00;
		m_BMask = 0x00;
		m_AMask = 0xFF;
		break;

	// GR - alpha format for HDR support
	case IMAGE_FORMAT_UVWQ8888:
		m_Size = 4;
		m_RShift = 0;
		m_GShift = 8;
		m_BShift = 16;
		m_AShift = 24;
		m_RMask = 0xFF;
		m_GMask = 0xFF;
		m_BMask = 0xFF;
		m_AMask = 0xFF;
		break;

	// FIXME: Add more color formats as need arises
	default:
		{
			static bool format_error_printed[ NUM_IMAGE_FORMATS ];

			if ( !format_error_printed[ format ] )
			{
				Assert(0);
				Msg( "CPixelWriter::SetPixelMemory:  Unsupported image format %i\n",
					format );
				format_error_printed[ format ] = true;
			}
			m_Size = 0; // set to zero so that we don't stomp memory for formats that we don't understand.
		}
		break;
	}						   
}

//-----------------------------------------------------------------------------
// Sets where we're writing to
//-----------------------------------------------------------------------------

FORCEINLINE void CPixelWriter::Seek( int x, int y )
{
	m_pBits = m_pBase + y * m_BytesPerRow + x * m_Size;
}

//-----------------------------------------------------------------------------
// Skips n bytes:
//-----------------------------------------------------------------------------

FORCEINLINE void* CPixelWriter::SkipBytes( int n )
{
	m_pBits += n;
	return m_pBits;
}

//-----------------------------------------------------------------------------
// Skips n pixels:
//-----------------------------------------------------------------------------

FORCEINLINE void CPixelWriter::SkipPixels( int n )
{
	SkipBytes( n * m_Size );
}

//-----------------------------------------------------------------------------
// Writes a pixel, advances the write index 
//-----------------------------------------------------------------------------

FORCEINLINE void CPixelWriter::WritePixel( int r, int g, int b, int a )
{
	WritePixelNoAdvance(r,g,b,a);
	m_pBits += m_Size;
}

//-----------------------------------------------------------------------------
// Writes a pixel, advances the write index 
//-----------------------------------------------------------------------------

FORCEINLINE void CPixelWriter::WritePixelSigned( int r, int g, int b, int a )
{
	WritePixelNoAdvanceSigned(r,g,b,a);
	m_pBits += m_Size;
}

//-----------------------------------------------------------------------------
// Writes a pixel without advancing the index
//-----------------------------------------------------------------------------

FORCEINLINE void CPixelWriter::WritePixelNoAdvance( int r, int g, int b, int a )
{
	int val = (r & m_RMask) << m_RShift;
	val |=  (g & m_GMask) << m_GShift;
	val |= (m_BShift > 0) ? ((b & m_BMask) << m_BShift) : ((b & m_BMask) >> -m_BShift);
	val |=	(a & m_AMask) << m_AShift;

	m_pBits[0] = (unsigned char)((val & 0xff));
	m_pBits[1] = (unsigned char)((val >> 8) & 0xff);
	if (m_Size > 1)
	{
		m_pBits[1] = (unsigned char)((val >> 8) & 0xff);
		if (m_Size > 2)
		{
			m_pBits[2] = (unsigned char)((val >> 16) & 0xff);
			if (m_Size > 3)
				m_pBits[3] = (unsigned char)((val >> 24) & 0xff);
		}
	}
}

FORCEINLINE void CPixelWriter::WritePixelNoAdvanceSigned( int r, int g, int b, int a )
{
	int val = (r & m_RMask) << m_RShift;
	val |=  (g & m_GMask) << m_GShift;
	val |= (m_BShift > 0) ? ((b & m_BMask) << m_BShift) : ((b & m_BMask) >> -m_BShift);
	val |=	(a & m_AMask) << m_AShift;

	signed char *pSignedBits = ( signed char * )m_pBits;
	pSignedBits[0] = (signed char)((val & 0xff));
	pSignedBits[1] = (signed char)((val >> 8) & 0xff);
	if (m_Size > 1)
	{
		pSignedBits[1] = (signed char)((val >> 8) & 0xff);
		if (m_Size > 2)
		{
			pSignedBits[2] = (signed char)((val >> 16) & 0xff);
			if (m_Size > 3)
				pSignedBits[3] = (signed char)((val >> 24) & 0xff);
		}
	}
}

FORCEINLINE void CPixelWriter::ReadPixelNoAdvance( int &r, int &g, int &b, int &a )
{
	int val = m_pBits[0];
	if (m_Size > 1)
	{
		val |= (int)m_pBits[1] << 8;
		if (m_Size > 2)
		{
			val |= (int)m_pBits[2]  << 16;
			if (m_Size > 3)
				val |= (int)m_pBits[3] << 24;
		}
	}

	r = (val>>m_RShift) & m_RMask;
	g = (val>>m_GShift) & m_GMask;
	b = (val>>m_BShift) & m_BMask;
	a = (val>>m_AShift) & m_AMask;
}

#endif // PIXELWRITER_H;
