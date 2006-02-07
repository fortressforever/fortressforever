//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "AI_Criteria.h"
#include "ai_speech.h"
#include <KeyValues.h>
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
AI_CriteriaSet::AI_CriteriaSet()
{
	m_pCriteria = new KeyValues( "CriteriaSet" );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : src - 
//-----------------------------------------------------------------------------
AI_CriteriaSet::AI_CriteriaSet( const AI_CriteriaSet& src )
{
	m_pCriteria = new KeyValues( "CriteriaSet" );
	if ( src.m_pCriteria )
	{
		*m_pCriteria = *src.m_pCriteria;
	}

	KeyValues *kv;
	for ( kv = m_pCriteria->GetFirstSubKey(); kv; kv = kv->GetNextKey() )
	{
		m_Lookup.Insert( kv->GetName(), kv );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
AI_CriteriaSet::~AI_CriteriaSet()
{
	m_pCriteria->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *criteria - 
//			"" - 
//			1.0f - 
//-----------------------------------------------------------------------------
void AI_CriteriaSet::AppendCriteria( const char *criteria, const char *value /*= ""*/, float weight /*= 1.0f*/ )
{
	Assert( m_pCriteria );

	KeyValues *sub = m_pCriteria->FindKey( criteria, true );
	Assert( sub );

	sub->SetString( "value", value );
	sub->SetFloat( "weight", weight );

	m_Lookup.Insert( criteria, sub );
}


//-----------------------------------------------------------------------------
// Removes criteria in a set
//-----------------------------------------------------------------------------
void AI_CriteriaSet::RemoveCriteria( const char *criteria )
{
	Assert( m_pCriteria );

	KeyValues *sub = m_pCriteria->FindKey( criteria );
	if ( sub )
	{
		m_Lookup.Remove( sub->GetName() );
		sub->deleteThis();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int AI_CriteriaSet::GetCount() const
{
	return m_Lookup.Count();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *name - 
// Output : int
//-----------------------------------------------------------------------------
int AI_CriteriaSet::FindCriterionIndex( const char *name ) const
{
	int idx = m_Lookup.Find( name );
	if ( idx == m_Lookup.InvalidIndex() )
		return -1;

	return idx;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : index - 
// Output : char const
//-----------------------------------------------------------------------------
const char *AI_CriteriaSet::GetName( int index ) const
{
	if ( index < 0 || index >= (int)m_Lookup.Count() )
		return "";

	KeyValues *sub = m_Lookup[ index ];
	return sub->GetName();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : index - 
// Output : char const
//-----------------------------------------------------------------------------
const char *AI_CriteriaSet::GetValue( int index ) const
{
	if ( index < 0 || index >= (int)m_Lookup.Count() )
		return "";

	KeyValues *sub = m_Lookup[ index ];
	return sub->GetString( "value", "" );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : index - 
// Output : float
//-----------------------------------------------------------------------------
float AI_CriteriaSet::GetWeight( int index ) const
{
	if ( index < 0 || index >= (int)m_Lookup.Count() )
		return 1.0f;

	KeyValues *sub = m_Lookup[ index ];
	return sub->GetFloat( "weight", 1.0f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void AI_CriteriaSet::Describe()
{
	KeyValues *kv = m_pCriteria->GetFirstSubKey();
	while( kv )
	{
		const char *value = kv->GetString( "value", "" );
		if ( value && value[ 0 ] )
		{
			float w = kv->GetFloat( "weight", 1.0f );
			if ( w != 1.0f )
			{
				DevMsg( "  %20s = '%s' (weight %f)\n", kv->GetName(), value, w );
			}
			else
			{
				DevMsg( "  %20s = '%s'\n", kv->GetName(), value );
			}
		}
		kv = kv->GetNextKey();
	}
}

BEGIN_SIMPLE_DATADESC( AI_ResponseParams )
	DEFINE_FIELD( flags,	FIELD_INTEGER ),
	DEFINE_FIELD( odds,	FIELD_INTEGER ),	
	DEFINE_FIELD( soundlevel,	FIELD_INTEGER ),	
	DEFINE_FIELD( delay,	FIELD_INTERVAL ),	
	DEFINE_FIELD( respeakdelay,	FIELD_INTERVAL ),	
END_DATADESC()

BEGIN_SIMPLE_DATADESC( AI_Response )
	DEFINE_FIELD( m_Type,	FIELD_INTEGER ),
	DEFINE_ARRAY( m_szResponseName, FIELD_CHARACTER, AI_Response::MAX_RESPONSE_NAME ),	
	DEFINE_ARRAY( m_szMatchingRule, FIELD_CHARACTER, AI_Response::MAX_RULE_NAME ),	
	// DEFINE_FIELD( m_pCriteria, FIELD_??? ), // Don't need to save this probably
	DEFINE_EMBEDDED( m_Params ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
AI_Response::AI_Response()
{
	m_Type = RESPONSE_NONE;
	m_szResponseName[0] = 0;
	m_pCriteria = NULL;
	m_szMatchingRule[0]=0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
AI_Response::AI_Response( const AI_Response &from )
{
	Assert( (void*)(&m_Type) == (void*)this );
	memcpy( this, &from, sizeof(*this) );
	m_pCriteria = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
AI_Response::~AI_Response()
{
	delete m_pCriteria;
}

//-----------------------------------------------------------------------------
AI_Response &AI_Response::operator=( const AI_Response &from )
{
	Assert( (void*)(&m_Type) == (void*)this );
	memcpy( this, &from, sizeof(*this) );
	m_pCriteria = NULL;

	return *this;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *response - 
//			*criteria - 
//-----------------------------------------------------------------------------
void AI_Response::Init( ResponseType_t type, const char *responseName, const AI_CriteriaSet& criteria, const AI_ResponseParams& responseparams, const char *ruleName )
{
	m_Type = type;
	Q_strncpy( m_szResponseName, responseName, sizeof( m_szResponseName ) );
	// Copy underlying criteria
	m_pCriteria = new AI_CriteriaSet( criteria );
	Q_strncpy( m_szMatchingRule, ruleName ? ruleName : "NULL", sizeof( m_szMatchingRule ) );
	m_Params = responseparams;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void AI_Response::Describe()
{
	if ( m_pCriteria )
	{
		DevMsg( "Search criteria:\n" );
		m_pCriteria->Describe();
	}
	if ( m_szMatchingRule[0] )
	{
		DevMsg( "Matched rule '%s', ", m_szMatchingRule );
	}
	DevMsg( "response %s = '%s'\n", DescribeResponse( m_Type ), m_szResponseName );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : char const
//-----------------------------------------------------------------------------
const char *AI_Response::GetName() const
{
	return m_szResponseName;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : char const
//-----------------------------------------------------------------------------
const char *AI_Response::GetResponse() const
{
	return GetName();
}
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : type - 
// Output : char const
//-----------------------------------------------------------------------------
const char *AI_Response::DescribeResponse( ResponseType_t type )
{
	if ( (int)type < 0 || (int)type >= NUM_RESPONSES )
	{
		Assert( 0 );
		return "???AI_Response bogus index";
	}

	switch( type )
	{
	default:
		{
			Assert( 0 );
		}
		// Fall through
	case RESPONSE_NONE:
		return "RESPONSE_NONE";
	case RESPONSE_SPEAK:
		return "RESPONSE_SPEAK";
	case RESPONSE_SENTENCE:
		return "RESPONSE_SENTENCE";
	case RESPONSE_SCENE:
		return "RESPONSE_SCENE";
	case RESPONSE_RESPONSE:
		return "RESPONSE_RESPONSE";
	case RESPONSE_PRINT:
		return "RESPONSE_PRINT";
	}

	return "RESPONSE_NONE";
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : const AI_CriteriaSet
//-----------------------------------------------------------------------------
const AI_CriteriaSet *AI_Response::GetCriteria()
{
	return m_pCriteria;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void AI_Response::Release()
{
	delete this;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : soundlevel_t
//-----------------------------------------------------------------------------
soundlevel_t AI_Response::GetSoundLevel() const
{
	if ( m_Params.flags & AI_ResponseParams::RG_SOUNDLEVEL )
	{
		return m_Params.soundlevel;
	}

	return SNDLVL_TALKING;
}

float AI_Response::GetRespeakDelay( void ) const
{
	if ( m_Params.flags & AI_ResponseParams::RG_RESPEAKDELAY )
	{
		return RandomInterval( m_Params.respeakdelay );
	}

	return 0.0f;
}

bool AI_Response::GetSpeakOnce( void ) const
{
	if ( m_Params.flags & AI_ResponseParams::RG_SPEAKONCE )
	{
		return true;
	}

	return false;
}

bool AI_Response::ShouldntUseScene( void ) const
{
	return ( m_Params.flags & AI_ResponseParams::RG_DONT_USE_SCENE ) != 0;
}

int AI_Response::GetOdds( void ) const
{
	if ( m_Params.flags & AI_ResponseParams::RG_ODDS )
	{
		return m_Params.odds;
	}
	return 100;
}

float AI_Response::GetDelay() const
{
	if ( m_Params.flags & AI_ResponseParams::RG_DELAYAFTERSPEAK )
	{
		return RandomInterval( m_Params.delay );
	}
	return 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *raw - 
//			*key - 
//			keylen - 
//			*value - 
//			valuelen - 
// Output : static bool
//-----------------------------------------------------------------------------
const char *SplitContext( const char *raw, char *key, int keylen, char *value, int valuelen )
{
	char *colon = Q_strstr( raw, ":" );
	if ( !colon )
	{
		DevMsg( "SplitContext:  warning, ignoring context '%s', missing colon separator!\n", raw );
		return NULL;
	}

	int len = colon - raw;
	Q_strncpy( key, raw, min( len + 1, keylen ) );
	key[ min( len, keylen ) ] = 0;

	bool last = false;
	char *end = Q_strstr( colon + 1, "," );
	if ( !end )
	{
		int remaining = Q_strlen( colon + 1 );
		end = colon + 1 + remaining;
		last = true;
	}
	len = end - ( colon + 1 );

	Q_strncpy( value, colon + 1, len + 1 );
	value[ len ] = 0;

	return last ? NULL : end + 1;
}
