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

};


extern IClientMode *GetClientModeNormal();
extern ClientModeFFNormal* GetClientModeFFNormal();


#endif // FF_CLIENTMODE_H
