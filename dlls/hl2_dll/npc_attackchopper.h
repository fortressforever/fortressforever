//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//


#ifndef NPC_ATTACKCHOPPER_H
#define NPC_ATTACKCHOPPER_H

#ifdef _WIN32
#pragma once
#endif


//-----------------------------------------------------------------------------
// Creates an avoidance sphere
//-----------------------------------------------------------------------------
CBaseEntity *CreateHelicopterAvoidanceSphere( CBaseEntity *pParent, int nAttachment, float flRadius, bool bAvoidBelow = false );


#endif // NPC_ATTACKCHOPPER_H


