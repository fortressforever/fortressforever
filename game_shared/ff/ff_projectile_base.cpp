/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_base.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief All projectiles derived from here; takes advantage of base_grenade code
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation


#include "cbase.h"
#include "ff_projectile_base.h"

#ifdef GAME_DLL
	#include "ff_player.h"
	#include "soundent.h"
	#include "util.h"
#else
	#include "c_ff_player.h"
#endif 

extern short	g_sModelIndexFireball;		// (in combatweapon.cpp) holds the index for the fireball 
extern short	g_sModelIndexWExplosion;	// (in combatweapon.cpp) holds the index for the underwater explosion
extern short	g_sModelIndexSmoke;			// (in combatweapon.cpp) holds the index for the smoke cloud

//=============================================================================
// CFFProjectileBase tables
//=============================================================================

#ifdef GAME_DLL
	BEGIN_DATADESC(CFFProjectileBase) 
	END_DATADESC() 
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFProjectileBase, DT_FFProjectileBase) 

BEGIN_NETWORK_TABLE(CFFProjectileBase, DT_FFProjectileBase) 
	#ifdef CLIENT_DLL
		RecvPropVector(RECVINFO(m_vInitialVelocity)) 
	#else
		SendPropVector(SENDINFO(m_vInitialVelocity), 
			20, 		// nbits
			0, 		// flags
			-3000, 	// low value
			3000	// high value
			) 
	#endif
END_NETWORK_TABLE() 

//=============================================================================
// CFFProjectileBase implementation
//=============================================================================

#ifdef CLIENT_DLL

	//----------------------------------------------------------------------------
	// Purpose: Client constructor
	//----------------------------------------------------------------------------
	CFFProjectileBase::CFFProjectileBase()
	{
		m_fNeedsEngineSoundAttached = false;
	}

	//----------------------------------------------------------------------------
	// Purpose: Add initial velocity into the interpolation history so that
	//			interp works okay
	//----------------------------------------------------------------------------
	void CFFProjectileBase::PostDataUpdate(DataUpdateType_t type) 
	{
		BaseClass::PostDataUpdate(type);

		if (type == DATA_UPDATE_CREATED) 
		{
			// Now stick our initial velocity into the interpolation history 
			CInterpolatedVar< Vector > &interpolator = GetOriginInterpolator();
			
			interpolator.ClearHistory();
			float changeTime = GetLastChangeTime(LATCH_SIMULATION_VAR);

			// Add a sample 1 second back.
			//VOOGRU: Taken this out due to #0000706
			/*
			Vector vCurOrigin = GetLocalOrigin() - m_vInitialVelocity;
			interpolator.AddToHead(changeTime - 1.0f, &vCurOrigin, false);
			*/

			// Add the current sample.
			Vector vCurOrigin = GetLocalOrigin();
			interpolator.AddToHead(changeTime, &vCurOrigin, false);

			// This projectile has entered the client's PVS so flag it as needing
			// a sound attached. We can't directly start the sound here because
			// the entity is not yet ready for queries (eg. GetAbsOrigin())
			if (GetFlightSound())
				m_fNeedsEngineSoundAttached = true;
		}
	}

	//----------------------------------------------------------------------------
	// Purpose: 
	//----------------------------------------------------------------------------
	int CFFProjectileBase::DrawModel(int flags) 
	{
		// Just putting this here for now.. not the best place for it admittedly
		if (m_fNeedsEngineSoundAttached)
		{
			EmitSound(GetFlightSound());
			m_fNeedsEngineSoundAttached = false;
		}

		return BaseClass::DrawModel(flags);
	}

	//----------------------------------------------------------------------------
	// Purpose: Entity is being released, so stop the sound effect
	//----------------------------------------------------------------------------
	void CFFProjectileBase::Release()
	{
		if (GetFlightSound())
			StopSound(GetFlightSound());

		BaseClass::Release();
	}

#else

	//----------------------------------------------------------------------------
	// Purpose: Specify what velocity we want to have on the client immediately.
	//			Without this, the entity wouldn't have an interpolation history initially, so it would
	//			sit still until it had gotten a few updates from the server.
	//
	//			[FIXME] The problem with this is that the projectiles now appear
	//					further back than their actual origin
	//----------------------------------------------------------------------------
	void CFFProjectileBase::SetupInitialTransmittedVelocity(const Vector &velocity) 
	{
		m_vInitialVelocity = velocity;
	}

	//----------------------------------------------------------------------------
	// Purpose: If caught in an emp then destroy
	//----------------------------------------------------------------------------
	int CFFProjectileBase::TakeEmp() 
	{
		UTIL_Remove(this);
		return m_flDamage;
	}

#endif

//----------------------------------------------------------------------------
// Purpose: Keep track of when spawned
//----------------------------------------------------------------------------
void CFFProjectileBase::Spawn() 
{
	m_flSpawnTime = gpGlobals->curtime;
	m_flNextBounceSoundTime = gpGlobals->curtime;

	BaseClass::Spawn();

	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
}

//----------------------------------------------------------------------------
// Purpose: Precache the bounce and flight sounds
//----------------------------------------------------------------------------
void CFFProjectileBase::Precache() 
{
	PrecacheScriptSound(GetBounceSound());

	if (GetFlightSound())
		PrecacheScriptSound(GetFlightSound());
}

//----------------------------------------------------------------------------
// Purpose: Play a bounce sound with a minimum interval
//----------------------------------------------------------------------------
void CFFProjectileBase::BounceSound()
{
	if (gpGlobals->curtime > m_flNextBounceSoundTime && GetAbsVelocity().LengthSqr() > 1)
	{
		EmitSound(GetBounceSound());

		m_flNextBounceSoundTime = gpGlobals->curtime + 0.1;
	}	
}