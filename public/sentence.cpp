//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include <assert.h>
#include "commonmacros.h"
#include "basetypes.h"
#include "sentence.h"
#include "utlbuffer.h"
#include <stdlib.h>
#include "vector.h"
#include "mathlib.h"
#include <ctype.h>
#include "checksum_crc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: converts an english string to unicode
//-----------------------------------------------------------------------------
int ConvertANSIToUnicode(const char *ansi, wchar_t *unicode, int unicodeBufferSize);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWordTag::CWordTag( void )
{
	m_pszWord = NULL;

	m_uiStartByte = 0;
	m_uiEndByte = 0;

	m_flStartTime = 0.0f;
	m_flEndTime = 0.0f;

	m_bSelected = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : from - 
//-----------------------------------------------------------------------------
CWordTag::CWordTag( const CWordTag& from )
{
	m_pszWord = NULL;
	SetWord( from.m_pszWord );

	m_uiStartByte = from.m_uiStartByte;
	m_uiEndByte = from.m_uiEndByte;

	m_flStartTime = from.m_flStartTime;
	m_flEndTime = from.m_flEndTime;

	m_bSelected = from.m_bSelected;

	for ( int p = 0; p < from.m_Phonemes.Size(); p++ )
	{
		CPhonemeTag *newPhoneme = new CPhonemeTag( *from.m_Phonemes[ p ] );
		m_Phonemes.AddToTail( newPhoneme );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *word - 
//-----------------------------------------------------------------------------
CWordTag::CWordTag( const char *word )
{
	m_uiStartByte = 0;
	m_uiEndByte = 0;

	m_flStartTime = 0.0f;
	m_flEndTime = 0.0f;

	m_pszWord = NULL;

	m_bSelected = false;

	SetWord( word );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWordTag::~CWordTag( void )
{
	delete[] m_pszWord;

	while ( m_Phonemes.Size() > 0 )
	{
		delete m_Phonemes[ 0 ];
		m_Phonemes.Remove( 0 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *tag - 
// Output : int
//-----------------------------------------------------------------------------
int CWordTag::IndexOfPhoneme( CPhonemeTag *tag )
{
	for ( int i = 0 ; i < m_Phonemes.Size(); i++ )
	{
		CPhonemeTag *p = m_Phonemes[ i ];
		if ( p == tag )
			return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *word - 
//-----------------------------------------------------------------------------
void CWordTag::SetWord( const char *word )
{
	delete[] m_pszWord;
	m_pszWord = NULL;
	if ( !word || !word[ 0 ] )
		return;

	int len = strlen( word ) + 1;
	m_pszWord = new char[ len ];
	Assert( m_pszWord );
	Q_strncpy( m_pszWord, word, len );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : const char
//-----------------------------------------------------------------------------
const char *CWordTag::GetWord() const
{
	return m_pszWord ? m_pszWord : "";
}


unsigned int CWordTag::ComputeDataCheckSum()
{
	int i;
	int c;
	CRC32_t crc;
	CRC32_Init( &crc );

	// Checksum the text
	if ( m_pszWord != NULL )
	{
		CRC32_ProcessBuffer( &crc, m_pszWord, Q_strlen( m_pszWord ) );
	}
	// Checksum phonemes
	c = m_Phonemes.Count();
	for ( i = 0; i < c; ++i )
	{
		CPhonemeTag *phoneme = m_Phonemes[ i ];
		unsigned int phonemeCheckSum = phoneme->ComputeDataCheckSum();
		CRC32_ProcessBuffer( &crc, &phonemeCheckSum, sizeof( unsigned int ) );
	}
	// Checksum timestamps
	CRC32_ProcessBuffer( &crc, &m_flStartTime, sizeof( float ) );
	CRC32_ProcessBuffer( &crc, &m_flEndTime, sizeof( float ) );

	CRC32_Final( &crc );

	return ( unsigned int )crc;
}

CBasePhonemeTag::CBasePhonemeTag()
{
	m_flStartTime = 0.0f;
	m_flEndTime = 0.0f;

	m_nPhonemeCode = 0;
}

CBasePhonemeTag::CBasePhonemeTag( const CBasePhonemeTag& from )
{
	m_flStartTime = from.m_flStartTime;
	m_flEndTime = from.m_flEndTime;

	m_nPhonemeCode = from.m_nPhonemeCode;

}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPhonemeTag::CPhonemeTag( void )
{
	m_szPhoneme = NULL;

	m_uiStartByte = 0;
	m_uiEndByte = 0;

	m_bSelected = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : from - 
//-----------------------------------------------------------------------------
CPhonemeTag::CPhonemeTag( const CPhonemeTag& from ) :
	BaseClass( from )
{
	m_uiStartByte = from.m_uiStartByte;
	m_uiEndByte = from.m_uiEndByte;

	m_bSelected = from.m_bSelected;

	m_szPhoneme = NULL;
	SetTag( from.GetTag() );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *phoneme - 
//-----------------------------------------------------------------------------
CPhonemeTag::CPhonemeTag( const char *phoneme )
{
	m_uiStartByte = 0;
	m_uiEndByte = 0;

	m_flStartTime = 0.0f;
	m_flEndTime = 0.0f;

	m_bSelected = false;

	m_nPhonemeCode = 0;

	m_szPhoneme = NULL;
	SetTag( phoneme );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPhonemeTag::~CPhonemeTag( void )
{
	delete[] m_szPhoneme;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *phoneme - 
//-----------------------------------------------------------------------------
void CPhonemeTag::SetTag( const char *phoneme )
{
	delete m_szPhoneme;
	m_szPhoneme = NULL;
	if ( !phoneme || !phoneme [ 0 ] )
		return;

	int len = Q_strlen( phoneme ) + 1;
	m_szPhoneme = new char[ len ];
	Assert( m_szPhoneme );
	Q_strncpy( m_szPhoneme, phoneme, len );
}

char const *CPhonemeTag::GetTag() const
{
	return m_szPhoneme ? m_szPhoneme : "";
}


unsigned int CPhonemeTag::ComputeDataCheckSum()
{
	CRC32_t crc;
	CRC32_Init( &crc );

	// Checksum the text
	CRC32_ProcessBuffer( &crc, m_szPhoneme, Q_strlen( m_szPhoneme ) );
	CRC32_ProcessBuffer( &crc, &m_nPhonemeCode, sizeof( int ) );

	// Checksum timestamps
	CRC32_ProcessBuffer( &crc, &m_flStartTime, sizeof( float ) );
	CRC32_ProcessBuffer( &crc, &m_flEndTime, sizeof( float ) );

	CRC32_Final( &crc );

	return ( unsigned int )crc;
}

//-----------------------------------------------------------------------------
// Purpose: Simple language to string and string to language lookup dictionary
//-----------------------------------------------------------------------------
struct CCLanguage
{
	int				type;
	char const		*name;
	unsigned char	r, g, b;  // For faceposer, indicator color for this language
};

static CCLanguage g_CCLanguageLookup[] =
{
	{ CC_ENGLISH,	"english",		0,		0,		0 },
	{ CC_FRENCH,	"french",		150,	0,		0 },
	{ CC_GERMAN,	"german",		0,		150,	0 },
	{ CC_ITALIAN,	"italian",		0,		150,	150 },
	{ CC_KOREAN,	"korean",		150,	0,		150 },
	{ CC_SCHINESE,	"schinese",		150,	0,		150 },
	{ CC_SPANISH,	"spanish",		0,		0,		150 },
	{ CC_TCHINESE,	"tchinese",		150,	0,		150 },
	{ CC_JAPANESE,	"japanese",		250,	150,	0 },
	{ CC_RUSSIAN,	"russian",		0,		250,	150 },
	{ CC_THAI,		"thai",			0 ,		150,	250 },
	{ CC_PORTUGUESE,"portuguese",	0 ,		0,		150 },	
};

void CSentence::ColorForLanguage( int language, unsigned char& r, unsigned char& g, unsigned char& b )
{
	r = g = b = 0;

	if ( language < 0 || language >= CC_NUM_LANGUAGES )
	{
		return;
	}

	r = g_CCLanguageLookup[ language ].r;
	g = g_CCLanguageLookup[ language ].g;
	b = g_CCLanguageLookup[ language ].b;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : language - 
// Output : char const
//-----------------------------------------------------------------------------
char const *CSentence::NameForLanguage( int language )
{
	if ( language < 0 || language >= CC_NUM_LANGUAGES )
		return "unknown_language";

	CCLanguage *entry = &g_CCLanguageLookup[ language ];
	Assert( entry->type == language );
	return entry->name;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *name - 
// Output : int
//-----------------------------------------------------------------------------
int CSentence::LanguageForName( char const *name )
{
	int l;
	for ( l = 0; l < CC_NUM_LANGUAGES; l++ )
	{
		CCLanguage *entry = &g_CCLanguageLookup[ l ];
		Assert( entry->type == l );
		if ( !stricmp( entry->name, name ) )
			return l;
	}
	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CSentence::CSentence( void )
{
	m_nResetWordBase = 0;
	m_szText = 0;
	m_bShouldVoiceDuck = false;
	m_bStoreCheckSum = false;
	m_bIsValid = false;
	m_uCheckSum = 0;
	m_bIsCached = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CSentence::~CSentence( void )
{
	Reset();
	delete[] m_szText;
}


void CSentence::ParsePlaintext( CUtlBuffer& buf )
{
	char token[ 4096 ];
	char text[ 4096 ];
	text[ 0 ] = 0;
	while ( 1 )
	{
		buf.GetString( token );
		if ( !stricmp( token, "}" ) )
			break;

		Q_strncat( text, token, sizeof( text ), COPY_ALL_CHARACTERS );
		Q_strncat( text, " ", sizeof( text ), COPY_ALL_CHARACTERS );
	}

	SetText( text );
}

void CSentence::ParseWords( CUtlBuffer& buf )
{
	char token[ 4096 ];
	char word[ 256 ];
	float start, end;

	while ( 1 )
	{
		buf.GetString( token );
		if ( !stricmp( token, "}" ) )
			break;

		if ( stricmp( token, "WORD" ) )
			break;

		buf.GetString( token );
		Q_strncpy( word, token, sizeof( word ) );

		buf.GetString( token );
		start = atof( token );
		buf.GetString( token );
		end = atof( token );

		CWordTag *wt = new CWordTag( word );
		assert( wt );
		wt->m_flStartTime = start;
		wt->m_flEndTime = end;

		AddWordTag( wt );

		buf.GetString( token );
		if ( stricmp( token, "{" ) )
			break;

		while ( 1 )
		{
			buf.GetString( token );
			if ( !stricmp( token, "}" ) )
				break;

			// Parse phoneme
			int code;
			char phonemename[ 256 ];
			float start, end;
			float volume;

			code = atoi( token );

			buf.GetString( token );
			Q_strncpy( phonemename, token, sizeof( phonemename ) );
			buf.GetString( token );
			start = atof( token );
			buf.GetString( token );
			end = atof( token );
			buf.GetString( token );
			volume = atof( token );

			CPhonemeTag *pt = new CPhonemeTag();
			assert( pt );
			pt->m_nPhonemeCode = code;
			pt->SetTag( phonemename );
			pt->m_flStartTime = start;
			pt->m_flEndTime = end;

			AddPhonemeTag( wt, pt );
		}
	}
}

void CSentence::ParseEmphasis( CUtlBuffer& buf )
{
	char token[ 4096 ];
	while ( 1 )
	{
		buf.GetString( token );
		if ( !stricmp( token, "}" ) )
			break;

		char t[ 256 ];
		Q_strncpy( t, token, sizeof( t ) );
		buf.GetString( token );

		char value[ 256 ];
		Q_strncpy( value, token, sizeof( value ) );

		CEmphasisSample sample;
		sample.selected = false;
		sample.time = atof( t );
		sample.value = atof( value );


		m_EmphasisSamples.AddToTail( sample );

	}
}

// This is obsolete, so it doesn't do anything with the data which is parsed.
void CSentence::ParseCloseCaption( CUtlBuffer& buf )
{
	char token[ 4096 ];
	while ( 1 )
	{
		// Format is 
		// language_name
		// {
		//   PHRASE char streamlength "streambytes" starttime endtime
		//   PHRASE unicode streamlength "streambytes" starttime endtime
		// }
		buf.GetString( token );
		if ( !stricmp( token, "}" ) )
			break;

		buf.GetString( token );
		if ( stricmp( token, "{" ) )
			break;

		buf.GetString( token );
		while ( 1 )
		{

			if ( !stricmp( token, "}" ) )
				break;

			if ( stricmp( token, "PHRASE" ) )
				break;

			char cc_type[32];
			char cc_stream[ 4096 ];
			int cc_length;

			memset( cc_stream, 0, sizeof( cc_stream ) );

			buf.GetString( token );
			Q_strncpy( cc_type, token, sizeof( cc_type ) );

			bool unicode = false;
			if ( !stricmp( cc_type, "unicode" ) )
			{
				unicode = true;
			}
			else if ( stricmp( cc_type, "char" ) )
			{
				Assert( 0 );
			}

			buf.GetString( token );
			cc_length = atoi( token );
			Assert( cc_length >= 0 && cc_length < sizeof( cc_stream ) );
			// Skip space
			buf.GetChar();
			buf.Get( cc_stream, cc_length );
			cc_stream[ cc_length ] = 0;
			
			// Skip space
			buf.GetChar();
			buf.GetString( token );
			buf.GetString( token );


			buf.GetString( token );
		}

	}
}

void CSentence::ParseOptions( CUtlBuffer& buf )
{
	char token[ 4096 ];
	while ( 1 )
	{
		buf.GetString( token );
		if ( !stricmp( token, "}" ) )
			break;

		if ( Q_strlen( token ) == 0 )
			break;

		char key[ 256 ];
		Q_strncpy( key, token, sizeof( key ) );
		char value[ 256 ];
		buf.GetString( token );
		Q_strncpy( value, token, sizeof( value ) );

		if ( !strcmpi( key, "voice_duck" ) )
		{
			SetVoiceDuck( atoi(value) ? true : false );
		}
		else if ( !strcmpi( key, "checksum" ) )
		{
			SetDataCheckSum( (unsigned int)atoi( value ) );
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: VERSION 1.0 parser, need to implement new ones if 
//  file format changes!!!
// Input  : buf - 
//-----------------------------------------------------------------------------
void CSentence::ParseDataVersionOnePointZero( CUtlBuffer& buf )
{
	char token[ 4096 ];

	while ( 1 )
	{
		buf.GetString( token );
		if ( strlen( token ) <= 0 )
			break;

		char section[ 256 ];
		Q_strncpy( section, token, sizeof( section ) );

		buf.GetString( token );
		if ( stricmp( token, "{" ) )
			break;

		if ( !stricmp( section, "PLAINTEXT" ) )
		{
			ParsePlaintext( buf );
		}
		else if ( !stricmp( section, "WORDS" ) )
		{
			ParseWords( buf );
		}
		else if ( !stricmp( section, "EMPHASIS" ) )
		{
			ParseEmphasis( buf );
		}		
		else if ( !stricmp( section, "CLOSECAPTION" ) )
		{
			// NOTE:  CLOSECAPTION IS NO LONGER VALID
			// This just skips the section of data.
			ParseCloseCaption( buf );
		}
		else if ( !stricmp( section, "OPTIONS" ) )
		{
			ParseOptions( buf );
		}
	}
}

#define CACHED_SENTENCE_VERSION	1

// This is a compressed save of just the data needed to drive phonemes in the engine (no word / sentence text, etc )
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : buf - 
//-----------------------------------------------------------------------------
void CSentence::CacheSaveToBuffer( CUtlBuffer& buf )
{
	Assert( !buf.IsText() );
	Assert( m_bIsCached );

	buf.PutChar( CACHED_SENTENCE_VERSION );

	int i;

	unsigned short pcount = GetRuntimePhonemeCount();

	buf.PutShort( pcount );

	for ( i = 0; i < pcount; ++i )
	{
		const CBasePhonemeTag *phoneme = GetRuntimePhoneme( i );
		Assert( phoneme );

		buf.PutShort( phoneme->m_nPhonemeCode );
		buf.PutFloat( phoneme->m_flStartTime );
		buf.PutFloat( phoneme->m_flEndTime );
	}

	// Now save out emphasis samples
	int c = m_EmphasisSamples.Count();
	Assert( c <= 32767 );
	buf.PutShort( c );

	for ( i = 0; i < c; i++ )
	{
		CEmphasisSample *sample = &m_EmphasisSamples[ i ];
		Assert( sample );

		buf.PutFloat( sample->time );
		short scaledValue = clamp( (short)( sample->value * 32767 ), 0, 32767 );

		buf.PutShort( scaledValue );
	}

	// And voice duck
	buf.PutChar( GetVoiceDuck() ? 1 : 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : buf - 
//-----------------------------------------------------------------------------
void CSentence::CacheRestoreFromBuffer( CUtlBuffer& buf )
{
	Assert( !buf.IsText() );

	Reset();

	m_bIsCached = true;

	int version = buf.GetChar();
	if ( version != CACHED_SENTENCE_VERSION )
	{
		// Uh oh, version changed...
		m_bIsValid = false;
		return;
	}

	unsigned short pcount = (unsigned short)buf.GetShort();
	
	CPhonemeTag pt;

	for ( int i = 0; i < pcount; ++i )
	{
		unsigned short code = buf.GetShort();
		float st = buf.GetFloat();
		float et = buf.GetFloat();

		pt.m_nPhonemeCode = code;
		pt.m_flStartTime = st;
		pt.m_flEndTime = et;

		AddRuntimePhoneme( &pt );
	}

	// Now read emphasis samples
	int c = buf.GetShort();

	for ( i = 0; i < c; i++ )
	{
		CEmphasisSample sample;
		sample.selected = false;

		sample.time = buf.GetFloat();
		sample.value = (float)buf.GetShort() / 32767.0f;

		m_EmphasisSamples.AddToTail( sample );
	}

	// And voice duck
	SetVoiceDuck( buf.GetChar() == 0 ? false : true );
	m_bIsValid = true;
}

int CSentence::GetRuntimePhonemeCount() const
{
	return m_RunTimePhonemes.Count();
}

const CBasePhonemeTag *CSentence::GetRuntimePhoneme( int i ) const
{
	Assert( m_bIsCached );
	return m_RunTimePhonemes[ i ];
}

void CSentence::ClearRuntimePhonemes()
{
	while ( m_RunTimePhonemes.Count() > 0 )
	{
		CBasePhonemeTag *tag = m_RunTimePhonemes[ 0 ];
		delete tag;
		m_RunTimePhonemes.Remove( 0 );
	}
}

void CSentence::AddRuntimePhoneme( const CPhonemeTag *src )
{
	Assert( m_bIsCached );

	CBasePhonemeTag *tag = new CBasePhonemeTag();
	tag->m_nPhonemeCode = src->m_nPhonemeCode;
	tag->m_flStartTime = src->m_flStartTime;
	tag->m_flEndTime = src->m_flEndTime;

	m_RunTimePhonemes.AddToTail( tag );
}

void CSentence::MakeRuntimeOnly()
{
	m_bIsCached = true;
	delete m_szText;
	m_szText = NULL;

	int c = m_Words.Count();
	for ( int i = 0; i < c; ++i )
	{
		CWordTag *word = m_Words[ i ];
		Assert( word );
		int pcount = word->m_Phonemes.Count();
		for ( int j = 0; j < pcount; ++j )
		{
			CPhonemeTag *phoneme = word->m_Phonemes[ j ];
			assert( phoneme );

			AddRuntimePhoneme( phoneme );
		}
	}

	// Remove all existing words
	while ( m_Words.Count() > 0 )
	{
		CWordTag *word = m_Words[ 0 ];
		delete word;
		m_Words.Remove( 0 );
	}

	m_bIsValid = true;
}


void CSentence::SaveToBuffer( CUtlBuffer& buf )
{
	Assert( !m_bIsCached );

	int i, j;

	buf.Printf( "VERSION 1.0\n" );
	buf.Printf( "PLAINTEXT\n" );
	buf.Printf( "{\n" );
	buf.Printf( "%s\n", GetText() );
	buf.Printf( "}\n" );
	buf.Printf( "WORDS\n" );
	buf.Printf( "{\n" );
	for ( i = 0; i < m_Words.Size(); i++ )
	{
		CWordTag *word = m_Words[ i ];
		Assert( word );

		buf.Printf( "WORD %s %.3f %.3f\n", 
			word->GetWord(),
			word->m_flStartTime,
			word->m_flEndTime );

		buf.Printf( "{\n" );
		for ( j = 0; j < word->m_Phonemes.Size(); j++ )
		{
			CPhonemeTag *phoneme = word->m_Phonemes[ j ];
			Assert( phoneme );

			buf.Printf( "%i %s %.3f %.3f 1\n", 
				phoneme->m_nPhonemeCode, 
				phoneme->GetTag(),
				phoneme->m_flStartTime,
				phoneme->m_flEndTime );
		}

		buf.Printf( "}\n" );
	}
	buf.Printf( "}\n" );
	buf.Printf( "EMPHASIS\n" );
	buf.Printf( "{\n" );
	int c = m_EmphasisSamples.Count();
	for ( i = 0; i < c; i++ )
	{
		CEmphasisSample *sample = &m_EmphasisSamples[ i ];
		Assert( sample );

		buf.Printf( "%f %f\n", sample->time, sample->value );
	}

	buf.Printf( "}\n" );
	buf.Printf( "OPTIONS\n" );
	buf.Printf( "{\n" );
	buf.Printf( "voice_duck %d\n", GetVoiceDuck() ? 1 : 0 );
	if ( m_bStoreCheckSum )
	{
		buf.Printf( "checksum %d\n", m_uCheckSum );
	}
	buf.Printf( "}\n" );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *data - 
//			size - 
//-----------------------------------------------------------------------------
void CSentence::InitFromDataChunk( void *data, int size )
{
	CUtlBuffer buf( 0, 0, true );
	buf.EnsureCapacity( size );
	buf.Put( data, size );
	buf.SeekPut( CUtlBuffer::SEEK_HEAD, size );

	InitFromBuffer( buf );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : buf - 
//-----------------------------------------------------------------------------
void CSentence::InitFromBuffer( CUtlBuffer& buf )
{
	Assert( buf.IsText() );

	Reset();

	char token[ 4096 ];
	buf.GetString( token );

	if ( stricmp( token, "VERSION" ) )
		return;

	buf.GetString( token );
	if ( atof( token ) == 1.0f )
	{
		ParseDataVersionOnePointZero( buf );
		m_bIsValid = true;
	}
	else
	{
		assert( 0 );
		return;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int CSentence::GetWordBase( void )
{
	return m_nResetWordBase;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSentence::ResetToBase( void )
{
	// Delete everything after m_nResetWordBase
	while ( m_Words.Size() > m_nResetWordBase )
	{
		delete m_Words[ m_Words.Size() - 1 ];
		m_Words.Remove( m_Words.Size() - 1 );
	}

	ClearRuntimePhonemes();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSentence::MarkNewPhraseBase( void )
{
	m_nResetWordBase = max( m_Words.Size(), 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSentence::Reset( void )
{
	m_nResetWordBase = 0;

	while ( m_Words.Size() > 0 )
	{
		delete m_Words[ 0 ];
		m_Words.Remove( 0 );
	}

	m_EmphasisSamples.RemoveAll();

	ClearRuntimePhonemes();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *tag - 
//-----------------------------------------------------------------------------
void CSentence::AddPhonemeTag( CWordTag *word, CPhonemeTag *tag )
{
	word->m_Phonemes.AddToTail( tag );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *tag - 
//-----------------------------------------------------------------------------
void CSentence::AddWordTag( CWordTag *tag )
{
	m_Words.AddToTail( tag );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int CSentence::CountPhonemes( void )
{
	int c = 0;
	for( int i = 0; i < m_Words.Size(); i++ )
	{
		CWordTag *word = m_Words[ i ];
		c += word->m_Phonemes.Size();
	}
	return c;
}

//-----------------------------------------------------------------------------
// Purpose: // For legacy loading, try to find a word that contains the time
// Input  : time - 
// Output : CWordTag
//-----------------------------------------------------------------------------
CWordTag *CSentence::EstimateBestWord( float time )
{
	CWordTag *bestWord = NULL;

	for( int i = 0; i < m_Words.Size(); i++ )
	{
		CWordTag *word = m_Words[ i ];
		if ( !word )
			continue;

		if ( word->m_flStartTime <= time && word->m_flEndTime >= time )
			return word;

		if ( time < word->m_flStartTime )
		{
			bestWord = word;
		}

		if ( time > word->m_flEndTime && bestWord )
			return bestWord;
	}

	// return best word if we found one
	if ( bestWord )
	{
		return bestWord;
	}

	// Return last word
	if ( m_Words.Size() >= 1 )
	{
		return m_Words[ m_Words.Size() - 1 ];
	}

	// Oh well
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *phoneme - 
// Output : CWordTag
//-----------------------------------------------------------------------------
CWordTag *CSentence::GetWordForPhoneme( CPhonemeTag *phoneme )
{
	for( int i = 0; i < m_Words.Size(); i++ )
	{
		CWordTag *word = m_Words[ i ];
		if ( !word )
			continue;

		for ( int j = 0 ; j < word->m_Phonemes.Size() ; j++ )
		{
			CPhonemeTag *p = word->m_Phonemes[ j ];
			if ( p == phoneme )
			{
				return word;
			}
		}

	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Assignment operator
// Input  : src - 
// Output : CSentence&
//-----------------------------------------------------------------------------
CSentence& CSentence::operator=( const CSentence& src )
{
	// Clear current stuff
	Reset();

	// Copy everything
	for ( int i = 0 ; i < src.m_Words.Size(); i++ )
	{
		CWordTag *word = src.m_Words[ i ];

		CWordTag *newWord = new CWordTag( *word );

		AddWordTag( newWord );
	}

	SetText( src.GetText() );
	m_nResetWordBase = src.m_nResetWordBase;

	int c = src.m_EmphasisSamples.Size();
	for ( i = 0; i < c; i++ )
	{
		CEmphasisSample s = src.m_EmphasisSamples[ i ];
		m_EmphasisSamples.AddToTail( s );
	}

	m_bIsCached = src.m_bIsCached;

	c = src.GetRuntimePhonemeCount();
	for ( i = 0; i < c; i++ )
	{
		Assert( m_bIsCached );

		const CBasePhonemeTag *tag = src.GetRuntimePhoneme( i );
		CPhonemeTag full;
		full.m_flStartTime = tag->m_flStartTime;
		full.m_flEndTime = tag->m_flEndTime;
		full.m_nPhonemeCode = tag->m_nPhonemeCode;

		AddRuntimePhoneme( &full );
	}

	m_bShouldVoiceDuck = src.m_bShouldVoiceDuck;

	m_bStoreCheckSum = src.m_bStoreCheckSum;
	m_uCheckSum = src.m_uCheckSum;
	m_bIsValid = src.m_bIsValid;

	return (*this);
}

void CSentence::Append( float starttime, const CSentence& src )
{
	// Combine
	for ( int i = 0 ; i < src.m_Words.Size(); i++ )
	{
		CWordTag *word = src.m_Words[ i ];

		CWordTag *newWord = new CWordTag( *word );

		newWord->m_flStartTime += starttime;
		newWord->m_flEndTime += starttime;

		// Offset times
		int c = newWord->m_Phonemes.Count();
		for ( int i = 0; i < c; ++i )
		{
			CPhonemeTag *tag = newWord->m_Phonemes[ i ];
			tag->m_flStartTime += starttime;
			tag->m_flEndTime += starttime;
		}

		AddWordTag( newWord );
	}

	if ( src.GetText()[ 0 ] )
	{
		char fulltext[ 4096 ];
		if ( GetText()[ 0 ] )
		{
			Q_snprintf( fulltext, sizeof( fulltext ), "%s %s", GetText(), src.GetText() );
		}
		else
		{
			Q_strncpy( fulltext, src.GetText(), sizeof( fulltext ) );
		}
		SetText( fulltext );
	}

	int c = src.m_EmphasisSamples.Size();
	for ( i = 0; i < c; i++ )
	{
		CEmphasisSample s = src.m_EmphasisSamples[ i ];

		s.time += starttime;

		m_EmphasisSamples.AddToTail( s );
	}

	// Or in voice duck settings
	m_bShouldVoiceDuck |= src.m_bShouldVoiceDuck;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *text - 
//-----------------------------------------------------------------------------
void CSentence::SetText( const char *text )
{
	delete[] m_szText;
	m_szText = NULL;

	if ( !text || !text[ 0 ] )
	{
		return;
	}

	int len = Q_strlen( text ) + 1;

	m_szText = new char[ len ];
	Assert( m_szText );
	Q_strncpy( m_szText, text, len );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : const char
//-----------------------------------------------------------------------------
const char *CSentence::GetText( void ) const
{
	return m_szText ? m_szText : "";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSentence::SetTextFromWords( void )
{
	char fulltext[ 1024 ];
	fulltext[ 0 ] = 0;
	for ( int i = 0 ; i < m_Words.Size(); i++ )
	{
		CWordTag *word = m_Words[ i ];

		Q_strncat( fulltext, word->GetWord(), sizeof( fulltext ), COPY_ALL_CHARACTERS );

		if ( i != m_Words.Size() )
		{
			Q_strncat( fulltext, " ", sizeof( fulltext ), COPY_ALL_CHARACTERS );
		}
	}

	SetText( fulltext );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSentence::Resort( void )
{
	int c = m_EmphasisSamples.Size();
	for ( int i = 0; i < c; i++ )
	{
		for ( int j = i + 1; j < c; j++ )
		{
			CEmphasisSample src = m_EmphasisSamples[ i ];
			CEmphasisSample dest = m_EmphasisSamples[ j ];

			if ( src.time > dest.time )
			{
				m_EmphasisSamples[ i ] = dest;
				m_EmphasisSamples[ j ] = src;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : number - 
// Output : CEmphasisSample
//-----------------------------------------------------------------------------
CEmphasisSample *CSentence::GetBoundedSample( int number, float endtime )
{
	// Search for two samples which span time f
	static CEmphasisSample nullstart;
	nullstart.time = 0.0f;
	nullstart.value = 0.5f;
	static CEmphasisSample nullend;
	nullend.time = endtime;
	nullend.value = 0.5f;
	
	if ( number < 0 )
	{
		return &nullstart;
	}
	else if ( number >= GetNumSamples() )
	{
		return &nullend;
	}
	
	return GetSample( number );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : time - 
//			type - 
// Output : float
//-----------------------------------------------------------------------------
float CSentence::GetIntensity( float time, float endtime )
{
	float zeroValue = 0.5f;
	
	int c = GetNumSamples();
	
	if ( c <= 0 )
	{
		return zeroValue;
	}
	
	int i;
	for ( i = -1 ; i < c; i++ )
	{
		CEmphasisSample *s = GetBoundedSample( i, endtime );
		CEmphasisSample *n = GetBoundedSample( i + 1, endtime );
		if ( !s || !n )
			continue;

		if ( time >= s->time && time <= n->time )
		{
			break;
		}
	}

	int prev = i - 1;
	int start = i;
	int end = i + 1;
	int next = i + 2;

	prev = max( -1, prev );
	start = max( -1, start );
	end = min( end, GetNumSamples() );
	next = min( next, GetNumSamples() );

	CEmphasisSample *esPre = GetBoundedSample( prev, endtime );
	CEmphasisSample *esStart = GetBoundedSample( start, endtime );
	CEmphasisSample *esEnd = GetBoundedSample( end, endtime );
	CEmphasisSample *esNext = GetBoundedSample( next, endtime );

	float dt = esEnd->time - esStart->time;
	dt = clamp( dt, 0.01f, 1.0f );

	Vector vPre( esPre->time, esPre->value, 0 );
	Vector vStart( esStart->time, esStart->value, 0 );
	Vector vEnd( esEnd->time, esEnd->value, 0 );
	Vector vNext( esNext->time, esNext->value, 0 );

	float f2 = ( time - esStart->time ) / ( dt );
	f2 = clamp( f2, 0.0f, 1.0f );

	Vector vOut;
	Catmull_Rom_Spline( 
		vPre,
		vStart,
		vEnd,
		vNext,
		f2, 
		vOut );

	float retval = clamp( vOut.y, 0.0f, 1.0f );
	return retval;
}

int CSentence::GetNumSamples( void )
{
	return m_EmphasisSamples.Count();
}

CEmphasisSample *CSentence::GetSample( int index )
{
	if ( index < 0 || index >= GetNumSamples() )
		return NULL;

	return &m_EmphasisSamples[ index ];
}

void CSentence::GetEstimatedTimes( float& start, float &end )
{
	float beststart = 100000.0f;
	float bestend = -100000.0f;

	int c = m_Words.Count();
	if ( !c )
	{
		start = end = 0.0f;
		return;
	}

	for ( int i = 0; i< c; i++ )
	{
		CWordTag *w = m_Words[ i ];
		Assert( w );
		if ( w->m_flStartTime < beststart )
		{
			beststart = w->m_flStartTime;
		}
		if ( w->m_flEndTime > bestend )
		{
			bestend = w->m_flEndTime;
		}
	}

	if ( beststart == 100000.0f )
	{
		Assert( 0 );
		beststart = 0.0f;
	}
	if ( bestend == -100000.0f )
	{
		Assert( 0 );
		bestend = 1.0f;
	}
	start = beststart;
	end = bestend;
}

void CSentence::SetDataCheckSum( unsigned int chk )
{
	m_bStoreCheckSum = true;
	m_uCheckSum = chk;
}

unsigned int CSentence::ComputeDataCheckSum()
{
	int i;
	int c;
	CRC32_t crc;
	CRC32_Init( &crc );

	// Checksum the text
	CRC32_ProcessBuffer( &crc, GetText(), Q_strlen( GetText() ) );
	// Checsum words and phonemes
	c = m_Words.Count();
	for ( i = 0; i < c; ++i )
	{
		CWordTag *word = m_Words[ i ];
		unsigned int wordCheckSum = word->ComputeDataCheckSum();
		CRC32_ProcessBuffer( &crc, &wordCheckSum, sizeof( unsigned int ) );
	}

	// Checksum emphasis data
	c = m_EmphasisSamples.Count();
	for ( i = 0; i < c; ++i )
	{
		CRC32_ProcessBuffer( &crc, &m_EmphasisSamples[ i ].time, sizeof( float ) );
		CRC32_ProcessBuffer( &crc, &m_EmphasisSamples[ i ].value, sizeof( float ) );
	}

	CRC32_Final( &crc );

	return ( unsigned int )crc;
}

unsigned int CSentence::GetDataCheckSum() const
{
	Assert( m_bStoreCheckSum );
	Assert( m_uCheckSum != 0 );
	return m_uCheckSum;
}
