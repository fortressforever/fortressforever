//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef SENTENCE_H
#define SENTENCE_H
#ifdef _WIN32
#pragma once
#endif

#include "utlvector.h"

class CUtlBuffer;

//-----------------------------------------------------------------------------
// Purpose: A sample point
//-----------------------------------------------------------------------------
struct CEmphasisSample
{
	float		time;
	float		value;

	// Used by editors only
	bool		selected;
};

class CBasePhonemeTag
{
public:
	CBasePhonemeTag();
	CBasePhonemeTag( const CBasePhonemeTag& from );

	float			m_flStartTime;
	float			m_flEndTime;
	unsigned short	m_nPhonemeCode;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CPhonemeTag : public CBasePhonemeTag
{
	typedef CBasePhonemeTag BaseClass;
public:

					CPhonemeTag( void );
					CPhonemeTag( const char *phoneme );
					CPhonemeTag( const CPhonemeTag& from );
					~CPhonemeTag( void );

	void			SetTag( const char *phoneme );
	char const		*GetTag() const;

	unsigned int	ComputeDataCheckSum();

	bool			m_bSelected;

	// Used by UI code
	unsigned int	m_uiStartByte;
	unsigned int	m_uiEndByte;

private:
	char			*m_szPhoneme;

};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CWordTag
{
public:
					CWordTag( void );
					CWordTag( const char *word );
					CWordTag( const CWordTag& from );
					~CWordTag( void );

	void			SetWord( const char *word );
	const char		*GetWord() const;

	int				IndexOfPhoneme( CPhonemeTag *tag );

	unsigned int	ComputeDataCheckSum();

	float			m_flStartTime;
	float			m_flEndTime;

	CUtlVector	< CPhonemeTag *> m_Phonemes;

	bool			m_bSelected;
	unsigned int	m_uiStartByte;
	unsigned int	m_uiEndByte;
private:
	char			*m_pszWord;
};

// A sentence can be closed captioned
// The default case is the entire sentence shown at start time
// 
// "<persist:2.0><clr:255,0,0,0>The <I>default<I> case"
// "<sameline>is the <U>entire<U> sentence shown at <B>start time<B>"

// Commands that aren't closed at end of phrase are automatically terminated
//
// Commands
// <linger:2.0>	The line should persist for 2.0 seconds beyond m_flEndTime
// <sameline>		Don't go to new line for next phrase on stack
// <clr:r,g,b,a>	Push current color onto stack and start drawing with new
//  color until we reach the next <clr> marker or a <clr> with no commands which
//  means restore the previous color
// <U>				Underline text (start/end)
// <I>				Italics text (start/end)
// <B>				Bold text (start/end)
// <position:where>	Draw caption at special location ??? needed
// <cr>				Go to new line

// Close Captioning Support
// The phonemes drive the mouth in english, but the CC text can
//  be one of several languages
enum
{
	CC_ENGLISH = 0,
	CC_FRENCH,
	CC_GERMAN,
	CC_ITALIAN,
	CC_KOREAN,
	CC_SCHINESE,  // Simplified Chinese
	CC_SPANISH,
	CC_TCHINESE,  // Traditional Chinese
	CC_JAPANESE,
	CC_RUSSIAN,
	CC_THAI,
	CC_PORTUGUESE,
	// etc etc

	CC_NUM_LANGUAGES
};

//-----------------------------------------------------------------------------
// Purpose: A sentence is a box of words, and words contain phonemes
//-----------------------------------------------------------------------------
class CSentence
{
public:
	static char const	*NameForLanguage( int language );
	static int			LanguageForName( char const *name );
	static void 	ColorForLanguage( int language, unsigned char& r, unsigned char& g, unsigned char& b );

	// Construction
					CSentence( void );
					~CSentence( void );

	// Assignment operator
	CSentence& operator =(const CSentence& src );

	void			Append( float starttime, const CSentence& src );

	void			SetText( const char *text );
	const char		*GetText( void ) const;

	void			InitFromDataChunk( void *data, int size );
	void			InitFromBuffer( CUtlBuffer& buf );
	void			SaveToBuffer( CUtlBuffer& buf );

	//				This strips out all of the stuff used by the editor, leaving just one blank work, no sentence text, and just
	//					the phonemes without the phoneme text...(same as the cacherestore version below)
	void			MakeRuntimeOnly();

	// This is a compressed save of just the data needed to drive phonemes in the engine (no word / sentence text, etc )
	void			CacheSaveToBuffer( CUtlBuffer& buf );
	void			CacheRestoreFromBuffer( CUtlBuffer& buf );

	// Add word/phoneme to sentence
	void			AddPhonemeTag( CWordTag *word, CPhonemeTag *tag );
	void			AddWordTag( CWordTag *tag );

	void			Reset( void );

	void			ResetToBase( void );

	void			MarkNewPhraseBase( void );

	int				GetWordBase( void );

	int				CountPhonemes( void );

	// For legacy loading, try to find a word that contains the time
	CWordTag		*EstimateBestWord( float time );

	CWordTag		*GetWordForPhoneme( CPhonemeTag *phoneme );

	void			SetTextFromWords( void );

	float			GetIntensity( float time, float endtime );
	void			Resort( void );
	CEmphasisSample *GetBoundedSample( int number, float endtime );
	int				GetNumSamples( void );
	CEmphasisSample	*GetSample( int index );

	// Compute start and endtime based on all words
	void			GetEstimatedTimes( float& start, float &end );

	void			SetVoiceDuck( bool shouldDuck ) { m_bShouldVoiceDuck = shouldDuck; }
	bool			GetVoiceDuck() const { return m_bShouldVoiceDuck; }

	unsigned int	ComputeDataCheckSum();

	void			SetDataCheckSum( unsigned int chk );
	unsigned int	GetDataCheckSum() const;

	int				GetRuntimePhonemeCount() const;
	const CBasePhonemeTag *GetRuntimePhoneme( int i ) const;
	void			ClearRuntimePhonemes();
	void			AddRuntimePhoneme( const CPhonemeTag *src );

public:
	char			*m_szText;

	CUtlVector< CWordTag * >	m_Words;
	CUtlVector	< CBasePhonemeTag *> m_RunTimePhonemes;

	int				m_nResetWordBase;
	
	// Phoneme emphasis data
	CUtlVector< CEmphasisSample > m_EmphasisSamples;

	unsigned int	m_uCheckSum;
	bool			m_bIsValid : 8;
	bool			m_bStoreCheckSum : 8;
	bool			m_bShouldVoiceDuck : 8;
	bool			m_bIsCached : 8;

private:
	void			ParseDataVersionOnePointZero( CUtlBuffer& buf );
	void			ParsePlaintext( CUtlBuffer& buf );
	void			ParseWords( CUtlBuffer& buf );
	void			ParseEmphasis( CUtlBuffer& buf );
	void			ParseOptions( CUtlBuffer& buf );
	void			ParseCloseCaption( CUtlBuffer& buf );

	void			ResetCloseCaptionAll( void );

	friend class PhonemeEditor;
};

#endif // SENTENCE_H
