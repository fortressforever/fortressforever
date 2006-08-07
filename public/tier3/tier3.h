//===== Copyright � 2005-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: A higher level link library for general use in the game and tools.
//
//===========================================================================//


#ifndef TIER3_H
#define TIER3_H

#if defined( _WIN32 )
#pragma once
#endif

#include "tier2/tier2.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class IStudioRender;
class IMatSystemSurface;
class IDataCache;
class IMDLCache;
class IAvi;

namespace vgui
{
	class ISurface;
	class IVGui;
	class IPanel;
}


//-----------------------------------------------------------------------------
// These tier3 libraries must be set by any users of this library.
// They can be set by calling ConnectTier3Libraries.
// It is hoped that setting this, and using this library will be the common mechanism for
// allowing link libraries to access tier3 library interfaces
//-----------------------------------------------------------------------------
extern IStudioRender *g_pStudioRender;
extern IMatSystemSurface *g_pMatSystemSurface;
extern vgui::ISurface *g_pVGuiSurface;
extern vgui::IVGui *g_pVGui;
extern vgui::IPanel *g_pVGuiPanel;
extern IDataCache *g_pDataCache;	// FIXME: Should IDataCache be in tier2?
extern IMDLCache *g_pMDLCache;
extern IAvi *g_pAVI;


//-----------------------------------------------------------------------------
// Call this to connect to/disconnect from all tier 3 libraries.
// It's up to the caller to check the globals it cares about to see if ones are missing
//-----------------------------------------------------------------------------
void ConnectTier3Libraries( CreateInterfaceFn *pFactoryList, int nFactoryCount );
void DisconnectTier3Libraries();


//-----------------------------------------------------------------------------
// Helper empty implementation of an IAppSystem for tier2 libraries
//-----------------------------------------------------------------------------
template< class IInterface, int ConVarFlag = 0 > 
class CTier3AppSystem : public CTier2AppSystem< IInterface, ConVarFlag >
{
	typedef CTier2AppSystem< IInterface, ConVarFlag > BaseClass;

public:
	CTier3AppSystem( bool bIsPrimaryAppSystem = true ) : BaseClass(	bIsPrimaryAppSystem )
	{
	}

	virtual bool Connect( CreateInterfaceFn factory ) 
	{
		if ( !BaseClass::Connect( factory ) )
			return false;

		if ( IsPrimaryAppSystem() )
		{
			ConnectTier3Libraries( &factory, 1 );
		}
		return true;
	}

	virtual void Disconnect() 
	{
		if ( IsPrimaryAppSystem() )
		{
			DisconnectTier3Libraries();
		}
		BaseClass::Disconnect();
	}
};


#endif // TIER3_H

