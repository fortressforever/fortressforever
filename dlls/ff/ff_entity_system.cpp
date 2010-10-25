/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_entity_system.cpp
/// @author Gavin Bramhill (Mirvin_Monkey)
/// @date 21 April 2005
/// @brief Handles the entity system
///
/// REVISIONS
/// ---------
/// Apr 21, 2005 Mirv: Begun
/// Jun 13, 2005 FryGuy: Added AddTeamScore
/// Jun 25, 2005 FryGuy: Added SpawnEntityAtPlayer, GetPlayerClass, GetPlayerName
/// Jun 29, 2005 FryGuy: Added PlayerHasItem, RemoveItem
/// Jul 10, 2005 FryGuy: Added ReturnItem
/// Jul 15, 2005 FryGuy: Changed ReturnItem to use a string instead of entity ID
/// Jul 31, 2005 FryGuy: Added the entity helper, along with the sound stuffs
/// Aug 01, 2005 FryGuy: Added BroadcastMessage and RespawnAllPlayers

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "ff_entity_system.h"
#include "ff_luacontext.h"
#include "ff_scheduleman.h"
#include "ff_timerman.h"
#include "ff_menuman.h"
#include "ff_scriptman.h"
#include "ff_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/////////////////////////////////////////////////////////////////////////////
// CFFEntitySystemHelper implementation
/////////////////////////////////////////////////////////////////////////////
LINK_ENTITY_TO_CLASS( ff_entity_system_helper, CFFEntitySystemHelper );

// Start of our data description for the class
BEGIN_DATADESC( CFFEntitySystemHelper )
	DEFINE_THINKFUNC( OnThink ),
END_DATADESC()

CFFEntitySystemHelper* CFFEntitySystemHelper::s_pInstance = NULL;

/////////////////////////////////////////////////////////////////////////////
// Purpose: Sets up the entity's initial state
/////////////////////////////////////////////////////////////////////////////
CFFEntitySystemHelper::CFFEntitySystemHelper()
{
	ASSERT(!s_pInstance);
	s_pInstance = this;
}

/////////////////////////////////////////////////////////////////////////////
CFFEntitySystemHelper::~CFFEntitySystemHelper()
{
	s_pInstance = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CFFEntitySystemHelper* CFFEntitySystemHelper::GetInstance()
{
	ASSERT(s_pInstance);
	return s_pInstance;
}

/////////////////////////////////////////////////////////////////////////////
void CFFEntitySystemHelper::Spawn()
{
	Msg("[EntSys] Entity System Helper Spawned\n");

	SetThink( &CFFEntitySystemHelper::OnThink );
	SetNextThink( gpGlobals->curtime + 1.0f );
}

/////////////////////////////////////////////////////////////////////////////
void CFFEntitySystemHelper::OnThink()
{
	VPROF_BUDGET( "CFFEntitySystemHelper::OnThink", VPROF_BUDGETGROUP_FF_LUA );

	_scheduleman.Update();
	//_menuman.Update();
	_timerman.Update();
	SetNextThink(gpGlobals->curtime + TICK_INTERVAL);
}

/////////////////////////////////////////////////////////////////////////////
void CFFEntitySystemHelper::Precache()
{
	VPROF_BUDGET( "CFFEntitySystemHelper::Precache", VPROF_BUDGETGROUP_FF_LUA );

	CFFLuaSC hPrecache;
	_scriptman.RunPredicates_LUA( NULL, &hPrecache, "precache" );
}

/////////////////////////////////////////////////////////////////////////////
CFFEntitySystemHelper* CFFEntitySystemHelper::Create()
{
	CFFEntitySystemHelper* pHelper = (CFFEntitySystemHelper*)CreateEntityByName("ff_entity_system_helper");
	pHelper->Spawn();
	pHelper->Precache();
	return pHelper;
}
