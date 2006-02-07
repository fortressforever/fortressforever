//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef CACHE_USER_H
#define CACHE_USER_H

#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: Engine cache_user_t block.  This pointer goes to NULL if the engine 
//  needs to bump the object out of the heap/cache.
// DO NOT CHANGE
//-----------------------------------------------------------------------------
struct cache_user_t
{
	void *data;
};

#endif // CACHE_USER_H
