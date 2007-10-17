/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file ff_grenade_normal.cpp
/// @author Shawn Smith (L0ki)
/// @date Jan. 29, 2005
/// @brief normal grenade class
/// 
/// Implementation of the CFFGrenadeNormal class. This is the primary grenade type for all classes except scout
/// 
/// Jan. 29, 2005	L0ki: Initial Creation
/// Apr. 23, 2005	L0ki: removed header file, moved everything to a single cpp file
/// Jan. 15, 2006   Mirv: Tidied this up a LOT!

#include "cbase.h"
#include "ff_grenade_base.h"

#define NORMALGRENADE_MODEL "models/grenades/frag/frag.mdl"

#ifdef CLIENT_DLL
	#define CFFGrenadeNormal C_FFGrenadeNormal
#endif

class CFFGrenadeNormal : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeNormal,CFFGrenadeBase)

	virtual void Precache();
	virtual void Spawn();
	virtual const char *GetBounceSound() { return "NormalGrenade.Bounce"; }

	// Jiggles: Lowered Frag damage from 180 to 126 (30% less), but kept the radius about the same
#ifdef GAME_DLL
	virtual float GetGrenadeDamage()		{ return 126.0f; }
	virtual float GetGrenadeRadius()		{ return GetGrenadeDamage() * 2.15f/*1.5f*/; }
#endif

#ifdef CLIENT_DLL
	CFFGrenadeNormal() {}
	CFFGrenadeNormal(const CFFGrenadeNormal &) {}
#endif
};

LINK_ENTITY_TO_CLASS(ff_grenade_normal, CFFGrenadeNormal);
PRECACHE_WEAPON_REGISTER(ff_grenade_normal);

//-----------------------------------------------------------------------------
// Purpose: Set model
//-----------------------------------------------------------------------------
void CFFGrenadeNormal::Spawn()
{
	SetModel(NORMALGRENADE_MODEL);

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Precache model
//-----------------------------------------------------------------------------
void CFFGrenadeNormal::Precache()
{
	PrecacheModel(NORMALGRENADE_MODEL);

	BaseClass::Precache();
}