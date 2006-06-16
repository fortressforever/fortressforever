//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "c_tracer.h"
#include "view.h"
#include "initializer.h"
#include "particles_simple.h"
#include "env_wind_shared.h"
#include "engine/IEngineTrace.h"
#include "engine/ivmodelinfo.h"
#include "precipitation_shared.h"
#include "fx_water.h"
#include "c_world.h"
#include "iviewrender.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar	cl_winddir			( "cl_winddir", "0", 0, "Weather effects wind direction angle" );
static ConVar	cl_windspeed		( "cl_windspeed", "0", 0, "Weather effects wind speed scalar" );

Vector g_vSplashColor( 0.5, 0.5, 0.5 );
float g_flSplashScale = 0.15;
float g_flSplashLifetime = 0.5f;
float g_flSplashAlpha = 0.3f;
ConVar r_RainSplashPercentage( "r_RainSplashPercentage", "20" ); // N% chance of a rain particle making a splash.


float GUST_INTERVAL_MIN = 1;
float GUST_INTERVAL_MAX = 2;

float GUST_LIFETIME_MIN = 1;
float GUST_LIFETIME_MAX = 3;

float MIN_SCREENSPACE_RAIN_WIDTH = 1;

ConVar r_RainHack( "r_RainHack", "0" );
ConVar r_RainRadius( "r_RainRadius", "1500" );
ConVar r_RainSideVel( "r_RainSideVel", "130", 0, "How much sideways velocity rain gets." );

ConVar r_RainSimulate( "r_RainSimulate", "1", 0, "Enable/disable rain simulation." );
ConVar r_DrawRain( "r_DrawRain", "1", FCVAR_CHEAT, "Enable/disable rain rendering." );
ConVar r_RainProfile( "r_RainProfile", "0", 0, "Enable/disable rain profiling." );


//-----------------------------------------------------------------------------
// Precipitation particle type
//-----------------------------------------------------------------------------

class CPrecipitationParticle
{
public:
	Vector	m_Pos;
	Vector	m_Velocity;
	float	m_SpawnTime;				// Note: Tweak with this to change lifetime
	float	m_Mass;
	float	m_Ramp;
	
	float	m_flCurLifetime;
	float	m_flMaxLifetime;
};
						  

class CClient_Precipitation;
static CUtlVector<CClient_Precipitation*> g_Precipitations;


//-----------------------------------------------------------------------------
// Precipitation base entity
//-----------------------------------------------------------------------------

class CClient_Precipitation : public C_BaseEntity
{
class CPrecipitationEffect;
friend class CClient_Precipitation::CPrecipitationEffect;

public:
	DECLARE_CLASS( CClient_Precipitation, C_BaseEntity );
	DECLARE_CLIENTCLASS();
	
	CClient_Precipitation();
	virtual ~CClient_Precipitation();

	// Inherited from C_BaseEntity
	virtual void Precache( );

	void Render();

private:

	// Creates a single particle
	CPrecipitationParticle* CreateParticle();

	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual void ClientThink();

	void Simulate( float dt );

	// Renders the particle
	void RenderParticle( CPrecipitationParticle* pParticle, CMeshBuilder &mb );

	void CreateWaterSplashes();

	// Emits the actual particles
	void EmitParticles( float fTimeDelta );
	
	// Computes where we're gonna emit
	bool ComputeEmissionArea( Vector& origin, Vector2D& size );

	// Gets the tracer width and speed
	float GetWidth() const;
	float GetLength() const;
	float GetSpeed() const;

	// Gets the remaining lifetime of the particle
	float GetRemainingLifetime( CPrecipitationParticle* pParticle ) const;

	// Computes the wind vector
	static void ComputeWindVector( );

	// simulation methods
	bool SimulateRain( CPrecipitationParticle* pParticle, float dt );
	bool SimulateSnow( CPrecipitationParticle* pParticle, float dt );

	// Information helpful in creating and rendering particles
	IMaterial		*m_MatHandle;	// material used 

	float			m_Color[4];		// precip color
	float			m_Lifetime;		// Precip lifetime
	float			m_InitialRamp;	// Initial ramp value
	float			m_Speed;		// Precip speed
	float			m_Width;		// Tracer width
	float			m_Remainder;	// particles we should render next time
	PrecipitationType_t	m_nPrecipType;			// Precip type
	float			m_flHalfScreenWidth;	// Precalculated each frame.

	// Some state used in rendering and simulation
	// Used to modify the rain density and wind from the console
	static ConVar s_raindensity;
	static ConVar s_rainwidth;
	static ConVar s_rainlength;
	static ConVar s_rainspeed;

	static Vector s_WindVector;			// Stores the wind speed vector
	
	CUtlLinkedList<CPrecipitationParticle> m_Particles;
	CUtlVector<Vector> m_Splashes;

private:
	CClient_Precipitation( const CClient_Precipitation & ); // not defined, not accessible
};


// Just receive the normal data table stuff
IMPLEMENT_CLIENTCLASS_DT(CClient_Precipitation, DT_Precipitation, CPrecipitation)
	RecvPropInt( RECVINFO( m_nPrecipType ) )
END_RECV_TABLE()



void DrawPrecipitation()
{
	for ( int i=0; i < g_Precipitations.Count(); i++ )
	{
		g_Precipitations[i]->Render();
	}
}


//-----------------------------------------------------------------------------
// determines if a weather particle has hit something other than air
//-----------------------------------------------------------------------------
static bool IsInAir( const Vector& position )
{
	int contents = enginetrace->GetPointContents( position ); 	
	return (contents & CONTENTS_SOLID) == 0;
}


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

ConVar CClient_Precipitation::s_raindensity( "r_raindensity","0.001");
ConVar CClient_Precipitation::s_rainwidth( "r_rainwidth", "0.5" );
ConVar CClient_Precipitation::s_rainlength( "r_rainlength", "0.1f" );
ConVar CClient_Precipitation::s_rainspeed( "r_rainspeed", "600.0f" );
ConVar r_rainalpha( "r_rainalpha", "0.4" );
ConVar r_rainalphapow( "r_rainalphapow", "0.8" );


Vector CClient_Precipitation::s_WindVector;		// Stores the wind speed vector


void CClient_Precipitation::OnDataChanged( DataUpdateType_t updateType )
{
	// Simulate every frame.
	if ( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}

	BaseClass::OnDataChanged( updateType );
}


void CClient_Precipitation::ClientThink()
{
	Simulate( gpGlobals->frametime );
}


//-----------------------------------------------------------------------------
//
// Utility methods for the various simulation functions
//
//-----------------------------------------------------------------------------
inline bool CClient_Precipitation::SimulateRain( CPrecipitationParticle* pParticle, float dt )
{
	if (GetRemainingLifetime( pParticle ) < 0.0f)
		return false;

	Vector vOldPos = pParticle->m_Pos;

	// Update position
	VectorMA( pParticle->m_Pos, dt, pParticle->m_Velocity, 
				pParticle->m_Pos );

	// wind blows rain around
	for ( int i = 0 ; i < 2 ; i++ )
	{
		if ( pParticle->m_Velocity[i] < s_WindVector[i] )
		{
			pParticle->m_Velocity[i] += ( 5 / pParticle->m_Mass );

			// clamp
			if ( pParticle->m_Velocity[i] > s_WindVector[i] )
				pParticle->m_Velocity[i] = s_WindVector[i];
		}
		else if (pParticle->m_Velocity[i] > s_WindVector[i] )
		{
			pParticle->m_Velocity[i] -= ( 5 / pParticle->m_Mass );

			// clamp.
			if ( pParticle->m_Velocity[i] < s_WindVector[i] )
				pParticle->m_Velocity[i] = s_WindVector[i];
		}
	}

	// No longer in the air? punt.
	if ( !IsInAir( pParticle->m_Pos ) )
	{
		// Possibly make a splash if we hit a water surface and it's in front of the view.
		if ( m_Splashes.Count() < 20 )
		{
			if ( RandomInt( 0, 100 ) < r_RainSplashPercentage.GetInt() )
			{
				trace_t trace;
				UTIL_TraceLine(vOldPos, pParticle->m_Pos, MASK_WATER, NULL, COLLISION_GROUP_NONE, &trace);
				if( trace.fraction < 1 )
				{
					m_Splashes.AddToTail( trace.endpos );
				}
			}
		}

		// Tell the framework it's time to remove the particle from the list
		return false;
	}

	// We still want this particle
	return true;
}


inline bool CClient_Precipitation::SimulateSnow( CPrecipitationParticle* pParticle, float dt )
{
	if ( IsInAir( pParticle->m_Pos ) )
	{
		// Update position
		VectorMA( pParticle->m_Pos, dt, pParticle->m_Velocity, 
					pParticle->m_Pos );

		// wind blows rain around
		for ( int i = 0 ; i < 2 ; i++ )
		{
			if ( pParticle->m_Velocity[i] < s_WindVector[i] )
			{
				pParticle->m_Velocity[i] += ( 5.0f / pParticle->m_Mass );

				// accelerating flakes get a trail
				pParticle->m_Ramp = 0.5f;

				// clamp
				if ( pParticle->m_Velocity[i] > s_WindVector[i] )
					pParticle->m_Velocity[i] = s_WindVector[i];
			}
			else if (pParticle->m_Velocity[i] > s_WindVector[i] )
			{
				pParticle->m_Velocity[i] -= ( 5.0f / pParticle->m_Mass );

				// accelerating flakes get a trail
				pParticle->m_Ramp = 0.5f;

				// clamp.
				if ( pParticle->m_Velocity[i] < s_WindVector[i] )
					pParticle->m_Velocity[i] = s_WindVector[i];
			}
		}

		return true;
	}


	// Kill the particle immediately!
	return false;
}


void CClient_Precipitation::Simulate( float dt )
{
	// NOTE: When client-side prechaching works, we need to remove this
	Precache();

	m_flHalfScreenWidth = (float)ScreenWidth() / 2;

	// Our sim methods needs dt	and wind vector
	if ( dt )
	{
		ComputeWindVector( );
	}

	// calculate the max amount of time it will take this flake to fall.
	// This works if we assume the wind doesn't have a z component
	if ( r_RainHack.GetInt() )
		m_Lifetime = (GetClientWorldEntity()->m_WorldMaxs[2] - GetClientWorldEntity()->m_WorldMins[2]) / m_Speed;
	else
		m_Lifetime = (WorldAlignMaxs()[2] - WorldAlignMins()[2]) / m_Speed;


	if ( !r_RainSimulate.GetInt() )
		return;

	CFastTimer timer;
	timer.Start();

	// Emit new particles
	EmitParticles( dt );

	// Simulate all the particles.
	int iNext;
	if ( m_nPrecipType == PRECIPITATION_TYPE_RAIN )
	{
		for ( int i=m_Particles.Head(); i != m_Particles.InvalidIndex(); i=iNext )
		{
			iNext = m_Particles.Next( i );
			if ( !SimulateRain( &m_Particles[i], dt ) )
				m_Particles.Remove( i );
		}
	}
	else
	{
		for ( int i=m_Particles.Head(); i != m_Particles.InvalidIndex(); i=iNext )
		{
			iNext = m_Particles.Next( i );
			if ( !SimulateSnow( &m_Particles[i], dt ) )
				m_Particles.Remove( i );
		}
	}

	if ( r_RainProfile.GetInt() )
	{
		timer.End();
		engine->Con_NPrintf( 15, "Rain simulation: %du (%d tracers)", timer.GetDuration().GetMicroseconds(), m_Particles.Count() );
	}
}


//-----------------------------------------------------------------------------
// tracer rendering
//-----------------------------------------------------------------------------

inline void CClient_Precipitation::RenderParticle( CPrecipitationParticle* pParticle, CMeshBuilder &mb )
{
	float scale;
	Vector start, delta;

	// make streaks 0.1 seconds long, but prevent from going past end
	float lifetimeRemaining = GetRemainingLifetime( pParticle );
	if (lifetimeRemaining >= GetLength())
		scale = GetLength() * pParticle->m_Ramp;
	else
		scale = lifetimeRemaining * pParticle->m_Ramp;
	
	// NOTE: We need to do everything in screen space
	Vector3DMultiplyPosition( CurrentWorldToViewMatrix(), pParticle->m_Pos, start );
	if ( start.z > -1 )
		return;

	Vector3DMultiply( CurrentWorldToViewMatrix(), pParticle->m_Velocity, delta );

	// give a spiraling pattern to snow particles
	if ( m_nPrecipType == PRECIPITATION_TYPE_SNOW )
	{
		Vector spiral, camSpiral;
		float s, c;

		if ( pParticle->m_Mass > 1.0f )
		{
			SinCos( gpGlobals->curtime * M_PI * (1+pParticle->m_Mass * 0.1f) + 
					pParticle->m_Mass * 5.0f, &s , &c );

			// only spiral particles with a mass > 1, so some fall straight down
			spiral[0] = 28 * c;
			spiral[1] = 28 * s;
			spiral[2] = 0.0f;

			Vector3DMultiply( CurrentWorldToViewMatrix(), spiral, camSpiral );

			// X and Y are measured in world space; need to convert to camera space
			VectorAdd( start, camSpiral, start );
			VectorAdd( delta, camSpiral, delta );
		}

		// shrink the trails on spiraling flakes.
		pParticle->m_Ramp = 0.3f;
	}

	delta[0] *= scale;
	delta[1] *= scale;
	delta[2] *= scale;

	// See c_tracer.* for this method
	float flAlpha = r_rainalpha.GetFloat();
	float flWidth = GetWidth();

	float flScreenSpaceWidth = flWidth * m_flHalfScreenWidth / -start.z;
	if ( flScreenSpaceWidth < MIN_SCREENSPACE_RAIN_WIDTH )
	{
		// Make the rain tracer at least the min size, but fade its alpha the smaller it gets.
		flAlpha *= flScreenSpaceWidth / MIN_SCREENSPACE_RAIN_WIDTH;
		flWidth = MIN_SCREENSPACE_RAIN_WIDTH * -start.z / m_flHalfScreenWidth;
	}
	flAlpha = pow( flAlpha, r_rainalphapow.GetFloat() );

	float flColor[4] = { 1, 1, 1, flAlpha };
	Tracer_Draw( &mb, start, delta, flWidth, flColor, 1 );
}


void CClient_Precipitation::CreateWaterSplashes()
{
	for ( int i=0; i < m_Splashes.Count(); i++ )
	{
		Vector vSplash = m_Splashes[i];
		
		if ( CurrentViewForward().Dot( vSplash - CurrentViewOrigin() ) > 1 )
		{
			FX_WaterRipple( vSplash, g_flSplashScale, &g_vSplashColor, g_flSplashLifetime, g_flSplashAlpha );
		}
	}
	m_Splashes.Purge();
}


void CClient_Precipitation::Render()
{
	if ( !r_DrawRain.GetInt() )
		return;

	// Don't render in monitors or in reflections or refractions.
	if ( view->GetDrawFlags() & (DF_MONITOR | DF_RENDER_REFLECTION | DF_RENDER_REFRACTION) )
		return;

	// Create any queued up water splashes.
	CreateWaterSplashes();

	
	CFastTimer timer;
	timer.Start();

	// We want to do our calculations in view space.
	VMatrix	tempView;
	materials->GetMatrix( MATERIAL_VIEW, &tempView );
	materials->MatrixMode( MATERIAL_VIEW );
	materials->LoadIdentity();

	// Force the user clip planes to use the old view matrix
	materials->EnableUserClipTransformOverride( true );
	materials->UserClipTransform( tempView );

	// Draw all the rain tracers.
	materials->Bind( m_MatHandle );
	IMesh *pMesh = materials->GetDynamicMesh();
	if ( pMesh )
	{
		CMeshBuilder mb;
		mb.Begin( pMesh, MATERIAL_QUADS, m_Particles.Count() );

		for ( int i=m_Particles.Head(); i != m_Particles.InvalidIndex(); i=m_Particles.Next( i ) )
		{
			CPrecipitationParticle *p = &m_Particles[i];
			RenderParticle( p, mb );
		}

		mb.End( false, true );
	}

	materials->EnableUserClipTransformOverride( false );
	materials->MatrixMode( MATERIAL_VIEW );
	materials->LoadMatrix( tempView );

	if ( r_RainProfile.GetInt() )
	{
		timer.End();
		engine->Con_NPrintf( 16, "Rain render    : %du", timer.GetDuration().GetMicroseconds() );
	}
}


//-----------------------------------------------------------------------------
// Constructor, destructor
//-----------------------------------------------------------------------------

CClient_Precipitation::CClient_Precipitation() : m_Remainder(0.0f)
{
	m_nPrecipType = PRECIPITATION_TYPE_RAIN;
	m_MatHandle = INVALID_MATERIAL_HANDLE;
	m_flHalfScreenWidth = 1;
	
	g_Precipitations.AddToTail( this );
}

CClient_Precipitation::~CClient_Precipitation()
{
	g_Precipitations.FindAndRemove( this );
}

//-----------------------------------------------------------------------------
// Precache data
//-----------------------------------------------------------------------------

#define SNOW_SPEED	80.0f
#define RAIN_SPEED	425.0f

#define RAIN_TRACER_WIDTH 0.35f
#define SNOW_TRACER_WIDTH 0.7f

void CClient_Precipitation::Precache( )
{
	if ( !m_MatHandle )
	{
		// Compute precipitation emission speed
		switch( m_nPrecipType )
		{
		case PRECIPITATION_TYPE_SNOW:
			m_Speed	= SNOW_SPEED;
			m_MatHandle = materials->FindMaterial( "particle/snow", TEXTURE_GROUP_CLIENT_EFFECTS );
			m_InitialRamp = 0.6f;
			m_Width = SNOW_TRACER_WIDTH;
			break;

		default:
			Assert( m_nPrecipType == PRECIPITATION_TYPE_RAIN );
			m_Speed	= RAIN_SPEED;
			m_MatHandle = materials->FindMaterial( "particle/rain", TEXTURE_GROUP_CLIENT_EFFECTS );
			m_InitialRamp = 1.0f;
			m_Color[3] = 1.0f;	// make translucent
			m_Width = RAIN_TRACER_WIDTH;
			break;
		}

		// Store off the color
		m_Color[0] = 1.0f;
		m_Color[1] = 1.0f;
		m_Color[2] = 1.0f;
	}
}


//-----------------------------------------------------------------------------
// Gets the tracer width and speed
//-----------------------------------------------------------------------------

inline float CClient_Precipitation::GetWidth() const
{
//	return m_Width;
	return s_rainwidth.GetFloat();
}

inline float CClient_Precipitation::GetLength() const
{
//	return m_Length;
	return s_rainlength.GetFloat();
}

inline float CClient_Precipitation::GetSpeed() const
{
//	return m_Speed;
	return s_rainspeed.GetFloat();
}


//-----------------------------------------------------------------------------
// Gets the remaining lifetime of the particle
//-----------------------------------------------------------------------------

inline float CClient_Precipitation::GetRemainingLifetime( CPrecipitationParticle* pParticle ) const
{
	float timeSinceSpawn = gpGlobals->curtime - pParticle->m_SpawnTime;
	return m_Lifetime - timeSinceSpawn;
}

//-----------------------------------------------------------------------------
// Creates a particle
//-----------------------------------------------------------------------------

inline CPrecipitationParticle* CClient_Precipitation::CreateParticle()
{
	int i = m_Particles.AddToTail();
	CPrecipitationParticle* pParticle = &m_Particles[i];

	pParticle->m_SpawnTime = gpGlobals->curtime;
	pParticle->m_Ramp = m_InitialRamp;

	return pParticle;
}


//-----------------------------------------------------------------------------
// Compute the emission area
//-----------------------------------------------------------------------------

bool CClient_Precipitation::ComputeEmissionArea( Vector& origin, Vector2D& size )
{
	// FIXME: Compute the precipitation area based on computational power
	float emissionSize = r_RainRadius.GetFloat();	// size of box to emit particles in

	Vector vMins = WorldAlignMins();
	Vector vMaxs = WorldAlignMaxs();
	if ( r_RainHack.GetInt() )
	{
		vMins = GetClientWorldEntity()->m_WorldMins;
		vMaxs = GetClientWorldEntity()->m_WorldMaxs;
	}

	// calculate a volume around the player to snow in. Intersect this big magic
	// box around the player with the volume of the current environmental ent.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return false;

	// Determine how much time it'll take a falling particle to hit the player
	float emissionHeight = min( vMaxs[2], pPlayer->GetAbsOrigin()[2] + 512 );
	float distToFall = emissionHeight - pPlayer->GetAbsOrigin()[2];
	float fallTime = distToFall / GetSpeed();
	
	// Based on the windspeed, figure out the center point of the emission
	Vector2D center;
	center[0] = pPlayer->GetAbsOrigin()[0] - fallTime * s_WindVector[0];
	center[1] = pPlayer->GetAbsOrigin()[1] - fallTime * s_WindVector[1];

	Vector2D lobound, hibound;
	lobound[0] = center[0] - emissionSize * 0.5f;
	lobound[1] = center[1] - emissionSize * 0.5f;
	hibound[0] = lobound[0] + emissionSize;
	hibound[1] = lobound[1] + emissionSize;

	// Cull non-intersecting.
	if ( ( vMaxs[0] < lobound[0] ) || ( vMaxs[1] < lobound[1] ) ||
		 ( vMins[0] > hibound[0] ) || ( vMins[1] > hibound[1] ) )
		return false;

	origin[0] = max( vMins[0], lobound[0] );
	origin[1] = max( vMins[1], lobound[1] );
	origin[2] = emissionHeight;

	hibound[0] = min( vMaxs[0], hibound[0] );
	hibound[1] = min( vMaxs[1], hibound[1] );

	size[0] = hibound[0] - origin[0];
	size[1] = hibound[1] - origin[1];

	return true;
}


//-----------------------------------------------------------------------------
// emit the precipitation particles
//-----------------------------------------------------------------------------

void CClient_Precipitation::EmitParticles( float fTimeDelta )
{
	Vector2D size;
	Vector vel, org;
	
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return;
	Vector vPlayerCenter = pPlayer->WorldSpaceCenter();

	// Compute where to emit
	if (!ComputeEmissionArea( org, size ))
		return;

	// clamp this to prevent creating a bunch of rain or snow at one time.
	if( fTimeDelta > 0.075f )
		fTimeDelta = 0.075f;

	// FIXME: Compute the precipitation density based on computational power
	float density = s_raindensity.GetFloat();
	if (density > 0.01f) 
		density = 0.01f;

	// Compute number of particles to emit based on precip density and emission area and dt
	float fParticles = size[0] * size[1] * density * fTimeDelta + m_Remainder; 
	int cParticles = (int)fParticles;
	m_Remainder = fParticles - cParticles;

	// calculate the max amount of time it will take this flake to fall.
	// This works if we assume the wind doesn't have a z component
	VectorCopy( s_WindVector, vel );
	vel[2] -= GetSpeed();

	// Emit all the particles
	for ( int i = 0 ; i < cParticles ; i++ )
	{									 
		Vector vParticlePos = org;
		vParticlePos[ 0 ] += size[ 0 ] * random->RandomFloat(0, 1);
		vParticlePos[ 1 ] += size[ 1 ] * random->RandomFloat(0, 1);

		// Figure out where the particle should lie in Z by tracing a line from the player's height up to the 
		// desired height and making sure it doesn't hit a wall.
		Vector vPlayerHeight = vParticlePos;
		vPlayerHeight.z = vPlayerCenter.z;

		trace_t trace;
		UTIL_TraceLine( vPlayerHeight, vParticlePos, MASK_SOLID_BRUSHONLY, NULL, COLLISION_GROUP_NONE, &trace );
		if ( trace.fraction < 1 )
		{
			// If we hit a brush, then don't spawn the particle.
			if ( trace.surface.flags & SURF_SKY )
			{
				vParticlePos = trace.endpos;
			}
			else
			{
				continue;
			}
		}

		// Create the particle
		CPrecipitationParticle* p = CreateParticle();
		if (!p) 
			return;

		VectorCopy( vel, p->m_Velocity );
		p->m_Pos = vParticlePos;

		p->m_Velocity[ 0 ] += random->RandomFloat(-r_RainSideVel.GetInt(), r_RainSideVel.GetInt());
		p->m_Velocity[ 1 ] += random->RandomFloat(-r_RainSideVel.GetInt(), r_RainSideVel.GetInt());

		p->m_Mass = random->RandomFloat( 0.5, 1.5 );
	}
}


//-----------------------------------------------------------------------------
// Computes the wind vector
//-----------------------------------------------------------------------------

void CClient_Precipitation::ComputeWindVector( )
{
	// Compute the wind direction
	QAngle windangle( 0, cl_winddir.GetFloat(), 0 );	// used to turn wind yaw direction into a vector

	// Randomize the wind angle and speed slightly to get us a little variation
	windangle[1] = windangle[1] + random->RandomFloat( -10, 10 );
	float windspeed = cl_windspeed.GetFloat() * (1.0 + random->RandomFloat( -0.2, 0.2 ));

	AngleVectors( windangle, &s_WindVector );
	VectorScale( s_WindVector, windspeed, s_WindVector );
}


CHandle<CClient_Precipitation> g_pPrecipHackEnt;

class CPrecipHack : public CAutoGameSystem
{
public:
	CPrecipHack()
	{
		m_bLevelInitted = false;
	}

	virtual void LevelInitPostEntity()
	{
		if ( r_RainHack.GetInt() )
		{
			CClient_Precipitation *pPrecipHackEnt = new CClient_Precipitation;
			pPrecipHackEnt->InitializeAsClientEntity( NULL, RENDER_GROUP_TRANSLUCENT_ENTITY );
			g_pPrecipHackEnt = pPrecipHackEnt;
		}
		m_bLevelInitted = true;
	}
	
	virtual void LevelShutdownPreEntity()
	{
		if ( r_RainHack.GetInt() && g_pPrecipHackEnt )
		{
			g_pPrecipHackEnt->Release();
		}
		m_bLevelInitted = false;
	}

	virtual void Update( float frametime )
	{
		// Handle changes to the cvar at runtime.
		if ( m_bLevelInitted )
		{
			if ( r_RainHack.GetInt() && !g_pPrecipHackEnt )
				LevelInitPostEntity();
			else if ( !r_RainHack.GetInt() && g_pPrecipHackEnt )
				LevelShutdownPreEntity();
		}
	}

	bool m_bLevelInitted;
};
CPrecipHack g_PrecipHack;


//-----------------------------------------------------------------------------
// EnvWind - global wind info
//-----------------------------------------------------------------------------
class C_EnvWind : public C_BaseEntity
{
public:
	C_EnvWind();

	DECLARE_CLIENTCLASS();
	DECLARE_CLASS( C_EnvWind, C_BaseEntity );

	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual bool	ShouldDraw( void ) { return false; }

	virtual void	ClientThink( );

private:
	C_EnvWind( const C_EnvWind & );

	CEnvWindShared m_EnvWindShared;
};

// Receive datatables
BEGIN_RECV_TABLE_NOBASE(CEnvWindShared, DT_EnvWindShared)
	RecvPropInt		(RECVINFO(m_iMinWind)),
	RecvPropInt		(RECVINFO(m_iMaxWind)),
	RecvPropInt		(RECVINFO(m_iMinGust)),
	RecvPropInt		(RECVINFO(m_iMaxGust)),
	RecvPropFloat	(RECVINFO(m_flMinGustDelay)),
	RecvPropFloat	(RECVINFO(m_flMaxGustDelay)),
	RecvPropInt		(RECVINFO(m_iGustDirChange)),
	RecvPropInt		(RECVINFO(m_iWindSeed)),
	RecvPropInt		(RECVINFO(m_iInitialWindDir)),
	RecvPropFloat	(RECVINFO(m_flInitialWindSpeed)),
	RecvPropFloat	(RECVINFO(m_flStartTime)),
//	RecvPropInt		(RECVINFO(m_iszGustSound)),
END_RECV_TABLE()

IMPLEMENT_CLIENTCLASS_DT( C_EnvWind, DT_EnvWind, CEnvWind )
	RecvPropDataTable(RECVINFO_DT(m_EnvWindShared), 0, &REFERENCE_RECV_TABLE(DT_EnvWindShared)),
END_RECV_TABLE()


C_EnvWind::C_EnvWind()
{
}

//-----------------------------------------------------------------------------
// Post data update!
//-----------------------------------------------------------------------------
void C_EnvWind::OnDataChanged( DataUpdateType_t updateType )
{
	// Whenever we get an update, reset the entire state.
	// Note that the fields have already been stored by the datatables,
	// but there's still work to be done in the init block
	m_EnvWindShared.Init( entindex(), m_EnvWindShared.m_iWindSeed, 
		m_EnvWindShared.m_flStartTime, m_EnvWindShared.m_iInitialWindDir,
		m_EnvWindShared.m_flInitialWindSpeed );

	SetNextClientThink(0.0f);
}

void C_EnvWind::ClientThink( )
{
	// Update the wind speed
	float flNextThink = m_EnvWindShared.WindThink( gpGlobals->curtime );
	SetNextClientThink(flNextThink);
}



//==================================================
// EmberParticle
//==================================================

class CEmberEmitter : public CSimpleEmitter
{
public:
							CEmberEmitter( const char *pDebugName );
	static CSmartPtr<CEmberEmitter>	Create( const char *pDebugName );
	virtual void			UpdateVelocity( SimpleParticle *pParticle, float timeDelta );
	virtual Vector			UpdateColor( const SimpleParticle *pParticle );

private:
							CEmberEmitter( const CEmberEmitter & );
};


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : fTimeDelta - 
// Output : Vector
//-----------------------------------------------------------------------------
CEmberEmitter::CEmberEmitter( const char *pDebugName ) : CSimpleEmitter( pDebugName )
{
}


CSmartPtr<CEmberEmitter> CEmberEmitter::Create( const char *pDebugName )
{
	return new CEmberEmitter( pDebugName );
}


void CEmberEmitter::UpdateVelocity( SimpleParticle *pParticle, float timeDelta )
{
	float	speed = VectorNormalize( pParticle->m_vecVelocity );
	Vector	offset;

	speed -= ( 1.0f * timeDelta );

	offset.Random( -0.025f, 0.025f );
	offset[2] = 0.0f;

	pParticle->m_vecVelocity += offset;
	VectorNormalize( pParticle->m_vecVelocity );

	pParticle->m_vecVelocity *= speed;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pParticle - 
//			timeDelta - 
//-----------------------------------------------------------------------------
Vector CEmberEmitter::UpdateColor( const SimpleParticle *pParticle )
{
	Vector	color;
	float	ramp = 1.0f - ( pParticle->m_flLifetime / pParticle->m_flDieTime );

	color[0] = ( (float) pParticle->m_uchColor[0] * ramp ) / 255.0f;
	color[1] = ( (float) pParticle->m_uchColor[1] * ramp ) / 255.0f;
	color[2] = ( (float) pParticle->m_uchColor[2] * ramp ) / 255.0f;

	return color;
}

//==================================================
// C_Embers
//==================================================

class C_Embers : public C_BaseEntity
{
public:
	DECLARE_CLIENTCLASS();
	DECLARE_CLASS( C_Embers, C_BaseEntity );

					C_Embers();
					~C_Embers();

	void	Start( void );

	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual bool	ShouldDraw( void );
	virtual void	AddEntity( void );

	//Server-side
	int		m_nDensity;
	int		m_nLifetime;
	int		m_nSpeed;
	bool	m_bEmit;

protected:

	void	SpawnEmber( void );

	PMaterialHandle		m_hMaterial;
	TimedEvent			m_tParticleSpawn;
	CSmartPtr<CEmberEmitter> m_pEmitter;

	bool	m_bNeedToSetOrigin;	// |-- Mirv
};

//Receive datatable
IMPLEMENT_CLIENTCLASS_DT( C_Embers, DT_Embers, CEmbers )
	RecvPropInt( RECVINFO( m_nDensity ) ),
	RecvPropInt( RECVINFO( m_nLifetime ) ),
	RecvPropInt( RECVINFO( m_nSpeed ) ),
	RecvPropInt( RECVINFO( m_bEmit ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bnewentity - 
//-----------------------------------------------------------------------------
C_Embers::C_Embers()
{
	m_pEmitter = CEmberEmitter::Create( "C_Embers" );

	// --> Mirv: Moved into Start; constructor is too early for GetAbsOrigin()
	//m_pEmitter->SetSortOrigin( GetAbsOrigin() );
	m_bNeedToSetOrigin = true;
	// <-- Mirv
}

C_Embers::~C_Embers()
{
}

void C_Embers::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		Start();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_Embers::ShouldDraw()
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Embers::Start( void )
{
	//Various setup info
	m_tParticleSpawn.Init( m_nDensity );
	
	m_hMaterial	= m_pEmitter->GetPMaterial( "particle/fire" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Embers::AddEntity( void ) 
{
	if ( m_bEmit == false )
		return;

	// --> Mirv: Moved here from the constructor
	if (m_bNeedToSetOrigin)
		m_pEmitter->SetSortOrigin(GetAbsOrigin());
	// <-- Mirv

	float tempDelta = gpGlobals->frametime;

	while( m_tParticleSpawn.NextEvent( tempDelta ) )
	{
		SpawnEmber();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Embers::SpawnEmber( void )
{
	Vector	offset, mins, maxs;
	
	modelinfo->GetModelBounds( GetModel(), mins, maxs );

	//Setup our spawn position
	offset[0] = random->RandomFloat( mins[0], maxs[0] );
	offset[1] = random->RandomFloat( mins[1], maxs[1] );
	offset[2] = random->RandomFloat( mins[2], maxs[2] );

	//Spawn the particle
	SimpleParticle	*sParticle = (SimpleParticle *) m_pEmitter->AddParticle( sizeof( SimpleParticle ), m_hMaterial, offset );

	if (sParticle == NULL)
		return;

	float	cScale = random->RandomFloat( 0.75f, 1.0f );

	//Set it up
	sParticle->m_flLifetime = 0.0f;
	sParticle->m_flDieTime	= m_nLifetime;

	sParticle->m_uchColor[0]	= m_clrRender->r * cScale;
	sParticle->m_uchColor[1]	= m_clrRender->g * cScale;
	sParticle->m_uchColor[2]	= m_clrRender->b * cScale;
	sParticle->m_uchStartAlpha	= 255;
	sParticle->m_uchEndAlpha	= 0;
	sParticle->m_uchStartSize	= 1;
	sParticle->m_uchEndSize		= 0;

	// --> Mirv: Initialise roll and rolldelta
	sParticle->m_flRoll			= 0;
	sParticle->m_flRollDelta	= 0;
	// <-- Mirv
	
	//Set the velocity
	Vector	velocity;

	AngleVectors( GetAbsAngles(), &velocity );

	sParticle->m_vecVelocity = velocity * m_nSpeed;

	sParticle->m_vecVelocity[0]	+= random->RandomFloat( -(m_nSpeed/8), (m_nSpeed/8) );
	sParticle->m_vecVelocity[1]	+= random->RandomFloat( -(m_nSpeed/8), (m_nSpeed/8) );
	sParticle->m_vecVelocity[2]	+= random->RandomFloat( -(m_nSpeed/8), (m_nSpeed/8) );

	UpdateVisibility();
}

//-----------------------------------------------------------------------------
// Quadratic spline beam effect 
//-----------------------------------------------------------------------------
#include "beamdraw.h"

class C_QuadraticBeam : public C_BaseEntity
{
public:
	DECLARE_CLIENTCLASS();
	DECLARE_CLASS( C_QuadraticBeam, C_BaseEntity );

	//virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual bool	ShouldDraw( void ) { return true; }
	virtual int		DrawModel( int );

	virtual void	GetRenderBounds( Vector& mins, Vector& maxs )
	{
		ClearBounds( mins, maxs );
		AddPointToBounds( vec3_origin, mins, maxs );
		AddPointToBounds( m_targetPosition, mins, maxs );
		AddPointToBounds( m_controlPosition, mins, maxs );
		mins -= GetRenderOrigin();
		maxs -= GetRenderOrigin();
	}

protected:

	Vector		m_targetPosition;
	Vector		m_controlPosition;
	float		m_scrollRate;
	float		m_flWidth;
};

//Receive datatable
IMPLEMENT_CLIENTCLASS_DT( C_QuadraticBeam, DT_QuadraticBeam, CEnvQuadraticBeam )
	RecvPropVector( RECVINFO(m_targetPosition) ),
	RecvPropVector( RECVINFO(m_controlPosition) ),
	RecvPropFloat( RECVINFO(m_scrollRate) ),
	RecvPropFloat( RECVINFO(m_flWidth) ),
END_RECV_TABLE()

Vector Color32ToVector( const color32 &color )
{
	return Vector( color.r * (1.0/255.0f), color.g * (1.0/255.0f), color.b * (1.0/255.0f) );
}

int	C_QuadraticBeam::DrawModel( int )
{
	Draw_SetSpriteTexture( GetModel(), 0, GetRenderMode() );
	Vector color = Color32ToVector( GetRenderColor() );
	DrawBeamQuadratic( GetRenderOrigin(), m_controlPosition, m_targetPosition, m_flWidth, color, gpGlobals->curtime*m_scrollRate );
	return 1;
}

