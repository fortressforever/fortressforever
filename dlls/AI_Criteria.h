//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef AI_CRITERIA_H
#define AI_CRITERIA_H
#ifdef _WIN32
#pragma once
#endif

#include "utldict.h"
#include "interval.h"

extern const char *SplitContext( const char *raw, char *key, int keylen, char *value, int valuelen );


class AI_CriteriaSet
{
public:
	AI_CriteriaSet();
	AI_CriteriaSet( const AI_CriteriaSet& src );
	~AI_CriteriaSet();

	void AppendCriteria( const char *criteria, const char *value = "", float weight = 1.0f );
	void RemoveCriteria( const char *criteria );
	
	void Describe();

	int GetCount() const;
	int			FindCriterionIndex( const char *name ) const;

	const char *GetName( int index ) const;
	const char *GetValue( int index ) const;
	float		GetWeight( int index ) const;

private:

	KeyValues	*m_pCriteria;
	CUtlDict< KeyValues *, int > m_Lookup;
};

struct AI_ResponseParams
{
	DECLARE_SIMPLE_DATADESC();

	enum
	{
		RG_DELAYAFTERSPEAK = (1<<0),
		RG_SPEAKONCE = (1<<1),
		RG_ODDS = (1<<2),
		RG_RESPEAKDELAY = (1<<3),
		RG_SOUNDLEVEL = (1<<4),
		RG_DONT_USE_SCENE = (1<<5),
	};

	AI_ResponseParams()
	{
		flags = 0;
		odds = 100;
		Q_memset( &delay, 0, sizeof( delay ) );
		Q_memset( &respeakdelay, 0, sizeof( respeakdelay ) );
		Q_memset( &soundlevel, 0, sizeof( soundlevel ) );
	}

	int						flags;
	int						odds;
	interval_t				delay;
	interval_t				respeakdelay;
	soundlevel_t			soundlevel;
};

//-----------------------------------------------------------------------------
// Purpose: Generic container for a response to a match to a criteria set
//  This is what searching for a response returns
//-----------------------------------------------------------------------------
enum ResponseType_t
{
	RESPONSE_NONE = 0,
	RESPONSE_SPEAK,
	RESPONSE_SENTENCE,
	RESPONSE_SCENE,
	RESPONSE_RESPONSE, // A reference to another response by name
	RESPONSE_PRINT,

	NUM_RESPONSES,
};

class AI_Response
{
public:
	DECLARE_SIMPLE_DATADESC();

	AI_Response();
	AI_Response( const AI_Response &from );
	~AI_Response();
	AI_Response &operator=( const AI_Response &from );

	void	Release();

	const char *	GetName() const;
	const char *	GetResponse() const;
	const AI_ResponseParams *GetParams() const { return &m_Params; }
	ResponseType_t	GetType() const { return m_Type; }
	soundlevel_t	GetSoundLevel() const;
	float			GetRespeakDelay() const;
	bool			GetSpeakOnce() const;
	bool			ShouldntUseScene( ) const;
	int				GetOdds() const;
	float			GetDelay() const;

	void Describe();

	const AI_CriteriaSet* GetCriteria();

	void	Init( ResponseType_t type, 
				const char *responseName, 
				const AI_CriteriaSet& criteria, 
				const AI_ResponseParams& responseparams,
				const char *matchingRule );

	static const char *DescribeResponse( ResponseType_t type );

private:
	enum
	{
		MAX_RESPONSE_NAME = 128,
		MAX_RULE_NAME = 128
	};

	ResponseType_t	m_Type;
	char			m_szResponseName[ MAX_RESPONSE_NAME ];

	// The initial criteria to which we are responsive
	AI_CriteriaSet	*m_pCriteria;

	AI_ResponseParams m_Params;

	char			m_szMatchingRule[ MAX_RULE_NAME ];
};

#endif // AI_CRITERIA_H
