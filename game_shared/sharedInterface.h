//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
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
extern IUniformRandomStream		*random;
extern IEngineSound				*enginesound;
extern IMapData					*g_pMapData;			// TODO: current implementations of the 
														// interface are in TF2, should probably move
														// to TF2/HL2 neutral territory

#endif // SHAREDINTERFACE_H

