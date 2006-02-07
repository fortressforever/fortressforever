//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_glyph.h
//	@author Patrick O'Leary (Mulchman)
//	@date 09/19/2005
//	@brief Some common glyph code stuff
//
//	REVISIONS
//	---------
//	09/19/2005, Mulchman: 
//		First created. Making glyph code
//		shared between scout radar and
//		radio tagging.

#ifndef FF_GLYPH_H
#define FF_GLYPH_H

// If we haven't received another update in
// this amount of seconds, stop drawing the
// box around the player
#define FF_RADIOTAG_TIMETOFORGET	0.7f
#define FF_RADIOTAG_NUMGLYPHS		10	// 10 classes...

struct Glyph_s
{
	CHudTexture	*m_pTexture;
	string_t	m_szMaterial;
};

// Just a wrapper class for glyphs and
// animation and ESP
class CGlyphESP : public ESP_Shared_s
{
protected:
	int m_iFrame;
	float m_flLastTime;

public:
	CGlyphESP( void )
	{
		m_iFrame = 0;
		m_flLastTime = 0.0;
	}

	// Returns what frame to use
	// for the animation and also
	// increments the frame for the
	// next usage
	int UpdateFrame( void )
	{
		int iRetval = m_iFrame;

		if( m_iFrame > 15 )
			iRetval = 15;

		// Only increase the frame if a certain amount
		// of game time has gone by
		if( gpGlobals->curtime > ( m_flLastTime + 0.0001f ) )
		{
			m_iFrame++;
			m_flLastTime = gpGlobals->curtime;
		}

		if( m_iFrame > 35 )
			m_iFrame = 0;

		return iRetval;
	}
};

void CacheGlyphs( void );

extern Glyph_s	g_ClassGlyphs[ FF_RADIOTAG_NUMGLYPHS ];
extern Glyph_s	g_RadioTowerGlyphs[ 16 ];
extern bool		g_bGlyphsCached;

#endif // FF_GLYPH_H
