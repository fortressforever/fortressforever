//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
// hltvdirector.cpp: implementation of the CHLTVDirector class.
//
//////////////////////////////////////////////////////////////////////

#include "cbase.h"
#include "hltvdirector.h"
#include "KeyValues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


static ConVar tv_delay( "tv_delay", "10", 0, "SrcTV broadcast delay in seconds" );

static bool GameEventLessFunc( CGameEvent const &e1, CGameEvent const &e2 )
{
	return e1.m_Tick < e2.m_Tick;
}

static float WeightedAngle( Vector vec1, Vector vec2)
{
	VectorNormalize( vec1 );
	VectorNormalize( vec2 );

	float a = DotProduct( vec1, vec2 ); // a = [-1,1]

	a = (a + 1.0f) / 2.0f;

	Assert ( a <= 1 && a >= 0 );
		
	return a*a;	// vectors are facing opposite direction
}

#ifndef CSTRIKE_DLL	// add your mod here if you use your own director

static CHLTVDirector s_HLTVDirector;	// singleton

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CHLTVDirector, IHLTVDirector, INTERFACEVERSION_HLTVDIRECTOR, s_HLTVDirector );

IHLTVDirector* HLTVDirector()
{
	return &s_HLTVDirector;
}

IGameSystem* HLTVDirectorSystem()
{
	return &s_HLTVDirector;
}

#endif



CHLTVDirector::CHLTVDirector()
{
	m_iPVSEntity = 0;
	m_fDelay = 30.0;
	m_iLastPlayer = 1;
	m_pHLTVServer = NULL;
	m_pHLTVClient = NULL;
	m_iCameraMan = 0;
	m_nNumFixedCameras = 0;
	m_EventHistory.SetLessFunc( GameEventLessFunc );
	m_nNextAnalyzeTick = 0;
}

CHLTVDirector::~CHLTVDirector()
{

}

bool CHLTVDirector::Init()
{
	return gameeventmanager->LoadEventsFromFile( "resource/hltvevents.res" ) > 0;
}

void CHLTVDirector::Shutdown()
{
	RemoveEventsFromHistory(-1); // all

	gameeventmanager->RemoveListener( this );
}

void CHLTVDirector::FireGameEvent( IGameEvent * event )
{
	if ( !m_pHLTVServer )
		return;	// don't do anything

	CGameEvent gameevent;

	gameevent.m_Event = gameeventmanager->DuplicateEvent( event );
	gameevent.m_Priority = event->GetInt( "priority", -1 ); // priorities are leveled between 0..10, -1 means ignore
	gameevent.m_Tick = gpGlobals->tickcount;
	
	m_EventHistory.Insert( gameevent );
}

IHLTVServer* CHLTVDirector::GetHLTVServer( void )
{
	return m_pHLTVServer;
}

void CHLTVDirector::SetHLTVServer( IHLTVServer *hltv )
{
	RemoveEventsFromHistory(-1); // all

	if ( hltv ) 
	{
		m_pHLTVClient = UTIL_PlayerByIndex( hltv->GetHLTVSlot() + 1 );

		if ( m_pHLTVClient && m_pHLTVClient->IsHLTV() )
		{
			m_pHLTVServer = hltv;
		}
		else
		{
			m_pHLTVServer  = NULL;
			Error( "Couldn't find HLTV client player." );
		}

		// register for events the director needs to know
		gameeventmanager->AddListener( this, "player_hurt", true );
		gameeventmanager->AddListener( this, "player_death", true );
		gameeventmanager->AddListener( this, "hltv_cameraman", true );
		gameeventmanager->AddListener( this, "hltv_rank_entity", true );
		gameeventmanager->AddListener( this, "hltv_rank_camera", true );
	}
	else
	{
		// deactivate HLTV director
		m_pHLTVServer = NULL;
	}
}

bool CHLTVDirector::IsActive( void )
{
	return (m_pHLTVServer != NULL );
}

float CHLTVDirector::GetDelay( void )
{
	return m_fDelay;
}

int	CHLTVDirector::GetDirectorTick( void )
{
	// just simple delay it
	return m_nBroadcastTick;
}

int	CHLTVDirector::GetPVSEntity( void )
{
	return m_iPVSEntity;
}

Vector CHLTVDirector::GetPVSOrigin( void )
{
	return m_vPVSOrigin;
}

void CHLTVDirector::UpdateSettings()
{
	// set delay
	m_fDelay = tv_delay.GetFloat();
	m_fDelay = clamp( m_fDelay, HLTV_MIN_DELAY, HLTV_MAX_DELAY );

	m_nBroadcastTick = gpGlobals->tickcount - TIME_TO_TICKS( m_fDelay );
	m_nBroadcastTick = max( 0, m_nBroadcastTick );



	//set cameraman
	// m_pViewEntity = UTIL_PlayerByIndex( hltv_viewent.GetInt() );
}

const char** CHLTVDirector::GetModEvents()
{
	static const char *s_modevents[] =
	{
		"hltv_status",
		"hltv_chat",
		"player_connect",
		"player_disconnect",
		"player_team",
		"player_info",
		"server_cvar",
		"player_death",
		"player_chat",
		"round_start",
		"round_end",
		NULL
	};

	return s_modevents;
}

// this is called with every new map 
void CHLTVDirector::LevelInitPostEntity( void )
{
	m_nNumFixedCameras = 0;
	
	CBaseEntity *pCamera = gEntList.FindEntityByClassname( NULL, "point_viewcontrol" );

	while ( pCamera && m_nNumFixedCameras < MAX_NUM_CAMERAS)
	{
		CBaseEntity *pTarget = gEntList.FindEntityByName( NULL, STRING(pCamera->m_target), pCamera );

		m_pFixedCameras[m_nNumFixedCameras] = pCamera;
		m_pCameraTargets[m_nNumFixedCameras] = pTarget; 

		m_nNumFixedCameras++;
		pCamera = gEntList.FindEntityByClassname( pCamera, "point_viewcontrol" );
	}

	m_vPVSOrigin.Init();
	m_iPVSEntity = 0;
	m_nNextShotTick = 0;
	m_nNextAnalyzeTick = 0;

	// DevMsg("HLTV Director: found %i fixed cameras.\n", m_nNumFixedCameras );
}

void CHLTVDirector::FrameUpdatePostEntityThink( void )
{
	if ( !m_pHLTVServer )
		return;	// don't do anything

	UpdateSettings();	// update settings from cvars

	if ( m_nNextAnalyzeTick < gpGlobals->tickcount )
	{
		m_nNextAnalyzeTick = gpGlobals->tickcount + TIME_TO_TICKS( 0.5f );

		AnalyzePlayers();

		AnalyzeCameras();
	}

	// This function is called each tick
	if ( m_nNextShotTick <= m_nBroadcastTick )
	{
		StartNewShot();		
	}
}

void CHLTVDirector::StartBestPlayerCameraShot()
{
	float flPlayerRanking[MAX_PLAYERS];

	memset( flPlayerRanking, 0, sizeof(flPlayerRanking) );

	int firstIndex = FindFirstEvent( m_nBroadcastTick );

	int index = firstIndex;

	float flBestRank = -1.0f;
	int iBestCamera = -1;
	int iBestTarget = -1;

	// sum all ranking values for the cameras

	while( index != m_EventHistory.InvalidIndex() )
	{
		CGameEvent &dc = m_EventHistory[index];

		if ( dc.m_Tick >= m_nNextShotTick )
			break; 

		// search for camera ranking events
		if ( Q_strcmp( dc.m_Event->GetName(), "hltv_rank_entity") == 0 )
		{
			int index = dc.m_Event->GetInt("index"); 

			if ( index < MAX_PLAYERS )
			{
				flPlayerRanking[index] += dc.m_Event->GetFloat("rank" );

				// find best camera
				if ( flPlayerRanking[index] > flBestRank )
				{
					iBestCamera = index;
					flBestRank = flPlayerRanking[index];
					iBestTarget = dc.m_Event->GetInt("target"); 
				}
			}
		}

		index = m_EventHistory.NextInorder( index );
	}

	if ( iBestCamera == -1 )
		return;

	IGameEvent *shot = gameeventmanager->CreateEvent( "hltv_chase", true );

	if ( shot )
	{
		shot->SetInt("target1", iBestCamera );
		shot->SetInt("target2", iBestTarget );
		shot->SetInt("offset", VEC_VIEW.z );
		shot->SetInt("distance", 112.0f );
        shot->SetInt("theta", 30 );
		shot->SetInt("phi", 30 );

		m_iPVSEntity = iBestCamera;
		
		// send spectators the HLTV director command as a game event
		m_pHLTVServer->BroadcastEvent( shot );
		gameeventmanager->FreeEvent( shot );
	}
}

void CHLTVDirector::StartFixedCameraShot(int iCamera, int iTarget)
{
	CBaseEntity *pCamera = m_pFixedCameras[iCamera];
	CBaseEntity *pTarget = m_pCameraTargets[iCamera];

	Vector vCamPos = pCamera->GetAbsOrigin();
	Vector vTargetPos = pTarget->GetAbsOrigin();
	QAngle aViewAngle;

	m_iPVSEntity = 0;	// don't use camera entity, since it may not been transmitted
	m_vPVSOrigin = vCamPos;

	VectorAngles( vTargetPos-vCamPos, aViewAngle );

	IGameEvent *shot = gameeventmanager->CreateEvent( "hltv_fixed", true );

	if ( shot )
	{
		shot->SetInt("posx", vCamPos.x );
		shot->SetInt("posy", vCamPos.y );
		shot->SetInt("posz", vCamPos.z );
		shot->SetInt("theta", aViewAngle.x );
		shot->SetInt("phi", aViewAngle.y );
		shot->SetInt("target", iTarget );
	
		// send spectators the HLTV director command as a game event
		m_pHLTVServer->BroadcastEvent( shot );
		gameeventmanager->FreeEvent( shot );
	}
}

void CHLTVDirector::StartBestFixedCameraShot()
{
	float	flCameraRanking[MAX_NUM_CAMERAS];

	if ( m_nNumFixedCameras <= 0 )
		return;

	memset( flCameraRanking, 0, sizeof(flCameraRanking) );

	int firstIndex = FindFirstEvent( m_nBroadcastTick );

	int index = firstIndex;

	float flBestRank = -1.0f;
	int iBestCamera = -1;
	int iBestTarget = -1;

	// sum all ranking values for the cameras

	while( index != m_EventHistory.InvalidIndex() )
	{
		CGameEvent &dc = m_EventHistory[index];

		if ( dc.m_Tick >= m_nNextShotTick )
			break; 

		// search for camera ranking events
		if ( Q_strcmp( dc.m_Event->GetName(), "hltv_rank_camera") == 0 )
		{
			int index = dc.m_Event->GetInt("index"); 
			flCameraRanking[index] += dc.m_Event->GetFloat("rank" );

			// find best camera
			if ( flCameraRanking[index] > flBestRank )
			{
				iBestCamera = index;
				flBestRank = flCameraRanking[index];
				iBestTarget = dc.m_Event->GetInt("target"); 
			}
		}

		index = m_EventHistory.NextInorder( index );
	}

	if ( iBestCamera != -1 )
	{
		StartFixedCameraShot( iBestCamera, iBestTarget );
	}
	else
	{
		StartBestPlayerCameraShot();
	}

	
}

void CHLTVDirector::StartRandomShot() 
{
	int toTick = m_nBroadcastTick + TIME_TO_TICKS ( DEF_SHOT_LENGTH );
	m_nNextShotTick = min( m_nNextShotTick, toTick );

	if ( RandomFloat(0,1) > 0.33 )
	{
		StartBestPlayerCameraShot();
	}
	else
	{
		StartBestFixedCameraShot();
	}
		
}

void CHLTVDirector::CreateShotFromEvent( CGameEvent *event )
{
	// show event at least for 2 more seconds after it occured
	const char *name = event->m_Event->GetName();
	IGameEvent *shot = NULL;

	if ( !Q_strcmp( "player_death", name ) ||
		 !Q_strcmp( "player_hurt", name ) )
	{
		CBaseEntity *victim = UTIL_PlayerByUserId( event->m_Event->GetInt("userid") );
		CBaseEntity *attacker = UTIL_PlayerByUserId( event->m_Event->GetInt("attacker") );

		if ( !victim )
			return;

		shot = gameeventmanager->CreateEvent( "hltv_chase", true );

		if ( attacker && attacker != victim )
		{
			// sometimes switch attacker and victim
			if ( RandomFloat(0,1) > 0.66  )
			{
				CBaseEntity *swap = attacker;
				attacker = victim;
				victim = swap;
			}

			shot->SetInt( "target2", attacker->entindex() );
			// view over shoulder, randomly left or right
			if ( RandomFloat(0,1) > 0.5  )
			{
				shot->SetInt( "theta", 40 ); // swing left
			}
			else
			{
				shot->SetInt( "theta", -40 ); // swing right
			}

			shot->SetInt( "phi", -30 ); // lower view point, dramatic
		}
		else
		{
			shot->SetInt( "target2", 0 );

			// view from behind over head
			shot->SetInt( "theta", 0 );
			shot->SetInt( "phi", 20 );
		}

		shot->SetInt( "target1", victim->entindex() );

		shot->SetInt( "offset", VEC_VIEW.z );
		shot->SetFloat( "distance", 96.0f );

		// shot 2 seconds after death/hurt
		m_nNextShotTick = min( m_nNextShotTick, (event->m_Tick+TIME_TO_TICKS(2.0)) );
		m_iPVSEntity = victim->entindex();
	}

	if ( shot )
	{
		m_pHLTVServer->BroadcastEvent( shot );
		gameeventmanager->FreeEvent( shot );
	}
}

void CHLTVDirector::CheckHistory()
{
	int index = m_EventHistory.FirstInorder();
	int lastTick = -1;

	while ( index != m_EventHistory.InvalidIndex() )
	{
		CGameEvent &dc = m_EventHistory[index];

		Assert( lastTick <= dc.m_Tick );
		lastTick = dc.m_Tick;

		index = m_EventHistory.NextInorder( index );
	}
}

void CHLTVDirector::RemoveEventsFromHistory(int tick)
{
	int index = m_EventHistory.FirstInorder();

	while ( index != m_EventHistory.InvalidIndex() )
	{
		CGameEvent &dc = m_EventHistory[index];

		if ( (dc.m_Tick < tick) || (tick == -1) )
		{
			gameeventmanager->FreeEvent( dc.m_Event );
			dc.m_Event = NULL;
			m_EventHistory.RemoveAt( index );
			index = m_EventHistory.FirstInorder();	// start again
		}
		else
		{
			index = m_EventHistory.NextInorder( index );
		}
	}

#ifdef _DEBUG
	CheckHistory();
#endif
}

int CHLTVDirector::FindFirstEvent( int tick )
{
	// TODO cache last queried ticks

	int index = m_EventHistory.FirstInorder();

	if ( index == m_EventHistory.InvalidIndex() )
		return index; // no commands in list

	CGameEvent *event = &m_EventHistory[index];

	while ( event->m_Tick < tick )
	{
		index = m_EventHistory.NextInorder( index );
		
		if ( index == m_EventHistory.InvalidIndex() )
			break;

		event = &m_EventHistory[index];
	}

	return index;
}

void CHLTVDirector::FinishCameraManShot()
{
	Assert( m_iCameraMan == m_iPVSEntity );

	// check next frame again if we don't find any commands in buffer( should never happen)
	m_nNextShotTick = m_nBroadcastTick+1;

	int index = FindFirstEvent( m_nBroadcastTick );

	//check if camera turns camera off within broadcast time and game time
	while( index != m_EventHistory.InvalidIndex() )
	{
		CGameEvent &dc = m_EventHistory[index];

		m_nNextShotTick = dc.m_Tick+1; 

		if ( Q_strcmp( dc.m_Event->GetName(), "hltv_cameraman") == 0 )
		{
			if ( !dc.m_Event->GetBool("active") && dc.m_Event->GetInt("index") == m_iCameraMan )
			{
				// current camera man switched camera off
				m_iCameraMan = 0;
				return;
			}
		}

		index = m_EventHistory.NextInorder( index );
	}

	// camera man is still recording and live, don't change anything
}


bool CHLTVDirector::StartCameraManShot()
{
	Assert( m_nNextShotTick <= m_nBroadcastTick );

	int index = FindFirstEvent( m_nNextShotTick );

	// check for cameraman mode
	while( index != m_EventHistory.InvalidIndex() )
	{
		CGameEvent &dc = m_EventHistory[index];

		// only check if this is the current tick
		if ( dc.m_Tick > m_nBroadcastTick )
			break; 

		if ( Q_strcmp( dc.m_Event->GetName(), "hltv_cameraman") == 0 )
		{
			if ( dc.m_Event->GetBool("active") )
			{
				// ok, this guy is now the active camera man
				m_iCameraMan = dc.m_Event->GetInt("index");

				// TODO check if camera man is still valid, not NULL
				
				m_iPVSEntity = m_iCameraMan;
				m_nNextShotTick = m_nBroadcastTick+1; // check setting right on next frame

				// send camera man command to client
				m_pHLTVServer->BroadcastEvent( dc.m_Event );
				return true;
			}
		}

		index = m_EventHistory.NextInorder( index );
	}

	return false;	// no camera man found
}

void CHLTVDirector::StartNewShot()
{
	// we can remove all events the
	int smallestTick = max(0, gpGlobals->tickcount - TIME_TO_TICKS(HLTV_MAX_DELAY) );
    RemoveEventsFromHistory( smallestTick );

	if ( m_iCameraMan > 0 )
	{
		// we already have an active camera man,
		// wait until he releases the "record" lock
		FinishCameraManShot();
		return;	
	}

	if ( StartCameraManShot() )
	{
		// now we have an active camera man
		return;
	} 

	// ok, no camera man active, now check how much time
	// we have for the next shot, if the time diff to the next
	// important event we have to switch to is too short (<2sec)
	// just extent the current shot and don't start a new one

	// check the next 8 seconds for interrupts/important events
	m_nNextShotTick = m_nBroadcastTick + TIME_TO_TICKS( MAX_SHOT_LENGTH );

	int index = FindFirstEvent( m_nBroadcastTick );

	while( index != m_EventHistory.InvalidIndex() )
	{
		CGameEvent &dc = m_EventHistory[index];

		if ( dc.m_Tick >= m_nNextShotTick )
			break; // we have searched enough

		// a camera man is always interrupting auto director
		if ( Q_strcmp( dc.m_Event->GetName(), "hltv_cameraman") == 0 )
		{
			if ( dc.m_Event->GetBool("active") )
			{
				// stop the next cut when this cameraman starts recording
				m_nNextShotTick = dc.m_Tick;
				break;
			}
		}

		index = m_EventHistory.NextInorder( index );
	} 

	float flDuration = TICKS_TO_TIME(m_nNextShotTick - m_nBroadcastTick);

	if ( flDuration < MIN_SHOT_LENGTH )
		return;	// not enough time for a new shot

	// find the most intesting game event for next shot
	CGameEvent *dc = FindBestGameEvent();

	if ( dc )
	{
		// show the game event
		CreateShotFromEvent( dc );
	}
	else
	{
		// no interesting events found, start random shot
		StartRandomShot();
	}
}

CGameEvent *CHLTVDirector::FindBestGameEvent()
{
	int	bestEvent[4];
	int	bestEventPrio[4];

	Q_memset( bestEvent, 0, sizeof(bestEvent) );
	Q_memset( bestEventPrio, 0, sizeof(bestEventPrio) );
	
	int index = FindFirstEvent( m_nBroadcastTick );
		
	// search for next 4 best events within next 8 seconds
	for (int i = 0; i<4; i ++)
	{
		bestEventPrio[i] = 0;
		bestEvent[i] = 0;
	
		int tillTick = m_nBroadcastTick + TIME_TO_TICKS( 2.0f*(1.0f+i) );

		if ( tillTick > m_nNextShotTick )
			break;

		// sum all action for the next time
		while ( index != m_EventHistory.InvalidIndex()  )
		{
			CGameEvent &event = m_EventHistory[index];

			if ( event.m_Tick > tillTick )
				break;

			int priority = event.m_Priority;

			if ( priority > bestEventPrio[i] )
			{
				bestEvent[i] = index;
				bestEventPrio[i] = priority;
			}
			
			index = m_EventHistory.NextInorder( index );
		}
	}

	if ( !( bestEventPrio[0] || bestEventPrio[1] || bestEventPrio[2] ) )
		return NULL; // no event found at all, give generic algorithm a chance

	// camera cut rules :

	if ( bestEventPrio[1] >= bestEventPrio[0] &&
		 bestEventPrio[1] >= bestEventPrio[2] &&
		 bestEventPrio[1] >= bestEventPrio[3] )
	{
		return &m_EventHistory[ bestEvent[1] ];	// best case
	}
	else if ( bestEventPrio[0] > bestEventPrio[1] &&
			  bestEventPrio[0] > bestEventPrio[2] )
	{
		return &m_EventHistory[ bestEvent[0] ];	// event 0 is very important
	}
	else if (	bestEventPrio[2] > bestEventPrio[3] ) 
	{
		return &m_EventHistory[ bestEvent[2] ];	
	}
	else
	{
		// event 4 is the best but too far away, so show event 1 
		if ( bestEvent[0] )
			return &m_EventHistory[ bestEvent[0] ];
		else
			return NULL;
	}
}

void CHLTVDirector::AnalyzeCameras()
{
	for ( int i = 0; i<m_nNumFixedCameras; i++ )
	{
		CBaseEntity *pCamera = m_pFixedCameras[ i ];

		float	flRank = 0.0f;
		int		iClosestPlayer = 0;
		float	flClosestPlayerDist = 100000.0f;
		int		nCount = 0; // Number of visible targets
		Vector	vDistribution; vDistribution.Init(); // distribution of targets
		
		Vector vCamPos = pCamera->GetAbsOrigin();

		for ( int j=0; j<m_nNumActivePlayers; j++ )
		{
			CBasePlayer *pPlayer = m_pActivePlayers[j];

			Vector vPlayerPos = pPlayer->GetAbsOrigin();

			float dist = VectorLength( vPlayerPos - vCamPos );

			if ( dist > 1024.0f || dist < 4.0f )
				continue;	// too colse or far away

			// check visibility
			trace_t tr;
			UTIL_TraceLine( vCamPos, pPlayer->GetAbsOrigin(), MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr  );

			if ( tr.fraction < 1.0 )
				continue;	// not visible for camera

			nCount++;

			// remember closest player
			if ( dist <  flClosestPlayerDist )
			{
				iClosestPlayer = pPlayer->entindex();
				flClosestPlayerDist = dist;
			}

			Vector v1; AngleVectors( pPlayer->EyeAngles(), &v1 );

			// check players orientation towards camera
			Vector v2 = vCamPos - vPlayerPos;
			VectorNormalize( v2 );

			// player/camera cost function:
			flRank += ( 1.0f/sqrt(dist) ) * WeightedAngle( v1, v2 );

			vDistribution += v2;
		}

		if ( nCount == 0 )
			continue; // camera doesn't see anybody

		float flDistribution =  VectorLength( vDistribution ) / nCount; // normalize distribution

		flRank *= flDistribution;

		IGameEvent *event = gameeventmanager->CreateEvent("hltv_rank_camera");

		if ( event )
		{
			event->SetFloat("rank", flRank );
			event->SetInt("index",  i ); // index in m_pFixedCameras
			event->SetInt("target",  iClosestPlayer ); // ent index
			gameeventmanager->FireEvent( event );
		}
	}
}

void CHLTVDirector::BuildActivePlayerList()
{
	// first build list of all active players

	m_nNumActivePlayers = 0;

	for ( int i =0; i < gpGlobals->maxClients; i++ )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i+1 );

		if ( !pPlayer )
			continue;

		if ( !pPlayer->IsAlive() )
			continue;

		if ( pPlayer->IsObserver() )
			continue;

		m_pActivePlayers[m_nNumActivePlayers] = pPlayer;
		m_nNumActivePlayers++;
	}
}

void CHLTVDirector::AnalyzePlayers()
{
	// build list of current active players
	BuildActivePlayerList();

	// analyzes every active player
	
	for ( int i = 0; i<m_nNumActivePlayers; i++ )
	{
		CBasePlayer *pPlayer = m_pActivePlayers[ i ];

		float	flRank = 0.0f;
		int		iBestFacingPlayer = i;
		float	flBestFacingPlayer = 0.0f;
		int		nCount = 0; // Number of visible targets
		Vector	vDistribution; vDistribution.Init(); // distribution of targets
		
		Vector vCamPos = pPlayer->GetAbsOrigin();

		Vector v1; AngleVectors( pPlayer->EyeAngles(), &v1 );

		v1 *= -1; // inverted

		for ( int j=0; j<m_nNumActivePlayers; j++ )
		{
			if ( i == j )
				continue;  // don't check against itself
			
			CBasePlayer *pOtherPlayer = m_pActivePlayers[j];

			Vector vPlayerPos = pOtherPlayer->GetAbsOrigin();

			float dist = VectorLength( vPlayerPos - vCamPos );

			if ( dist > 1024.0f || dist < 4.0f )
				continue;	// too close or far away

			// check visibility
			trace_t tr;
			UTIL_TraceLine( vCamPos, pOtherPlayer->GetAbsOrigin(), MASK_SOLID, pOtherPlayer, COLLISION_GROUP_NONE, &tr  );

			if ( tr.fraction < 1.0 )
				continue;	// not visible for camera

			nCount++;

			// check players orientation towards camera
			Vector v2; AngleVectors( pOtherPlayer->EyeAngles(), &v2 );

			float facing = WeightedAngle( v1, v2 );

			// remember closest player
			if ( facing > flBestFacingPlayer )
			{
				iBestFacingPlayer = pOtherPlayer->entindex();
				flBestFacingPlayer = facing;
			}

			// player/camera cost function:
			flRank += ( 1.0f/sqrt(dist) ) * facing;

			vDistribution += v2;
		}

		if ( nCount > 0 )
		{
			float flDistribution =  VectorLength( vDistribution ) / nCount; // normalize distribution
			flRank *= flDistribution;
		}

		IGameEvent *event = gameeventmanager->CreateEvent("hltv_rank_entity");
		if ( event )
		{
			event->SetInt("index",  pPlayer->entindex() );
			event->SetFloat("rank", flRank );
			event->SetInt("target",  iBestFacingPlayer ); // ent index
			gameeventmanager->FireEvent( event );
		}
	}
}

