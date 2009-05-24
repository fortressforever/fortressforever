//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======
//
// Baked Bitmap fonts
//
//===========================================================================

#ifndef _BITMAPFONTFILE_H_
#define _BITMAPFONTFILE_H_

#define BITMAPFONT_ID		(('T'<<24)|('N'<<16)|('F'<<8)|('V'))
#define BITMAPFONT_VERSION	3

// style flags
#define BF_BOLD			0x0001
#define BF_ITALIC		0x0002
#define BF_OUTLINED		0x0004
#define BF_DROPSHADOW	0x0008
#define BF_BLURRED		0x0010
#define BF_SCANLINES	0x0020
#define BF_ANTIALIASED	0x0040
#define BF_CUSTOM		0x0080

#pragma pack(1)

typedef struct 
{
	short	x;
	short	y;
	short	w;
	short	h;
	short	a;
	short	b;
	short	c;
} BitmapGlyph_t;

typedef struct 
{
	int				m_id;
	int				m_Version;
	short			m_PageWidth;
	short			m_PageHeight;
	short			m_MaxCharWidth;
	short			m_MaxCharHeight;
	short			m_Flags;
	short			m_Ascent;
	short			m_NumGlyphs;
	unsigned char	m_TranslateTable[256];
} BitmapFont_t;

#pragma pack()

#endif