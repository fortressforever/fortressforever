//=============== Fortress Forever ===============
//======== A modification for Half-Life 2 ========
/**
@file ff_grenade_base.h
@brief Declaration of the base class for all primeable grenades

All primeable grenades in the game inherit from this class. These grenades include:
- Frag
- Caltrops
- Concussion
- Nail
- MIRV
- Napalm
- Gas
- EMP

@author Shawn Smith (L0ki)
@date Dec. 10, 2004

@revision <dl compact><dt><u>Dec. 10, 2004</u></dt><dd>L0ki: <i>Initial Creation</i></dd></dl>
@revision <dl compact><dt><u>Apr. 20, 2005</u></dt><dd>L0ki: <i>Reorganized code structure</i></dd></dl>
/// Jan. 15, 2006   Mirv: Tidied this up a LOT!
*/

#ifndef FF_GRENADE_BASE_H
#define FF_GRENADE_BASE_H

#include "ff_projectile_base.h"

#ifdef CLIENT_DLL
	#define CFFGrenadeBase C_FFGrenadeBase
#endif

//========================================================================
// Development convers
//========================================================================
#ifdef GAME_DLL
	extern ConVar gren_radius;
	extern ConVar gren_grav;
	extern ConVar gren_fric;
	extern ConVar gren_elas;
#endif

//=============================================================================
// CFFGrenadeBase class declaration
//=============================================================================
class CFFGrenadeBase : public CFFProjectileBase
{
public:
	DECLARE_CLASS( CFFGrenadeBase, CFFProjectileBase );

	virtual void Precache();
	virtual Class_T Classify( void ) { return CLASS_GREN; }
	virtual void Explode( trace_t *pTrace, int bitsDamageType );

#ifdef GAME_DLL
	virtual float GetGrenadeGravity() { return gren_grav.GetFloat(); }
	virtual float GetGrenadeFriction() { return gren_fric.GetFloat(); }
	virtual float GetGrenadeElasticity() { return gren_elas.GetFloat(); }
	virtual float GetGrenadeDamage() { return 180.0f; }
	virtual float GetGrenadeRadius() { return GetGrenadeDamage() * 1.5f; }	

	bool m_fIsHandheld;	// This will eventually need to be on the client too
#endif

	static int m_iShockwaveTexture;
	static int m_iFlameSprite;

#ifdef CLIENT_DLL
	CFFGrenadeBase() {}
	CFFGrenadeBase( const CFFGrenadeBase& ) {}

	int DrawModel(int flags) 
	{
		return CBaseGrenade::DrawModel(flags);
	}

#else

	DECLARE_DATADESC(); // Since we're adding new thinks etc

	virtual void Spawn( void );
	virtual void PreExplode( trace_t *pTrace );
	virtual void PreExplode( trace_t *pTrace, const char* pSound, const char *pEffect );
	virtual void PostExplode( void );
	virtual void GrenadeThink( void );
	virtual void WaterThink( void );	// |-- Mulch: Not a real think func

	void SetDetonateTimerLength( float timer );

	// BaseClass projectile was making grens use its takeemp which just removed
	// them from the level
	// Bug #0000527: Emps makes other grenades disappear instead of detonating them.
	int TakeEmp( void ) { return 0; }

	// Public members
	CNetworkVector( m_vInitialVelocity );
protected:

	//Custom collision to allow for constant elasticity on hit surfaces
	virtual void ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity );

	float	m_flDetonateTime;

	bool	m_bHitwater;
	float	m_flHitwaterTimer;

#endif
};

#endif //FF_GRENADE_BASE_H
