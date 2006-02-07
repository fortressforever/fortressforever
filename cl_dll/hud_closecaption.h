//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HUD_CLOSECAPTION_H
#define HUD_CLOSECAPTION_H
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include <vgui_controls/Panel.h>

class CSentence;
class C_BaseFlex;
class CCloseCaptionItem;
struct WorkUnitParams;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CHudCloseCaption : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudCloseCaption, vgui::Panel );
public:
	DECLARE_MULTIPLY_INHERITED();

					CHudCloseCaption( const char *pElementName );
	virtual 		~CHudCloseCaption();

	// Expire lingering items
	virtual void	OnTick( void );

	virtual void	LevelInit( void );

	virtual void	LevelShutdown( void )
	{
		Reset();
	}

	// Painting methods
	virtual void	Paint();

	void MsgFunc_CloseCaption(bf_read &msg);

	// Clear all CC data
	void			Reset( void );
	void			Process( const wchar_t *stream, float duration, char const *tokenstream, bool fromplayer );
	
	void			ProcessCaption( char const *tokenname, float duration, bool fromplayer = false );

	void			ProcessSentenceCaptionStream( char const *tokenstream );

	enum
	{
		CCFONT_NORMAL = 0,
		CCFONT_ITALIC,
		CCFONT_BOLD,
		CCFONT_ITALICBOLD
	};

	static int GetFontNumber( bool bold, bool italic );

public:

	struct CaptionRepeat
	{
		CaptionRepeat() :
			m_nTokenIndex( 0 ),
			m_flLastEmitTime( 0 ),
			m_flInterval( 0 ),
			m_nLastEmitTick( 0 )
		{
		}
		int		m_nTokenIndex;
		int		m_nLastEmitTick;
		float	m_flLastEmitTime;
		float	m_flInterval;
	};

	struct TokenNameLookup
	{
		CRC32_t	crc;
		int		stringIndex;
	};

private:
	
	CUtlRBTree< CaptionRepeat, int >	m_CloseCaptionRepeats;
	CUtlRBTree< TokenNameLookup, int >	m_TokenNameLookup;

private:

	static bool CaptionTokenLessFunc( const CaptionRepeat &lhs, const CaptionRepeat &rhs );
	static bool TokenNameLessFunc( const TokenNameLookup &lhs, const TokenNameLookup &rhs );

	void	DrawStream( wrect_t& rect, CCloseCaptionItem *item ); 
	void	ComputeStreamWork( int available_width, CCloseCaptionItem *item );
	bool	LookupUnicodeText( char const *token, wchar_t *outbuf, size_t count );
	bool	SplitCommand( wchar_t const **ppIn, wchar_t *cmd, wchar_t *args ) const;

	bool	StreamHasCommand( const wchar_t *stream, const wchar_t *findcmd ) const;
	bool	GetFloatCommandValue( const wchar_t *stream, const wchar_t *findcmd, float& value ) const;

	bool	GetNoRepeatValue( const wchar_t *caption, float &retval );

	void	ParseCloseCaptionStream( const wchar_t *in, int available_width );
	bool	StreamHasCommand( const wchar_t *stream, const wchar_t *search );

	void	DumpWork( CCloseCaptionItem *item );

	void	BuildTokenNameLookup();

	void AddWorkUnit( 
		CCloseCaptionItem *item,	
		WorkUnitParams& params );

	CUtlVector< CCloseCaptionItem * > m_Items;


	vgui::HFont		m_hFonts[ 4 ];

	void			CreateFonts( void );

	int			m_nLineHeight;

	int			m_nGoalHeight;
	int			m_nCurrentHeight;
	float		m_flGoalAlpha;
	float		m_flCurrentAlpha;
	float		m_flGoalHeightStartTime;
	float		m_flGoalHeightFinishTime;

	CPanelAnimationVar( float, m_flBackgroundAlpha, "BgAlpha", "192" );
	CPanelAnimationVar( float, m_flGrowTime, "GrowTime", "0.25" );
	CPanelAnimationVar( float, m_flItemHiddenTime, "ItemHiddenTime", "0.2" );
	CPanelAnimationVar( float, m_flItemFadeInTime, "ItemFadeInTime", "0.15" );
	CPanelAnimationVar( float, m_flItemFadeOutTime, "ItemFadeOutTime", "0.3" );

};


#endif // HUD_CLOSECAPTION_H
