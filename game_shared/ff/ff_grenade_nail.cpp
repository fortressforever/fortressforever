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

#ifdef GAME_DLL
	#include "ff_entity_system.h"
#endif

#define NAILGRENADE_MODEL "models/grenades/nailgren/nailgren.mdl"

#ifdef CLIENT_DLL
	#define CFFGrenadeNail C_FFGrenadeNail
#endif

class CFFGrenadeNail : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeNail, CFFGrenadeBase) 

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
	virtual void Explode(trace_t *pTrace, int bitsDamageType);

private:
	void ShootNail( const Vector& vecOrigin, const QAngle& vecAngles, bool bPlaySound );

protected:
	float	m_flNailSpit;
	float	m_flAngleOffset;
#endif
};

#ifdef GAME_DLL
	BEGIN_DATADESC(CFFGrenadeNail) 
		DEFINE_THINKFUNC(NailEmit), 
	END_DATADESC() 
#endif

LINK_ENTITY_TO_CLASS(ff_grenade_nail, CFFGrenadeNail);
PRECACHE_WEAPON_REGISTER(ff_grenade_nail);

#ifndef CLIENT_DLL
	ConVar nailspeed("ffdev_nailspeed", "800");
	ConVar naildamage("ffdev_naildamage", "8");
	ConVar nailgren_spittime( "ffdev_nailgren_spittime", "0.1" );
	//ConVar nailgren_angleoffset( "ffdev_nailgren_angleoffset", "5.0" );
	//ConVar nailspread( "ffdev_nailgren_spread", "5.0" );
	ConVar nailstreams( "ffdev_nailgren_streams", "8" );
#endif


//-----------------------------------------------------------------------------
// Purpose: Various precache things
//-----------------------------------------------------------------------------
void CFFGrenadeNail::Precache() 
{
	PrecacheModel(NAILGRENADE_MODEL);
	PrecacheScriptSound( "NailGrenade.shoot" );

	BaseClass::Precache();
}

#ifdef GAME_DLL

	//-----------------------------------------------------------------------------
	// Purpose: Various spawny flag things
	//-----------------------------------------------------------------------------
	void CFFGrenadeNail::Spawn() 
	{
		SetModel(NAILGRENADE_MODEL);
		BaseClass::Spawn();

		m_flAngleOffset = 0.0f;
		m_flNailSpit = 0.0f;
		SetLocalAngularVelocity(QAngle(0, 0, 0));
	}

	//-----------------------------------------------------------------------------
	// Purpose: Shoot a nail out
	//-----------------------------------------------------------------------------
	void CFFGrenadeNail::ShootNail( const Vector& vecOrigin, const QAngle& vecAngles, bool bPlaySound )
	{
		// Create the nail and tell it it's a nail grenade nail
		CFFProjectileNail *pNail = CFFProjectileNail::CreateNail( this, vecOrigin, vecAngles, GetOwnerEntity(), naildamage.GetInt(), nailspeed.GetInt() );
		if( pNail )
			pNail->m_bNailGrenadeNail = true;

		if( bPlaySound )
			EmitSound( "NailGrenade.shoot" );
	}

	//-----------------------------------------------------------------------------
	// Purpose: Instead of exploding, change to 
	//-----------------------------------------------------------------------------
	void CFFGrenadeNail::Explode(trace_t *pTrace, int bitsDamageType)
	{
		// Clumsy, will do for now
		if (GetMoveType() == MOVETYPE_FLY)
		{
			BaseClass::Explode(pTrace, bitsDamageType);
			return;
		}

		SetDetonateTimerLength(3.0f);

		// Should this maybe be noclip?
		SetMoveType(MOVETYPE_FLY);

		// Go into nail mode
		SetThink(&CFFGrenadeNail::NailEmit);
		SetNextThink(gpGlobals->curtime);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Spin round emitting nails
	//-----------------------------------------------------------------------------
	void CFFGrenadeNail::NailEmit() 
	{
		// Blow up if we've reached the end of our fuse
		if (gpGlobals->curtime > m_flDetonateTime) 
		{
			Detonate();
			return;
		}

		float flRisingheight = 0;

		// Lasts for 3 seconds, rise for 0.3, but only if not handheld
		if (m_flDetonateTime - gpGlobals->curtime > 2.6 && !m_fIsHandheld)
			flRisingheight = 80;

		SetAbsVelocity(Vector(0, 0, flRisingheight + 20 * sin(DEG2RAD(GetAbsAngles().y))));
		SetAbsAngles(GetAbsAngles() + QAngle(0, 15, 0));

		static int iPattern[] =
		{
			5, 10, 15, 20, 25
		};

		// Time to spit out nails again?
		if( m_flNailSpit < gpGlobals->curtime )
		{
			int iStreamRotationOffset = 0;

			for( int i = 0; i < nailstreams.GetInt(); i++ )
			{
				Vector vecNailDir;
				QAngle vecAngles = QAngle( 0, 0 + iPattern[rand() % 5] + iStreamRotationOffset, 0 );

				AngleVectors( vecAngles, &vecNailDir );
				VectorNormalizeFast( vecNailDir );

				ShootNail( GetAbsOrigin() + ( 8.0f * vecNailDir ), vecAngles, ( iStreamRotationOffset == 0 ) ? true : false );

				iStreamRotationOffset += ( 360 / nailstreams.GetInt() );
			}

			// Set up next nail spit time
			m_flNailSpit = gpGlobals->curtime + nailgren_spittime.GetFloat();
		}

		/*
		// Bug #0000674: Nail grenade doesn't shoot nails out like TFC nail grenade
		//	Otherwise known as: Make nail grens look rubbish again		

		// Time to spit out nails again?
		if( m_flNailSpit < gpGlobals->curtime )
		{
			// Do the classic TFC pattern.
			// 9/27/2006 - upping from 12 to 24 and change angle from 30 to 15
			for( int i = 0; i < 23; i++ )
			{
				Vector vecNailDir;
				QAngle vecAngles = GetAbsAngles() + QAngle( 0, ( 15.0f * i ) + m_flAngleOffset, 0 );				
				AngleVectors( vecAngles, &vecNailDir );
				VectorNormalizeFast( vecNailDir );

				CFFProjectileNail *pNail = CFFProjectileNail::CreateNail(this, GetAbsOrigin() + ( 8.0f * vecNailDir ), vecAngles, GetOwnerEntity(), naildamage.GetInt(), nailspeed.GetInt());
				if(pNail)
					pNail->m_bNailGrenadeNail = true;				
			}

			// Increment the starting spot for nails a bit so we
			// don't have gaps due to the angular pattern we shoot with
			m_flAngleOffset += nailgren_angleoffset.GetFloat();

			// TODO: Get tang to make this sound sound like 12 nails being shot out
			// Play this each time we spawn a nail?
			EmitSound( "NailGrenade.shoot" );

			// Set up next nail spit time
			m_flNailSpit = gpGlobals->curtime + nailgren_spittime.GetFloat();
		}
		*/

		SetNextThink(gpGlobals->curtime);
	}

#endif
