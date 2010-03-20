/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_grenade_laser.cpp
/// @author David "Mervaka" Cook
/// @date March 18, 2010
/// @brief The FF laser grenade code.
///
/// REVISIONS
/// ---------
/// Mar 29, 2005	Mervaka: First created, based on ff_grenade_nail.cpp by Mirvin_Monkey

//========================================================================
// Required includes
//========================================================================
#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"
#include "ff_projectile_nail.h"
#include "effect_dispatch_data.h"
#include "IEffects.h"
#include "beam_shared.h"

#ifdef GAME_DLL
	#include "ff_entity_system.h"
	#include "te_effect_dispatch.h"
#else
	#include "c_te_effect_dispatch.h"
#endif

#define GRENADE_BEAM_SPRITE			"effects/bluelaser1.vmt"
#define NAILGRENADE_MODEL			"models/grenades/nailgren/nailgren.mdl"

#ifdef CLIENT_DLL
	#define CFFGrenadeLaser C_FFGrenadeLaser
#endif

#define MAX_BEAMS 16

#ifdef GAME_DLL

	ConVar laserdamage("ffdev_lasergrenade_damage", "10", FCVAR_CHEAT, "Damage of laser");
	ConVar lasertime("ffdev_lasergrenade_time", "3", FCVAR_CHEAT, "Laser active time");
	ConVar laserangv("ffdev_lasergrenade_angv", "3.75", FCVAR_CHEAT, "Laser angular increment");
	ConVar laserexplode("ffdev_lasergrenade_explode", "0", FCVAR_CHEAT, "Explosion at end of active time");
	ConVar laserbeams( "ffdev_lasergrenade_beams", "2", FCVAR_CHEAT, "Number of laser beams", true, 1, true, MAX_BEAMS);
	ConVar laserdistance( "ffdev_lasergrenade_distance", "256", FCVAR_CHEAT, "Laser beam max radius",true, 0, true, 4096 );
	ConVar laserjump( "ffdev_lasergrenade_jump", "80", FCVAR_CHEAT, "Laser grenade jump distance" );
	ConVar laserbob( "ffdev_lasergrenade_bob", "20", FCVAR_CHEAT, "Laser grenade bob factor" );

#endif




class CFFGrenadeLaser : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeLaser, CFFGrenadeBase) 
	DECLARE_NETWORKCLASS(); 

	virtual void Precache();
	virtual const char *GetBounceSound() { return "NailGrenade.Bounce"; }

	virtual Class_T Classify( void ) { return CLASS_GREN_LASER; } 

	virtual color32 GetColour() { color32 col = { 128, 225, 255, GREN_ALPHA_DEFAULT }; return col; }

#ifdef CLIENT_DLL
	CFFGrenadeLaser() {}
	CFFGrenadeLaser(const CFFGrenadeLaser&) {}
#else
	DECLARE_DATADESC(); // Since we're adding new thinks etc
	virtual void Spawn();
	virtual void BeamEmit();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);
	virtual void DoDamage( CBaseEntity *pTarget );

protected:
	float	m_flAngleOffset;
	CBeam	*pBeam[MAX_BEAMS];

	float	m_flLastThinkTime;

#endif
};

#ifdef GAME_DLL
	BEGIN_DATADESC(CFFGrenadeLaser) 
		DEFINE_THINKFUNC(BeamEmit), 
	END_DATADESC() 
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeLaser, DT_FFGrenadeLaser)

BEGIN_NETWORK_TABLE(CFFGrenadeLaser, DT_FFGrenadeLaser)
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(ff_grenade_laser, CFFGrenadeLaser);
PRECACHE_WEAPON_REGISTER(ff_grenade_laser);

#define NAIL_BBOX 2.0f

//-----------------------------------------------------------------------------
// Purpose: Various precache things
//-----------------------------------------------------------------------------
void CFFGrenadeLaser::Precache() 
{
	PrecacheModel(NAILGRENADE_MODEL);
	PrecacheModel(GRENADE_BEAM_SPRITE);
//	PrecacheScriptSound( "NailGrenade.shoot" );

	BaseClass::Precache();
}

#ifdef GAME_DLL

	//-----------------------------------------------------------------------------
	// Purpose: Various spawny flag things
	//-----------------------------------------------------------------------------
	void CFFGrenadeLaser::Spawn() 
	{
		SetModel(NAILGRENADE_MODEL);
		BaseClass::Spawn();

		m_flAngleOffset = 0.0f;
		SetLocalAngularVelocity(QAngle(0, 0, 0));
	}

	//-----------------------------------------------------------------------------
	// Purpose: Instead of exploding, change to 
	//-----------------------------------------------------------------------------
	void CFFGrenadeLaser::Explode(trace_t *pTrace, int bitsDamageType)
	{
		// Clumsy, will do for now
		if (GetMoveType() == MOVETYPE_FLY)
		{
			BaseClass::Explode(pTrace, bitsDamageType);
			return;
		}

		SetDetonateTimerLength( lasertime.GetInt() );

		// Should this maybe be noclip?
		SetMoveType(MOVETYPE_FLY);

		char i;
		for( i=0; i < laserbeams.GetInt(); i++ )
		{
			pBeam[i] = CBeam::BeamCreate( GRENADE_BEAM_SPRITE, 0.5 );
			pBeam[i]->SetWidth( 1 );
			pBeam[i]->SetEndWidth( 0.05f );
			pBeam[i]->SetBrightness( 255 );
			pBeam[i]->SetColor( 255, 255, 255 );
	//		pBeam[i]->RelinkBeam();
			pBeam[i]->LiveForTime( lasertime.GetFloat() );
		}
		SetThink(&CFFGrenadeLaser::BeamEmit);
		SetNextThink(gpGlobals->curtime);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Spin round emitting lasers
	//-----------------------------------------------------------------------------
	void CFFGrenadeLaser::BeamEmit() 
	{
		// Blow up if we've reached the end of our fuse
		if (gpGlobals->curtime > m_flDetonateTime) 
		{
			if( laserexplode.GetBool() )
				Detonate();
			else
				UTIL_Remove( this );
			return;
		}

		float flRisingheight = 0;

		// Lasts for 3 seconds, rise for 0.3, but only if not handheld
		if (m_flDetonateTime - gpGlobals->curtime > lasertime.GetFloat() - 0.3 && !m_fIsHandheld)
			flRisingheight = laserjump.GetFloat();

		SetAbsVelocity(Vector(0, 0, flRisingheight + laserbob.GetFloat() * sin(DEG2RAD(GetAbsAngles().y))));
		SetAbsAngles(GetAbsAngles() + QAngle(0, laserangv.GetFloat(), 0));

		Vector vecDirection;
		Vector vecOrigin = GetAbsOrigin();
		QAngle angRadial = GetAbsAngles();

		float flSize = 20.0f;
		trace_t tr;
		char i;

		float flDeltaAngle = 360.0f / laserbeams.GetInt();

		for( i = 0; i < laserbeams.GetInt(); i++ )
		{
			AngleVectors(angRadial, &vecDirection);
			VectorNormalizeFast(vecDirection);

			UTIL_TraceLine( vecOrigin + vecDirection * flSize, 
				vecOrigin + vecDirection * laserdistance.GetFloat(), MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER, &tr );
			
			pBeam[i]->PointsInit( vecOrigin, tr.endpos );

			if ( tr.m_pEnt )
				DoDamage( tr.m_pEnt );

			angRadial.y += flDeltaAngle;
		}
		
		SetNextThink(gpGlobals->curtime);
	}

	void CFFGrenadeLaser::DoDamage( CBaseEntity *pTarget )
	{
		// only interested in players, dispensers & sentry guns
		if ( pTarget->IsPlayer() || pTarget->Classify() == CLASS_DISPENSER || pTarget->Classify() == CLASS_SENTRYGUN )
		{
			// If pTarget can take damage from nails...
			if ( g_pGameRules->FCanTakeDamage( pTarget, ToFFPlayer( GetOwnerEntity() ) ) )
			{
				if (pTarget->IsPlayer() )
				{
					CFFPlayer *pPlayerTarget = dynamic_cast< CFFPlayer* > ( pTarget );
					pPlayerTarget->TakeDamage( CTakeDamageInfo( this, ToFFPlayer( GetOwnerEntity() ), laserdamage.GetFloat(), DMG_ENERGYBEAM ) );
				}
				else if( FF_IsDispenser( pTarget ) )
				{
					CFFDispenser *pDispenser = FF_ToDispenser( pTarget );
					if( pDispenser )
						pDispenser->TakeDamage( CTakeDamageInfo( this, ToFFPlayer( GetOwnerEntity() ), laserdamage.GetFloat() + 2, DMG_ENERGYBEAM ) );
				}
				else /*if( FF_IsSentrygun( pTarget ) )*/
				{
					CFFSentryGun *pSentrygun = FF_ToSentrygun( pTarget );
					if( pSentrygun )
						pSentrygun->TakeDamage( CTakeDamageInfo( this, ToFFPlayer( GetOwnerEntity() ), laserdamage.GetFloat() + 2, DMG_ENERGYBEAM ) );
				}
			}
		}
	}

#endif


