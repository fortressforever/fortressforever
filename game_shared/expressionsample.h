//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef EXPRESSIONSAMPLE_H
#define EXPRESSIONSAMPLE_H
#ifdef _WIN32
#pragma once
#endif

struct CExpressionSample
{
	// Height
	float			value;
	// time from start of event
	float			time;

	bool			selected;
};

#endif // EXPRESSIONSAMPLE_H
