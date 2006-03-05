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

#define NORMALGRENADE_MODEL "models/weapons/w_eq_fraggrenade_thrown.mdl"

#ifdef CLIENT_DLL
	#define CFFGrenadeNormal C_FFGrenadeNormal
#endif

class CFFGrenadeNormal : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeNormal,CFFGrenadeBase)

	CNetworkVector(m_vInitialVelocity);

	virtual void Precache();
	virtual void Spawn();
	virtual const char *GetBounceSound() { return "NormalGrenade.Bounce"; }

#ifdef CLIENT_DLL
	CFFGrenadeNormal() {}
	CFFGrenadeNormal(const CFFGrenadeNormal &) {}
#endif
};

LINK_ENTITY_TO_CLASS(normalgrenade, CFFGrenadeNormal);
PRECACHE_WEAPON_REGISTER(normalgrenade);

void CFFGrenadeNormal::Spawn()
{
	SetModel(NORMALGRENADE_MODEL);

	BaseClass::Spawn();
}
void CFFGrenadeNormal::Precache()
{
	PrecacheModel(NORMALGRENADE_MODEL);

	BaseClass::Precache();
}