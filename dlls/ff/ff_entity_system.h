/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_entity_system.h
/// @author Gavin Bramhill (Mirvin_Monkey)
/// @date 21 April 2005
/// @brief Handles the entity system
///
/// REVISIONS
/// ---------
/// Apr 21, 2005 Mirv: Begun

#ifndef FF_ENTITY_SYSTEM_H
#define FF_ENTITY_SYSTEM_H

#ifdef _WIN32
#pragma once
#endif

class CFFLuaSC;

/////////////////////////////////////////////////////////////////////////////
// CFFEntitySystemHelper
/////////////////////////////////////////////////////////////////////////////
class CFFEntitySystemHelper : public CBaseEntity
{
	DECLARE_CLASS( CFFEntitySystemHelper, CBaseEntity );
	DECLARE_DATADESC();

public:
	// 'structors
	CFFEntitySystemHelper();
	virtual ~CFFEntitySystemHelper();

public:
	// CBaseEntity
	void Spawn();
	void OnThink();
	void Precache();

public:
	// spawns an instance of this type
	static CFFEntitySystemHelper* Create();

	// there can only be one!!
	static CFFEntitySystemHelper* GetInstance();

private:
	static CFFEntitySystemHelper* s_pInstance;
};

/////////////////////////////////////////////////////////////////////////////
// utility functions
bool FFScriptRunPredicates( CBaseEntity *pEntity, const char *pszFunction, bool bExpectedVal );
bool FFScriptRunPredicates( CBaseEntity *pEntity, const char *pszFunction, bool bExpectedVal, Vector vecOrigin, Vector vecMins = Vector(-16,-16,-16), Vector vecMaxs = Vector(16,16,16) ); // UTIL_EntitiesInBox
bool FFScriptRunPredicates( CBaseEntity *pEntity, const char *pszFunction, bool bExpectedVal, Vector vecOrigin, float flRadius = 16 ); // UTIL_EntitiesInSphere
bool FFScriptRunPredicates( CFFLuaSC *pContext, const char *pszFunction, bool bExpectedVal, Vector vecOrigin, float flRadius = 16 ); // UTIL_EntitiesInSphere

/////////////////////////////////////////////////////////////////////////////
#endif // FF_ENTITY_SYSTEM_H
