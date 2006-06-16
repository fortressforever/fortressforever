/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_grenade_nail.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF nail projectile code.
///
/// REVISIONS
/// ---------
/// Mar 22, 2005	Mirv: First created
/// Apr. 23, 2005	L0ki: removed header file, moved everything to a single cpp file

//========================================================================
// Required includes
//========================================================================
#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"
#include "ff_projectile_nail.h"

#define NAILGRENADE_MODEL "models/grenades/nailgren/nailgren.mdl"

#ifdef CLIENT_DLL
	#define CFFGrenadeNail C_FFGrenadeNail
#endif

class CFFGrenadeNail : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeNail, CFFGrenadeBase) 

	CNetworkVector(m_vInitialVelocity);

	virtual void Precache();
	virtual const char *GetBounceSound() { return "NailGrenade.Bounce"; }

	virtual Class_T Classify( void ) { return CLASS_GREN_NAIL; } 

#ifdef CLIENT_DLL
	CFFGrenadeNail() {}
	CFFGrenadeNail(const CFFGrenadeNail&) {}
#else
	DECLARE_DATADESC(); // Since we're adding new thinks etc
	virtual void Spawn();
	virtual void NailEmit();
	virtual void GrenadeThink();

protected:
	float m_flNailSpit;
#endif
};

#ifdef GAME_DLL
	BEGIN_DATADESC(CFFGrenadeNail) 
		DEFINE_THINKFUNC(GrenadeThink), 
		DEFINE_THINKFUNC(NailEmit), 
	END_DATADESC() 
#endif

LINK_ENTITY_TO_CLASS(nailgrenade, CFFGrenadeNail);
PRECACHE_WEAPON_REGISTER(nailgrenade);

#ifndef CLIENT_DLL
	ConVar nailspeed("ffdev_nailspeed", "800");
	ConVar nailgren_spittime( "ffdev_nailgren_spittime", "0.4" );
#endif

#ifdef GAME_DLL

	void CFFGrenadeNail::Spawn() 
	{
		DevMsg("[Grenade Debug] CFFGrenadeNail\n");
		SetModel(NAILGRENADE_MODEL);
		BaseClass::Spawn();

		m_flNailSpit = 0.0f;
		SetLocalAngularVelocity(QAngle(0, 0, 0));
	}

	void CFFGrenadeNail::GrenadeThink() 
	{
		// Remove if we're nolonger in the world
		if (!IsInWorld()) 
		{
			Remove();
			return;
		}

		// Blow up if we've reached the end of our fuse
		if (gpGlobals->curtime > m_flDetonateTime) 
		{
			DevMsg("[Grenade Debug] CFFGrenadeNail::GrenadeThink\n[Grenade Debug] Changing to nail mode\n");

			// Reset the detonation time
			SetDetonateTimerLength(3);

			// Should this maybe be noclip?
			SetMoveType(MOVETYPE_FLY);

			// Go into nail mode
			SetThink(&CFFGrenadeNail::NailEmit);		// |-- Mirv: Account for GCC strictness
			SetNextThink(gpGlobals->curtime);

			return;
		}

		// Next think straight away
		SetNextThink(gpGlobals->curtime);

		CFFGrenadeBase::WaterThink(); // Mulch: bug 0000273: make grens sink-ish in water
	}

	void CFFGrenadeNail::NailEmit() 
	{
		// Blow up if we've reached the end of our fuse
		if (gpGlobals->curtime > m_flDetonateTime) 
		{
			DevMsg("[Grenade Debug] CFFGrenadeNail::NailEmit\n[Grenade Debug] Detonating\n");
			Detonate();
			return;
		}

		float risingheight = 0;

		// Lasts for 3 seconds, rise for 0.3, but only if not handheld
		if (m_flDetonateTime - gpGlobals->curtime > 2.6 && !m_fIsHandheld)
			risingheight = 80;

		SetAbsVelocity(Vector(0, 0, risingheight + 20 * sin(DEG2RAD(GetAbsAngles().y))));

		SetAbsAngles(GetAbsAngles() + QAngle(0, 15, 0));

		// Bug #0000674: Nail grenade doesn't shoot nails out like TFC nail grenade

		// Time to spit out nails again?
		if( m_flNailSpit < gpGlobals->curtime )
		{
			// Do the classic TFC pattern
			for( int i = 0; i < 11; i++ )
			{
				Vector vecNailDir;
				QAngle vecAngles = GetAbsAngles() + QAngle( 0, 30.0f * i, 0 );				
				AngleVectors( vecAngles, &vecNailDir );
				VectorNormalizeFast( vecNailDir );

				CFFProjectileNail::CreateNail( GetAbsOrigin() + ( 8.0f * vecNailDir ), vecAngles, GetOwnerEntity(), 30, nailspeed.GetInt() );
			}
			
			EmitSound( "NailGrenade.shoot" );

			// Set up next nail spit time
			m_flNailSpit = gpGlobals->curtime + nailgren_spittime.GetFloat();
		}

		SetNextThink(gpGlobals->curtime);
	}

#endif

void CFFGrenadeNail::Precache() 
{
	DevMsg("[Grenade Debug] CFFGrenadeNail::Precache\n");
	PrecacheModel(NAILGRENADE_MODEL);
	PrecacheScriptSound( "NailGrenade.shoot" );
	BaseClass::Precache();
}
