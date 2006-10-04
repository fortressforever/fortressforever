//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef FF_CLIENTMODE_H
#define FF_CLIENTMODE_H
#ifdef _WIN32
#pragma once
#endif

#include "clientmode_shared.h"
#include "ffviewport.h"

class ClientModeFFNormal : public ClientModeShared 
{
DECLARE_CLASS( ClientModeFFNormal, ClientModeShared );

private:

// IClientMode overrides.
public:

					ClientModeFFNormal();
	virtual			~ClientModeFFNormal();

	virtual void	InitViewport();

	virtual float	GetViewModelFOV( void );

	int				GetDeathMessageStartHeight( void );

	virtual void	PostRenderVGui();

	virtual void	Init( void );
	virtual void	FireGameEvent( IGameEvent *pEvent );

	void SetNextValidationFilePath(const char* szFilePath);

public:
	static bool ValidateLevel(const char* szValidateFilePath,
							  const char* szLevelName,
							  CRC32_t checksum,
							  char* szDescription,
							  int descMaxLength);
	static void FFScriptCRC_MsgHandler(bf_read& msg);
	
private:
	//	void	UpdateSpectatorMode( void );

private:
	bool	m_serverScriptValid;
	CRC32_t	m_servercriptCRC;
	char	m_szValidationFile[1024];

};


extern IClientMode *GetClientModeNormal();
extern ClientModeFFNormal* GetClientModeFFNormal();


#endif // FF_CLIENTMODE_H
