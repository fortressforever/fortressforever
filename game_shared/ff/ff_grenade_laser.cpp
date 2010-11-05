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

ConVar laser_ng_nailspeed("ffdev_lasergren_ng_nailspeed", "1000", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar laser_ng_nailstreams( "ffdev_lasergren_ng_arms", "3", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar laserbeams( "ffdev_lasergren_beams", "3", FCVAR_REPLICATED | FCVAR_CHEAT, "Number of laser beams", true, 1, true, MAX_BEAMS);
ConVar laserdistance( "ffdev_lasergren_distance", "256", FCVAR_REPLICATED | FCVAR_CHEAT, "Laser beam max radius",true, 0, true, 4096 );
ConVar lasertime("ffdev_lasergren_time", "3", FCVAR_REPLICATED | FCVAR_CHEAT, "Laser active time");

#ifdef GAME_DLL

	ConVar laserdamage("ffdev_lasergren_damage", "10", FCVAR_CHEAT, "Damage of laser");
	ConVar laserangv("ffdev_lasergren_angv", "3.75", FCVAR_CHEAT, "Laser angular increment");
	ConVar laserjump( "ffdev_lasergren_jump", "80", FCVAR_CHEAT, "Laser grenade jump distance" );
	ConVar laserbob( "ffdev_lasergren_bob", "20", FCVAR_CHEAT, "Laser grenade bob factor" );
	ConVar laserbeamtime( "ffdev_lasergren_beamtime", "0.0", FCVAR_CHEAT, "Laser grenade update time" );

	ConVar usenails( "ffdev_lasergren_usenails", "0", FCVAR_CHEAT | FCVAR_NOTIFY, "Use nails instead of lasers" );
	ConVar bobfrequency( "ffdev_lasergren_bobfreq", "0.5", FCVAR_CHEAT, "Bob Frequency");
	
	ConVar laserexplode("ffdev_lasergren_explode", "0", FCVAR_CHEAT, "Explosion at end of active time");
	ConVar explosiondamage("ffdev_lasergren_explosiondamage", "180", FCVAR_CHEAT | FCVAR_NOTIFY, "Explosion damage at end of active period" );
	ConVar explosionradius("ffdev_lasergren_explosionradius", "270", FCVAR_CHEAT | FCVAR_NOTIFY, "Explosion radius at end of active period" );

	/******************************************************************/
	ConVar laser_ng_naildamage("ffdev_lasergren_ng_naildamage", "10", FCVAR_CHEAT);
	ConVar laser_ng_spittime( "ffdev_lasergren_ng_spittime", "0.075", FCVAR_CHEAT );
	ConVar laser_ng_angleoffset( "ffdev_lasergren_ng_angleoffset", "360.0", FCVAR_CHEAT );

	//ConVar nailspread( "ffdev_nailgren_spread", "5.0", FCVAR_CHEAT );
	ConVar ffdev_lasergren_ng_nailstreams( "ffdev_lasergren_nailgren_streams", "10", FCVAR_CHEAT );
	//ConVar ffdev_nailgren_flatten("ffdev_nailgren_flatten", "100", FCVAR_CHEAT);

	ConVar ffdev_lasergren_ng_nail_bounds("ffdev_lasergren_ng_nail_bounds", "5.0", FCVAR_REPLICATED | FCVAR_CHEAT, "NG Nails bbox");
	ConVar ffdev_lasergren_ng_visualizenails("ffdev_lasergren_ng_visualizenails", "0", FCVAR_CHEAT, "Show NG nails trace");
	ConVar ffdev_lasergren_ng_nail_length("ffdev_lasergren_ng_nail_length", "5.0", FCVAR_CHEAT, "Length of NG nails");











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
			if ( ffdev_lasergren_ng_visualizenails.GetBool() )
			{
				NDebugOverlay::Line(m_vecOrigin, m_vecOrigin + ( vecForward * ffdev_lasergren_ng_nail_length.GetInt() ), 255, 255, 0, false, 5.0f);
				NDebugOverlay::SweptBox(m_vecOrigin, m_vecOrigin + ( vecForward * ffdev_lasergren_ng_nail_length.GetInt() ), -Vector( 1.0f, 1.0f, 1.0f ) * ffdev_lasergren_ng_nail_bounds.GetFloat(), Vector( 1.0f, 1.0f, 1.0f ) * ffdev_lasergren_ng_nail_bounds.GetFloat(), m_vecAngles, 200, 100, 0, 100, 0.1f);
			}

			trace_t traceHit;
			UTIL_TraceHull( m_vecOrigin, m_vecOrigin + ( vecForward * ffdev_lasergren_ng_nail_length.GetInt() ), -Vector( 1.0f, 1.0f, 1.0f ) * ffdev_lasergren_ng_nail_bounds.GetFloat(), Vector( 1.0f, 1.0f, 1.0f ) * ffdev_lasergren_ng_nail_bounds.GetFloat(), MASK_SHOT_HULL, NULL, COLLISION_GROUP_NONE, &traceHit );

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
							pPlayerTarget->TakeDamage( CTakeDamageInfo( pNailOwner, pNailGrenOwner, ffdev_lasergren_ng_naildamage.GetInt(), DMG_BULLET ) );
						}
						else if( FF_IsDispenser( pTarget ) )
						{
							CFFDispenser *pDispenser = FF_ToDispenser( pTarget );
							if( pDispenser )
								pDispenser->TakeDamage( CTakeDamageInfo( pNailOwner, pNailGrenOwner, ffdev_lasergren_ng_naildamage.GetInt() + 2, DMG_BULLET ) );
						}
						else /*if( FF_IsSentrygun( pTarget ) )*/
						{
							CFFSentryGun *pSentrygun = FF_ToSentrygun( pTarget );
							if( pSentrygun )
								pSentrygun->TakeDamage( CTakeDamageInfo( pNailOwner, pNailGrenOwner, ffdev_lasergren_ng_naildamage.GetInt() + 2, DMG_BULLET ) );
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

	CNetworkVar( unsigned int, m_bIsOn );

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


private:
	void ShootNail( const Vector& vecOrigin, const QAngle& vecAngles );

protected:
	float	m_flNailSpit;
	float	m_flAngleOffset;

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
#ifdef GAME_DLL
SendPropInt(SENDINFO(m_bIsOn), 1, SPROP_UNSIGNED),
#else
RecvPropInt(RECVINFO(m_bIsOn)),
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
		m_bIsOn = 0;
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

		if( usenails.GetBool() )
			SetThink(&CFFGrenadeLaser::NailEmit);
		else
		{
			m_bIsOn = 1;
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
				UTIL_Remove( this );
			return;
		}

		float flRisingheight = 0;

		// Lasts for 3 seconds, rise for 0.3, but only if not handheld
		if (m_flDetonateTime - gpGlobals->curtime > lasertime.GetFloat() - 0.3 && !m_fIsHandheld)
			flRisingheight = laserjump.GetFloat();

		SetAbsVelocity(Vector(0, 0, flRisingheight + laserbob.GetFloat() * sin(DEG2RAD(gpGlobals->curtime * 360 * bobfrequency.GetFloat()))));
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
			
			if ( tr.m_pEnt )
				DoDamage( tr.m_pEnt );

			angRadial.y += flDeltaAngle;
		}
		
		SetNextThink( gpGlobals->curtime + 0.01f );
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
				m_NailsVector[i].UpdateNailPosition( ffdev_lasergren_ng_nailspeed.GetInt() * ( gpGlobals->curtime - m_flLastThinkTime ) );
			//if MAX DISTANCE
			//	Remove
		}
		

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

		SetAbsVelocity(Vector(0, 0, flRisingheight + laserbob.GetFloat() * sin(DEG2RAD(gpGlobals->curtime * 360 * bobfrequency.GetFloat()))));
		SetAbsAngles(GetAbsAngles() + QAngle(0, laserangv.GetFloat(), 0));

		Vector vecOrigin = GetAbsOrigin();

		// Time to spit out nails again?
		if( m_flNailSpit < gpGlobals->curtime )
		{
			int iStreams = ffdev_lasergren_ng_nailstreams.GetInt();
			//float flSize = ffdev_nailgren_flatten.GetFloat();
			float flSize = 20.0f;

			float flDeltaAngle = 360.0f / iStreams;
			QAngle angRadial = QAngle(0.0f, random->RandomFloat(0.0f, flDeltaAngle), 0.0f);
			
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
			data.m_vAngles = QAngle(0, 0, 0);

#ifdef GAME_DLL
			data.m_nEntIndex = entindex();
#else
			data.m_hEntity = this;
#endif

			DispatchEffect("Projectile_Nail_Radial", data);

			// Set up next nail spit time
			m_flNailSpit = gpGlobals->curtime + ffdev_lasergren_ng_spittime.GetFloat();
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
								vecOrigin + vecDirection * laserdistance.GetFloat(), 
								MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER, &tr );
				
				if( !pBeam[i] )
				{
					pBeam[i] = CBeam::BeamCreate( GRENADE_BEAM_SPRITE, 0.5 );
					pBeam[i]->SetWidth( 1 );
					pBeam[i]->SetEndWidth( 0.05f );
					pBeam[i]->SetBrightness( 255 );
					pBeam[i]->SetColor( 255, 255, 255 );
					pBeam[i]->LiveForTime( lasertime.GetFloat()  );
				}
				pBeam[i]->PointsInit( vecOrigin, tr.endpos );

				angRadial.y += flDeltaAngle;
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


