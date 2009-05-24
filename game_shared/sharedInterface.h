//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Exposes client-server neutral interfaces implemented in both places
//
// $NoKeywords: $
//=============================================================================//

#ifndef SHAREDINTERFACE_H
#define SHAREDINTERFACE_H

class IFileSystem;
class IUniformRandomStream;
class IEngineSound;
class IMapData;

extern IFileSystem				*filesystem;
#if defined(_STATIC_LINKED) && defined(_SUBSYSTEM) && (defined(CLIENT_DLL) || defined(GAME_DLL))
namespace _SUBSYSTEM
{
extern IUniformRandomStream		*random;
}
#else
extern IUniformRandomStream		*random;
#endif
extern IEngineSound				*enginesound;
extern IMapData					*g_pMapData;			

#endif // SHAREDINTERFACE_H

