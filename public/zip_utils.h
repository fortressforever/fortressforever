//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef ZIP_UTILS_H
#define ZIP_UTILS_H
#ifdef _WIN32
#pragma once
#endif

class CUtlBuffer;

class IZip
{
public:
	virtual void		Reset() = 0;

	// Add buffer to zip as a file with given name
	virtual void		AddBufferToZip( const char *relativename, void *data, int length, bool bTextMode ) = 0;

	virtual void		SaveToBuffer( CUtlBuffer& outbuf ) = 0;

	virtual void		ParseFromBuffer( unsigned char *buffer, int bufferlength ) = 0;

};

extern IZip *zip_utils;

#endif // ZIP_UTILS_H
