//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hltvcamera.h"
#include "cdll_client_int.h"
#include "util_shared.h"
#include "prediction.h"
#include "movevars_shared.h"
#include "in_buttons.h"
#include <KeyValues.h>

#ifdef CSTRIKE_DLL
	#include "c_cs_player.h"
#endif


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define CHASE_CAM_DISTANCE		96.0f
#define WALL_OFFSET				6.0f

static Vector WALL_MIN(-WALL_OFFSET,-WALL_OFFSET,-WALL_OFFSET);
static Vector WALL_MAX(WALL_OFFSET,WALL_OFFSET,WALL_OFFSET);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static C_HLTVCamera s_HLTVCamera;

C_HLTVCamera *HLTVCamera() { return &s_HLTVCamera; }

C_HLTVCamera::C_HLTVCamera()
{

}

C_HLTVCamera::~C_HLTVCamera()
{

}

void C_HLTVCamera::Init()
{
	gameeventmanager->AddListener( this, "game_newmap", false );
	gameeventmanager->AddListener( this, "hltv_cameraman", false );
	gameeventmanager->AddListener( this, "hltv_fixed", false );
	gameeventmanager->AddListener( this, "hltv_chase", false );
	gameeventmanager->AddListener( this, "hltv_fov", false );
	gameeventmanager->AddListener( this, "hltv_inertia", false );
	
	Reset();
}

void C_HLTVCamera::Shutdown()
{
	gameeventmanager->RemoveListener( this );
}

void C_HLTVCamera::Reset()
{
	m_nCameraMode = OBS_MODE_FIXED;
	m_iCameraMan  = 0;
	m_iTraget1 = m_iTraget2 = 0;
	m_flFOV = 90;
	m_flDistance = m_flLastDistance = 96.0f;
	m_flInertia = 10.0f;
	m_flPhi = 0;
	m_flTheta = 0;
	m_flOffset = 0;

	m_vCamOrigin.Init();
	m_aCamAngle.Init();
}

void C_HLTVCamera::CalcChaseCamView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov )
{
	bool bManual = false;	// chase camera controlled manually

	if ( m_iTraget1 == 0 )
		return;

	// get primary target, also translates to ragdoll
	C_BaseEntity *target1 = GetObserverTarget();

	if ( !target1 )
		return;

	// get secondary target if set
	C_BaseEntity *target2 = NULL;

	if ( m_iTraget2 > 0 )
	{
		target2 = ClientEntityList().GetBaseEntity( m_iTraget2 );

		if ( target2 )
		{
			// if target is out PVS and not dead, it's not valid
			if ( target2->IsDormant() && target2->IsAlive() )
				target2 = NULL;

		}
	}

	Vector targetOrigin, cameraOrigin, forward;
	QAngle cameraAngles;

	if ( target2 )
	{
		forward = target2->GetRenderOrigin() - target1->GetRenderOrigin();
		VectorAngles( forward, cameraAngles );
		cameraAngles.z = 0; // no ROLL
	}
	else
	{
		if ( engine->IsPlayingDemo() )
		{
			engine->GetViewAngles( cameraAngles );
			bManual = true;
		}
		else
		{
			cameraAngles = target1->EyeAngles();
			cameraAngles.x = 0; // no PITCH
			cameraAngles.z = 0; // no ROLL
		}
	}

	// GetRenderOrigin() returns ragdoll pos if player is ragdolled
	targetOrigin = target1->GetRenderOrigin();

	if ( bManual )
	{
		if ( target1->GetFlags() & FL_DUCKING )
		{
			targetOrigin.z += VEC_DUCK_VIEW.z;
		}
		else
		{
			targetOrigin.z += VEC_VIEW.z;
		}
		
	}
	else
	{
		targetOrigin.z += m_flOffset; // add offset
	}

	// apply angle offset & smoothing
	if ( !bManual )
	{
		QAngle angleOffset(  m_flPhi, m_flTheta, 0 );
		cameraAngles += angleOffset;
	
		// smooth angles over time
		InterpolateAngles( m_aCamAngle,	cameraAngles, cameraAngles, m_flInertia/100.0f );
	}

	AngleVectors( cameraAngles, &forward );

	VectorNormalize( forward );

	// smooth distance over time
	if ( m_flLastDistance < m_flDistance )
	{
		// grow distance by 32 unit a second
		m_flLastDistance += gpGlobals->frametime * 32.0f; 

		// but not more than m_flDistance
		m_flLastDistance = min( m_flDistance, m_flLastDistance );
	}
	else
	{
		m_flLastDistance = m_flDistance;
	}

	// calc optimal camera position
	VectorMA(targetOrigin, -m_flLastDistance, forward, cameraOrigin );

	int tries = bManual?1:3;

	while ( tries > 0 )
	{
		// clip against walls
		trace_t trace;
		C_BaseEntity::EnableAbsRecomputations( false ); // HACK don't recompute positions while doing RayTrace
		UTIL_TraceHull( targetOrigin, cameraOrigin, WALL_MIN, WALL_MAX, MASK_SOLID, target1, COLLISION_GROUP_NONE, &trace );
		C_BaseEntity::EnableAbsRecomputations( true );

		if (trace.fraction < 1.0)
		{
			// we hit a wall, correct position and distance
			float dist = VectorLength( trace.endpos -  targetOrigin );

			// if distance is to small and not the last try...
			if ( dist < 48.0f && tries > 1 )
			{
				// camera is really close to target, move it up a bit
				cameraOrigin.z += 24.0f;
				tries--; // try again
			}
			else
			{
				// distance is fine
				cameraOrigin = trace.endpos;
				m_flLastDistance = VectorLength( cameraOrigin -  targetOrigin );
				tries = 0; // stop 
			}
		}
		else
		{
			tries = 0; // everything is fine, done
		}
	}

	// if we have 2 targets look at point between them
	if ( target2 )
	{
		forward = target2->GetRenderOrigin() - target1->GetRenderOrigin();
		VectorMA( target1->GetRenderOrigin(), 0.5f, forward, targetOrigin );
		forward = targetOrigin - cameraOrigin;

		VectorAngles( forward, cameraAngles );
		cameraAngles.z = 0; // no ROLL
	}

	VectorCopy( cameraAngles, m_aCamAngle );
	VectorCopy( cameraOrigin, m_vCamOrigin );

	VectorCopy( m_aCamAngle, eyeAngles );
	VectorCopy( m_vCamOrigin, eyeOrigin );
}

int C_HLTVCamera::GetObserverMode()
{
	if ( m_iCameraMan > 0 )
	{
		C_BasePlayer *pCameraMan = UTIL_PlayerByIndex( m_iCameraMan );

		if ( pCameraMan )
			return pCameraMan->GetObserverMode();
	}

	return m_nCameraMode;	
}

C_BaseEntity* C_HLTVCamera::GetObserverTarget()
{
	if ( m_iCameraMan > 0 )
	{
		C_BasePlayer *pCameraMan = UTIL_PlayerByIndex( m_iCameraMan );
		
		if ( pCameraMan )
		{
			return pCameraMan->GetObserverTarget();
		}
	}

	if ( m_iTraget1 <= 0 )
		return NULL;

	C_BaseEntity* target = ClientEntityList().GetEnt( m_iTraget1 );

	return target;
}

C_BaseEntity *C_HLTVCamera::GetCameraMan()
{
	return ClientEntityList().GetEnt( m_iCameraMan );
}

void C_HLTVCamera::CalcInEyeCamView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov )
{
	C_BasePlayer *pPlayer = UTIL_PlayerByIndex( m_iTraget1 );

	if ( !pPlayer )
		return;

	m_aCamAngle	= pPlayer->EyeAngles();
	m_vCamOrigin = pPlayer->GetAbsOrigin();
	fov = m_flFOV =	pPlayer->GetFOV();

	if ( pPlayer->GetFlags() & FL_DUCKING )
	{
		m_vCamOrigin += VEC_DUCK_VIEW;
	}
	else
	{
		m_vCamOrigin += VEC_VIEW;
	}

	eyeOrigin = m_vCamOrigin;
	eyeAngles = m_aCamAngle;
	
}

void C_HLTVCamera::CalcRoamingView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer )
		return;

	eyeOrigin = m_vCamOrigin = pPlayer->EyePosition();
	eyeAngles = m_aCamAngle = pPlayer->EyeAngles();
	fov = m_flFOV = pPlayer->GetFOV();
}

void C_HLTVCamera::CalcFixedView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov)
{
	eyeOrigin = m_vCamOrigin;
	eyeAngles = m_aCamAngle;
	fov = m_flFOV;

	if ( m_iTraget1 == 0 )
		return;

	C_BaseEntity * traget = ClientEntityList().GetBaseEntity( m_iTraget1 );
	
	if ( traget )
	{
		// if we're chasing a target, change viewangles
		VectorAngles( traget->GetAbsOrigin() - m_vCamOrigin, m_aCamAngle );
	}
}

void C_HLTVCamera::CalcView(Vector& origin, QAngle& angles, float& fov)
{
	if ( m_iCameraMan > 0 )
	{
		C_BasePlayer *pCameraMan = UTIL_PlayerByIndex( m_iCameraMan );
		if ( pCameraMan )
		{
			float zNear,zFar;
			pCameraMan->CalcView( origin, angles, zNear, zFar, fov );
			pCameraMan->CalcViewModelView( origin, angles );
			return;
		}
	}

	switch ( m_nCameraMode )
	{
		case OBS_MODE_ROAMING	:	CalcRoamingView( origin, angles, fov );
									break;

		case OBS_MODE_FIXED		:	CalcFixedView( origin, angles, fov );
									break;

		case OBS_MODE_IN_EYE	:	CalcInEyeCamView( origin, angles, fov );
									break;

		case OBS_MODE_CHASE		:	CalcChaseCamView( origin, angles, fov  );
									break;
	}
}

void C_HLTVCamera::SetMode(int iMode)
{
	Assert( iMode > OBS_MODE_NONE && iMode <= OBS_MODE_ROAMING );
	m_nCameraMode = iMode;
}

void C_HLTVCamera::SpecNextPlayer( bool bInverse )
{
	int start = 1;

	if ( m_iTraget1 > 0 && m_iTraget1 <= gpGlobals->maxClients )
		start = m_iTraget1;

	int index = start;

	while ( true )
	{	
		// got next/prev player
		if ( bInverse )
			index--;
		else
			index++;

		// check bounds
		if ( index < 1 )
			index = gpGlobals->maxClients;
		else if ( index > gpGlobals->maxClients )
			index = 1;

		if ( index == start )
			break; // couldn't find a new valid player

		C_BasePlayer *pPlayer =	UTIL_PlayerByIndex( index );

		if ( !pPlayer )
			continue;

		// only follow living players 
		if ( pPlayer->IsObserver() )
			continue;

		break; // found a new player
	}

	m_iTraget1 = index;
}

void C_HLTVCamera::SpecNamedPlayer( const char *szPlayerName )
{
	for ( int index = 1; index < gpGlobals->maxClients; ++index )
	{
		C_BasePlayer *pPlayer =	UTIL_PlayerByIndex( index );

		if ( !pPlayer )
			continue;

		if ( !FStrEq( szPlayerName, pPlayer->GetPlayerName() ) )
			continue;

		// only follow living players or dedicated spectators
		if ( pPlayer->IsObserver() && pPlayer->GetTeamNumber() != TEAM_SPECTATOR )
			continue;

		m_iTraget1 = index;
		return;
	}
}

void C_HLTVCamera::FireGameEvent( IGameEvent * event)
{
	if ( !engine->IsHLTV() )
		return;	// not in HLTV mode

	const char *type = event->GetName();

	if ( Q_strcmp( "game_newmap", type ) == 0 )
	{
		Reset();	// reset all camera settings
		return;
	}

	else if ( Q_strcmp( "hltv_cameraman", type ) == 0 )
	{
		Reset();

		m_nCameraMode = OBS_MODE_ROAMING;
		
		if ( event->GetBool("active") )
		{
			m_iCameraMan = event->GetInt( "index" ); 
		}
		else 
		{
			m_iCameraMan = 0;
		}

		return;
	}

	else if ( Q_strcmp( "hltv_fixed", type ) == 0 )
	{
		m_nCameraMode = OBS_MODE_FIXED;
		m_iCameraMan  = 0;
		
		m_vCamOrigin.x = event->GetInt( "posx" );
		m_vCamOrigin.y = event->GetInt( "posy" );
		m_vCamOrigin.z = event->GetInt( "posz" );

		m_aCamAngle.x = event->GetInt( "theta" );
		m_aCamAngle.y = event->GetInt( "phi" );
		m_aCamAngle.z = 0; // no roll yet

		m_iTraget1 = event->GetInt( "target" );
		
		return;
	}

	else if ( Q_strcmp( "hltv_chase", type ) == 0 )
	{
		m_nCameraMode = OBS_MODE_CHASE;
		m_iCameraMan  = 0;
				
		m_iTraget1		= event->GetInt( "target1" );
		m_iTraget2		= event->GetInt( "target2" );
		m_flDistance	= event->GetFloat( "distance", m_flDistance );
		m_flOffset		= event->GetFloat( "offset", m_flOffset );
		m_flTheta		= event->GetFloat( "theta", m_flTheta );
		m_flPhi			= event->GetFloat( "phi", m_flPhi );
						
		return;
	}
}

// this is a cheap version of FullNoClipMove():
void C_HLTVCamera::CreateMove( CUserCmd *cmd)
{
	// only do this during HLTV demo playbacks
	if ( !engine->IsPlayingDemo() || !engine->IsHLTV() )
		return;

	Vector origin;
	prediction->GetViewOrigin( origin );

	float factor = sv_specspeed.GetFloat();
	Vector wishvel;
	Vector forward, right, up;
	Vector wishdir;
	float wishspeed;
	float maxspeed = sv_maxspeed.GetFloat() * factor;

	AngleVectors ( cmd->viewangles, &forward, &right, &up);  // Determine movement angles

	if ( cmd->buttons & IN_SPEED )
	{
		factor /= 2.0f;
	}

	// Copy movement amounts
	float fmove = cmd->forwardmove * factor;
	float smove = cmd->sidemove * factor;

	VectorNormalize (forward);  // Normalize remainder of vectors
	VectorNormalize (right);    // 

	for (int i=0 ; i<3 ; i++)       // Determine x and y parts of velocity
		wishvel[i] = forward[i]*fmove + right[i]*smove;
	wishvel[2] += cmd->upmove * factor;

	VectorCopy (wishvel, wishdir);   // Determine magnitude of speed of move
	wishspeed = VectorNormalize(wishdir);

	//
	// Clamp to server defined max speed
	//
	if (wishspeed > maxspeed )
	{
		VectorScale (wishvel, maxspeed/wishspeed, wishvel);
		wishspeed = maxspeed;
	}

	// Just move ( don't clip or anything )
	VectorMA( origin, TICK_INTERVAL, wishvel, origin );

	prediction->SetViewOrigin( origin );
}
