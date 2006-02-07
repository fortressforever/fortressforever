//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
// Client-side CBasePlayer

#ifndef C_STUDIOFLEX_H
#define C_STUDIOFLEX_H
#pragma once


#include "c_baseanimating.h"
#include "c_baseanimatingoverlay.h"

#include "UtlVector.h"

//-----------------------------------------------------------------------------
// Purpose: Item in list of loaded scene files
//-----------------------------------------------------------------------------
class CFlexSceneFile
{
public:
	enum
	{
		MAX_FLEX_FILENAME = 128,
	};

	char			filename[ MAX_FLEX_FILENAME ];
	void			*buffer;
};

// For phoneme emphasis track
struct Emphasized_Phoneme;
class CSentence;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_BaseFlex : public C_BaseAnimatingOverlay
{
	DECLARE_CLASS( C_BaseFlex, C_BaseAnimatingOverlay );
public:
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();

					C_BaseFlex();
	virtual			~C_BaseFlex();

	virtual studiohdr_t *OnNewModel( void );

	// model specific
	virtual	void	SetupWeights( );

	virtual void	RunFlexRules( float *dest );

	virtual void	SetViewTarget( void );

	virtual bool	GetSoundSpatialization( SpatializationInfo_t& info );

public:
	Vector			m_viewtarget;
	CInterpolatedVar< Vector >	m_iv_viewtarget;
	float			m_flexWeight[64];
	CInterpolatedVarArray< float, 64 >	m_iv_flexWeight;

	int				m_blinktoggle;

	static int		AddGlobalFlexController( char *szName );

	// bah, this should be unified with all prev/current stuff.
private:
	float			m_blinktime;
	int				m_prevblinktoggle;

	int				m_iBlink;
	int				m_iEyeUpdown;
	int				m_iEyeRightleft;
	int				m_iMouthAttachment;

	float			*m_flFlexDelayedWeight;

	// shared flex controllers
	static int		g_numflexcontrollers;
	static char		*g_flexcontroller[MAXSTUDIOFLEXCTRL*4]; // room for global set of flexcontrollers
	static float	g_flexweight[MAXSTUDIOFLEXDESC];

private:
	C_BaseFlex( const C_BaseFlex & ); // not defined, not accessible

	enum
	{
		PHONEME_CLASS_WEAK = 0,
		PHONEME_CLASS_NORMAL,
		PHONEME_CLASS_STRONG,

		NUM_PHONEME_CLASSES
	};

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	struct Emphasized_Phoneme
	{
		// Global fields, setup at start
		char			classname[ 64 ];
		bool			required;
		// Global fields setup first time tracks played
		bool			basechecked;
		const flexsettinghdr_t *base;
		bool			overridechecked;
		const flexsettinghdr_t *override;

		const flexsetting_t *exp;

		// Local fields, processed for each sentence
		bool			valid;
		float			amount;
	};

	// For handling scene files
	void			*FindSceneFile( const char *filename );

	const flexsetting_t *FindNamedSetting( const flexsettinghdr_t *pSettinghdr, const char *expr );

	void			ProcessVisemes( Emphasized_Phoneme *classes );
	void			AddVisemesForSentence( Emphasized_Phoneme *classes, float emphasis_intensity, CSentence *sentence, float t, float dt, bool juststarted );
	void			AddViseme( Emphasized_Phoneme *classes, float emphasis_intensity, int phoneme, float scale, bool newexpression );
	bool			SetupEmphasisBlend( Emphasized_Phoneme *classes, int phoneme );
	void			ComputeBlendedSetting( Emphasized_Phoneme *classes, float emphasis_intensity );

	Emphasized_Phoneme m_PhonemeClasses[ NUM_PHONEME_CLASSES ];
};

EXTERN_RECV_TABLE(DT_BaseFlex);

float *GetVisemeWeights( int phoneme );

#endif // C_STUDIOFLEX_H




