//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "rangecheckedvar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

bool g_bDoRangeChecks = true;


static int g_nDisables = 0;


CDisableRangeChecks::CDisableRangeChecks()
{
	g_nDisables++;
	g_bDoRangeChecks = false;
}


CDisableRangeChecks::~CDisableRangeChecks()
{
	Assert( g_nDisables > 0 );
	--g_nDisables;
	if ( g_nDisables == 0 )
	{
		g_bDoRangeChecks = true;
	}
}




