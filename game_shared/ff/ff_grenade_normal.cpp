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
	DECLARE_CLASS(CFFGrenadeNormal,CFFGrenadeBase);
	DECLARE_NETWORKCLASS(); 

	virtual void Precache();
	virtual void Spawn();
	virtual const char *GetBounceSound() { return "NormalGrenade.Bounce"; }

#ifdef GAME_DLL
	virtual float GetGrenadeDamage()		{ return FRAG_GREN_DAMAGE; }
	virtual float GetGrenadeRadius()		{ return FRAG_GREN_RADIUS; }
	virtual float GetGrenadeFallOff()		{ return 0.25f; }
#endif

	virtual color32 GetColour() { color32 col = { 255, 64, 64, GREN_ALPHA_DEFAULT }; return col; }

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