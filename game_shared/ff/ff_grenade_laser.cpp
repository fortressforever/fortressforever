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

#define GRENADE_BEAM_SPRITE			"sprites/plasma.spr"
#define NAILGRENADE_MODEL			"models/grenades/nailgren/nailgren.mdl"

#ifdef CLIENT_DLL
	#define CFFGrenadeLaser C_FFGrenadeLaser
#endif

#define MAX_BEAMS 16

//ConVar laser_ng_streams("ffdev_lasergren_ng_streams", "2", FCVAR_REPLICATED, "Number of streams per arm" );
ConVar laser_ng_offset("ffdev_lasergren_ng_offset", "8", FCVAR_REPLICATED, "Stream offset" );
ConVar laser_ng_nailspeed("ffdev_lasergren_ng_nailspeed", "1000", FCVAR_REPLICATED );
ConVar laser_ng_arms( "ffdev_lasergren_ng_arms", "3", FCVAR_REPLICATED );
ConVar laserbeams( "ffdev_lasergren_beams", "3", FCVAR_REPLICATED, "Number of laser beams", true, 1, true, MAX_BEAMS);
ConVar laserdistance( "ffdev_lasergren_distance", "256", FCVAR_REPLICATED, "Laser beam max radius",true, 0, true, 4096 );
ConVar growTime( "ffdev_lasergren_growTime", "0.7", FCVAR_REPLICATED, "Time taken to grow to full length" );
ConVar shrinkTime( "ffdev_lasergren_shrinkTime", "1", FCVAR_REPLICATED, "Time taken to shrink to nothing" );
ConVar lasertime("ffdev_lasergren_time", "10", FCVAR_REPLICATED, "Laser active time");

#ifdef CLIENT_DLL
	ConVar hud_lasergren_customColor_enable( "hud_lasergren_customColor_enable", "0", FCVAR_ARCHIVE, "Use custom laser colors (1 = use custom colour)");
	ConVar hud_lasergren_customColor_r( "hud_lasergren_customColor_r", "255", FCVAR_ARCHIVE, "Custom laser color - Red Component (0-255)");
	ConVar hud_lasergren_customColor_g( "hud_lasergren_customColor_g", "128", FCVAR_ARCHIVE, "Custom laser color - Green Component(0-255)");
	ConVar hud_lasergren_customColor_b( "hud_lasergren_customColor_b", "255", FCVAR_ARCHIVE, "Custom laser color - Blue Component(0-255)");
	ConVar ffdev_lasergren_widthcreate("ffdev_lasergren_widthcreate", "0.5", FCVAR_REPLICATED, "Width given in the constructor; not used");
	ConVar ffdev_lasergren_widthstart("ffdev_lasergren_widthstart", "6", FCVAR_REPLICATED, "Width at the start of the beam");
	ConVar ffdev_lasergren_widthend("ffdev_lasergren_widthend", "3", FCVAR_REPLICATED, "Width at the end of the beam");
	#define LASERGREN_WIDTHCREATE ffdev_lasergren_widthcreate.GetFloat()
	#define LASERGREN_WIDTHSTART ffdev_lasergren_widthstart.GetFloat()
	#define LASERGREN_WIDTHEND ffdev_lasergren_widthend.GetFloat()
#endif

#ifdef GAME_DLL

	ConVar laserdamage("ffdev_lasergren_damage", "350", FCVAR_NOTIFY, "Damage of laser");
	#define LASERGREN_DAMAGE_PER_TICK laserdamage.GetFloat()*gpGlobals->interval_per_tick
	ConVar laserdamage_buildablemult("ffdev_lasergren_damage_buildablemult", "1.0", FCVAR_NOTIFY, "Damage multiplier of laser against buildables");
	ConVar laserangv("ffdev_lasergren_angv", "120", FCVAR_NOTIFY, "Laser angular increment");
	#define LASERGREN_ROTATION_PER_TICK laserangv.GetFloat()*gpGlobals->interval_per_tick
	ConVar laserjump( "ffdev_lasergren_jump", "80", FCVAR_NOTIFY, "Laser grenade jump distance" );
	ConVar laserbob( "ffdev_lasergren_bob", "10", FCVAR_NOTIFY, "Laser grenade bob factor" );
	ConVar laserbeamtime( "ffdev_lasergren_beamtime", "0.0", FCVAR_CHEAT, "Laser grenade update time" );

	ConVar usenails( "ffdev_lasergren_usenails", "0", FCVAR_NOTIFY, "Use nails instead of lasers" );
	ConVar bobfrequency( "ffdev_lasergren_bobfreq", "0.5", FCVAR_NOTIFY, "Bob Frequency");
	
	ConVar laserexplode("ffdev_lasergren_explode", "0", FCVAR_NOTIFY, "Explosion at end of active time");
	ConVar explosiondamage("ffdev_lasergren_explosiondamage", "90", FCVAR_NOTIFY, "Explosion damage at end of active period" );
	ConVar explosionradius("ffdev_lasergren_explosionradius", "180", FCVAR_NOTIFY, "Explosion radius at end of active period" );

	/******************************************************************/
	ConVar laser_ng_naildamage("ffdev_lasergren_ng_naildamage", "10", FCVAR_NOTIFY);
	ConVar laser_ng_naildamage_buildablemult("ffdev_lasergren_ng_naildamage_buildablemult", "1.0", FCVAR_NOTIFY);
	ConVar laser_ng_spittime( "ffdev_lasergren_ng_spittime", "0.025", FCVAR_NOTIFY );
	ConVar laser_ng_angleoffset( "ffdev_lasergren_ng_angleoffset", "360.0", 0 );
	//ConVar nailspread( "ffdev_nailgren_spread", "5.0", FCVAR_CHEAT );
	//ConVar ffdev_nailgren_flatten("ffdev_nailgren_flatten", "100", FCVAR_CHEAT);

	ConVar laser_ng_nail_bounds("ffdev_lasergren_ng_nail_bounds", "5.0", FCVAR_REPLICATED, "NG Nails bbox");
	ConVar laser_ng_visualizenails("ffdev_lasergren_ng_visualizenails", "0", FCVAR_CHEAT, "Show NG nails trace");
	ConVar laser_ng_nail_length("ffdev_lasergren_ng_nail_length", "5.0", 0, "Length of NG nails");











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

				if (!pTarget)
					return false;
				
				// only interested in players, dispensers & sentry guns
				if ( pTarget->IsPlayer() || pTarget->Classify() == CLASS_DISPENSER || pTarget->Classify() == CLASS_SENTRYGUN )
				{
					// If pTarget can take damage from nails...
					if ( g_pGameRules->FCanTakeDamage( pTarget, pNailGrenOwner ) )
					{
						if (traceHit.m_pEnt->IsPlayer() )
						{
							CFFPlayer *pPlayerTarget = dynamic_cast< CFFPlayer* > ( pTarget );
							if (pPlayerTarget)
								pPlayerTarget->TakeDamage( CTakeDamageInfo( pNailOwner, pNailGrenOwner, laser_ng_naildamage.GetInt(), DMG_BULLET ) );
						}
						else if( FF_IsDispenser( pTarget ) )
						{
							CFFDispenser *pDispenser = FF_ToDispenser( pTarget );
							if( pDispenser )
								pDispenser->TakeDamage( CTakeDamageInfo( pNailOwner, pNailGrenOwner, laser_ng_naildamage.GetInt() * laser_ng_naildamage_buildablemult.GetFloat(), DMG_BULLET ) );
						}
						else if( FF_IsSentrygun( pTarget ) )
						{
							CFFSentryGun *pSentrygun = FF_ToSentrygun( pTarget );
							if( pSentrygun )
								pSentrygun->TakeDamage( CTakeDamageInfo( pNailOwner, pNailGrenOwner, laser_ng_naildamage.GetInt() * laser_ng_naildamage_buildablemult.GetFloat(), DMG_BULLET ) );
						}
						else /*if( FF_IsManCannon( pTarget ) )*/
						{
							CFFManCannon *pManCannon = FF_ToManCannon( pTarget );
							if( pManCannon )
								pManCannon->TakeDamage( CTakeDamageInfo( pNailOwner, pNailGrenOwner, laser_ng_naildamage.GetInt() * laser_ng_naildamage_buildablemult.GetFloat(), DMG_BULLET ) );
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

	float getLengthPercent();
	bool m_bPlayingDeploySound;

#ifdef CLIENT_DLL
	CFFGrenadeLaser() {}
	CFFGrenadeLaser(const CFFGrenadeLaser&) {}
	virtual void ClientThink();
	virtual void OnDataChanged(DataUpdateType_t updateType);
	virtual void UpdateOnRemove( void );
protected:
	CBeam	*pBeam[MAX_BEAMS];
	IMaterial	*m_pMaterial;

public:
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
	int		m_iOffset;

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
#ifdef CLIENT_DLL
	RecvPropFloat(RECVINFO(m_flDetonateTime)), 
#else
	SendPropFloat(SENDINFO(m_flDetonateTime)),
#endif
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
	PrecacheScriptSound( "NailGrenade.LaserLoop" );
	PrecacheScriptSound( "NailGrenade.LaserDeploy" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: To calc % length for laser grow/shrink
//-----------------------------------------------------------------------------
float CFFGrenadeLaser::getLengthPercent()
{
	float spawnTime = m_flDetonateTime - lasertime.GetFloat();
	float growTillTime = spawnTime + growTime.GetFloat();
	float startShrinkTime = m_flDetonateTime - shrinkTime.GetFloat();

	if(gpGlobals->curtime > growTillTime && gpGlobals->curtime < startShrinkTime && m_flDetonateTime > 0)
	{
		if ( m_bPlayingDeploySound )
		{
			StopSound("NailGrenade.LaserDeploy");
			m_bPlayingDeploySound = 0;
		}
		return 1.0f;
	}
	else if(gpGlobals->curtime >= startShrinkTime && m_flDetonateTime > 0)
	{
		if ( !m_bPlayingDeploySound )
		{
			EmitSound("NailGrenade.LaserDeploy");
			m_bPlayingDeploySound = 1;
		}
		return 1 - (gpGlobals->curtime - startShrinkTime) / (m_flDetonateTime - startShrinkTime);
	}
	else if(gpGlobals->curtime < growTillTime && m_flDetonateTime > 0)
	{
		if ( !m_bPlayingDeploySound )
		{
			EmitSound("NailGrenade.LaserDeploy");
			m_bPlayingDeploySound = 1;
		}
		return (gpGlobals->curtime - spawnTime) / (growTillTime - spawnTime);
	}
	else
		return 0.0f;
}

#ifdef GAME_DLL

	//-----------------------------------------------------------------------------
	// Purpose: Various spawny flag things
	//-----------------------------------------------------------------------------
	void CFFGrenadeLaser::Spawn() 
	{
		SetModel(NAILGRENADE_MODEL);
		BaseClass::Spawn();
		m_bIsOn = false;
		m_flAngleOffset = 0.0f;
		m_iOffset = 0;
		SetLocalAngularVelocity(QAngle(0, 0, 0));

		m_bPlayingDeploySound = 0;
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

		SetDetonateTimerLength( lasertime.GetFloat() );

		// Should this maybe be noclip?
		SetMoveType(MOVETYPE_FLY);

		if( usenails.GetBool() )
			SetThink(&CFFGrenadeLaser::NailEmit);
		else
		{
			m_bIsOn = true;
			SetThink(&CFFGrenadeLaser::BeamEmit);
		}
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
			{
				g_pEffects->EnergySplash(GetAbsOrigin(), Vector(0, 0, 1.0f), true);
				UTIL_Remove( this );
			}
			return;
		}

		float flRisingheight = 0;

		// Lasts for 3 seconds, rise for 0.3, but only if not handheld
		if (m_flDetonateTime - gpGlobals->curtime > lasertime.GetFloat() - 0.3 && !m_fIsHandheld)
			flRisingheight = laserjump.GetFloat();

		SetAbsVelocity(Vector(0, 0, flRisingheight + laserbob.GetFloat() * sin(DEG2RAD(gpGlobals->curtime * 360 * bobfrequency.GetFloat()))));
		SetAbsAngles(GetAbsAngles() + QAngle(0, LASERGREN_ROTATION_PER_TICK, 0));

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
				vecOrigin + vecDirection * laserdistance.GetFloat() * getLengthPercent(), MASK_PLAYERSOLID, NULL, COLLISION_GROUP_PLAYER, &tr );

			if ( tr.m_pEnt )
				DoDamage( tr.m_pEnt );

			angRadial.y += flDeltaAngle;
		}
		SetNextThink( gpGlobals->curtime );
	}

	void CFFGrenadeLaser::DoDamage( CBaseEntity *pTarget )
	{
		if (!pTarget)
			return;

		// only interested in players, dispensers & sentry guns
		if ( pTarget->IsPlayer() || pTarget->Classify() == CLASS_DISPENSER || pTarget->Classify() == CLASS_SENTRYGUN || pTarget->Classify() == CLASS_MANCANNON )
		{
			// If pTarget can take damage from nails...
			if ( g_pGameRules->FCanTakeDamage( pTarget, ToFFPlayer( GetOwnerEntity() ) ) )
			{
				if (pTarget->IsPlayer() )
				{
					CFFPlayer *pPlayerTarget = dynamic_cast< CFFPlayer* > ( pTarget );
					if (!pPlayerTarget)
						return;

					CTakeDamageInfo info = CTakeDamageInfo( this, ToFFPlayer( GetOwnerEntity() ), LASERGREN_DAMAGE_PER_TICK, DMG_ENERGYBEAM );
					//Adding the position for demoman shield blocks -GreenMushy
					info.SetDamagePosition( GetAbsOrigin() );
					info.SetImpactPosition( pPlayerTarget->GetAbsOrigin() );
					pPlayerTarget->TakeDamage( info );
				}
				else if( FF_IsDispenser( pTarget ) )
				{
					CFFDispenser *pDispenser = FF_ToDispenser( pTarget );
					if( pDispenser )
						pDispenser->TakeDamage( CTakeDamageInfo( this, ToFFPlayer( GetOwnerEntity() ), LASERGREN_DAMAGE_PER_TICK * laserdamage_buildablemult.GetFloat(), DMG_ENERGYBEAM ) );
				}
				else if( FF_IsSentrygun( pTarget ) )
				{
					CFFSentryGun *pSentrygun = FF_ToSentrygun( pTarget );
					if( pSentrygun )
						pSentrygun->TakeDamage( CTakeDamageInfo( this, ToFFPlayer( GetOwnerEntity() ), LASERGREN_DAMAGE_PER_TICK * laserdamage_buildablemult.GetFloat(), DMG_ENERGYBEAM ) );
				}
				else /*if( FF_IsManCannon( pTarget ) )*/
				{
					CFFManCannon *pManCannon = FF_ToManCannon( pTarget );
					if( pManCannon )
						pManCannon->TakeDamage( CTakeDamageInfo( this, ToFFPlayer( GetOwnerEntity() ), LASERGREN_DAMAGE_PER_TICK * laserdamage_buildablemult.GetFloat(), DMG_ENERGYBEAM ) );
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

		SetAbsVelocity(Vector(0, 0, flRisingheight + laserbob.GetFloat() * sin(DEG2RAD(gpGlobals->curtime * 360 * bobfrequency.GetFloat()))));
		SetAbsAngles(GetAbsAngles() + QAngle(0, LASERGREN_ROTATION_PER_TICK, 0));

		Vector vecOrigin = GetAbsOrigin();

		// Time to spit out nails again?
		if( m_flNailSpit < gpGlobals->curtime )
		{
			int iArms = laser_ng_arms.GetInt();
			//float flSize = ffdev_nailgren_flatten.GetFloat();
			float flSize = 20.0f;

			float flDeltaAngle = 360.0f / iArms;
			QAngle angRadial = GetAbsAngles();
			
			//float flOffset = (m_iOffset % laser_ng_streams.GetInt()) - (laser_ng_streams.GetInt() / 2 );
			//DevMsg("flOffset: %f\n", flOffset);
			Vector vecDirection, vecOffset;
			float flOffset = laser_ng_offset.GetFloat();
			if( m_iOffset % 2 )
				flOffset *= -1;

			while (iArms-- > 0)
			{
				AngleVectors(angRadial, &vecDirection);
				VectorNormalizeFast(vecDirection);
				VectorRotate( Vector( 0, flOffset, 0 ), angRadial, vecOffset );


				ShootNail(vecOrigin + vecDirection * flSize + vecOffset, angRadial);
				angRadial.y += flDeltaAngle;
			}

			EmitSound("NailGrenade.shoot");
			
			CEffectData data;
			data.m_vOrigin = vecOrigin;
			data.m_vAngles = GetAbsAngles();
			data.m_nDamageType = m_iOffset;


#ifdef GAME_DLL
			data.m_nEntIndex = entindex();
#else
			data.m_hEntity = this;
#endif

			DispatchEffect("Projectile_Nail_Radial", data);

			m_iOffset++;
			// Set up next nail spit time
			m_flNailSpit = gpGlobals->curtime + laser_ng_spittime.GetFloat();
		}
		SetNextThink(gpGlobals->curtime);
		m_flLastThinkTime = gpGlobals->curtime;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Shoot a nail out
	//-----------------------------------------------------------------------------
	void CFFGrenadeLaser::ShootNail( const Vector& vecOrigin, const QAngle& vecAngles )
	{
		m_NailsVector.AddToTail( PseudoNail( vecOrigin, vecAngles ) );
	}

#else
	void CFFGrenadeLaser::UpdateOnRemove( void )
	{
		StopSound("NailGrenade.LaserLoop");
		StopSound("NailGrenade.LaserDeploy");

		int i;
		for( i = 0; i < laserbeams.GetInt(); i++ )
		{
			if( pBeam[i] )
			{
				delete pBeam[i];
			}
		}

		BaseClass::UpdateOnRemove();
	}
	//-----------------------------------------------------------------------------
	// Purpose: Emit gas.
	//-----------------------------------------------------------------------------
	void CFFGrenadeLaser::ClientThink()
	{
		if ( m_bIsOn )
		{
			EmitSound("NailGrenade.LaserLoop");

			Vector vecDirection;
			Vector vecOrigin = GetAbsOrigin();
			QAngle angRadial = GetAbsAngles();

			float flSize = 20.0f;
			trace_t tr;
			char i;

			CFFPlayer *pgrenOwner = dynamic_cast<CFFPlayer *> (this->GetOwnerEntity());

			if (!pgrenOwner)
				return;

			float flDeltaAngle = 360.0f / laserbeams.GetInt();
		
			for( i = 0; i < laserbeams.GetInt(); i++ )
			{
				AngleVectors(angRadial, &vecDirection);
				VectorNormalizeFast(vecDirection);

				UTIL_TraceLine( vecOrigin + vecDirection * flSize, 
								vecOrigin + vecDirection * laserdistance.GetFloat() * getLengthPercent(), 
								MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER, &tr );
				
				if( !pBeam[i] )
				{
					pBeam[i] = CBeam::BeamCreate( GRENADE_BEAM_SPRITE, LASERGREN_WIDTHCREATE );
					if (!pBeam[i])
						continue;

					pBeam[i]->SetWidth( LASERGREN_WIDTHSTART );
					pBeam[i]->SetEndWidth( LASERGREN_WIDTHEND );
					pBeam[i]->LiveForTime( 1  );
					pBeam[i]->SetBrightness( 255 );
					if(hud_lasergren_customColor_enable.GetBool() == true)
						pBeam[i]->SetColor( hud_lasergren_customColor_r.GetInt(), hud_lasergren_customColor_g.GetInt(), hud_lasergren_customColor_b.GetInt() );
					else if(pgrenOwner->GetTeamNumber() == TEAM_RED)
						pBeam[i]->SetColor( 255, 64, 64 );
					else if(pgrenOwner->GetTeamNumber() == TEAM_BLUE)
						pBeam[i]->SetColor( 64, 128, 255 );
					else if(pgrenOwner->GetTeamNumber() == TEAM_GREEN)
						pBeam[i]->SetColor( 153, 255, 153 );
					else if(pgrenOwner->GetTeamNumber() == TEAM_YELLOW)
						pBeam[i]->SetColor( 255, 178, 0 );
					else // just in case
						pBeam[i]->SetColor( 204, 204, 204 );
				}
				pBeam[i]->PointsInit( vecOrigin, tr.endpos );

				angRadial.y += flDeltaAngle;

				/*
				g_pEffects->Sparks(tr.endpos);

				//UTIL_DecalTrace( &tr, "LaserBurn" );

				if(tr.fraction != 1)
					g_pEffects->Smoke(tr.endpos, -1, 6, -1);
				*/

				if ( tr.fraction == 1.0f )
					g_pEffects->MetalSparks( tr.endpos, vecDirection );
				else
					g_pEffects->MetalSparks( tr.endpos, -vecDirection );

			}
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Called when data changes on the server
	//-----------------------------------------------------------------------------
	void CFFGrenadeLaser::OnDataChanged(DataUpdateType_t updateType)
	{
		// NOTE: We MUST call the base classes' implementation of this function
//		BaseClass::OnDataChanged(updateType);

		// Setup our entity's particle system on creation
		if (updateType == DATA_UPDATE_CREATED)
		{
			m_pMaterial = materials->FindMaterial("effects/blueblacklargebeam", TEXTURE_GROUP_CLIENT_EFFECTS);
			m_pMaterial->IncrementReferenceCount();

			// Call our ClientThink() function once every client frame
			SetNextClientThink(CLIENT_THINK_ALWAYS);
		}
	}

#endif


