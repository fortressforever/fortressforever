//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include <ctype.h>
#include "sentence.h"
#include "hud_closecaption.h"
#include "vstdlib/strtools.h"
#include <vgui_controls/Controls.h>
#include <vgui/IVgui.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include "iclientmode.h"
#include "hud_macros.h"
#include "checksum_crc.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define CC_INSET		12

// Marked as FCVAR_USERINFO so that the server can cull CC messages before networking them down to us!!!
ConVar closecaption( "closecaption", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Enable close captioning." );
extern ConVar cc_lang;
static ConVar cc_linger_time( "cc_linger_time", "1.0", FCVAR_ARCHIVE, "Close caption linger time." );
static ConVar cc_predisplay_time( "cc_predisplay_time", "0.25", FCVAR_ARCHIVE, "Close caption delay before showing caption." );
static ConVar cc_captiontrace( "cc_captiontrace", "1", 0, "Show missing closecaptions (0 = no, 1 = devconsole, 2 = show in hud)" );
static ConVar cc_subtitles( "cc_subtitles", "0", FCVAR_ARCHIVE, "If set, don't show sound effect captions, just voice overs (i.e., won't help hearing impaired players)." );
static ConVar english( "english", "1", FCVAR_USERINFO, "If set to 1, running the english language set of assets." );

#define	MAX_CAPTION_CHARACTERS 2048

//-----------------------------------------------------------------------------
// Purpose: Helper for sentence.cpp
// Input  : *ansi - 
//			*unicode - 
//			unicodeBufferSize - 
// Output : int
//-----------------------------------------------------------------------------
int ConvertANSIToUnicode(const char *ansi, wchar_t *unicode, int unicodeBufferSize)
{
	return vgui::localize()->ConvertANSIToUnicode( ansi, unicode, unicodeBufferSize );
}

// A work unit is a pre-processed chunk of CC text to display
// Any state changes (font/color/etc) cause a new work unit to be precomputed
// Moving onto a new line also causes a new Work Unit
// The width and height are stored so that layout can be quickly recomputed each frame
class CCloseCaptionWorkUnit
{
public:
	CCloseCaptionWorkUnit();
	~CCloseCaptionWorkUnit();

	void	SetWidth( int w );
	int		GetWidth() const;

	void	SetHeight( int h );
	int		GetHeight() const;

	void	SetPos( int x, int y );
	void	GetPos( int& x, int &y ) const;

	void	SetBold( bool bold );
	bool	GetBold() const;

	void	SetItalic( bool ital );
	bool	GetItalic() const;

	void	SetStream( const wchar_t *stream );
	const wchar_t	*GetStream() const;

	void	SetColor( Color& clr );
	Color GetColor() const;

	int		GetFontNumber() const
	{
		return CHudCloseCaption::GetFontNumber( m_bBold, m_bItalic );
	}
	
	void Dump()
	{
		char buf[ 2048 ];
		vgui::localize()->ConvertUnicodeToANSI( GetStream(), buf, sizeof( buf ) );

		Msg( "x = %i, y = %i, w = %i h = %i text %s\n", m_nX, m_nY, m_nWidth, m_nHeight, buf );
	}

private:

	int				m_nX;
	int				m_nY;
	int				m_nWidth;
	int				m_nHeight;

	bool			m_bBold;
	bool			m_bItalic;
	wchar_t			*m_pszStream;
	Color		m_Color;
};

CCloseCaptionWorkUnit::CCloseCaptionWorkUnit() :
	m_nWidth(0),
	m_nHeight(0),
	m_bBold(false),
	m_bItalic(false),
	m_pszStream(0),
	m_Color( Color( 255, 255, 255, 255 ) )
{
}

CCloseCaptionWorkUnit::~CCloseCaptionWorkUnit()
{
	delete[] m_pszStream;
	m_pszStream = NULL;
}

void CCloseCaptionWorkUnit::SetWidth( int w )
{
	m_nWidth = w;
}

int CCloseCaptionWorkUnit::GetWidth() const
{
	return m_nWidth;
}

void CCloseCaptionWorkUnit::SetHeight( int h )
{
	m_nHeight = h;
}

int CCloseCaptionWorkUnit::GetHeight() const
{
	return m_nHeight;
}

void CCloseCaptionWorkUnit::SetPos( int x, int y )
{
	m_nX = x;
	m_nY = y;
}

void CCloseCaptionWorkUnit::GetPos( int& x, int &y ) const
{
	x = m_nX;
	y = m_nY;
}

void CCloseCaptionWorkUnit::SetBold( bool bold )
{
	m_bBold = bold;
}

bool CCloseCaptionWorkUnit::GetBold() const
{
	return m_bBold;
}

void CCloseCaptionWorkUnit::SetItalic( bool ital )
{
	m_bItalic = ital;
}

bool CCloseCaptionWorkUnit::GetItalic() const
{
	return m_bItalic;
}

void CCloseCaptionWorkUnit::SetStream( const wchar_t *stream )
{
	delete[] m_pszStream;
	m_pszStream = NULL;

	int len = wcslen( stream );
	Assert( len < 4096 );
	m_pszStream = new wchar_t[ len + 1 ];
	wcsncpy( m_pszStream, stream, len );
	m_pszStream[ len ] = L'\0';
}

const wchar_t *CCloseCaptionWorkUnit::GetStream() const
{
	return m_pszStream ? m_pszStream : L"";
}

void CCloseCaptionWorkUnit::SetColor( Color& clr )
{
	m_Color = clr;
}

Color CCloseCaptionWorkUnit::GetColor() const
{
	return m_Color;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CCloseCaptionItem
{
public:
	CCloseCaptionItem( 
		const wchar_t	*stream,
		float timetolive,
		float addedtime,
		float predisplay,
		bool valid,
		bool fromplayer
	) :
		m_flTimeToLive( 0.0f ),
		m_flAddedTime( addedtime ),
		m_bValid( false ),
		m_nTotalWidth( 0 ),
		m_nTotalHeight( 0 ),
		m_bSizeComputed( false ),
		m_bFromPlayer( fromplayer )

	{
		SetStream( stream );
		SetTimeToLive( timetolive );
		SetInitialLifeSpan( timetolive );
		SetPreDisplayTime( cc_predisplay_time.GetFloat() + predisplay );
		m_bValid = valid;
	}

	CCloseCaptionItem( const CCloseCaptionItem& src )
	{
		SetStream( src.m_szStream );
		m_flTimeToLive = src.m_flTimeToLive;
		m_bValid = src.m_bValid;
		m_bFromPlayer = src.m_bFromPlayer;
		m_flAddedTime = src.m_flAddedTime;
	}

	~CCloseCaptionItem( void )
	{
		while ( m_Work.Count() > 0 )
		{
			CCloseCaptionWorkUnit *unit = m_Work[ 0 ];
			m_Work.Remove( 0 );
			delete unit;
		}

	}

	void SetStream( const wchar_t *stream)
	{
		wcsncpy( m_szStream, stream, sizeof( m_szStream ) / sizeof( wchar_t ) );
	}

	const wchar_t *GetStream() const
	{
		return m_szStream;
	}

	void SetTimeToLive( float ttl )
	{
		m_flTimeToLive = ttl;
	}

	float GetTimeToLive( void ) const
	{
		return m_flTimeToLive;
	}

	void SetInitialLifeSpan( float t )
	{
		m_flInitialLifeSpan = t;
	}

	float GetInitialLifeSpan() const
	{
		return m_flInitialLifeSpan;
	}

	bool IsValid() const
	{
		return m_bValid;
	}

	void	SetHeight( int h )
	{
		m_nTotalHeight = h;
	}
	int		GetHeight() const
	{
		return m_nTotalHeight;
	}
	void	SetWidth( int w )
	{
		m_nTotalWidth = w;
	}
	int		GetWidth() const
	{
		return m_nTotalWidth;
	}

	void	AddWork( CCloseCaptionWorkUnit *unit )
	{
		m_Work.AddToTail( unit );
	}

	int		GetNumWorkUnits() const
	{
		return m_Work.Count();
	}

	CCloseCaptionWorkUnit *GetWorkUnit( int index )
	{
		Assert( index >= 0 && index < m_Work.Count() );

		return m_Work[ index ];
	}

	void		SetSizeComputed( bool computed )
	{
		m_bSizeComputed = computed;
	}

	bool		GetSizeComputed() const
	{
		return m_bSizeComputed;
	}

	void		SetPreDisplayTime( float t )
	{
		m_flPreDisplayTime = t;
	}

	float		GetPreDisplayTime() const
	{
		return m_flPreDisplayTime;
	}

	float		GetAlpha( float fadeintimehidden, float fadeintime, float fadeouttime )
	{
		float time_since_start = m_flInitialLifeSpan - m_flTimeToLive;
		float time_until_end =  m_flTimeToLive;

		float totalfadeintime = fadeintimehidden + fadeintime;

		if ( totalfadeintime > 0.001f && 
			time_since_start < totalfadeintime )
		{
			if ( time_since_start >= fadeintimehidden )
			{
				float f = 1.0f;
				if ( fadeintime > 0.001f )
				{
					f = ( time_since_start - fadeintimehidden ) / fadeintime;
				}
				f = clamp( f, 0.0f, 1.0f );
				return f;
			}
			
			return 0.0f;
		}

		if ( fadeouttime > 0.001f &&
			time_until_end < fadeouttime )
		{
			float f = time_until_end / fadeouttime;
			f = clamp( f, 0.0f, 1.0f );
			return f;
		}

		return 1.0f;
	}

	float	GetAddedTime() const
	{
		return m_flAddedTime;
	}

	void	SetAddedTime( float addt )
	{
		m_flAddedTime = addt;
	}

	bool	IsFromPlayer() const
	{
		return m_bFromPlayer;
	}

private:
	wchar_t				m_szStream[ MAX_CAPTION_CHARACTERS ];

	float				m_flPreDisplayTime;
	float				m_flTimeToLive;
	float				m_flInitialLifeSpan;
	float				m_flAddedTime;
	bool				m_bValid;
	int					m_nTotalWidth;
	int					m_nTotalHeight;

	bool				m_bSizeComputed;
	bool				m_bFromPlayer;

	CUtlVector< CCloseCaptionWorkUnit * >	m_Work;

};

struct VisibleStreamItem
{
	int					height;
	int					width;
	CCloseCaptionItem	*item;
};

DECLARE_HUDELEMENT( CHudCloseCaption );

DECLARE_HUD_MESSAGE( CHudCloseCaption, CloseCaption );

CHudCloseCaption::CHudCloseCaption( const char *pElementName )
	: CHudElement( pElementName ), 
	vgui::Panel( NULL, "HudCloseCaption" ),
	m_CloseCaptionRepeats( 0, 0, CaptionTokenLessFunc ),
	m_TokenNameLookup( 0, 0, TokenNameLessFunc )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_nGoalHeight = 0;
	m_nCurrentHeight = 0;
	m_flGoalAlpha = 1.0f;
	m_flCurrentAlpha = 1.0f;

	m_flGoalHeightStartTime = 0;
	m_flGoalHeightFinishTime = 0;

	SetPaintBorderEnabled( false );
	SetPaintBackgroundEnabled( false );

	vgui::ivgui()->AddTickSignal( GetVPanel() );

	vgui::localize()->AddFile( vgui::filesystem(), "resource/closecaption_%language%.txt" );

	HOOK_HUD_MESSAGE( CHudCloseCaption, CloseCaption );

	BuildTokenNameLookup();


	char uilanguage[ 64 ];
	engine->GetUILanguage( uilanguage, sizeof( uilanguage ) );

	if ( !Q_stricmp( uilanguage, "english" ) )
	{
		english.SetValue( 1 );
	}
	else
	{
		english.SetValue( 0 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudCloseCaption::~CHudCloseCaption()
{
	m_CloseCaptionRepeats.RemoveAll();
	m_TokenNameLookup.RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *newmap - 
//-----------------------------------------------------------------------------
void CHudCloseCaption::LevelInit( void )
{
	CreateFonts();
	// Reset repeat counters per level
	m_CloseCaptionRepeats.RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCloseCaption::Paint( void )
{
	int w, h;
	GetSize( w, h );

	wrect_t rcOutput;
	rcOutput.left = 0;
	rcOutput.right = w;
	rcOutput.bottom = h;
	rcOutput.top = 0;

	wrect_t rcText = rcOutput;

	int avail_width = rcText.right - rcText.left - 2 * CC_INSET;
	int avail_height = rcText.bottom - rcText.top - 2 * CC_INSET;

	int totalheight = 0;
	int i;
	CUtlVector< VisibleStreamItem > visibleitems;
	int c = m_Items.Count();
	int maxwidth = 0;

	for  ( i = 0; i < c; i++ )
	{
		CCloseCaptionItem *item = m_Items[ i ];

		// Not ready for display yet.
		if ( item->GetPreDisplayTime() > 0.0f )
		{
			continue;
		}

		if ( !item->GetSizeComputed() )
		{
			ComputeStreamWork( avail_width, item );
		}

		int itemwidth = item->GetWidth();
		int itemheight = item->GetHeight();

		totalheight += itemheight;
		if ( itemwidth > maxwidth )
		{
			maxwidth = itemwidth;
		}

		VisibleStreamItem si;
		si.height = itemheight;
		si.width = itemwidth;
		si.item = item;

		visibleitems.AddToTail( si );

		// Start popping really old items off the stack if we run out of space
		while ( totalheight > avail_height &&
			visibleitems.Count() > 0 )
		{
			VisibleStreamItem & pop = visibleitems[ 0 ];
			totalheight -= pop.height;

			// And make it die right away...
			pop.item->SetTimeToLive( 0.0f );

			visibleitems.Remove( 0 );
		}	
	}

	float desiredAlpha = visibleitems.Count() >= 1 ? 1.0f : 0.0f;

	// Always return at least one line height for drawing the surrounding box
	totalheight = max( totalheight, m_nLineHeight ); 

	// Trigger box growing
	if ( totalheight != m_nGoalHeight )
	{
		m_nGoalHeight = totalheight;
		m_flGoalHeightStartTime = gpGlobals->curtime;
		m_flGoalHeightFinishTime = gpGlobals->curtime + m_flGrowTime;
	}
	if ( desiredAlpha != m_flGoalAlpha )
	{
		m_flGoalAlpha = desiredAlpha;
		m_flGoalHeightStartTime = gpGlobals->curtime;
		m_flGoalHeightFinishTime = gpGlobals->curtime + m_flGrowTime;
	}

	// If shrunk to zero and faded out, nothing left to do
	if ( !visibleitems.Count() &&
		m_nGoalHeight == m_nCurrentHeight &&
		m_flGoalAlpha == m_flCurrentAlpha )
	{
		m_flGoalHeightStartTime = 0;
		m_flGoalHeightFinishTime = 0;
		return;
	}

	bool growingDown = false;

	// Continue growth?
	if ( m_flGoalHeightFinishTime &&
		m_flGoalHeightStartTime &&
		m_flGoalHeightFinishTime > m_flGoalHeightStartTime )
	{
		float togo = m_nGoalHeight - m_nCurrentHeight;
		float alphatogo = m_flGoalAlpha - m_flCurrentAlpha;

		growingDown = togo < 0.0f ? true : false;

		float dt = m_flGoalHeightFinishTime - m_flGoalHeightStartTime;
		float frac = ( gpGlobals->curtime - m_flGoalHeightStartTime ) / dt;
		frac = clamp( frac, 0.0f, 1.0f );
		int newHeight = m_nCurrentHeight + (int)( frac * togo );
		m_nCurrentHeight = newHeight;
		float newAlpha = m_flCurrentAlpha + frac * alphatogo;
		m_flCurrentAlpha = clamp( newAlpha, 0.0f, 1.0f );
	}
	else
	{
		m_nCurrentHeight = m_nGoalHeight;
		m_flCurrentAlpha = m_flGoalAlpha;
	}

	rcText.top = rcText.bottom - m_nCurrentHeight - 2 * CC_INSET;

	Color bgColor = GetBgColor();
	bgColor[3] = m_flBackgroundAlpha;
	DrawBox( rcText.left, rcText.top, rcText.right - rcText.left, rcText.bottom - rcText.top, bgColor, m_flCurrentAlpha );

	if ( !visibleitems.Count() )
	{
		return;
	}

	rcText.left += CC_INSET;
	rcText.right -= CC_INSET;

	int textHeight = m_nCurrentHeight;
	if ( growingDown )
	{
		// If growing downward, keep the text locked to the bottom of the window instead of anchored to the top
		textHeight = totalheight;
	}

	rcText.top = rcText.bottom - textHeight - CC_INSET;

	// Now draw them
	c = visibleitems.Count();
	for ( i = 0; i < c; i++ )
	{
		VisibleStreamItem *si = &visibleitems[ i ];

		// If the oldest/top item was created with additional time, we can remove that now
		if ( i == 0 )
		{
			if ( si->item->GetAddedTime() > 0.0f )
			{
				float ttl = si->item->GetTimeToLive();
				ttl -= si->item->GetAddedTime();
				ttl = max( 0.0f, ttl );
				si->item->SetTimeToLive( ttl );
				si->item->SetAddedTime( 0.0f );
			}
		}

		int height = si->height;
		CCloseCaptionItem *item = si->item;
	
		rcText.bottom = rcText.top + height;

		wrect_t rcOut = rcText;

		rcOut.right = rcOut.left + si->width + 6;
		
		DrawStream( rcOut, item );

		rcText.top += height;
		rcText.bottom += height;

		if ( rcText.top >= rcOutput.bottom )
			break;
	}
}

bool CHudCloseCaption::LookupUnicodeText( char const *token, wchar_t *outbuf, size_t count )
{
	wchar_t *outstr = vgui::localize()->Find( token );
	if ( !outstr )
	{
		wcsncpy( outbuf, L"<can't find entry>", count );
		return false;
	}

	wcsncpy( outbuf, outstr, count );

	return true;
}

void CHudCloseCaption::OnTick( void )
{
	SetVisible( closecaption.GetBool() );

	float dt = gpGlobals->frametime;

	int c = m_Items.Count();
	int i;

	// Pass one decay all timers
	for ( i = 0 ; i < c ; ++i )
	{
		CCloseCaptionItem *item = m_Items[ i ];

		float predisplay = item->GetPreDisplayTime();
		if ( predisplay > 0.0f )
		{
			predisplay -= dt;
			predisplay = max( 0.0f, predisplay );
			item->SetPreDisplayTime( predisplay );
		}
		else
		{
			// remove time from actual playback
			float ttl = item->GetTimeToLive();
			ttl -= dt;
			ttl = max( 0.0f, ttl );
			item->SetTimeToLive( ttl );
		}
	}

	// Pass two, remove from head until we get to first item with time remaining
	bool foundfirstnondeletion = false;
	for ( i = 0 ; i < c ; ++i )
	{
		CCloseCaptionItem *item = m_Items[ i ];

		// Skip items not yet showing...
		float predisplay = item->GetPreDisplayTime();
		if ( predisplay > 0.0f )
		{
			continue;
		}

		float ttl = item->GetTimeToLive();
		if ( ttl > 0.0f )
		{
			foundfirstnondeletion = true;
			continue;
		}

		// Skip the remainder of the items after we find the first/oldest active item
		if ( foundfirstnondeletion )
		{
			continue;
		}

		delete item;
		m_Items.Remove( i );
		--i;
		--c;
	}
}

void CHudCloseCaption::Reset( void )
{
	while ( m_Items.Count() > 0 )
	{
		CCloseCaptionItem *i = m_Items[ 0 ];
		delete i;
		m_Items.Remove( 0 );
	}
}

bool CHudCloseCaption::SplitCommand( wchar_t const **ppIn, wchar_t *cmd, wchar_t *args ) const
{
	const wchar_t *in = *ppIn;
	const wchar_t *oldin = in;

	if ( in[0] != L'<' )
	{
		*ppIn += ( oldin - in );
		return false;
	}

	args[ 0 ] = 0;
	cmd[ 0 ]= 0;
	wchar_t *out = cmd;
	in++;
	while ( *in != L'\0' && *in != L':' && *in != L'>' && !isspace( *in ) )
	{
		*out++ = *in++;
	}
	*out = L'\0';

	if ( *in != L':' )
	{
		*ppIn += ( in - oldin );
		return true;
	}

	in++;
	out = args;
	while ( *in != L'\0' && *in != L'>' )
	{
		*out++ = *in++;
	}
	*out = L'\0';

	//if ( *in == L'>' )
	//	in++;

	*ppIn += ( in - oldin );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *stream - 
//			*findcmd - 
//			value - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHudCloseCaption::GetFloatCommandValue( const wchar_t *stream, const wchar_t *findcmd, float& value ) const
{
	const wchar_t *curpos = stream;
	
	for ( ; curpos && *curpos != L'\0'; ++curpos )
	{
		wchar_t cmd[ 256 ];
		wchar_t args[ 256 ];

		if ( SplitCommand( &curpos, cmd, args ) )
		{
			if ( !wcscmp( cmd, findcmd ) )
			{
				value = (float)wcstod( args, NULL );
				return true;
			}
			continue;
		}
	}

	return false;
}


bool CHudCloseCaption::StreamHasCommand( const wchar_t *stream, const wchar_t *findcmd ) const
{
	const wchar_t *curpos = stream;
	
	for ( ; curpos && *curpos != L'\0'; ++curpos )
	{
		wchar_t cmd[ 256 ];
		wchar_t args[ 256 ];

		if ( SplitCommand( &curpos, cmd, args ) )
		{
			if ( !wcscmp( cmd, findcmd ) )
			{
				return true;
			}
			continue;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: It's blank or only comprised of whitespace/space characters...
// Input  : *stream - 
// Output : static bool
//-----------------------------------------------------------------------------
static bool IsAllSpaces( const wchar_t *stream )
{
	const wchar_t *p = stream;
	while ( *p != L'\0' )
	{
		if ( !iswspace( *p ) )
			return false;

		p++;
	}

	return true;
}

bool CHudCloseCaption::StreamHasCommand( const wchar_t *stream, const wchar_t *search )
{
	for ( const wchar_t *curpos = stream; curpos && *curpos != L'\0'; ++curpos )
	{
		wchar_t cmd[ 256 ];
		wchar_t args[ 256 ];

		if ( SplitCommand( &curpos, cmd, args ) )
		{
			if ( !wcscmp( cmd, search ) )
			{
				return true;
			}
		}
	}
	return false;
}

void CHudCloseCaption::Process( const wchar_t *stream, float duration, char const *tokenstream, bool fromplayer )
{
	if ( !closecaption.GetBool() )
	{
		Reset();
		return;
	}

	// Nothing to do...
	if ( IsAllSpaces( stream) )
	{
		return;
	}

	// If subtitling, don't show sfx captions at all
	if ( cc_subtitles.GetBool() && StreamHasCommand( stream, L"sfx" ) )
	{
		return;
	}

	bool valid = true;
	if ( !wcsncmp( stream, L"!!!", wcslen( L"!!!" ) ) )
	{
		// It's in the text file, but hasn't been translated...
		valid = false;
	}

	if ( !wcsncmp( stream, L"-->", wcslen( L"-->" ) ) )
	{
		// It's in the text file, but hasn't been translated...
		valid = false;

		if ( cc_captiontrace.GetInt() < 2 )
		{
			if ( cc_captiontrace.GetInt() == 1 )
			{
				Msg( "Missing caption for '%s'\n", tokenstream );
			}

			return;
		}
	}

	float lifespan = duration + cc_linger_time.GetFloat();
	
	float addedlife = 0.0f;

	if ( m_Items.Count() > 0 )
	{
		// Get the remaining life span of the last item
		CCloseCaptionItem *final = m_Items[ m_Items.Count() - 1 ];
		float prevlife = final->GetTimeToLive();

		if ( prevlife > lifespan )
		{
			addedlife = prevlife - lifespan;
		}

		lifespan = max( lifespan, prevlife );
	}
	
	float delay = 0.0f;
	float override_duration = 0.0f;

	wchar_t phrase[ MAX_CAPTION_CHARACTERS ];
	wchar_t *out = phrase;

	for ( const wchar_t *curpos = stream; curpos && *curpos != L'\0'; ++curpos )
	{
		wchar_t cmd[ 256 ];
		wchar_t args[ 256 ];

		const wchar_t *prevpos = curpos;

		if ( SplitCommand( &curpos, cmd, args ) )
		{
			if ( !wcscmp( cmd, L"delay" ) )
			{

				// End current phrase
				*out = L'\0';

				if ( wcslen( phrase ) > 0 )
				{
					CCloseCaptionItem *item = new CCloseCaptionItem( phrase, lifespan, addedlife, delay, valid, fromplayer );
					m_Items.AddToTail( item );
					if ( StreamHasCommand( phrase, L"sfx" ) )
					{
						// SFX show up instantly.
						item->SetPreDisplayTime( 0.0f );
					}
					
					if ( GetFloatCommandValue( phrase, L"len", override_duration ) )
					{
						item->SetTimeToLive( override_duration );
					}
				}

				// Start new phrase
				out = phrase;

				// Delay must be positive
				delay = max( 0.0f, (float)wcstod( args, NULL ) );

				continue;
			}

			int copychars = curpos - prevpos;
			while ( --copychars >= 0 )
			{
				*out++ = *prevpos++;
			}
		}

		*out++ = *curpos;
	}

	// End final phrase, if any
	*out = L'\0';
	if ( wcslen( phrase ) > 0 )
	{
		CCloseCaptionItem *item = new CCloseCaptionItem( phrase, lifespan, addedlife, delay, valid, fromplayer );
		m_Items.AddToTail( item );

		if ( StreamHasCommand( phrase, L"sfx" ) )
		{
			// SFX show up instantly.
			item->SetPreDisplayTime( 0.0f );
		}

		if ( GetFloatCommandValue( phrase, L"len", override_duration ) )
		{
			item->SetTimeToLive( override_duration );
		}
	}
}

void CHudCloseCaption::CreateFonts( void )
{
	vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( GetScheme() );

	m_hFonts[ CCFONT_NORMAL ]		= pScheme->GetFont( "CloseCaption_Normal" );
	m_hFonts[ CCFONT_BOLD ]			= pScheme->GetFont( "CloseCaption_Bold" );
	m_hFonts[ CCFONT_ITALIC ]		= pScheme->GetFont( "CloseCaption_Italic" );
	m_hFonts[ CCFONT_ITALICBOLD ]	= pScheme->GetFont( "CloseCaption_BoldItalic" );

	m_nLineHeight = max( 6, vgui::surface()->GetFontTall( m_hFonts[ CCFONT_NORMAL ] ) );
}

struct WorkUnitParams
{
	WorkUnitParams()
	{
		Q_memset( stream, 0, sizeof( stream ) );
		out = stream;
		x = 0;
		y = 0;
		width = 0;
		bold = italic = false;
		clr = Color( 255, 255, 255, 255 );
		newline = false;
	}

	~WorkUnitParams()
	{
	}

	void Finalize( int lineheight )
	{
		*out = L'\0';
	}

	void Next( int lineheight )
	{
		// Restart output
		Q_memset( stream, 0, sizeof( stream ) );
		out = stream;

		x += width;

		width = 0;
		// Leave bold, italic and color alone!!!
		if ( newline )
		{
			newline = false;
			x = 0;
			y += lineheight;
		}
	}

	int GetFontNumber()
	{
		return CHudCloseCaption::GetFontNumber( bold, italic );
	}
		
	wchar_t	stream[ MAX_CAPTION_CHARACTERS ];
	wchar_t	*out;

	int		x;
	int		y;
	int		width;
	bool	bold;
	bool	italic;
	Color clr;
	bool	newline;
};

void CHudCloseCaption::AddWorkUnit( CCloseCaptionItem *item,
	WorkUnitParams& params )
{
	params.Finalize( m_nLineHeight );

	if ( wcslen( params.stream ) > 0 )
	{
		CCloseCaptionWorkUnit *wu = new CCloseCaptionWorkUnit();

		wu->SetStream( params.stream );
		wu->SetColor( params.clr );
		wu->SetBold( params.bold );
		wu->SetItalic( params.italic );
		wu->SetWidth( params.width );
		wu->SetHeight( m_nLineHeight );
		wu->SetPos( params.x, params.y );


		int curheight = item->GetHeight();
		int curwidth = item->GetWidth();

		curheight = max( curheight, params.y + wu->GetHeight() );
		curwidth = max( curwidth, params.x + params.width );

		item->SetHeight( curheight );
		item->SetWidth( curwidth );

		// Add it
		item->AddWork( wu );

		params.Next( m_nLineHeight );
	}
}

void CHudCloseCaption::ComputeStreamWork( int available_width, CCloseCaptionItem *item )
{
	// Start with a clean param block
	WorkUnitParams params;

	const wchar_t *curpos = item->GetStream();
	
	CUtlVector< Color > colorStack;

	const wchar_t *most_recent_space = NULL;
	int				most_recent_space_w = -1;

	for ( ; curpos && *curpos != L'\0'; ++curpos )
	{
		wchar_t cmd[ 256 ];
		wchar_t args[ 256 ];

		if ( SplitCommand( &curpos, cmd, args ) )
		{
			if ( !wcscmp( cmd, L"cr" ) )
			{
				params.newline = true;
				AddWorkUnit( item, params);
			}
			else if ( !wcscmp( cmd, L"clr" ) )
			{
				AddWorkUnit( item, params );

				if ( args[0] == 0 && colorStack.Count()>= 2)
				{
					colorStack.Remove( colorStack.Count() - 1 );
					params.clr = colorStack[ colorStack.Count() - 1 ];
				}
				else
				{
					int r, g, b;
					Color newcolor;
					if ( 3 == swscanf( args, L"%i,%i,%i", &r, &g, &b ) )
					{
						newcolor = Color( r, g, b, 255 );
						colorStack.AddToTail( newcolor );
						params.clr = colorStack[ colorStack.Count() - 1 ];
					}
				}
			}
			else if ( !wcscmp( cmd, L"playerclr" ) )
			{
				AddWorkUnit( item, params );

				if ( args[0] == 0 && colorStack.Count()>= 2)
				{
					colorStack.Remove( colorStack.Count() - 1 );
					params.clr = colorStack[ colorStack.Count() - 1 ];
				}
				else
				{
					// player and npc color selector
					// e.g.,. 255,255,255:200,200,200
					int pr, pg, pb, nr, ng, nb;
					Color newcolor;
					if ( 6 == swscanf( args, L"%i,%i,%i:%i,%i,%i", &pr, &pg, &pb, &nr, &ng, &nb ) )
					{
						newcolor = item->IsFromPlayer() ? Color( pr, pg, pb, 255 ) : Color( nr, ng, nb, 255 );
						colorStack.AddToTail( newcolor );
						params.clr = colorStack[ colorStack.Count() - 1 ];
					}
				}
			}
			else if ( !wcscmp( cmd, L"I" ) )
			{
				AddWorkUnit( item, params );
				params.italic = !params.italic;
			}
			else if ( !wcscmp( cmd, L"B" ) )
			{
				AddWorkUnit( item, params );
				params.bold = !params.bold;
			}

			continue;
		}

		vgui::HFont useF = m_hFonts[ params.GetFontNumber() ];
		
		int w, h;

		wchar_t sz[2];
		sz[ 0 ] = *curpos;
		sz[ 1 ] = L'\0';
		vgui::surface()->GetTextSize( useF, sz, w, h );

		if ( ( params.x + params.width ) + w > available_width )
		{
			if ( most_recent_space && curpos >= most_recent_space + 1 )
			{
				// Roll back to previous space character if there is one...
				int goback = curpos - most_recent_space - 1;
				params.out -= ( goback + 1 );
				params.width = most_recent_space_w;
				
				wchar_t *extra = new wchar_t[ goback + 1 ];
				wcsncpy( extra, most_recent_space + 1, goback );
				extra[ goback ] = L'\0';

				params.newline = true;
				AddWorkUnit( item, params );

				wcsncpy( params.out, extra, goback );
				params.out += goback;
				int textw, texth;
				vgui::surface()->GetTextSize( useF, extra, textw, texth );

				params.width = textw;

				delete[] extra;

				most_recent_space = NULL;
				most_recent_space_w = -1;
			}
			else
			{
				params.newline = true;
				AddWorkUnit( item, params );
			}
		}
		*params.out++ = *curpos;
		params.width += w;

		if ( iswspace( *curpos ) )
		{
			most_recent_space = curpos;
			most_recent_space_w = params.width;
		}
	}

	// Add the final unit.
	params.newline = true;
	AddWorkUnit( item, params );

	item->SetSizeComputed( true );

	// DumpWork( item );
}

void CHudCloseCaption::	DumpWork( CCloseCaptionItem *item )
{
	int c = item->GetNumWorkUnits();
	for ( int i = 0 ; i < c; ++i )
	{
		CCloseCaptionWorkUnit *wu = item->GetWorkUnit( i );
		wu->Dump();
	}
}

void CHudCloseCaption::DrawStream( wrect_t &rcText, CCloseCaptionItem *item )
{
	int c = item->GetNumWorkUnits();

	wrect_t rcOut;

	float alpha = item->GetAlpha( m_flItemHiddenTime, m_flItemFadeInTime, m_flItemFadeOutTime );

	for ( int i = 0 ; i < c; ++i )
	{
		int x = 0;
		int y = 0;

		CCloseCaptionWorkUnit *wu = item->GetWorkUnit( i );
	
		vgui::HFont useF = m_hFonts[ wu->GetFontNumber() ];

		wu->GetPos( x, y );

		rcOut.left = rcText.left + x + 3;
		rcOut.right = rcOut.left + wu->GetWidth();
		rcOut.top = rcText.top + y;
		rcOut.bottom = rcOut.top + wu->GetHeight();

		Color useColor = wu->GetColor();

		useColor[ 3 ] *= alpha;

		if ( !item->IsValid() )
		{
			useColor = Color( 255, 255, 255, 255 * alpha );
			rcOut.right += 2;
			vgui::surface()->DrawSetColor( Color( 100, 100, 40, 255 * alpha ) );
			vgui::surface()->DrawFilledRect( rcOut.left, rcOut.top, rcOut.right, rcOut.bottom );
		}

		vgui::surface()->DrawSetTextFont( useF );
		vgui::surface()->DrawSetTextPos( rcOut.left, rcOut.top );
		vgui::surface()->DrawSetTextColor( useColor );
		vgui::surface()->DrawPrintText( wu->GetStream(), wcslen( wu->GetStream() ) );
	}
}

bool CHudCloseCaption::GetNoRepeatValue( const wchar_t *caption, float &retval )
{
	retval = 0.0f;
	const wchar_t *curpos = caption;
	
	for ( ; curpos && *curpos != L'\0'; ++curpos )
	{
		wchar_t cmd[ 256 ];
		wchar_t args[ 256 ];

		if ( SplitCommand( &curpos, cmd, args ) )
		{
			if ( !wcscmp( cmd, L"norepeat" ) )
			{
				retval = (float)wcstod( args, NULL );
				return true;
			}
			continue;
		}
	}
	return false;
}

bool CHudCloseCaption::CaptionTokenLessFunc( const CaptionRepeat &lhs, const CaptionRepeat &rhs )
{ 
	return ( lhs.m_nTokenIndex < rhs.m_nTokenIndex );	
}

bool CHudCloseCaption::TokenNameLessFunc( const TokenNameLookup &lhs, const TokenNameLookup &rhs )
{
	return (unsigned int)lhs.crc < (unsigned int)rhs.crc;
}

static bool CaptionTrace( char const *token )
{
	static CUtlSymbolTable s_MissingCloseCaptions;

	// Make sure we only show the message once
	if ( UTL_INVAL_SYMBOL == s_MissingCloseCaptions.Find( token ) )
	{
		s_MissingCloseCaptions.AddString( token );
		return true;
	}

	return false;
}

static ConVar cc_sentencecaptionnorepeat( "cc_sentencecaptionnorepeat", "4", 0, "How often a sentence can repeat." );

void CHudCloseCaption::ProcessSentenceCaptionStream( char const *tokenstream )
{
	float interval = cc_sentencecaptionnorepeat.GetFloat();
	interval = clamp( interval, 0.1f, 60.0f );

	// The first token from the stream is the name of the sentence
	char tokenname[ 512 ];

	tokenname[ 0 ] = 0;

	char const *p = tokenstream;

	p = nexttoken( tokenname, p, ' ' );

	if ( Q_strlen( tokenname ) > 0 )
	{
		//  Use it to check for "norepeat" rules
		CaptionRepeat entry;
		entry.m_nTokenIndex = vgui::localize()->FindIndex( tokenname );

		int idx = m_CloseCaptionRepeats.Find( entry );
		if ( m_CloseCaptionRepeats.InvalidIndex() == idx )
		{
			entry.m_flLastEmitTime = gpGlobals->curtime;
			entry.m_nLastEmitTick = gpGlobals->tickcount;
			entry.m_flInterval = interval;
			m_CloseCaptionRepeats.Insert( entry );
		}
		else
		{
			CaptionRepeat &entry = m_CloseCaptionRepeats[ idx ];
			if ( gpGlobals->curtime < ( entry.m_flLastEmitTime + entry.m_flInterval ) )
			{
				return;
			}

			entry.m_flLastEmitTime = gpGlobals->curtime;
			entry.m_nLastEmitTick = gpGlobals->tickcount;
		}
	}

	wchar_t caption_full[ 2048 ];
	caption_full[ 0 ] = L'\0';

	unsigned int curlen = 0;

	int wordCount = 0;
	// p points to reset of sentence tokens, build up a unicode string from them...
	while ( p && Q_strlen( tokenname ) > 0 )
	{
		p = nexttoken( tokenname, p, ' ' );

		if ( Q_strlen( tokenname ) == 0 )
			break;

		wchar_t caption[ MAX_CAPTION_CHARACTERS ];
		if ( !LookupUnicodeText( tokenname, caption, sizeof( caption ) / sizeof( wchar_t ) ) )
		{
			vgui::localize()->ConvertANSIToUnicode( VarArgs( "!!!%s", tokenname ), caption, sizeof( caption ) );
		}
		
		// Append to stream
		unsigned int len = wcslen( caption );

		if ( ( len + curlen + 2 ) >= ( sizeof( caption_full ) / sizeof( wchar_t ) ) )
		{
			Assert( !"Caption would exceed 2048 wide characters!" );
			break;
		}

		wcscat( caption_full, caption );
		wcscat( caption_full, L" " );

		curlen += len + 1;

		++wordCount;
	}

	// Remove extra space at end
	if ( wordCount >= 1 )
	{
		caption_full[ wcslen( caption_full ) - 1 ] = L'\0';
	}


	/*
	char foo[ 2048 ];
	vgui::localize()->ConvertUnicodeToANSI( caption_full, foo, sizeof( foo ) );
	Msg( "'%s'\n", foo );
	*/

	// ( words + 1 ) * 3/4 second for estimated cc duration
	if ( curlen >= 1 )
	{
		Process( caption_full, ( wordCount + 1 ) * 0.75f, tokenstream, false /*never from player!*/ );
	}
}

void CHudCloseCaption::ProcessCaption( char const *tokenname, float duration, bool fromplayer /* = false */ )
{
	wchar_t caption[ MAX_CAPTION_CHARACTERS ];
	if ( !LookupUnicodeText( tokenname, caption, sizeof( caption ) / sizeof( wchar_t ) ) )
	{
		if ( !CaptionTrace( tokenname ) )
			return;

		vgui::localize()->ConvertANSIToUnicode( VarArgs( "--> Missing Caption[%s]", tokenname ), caption, sizeof( caption ) );
	}
	else
	{
		// Get the string for the token
		float interval = 0.0f;
		bool hasnorepeat = GetNoRepeatValue( caption, interval );

		CaptionRepeat entry;
		entry.m_nTokenIndex = vgui::localize()->FindIndex( tokenname );

		int idx = m_CloseCaptionRepeats.Find( entry );
		if ( m_CloseCaptionRepeats.InvalidIndex() == idx )
		{
			entry.m_flLastEmitTime = gpGlobals->curtime;
			entry.m_nLastEmitTick = gpGlobals->tickcount;
			entry.m_flInterval = interval;
			m_CloseCaptionRepeats.Insert( entry );
		}
		else
		{
			CaptionRepeat &entry = m_CloseCaptionRepeats[ idx ];

			// Interval of 0.0 means just don't double emit on same tick #
			if ( entry.m_flInterval <= 0.0f )
			{
				if ( gpGlobals->tickcount <= entry.m_nLastEmitTick )
				{
					return;
				}
			}
			else if ( hasnorepeat )
			{
				if ( gpGlobals->curtime < ( entry.m_flLastEmitTime + entry.m_flInterval ) )
				{
					return;
				}
			}

			entry.m_flLastEmitTime = gpGlobals->curtime;
			entry.m_nLastEmitTick = gpGlobals->tickcount;
		}
	}

	Process( caption, duration, tokenname, fromplayer );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pszName - 
//			iSize - 
//			*pbuf - 
//-----------------------------------------------------------------------------
void CHudCloseCaption::MsgFunc_CloseCaption(bf_read &msg)
{
	CRC32_t crc = (CRC32_t)msg.ReadLong();
	float duration = msg.ReadByte() * 0.1f;
	byte flagbyte = msg.ReadByte();
	bool warnonmissing = flagbyte & (1<<0)? true : false;
	bool fromplayer = flagbyte & (1<<1) ? true : false;

	TokenNameLookup lookup;
	lookup.crc = crc;

	int idx = m_TokenNameLookup.Find( lookup );
	if ( idx == m_TokenNameLookup.InvalidIndex() )
	{
		if ( warnonmissing )
		{
			// Send message to server to spew warning
			engine->ClientCmd( VarArgs( "cc_lookup_crc %u\n", (unsigned int)crc ) );
		}
		return;
	}

	char const *tokenname = vgui::localize()->GetNameByIndex( m_TokenNameLookup[ idx ].stringIndex );
	if ( !tokenname )
	{
		Warning( "MsgFunc_CloseCaption to find token name for index %i\n", m_TokenNameLookup[ idx ].stringIndex );
		return;
	}

	ProcessCaption( tokenname, duration, fromplayer );	
}

int CHudCloseCaption::GetFontNumber( bool bold, bool italic )
{
	if ( bold || italic )
	{
		if( bold && italic )
		{
			return CHudCloseCaption::CCFONT_ITALICBOLD;
		}

		if ( bold )
		{
			return CHudCloseCaption::CCFONT_BOLD;
		}

		if ( italic )
		{
			return CHudCloseCaption::CCFONT_ITALIC;
		}
	}

	return CHudCloseCaption::CCFONT_NORMAL;
}

void CHudCloseCaption::BuildTokenNameLookup()
{
	vgui::StringIndex_t i = vgui::localize()->GetFirstStringIndex();
	while ( vgui::INVALID_STRING_INDEX != i )
	{
		TokenNameLookup lookup;

		CRC32_t crc;
		CRC32_Init( &crc );
		char const *tokenName = vgui::localize()->GetNameByIndex( i );

		char lowercase[ 256 ];
		Q_strncpy( lowercase, tokenName, sizeof( lowercase ) );
		Q_strlower( lowercase );

		CRC32_ProcessBuffer( &crc, lowercase, Q_strlen( lowercase ) );
		CRC32_Final( &crc );

		lookup.crc = crc;
		lookup.stringIndex = i;

		if ( m_TokenNameLookup.Find( lookup ) == m_TokenNameLookup.InvalidIndex() )
		{
			m_TokenNameLookup.Insert( lookup );
		}

		i = vgui::localize()->GetNextStringIndex( i );
	}
}

static int EmitCaptionCompletion( const char *partial, char commands[ COMMAND_COMPLETION_MAXITEMS ][ COMMAND_COMPLETION_ITEM_LENGTH ] )
{
	int current = 0;
	if ( !vgui::localize() )
		return current;

	const char *cmdname = "cc_emit";
	char *substring = NULL;
	int substringLen = 0;
	if ( Q_strstr( partial, cmdname ) && strlen(partial) > strlen(cmdname) + 1 )
	{
		substring = (char *)partial + strlen( cmdname ) + 1;
		substringLen = strlen(substring);
	}
	
	vgui::StringIndex_t i = vgui::localize()->GetFirstStringIndex();

	while ( i != vgui::INVALID_STRING_INDEX &&
		 current < COMMAND_COMPLETION_MAXITEMS )
	{
		char const *ccname = vgui::localize()->GetNameByIndex( i );
		if ( ccname )
		{
			if ( !substring || !Q_strncasecmp( ccname, substring, substringLen ) )
			{
				Q_snprintf( commands[ current ], sizeof( commands[ current ] ), "%s %s", cmdname, ccname );
				current++;
			}
		}
		i = vgui::localize()->GetNextStringIndex( i );
	}

	return current;
}

void EmitCaption_f()
{
	if ( engine->Cmd_Argc() != 2 )
	{
		Msg( "usage:  cc_emit tokenname\n" );
		return;
	}

	CHudCloseCaption *hudCloseCaption = GET_HUDELEMENT( CHudCloseCaption );
	if ( hudCloseCaption )
	{
		hudCloseCaption->ProcessCaption( engine->Cmd_Argv(1), 5.0f );	
	}
}

static ConCommand cc_emit( "cc_emit", EmitCaption_f, "Emits a closed caption", 0, EmitCaptionCompletion );

void OnCaptionLanguageChanged( ConVar *var, char const *pOldString )
{
	if ( !vgui::localize() )
		return;

	char fn[ 512 ];
	Q_snprintf( fn, sizeof( fn ), "resource/closecaption_%s.txt", var->GetString() );

	// Re-adding the file, even if it's "english" will overwrite the tokens as needed
	vgui::localize()->AddFile( vgui::filesystem(), "resource/closecaption_%language%.txt" );

	char uilanguage[ 64 ];
	engine->GetUILanguage( uilanguage, sizeof( uilanguage ) );

	// If it's not the default, load the language on top of the user's default language
	if ( Q_strlen( var->GetString() ) > 0 && Q_stricmp( var->GetString(), uilanguage ) )
	{
		if ( vgui::filesystem()->FileExists( fn ) )
		{
			vgui::localize()->AddFile( vgui::filesystem(), fn );
		}
		else
		{
			char fallback[ 512 ];
			Q_snprintf( fallback, sizeof( fallback ), "resource/closecaption_%s.txt", uilanguage );

			Msg( "%s not found\n", fn );
			Msg( "%s will be used\n", fallback );
		}
	}

	DevMsg( "cc_lang = %s\n", var->GetString() );
}

ConVar cc_lang( "cc_lang", "", FCVAR_ARCHIVE, "Current close caption language (emtpy = use game UI language)", OnCaptionLanguageChanged );