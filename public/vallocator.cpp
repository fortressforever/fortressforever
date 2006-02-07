//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include <malloc.h>
#include "vallocator.h"
#include "basetypes.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

VStdAllocator g_StdAllocator;


void* VStdAllocator::Alloc(unsigned long size)
{
	if(size)
	{
		void *ret = malloc(size);
		return ret;
	}
	else
		return 0;
}


void VStdAllocator::Free(void *ptr)
{
	free(ptr);
}

