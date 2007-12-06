// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_info_script.cpp
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

#include "cbase.h"
#include "ff_info_script.h"

#ifdef CLIENT_DLL 	
#else
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
//
// Class CFF_InfoScript
//
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED( FF_InfoScript, DT_FF_InfoScript ) 

BEGIN_NETWORK_TABLE( CFF_InfoScript, DT_FF_InfoScript )
#ifdef CLIENT_DLL 
#else
#endif // CLIENT_DLL
END_NETWORK_TABLE() 

LINK_ENTITY_TO_CLASS( ff_info_ff_script, CFF_InfoScript );
PRECACHE_REGISTER( ff_info_ff_script );

#ifdef GAME_DLL
// Datatable
BEGIN_DATADESC( CFF_InfoScript )
END_DATADESC()
#endif // GAME_DLL

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFF_InfoScript::CFF_InfoScript( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CFF_InfoScript::~CFF_InfoScript( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Precache
//-----------------------------------------------------------------------------
void CFF_InfoScript::Precache( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Spawn
//-----------------------------------------------------------------------------
void CFF_InfoScript::Spawn( void )
{
}
