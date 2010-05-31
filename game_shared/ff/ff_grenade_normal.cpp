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

// these are for player and team info
#ifdef CLIENT_DLL
	#include "c_ff_player.h"
#else
	#include "ff_player.h"
#endif

#define NORMALGRENADE_MODEL "models/grenades/frag/frag.mdl"

#ifdef CLIENT_DLL
	#define CFFGrenadeNormal C_FFGrenadeNormal
#endif

class CFFGrenadeNormal : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeNormal,CFFGrenadeBase);
	DECLARE_NETWORKCLASS(); 

	virtual void Precache();
	virtual void Spawn();
	virtual const char *GetBounceSound() { return "NormalGrenade.Bounce"; }

	// Jiggles: Lowered Frag damage from 180 to 126 (30% less), but kept the radius about the same
	// AfterShock: back up to 145, 30% was too much. Radius still the same, and separated the values.
#ifdef GAME_DLL
	virtual float GetGrenadeDamage()		{ return 145.0f; }
	virtual float GetGrenadeRadius()		{ return 270.0f; }
#endif

//	virtual color32 GetColour()  { color32 col = { 255, 64, 64, GREN_ALPHA_DEFAULT }; return col; }
//	virtual color32 GetColour()  { return BaseClass::GetColour(); }

#ifdef CLIENT_DLL
	CFFGrenadeNormal() {}
	CFFGrenadeNormal(const CFFGrenadeNormal &) {}
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeNormal, DT_FFGrenadeNormal)

BEGIN_NETWORK_TABLE(CFFGrenadeNormal, DT_FFGrenadeNormal)
END_NETWORK_TABLE()

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