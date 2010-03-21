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

ConVar laser_ng_nailspeed("ffdev_lasergren_ng_nailspeed", "300", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar laser_ng_nailstreams( "ffdev_lasergren_ng_arms", "3", FCVAR_REPLICATED | FCVAR_CHEAT );

#ifdef GAME_DLL

	ConVar laserdamage("ffdev_lasergren_damage", "10", FCVAR_CHEAT, "Damage of laser");
	ConVar lasertime("ffdev_lasergren_time", "3", FCVAR_CHEAT, "Laser active time");
	ConVar laserangv("ffdev_lasergren_angv", "3.75", FCVAR_CHEAT, "Laser angular increment");
	ConVar laserbeams( "ffdev_lasergren_beams", "2", FCVAR_CHEAT, "Number of laser beams", true, 1, true, MAX_BEAMS);
	ConVar laserdistance( "ffdev_lasergren_distance", "256", FCVAR_CHEAT, "Laser beam max radius",true, 0, true, 4096 );
	ConVar laserjump( "ffdev_lasergren_jump", "80", FCVAR_CHEAT, "Laser grenade jump distance" );
	ConVar laserbob( "ffdev_lasergren_bob", "20", FCVAR_CHEAT, "Laser grenade bob factor" );
	ConVar laserbeamtime( "ffdev_lasergren_beamtime", "0.02", FCVAR_CHEAT, "Laser grenade update time" );

	ConVar usenails( "ffdev_lasergren_usenails", "1", FCVAR_CHEAT | FCVAR_NOTIFY, "Use nails instead of lasers" );
	
	ConVar laserexplode("ffdev_lasergren_explode", "1", FCVAR_CHEAT, "Explosion at end of active time");
	ConVar explosiondamage("ffdev_lasergren_explosiondamage", "180", FCVAR_CHEAT, "Explosion damage at end of active period" );
	ConVar explosionradius("ffdev_lasergren_explosionradius", "270", FCVAR_CHEAT, "Explosion radius at end of active period" );

	/******************************************************************/
	ConVar laser_ng_naildamage("ffdev_lasergren_ng_naildamage", "10", FCVAR_CHEAT);
	ConVar laser_ng_spittime( "ffdev_lasergren_ng_spittime", "0.02", FCVAR_CHEAT );
	ConVar laser_ng_angleoffset( "ffdev_lasergren_ng_angleoffset", "360.0", FCVAR_CHEAT );
	//ConVar nailspread( "ffdev_nailgren_spread", "5.0", FCVAR_CHEAT );
	//ConVar ffdev_nailgren_flatten("ffdev_nailgren_flatten", "100", FCVAR_CHEAT);

	ConVar laser_ng_nail_bounds("ffdev_lasergren_ng_nail_bounds", "5.0", FCVAR_REPLICATED | FCVAR_CHEAT, "NG Nails bbox");
	ConVar laser_ng_visualizenails("ffdev_lasergren_ng_visualizenails", "0", FCVAR_CHEAT, "Show NG nails trace");
	ConVar laser_ng_nail_length("ffdev_lasergren_ng_nail_length", "5.0", FCVAR_CHEAT, "Length of NG nails");











//-------------------------------------------------------------------------------------------------------
// Simulates a nail as a tracehull moving in a straight line until it hits something
//-------------------------------------------------------------------------------------------------------
class PseudoNail
{
	public:

	//-------------------------------------------------------------------------------------------------------
	// Purpose: Constructor -- only the position and angle are unique for each nail
	//-------------------------------------------------------------------------------------------------------
		PseudoNail( const Vector &vOrigin, const QAngle &vAngles )
		{
			m_vecOrigin = vOrigin;
			m_vecAngles = vAngles;
		}

	//-------------------------------------------------------------------------------------------------------
	// Purpose: Traces the hull of the nail and damages what it "hits"
	//			Returns true if the nail hits something (whether or not it caused damage), and false otherwise
	//-------------------------------------------------------------------------------------------------------
		bool TraceNail( CBaseEntity * pNailOwner, CFFPlayer * pNailGrenOwner )
		{
			if ( !pNailOwner || !pNailGrenOwner )
				return false;
			
			Vector vecForward;
			AngleVectors( m_vecAngles, &vecForward );
			
			// Visualise trace
			if ( laser_ng_visualizenails.GetBool() )
			{
				NDebugOverlay::Line(m_vecOrigin, m_vecOrigin + ( vecForward * laser_ng_nail_length.GetInt() ), 255, 255, 0, false, 5.0f);
				NDebugOverlay::SweptBox(m_vecOrigin, m_vecOrigin + ( vecForward * laser_ng_nail_length.GetInt() ), -Vector( 1.0f, 1.0f, 1.0f ) * laser_ng_nail_bounds.GetFloat(), Vector( 1.0f, 1.0f, 1.0f ) * laser_ng_nail_bounds.GetFloat(), m_vecAngles, 200, 100, 0, 100, 0.1f);
			}

			trace_t traceHit;
			UTIL_TraceHull( m_vecOrigin, m_vecOrigin + ( vecForward * laser_ng_nail_length.GetInt() ), -Vector( 1.0f, 1.0f, 1.0f ) * laser_ng_nail_bounds.GetFloat(), Vector( 1.0f, 1.0f, 1.0f ) * laser_ng_nail_bounds.GetFloat(), MASK_SHOT_HULL, NULL, COLLISION_GROUP_NONE, &traceHit );

			if (traceHit.m_pEnt)
			{
				CBaseEntity *pTarget = traceHit.m_pEnt;
				
				// only interested in players, dispensers & sentry guns
				if ( pTarget->IsPlayer() || pTarget->Classify() == CLASS_DISPENSER || pTarget->Classify() == CLASS_SENTRYGUN )
				{
					// If pTarget can take damage from nails...
					if ( g_pGameRules->FCanTakeDamage( pTarget, pNailGrenOwner ) )
					{
						if (traceHit.m_pEnt->IsPlayer() )
						{
							CFFPlayer *pPlayerTarget = dynamic_cast< CFFPlayer* > ( pTarget );
							pPlayerTarget->TakeDamage( CTakeDamageInfo( pNailOwner, pNailGrenOwner, laser_ng_naildamage.GetInt(), DMG_BULLET ) );
						}
						else if( FF_IsDispenser( pTarget ) )
						{
							CFFDispenser *pDispenser = FF_ToDispenser( pTarget );
							if( pDispenser )
								pDispenser->TakeDamage( CTakeDamageInfo( pNailOwner, pNailGrenOwner, laser_ng_naildamage.GetInt() + 2, DMG_BULLET ) );
						}
						else /*if( FF_IsSentrygun( pTarget ) )*/
						{
							CFFSentryGun *pSentrygun = FF_ToSentrygun( pTarget );
							if( pSentrygun )
								pSentrygun->TakeDamage( CTakeDamageInfo( pNailOwner, pNailGrenOwner, laser_ng_naildamage.GetInt() + 2, DMG_BULLET ) );
						}
					}
				}
				return true;
			}
			else
				return false;
		}
	//-----------------------------------------------------------------------------
	// Purpose: Moves a nail forward a certain distance
	//-----------------------------------------------------------------------------
		void UpdateNailPosition( float flForwardDist )
		{
			Vector vecForward;
			AngleVectors( m_vecAngles, &vecForward );
			vecForward *= flForwardDist;

			m_vecOrigin += vecForward;
		}

	private:

		Vector m_vecOrigin;		// Nail's position
		QAngle m_vecAngles;		// Nail's angle of travel

};















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
	virtual void NailEmit();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);
	virtual void DoDamage( CBaseEntity *pTarget );

	virtual float GetGrenadeDamage()		{ return explosiondamage.GetFloat(); }
	virtual float GetGrenadeRadius()		{ return explosionradius.GetFloat(); }

private:
	void ShootNail( const Vector& vecOrigin, const QAngle& vecAngles );

protected:
	float	m_flBeams;
	float	m_flNailSpit;
	float	m_flAngleOffset;
	CBeam	*pBeam[MAX_BEAMS];

	CUtlVector< PseudoNail > m_NailsVector;

	float	m_flLastThinkTime;

#endif
};

#ifdef GAME_DLL
	BEGIN_DATADESC(CFFGrenadeLaser) 
		DEFINE_THINKFUNC(BeamEmit),
		DEFINE_THINKFUNC(NailEmit),
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
	PrecacheScriptSound( "NailGrenade.shoot" );

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
		if( usenails.GetBool() )
			SetThink(&CFFGrenadeLaser::NailEmit);
		else
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

		if( m_flBeams < gpGlobals->curtime )
		{

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
			m_flBeams = gpGlobals->curtime + laserbeamtime.GetFloat();
		}
		SetNextThink( gpGlobals->curtime );
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

/*********************************************************/

/***************** NAIL GREN CODE ************************/

/*********************************************************/

	//-----------------------------------------------------------------------------
	// Purpose: Spin round emitting nails
	//-----------------------------------------------------------------------------
	void CFFGrenadeLaser::NailEmit() 
	{
		// First we need to trace each nail's bounds to check for a hit, then "move" the nails that didn't hit anything
		for ( int i=0; i < m_NailsVector.Count(); i++)
		{
			// Remove a nail that hits something
			if ( m_NailsVector[i].TraceNail( this, ToFFPlayer( GetOwnerEntity() ) ) )
			{
				m_NailsVector.Remove(i);
				// Remove shifts all the vector elements forward, so we have to adjust the index or we will skip a nail
				i--;
				// Don't jump off the begining or end of the vector, though
				if ( i < 0 || i >= m_NailsVector.Count() )
					i++;
			}
			else // No hit -- move the nail forward
				m_NailsVector[i].UpdateNailPosition( laser_ng_nailspeed.GetInt() * ( gpGlobals->curtime - m_flLastThinkTime ) );
			//if MAX DISTANCE
			//	Remove
		}
		

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

		Vector vecOrigin = GetAbsOrigin();

		// Time to spit out nails again?
		if( m_flNailSpit < gpGlobals->curtime )
		{
			int iStreams = laser_ng_nailstreams.GetInt();
			//float flSize = ffdev_nailgren_flatten.GetFloat();
			float flSize = 20.0f;

			float flDeltaAngle = 360.0f / iStreams;
			QAngle angRadial = GetAbsAngles();
			
			Vector vecDirection;

			while (iStreams-- > 0)
			{
				AngleVectors(angRadial, &vecDirection);
				VectorNormalizeFast(vecDirection);

				ShootNail(vecOrigin + vecDirection * flSize, angRadial);
				angRadial.y += flDeltaAngle;
			}

			EmitSound("NailGrenade.shoot");
			
			CEffectData data;
			data.m_vOrigin = vecOrigin;
			data.m_vAngles = GetAbsAngles();


#ifdef GAME_DLL
			data.m_nEntIndex = entindex();
#else
			data.m_hEntity = this;
#endif

			DispatchEffect("Projectile_Nail_Radial", data);

			// Default to 45 degrees
/*			int iNailSpreadInterval = 360 / ((iStreams == 0) ? 45 : iStreams);

			// Start position (0 to (360 - iNailSpreadInterval))
			int iNailStartPos = (rand() % iStreams) * iNailSpreadInterval;

			int iNailOffset = rand() % laser_ng_angleoffset.GetInt();

			Vector vecNailDir;
			QAngle vecAngles( 0.f, iNailStartPos + iNailOffset, 0.f );

			for( int i = 0; i < iStreams; i++ )
			{
				AngleVectors( vecAngles, &vecNailDir );
				VectorNormalizeFast( vecNailDir );

				ShootNail( GetAbsOrigin() + ( 8.0f * vecNailDir ), vecAngles, ( i == 0 ) ? true : false );

				// Update next position
				vecAngles.y += (iNailSpreadInterval + iNailOffset);
			}*/

			// Set up next nail spit time
			m_flNailSpit = gpGlobals->curtime + laser_ng_spittime.GetFloat();
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

				CFFProjectileNail *pNail = CFFProjectileNail::CreateNail(this, GetAbsOrigin() + ( 8.0f * vecNailDir ), vecAngles, GetOwnerEntity(), laser_ng_naildamage.GetInt(), laser_ng_nailspeed.GetInt());
				if(pNail)
					pNail->m_bNailGrenadeNail = true;				
			}

			// Increment the starting spot for nails a bit so we
			// don't have gaps due to the angular pattern we shoot with
			m_flAngleOffset += laser_ng_angleoffset.GetFloat();

			// TODO: Get tang to make this sound sound like 12 nails being shot out
			// Play this each time we spawn a nail?
			EmitSound( "NailGrenade.shoot" );

			// Set up next nail spit time
			m_flNailSpit = gpGlobals->curtime + laser_ng_spittime.GetFloat();
		}
		*/

		SetNextThink(gpGlobals->curtime);
		m_flLastThinkTime = gpGlobals->curtime;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Shoot a nail out
	//-----------------------------------------------------------------------------
	void CFFGrenadeLaser::ShootNail( const Vector& vecOrigin, const QAngle& vecAngles )
	{
		/*
		// Create the nail and tell it it's a nail grenade nail
		// We don't do a clientside nail with this because that is being done itself
		CFFProjectileNail *pNail = CFFProjectileNail::CreateNail( this, vecOrigin, vecAngles, GetOwnerEntity(), laser_ng_naildamage.GetInt(), laser_ng_nailspeed.GetInt(), true );

		if (!pNail)
			return;

		pNail->m_bNailGrenadeNail = true;

		float flFlatten = ffdev_nailgren_flatten.GetFloat();
		Vector vecFlattened = Vector(flFlatten, flFlatten, 1.0f);
		pNail->SetSize(-vecFlattened, vecFlattened);
		*/
		
		m_NailsVector.AddToTail( PseudoNail( vecOrigin, vecAngles ) );
	}

#endif


