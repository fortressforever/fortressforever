// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_info_script.h
// @author Patrick O'Leary (Mulchman)
// @date 07/13/2006
// @brief info_ff_script, the C++ class
//
// REVISIONS
// ---------
// 07/13/2006, Mulchman: 
//		This file First created
//		Rewriting "ff_item_flag"
//		CFF_InfoScript name will change once complete

#ifndef FF_INFO_SCRIPT_H
#define FF_INFO_SCRIPT_H

#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL 	
#define CFF_InfoScript C_FF_InfoScript
	#include "c_baseanimating.h"
#else
	#include "baseanimating.h"
#endif

//=============================================================================
//
// Class CFF_InfoScript
//
//=============================================================================
class CFF_InfoScript : public CBaseAnimating
{
public:
	DECLARE_CLASS( CFF_InfoScript, CBaseAnimating );
	DECLARE_NETWORKCLASS();

	// --> Shared
	CFF_InfoScript( void );
	~CFF_InfoScript( void );

	virtual void	Precache( void );
	virtual void	Spawn( void );
	// <-- Shared

#ifdef CLIENT_DLL 
#else
	DECLARE_DATADESC();
#endif // CLIENT_DLL
};

//-----------------------------------------------------------------------------
// Purpose: Create the laser dot
//-----------------------------------------------------------------------------

#endif // FF_INFO_SCRIPT_H
