//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "soundscape_system.h"
#include "soundscape.h"
#include "KeyValues.h"
#include "filesystem.h"
#include "game.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SOUNDSCAPE_MANIFEST_FILE				"scripts/soundscapes_manifest.txt"

CON_COMMAND(soundscape_flush, "Flushes the server & client side soundscapes")
{
	g_SoundscapeSystem.Shutdown();
	g_SoundscapeSystem.Init();
	engine->ClientCommand( UTIL_GetCommandClient()->edict(), "cl_soundscape_flush\n" );
}

CSoundscapeSystem g_SoundscapeSystem;


void CSoundscapeSystem::AddSoundscapeFile( const char *filename )
{
	// Open the soundscape data file, and abort if we can't
	KeyValues *pKeyValuesData = new KeyValues( filename );
	if ( pKeyValuesData->LoadFromFile( filesystem, filename ) )
	{
		// parse out all of the top level sections and save their names
		KeyValues *pKeys = pKeyValuesData;
		while ( pKeys )
		{
			if ( pKeys->GetFirstSubKey() )
			{
				if ( g_pDeveloper->GetBool() )
				{
					if ( strstr( pKeys->GetName(), "{" ) )
					{
						Msg("Error parsing soundscape file %s after %s\n", filename, m_soundscapeCount>0 ?m_soundscapes.GetStringText( m_soundscapeCount-1 ) : "FIRST" );
					}
				}
				m_soundscapes.AddString( pKeys->GetName(), m_soundscapeCount );
				m_soundscapeCount++;
			}
			pKeys = pKeys->GetNextKey();
		}
	}
	pKeyValuesData->deleteThis();
}

CON_COMMAND(sv_soundscape_printdebuginfo, "print soundscapes")
{
	g_SoundscapeSystem.PrintDebugInfo();
}


void CSoundscapeSystem::PrintDebugInfo()
{
	Msg( "\n------- SERVER SOUNDSCAPES -------\n" );
	for ( int key=m_soundscapes.First(); key != m_soundscapes.InvalidIndex(); key = m_soundscapes.Next( key ) )
	{
		int id = m_soundscapes.GetIDForKey( key );
		const char *pName = m_soundscapes.GetStringForKey( key );

		Msg( "- %d: %s\n", id, pName );
	}
	Msg( "----------------------------------\n\n" );
}

bool CSoundscapeSystem::Init()
{
	m_soundscapeCount = 0;

	const char *mapname = STRING( gpGlobals->mapname );
	const char *mapSoundscapeFilename = NULL;
	if ( mapname && *mapname )
	{
		// BEG: Mulch
		//mapSoundscapeFilename = UTIL_VarArgs( "scripts/soundscapes_%s.txt", mapname );		
		mapSoundscapeFilename = UTIL_VarArgs( "maps/%s_soundscapes.txt", mapname );
		//DevMsg( "[Soundscape] Setting soundscape file to: %s\n", mapSoundscapeFilename );
		// END: Mulch
	}

	KeyValues *manifest = new KeyValues( SOUNDSCAPE_MANIFEST_FILE );
	if ( manifest->LoadFromFile( filesystem, SOUNDSCAPE_MANIFEST_FILE, "GAME" ) )
	{
		for ( KeyValues *sub = manifest->GetFirstSubKey(); sub != NULL; sub = sub->GetNextKey() )
		{
			if ( !Q_stricmp( sub->GetName(), "file" ) )
			{
				// Add
				AddSoundscapeFile( sub->GetString() );
				if ( mapSoundscapeFilename && FStrEq( sub->GetString(), mapSoundscapeFilename ) )
				{
					mapSoundscapeFilename = NULL; // we've already loaded the map's soundscape
				}
				continue;
			}

			Warning( "CSoundscapeSystem::Init:  Manifest '%s' with bogus file type '%s', expecting 'file'\n", 
				SOUNDSCAPE_MANIFEST_FILE, sub->GetName() );
		}

		if ( mapSoundscapeFilename && filesystem->FileExists( mapSoundscapeFilename ) )
		{
			AddSoundscapeFile( mapSoundscapeFilename );
		}
	}
	else
	{
		Error( "Unable to load manifest file '%s'\n", SOUNDSCAPE_MANIFEST_FILE );
	}
	manifest->deleteThis();
	m_activeIndex = -1;

	return true;
}

void CSoundscapeSystem::Shutdown()
{
	m_soundscapeCount = 0;
	m_soundscapes.ClearStrings();
	m_soundscapeEntities.RemoveAll();
	m_activeIndex = -1;
}

void CSoundscapeSystem::LevelInitPreEntity()
{
	g_SoundscapeSystem.Shutdown();
	g_SoundscapeSystem.Init();
}

int	CSoundscapeSystem::GetSoundscapeIndex( const char *pName )
{
	return m_soundscapes.GetStringID( pName );
}

bool CSoundscapeSystem::IsValidIndex( int index )
{
	if ( index >= 0 && index < m_soundscapeCount )
		return true;
	return false;
}

void CSoundscapeSystem::AddSoundscapeEntity( CEnvSoundscape *pSoundscape )
{
	if ( m_soundscapeEntities.Find( pSoundscape ) == -1 )
	{
		m_soundscapeEntities.AddToTail( pSoundscape );
	}
}

void CSoundscapeSystem::RemoveSoundscapeEntity( CEnvSoundscape *pSoundscape )
{
	m_soundscapeEntities.FindAndRemove( pSoundscape );
}

void CSoundscapeSystem::FrameUpdatePostEntityThink()
{
	int total = m_soundscapeEntities.Count();
	if ( total > 0 )
	{
		if ( !m_soundscapeEntities.IsValidIndex(m_activeIndex) )
		{
			m_activeIndex = 0;
		}

		// update 2 soundscape entities each tick
		int count = min(2, total);
		for ( int i = 0; i < count; i++ )
		{
			m_activeIndex++;
			m_activeIndex = m_activeIndex % total;
			m_soundscapeEntities[m_activeIndex]->Update();
		}
	}
}
