//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef IVTEX_H
#define IVTEX_H
#ifdef _WIN32
#pragma once
#endif

#include "interface.h"


// This shipped with HL2.
class IVTex
{
public:
	virtual int VTex( int argc, char **argv ) = 0;
};

#define IVTEX_VERSION_STRING "VTEX001"


// This is what we 
class IVTex2
{
public:
	virtual int VTex( IFileSystem *pFSInherit, const char *pGameDir, int argc, char **argv ) = 0;
};

#define IVTEX2_VERSION_STRING "VTEX2_001"


#endif // IVTEX_H
