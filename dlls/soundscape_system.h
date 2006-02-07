//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef SOUNDSCAPE_SYSTEM_H
#define SOUNDSCAPE_SYSTEM_H
#ifdef _WIN32
#pragma once
#endif


#include "stringregistry.h"
class CEnvSoundscape;

class CSoundscapeSystem : public CAutoGameSystem
{
public:
	// game system
	virtual bool Init();
	virtual void Shutdown();
	virtual void FrameUpdatePostEntityThink();
	virtual void LevelInitPreEntity();

	virtual void AddSoundscapeFile( const char *filename );
	int	GetSoundscapeIndex( const char *pName );
	bool IsValidIndex( int index );

	void AddSoundscapeEntity( CEnvSoundscape *pSoundscape );
	void RemoveSoundscapeEntity( CEnvSoundscape *pSoundscape );
	void PrintDebugInfo();

private:
	CStringRegistry		m_soundscapes;
	int					m_soundscapeCount;
	CUtlVector<CEnvSoundscape *>	m_soundscapeEntities;
	int					m_activeIndex;
};

extern CSoundscapeSystem g_SoundscapeSystem;


#endif // SOUNDSCAPE_SYSTEM_H
