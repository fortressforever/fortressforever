//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include <stdio.h>
#include "basetypes.h"
#include "pacifier.h"
#include "tier0/dbg.h"


static int g_LastPacifierDrawn = -1;


void StartPacifier( char const *pPrefix )
{
	Msg( "%s", pPrefix );
	g_LastPacifierDrawn = -1;
	UpdatePacifier( 0.001f );
}

void UpdatePacifier( float flPercent )
{
	int iCur = (int)(flPercent * 40.0f);
	iCur = clamp( iCur, g_LastPacifierDrawn, 40 );
	
	if( iCur != g_LastPacifierDrawn )
	{
		for( int i=g_LastPacifierDrawn+1; i <= iCur; i++ )
		{
			if ( !( i % 4 ) )
			{
				Msg("%d", i/4);
			}
			else
			{
				if( i != 40 )
				{
					Msg(".");
				}
			}
		}
		
		g_LastPacifierDrawn = iCur;
	}
}

void EndPacifier( bool bCarriageReturn )
{
	UpdatePacifier(1);
	
	if( bCarriageReturn )
		Msg("\n");
}
