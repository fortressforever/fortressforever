//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef TGAWRITER_H
#define TGAWRITER_H
#pragma once


#include "imageloader.h"
class CUtlBuffer;

namespace TGAWriter
{

bool Write( unsigned char *pImageData, const char *fileName, int width, int height, 
            enum ImageFormat srcFormat, enum ImageFormat dstFormat );

bool WriteToBuffer( unsigned char *pImageData, CUtlBuffer &buffer, int width, int height, 
            enum ImageFormat srcFormat, enum ImageFormat dstFormat );

#ifndef TGAWRITER_USE_FOPEN
bool SetFileSystem( CreateInterfaceFn fileSystemFactory );
#endif

} // end namespace TGAWriter

#endif // TGAWRITER_H
