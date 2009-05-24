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

#ifndef FF_GOAL_H
#define FF_GOAL_H

#include "cbase.h"

class CFFGoal : public CLogicalEntity
{
public:
	DECLARE_CLASS( CFFGoal , CLogicalEntity );
	DECLARE_DATADESC();

	CFFGoal();

	void FireOutput();

private:

	COutputEvent	m_OnOutput;
};

#endif