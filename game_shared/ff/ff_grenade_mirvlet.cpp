/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file ff_grenade_mirvlet.cpp
/// @author Shawn Smith (L0ki)
/// @date Jan. 30, 2005
/// @brief mirvlet class
/// 
/// Implementation of the CFFgrenadeMirvlet class. These are the smaller "grenades" that explode from the MIRV grenade.
/// 
/// Revisions
/// ---------
/// Jan. 30, 2005	L0ki: Initial Creation
/// Apr. 23, 2005	L0ki: removed header file, moved everything to a single cpp file

#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"

#define MIRVLET_MODEL "models/grenades/mirv/mirvlet.mdl"

#ifdef CLIENT_DLL
	#define CFFGrenadeMirvlet C_FFGrenadeMirvlet
#endif

class CFFGrenadeMirvlet : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeMirvlet,CFFGrenadeBase)

	virtual void Precache();
	virtual const char *GetBounceSound() { return "MirvletGrenade.Bounce"; }

#ifdef CLIENT_DLL
	CFFGrenadeMirvlet() {}
	CFFGrenadeMirvlet( const CFFGrenadeMirvlet& ) {}
#else
	virtual void Spawn();
#endif
};

LINK_ENTITY_TO_CLASS( mirvlet, CFFGrenadeMirvlet);
PRECACHE_WEAPON_REGISTER( mirvlet );

#ifdef GAME_DLL
	//-----------------------------------------------------------------------------
	// Purpose: Spawn stuff
	//-----------------------------------------------------------------------------
	void CFFGrenadeMirvlet::Spawn( void )
	{
		SetModel( MIRVLET_MODEL );
		BaseClass::Spawn();
	}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFGrenadeMirvlet::Precache()
{
	PrecacheModel( MIRVLET_MODEL );
	BaseClass::Precache();
}