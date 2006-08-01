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
		RecvPropVector(RECVINFO(m_vecInitialVelocity)) 
	#else
		SendPropVector(SENDINFO(m_vecInitialVelocity), 
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
		m_bNeedsCleanup = true;
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
			// Store start origin
			m_vecStartOrigin = GetLocalOrigin();

			// Now stick our initial velocity into the interpolation history 
			CInterpolatedVar< Vector > &interpolator = GetOriginInterpolator();
			
			interpolator.ClearHistory();
			float changeTime = GetLastChangeTime(LATCH_SIMULATION_VAR);

			// Add a sample 1 second back.
			Vector vecCurOrigin = GetLocalOrigin() - m_vecInitialVelocity;
			interpolator.AddToHead(changeTime - 1.0f, &vecCurOrigin, false);

			// Add the current sample.
			vecCurOrigin = GetLocalOrigin();
			interpolator.AddToHead(changeTime, &vecCurOrigin, false);
		}
	}

	//----------------------------------------------------------------------------
	// Purpose: When the rocket enters the client's PVS, add the flight sound
	//			to it. This is done here rather than PostDataUpdate because 
	//			origins (needed for emitsound) are not valid there
	//----------------------------------------------------------------------------
	void CFFProjectileBase::OnDataChanged(DataUpdateType_t type) 
	{
		if (type == DATA_UPDATE_CREATED)
		{
			if (GetFlightSound())
			{
				EmitSound(GetFlightSound());
			}
		}

		BaseClass::OnDataChanged(type);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Because we're adding interpolation history, the projectile will be
	//			drawn slightly in the past at various times. This function just
	//			calculates whether or not this is the case. It is needed by Draw()
	//			and for any entities that use this as their move parent (eg. rocket
	//			trails).
	//-----------------------------------------------------------------------------
	bool CFFProjectileBase::IsDrawingHistory()
	{
		Vector vecDisplacement = GetLocalOrigin() - m_vecStartOrigin;

		// We don't need to normalise because the magnitude doesn't matter.
		float flDot = vecDisplacement.Dot(m_vecInitialVelocity);

		// If the rocket is behind our start point (thanks to the interpolation)
		// then don't draw it. We also need to stop drawing the trail too.
		return (flDot < 0);
	}

	//----------------------------------------------------------------------------
	// Purpose: 
	//----------------------------------------------------------------------------
	int CFFProjectileBase::DrawModel(int flags) 
	{
		// Don't draw us if we're in some interpolated past.
		if (IsDrawingHistory())
		{
			return 0;
		}

		return BaseClass::DrawModel(flags);
	}

	//----------------------------------------------------------------------------
	// Purpose: Entity is being released, so stop the sound effect
	//----------------------------------------------------------------------------
	void CFFProjectileBase::Release()
	{
		if (m_bNeedsCleanup)
		{
			CleanUp();
			m_bNeedsCleanup = false;
		}

		BaseClass::Release();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Entity has been made dormant, clean up. Sometimes Release isn't
	//			called (due to latency) and so dormant has to step in to solve this
	//-----------------------------------------------------------------------------
	void CFFProjectileBase::SetDormant(bool bDormant)
	{
		if (bDormant && m_bNeedsCleanup)
		{
			CleanUp();
			m_bNeedsCleanup = false;
		}

		BaseClass::SetDormant(bDormant);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Remove sound
	//-----------------------------------------------------------------------------
	void CFFProjectileBase::CleanUp()
	{
		if (GetFlightSound())
		{
			StopSound(GetFlightSound());
		}
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
		m_vecInitialVelocity = velocity;
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