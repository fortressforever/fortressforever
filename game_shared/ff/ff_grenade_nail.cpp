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
/// Dec. 27, 2007	Jiggles: Significantly revamped -- nails are now "simulated" as short tracehulls; 
///								nail entities are apparently too expensive to spam as many as we need for a decent nailgren

//========================================================================
// Required includes
//========================================================================
#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"
#include "ff_projectile_nail.h"
#include "effect_dispatch_data.h"
#include "IEffects.h"

#ifdef GAME_DLL
	#include "ff_entity_system.h"
	#include "te_effect_dispatch.h"
#else
	#include "c_te_effect_dispatch.h"
#endif

#define NAILGRENADE_MODEL "models/grenades/nailgren/nailgren.mdl"

#ifdef CLIENT_DLL
	#define CFFGrenadeNail C_FFGrenadeNail
#endif


	// Dexter - moved and renamed these from the laser gren stuff 
	//			.. for some reason the FX was using laser ones for old nail gren still
	ConVar ffdev_ng_offset("ffdev_ng_offset", "8", FCVAR_REPLICATED | FCVAR_CHEAT, "Stream offset" );
	ConVar ffdev_ng_nailspeed("ffdev_ng_nailspeed", "1000", FCVAR_REPLICATED | FCVAR_CHEAT );
	ConVar ffdev_ng_arms( "ffdev_ng_arms", "3", FCVAR_REPLICATED | FCVAR_CHEAT );

#ifdef GAME_DLL

	ConVar nailspeed("ffdev_nailspeed", "600", FCVAR_CHEAT);
	ConVar naildamage("ffdev_naildamage", "10", FCVAR_CHEAT);
	ConVar nailgren_spittime( "ffdev_nailgren_spittime", "0.2", FCVAR_CHEAT );
	ConVar nailgren_angleoffset( "ffdev_nailgren_angleoffset", "360.0", FCVAR_CHEAT );
	//ConVar nailspread( "ffdev_nailgren_spread", "5.0", FCVAR_CHEAT );
	ConVar nailstreams( "ffdev_nailgren_streams", "10", FCVAR_CHEAT );
	//ConVar ffdev_nailgren_flatten("ffdev_nailgren_flatten", "100", FCVAR_CHEAT);

	ConVar ffdev_ng_nail_bounds("ffdev_ng_nail_bounds", "5.0", FCVAR_REPLICATED | FCVAR_CHEAT, "NG Nails bbox");
	ConVar ffdev_ng_visualizenails("ffdev_ng_visualizenails", "0", FCVAR_CHEAT, "Show NG nails trace");
	ConVar ffdev_ng_nail_length("ffdev_ng_nail_length", "5.0", FCVAR_CHEAT, "Length of NG nails");

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
			if ( ffdev_ng_visualizenails.GetBool() )
			{
				NDebugOverlay::Line(m_vecOrigin, m_vecOrigin + ( vecForward * ffdev_ng_nail_length.GetInt() ), 255, 255, 0, false, 5.0f);
				NDebugOverlay::SweptBox(m_vecOrigin, m_vecOrigin + ( vecForward * ffdev_ng_nail_length.GetInt() ), -Vector( 1.0f, 1.0f, 1.0f ) * ffdev_ng_nail_bounds.GetFloat(), Vector( 1.0f, 1.0f, 1.0f ) * ffdev_ng_nail_bounds.GetFloat(), m_vecAngles, 200, 100, 0, 100, 0.1f);
			}

			trace_t traceHit;
			UTIL_TraceHull( m_vecOrigin, m_vecOrigin + ( vecForward * ffdev_ng_nail_length.GetInt() ), -Vector( 1.0f, 1.0f, 1.0f ) * ffdev_ng_nail_bounds.GetFloat(), Vector( 1.0f, 1.0f, 1.0f ) * ffdev_ng_nail_bounds.GetFloat(), MASK_SHOT_HULL, NULL, COLLISION_GROUP_NONE, &traceHit );

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
							pPlayerTarget->TakeDamage( CTakeDamageInfo( pNailOwner, pNailGrenOwner, naildamage.GetInt(), DMG_BULLET ) );
						}
						else if( FF_IsDispenser( pTarget ) )
						{
							CFFDispenser *pDispenser = FF_ToDispenser( pTarget );
							if( pDispenser )
								pDispenser->TakeDamage( CTakeDamageInfo( pNailOwner, pNailGrenOwner, naildamage.GetInt() + 2, DMG_BULLET ) );
						}
						else /*if( FF_IsSentrygun( pTarget ) )*/
						{
							CFFSentryGun *pSentrygun = FF_ToSentrygun( pTarget );
							if( pSentrygun )
								pSentrygun->TakeDamage( CTakeDamageInfo( pNailOwner, pNailGrenOwner, naildamage.GetInt() + 2, DMG_BULLET ) );
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




class CFFGrenadeNail : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeNail, CFFGrenadeBase) 
	DECLARE_NETWORKCLASS(); 

	virtual void Precache();
	virtual const char *GetBounceSound() { return "NailGrenade.Bounce"; }

	virtual Class_T Classify( void ) { return CLASS_GREN_NAIL; } 

	virtual color32 GetColour() { color32 col = { 128, 225, 255, GREN_ALPHA_DEFAULT }; return col; }

#ifdef CLIENT_DLL
	CFFGrenadeNail() {}
	CFFGrenadeNail(const CFFGrenadeNail&) {}
#else
	DECLARE_DATADESC(); // Since we're adding new thinks etc
	virtual void Spawn();
	virtual void NailEmit();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);

private:
	void ShootNail( const Vector& vecOrigin, const QAngle& vecAngles );

protected:
	float	m_flNailSpit;
	float	m_flAngleOffset;

	// Contains all the nails currently being simulated
	CUtlVector< PseudoNail > m_NailsVector;

	float	m_flLastThinkTime;

#endif
};

#ifdef GAME_DLL
	BEGIN_DATADESC(CFFGrenadeNail) 
		DEFINE_THINKFUNC(NailEmit), 
	END_DATADESC() 
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeNail, DT_FFGrenadeNail)

BEGIN_NETWORK_TABLE(CFFGrenadeNail, DT_FFGrenadeNail)
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(ff_grenade_nail, CFFGrenadeNail);
PRECACHE_WEAPON_REGISTER(ff_grenade_nail);

#ifndef CLIENT_DLL
	//ConVar nailspeed("ffdev_nailspeed", "800", FCVAR_CHEAT);
	//ConVar naildamage("ffdev_naildamage", "8", FCVAR_CHEAT);
	//ConVar nailgren_spittime( "ffdev_nailgren_spittime", "0.3", FCVAR_CHEAT );
	//ConVar nailgren_angleoffset( "ffdev_nailgren_angleoffset", "360.0", FCVAR_CHEAT );
	//ConVar nailspread( "ffdev_nailgren_spread", "5.0", FCVAR_CHEAT );
	//ConVar nailstreams( "ffdev_nailgren_streams", "4", FCVAR_CHEAT );
	//ConVar ffdev_nailgren_flatten("ffdev_nailgren_flatten", "100", FCVAR_CHEAT);

	//ConVar ffdev_ng_nail_bounds("ffdev_ng_nail_bounds", "2.0", FCVAR_REPLICATED | FCVAR_CHEAT, "NG Nails bbox");
	//ConVar ffdev_ng_visualizenails("ffdev_ng_visualizenails", "1", FCVAR_CHEAT, "Show NG nails trace");
	//ConVar ffdev_ng_nail_length("ffdev_ng_nail_length", "5.0", FCVAR_CHEAT, "Length of NG nails");
#endif

	//extern ConVar ffdev_nail_bbox;
#define NAIL_BBOX 2.0f

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
	void CFFGrenadeNail::ShootNail( const Vector& vecOrigin, const QAngle& vecAngles )
	{
		/*
		// Create the nail and tell it it's a nail grenade nail
		// We don't do a clientside nail with this because that is being done itself
		CFFProjectileNail *pNail = CFFProjectileNail::CreateNail( this, vecOrigin, vecAngles, GetOwnerEntity(), naildamage.GetInt(), nailspeed.GetInt(), true );

		if (!pNail)
			return;

		pNail->m_bNailGrenadeNail = true;

		float flFlatten = ffdev_nailgren_flatten.GetFloat();
		Vector vecFlattened = Vector(flFlatten, flFlatten, 1.0f);
		pNail->SetSize(-vecFlattened, vecFlattened);
		*/
		
		m_NailsVector.AddToTail( PseudoNail( vecOrigin, vecAngles ) );
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
				m_NailsVector[i].UpdateNailPosition( nailspeed.GetInt() * ( gpGlobals->curtime - m_flLastThinkTime ) );
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

		SetAbsVelocity(Vector(0, 0, flRisingheight + 20 * sin(DEG2RAD(GetAbsAngles().y))));
		SetAbsAngles(GetAbsAngles() + QAngle(0, 15, 0));

		Vector vecOrigin = GetAbsOrigin();

		// Time to spit out nails again?
		if( m_flNailSpit < gpGlobals->curtime )
		{
			int iStreams = nailstreams.GetInt();
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

			// Default to 45 degrees
/*			int iNailSpreadInterval = 360 / ((iStreams == 0) ? 45 : iStreams);

			// Start position (0 to (360 - iNailSpreadInterval))
			int iNailStartPos = (rand() % iStreams) * iNailSpreadInterval;

			int iNailOffset = rand() % nailgren_angleoffset.GetInt();

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
		m_flLastThinkTime = gpGlobals->curtime;
	}




#endif


