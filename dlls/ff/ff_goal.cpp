/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file \Steam\SteamApps\SourceMods\FortressForeverCode\dlls\ff\ff_goal.cpp
/// @author Kevin Hjelden (FryGuy)
/// @date Aug. 27, 2005
/// @brief Flag Item (generic lua entity)
/// 
/// Implements a basic goal entity for use in the entity system (lua)
/// 
/// Revisions
/// ---------
/// Aug. 27, 2005	FryGuy: Initial Creation

#include "cbase.h"
#include "ff_goal.h"

BEGIN_DATADESC( CFFGoal )
	DEFINE_OUTPUT( m_OnOutput, "OnOutput" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( ff_goal, CFFGoal  );
PRECACHE_REGISTER( ff_goal );

CFFGoal::CFFGoal()
{
}

void CFFGoal::FireOutput()
{
	m_OnOutput.FireOutput( this, this );
}