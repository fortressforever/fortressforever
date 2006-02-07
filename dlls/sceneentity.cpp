//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include <stdarg.h>
#include "baseflex.h"
#include "entitylist.h"
#include "choreoevent.h"
#include "choreoactor.h"
#include "choreochannel.h"
#include "choreoscene.h"
#include "ichoreoeventcallback.h"
#include "iscenetokenprocessor.h"
#include "studio.h"
#include "networkstringtable_gamedll.h"
#include "ai_basenpc.h"
#include "engine/IEngineSound.h"
#include "ai_navigator.h"
#include "saverestore_utlvector.h"
#include "ai_baseactor.h"
#include "AI_Criteria.h"
#include "vstdlib/strtools.h"
#include "checksum_crc.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "UtlCachedFileData.h"
#include "utlbuffer.h"
#include "vstdlib/ICommandLine.h"
#include "resourcemanager.h"
#include "sceneentity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ISoundEmitterSystemBase *soundemitterbase;

class CSceneEntity;
class CBaseFlex;

static ConVar scene_print( "scene_print", "0", 0, "When playing back a scene, print timing and event info to console." );
static ConVar scene_forcecombined( "scene_forcecombined", "0", 0, "When playing back, force use of combined .wav files even in english." );
static ConVar scene_maxcaptionradious( "scene_maxcaptionradius", "1200", 0, "Only show closed captions if recipient is within this many units of speaking actor (0==disabled)." );

// Assume sound system is 100 msec lagged (only used if we can't find snd_mixahead cvar!)
#define SOUND_SYSTEM_LATENCY_DEFAULT ( 0.1f )

// Think every 50 msec (FIXME: Try 10hz?)
#define SCENE_THINK_INTERVAL 0.001 // FIXME: make scene's think in concert with their npc's	

static bool CopySceneFileIntoMemory( char const *filename, void **buffer );
static void FreeSceneFileMemory( void *buffer );

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pFormat - 
//			... - 
// Output : static void
//-----------------------------------------------------------------------------
static void Scene_Printf( const char *pFormat, ... )
{
	if ( !scene_print.GetInt() )
		return;

	va_list marker;
	char msg[8192];

	va_start(marker, pFormat);
	Q_vsnprintf(msg, sizeof(msg), pFormat, marker);
	va_end(marker);	
	
	Msg( "%s", msg );
}

//-----------------------------------------------------------------------------
// Purpose: Embedded interface handler, just called method in m_pScene
//-----------------------------------------------------------------------------
class CChoreoEventCallback : public IChoreoEventCallback
{
public:
						CChoreoEventCallback( void );

	void				SetScene( CSceneEntity *scene );

	// Implements IChoreoEventCallback
	virtual void		StartEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event );
	virtual void		EndEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event );
	virtual void		ProcessEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event );
	virtual bool		CheckEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event );

private:
	CSceneEntity		*m_pScene;
};

//-----------------------------------------------------------------------------
// Purpose: This class exists solely to call think on all scene entities in a deterministic order
//-----------------------------------------------------------------------------
class CSceneManager : public CLogicalEntity
{
	DECLARE_CLASS( CSceneManager, CLogicalEntity );
	DECLARE_DATADESC();

public:
	virtual void			Spawn()
	{
		BaseClass::Spawn();
		SetNextThink( gpGlobals->curtime );
	}

	virtual int				ObjectCaps( void ) { return BaseClass::ObjectCaps() | FCAP_DONT_SAVE; }

	virtual void			Think();

			void			ClearAllScenes();

			void			AddSceneEntity( CSceneEntity *scene );
			void			RemoveSceneEntity( CSceneEntity *scene );

			void			QueueRestoredSound( CBaseFlex *actor, char const *soundname, soundlevel_t soundlevel, float time_in_past );

			void			OnClientActive( CBasePlayer *player );
			
			void			RemoveActorFromScenes( CBaseFlex *pActor, bool bInstancedOnly  );
			bool			IsRunningScriptedScene( CBaseFlex *pActor, bool bIgnoreInstancedScenes );
			bool			IsRunningScriptedSceneWithSpeech( CBaseFlex *pActor, bool bIgnoreInstancedScenes );


private:

	struct CRestoreSceneSound
	{
		CRestoreSceneSound()
		{
			actor = NULL;
			soundname[ 0 ] = NULL;
			soundlevel = SNDLVL_NORM;
			time_in_past = 0.0f;
		}

		CHandle< CBaseFlex >	actor;
		char					soundname[ 128 ];
		soundlevel_t			soundlevel;
		float					time_in_past;
	};

	CUtlVector< CHandle< CSceneEntity > >	m_ActiveScenes;

	CUtlVector< CRestoreSceneSound >		m_QueuedSceneSounds;
};

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CSceneManager )

	DEFINE_UTLVECTOR( m_ActiveScenes,	FIELD_EHANDLE ),
	// DEFINE_FIELD( m_QueuedSceneSounds, CUtlVector < CRestoreSceneSound > ),  // Don't save/restore this, it's created and used by OnRestore only

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: Singleton scene manager.  Created by first placed scene or recreated it it's deleted for some unknown reason
// Output : CSceneManager
//-----------------------------------------------------------------------------
CSceneManager *GetSceneManager()
{
	// Create it if it doesn't exist
	static CHandle< CSceneManager >	s_SceneManager;
	if ( s_SceneManager == NULL )
	{
		s_SceneManager = ( CSceneManager * )CreateEntityByName( "scene_manager" );
		Assert( s_SceneManager );
		if ( s_SceneManager )
		{
			s_SceneManager->Spawn();
		}
	}

	Assert( s_SceneManager );
	return s_SceneManager;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *player - 
//-----------------------------------------------------------------------------
void SceneManager_ClientActive( CBasePlayer *player )
{
	Assert( GetSceneManager() );

	if ( GetSceneManager() )
	{
		GetSceneManager()->OnClientActive( player );
	}
}

//-----------------------------------------------------------------------------
// Purpose: FIXME, need to deal with save/restore
//-----------------------------------------------------------------------------
class CSceneEntity : public CLogicalEntity
{
	friend class CInstancedSceneEntity;
public:

	enum
	{
		SCENE_ACTION_UNKNOWN = 0,
		SCENE_ACTION_CANCEL,
		SCENE_ACTION_RESUME,
	};

	enum
	{
		SCENE_BUSYACTOR_DEFAULT = 0,
		SCENE_BUSYACTOR_WAIT,
	};




	DECLARE_CLASS( CSceneEntity, CLogicalEntity );

							CSceneEntity( void );
							~CSceneEntity( void );
				
	virtual void			Activate();

	virtual	void			Precache( void );
	virtual void			Spawn( void );
	virtual void			UpdateOnRemove( void );

	virtual void			OnRestore();

	DECLARE_DATADESC();

	virtual void			OnSceneFinished( bool canceled, bool fireoutput );

	virtual void			DoThink( float frametime );
	virtual void			PauseThink( void );

	bool					IsPlayingBack() const	{ return m_bIsPlayingBack; }

	bool					IsInterruptable();
	virtual void			ClearInterrupt();
	virtual void			CheckInterruptCompletion();

	virtual bool			InterruptThisScene( CSceneEntity *otherScene );
	void					RequestCompletionNotification( CSceneEntity *otherScene );

	virtual void			NotifyOfCompletion( CSceneEntity *interruptor );

	void					ClearActivatorTargets( void );

	// Inputs
	void InputStartPlayback( inputdata_t &inputdata );
	void InputPausePlayback( inputdata_t &inputdata );
	void InputResumePlayback( inputdata_t &inputdata );
	void InputCancelPlayback( inputdata_t &inputdata );
	// Not enabled, see note below in datadescription table
	void InputReloadScene( inputdata_t &inputdata );

	// If the scene is playing, finds an actor in the scene who can respond to the specified concept token
	void InputInterjectResponse( inputdata_t &inputdata );

	// If this scene is waiting on an actor, give up and quit trying.
	void InputStopWaitingForActor( inputdata_t &inputdata );

	virtual void StartPlayback( void );
	virtual void PausePlayback( void );
	virtual void ResumePlayback( void );
	virtual void CancelPlayback( void );
	virtual void ReloadScene( void );

	bool		 ValidScene() const;

	// From scene callback object
	virtual void			StartEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event );
	virtual void			EndEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event );
	virtual void			ProcessEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event );
	virtual bool			CheckEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event );

	// Scene load/unload
	CChoreoScene			*LoadScene( const char *filename );

	void					LoadSceneFromFile( const char *filename );
	void					UnloadScene( void );

	struct SpeakEventSound_t
	{
		CUtlSymbol	m_Symbol;
		float		m_flStartTime;
	};

	static bool SpeakEventSoundLessFunc( const SpeakEventSound_t& lhs, const SpeakEventSound_t& rhs );

	bool					GetSoundNameForPlayer( CChoreoEvent *event, CBasePlayer *player, char *buf, size_t buflen );

	void					BuildSortedSpeakEventSoundsPrefetchList( 
								CChoreoScene *scene, 
								CUtlSymbolTable& table, 
								CUtlRBTree< SpeakEventSound_t >& soundnames, 
								float timeOffset );
	void					PrefetchSpeakEventSounds( CUtlSymbolTable& table, CUtlRBTree< SpeakEventSound_t >& soundnames );

	// Event handlers
	virtual void			DispatchStartExpression( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchEndExpression( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchStartFlexAnimation( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchEndFlexAnimation( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchStartGesture( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchEndGesture( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchStartLookAt( CChoreoScene *scene, CBaseFlex *actor, CBaseEntity *actor2, CChoreoEvent *event );
	virtual void			DispatchEndLookAt( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchStartMoveTo( CChoreoScene *scene, CBaseFlex *actor, CBaseEntity *actor2, CChoreoEvent *event );
	virtual void			DispatchEndMoveTo( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event );
	virtual	void			DispatchStartSpeak( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event, soundlevel_t iSoundlevel );
	virtual void			DispatchEndSpeak( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchStartFace( CChoreoScene *scene, CBaseFlex *actor, CBaseEntity *actor2, CChoreoEvent *event );
	virtual void			DispatchEndFace( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchStartSequence( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchEndSequence( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchStartSubScene( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchStartInterrupt( CChoreoScene *scene, CChoreoEvent *event );
	virtual void			DispatchEndInterrupt( CChoreoScene *scene, CChoreoEvent *event );
	virtual void			DispatchStartGeneric( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchEndGeneric( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event );

	// NPC can play interstitial vcds (such as responding to the player doing something during a scene)
	virtual void			DispatchStartPermitResponses( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event );
	virtual void			DispatchEndPermitResponses( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event );


	// Global events
	virtual void			DispatchProcessLoop( CChoreoScene *scene, CChoreoEvent *event );
	virtual void			DispatchPauseScene( CChoreoScene *scene, const char *parameters );
	virtual void			DispatchStopPoint( CChoreoScene *scene, const char *parameters );

	float					EstimateLength( void );
	
	bool					InvolvesActor( CBaseEntity *pActor );

	void					GenerateSoundScene( CBaseFlex *pActor, const char *soundname );

	virtual float			GetPostSpeakDelay()	{ return 1.0; }

	bool					HasUnplayedSpeech( void );
	bool					HasFlexAnimation( void );

// Data
public:
	string_t				m_iszSceneFile;

	string_t				m_iszTarget1;
	string_t				m_iszTarget2;
	string_t				m_iszTarget3;
	string_t				m_iszTarget4;
	string_t				m_iszTarget5;
	string_t				m_iszTarget6;
	string_t				m_iszTarget7;
	string_t				m_iszTarget8;

	EHANDLE					m_hTarget1;
	EHANDLE					m_hTarget2;
	EHANDLE					m_hTarget3;
	EHANDLE					m_hTarget4;
	EHANDLE					m_hTarget5;
	EHANDLE					m_hTarget6;
	EHANDLE					m_hTarget7;
	EHANDLE					m_hTarget8;

	bool					m_bIsPlayingBack;
	bool					m_bPaused;
	float					m_flCurrentTime;
	float					m_flFrameTime;

	bool					m_bAutomated;
	int						m_nAutomatedAction;
	float					m_flAutomationDelay;
	float					m_flAutomationTime;

	// A pause from an input requires another input to unpause (it's a hard pause)
	bool					m_bPausedViaInput;

	// Waiting for the actor to be able to speak.
	bool					m_bWaitingForActor;

public:
	virtual CBaseFlex		*FindNamedActor( int index );
	virtual CBaseFlex		*FindNamedActor( CChoreoActor *pChoreoActor );
	virtual CBaseFlex		*FindNamedActor( const char *name );
	virtual CBaseEntity		*FindNamedEntity( const char *name, CBaseEntity *pActor = NULL, bool bBaseFlexOnly = false );
	CBaseEntity				*FindNamedTarget( string_t iszTarget, bool bBaseFlexOnly = false );

private:
	CUtlVector< CHandle< CBaseFlex > >	m_hActorList;

private:

	// Prevent derived classed from using this!
	virtual void			Think( void ) {};


	void					ClearSceneEvents( CChoreoScene *scene, bool canceled );
	void					ClearSchedules( CChoreoScene *scene );

	float					GetSoundSystemLatency( void );
	void					PrecacheScene( CChoreoScene *scene );

	CChoreoScene			*GenerateSceneForSound( CBaseFlex *pFlexActor, const char *soundname );

	// The underlying scene we are editing
	CChoreoScene			*m_pScene;
	CChoreoEventCallback	m_SceneCallback;

	const ConVar			*m_pcvSndMixahead;

	COutputEvent			m_OnStart;
	COutputEvent			m_OnCompletion;
	COutputEvent			m_OnCanceled;
	COutputEvent			m_OnTrigger1;
	COutputEvent			m_OnTrigger2;
	COutputEvent			m_OnTrigger3;
	COutputEvent			m_OnTrigger4;
	COutputEvent			m_OnTrigger5;
	COutputEvent			m_OnTrigger6;
	COutputEvent			m_OnTrigger7;
	COutputEvent			m_OnTrigger8;

	int						m_nInterruptCount;
	bool					m_bInterrupted;
	CHandle< CSceneEntity >	m_hInterruptScene;

	bool					m_bCompletedEarly;

	bool					m_bInterruptSceneFinished;
	CUtlVector< CHandle< CSceneEntity > >	m_hNotifySceneCompletion;

	bool					m_bRestoring;

	bool					m_bGenerated;
	string_t				m_iszSoundName;
	CHandle< CBaseFlex >	m_hActor;

	EHANDLE					m_hActivator;

	int						m_BusyActor;

public:
	void					SetBackground( bool bIsBackground );
	bool					IsBackground( void );
};

LINK_ENTITY_TO_CLASS( logic_choreographed_scene, CSceneEntity );
LINK_ENTITY_TO_CLASS( scripted_scene, CSceneEntity );

BEGIN_DATADESC( CSceneEntity )

	// Keys
	DEFINE_KEYFIELD( m_iszSceneFile, FIELD_STRING, "SceneFile" ),

	DEFINE_KEYFIELD( m_iszTarget1, FIELD_STRING, "target1" ),
	DEFINE_KEYFIELD( m_iszTarget2, FIELD_STRING, "target2" ),
	DEFINE_KEYFIELD( m_iszTarget3, FIELD_STRING, "target3" ),
	DEFINE_KEYFIELD( m_iszTarget4, FIELD_STRING, "target4" ),
	DEFINE_KEYFIELD( m_iszTarget5, FIELD_STRING, "target5" ),
	DEFINE_KEYFIELD( m_iszTarget6, FIELD_STRING, "target6" ),
	DEFINE_KEYFIELD( m_iszTarget7, FIELD_STRING, "target7" ),
	DEFINE_KEYFIELD( m_iszTarget8, FIELD_STRING, "target8" ),

	DEFINE_KEYFIELD( m_BusyActor, FIELD_INTEGER, "busyactor" ),

	DEFINE_FIELD( m_hTarget1, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hTarget2, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hTarget3, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hTarget4, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hTarget5, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hTarget6, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hTarget7, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hTarget8, FIELD_EHANDLE ),

	DEFINE_FIELD( m_bIsPlayingBack, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bPaused, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flCurrentTime, FIELD_FLOAT ),  // relative, not absolute time
	DEFINE_FIELD( m_flFrameTime, FIELD_FLOAT ),  // last frametime
	DEFINE_FIELD( m_bAutomated, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_nAutomatedAction, FIELD_INTEGER ),
	DEFINE_FIELD( m_flAutomationDelay, FIELD_FLOAT ),
	DEFINE_FIELD( m_flAutomationTime, FIELD_FLOAT ),  // relative, not absolute time

	DEFINE_FIELD( m_bPausedViaInput, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bWaitingForActor, FIELD_BOOLEAN ),

	DEFINE_UTLVECTOR( m_hActorList, FIELD_EHANDLE ),

	// DEFINE_FIELD( m_pScene, FIELD_XXXX ) // Special processing used for this

	// These are set up in the constructor
	// DEFINE_FIELD( m_SceneCallback, CChoreoEventCallback ),
	// DEFINE_FIELD( m_pcvSndMixahead, FIELD_XXXXX ),
	// DEFINE_FIELD( m_bRestoring, FIELD_BOOLEAN ),

	DEFINE_FIELD( m_nInterruptCount, FIELD_INTEGER ),
	DEFINE_FIELD( m_bInterrupted, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hInterruptScene, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bCompletedEarly, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bInterruptSceneFinished, FIELD_BOOLEAN ),

	DEFINE_FIELD( m_bGenerated, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iszSoundName, FIELD_STRING ),
	DEFINE_FIELD( m_hActor, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hActivator, FIELD_EHANDLE ),

	DEFINE_UTLVECTOR( m_hNotifySceneCompletion, FIELD_EHANDLE ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "Start", InputStartPlayback ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Pause", InputPausePlayback ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Resume", InputResumePlayback ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Cancel", InputCancelPlayback ),
	DEFINE_INPUTFUNC( FIELD_STRING, "InterjectResponse", 	InputInterjectResponse ),
	DEFINE_INPUTFUNC( FIELD_VOID, "StopWaitingForActor", 	InputStopWaitingForActor ),

//	DEFINE_INPUTFUNC( FIELD_VOID, "Reload", ReloadScene ),

	// Outputs
	DEFINE_OUTPUT( m_OnStart, "OnStart"),
	DEFINE_OUTPUT( m_OnCompletion, "OnCompletion"),
	DEFINE_OUTPUT( m_OnCanceled, "OnCanceled"),
	DEFINE_OUTPUT( m_OnTrigger1, "OnTrigger1"),
	DEFINE_OUTPUT( m_OnTrigger2, "OnTrigger2"),
	DEFINE_OUTPUT( m_OnTrigger3, "OnTrigger3"),
	DEFINE_OUTPUT( m_OnTrigger4, "OnTrigger4"),
	DEFINE_OUTPUT( m_OnTrigger5, "OnTrigger5"),
	DEFINE_OUTPUT( m_OnTrigger6, "OnTrigger6"),
	DEFINE_OUTPUT( m_OnTrigger7, "OnTrigger7"),
	DEFINE_OUTPUT( m_OnTrigger8, "OnTrigger8"),
END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CChoreoEventCallback::CChoreoEventCallback( void )
{
	m_pScene = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *scene - 
//-----------------------------------------------------------------------------
void CChoreoEventCallback::SetScene( CSceneEntity *scene )
{
	m_pScene = scene;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : currenttime - 
//			*event - 
//-----------------------------------------------------------------------------
void CChoreoEventCallback::StartEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event )
{
	if ( !m_pScene )
		return;

	m_pScene->StartEvent( currenttime, scene, event );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : currenttime - 
//			*event - 
//-----------------------------------------------------------------------------
void CChoreoEventCallback::EndEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event )
{
	if ( !m_pScene )
		return;

	m_pScene->EndEvent( currenttime, scene, event );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : currenttime - 
//			*event - 
//-----------------------------------------------------------------------------
void CChoreoEventCallback::ProcessEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event )
{
	if ( !m_pScene )
		return;

	m_pScene->ProcessEvent( currenttime, scene, event );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : currenttime - 
//			*event - 
//-----------------------------------------------------------------------------
bool CChoreoEventCallback::CheckEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event )
{
	if ( !m_pScene )
		return true;

	return m_pScene->CheckEvent( currenttime, scene, event );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CSceneEntity::CSceneEntity( void )
{
	m_bWaitingForActor	= false;
	m_bIsPlayingBack	= false;
	m_bPaused			= false;
	m_iszSceneFile		= NULL_STRING;
	m_flCurrentTime		= 0.0f;

	m_bAutomated		= false;
	m_nAutomatedAction	= SCENE_ACTION_UNKNOWN;
	m_flAutomationDelay = 0.0f;
	m_flAutomationTime = 0.0f;

	m_bPausedViaInput	= false;
	ClearInterrupt();

	m_pScene			= NULL;
	m_SceneCallback.SetScene( this );

	m_bCompletedEarly	= false;

	m_pcvSndMixahead	= cvar->FindVar( "snd_mixahead" );

	m_BusyActor			= SCENE_BUSYACTOR_DEFAULT;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CSceneEntity::~CSceneEntity( void )
{

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSceneEntity::UpdateOnRemove( void )
{
	UnloadScene();
	BaseClass::UpdateOnRemove();

	if ( GetSceneManager() )
	{
		GetSceneManager()->RemoveSceneEntity( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *actor - 
//			*soundname - 
// Output : CChoreoScene
//-----------------------------------------------------------------------------
CChoreoScene *CSceneEntity::GenerateSceneForSound( CBaseFlex *pFlexActor, const char *soundname )
{
	float duration = CBaseEntity::GetSoundDuration( soundname, pFlexActor ? STRING( pFlexActor->GetModelName() ) : NULL );
	if( duration <= 0.0f )
	{
		Warning( "CSceneEntity::GenerateSceneForSound:  Couldn't determine duration of %s\n", soundname );
		return NULL;
	}

	CChoreoScene *scene = new CChoreoScene( &m_SceneCallback );
	if ( !scene )
	{
		Warning( "CSceneEntity::GenerateSceneForSound:  Failed to allocated new scene!!!\n" );
	}
	else
	{
		scene->SetPrintFunc( Scene_Printf );


		CChoreoActor *actor = scene->AllocActor();
		CChoreoChannel *channel = scene->AllocChannel();
		CChoreoEvent *event = scene->AllocEvent();

		Assert( actor );
		Assert( channel );
		Assert( event );

		if ( !actor || !channel || !event )
		{
			Warning( "CSceneEntity::GenerateSceneForSound:  Alloc of actor, channel, or event failed!!!\n" );
			delete scene;
			return NULL;
		}

		// Set us up the actorz
		actor->SetName( "!self" );  // Could be pFlexActor->GetName()?
		actor->SetActive( true );

		// Set us up the channelz
		channel->SetName( STRING( m_iszSceneFile ) );
		channel->SetActor( actor );

		// Add to actor
		actor->AddChannel( channel );
	
		// Set us up the eventz
		event->SetType( CChoreoEvent::SPEAK );
		event->SetName( soundname );
		event->SetParameters( soundname );
		event->SetStartTime( 0.0f );
		event->SetUsingRelativeTag( false );
		event->SetEndTime( duration );
		event->SnapTimes();

		// Add to channel
		channel->AddEvent( event );

		// Point back to our owners
		event->SetChannel( channel );
		event->SetActor( actor );

	}

	return scene;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSceneEntity::Activate()
{
	if ( m_bGenerated && !m_pScene )
	{
		m_pScene = GenerateSceneForSound( m_hActor, STRING( m_iszSoundName ) );
	}

	BaseClass::Activate();

	if ( GetSceneManager() )
	{
		GetSceneManager()->AddSceneEntity( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CSceneEntity::GetSoundSystemLatency( void )
{
	if ( m_pcvSndMixahead )
	{
		return m_pcvSndMixahead->GetFloat();
	}
	
	// Assume 100 msec sound system latency
	return SOUND_SYSTEM_LATENCY_DEFAULT;
}
		
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *scene - 
//-----------------------------------------------------------------------------
void CSceneEntity::PrecacheScene( CChoreoScene *scene )
{
	Assert( scene );

	// Iterate events and precache necessary resources
	for ( int i = 0; i < scene->GetNumEvents(); i++ )
	{
		CChoreoEvent *event = scene->GetEvent( i );
		if ( !event )
			continue;

		// load any necessary data
		switch (event->GetType() )
		{
		default:
			break;
		case CChoreoEvent::SPEAK:
			{
				// Defined in SoundEmitterSystem.cpp
				// NOTE:  The script entries associated with .vcds are forced to preload to avoid
				//  loading hitches during triggering
				PrecacheScriptSound( event->GetParameters() );

				if ( event->GetCloseCaptionType() == CChoreoEvent::CC_MASTER && 
					 event->GetNumSlaves() > 0 )
				{
					char tok[ CChoreoEvent::MAX_CCTOKEN_STRING ];
					if ( event->GetPlaybackCloseCaptionToken( tok, sizeof( tok ) ) )
					{
						PrecacheScriptSound( tok );
					}
				}
			}
			break;
		case CChoreoEvent::SUBSCENE:
			{
				// Only allow a single level of subscenes for now
				if ( !scene->IsSubScene() )
				{
					CChoreoScene *subscene = event->GetSubScene();
					if ( !subscene )
					{
						subscene = LoadScene( event->GetParameters() );
						subscene->SetSubScene( true );
						event->SetSubScene( subscene );

						// Now precache it's resources, if any
						PrecacheScene( subscene );
					}
				}
			}
			break;
		}
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSceneEntity::Precache( void )
{
	if ( m_bGenerated )
		return;

	LoadSceneFromFile( STRING( m_iszSceneFile ) );
	if ( !m_pScene )
		return;

	PrecacheScene( m_pScene );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pActor - 
//			*soundname - 
//-----------------------------------------------------------------------------
void CSceneEntity::GenerateSoundScene( CBaseFlex *pActor, const char *soundname )
{
	m_bGenerated	= true;
	m_iszSoundName	= MAKE_STRING( soundname );
	m_hActor		= pActor;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CSceneEntity::HasUnplayedSpeech( void )
{
	if ( m_pScene )
		return m_pScene->HasUnplayedSpeech();

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CSceneEntity::HasFlexAnimation( void )
{
	if ( m_pScene )
		return m_pScene->HasFlexAnimation();

	return false;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------

void CSceneEntity::SetBackground( bool bIsBackground )
{
	if ( m_pScene )
		m_pScene->SetBackground( bIsBackground );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------

bool CSceneEntity::IsBackground( void )
{
	if ( m_pScene )
		return m_pScene->IsBackground( );

	return false;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

void CSceneEntity::OnRestore()
{
	BaseClass::OnRestore();

	if ( !m_pScene )
		return;

	if ( !m_bIsPlayingBack )
		return;

	int i;
	for ( i = 0 ; i < m_pScene->GetNumActors(); i++ )
	{
		CBaseFlex *pTestActor = FindNamedActor( i );
		if ( !pTestActor )
			continue;

		if ( !pTestActor->MyNPCPointer() )
			continue;

		// Needed?
		//if ( !pTestActor->MyNPCPointer()->IsAlive() )
		//	return;

		pTestActor->StartChoreoScene( m_pScene );
	}

	float dt = SCENE_THINK_INTERVAL;

	bool paused = m_bPaused;

	m_bPaused = false;

	// roll back slightly so that pause events still trigger
	m_pScene->ResetSimulation( true, m_flCurrentTime - SCENE_THINK_INTERVAL, m_flCurrentTime );
	m_pScene->SetTime( m_flCurrentTime - SCENE_THINK_INTERVAL );
	ClearInterrupt();

	m_bRestoring = true;

	DoThink( dt );

	m_bRestoring = false;
	if ( paused )
	{
		PausePlayback();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSceneEntity::Spawn( void )
{
	Precache();
}

void CSceneEntity::PauseThink( void )
{
	if ( !m_pScene )
		return;

	// Stay paused if pause occurred from interrupt
	if ( m_bInterrupted )
		return;

	// If entity I/O paused the scene, then it'll have to resume/cancel the scene...
	if ( m_bPausedViaInput )
		return;

	// FIXME:  Game code should check for AI waiting conditions being met, etc.
	//
	//
	//
	bool bAllFinished = m_pScene->CheckEventCompletion( );

	if ( bAllFinished )
	{
		// Perform action
		switch ( m_nAutomatedAction )
		{
		case SCENE_ACTION_RESUME:
			ResumePlayback();
			break;
		case SCENE_ACTION_CANCEL:
			CancelPlayback();
			break;
		default:
			ResumePlayback();
			break;
		}

		// Reset
		m_bAutomated = false;
		m_nAutomatedAction = SCENE_ACTION_UNKNOWN;
		m_flAutomationTime = 0.0f;
		m_flAutomationDelay = 0.0f;
		m_bPausedViaInput = false;
		return;
	}

	// Otherwise, see if resume/cancel is automatic and act accordingly if enough time
	//  has passed
	if ( !m_bAutomated )
		return;

	m_flAutomationTime += (gpGlobals->curtime - GetLastThink());

	if ( m_flAutomationDelay > 0.0f &&
		m_flAutomationTime < m_flAutomationDelay )
		return;

	// Perform action
	switch ( m_nAutomatedAction )
	{
	case SCENE_ACTION_RESUME:
		Scene_Printf( "%s : Automatically resuming playback\n", STRING( m_iszSceneFile ) );
		ResumePlayback();
		break;
	case SCENE_ACTION_CANCEL:
		Scene_Printf( "%s : Automatically canceling playback\n", STRING( m_iszSceneFile ) );
		CancelPlayback();
		break;
	default:
		Scene_Printf( "%s : Unknown action %i, automatically resuming playback\n", STRING( m_iszSceneFile ), m_nAutomatedAction );
		ResumePlayback();
		break;
	}

	// Reset
	m_bAutomated = false;
	m_nAutomatedAction = SCENE_ACTION_UNKNOWN;
	m_flAutomationTime = 0.0f;
	m_flAutomationDelay = 0.0f;
	m_bPausedViaInput = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchPauseScene( CChoreoScene *scene, const char *parameters )
{
	// Don't pause during restore, since we'll be restoring the pause state already
	if ( m_bRestoring )
		return;

	// FIXME:  Hook this up to AI, etc. somehow, perhaps poll each actor for conditions using
	//  scene resume condition iterator
	PausePlayback();

	char token[1024];

	m_bPausedViaInput = false;
	m_bAutomated		= false;
	m_nAutomatedAction	= SCENE_ACTION_UNKNOWN;
	m_flAutomationDelay = 0.0f;
	m_flAutomationTime = 0.0f;

	// Check for auto resume/cancel
	const char *buffer = parameters;
	buffer = engine->ParseFile( buffer, token, sizeof( token ) );
	if ( !stricmp( token, "automate" ) )
	{
		buffer = engine->ParseFile( buffer, token, sizeof( token ) );
		if ( !stricmp( token, "Cancel" ) )
		{
			m_nAutomatedAction = SCENE_ACTION_CANCEL;
		}
		else if ( !stricmp( token, "Resume" ) )
		{
			m_nAutomatedAction = SCENE_ACTION_RESUME;
		}

		if ( m_nAutomatedAction != SCENE_ACTION_UNKNOWN )
		{
			buffer = engine->ParseFile( buffer, token, sizeof( token ) );
			m_flAutomationDelay = (float)atof( token );

			if ( m_flAutomationDelay > 0.0f )
			{
				// Success
				m_bAutomated = true;
				m_flAutomationTime = 0.0f;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *scene - 
//			*event - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchProcessLoop( CChoreoScene *scene, CChoreoEvent *event )
{
	// Don't restore this event since it's implied in the current "state" of the scene timer, etc.
	if ( m_bRestoring )
		return;

	Assert( scene );
	Assert( event->GetType() == CChoreoEvent::LOOP );

	float backtime = (float)atof( event->GetParameters() );

	bool process = true;
	int counter = event->GetLoopCount();
	if ( counter != -1 )
	{
		int remaining = event->GetNumLoopsRemaining();
		if ( remaining <= 0 )
		{
			process = false;
		}
		else
		{
			event->SetNumLoopsRemaining( --remaining );
		}
	}

	if ( !process )
		return;

	scene->LoopToTime( backtime );
	m_flCurrentTime = backtime;
}

//-----------------------------------------------------------------------------
// Purpose: Flag the scene as already "completed"
// Input  : *scene - 
//			*parameters - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchStopPoint( CChoreoScene *scene, const char *parameters )
{
	// Fire completion trigger early
	m_bCompletedEarly = true;
	m_OnCompletion.FireOutput( this, this, 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CSceneEntity::IsInterruptable()
{
	return ( m_nInterruptCount > 0 ) ? true : false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *scene - 
//			*actor - 
//			*event - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchStartInterrupt( CChoreoScene *scene, CChoreoEvent *event )
{
	// Don't re-interrupt during restore
	if ( m_bRestoring )
		return;

	++m_nInterruptCount;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *scene - 
//			*actor - 
//			*event - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchEndInterrupt( CChoreoScene *scene, CChoreoEvent *event )
{
	// Don't re-interrupt during restore
	if ( m_bRestoring )
		return;

	--m_nInterruptCount;

	if ( m_nInterruptCount < 0 )
	{
		m_nInterruptCount = 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *actor - 
//			*event - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchStartExpression( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event )
{
	actor->AddSceneEvent( scene, event );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *actor - 
//			*event - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchEndExpression( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event )
{
	actor->RemoveSceneEvent( scene, event, false );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *actor - 
//			*event - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchStartFlexAnimation( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event )
{
	actor->AddSceneEvent( scene, event );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *actor - 
//			*event - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchEndFlexAnimation( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event )
{
	actor->RemoveSceneEvent( scene, event, false );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *actor - 
//			*parameters - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchStartGesture( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event )
{
	// Ingore null gestures
	if ( !Q_stricmp( event->GetName(), "NULL" ) )
		return;

	actor->AddSceneEvent( scene, event); 
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *actor - 
//			*parameters - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchEndGesture( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event )
{
	// Ingore null gestures
	if ( !Q_stricmp( event->GetName(), "NULL" ) )
		return;

	actor->RemoveSceneEvent( scene, event, m_bRestoring );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *actor - 
//			*parameters - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchStartGeneric( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event )
{
	actor->AddSceneEvent( scene, event );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *actor - 
//			*parameters - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchEndGeneric( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event )
{
	actor->RemoveSceneEvent( scene, event, m_bRestoring );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *actor - 
//			*actor2 - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchStartLookAt( CChoreoScene *scene, CBaseFlex *actor, CBaseEntity *actor2, CChoreoEvent *event )
{
	actor->AddSceneEvent( scene, event, actor2 );
}


void CSceneEntity::DispatchEndLookAt( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event )
{
	actor->RemoveSceneEvent( scene, event, m_bRestoring );
}


//-----------------------------------------------------------------------------
// Purpose: Move to spot/actor
// FIXME:  Need to allow this to take arbitrary amount of time and pause playback
//  while waiting for actor to move into position
// Input  : *actor - 
//			*parameters - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchStartMoveTo( CChoreoScene *scene, CBaseFlex *actor, CBaseEntity *actor2, CChoreoEvent *event )
{
	actor->AddSceneEvent( scene, event, actor2 );
}


void CSceneEntity::DispatchEndMoveTo( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event )
{
	actor->RemoveSceneEvent( scene, event, m_bRestoring );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *token - 
//			listener - 
//			soundorigins - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool AttenuateCaption( const char *token, const Vector& listener, CUtlVector< Vector >& soundorigins )
{
	if ( scene_maxcaptionradious.GetFloat() <= 0.0f )
	{
		return false;
	}

	int c = soundorigins.Count();

	if ( c <= 0 )
	{
		return false;
	}

	float maxdistSqr = scene_maxcaptionradious.GetFloat() * scene_maxcaptionradious.GetFloat();

	for ( int i = 0; i  < c; ++i )
	{
		const Vector& org = soundorigins[ i ];

		float distSqr = ( org - listener ).LengthSqr();
		if ( distSqr <= maxdistSqr )
		{
			return false;
		}
	}

	// All sound sources too far, don't show caption...
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *event - which event 
//			player - which recipient 
//			buf, buflen:  where to put the data 
// Output : Returns true if the sound should be played/prefetched
//-----------------------------------------------------------------------------
bool CSceneEntity::GetSoundNameForPlayer( CChoreoEvent *event, CBasePlayer *player, char *buf, size_t buflen )
{
	Assert( event );
	Assert( player );
	Assert( buf );
	Assert( buflen > 0 );

	bool ismasterevent = true;
	char tok[ CChoreoEvent::MAX_CCTOKEN_STRING ];
	bool validtoken = false;

	tok[ 0 ] = 0;

	if ( event->GetCloseCaptionType() == CChoreoEvent::CC_SLAVE ||
		event->GetCloseCaptionType() == CChoreoEvent::CC_DISABLED )
	{
		ismasterevent = false;
	}
	else
	{
		validtoken = event->GetPlaybackCloseCaptionToken( tok, sizeof( tok ) );
	}

	// For english users, assume we emit the sound normally
	Q_strncpy( buf, event->GetParameters(), buflen );

	bool usingEnglish = true;
	char const *cvarvalue = engine->GetClientConVarValue( player->entindex(), "english" );
	if ( cvarvalue && Q_atoi( cvarvalue ) != 1 )
	{
		usingEnglish = false;
	}

	// This makes it like they are running in another language
	if ( scene_forcecombined.GetBool() )
	{
		usingEnglish = false;
	}

	if ( usingEnglish )
	{
		// English sounds always play
		return true;
	}
	
	if ( ismasterevent )
	{
		// Master event sounds always play too (master will be the combined .wav)
		if ( validtoken )
		{
			Q_strncpy( buf, tok, buflen );
		}
		return true;
	}

	// Slave events don't play any sound...
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Playback sound file that contains phonemes
// Input  : *actor - 
//			*parameters - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchStartSpeak( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event, soundlevel_t iSoundlevel )
{
	// Emit sound
	if ( actor )
	{
		CPASAttenuationFilter filter( actor );

		float time_in_past = m_flCurrentTime - event->GetStartTime() ;

		float soundtime = gpGlobals->curtime - time_in_past;

		if ( m_bRestoring )
		{
			// Need to queue sounds on restore because the player has not yet connected
			GetSceneManager()->QueueRestoredSound( actor, event->GetParameters(), iSoundlevel, time_in_past );

			return;
		}

		// Add padding to prevent any other talker talking right after I'm done, because I might 
		// be continuing speaking with another scene.
		float flDuration = event->GetDuration() - time_in_past;

		CAI_BaseActor *pBaseActor = dynamic_cast<CAI_BaseActor*>(actor);
		if ( pBaseActor )
		{
			pBaseActor->NoteSpeaking( flDuration, GetPostSpeakDelay() );
		}
		else if ( actor->IsNPC() )
		{
			GetSpeechSemaphore( actor->MyNPCPointer() )->Acquire( flDuration + GetPostSpeakDelay(), actor );
		}

		EmitSound_t es;
		es.m_nChannel = CHAN_VOICE;
		es.m_flVolume = 1;
		es.m_SoundLevel = iSoundlevel;
		es.m_flSoundTime = soundtime;

		// No CC since we do it manually
		// FIXME:  This will  change
		es.m_bEmitCloseCaption = false;

		int c = filter.GetRecipientCount();
		for ( int i = 0; i < c; ++i )
		{
			int playerindex = filter.GetRecipientIndex( i );
			CBasePlayer *player = UTIL_PlayerByIndex( playerindex );
			if ( !player )
				continue;

			CSingleUserRecipientFilter filter2( player );

			char soundname[ 512 ];
			if ( !GetSoundNameForPlayer( event, player, soundname, sizeof( soundname ) ) )
			{
				continue;
			}

			es.m_pSoundName = soundname;

			// Warning( "Speak %s\n", soundname );

			EmitSound( filter2, actor->entindex(), es );
			actor->AddSceneEvent( scene, event );
		}
	

		// Close captioning only on master token no matter what...
		if ( event->GetCloseCaptionType() == CChoreoEvent::CC_MASTER )
		{
			char tok[ CChoreoEvent::MAX_CCTOKEN_STRING ];
			bool validtoken = event->GetPlaybackCloseCaptionToken( tok, sizeof( tok ) );
			if ( validtoken )
			{
				CRC32_t tokenCRC;
				CRC32_Init( &tokenCRC );

				char lowercase[ 256 ];
				Q_strncpy( lowercase, tok, sizeof( lowercase ) );
				Q_strlower( lowercase );

				CRC32_ProcessBuffer( &tokenCRC, lowercase, Q_strlen( lowercase ) );
				CRC32_Final( &tokenCRC );

#if !defined( CLIENT_DLL )
				{
					void RememberCRC( const CRC32_t& crc, const char *tokenname );
					RememberCRC( tokenCRC, lowercase );
				}
#endif

				// Remove any players who don't want close captions
				CBaseEntity::RemoveRecipientsIfNotCloseCaptioning( filter );

				// Certain events are marked "don't attenuate", (breencast), skip those here
				if ( !event->IsSuppressingCaptionAttenuation() && 
					( filter.GetRecipientCount() > 0 ) )
				{
					int c = filter.GetRecipientCount();
					for ( int i = c - 1 ; i >= 0; --i )
					{
						CBasePlayer *player = UTIL_PlayerByIndex( filter.GetRecipientIndex( i ) );
						if ( !player )
							continue;

						Vector playerOrigin = player->GetAbsOrigin();

						if ( AttenuateCaption( lowercase, playerOrigin, es.m_UtlVecSoundOrigin ) )
						{
							filter.RemoveRecipient( player );
						}
					}
				}

				// Anyone left?
				if ( filter.GetRecipientCount() > 0 )
				{
					float endtime = event->GetLastSlaveEndTime();
					float durationShort = event->GetDuration();
					float durationLong = endtime - event->GetStartTime();

					float duration = max( durationShort, durationLong );


					byte byteflags = ( 1<<0 ); // warnifmissing
					/*
					// Never for .vcds...
					if ( fromplayer )
					{
						byteflags |= ( 1<<1 );
					}
					*/
					// Send caption and duration hint down to client
					UserMessageBegin( filter, "CloseCaption" );
						WRITE_LONG( tokenCRC );
						WRITE_BYTE( min( 255, (int)( duration * 10.0f ) ) );
						WRITE_BYTE( byteflags ); // warn on missing
					MessageEnd();
				}
			}
		}


	}
}

void CSceneEntity::DispatchEndSpeak( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event )
{
	actor->RemoveSceneEvent( scene, event, m_bRestoring );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *actor - 
//			*actor2 - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchStartFace( CChoreoScene *scene, CBaseFlex *actor, CBaseEntity *actor2, CChoreoEvent *event )
{
	actor->AddSceneEvent( scene, event, actor2 );
}



//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *actor - 
//			*actor2 - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchEndFace( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event )
{
	actor->RemoveSceneEvent( scene, event, m_bRestoring );
}



//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *actor - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchStartSequence( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event )
{
	actor->AddSceneEvent( scene, event );
}



//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *actor - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchEndSequence( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event )
{
	actor->RemoveSceneEvent( scene, event, m_bRestoring );
}

//-----------------------------------------------------------------------------
// Purpose: NPC can play interstitial vcds (such as responding to the player doing something during a scene)
// Input  : *scene - 
//			*actor - 
//			*event - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchStartPermitResponses( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event )
{
	actor->SetPermitResponse( gpGlobals->curtime + event->GetDuration() );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *scene - 
//			*actor - 
//			*event - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchEndPermitResponses( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event )
{
	actor->SetPermitResponse( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CSceneEntity::EstimateLength( void )
{
	return ( m_pScene ) ? m_pScene->FindStopTime( ) : 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CSceneEntity::InvolvesActor( CBaseEntity *pActor )
{
 	if ( !m_pScene )
		return false;	

	int i;
	for ( i = 0 ; i < m_pScene->GetNumActors(); i++ )
	{
		CBaseFlex *pTestActor = FindNamedActor( i );
		if ( !pTestActor )
			continue;

		if ( pTestActor == pActor )
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSceneEntity::DoThink( float frametime )
{
	CheckInterruptCompletion();

	if ( m_bWaitingForActor )
	{
		// Try to start playback.
		StartPlayback();
	}

	if ( !m_pScene )
		return;

	if ( !m_bIsPlayingBack )
		return;

	if ( m_bPaused )
	{
		PauseThink();
		return;
	}

	// Msg("%.2f %s\n", gpGlobals->curtime, STRING( m_iszSceneFile ) );

	m_flFrameTime = frametime;

	m_pScene->SetSoundFileStartupLatency( GetSoundSystemLatency() );

	// Tell scene to go
	m_pScene->Think( m_flCurrentTime );

	// Drive simulation time for scene
	m_flCurrentTime += m_flFrameTime;

	// Did we get to the end
	if ( !m_bPaused && m_pScene->SimulationFinished() )
	{
		OnSceneFinished( false, true );

		// Stop them from doing anything special
		ClearSchedules( m_pScene );
	}

	if ( m_bPaused )
	{
		// roll back clock to pause point
		m_flCurrentTime = m_pScene->GetTime();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Input handlers
//-----------------------------------------------------------------------------
void CSceneEntity::InputStartPlayback( inputdata_t &inputdata )
{
	// Already playing, ignore
	if ( m_bIsPlayingBack )
		return;

	// Already waiting on someone.
	if ( m_bWaitingForActor )
		return;

	ClearActivatorTargets();
	m_hActivator = inputdata.pActivator;
	StartPlayback();
}

void CSceneEntity::InputPausePlayback( inputdata_t &inputdata )
{
	PausePlayback();
	m_bPausedViaInput = true;
}

void CSceneEntity::InputResumePlayback( inputdata_t &inputdata )
{
	ResumePlayback();
}

void CSceneEntity::InputCancelPlayback( inputdata_t &inputdata )
{
	CancelPlayback();
}

void CSceneEntity::InputReloadScene( inputdata_t &inputdata )
{
	ReloadScene();
}

struct NPCInterjection
{
	AI_Response *response;
	CAI_BaseActor *npc;
};
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CSceneEntity::InputInterjectResponse( inputdata_t &inputdata )
{
	// Not currently playing a scene
	if ( !m_pScene )
	{
		return;
	}

	CUtlVector< CAI_BaseActor * >	candidates;
	int i;
	for ( i = 0 ; i < m_pScene->GetNumActors(); i++ )
	{
		CBaseFlex *pTestActor = FindNamedActor( i );
		if ( !pTestActor )
			continue;

		CAI_BaseActor *pBaseActor = dynamic_cast<CAI_BaseActor*>(pTestActor);
		if ( !pBaseActor )
			continue;

		if ( !pBaseActor->IsAlive() )
			continue;

		candidates.AddToTail( pBaseActor );
	}

	int c = candidates.Count();

	if ( !c )
	{
		return;
	}
	
	int useIndex = 0;
	if ( !m_bIsPlayingBack )
	{
		// Use any actor if not playing a scene
		useIndex = RandomInt( 0, c - 1 );
	}
	else
	{
		CUtlVector< NPCInterjection > validResponses;

		char modifiers[ 512 ];
		Q_snprintf( modifiers, sizeof( modifiers ), "scene:%s", STRING( GetEntityName() ) );

		for ( int i = 0; i < c; i++ )
		{
			CAI_BaseActor *npc = candidates[ i ];
			Assert( npc );

			AI_Response *response = npc->SpeakFindResponse( inputdata.value.String(), modifiers );
			if ( !response )
				continue;

			float duration = npc->GetResponseDuration( response );
			// Couldn't look it up
			if ( duration <= 0.0f )
				continue;

			if ( !npc->PermitResponse( duration ) )
			{
				delete response;
				continue;
			}

			// 
			NPCInterjection inter;
			inter.response = response;
			inter.npc = npc;

			validResponses.AddToTail( inter );
		}

		int rcount = validResponses.Count();
		if ( rcount >= 1 )
		{
			int slot = RandomInt( 0, rcount - 1 );

			for ( int i = 0; i < rcount; i++ )
			{
				NPCInterjection *pInterjection = &validResponses[ i ];
				if ( i == slot )
				{
					pInterjection->npc->SpeakDispatchResponse( inputdata.value.String(), pInterjection->response );
				}
				else
				{
					delete pInterjection->response;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CSceneEntity::InputStopWaitingForActor( inputdata_t &inputdata )
{
	if( m_bIsPlayingBack )
	{
		// Already started.
		return;
	}

	m_bWaitingForActor = false;
}

//-----------------------------------------------------------------------------
// Purpose: Initiate scene playback
//-----------------------------------------------------------------------------
void CSceneEntity::StartPlayback( void )
{
	if ( !m_pScene )
		return;

	if ( m_bIsPlayingBack )
		return;

	int i;
	for ( i = 0 ; i < m_pScene->GetNumActors(); i++ )
	{
		CBaseFlex *pTestActor = FindNamedActor( i );
		if ( !pTestActor )
			continue;

		if ( !pTestActor->MyNPCPointer() )
			continue;

		if ( !pTestActor->MyNPCPointer()->IsAlive() )
			return;

		if ( m_BusyActor == SCENE_BUSYACTOR_WAIT )
		{
			CAI_BaseNPC *pActor = pTestActor->MyNPCPointer();
			if( pActor && pActor->GetExpresser()->IsSpeaking() )
			{
				// One of the actors for this scene is talking already.
				// Try again next think.
				m_bWaitingForActor = true;
				return;
			}
		}

		pTestActor->StartChoreoScene( m_pScene );
	}

	m_bWaitingForActor	= false;
	m_bIsPlayingBack	= true;
	m_bPaused			= false;
	m_flCurrentTime		= 0.0f;
	m_pScene->ResetSimulation();
	ClearInterrupt();

	// Put face back in neutral pose
	ClearSceneEvents( m_pScene, false );

	m_OnStart.FireOutput( this, this, 0 );

	// Aysnchronously load speak sounds
	CUtlSymbolTable prefetchSoundSymbolTable;
	CUtlRBTree< SpeakEventSound_t > soundnames( 0, 0, SpeakEventSoundLessFunc );

	BuildSortedSpeakEventSoundsPrefetchList( m_pScene, prefetchSoundSymbolTable, soundnames, 0.0f );
	PrefetchSpeakEventSounds( prefetchSoundSymbolTable, soundnames );

	return;
}

//-----------------------------------------------------------------------------
// Purpose: Static method used to sort by event start time
//-----------------------------------------------------------------------------
bool CSceneEntity::SpeakEventSoundLessFunc( const SpeakEventSound_t& lhs, const SpeakEventSound_t& rhs )
{
	return lhs.m_flStartTime < rhs.m_flStartTime;
}

//-----------------------------------------------------------------------------
// Purpose: Prefetches the list of sounds build by BuildSortedSpeakEventSoundsPrefetchList
//-----------------------------------------------------------------------------
void CSceneEntity::PrefetchSpeakEventSounds( CUtlSymbolTable& table, CUtlRBTree< SpeakEventSound_t >& soundnames )
{
	for ( int i = soundnames.FirstInorder(); i != soundnames.InvalidIndex() ; i = soundnames.NextInorder( i ) )
	{
		SpeakEventSound_t& sound = soundnames[ i ];
		// Look it up in the string table
		char const *soundname = table.String( sound.m_Symbol );

		// Warning( "Prefetch %s\n", soundname );

		PrefetchScriptSound( soundname );  
	}
}

//-----------------------------------------------------------------------------
// Purpose:  Builds list of sounds sorted by start time for prefetching 
//-----------------------------------------------------------------------------
void CSceneEntity::BuildSortedSpeakEventSoundsPrefetchList( 
	CChoreoScene *scene, 
	CUtlSymbolTable& table, 
	CUtlRBTree< SpeakEventSound_t >& soundnames,
	float timeOffset )
{
	Assert( scene );

	// Iterate events and precache necessary resources
	for ( int i = 0; i < scene->GetNumEvents(); i++ )
	{
		CChoreoEvent *event = scene->GetEvent( i );
		if ( !event )
			continue;

		// load any necessary data
		switch (event->GetType() )
		{
		default:
			break;
		case CChoreoEvent::SPEAK:
			{
				
				// NOTE:  The script entries associated with .vcds are forced to preload to avoid
				//  loading hitches during triggering
				char soundname[ CChoreoEvent::MAX_CCTOKEN_STRING ];
				Q_strncpy( soundname, event->GetParameters(), sizeof( soundname ) );
				
				if ( event->GetCloseCaptionType() == CChoreoEvent::CC_MASTER )
				{
					event->GetPlaybackCloseCaptionToken( soundname, sizeof( soundname ) );
				}
				
				// In single player, try to use the combined or regular .wav files as needed
				if ( gpGlobals->maxClients == 1 )
				{
					CBasePlayer *player = UTIL_GetLocalPlayer();
					if ( player && !GetSoundNameForPlayer( event, player, soundname, sizeof( soundname ) ) )
					{
						// Skip to next event
						continue;
					}
				}
				/*
				else
				{
					// UNDONE:  Probably need some other solution in multiplayer... (not sure how to "prefetch" on certain players
					// with one sound, but not prefetch the same sound for others...)
				}
				*/

				SpeakEventSound_t ses;
				ses.m_Symbol = table.AddString( soundname );
				ses.m_flStartTime = timeOffset + event->GetStartTime();

				soundnames.Insert( ses );
			}
			break;
		case CChoreoEvent::SUBSCENE:
			{
				// Only allow a single level of subscenes for now
				if ( !scene->IsSubScene() )
				{
					CChoreoScene *subscene = event->GetSubScene();
					if ( !subscene )
					{
						subscene = LoadScene( event->GetParameters() );
						subscene->SetSubScene( true );
						event->SetSubScene( subscene );

						// Now precache it's resources, if any
						BuildSortedSpeakEventSoundsPrefetchList( subscene, table, soundnames, event->GetStartTime() );
					}
				}
			}
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSceneEntity::PausePlayback( void )
{
	if ( !m_bIsPlayingBack )
		return;

	if ( m_bPaused )
		return;

	m_bPaused = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSceneEntity::ResumePlayback( void )
{
	if ( !m_bIsPlayingBack )
		return;

	if ( !m_bPaused )
		return;

	// FIXME:  Iterate using m_pScene->IterateResumeConditionEvents and 
	//  only resume if the event conditions have all been satisfied

	// FIXME:  Just resume for now
	m_pScene->ResumeSimulation();

	m_bPaused = false;
	m_bPausedViaInput = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSceneEntity::CancelPlayback( void )
{
	if ( !m_bIsPlayingBack )
		return;

	m_bIsPlayingBack	= false;
	m_bPaused			= false;

	m_OnCanceled.FireOutput( this, this, 0 );

	OnSceneFinished( true, false );
}

//-----------------------------------------------------------------------------
// Purpose: Reload from hard drive
//-----------------------------------------------------------------------------
void CSceneEntity::ReloadScene( void )
{
	CancelPlayback();

	Msg( "Reloading %s from disk\n", STRING( m_iszSceneFile ) );

	// Note:  LoadSceneFromFile calls UnloadScene()
	LoadSceneFromFile( STRING( m_iszSceneFile ) );
}

//-----------------------------------------------------------------------------
// Purpose: Query whether the scene actually loaded. Only meaninful after Spawn()
//-----------------------------------------------------------------------------
bool CSceneEntity::ValidScene() const
{
	return ( m_pScene != NULL );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pActor - 
//			*scene - 
//			*event - 
//-----------------------------------------------------------------------------
void CSceneEntity::DispatchStartSubScene( CChoreoScene *scene, CBaseFlex *pActor, CChoreoEvent *event)
{
	if ( !scene->IsSubScene() )
	{
		CChoreoScene *subscene = event->GetSubScene();
		if ( !subscene )
		{
			subscene = LoadScene( event->GetParameters() );
			subscene->SetSubScene( true );
			event->SetSubScene( subscene );
		}

		if ( subscene )
		{
			subscene->ResetSimulation();
		}
	}
}
//-----------------------------------------------------------------------------
// Purpose: All events are leading edge triggered
// Input  : currenttime - 
//			*event - 
//-----------------------------------------------------------------------------
void CSceneEntity::StartEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event )
{
	Assert( event );

	if ( !Q_stricmp( event->GetName(), "NULL" ) )
 	{
 		Scene_Printf( "%s : %8.2f:  ignored %s\n", STRING( m_iszSceneFile ), currenttime, event->GetDescription() );
 		return;
 	}
 

	CBaseFlex *pActor = NULL;
	CChoreoActor *actor = event->GetActor();
	if ( actor )
	{
		pActor = FindNamedActor( actor );
		if (pActor == NULL)
		{
			Warning( "CSceneEntity %s unable to find actor named \"%s\"\n", STRING(GetEntityName()), actor->GetName() );
			return;
		}
	}

	Scene_Printf( "%s : %8.2f:  start %s\n", STRING( m_iszSceneFile ), currenttime, event->GetDescription() );

	switch ( event->GetType() )
	{
	case CChoreoEvent::SUBSCENE:
		{
			if ( pActor )
			{
				DispatchStartSubScene( scene, pActor, event );
			}
		}
		break;
	case CChoreoEvent::EXPRESSION:
		{
			if ( pActor )
			{
				DispatchStartExpression( scene, pActor, event );
			}
		}
		break;
	case CChoreoEvent::FLEXANIMATION:
		{
			if ( pActor )
			{
				DispatchStartFlexAnimation( scene, pActor, event );
			}
		}
		break;
	case CChoreoEvent::LOOKAT:
		{
			if ( pActor )
			{
				CBaseEntity *pActor2 = FindNamedEntity( event->GetParameters( ), pActor );
				if ( pActor2 )
				{
					// Huh?
					DispatchStartLookAt( scene, pActor, pActor2, event );
				}
				else
				{
					Warning( "CSceneEntity %s unable to find actor named \"%s\"\n", STRING(GetEntityName()), event->GetParameters() );
				}
			}
		}
		break;
	case CChoreoEvent::SPEAK:
		{
			if ( pActor )
			{
				// Speaking is edge triggered

				// FIXME: dB hack.  soundlevel needs to be moved into inside of wav?
				soundlevel_t iSoundlevel = SNDLVL_TALKING;
				if (event->GetParameters2())
				{
					iSoundlevel = (soundlevel_t)atoi( event->GetParameters2() );
					if (iSoundlevel == SNDLVL_NONE)
						iSoundlevel = SNDLVL_TALKING;
				}

				DispatchStartSpeak( scene, pActor, event, iSoundlevel );
			}
		}
		break;
	case CChoreoEvent::MOVETO:
		{
			// FIXME: make sure moveto's aren't edge triggered
			if ( !event->HasEndTime() )
			{
				event->SetEndTime( event->GetStartTime() + 1.0 );
			}

			if ( pActor )
			{
				CBaseEntity *pActor2 = FindNamedEntity( event->GetParameters( ), pActor );
				if ( pActor2 )
				{
					DispatchStartMoveTo( scene, pActor, pActor2, event );
				}
				else
				{
					Warning( "CSceneEntity %s unable to find actor named \"%s\"\n", STRING(GetEntityName()), event->GetParameters() );
				}
			}
		}
		break;
	case CChoreoEvent::FACE:
		{
			if ( pActor )
			{
				CBaseEntity *pActor2 = FindNamedEntity( event->GetParameters( ), pActor );
				if ( pActor2 )
				{
					DispatchStartFace( scene, pActor, pActor2, event );
				}
				else
				{
					Warning( "CSceneEntity %s unable to find actor named \"%s\"\n", STRING(GetEntityName()), event->GetParameters() );
				}
			}
		}
		break;
	case CChoreoEvent::GESTURE:
		{
			if ( pActor )
			{
				DispatchStartGesture( scene, pActor, event );
			}
		}
		break;
	case CChoreoEvent::GENERIC:
		{
			// If the first token in the parameters is "debugtext", print the rest of the text
			if ( event->GetParameters() && !Q_strncmp( event->GetParameters(), "debugtext", 9 ) )
			{
				const char *pszText = event->GetParameters() + 10;

				hudtextparms_s tTextParam;
				tTextParam.x			= -1;
				tTextParam.y			= 0.65;
				tTextParam.effect		= 0;
				tTextParam.r1			= 255;
				tTextParam.g1			= 170;
				tTextParam.b1			= 0;
				tTextParam.a1			= 255;
				tTextParam.r2			= 255;
				tTextParam.g2			= 170;
				tTextParam.b2			= 0;
				tTextParam.a2			= 255;
				tTextParam.fadeinTime	= 0;
				tTextParam.fadeoutTime	= 0;
				tTextParam.holdTime		= 3.1;
				tTextParam.fxTime		= 0;
				tTextParam.channel		= 1;
				UTIL_HudMessageAll( tTextParam, pszText );
				break;
			}

			if ( pActor )
			{
				DispatchStartGeneric( scene, pActor, event );
			}
		}
		break;
	case CChoreoEvent::FIRETRIGGER:
		{
			// Don't re-fire triggers during restore, the entities should already reflect all such state...
			if ( m_bRestoring )
			{
				break;
			}

			CBaseEntity *pActivator = pActor;
			if (!pActivator)
			{
				pActivator = this;
			}

			// FIXME:  how do I decide who fired it??
			switch( atoi( event->GetParameters() ) )
			{
			case 1:
				m_OnTrigger1.FireOutput( pActivator, this, 0 );
				break;
			case 2:
				m_OnTrigger2.FireOutput( pActivator, this, 0 );
				break;
			case 3:
				m_OnTrigger3.FireOutput( pActivator, this, 0 );
				break;
			case 4:
				m_OnTrigger4.FireOutput( pActivator, this, 0 );
				break;
			case 5:
				m_OnTrigger5.FireOutput( pActivator, this, 0 );
				break;
			case 6:
				m_OnTrigger6.FireOutput( pActivator, this, 0 );
				break;
			case 7:
				m_OnTrigger7.FireOutput( pActivator, this, 0 );
				break;
			case 8:
				m_OnTrigger8.FireOutput( pActivator, this, 0 );
				break;
			}
		}
		break;
	case CChoreoEvent::SEQUENCE:
		{
			if ( pActor )
			{
				DispatchStartSequence( scene, pActor, event );
			}
		}
		break;
	case CChoreoEvent::SECTION:
		{
			// Pauses scene playback
			DispatchPauseScene( scene, event->GetParameters() );
		}
		break;
	case CChoreoEvent::LOOP:
		{
			DispatchProcessLoop( scene, event );
		}
		break;
	case CChoreoEvent::INTERRUPT:
		{
			DispatchStartInterrupt( scene, event );
		}
		break;

	case CChoreoEvent::STOPPOINT:
		{
			DispatchStopPoint( scene, event->GetParameters() );
		}
		break;

	case CChoreoEvent::PERMIT_RESPONSES:
		{
			DispatchStartPermitResponses( scene, pActor, event );
		}
		break;
	default:
		{
			// FIXME: Unhandeled event
			// Assert(0);
		}
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : currenttime - 
//			*event - 
//-----------------------------------------------------------------------------
void CSceneEntity::EndEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event )
{
	Assert( event );

	if ( !Q_stricmp( event->GetName(), "NULL" ) )
 	{
 		return;
 	}

	CBaseFlex *pActor = NULL;
	CChoreoActor *actor = event->GetActor();
	if ( actor )
	{
		pActor = FindNamedActor( actor );
	}

	Scene_Printf( "%s : %8.2f:  finish %s\n", STRING( m_iszSceneFile ), currenttime, event->GetDescription() );

	switch ( event->GetType() )
	{
	case CChoreoEvent::EXPRESSION:
		{
			if ( pActor )
			{
				DispatchEndExpression( scene, pActor, event );
			}
		}
		break;
	case CChoreoEvent::SPEAK:
		{
			if ( pActor )
			{
				DispatchEndSpeak( scene, pActor, event );
			}
		}
		break;
	case CChoreoEvent::FLEXANIMATION:
		{
			if ( pActor )
			{
				DispatchEndFlexAnimation( scene, pActor, event );
			}
		}
		break;

	case CChoreoEvent::LOOKAT:
		{
			if ( pActor )
			{
				DispatchEndLookAt( scene, pActor, event );
			}
		}
		break;


	case CChoreoEvent::GESTURE:
		{
			if ( pActor )
			{
				DispatchEndGesture( scene, pActor, event );
			}
		}
		break;
	case CChoreoEvent::GENERIC:
		{
			// If the first token in the parameters is "debugtext", we printed it and we're done
			if ( event->GetParameters() && !Q_strncmp( event->GetParameters(), "debugtext", 9 ) )
				break;

			if ( pActor )
			{
				DispatchEndGeneric( scene, pActor, event );
			}
		}
		break;
	case CChoreoEvent::SEQUENCE:
		{
			if ( pActor )
			{
				DispatchEndSequence( scene, pActor, event );
			}
		}
		break;

	case CChoreoEvent::FACE:
		{
			if ( pActor )
			{
				DispatchEndFace( scene, pActor, event );
			}
		}
		break;

	case CChoreoEvent::MOVETO:
		{
			if ( pActor )
			{
				DispatchEndMoveTo( scene, pActor, event );
			}
		}
		break;

	case CChoreoEvent::SUBSCENE:
		{
			CChoreoScene *subscene = event->GetSubScene();
			if ( subscene )
			{
				subscene->ResetSimulation();
			}
		}
		break;
	case CChoreoEvent::INTERRUPT:
		{
			DispatchEndInterrupt( scene, event );
		}
		break;

	case CChoreoEvent::PERMIT_RESPONSES:
		{
			DispatchEndPermitResponses( scene, pActor, event );
		}
		break;
	default:
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Helper for parsing scene data file
//-----------------------------------------------------------------------------
class CSceneTokenProcessor : public ISceneTokenProcessor
{
public:
	const char	*CurrentToken( void );
	bool		GetToken( bool crossline );
	bool		TokenAvailable( void );
	void		Error( const char *fmt, ... );
	void		SetBuffer( char *buffer );
private:
	const char	*m_pBuffer;
	char		m_szToken[ 1024 ];
};

//-----------------------------------------------------------------------------
// Purpose: 
// Output : const char
//-----------------------------------------------------------------------------
const char *CSceneTokenProcessor::CurrentToken( void )
{
	return m_szToken;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : crossline - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CSceneTokenProcessor::GetToken( bool crossline )
{
	// NOTE: crossline is ignored here, may need to implement if needed
	m_pBuffer = engine->ParseFile( m_pBuffer, m_szToken, sizeof( m_szToken ) );
	if ( strlen( m_szToken ) >= 0 )
		return true;
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CSceneTokenProcessor::TokenAvailable( void )
{
	if ( m_pBuffer[ 0 ] == '\n' )
		return false;

	if (m_pBuffer[ 0 ] == ';' || m_pBuffer[ 0 ] == '#' ||		 // semicolon and # is comment field
		(m_pBuffer[ 0 ] == '/' && m_pBuffer[ 1 ] == '/'))		// also make // a comment field
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *fmt - 
//			... - 
//-----------------------------------------------------------------------------
void CSceneTokenProcessor::Error( const char *fmt, ... )
{
	char string[ 2048 ];
	va_list argptr;
	va_start( argptr, fmt );
	Q_vsnprintf( string, sizeof(string), fmt, argptr );
	va_end( argptr );

	Warning( "%s", string );
	Assert(0);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *buffer - 
//-----------------------------------------------------------------------------
void CSceneTokenProcessor::SetBuffer( char *buffer )
{
	m_pBuffer = buffer;
}

static CSceneTokenProcessor g_TokenProcessor;

//-----------------------------------------------------------------------------
// Purpose: Only spew one time per missing scene!!!
// Input  : *scenename - 
//-----------------------------------------------------------------------------
void MissingSceneWarning( char const *scenename )
{
	static CUtlSymbolTable missing;

	// Make sure we only show the message once
	if ( UTL_INVAL_SYMBOL == missing.Find( scenename ) )
	{
		missing.AddString( scenename );

		Warning( "Scene '%s' missing!\n", scenename );
	}
}

CChoreoScene *CSceneEntity::LoadScene( const char *filename )
{
	char loadfile[ 512 ];
	Q_strncpy( loadfile, filename, sizeof( loadfile ) );
	Q_SetExtension( loadfile, ".vcd", sizeof( loadfile ) );
	Q_FixSlashes( loadfile );

	// Load the file
	char *buffer = NULL;
	if ( !CopySceneFileIntoMemory( loadfile, (void **)&buffer ) )
	{
		MissingSceneWarning( loadfile );
		return NULL;
	}

	g_TokenProcessor.SetBuffer( buffer );
	CChoreoScene *scene = ChoreoLoadScene( loadfile, &m_SceneCallback, &g_TokenProcessor, Scene_Printf );

	FreeSceneFileMemory( buffer );
	return scene;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *filename - 
//-----------------------------------------------------------------------------
void CSceneEntity::LoadSceneFromFile( const char *filename )
{
	UnloadScene();

	m_pScene = LoadScene( filename );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSceneEntity::UnloadScene( void )
{
	if ( m_pScene )
	{
		ClearSceneEvents( m_pScene, false );

		for ( int i = 0 ; i < m_pScene->GetNumActors(); i++ )
		{
			CBaseFlex *pTestActor = FindNamedActor( i );

			if ( !pTestActor )
				continue;
		
			pTestActor->RemoveChoreoScene( m_pScene );
		}
	}
	delete m_pScene;
	m_pScene = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame that an event is active (Start/EndEvent as also
//  called)
// Input  : *event - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
void CSceneEntity::ProcessEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event )
{
	switch ( event->GetType() )
	{
	case CChoreoEvent::SUBSCENE:
		{
			Assert( event->GetType() == CChoreoEvent::SUBSCENE );

			CChoreoScene *subscene = event->GetSubScene();
			if ( !subscene )
				return;

			if ( subscene->SimulationFinished() )
				return;
			
			// Have subscenes think for appropriate time
			subscene->Think( m_flFrameTime );
		}
		break;

	default:
		break;
	}
	
	return;
}



//-----------------------------------------------------------------------------
// Purpose: Called for events that are part of a pause condition
// Input  : *event - 
// Output : Returns true on event completed, false on non-completion.
//-----------------------------------------------------------------------------
bool CSceneEntity::CheckEvent( float currenttime, CChoreoScene *scene, CChoreoEvent *event )
{
	switch ( event->GetType() )
	{
	case CChoreoEvent::SUBSCENE:
		{
		}
		break;
	default:
		{
			CBaseFlex *pActor = NULL;
			CChoreoActor *actor = event->GetActor();
			if ( actor )
			{
				pActor = FindNamedActor( actor );
				if (pActor == NULL)
				{
					Warning( "CSceneEntity %s unable to find actor \"%s\"\n", STRING(GetEntityName()), actor->GetName() );
					return true;
				}
			}
			if (pActor)
			{
				return pActor->CheckSceneEvent( currenttime, scene, event );
			}
		}
		break;
	}
	
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Get a sticky version of a named actor
// Input  : CChoreoActor 
// Output : CBaseFlex
//-----------------------------------------------------------------------------

CBaseFlex *CSceneEntity::FindNamedActor( int index )
{
	if (m_hActorList.Count() == 0)
	{
		m_hActorList.SetCount( m_pScene->GetNumActors() );
	}

	if ( !m_hActorList.IsValidIndex( index ) )
	{
		DevWarning( "Scene %s has %d actors, but scene entity only has %d actors\n", m_pScene->GetFilename(), m_pScene->GetNumActors(), m_hActorList.Size() );
		return NULL;
	}

	CBaseFlex *pActor = m_hActorList[ index ];

	if (pActor == NULL || !pActor->IsAlive() )
	{
		CChoreoActor *pChoreoActor = m_pScene->GetActor( index );
		if ( !pChoreoActor )
			return NULL;
		
		pActor = FindNamedActor( pChoreoActor->GetName() );

		if (pActor)
		{
			// save who we found so we'll use them again
			m_hActorList[ index ] = pActor;
		}
	}

	return pActor;
}



//-----------------------------------------------------------------------------
// Purpose: Get a sticky version of a named actor
// Input  : CChoreoActor 
// Output : CBaseFlex
//-----------------------------------------------------------------------------

CBaseFlex *CSceneEntity::FindNamedActor( CChoreoActor *pChoreoActor )
{
	int index = m_pScene->FindActorIndex( pChoreoActor );

	if (index >= 0)
	{
		return FindNamedActor( index );
	}
	return NULL;
}


//-----------------------------------------------------------------------------
// Purpose: Search for an actor by name, make sure it can do face poses
// Input  : *name - 
// Output : CBaseFlex
//-----------------------------------------------------------------------------
CBaseFlex *CSceneEntity::FindNamedActor( const char *name )
{
	CBaseEntity *entity = FindNamedEntity( name, NULL, true );

	if ( !entity )
	{
		// Couldn't find actor!
		return NULL;
	}

	// Make sure it can actually do facial animation, etc.
	CBaseFlex *flexEntity = dynamic_cast< CBaseFlex * >( entity );
	if ( !flexEntity )
	{
		// That actor was not a CBaseFlex!
		return NULL;
	}

	return flexEntity;
}

//-----------------------------------------------------------------------------
// Purpose: Find an entity specified by a target name
// Input  : *name - 
// Output : CBaseEntity
//-----------------------------------------------------------------------------
CBaseEntity *CSceneEntity::FindNamedTarget( string_t iszTarget, bool bBaseFlexOnly )
{
	if ( !stricmp( STRING(iszTarget), "!activator" ) )
		return m_hActivator;

	// If we don't have a wildcard in the target, just return the first entity found
	if ( !strchr( STRING(iszTarget), '*' ) )
		return gEntList.FindEntityByName( NULL, iszTarget, NULL );

	CBaseEntity *pTarget = NULL;
	while ( (pTarget = gEntList.FindEntityByName( pTarget, iszTarget, NULL )) != NULL )
	{
		if ( bBaseFlexOnly )
		{
			// Make sure it can actually do facial animation, etc.
			if ( dynamic_cast< CBaseFlex * >( pTarget ) )
				return pTarget;
		}
		else
		{
			return pTarget;
		}
	}

	// Failed to find one
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Search for an actor by name, make sure it can do face poses
// Input  : *name - 
// Output : CBaseFlex
//-----------------------------------------------------------------------------
CBaseEntity *CSceneEntity::FindNamedEntity( const char *name, CBaseEntity *pActor, bool bBaseFlexOnly )
{
	CBaseEntity *entity = NULL;

	if ( !stricmp( name, "Player" ) || !stricmp( name, "!player" ))
	{
		entity = ( gpGlobals->maxClients == 1 ) ? ( CBaseEntity * )UTIL_GetLocalPlayer() : NULL;
	}
	else if ( !stricmp( name, "!target1" ) )
	{
		if (m_hTarget1 == NULL)
		{
			m_hTarget1 = FindNamedTarget( m_iszTarget1, bBaseFlexOnly );
		}
		return m_hTarget1;
	}
	else if ( !stricmp( name, "!target2" ) )
	{
		if (m_hTarget2 == NULL)
		{
			m_hTarget2 = FindNamedTarget( m_iszTarget2, bBaseFlexOnly );
		}
		return m_hTarget2;
	}
	else if ( !stricmp( name, "!target3" ) )
	{
		if (m_hTarget3 == NULL)
		{
			m_hTarget3 = FindNamedTarget( m_iszTarget3, bBaseFlexOnly );
		}
		return m_hTarget3;
	}
	else if ( !stricmp( name, "!target4" ) )
	{
		if (m_hTarget4 == NULL)
		{
			m_hTarget4 = FindNamedTarget( m_iszTarget4, bBaseFlexOnly );
		}
		return m_hTarget4;
	}
	else if ( !stricmp( name, "!target5" ) )
	{
		if (m_hTarget5 == NULL)
		{
			m_hTarget5 = FindNamedTarget( m_iszTarget5, bBaseFlexOnly );
		}
		return m_hTarget5;
	}
	else if ( !stricmp( name, "!target6" ) )
	{
		if (m_hTarget6 == NULL)
		{
			m_hTarget6 = FindNamedTarget( m_iszTarget6, bBaseFlexOnly );
		}
		return m_hTarget6;
	}
	else if ( !stricmp( name, "!target7" ) )
	{
		if (m_hTarget7 == NULL)
		{
			m_hTarget7 = FindNamedTarget( m_iszTarget7, bBaseFlexOnly );
		}
		return m_hTarget7;
	}
	else if ( !stricmp( name, "!target8" ) )
	{
		if (m_hTarget8 == NULL)
		{
			m_hTarget8 = FindNamedTarget( m_iszTarget8, bBaseFlexOnly );
		}
		return m_hTarget8;
	}
	else if (pActor && pActor->MyNPCPointer())
	{
		entity = pActor->MyNPCPointer()->FindNamedEntity( name );
	}	
	else
	{
		entity = gEntList.FindEntityByName( NULL, name, pActor );
	}

	return entity;
}

//-----------------------------------------------------------------------------
// Purpose: Remove all "scene" expressions from all actors in this scene
//-----------------------------------------------------------------------------
void CSceneEntity::ClearSceneEvents( CChoreoScene *scene, bool canceled )
{
	if ( !m_pScene )
		return;

	int i;
	for ( i = 0 ; i < m_pScene->GetNumActors(); i++ )
	{
		CBaseFlex *pActor = FindNamedActor( i );
		if ( !pActor )
			continue;

		// Clear any existing expressions
		pActor->ClearSceneEvents( scene, canceled );
	}

	// Iterate events and precache necessary resources
	for ( i = 0; i < scene->GetNumEvents(); i++ )
	{
		CChoreoEvent *event = scene->GetEvent( i );
		if ( !event )
			continue;

		// load any necessary data
		switch (event->GetType() )
		{
		default:
			break;
		case CChoreoEvent::SUBSCENE:
			{
				// Only allow a single level of subscenes for now
				if ( !scene->IsSubScene() )
				{
					CChoreoScene *subscene = event->GetSubScene();
					if ( subscene )
					{
						ClearSceneEvents( subscene, canceled );
					}
				}
			}
			break;
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Remove all imposed schedules from all actors in this scene
//-----------------------------------------------------------------------------
void CSceneEntity::ClearSchedules( CChoreoScene *scene )
{
	if ( !m_pScene )
		return;

	int i;
	for ( i = 0 ; i < m_pScene->GetNumActors(); i++ )
	{
		CBaseFlex *pActor = FindNamedActor( i );
		if ( !pActor )
			continue;

		CAI_BaseNPC *pNPC = pActor->MyNPCPointer();

		if ( pNPC )
		{
			/*
			if ( pNPC->IsCurSchedule( SCHED_SCENE_GENERIC ) )
				pNPC->ClearSchedule();
			*/
		}
		else
		{
			pActor->ResetSequence( pActor->SelectWeightedSequence( ACT_IDLE ) );
			pActor->SetCycle( 0 );
		}
		// Clear any existing expressions
	}

	// Iterate events and precache necessary resources
	for ( i = 0; i < scene->GetNumEvents(); i++ )
	{
		CChoreoEvent *event = scene->GetEvent( i );
		if ( !event )
			continue;

		// load any necessary data
		switch (event->GetType() )
		{
		default:
			break;
		case CChoreoEvent::SUBSCENE:
			{
				// Only allow a single level of subscenes for now
				if ( !scene->IsSubScene() )
				{
					CChoreoScene *subscene = event->GetSubScene();
					if ( subscene )
					{
						ClearSchedules( subscene );
					}
				}
			}
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: If we are currently interruptable, pause this scene and wait for the other
//  scene to finish
// Input  : *otherScene - 
//-----------------------------------------------------------------------------
bool CSceneEntity::InterruptThisScene( CSceneEntity *otherScene )
{
	Assert( otherScene );

	if ( !IsInterruptable() )
	{
		return false;
	}

	// Already interrupted
	if ( m_bInterrupted )
	{
		return false;
	}

	m_bInterrupted		= true;
	m_hInterruptScene	= otherScene;

	// Ask other scene to tell us when it's finished or canceled
	otherScene->RequestCompletionNotification( this );

	PausePlayback();
	return true;
}

/*
void scene_interrupt()
{
	if ( engine->Cmd_Argc() != 3 )
		return;

	const char *scene1 = engine->Cmd_Argv(1);
	const char *scene2 = engine->Cmd_Argv(2);

	CSceneEntity *s1 = dynamic_cast< CSceneEntity * >( gEntList.FindEntityByName( NULL, scene1, NULL ) );
	CSceneEntity *s2 = dynamic_cast< CSceneEntity * >( gEntList.FindEntityByName( NULL, scene2, NULL ) );

	if ( !s1 || !s2 )
		return;

	if ( s1->InterruptThisScene( s2 ) )
	{
		s2->StartPlayback();
	}
}

static ConCommand interruptscene( "int", scene_interrupt, "interrupt scene 1 with scene 2.", FCVAR_CHEAT );
*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSceneEntity::CheckInterruptCompletion()
{
	if ( !m_bInterrupted )
		return;

	// If the interruptor goes away it's the same as having that scene finish up...
	if ( m_hInterruptScene != NULL && 
		!m_bInterruptSceneFinished )
	{
		return;
	}

	m_bInterrupted = false;
	m_hInterruptScene = NULL;

	ResumePlayback();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSceneEntity::ClearInterrupt()
{
	m_nInterruptCount = 0;
	m_bInterrupted = false;
	m_hInterruptScene = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Another scene is asking us to notify upon completion
// Input  : *notify - 
//-----------------------------------------------------------------------------
void CSceneEntity::RequestCompletionNotification( CSceneEntity *notify )
{
	CHandle< CSceneEntity > h;
	h = notify;
	// Only add it once
	if ( m_hNotifySceneCompletion.Find( h ) == m_hNotifySceneCompletion.InvalidIndex() )
	{
		m_hNotifySceneCompletion.AddToTail( h );
	}
}

//-----------------------------------------------------------------------------
// Purpose: An interrupt scene has finished or been canceled, we can resume once we pick up this state in CheckInterruptCompletion
// Input  : *interruptor - 
//-----------------------------------------------------------------------------
void CSceneEntity::NotifyOfCompletion( CSceneEntity *interruptor )
{
	Assert( m_bInterrupted );
	Assert( m_hInterruptScene == interruptor );
	m_bInterruptSceneFinished = true;

	CheckInterruptCompletion();
}

//-----------------------------------------------------------------------------
// Purpose: Clear any targets that a referencing !activator
//-----------------------------------------------------------------------------
void CSceneEntity::ClearActivatorTargets( void )
{
	if ( !stricmp( STRING(m_iszTarget1), "!activator" ) )
	{
		// We need to clear out actors so they're re-evaluated
		m_hActorList.Purge();
		m_hTarget1 = NULL;
	}
	if ( !stricmp( STRING(m_iszTarget2), "!activator" ) )
	{
		// We need to clear out actors so they're re-evaluated
		m_hActorList.Purge();
		m_hTarget2 = NULL;
	}
	if ( !stricmp( STRING(m_iszTarget3), "!activator" ) )
	{
		// We need to clear out actors so they're re-evaluated
		m_hActorList.Purge();
		m_hTarget3 = NULL;
	}
	if ( !stricmp( STRING(m_iszTarget4), "!activator" ) )
	{
		// We need to clear out actors so they're re-evaluated
		m_hActorList.Purge();
		m_hTarget4 = NULL;
	}
	if ( !stricmp( STRING(m_iszTarget5), "!activator" ) )
	{
		// We need to clear out actors so they're re-evaluated
		m_hActorList.Purge();
		m_hTarget5 = NULL;
	}
	if ( !stricmp( STRING(m_iszTarget6), "!activator" ) )
	{
		// We need to clear out actors so they're re-evaluated
		m_hActorList.Purge();
		m_hTarget6 = NULL;
	}
	if ( !stricmp( STRING(m_iszTarget7), "!activator" ) )
	{
		// We need to clear out actors so they're re-evaluated
		m_hActorList.Purge();
		m_hTarget7 = NULL;
	}
	if ( !stricmp( STRING(m_iszTarget8), "!activator" ) )
	{
		// We need to clear out actors so they're re-evaluated
		m_hActorList.Purge();
		m_hTarget8 = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called when a scene is completed or canceled
//-----------------------------------------------------------------------------
void CSceneEntity::OnSceneFinished( bool canceled, bool fireoutput )
{
	if ( !m_pScene )
		return;

	// Notify any listeners
	int c = m_hNotifySceneCompletion.Count();
	int i;
	for ( i = 0; i < c; i++ )
	{
		CSceneEntity *ent = m_hNotifySceneCompletion[ i ].Get();
		if ( !ent )
			continue;

		ent->NotifyOfCompletion( this );
	}
	m_hNotifySceneCompletion.RemoveAll();

	// Clear simulation
	m_pScene->ResetSimulation();
	m_bIsPlayingBack = false;
	m_bPaused = false;
	m_flCurrentTime = 0.0f;
	
	// Clear interrupt state if we were interrupted for some reason
	ClearInterrupt();

	if ( fireoutput && !m_bCompletedEarly)
	{
		m_OnCompletion.FireOutput( this, this, 0 );
	}

	// Put face back in neutral pose
	ClearSceneEvents( m_pScene, canceled );

	for ( i = 0 ; i < m_pScene->GetNumActors(); i++ )
	{
		CBaseFlex *pTestActor = FindNamedActor( i );

		if ( !pTestActor )
			continue;
	
		pTestActor->RemoveChoreoScene( m_pScene );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  :
// Output :
//-----------------------------------------------------------------------------
class CInstancedSceneEntity : public CSceneEntity
{
	DECLARE_DATADESC();
	DECLARE_CLASS( CInstancedSceneEntity, CSceneEntity ); 
public:
	EHANDLE					m_hOwner;
	bool					m_bHadOwner;
	float					m_flPostSpeakDelay;
	char					m_szInstanceFilename[ CChoreoScene::MAX_SCENE_FILENAME ];
	bool					m_bIsBackground;

	virtual void			DoThink( float frametime );
	virtual CBaseFlex		*FindNamedActor( const char *name );
	virtual CBaseEntity		*FindNamedEntity( const char *name );
	virtual float			GetPostSpeakDelay()	{ return m_flPostSpeakDelay; }
	virtual void			SetPostSpeakDelay( float flDelay ) { m_flPostSpeakDelay = flDelay; }

	virtual void			DispatchStartMoveTo( CChoreoScene *scene, CBaseFlex *actor, CBaseEntity *actor2, CChoreoEvent *event )
							{ 
								if (PassThrough( actor )) BaseClass::DispatchStartMoveTo( scene, actor, actor2, event ); 
							};

	virtual void			DispatchEndMoveTo( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event ) 
							{ 
								if (PassThrough( actor )) BaseClass::DispatchEndMoveTo( scene, actor, event ); 
							};

	virtual void			DispatchStartFace( CChoreoScene *scene, CBaseFlex *actor, CBaseEntity *actor2, CChoreoEvent *event ) 
							{ 
								if (PassThrough( actor )) BaseClass::DispatchStartFace( scene, actor, actor2, event ); 
							};

	virtual void			DispatchEndFace( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event ) 
							{ 
								if (PassThrough( actor )) BaseClass::DispatchEndFace( scene, actor, event ); 
							};

	virtual void			DispatchStartSequence( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event )  { /* suppress */ };
	virtual void			DispatchEndSequence( CChoreoScene *scene, CBaseFlex *actor, CChoreoEvent *event ) { /* suppress */ };
	virtual void			DispatchPauseScene( CChoreoScene *scene, const char *parameters ) { /* suppress */ };

	void OnRestore();
private:
	bool					PassThrough( CBaseFlex *actor );
};

LINK_ENTITY_TO_CLASS( instanced_scripted_scene, CInstancedSceneEntity );


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CInstancedSceneEntity )

	DEFINE_FIELD( m_hOwner,				FIELD_EHANDLE ),
	DEFINE_FIELD( m_bHadOwner,			FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flPostSpeakDelay,	FIELD_FLOAT ),
	DEFINE_AUTO_ARRAY( m_szInstanceFilename, FIELD_CHARACTER ),
	DEFINE_FIELD( m_bIsBackground,		FIELD_BOOLEAN ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: create a one-shot scene, no movement, sequences, etc.
// Input  :
// Output :
//-----------------------------------------------------------------------------

float InstancedScriptedScene( CBaseFlex *pActor, const char *pszScene, EHANDLE *phSceneEnt, float flPostDelay, bool bIsBackground )
{
	CInstancedSceneEntity *pScene = (CInstancedSceneEntity *)CBaseEntity::CreateNoSpawn( "instanced_scripted_scene", vec3_origin, vec3_angle );

	// This code expands any $gender tags into male or female tags based on the gender of the actor (based on his/her .mdl)
	if ( pActor )
	{
		pActor->GenderExpandString( pszScene, pScene->m_szInstanceFilename, sizeof( pScene->m_szInstanceFilename ) );
	}
	else
	{
		Q_strncpy( pScene->m_szInstanceFilename, pszScene, sizeof( pScene->m_szInstanceFilename ) );
	}
	pScene->m_iszSceneFile = MAKE_STRING( pScene->m_szInstanceFilename );

	// FIXME: I should set my output to fire something that kills me....

	// FIXME: add a proper initialization function
	pScene->m_hOwner = pActor;
	pScene->m_bHadOwner = pActor != NULL;
	pScene->SetPostSpeakDelay( flPostDelay );
	DispatchSpawn( pScene );
	pScene->Activate();
	pScene->m_bIsBackground = bIsBackground;
	pScene->SetBackground( bIsBackground );
	pScene->StartPlayback();

	if ( !pScene->ValidScene() )
	{
		Warning( "Unknown scene specified: \"%s\"\n", pScene->m_szInstanceFilename );
		if ( phSceneEnt )
		{
			*phSceneEnt = NULL;
		}
		return 0;
	}

	if ( phSceneEnt )
	{
		*phSceneEnt = pScene;
	}

	return pScene->EstimateLength();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pActor - 
//			*soundnmame - 
//			*phSceneEnt - 
// Output : float
//-----------------------------------------------------------------------------
float InstancedAutoGeneratedSoundScene( CBaseFlex *pActor, char const *soundname, EHANDLE *phSceneEnt /*= NULL*/ )
{
	if ( !pActor )
	{
		Warning( "InstancedAutoGeneratedSoundScene:  Expecting non-NULL pActor for sound %s\n", soundname );
		return 0;
	}

	CInstancedSceneEntity *pScene = (CInstancedSceneEntity *)CBaseEntity::CreateNoSpawn( "instanced_scripted_scene", vec3_origin, vec3_angle );

	Q_strncpy( pScene->m_szInstanceFilename, UTIL_VarArgs( "AutoGenerated(%s)", soundname ), sizeof( pScene->m_szInstanceFilename ) );
	pScene->m_iszSceneFile = MAKE_STRING( pScene->m_szInstanceFilename );

	pScene->m_hOwner = pActor;
	pScene->m_bHadOwner = pActor != NULL;

	pScene->GenerateSoundScene( pActor, soundname );

	pScene->Spawn();
	pScene->Activate();
	pScene->StartPlayback();

	if ( !pScene->ValidScene() )
	{
		Msg( "Couldn't instance scene for sound \"%s\"\n", soundname );
		if ( phSceneEnt )
		{
			*phSceneEnt = NULL;
		}
		return 0;
	}

	if ( phSceneEnt )
	{
		*phSceneEnt = pScene;
	}

	return pScene->EstimateLength();
}

//-----------------------------------------------------------------------------

void StopScriptedScene( CBaseFlex *pActor, EHANDLE hSceneEnt )
{
	CBaseEntity *pEntity = hSceneEnt;
	CSceneEntity *pScene = dynamic_cast<CSceneEntity *>(pEntity);

	if ( pScene )
		pScene->CancelPlayback();
}

static void PrecacheSceneEvent( CChoreoEvent *event, CUtlVector< int >& soundlist )
{
	if ( !event )
	{
		return;
	}

	switch ( event->GetType() )
	{
	case CChoreoEvent::SPEAK:
		{
			/*
			int idx = soundemitterbase->GetSoundIndex( event->GetParameters() );
			if ( idx != -1 )
			{
				soundlist.AddToTail( idx );
			}
			*/

			if ( event->GetCloseCaptionType() == CChoreoEvent::CC_MASTER /*&& 
				 event->GetNumSlaves() > 0*/ )
			{
				char tok[ CChoreoEvent::MAX_CCTOKEN_STRING ];
				if ( event->GetPlaybackCloseCaptionToken( tok, sizeof( tok ) ) )
				{
					int idx = soundemitterbase->GetSoundIndex( tok );
					if ( idx != -1 )
					{
						soundlist.AddToTail( idx );
					}
				}
			}
		}
		break;
	default:
		break;
	}
}

class CSceneCache : public IBaseCacheInfo
{
public:
	float		duration;
	CUtlVector< int > soundnames;

	CSceneCache():
		duration( 0.0f ),
		soundnames()
	{
	}

	CSceneCache( const CSceneCache& src )
	{
		duration = src.duration;
		int c = src.soundnames.Count();
		for ( int i = 0; i < c; ++i )
		{
			soundnames.AddToTail( src.soundnames[ i ] );
		}
	}


	int	GetSoundCount() const
	{
		return soundnames.Count();
	}

	char const *GetSoundName( int index )
	{
		return soundemitterbase->GetSoundName( soundnames[ index ] );
	}

	virtual void Save( CUtlBuffer& buf  )
	{
		buf.PutFloat( duration );
	
		unsigned short c = GetSoundCount();
		buf.PutShort( c );
		
		Assert( soundnames.Count() <= 65536 );

		for ( int i = 0; i < c; ++i )
		{
			buf.PutString( GetSoundName( i ) );
		}
	}

	virtual void Restore( CUtlBuffer& buf  )
	{
		soundnames.RemoveAll();

		duration = buf.GetFloat();

		unsigned short c;

		c = (unsigned short)buf.GetShort();

		for ( int i = 0; i < c; ++i )
		{
			char soundname[ 512 ];

			buf.GetString( soundname, sizeof( soundname ) );

			int idx = soundemitterbase->GetSoundIndex( soundname );
			if ( idx != -1 )
			{
				soundnames.AddToTail( idx );
			}
		}
	}

	virtual void Rebuild( char const *filename )
	{
		soundnames.RemoveAll();
		duration = 0.0f;

		// Load the file
		char *buffer = NULL;
		if ( CopySceneFileIntoMemory( filename, (void **)&buffer ) )
		{
			g_TokenProcessor.SetBuffer( buffer );
			CChoreoScene *scene = ChoreoLoadScene( filename, NULL, &g_TokenProcessor, Scene_Printf );
			if ( scene )
			{
				// Walk all events looking for SPEAK events
				CChoreoEvent *event;
				int c = scene->GetNumEvents();
				for ( int i = 0; i < c; ++i )
				{
					event = scene->GetEvent( i );

					PrecacheSceneEvent( event, soundnames );
				}

				// Update scene duration, too
				duration = scene->FindStopTime();

				delete scene;
			}

			FreeSceneFileMemory( buffer );
		}
		else
		{
			MissingSceneWarning( filename );
		}
	}

	static unsigned int		ComputeSoundScriptFileTimestampChecksum()
	{
		return soundemitterbase->GetManifestFileTimeChecksum();
	}
};

#define SCENECACHE_VERSION		3
static CUtlCachedFileData< CSceneCache >	g_SceneCache( "scene.cache", SCENECACHE_VERSION, CSceneCache::ComputeSoundScriptFileTimestampChecksum );

void ResetPrecacheInstancedSceneDictionary()
{
	// Reload the cache
	g_SceneCache.Reload();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool SceneCacheInit()
{
	return g_SceneCache.Init();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void SceneCacheShutdown()
{
	g_SceneCache.Shutdown();
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pszScene - 
// Output : float
//-----------------------------------------------------------------------------
float GetSceneDuration( char const *pszScene )
{
	CSceneCache* entry = g_SceneCache.Get( pszScene );
	if ( !entry )
		return 0;
	return entry->duration;
}

//-----------------------------------------------------------------------------
// Purpose: The only resource manager parameter we currently care about is the name 
//  of the .vcd to cache into memory
//-----------------------------------------------------------------------------
struct scenedataparams_t
{
	char const *filename;
};

// 2.0 Mb cache (d1_trainstation_05 takes 1.77 Mb raw)
#define MAX_SCENE_MEMORY_CACHE (int)( 1024.0f * 1024.0f * 2.0f )

//-----------------------------------------------------------------------------
// Purpose: The vcd cache is just the raw .vcd file copied into RAM.  When the .vcd gets referenced
//  by the SceneEntity, we essentially copy the raw .vcd memory over to the passed in pointer and then
//  that memory is passed into the token parser when creating the CChoreoScene from memory
// Most of our levels use about 1 Mb of .vcds (or they cause that many to be precached) so I conservatively
//  set the cache to 1.5 Mb for now
//-----------------------------------------------------------------------------
struct SceneData_t
{
	SceneData_t() :
		m_nFileSize( 0 ),
		m_pvData( 0 )
	{
	}

	// APIS required by CResourceManager
	void DestroyResource()
	{
		free( m_pvData );
		delete this;
	}
	SceneData_t		*GetData()
	{ 
		return this; 
	}
	unsigned int	Size()
	{ 
		return m_nFileSize; 
	}

	// you must implement these static functions for the ResourceManager
	// -----------------------------------------------------------
	static SceneData_t *CreateResource( const scenedataparams_t &params )
	{
		SceneData_t *data = new SceneData_t;

		FileHandle_t fh = filesystem->Open( params.filename, "rb" );
		if ( FILESYSTEM_INVALID_HANDLE != fh )
		{
			int filesize = filesystem->Size( fh );
			if ( filesize > 0 )
			{
				char *pvData = (char *)malloc( filesize + 1 );
				if ( !pvData )
				{
					Error( "Unable to allocate %i bytes for SceneData_t for '%s'", filesize + 1, params.filename );
				}

				filesystem->Read( pvData, filesize, fh );
					
				pvData[ filesize ] = 0;

				data->m_nFileSize = filesize;
				data->m_pvData = (void *)pvData;
			}

			filesystem->Close( fh );
		}

		return data;
	}

	static unsigned int EstimatedSize( const scenedataparams_t &params )
	{
		// The average size of all .vcd files on the HD is about 4.3K so use this for the estimate
		int size = 5000; 
		return ( sizeof( SceneData_t ) + size );
	}

	int			m_nFileSize;
	void		*m_pvData;
};

//-----------------------------------------------------------------------------
// Purpose: This manages the instanced scene memory handles.  We essentially grow a handle list by scene filename where
//  the handle is a pointer to a SceneData_t defined above.  If the resource manager uncaches the handle, we reload the
//  .vcd from disk.  Precaching a .vcd calls into FindOrAddScene which moves the .vcd to the head of the LRU if it's in memory
//  or it reloads it from disk otherwise.
//-----------------------------------------------------------------------------
class CInstancedSceneResourceManager : public CAutoGameSystem
{
public:
	CInstancedSceneResourceManager() : 
	  m_SceneMemoryCache( MAX_SCENE_MEMORY_CACHE )
	{
	}

	virtual bool Init()
	{
		return true;
	}
	virtual void Shutdown()
	{
		Clear();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Spew a cache summary to the console
	//-----------------------------------------------------------------------------
	void SpewMemoryUsage()
	{
		int bytesUsed = m_SceneMemoryCache.UsedSize();
		int bytesTotal = m_SceneMemoryCache.TargetSize();

		float percent = 100.0f * (float)bytesUsed / (float)bytesTotal;

		DevMsg( "CInstancedSceneResourceManager:  %i .vcds total %s, %.2f %% of capacity\n", m_PrecachedScenes.Count(), Q_pretifymem( bytesUsed, 2 ), percent );
	}

	virtual void LevelInitPostEntity()
	{
		SpewMemoryUsage();
	}
	
	//-----------------------------------------------------------------------------
	// Purpose: Touch the cache or load the scene into the cache for the first time
	// Input  : *filename - 
	//-----------------------------------------------------------------------------
	void FindOrAddScene( char const *filename )
	{
		char fn[ 256 ];
		Q_strncpy( fn, filename, sizeof( fn ) );
		Q_FixSlashes( fn );
		Q_strlower( fn );

		int idx = m_PrecachedScenes.Find( fn );
		if ( idx != m_PrecachedScenes.InvalidIndex() )
		{
			// Move it to head of LRU
			m_SceneMemoryCache.TouchResource( m_PrecachedScenes[ idx ] );
			return;
		}

		scenedataparams_t params;
		params.filename = fn;
		memhandle_t handle = m_SceneMemoryCache.CreateResource( params );

		// Add scene filename to dictionary
		m_PrecachedScenes.Insert( fn, handle );
	}

	//-----------------------------------------------------------------------------
	// Purpose: Looks up the scene in the dictionary by filename, ensures the physical
	//  data is in the resource cache, and allocates *buffer and copies the data into the
	//  passed in buffer for the caller..  The caller MUST call FreeSceneFileMemory or
	//  else the memory will leak!!!
	// Input  : *filename - 
	//			**buffer - 
	// Output : 	bool - loading failed (unknown scene or some other problem)
	//-----------------------------------------------------------------------------
	bool CopySceneFileIntoMemory( char const *filename, void **buffer )
	{
		bool bret = false;

		*buffer = NULL;

		char fn[ 256 ];
		Q_strncpy( fn, filename, sizeof( fn ) );
		Q_FixSlashes( fn );
		Q_strlower( fn );

		// Add to caching system
		FindOrAddScene( fn );

		// Now look it up, it should be in the system
		int idx = m_PrecachedScenes.Find( fn );
		if ( idx == m_PrecachedScenes.InvalidIndex() )
		{
			return bret;
		}

		// Now see if the handle has been paged out...
		memhandle_t handle = m_PrecachedScenes[ idx ];
		
		SceneData_t *data = m_SceneMemoryCache.LockResource( handle );
		if ( !data )
		{
			// Try and reload it
			scenedataparams_t params;
			params.filename = fn;
			handle = m_PrecachedScenes[ idx ] = m_SceneMemoryCache.CreateResource( params );
			data = m_SceneMemoryCache.LockResource( handle );
			if ( !data )
			{
				return bret;
			}
		}


		// Cache entry exists, but if filesize == 0 then the file itself wasn't on disk...
		if ( data->m_nFileSize != 0 )
		{
			// Success
			bret = true;

			// Allocate dest buffer
			byte *dest = new byte[ data->m_nFileSize + 1 ];
			// Copy in data
			Q_memcpy( dest, data->m_pvData, data->m_nFileSize );
			// Terminate with 0
			dest[ data->m_nFileSize ] = 0;
			// Set caller's buffer pointer
			*buffer = dest;
		}

		// Release lock
		m_SceneMemoryCache.UnlockResource( handle );
		return bret;
	}

	// Delete instance scene memory
	void FreeSceneFileMemory( void *buffer )
	{
		delete[] buffer;
	}

	void Flush()
	{
		m_SceneMemoryCache.FlushAllUnlocked();

		// Refresh cache entries, too
		g_SceneCache.ForceRecheckDiskInfo();
	}

private:

	void Clear()
	{
		int c = m_PrecachedScenes.Count();
		for ( int i = 0; i  < c; ++i )
		{
			memhandle_t dat = m_PrecachedScenes[ i ];
			m_SceneMemoryCache.DestroyResource( dat );
		}

		m_PrecachedScenes.RemoveAll();
	}

	CUtlDict< memhandle_t, int >	m_PrecachedScenes;
	CResourceManager< SceneData_t, scenedataparams_t >	m_SceneMemoryCache;
};

CInstancedSceneResourceManager g_InstancedSceneResourceManager;

CON_COMMAND( scene_flush, "Flush all .vcds from the cache and reload from disk." )
{
	g_InstancedSceneResourceManager.Flush();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *filename - 
//			**buffer - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CopySceneFileIntoMemory( char const *filename, void **buffer )
{
	return g_InstancedSceneResourceManager.CopySceneFileIntoMemory( filename, buffer );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *buffer - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
void FreeSceneFileMemory( void *buffer )
{
	g_InstancedSceneResourceManager.FreeSceneFileMemory( buffer );
}

//-----------------------------------------------------------------------------
// Purpose: Used for precaching instanced scenes
// Input  : *pszScene - 
//-----------------------------------------------------------------------------
void PrecacheInstancedScene( char const *pszScene )
{
	//	Msg( "Precache %s\n", pszScene );
	if ( !g_SceneCache.EntryExists( pszScene ) )
	{
		if ( !CBaseEntity::IsPrecacheAllowed() )
		{
			Assert( !"PrecacheInstancedScene:  too late" );
			Warning( "Late precache of %s\n", pszScene );
		}
	}

	// The g_SceneCache doesn't require loading the .vcds, it just reads the soundnames
	//  out of the vcd
	CSceneCache *entry = g_SceneCache.Get( pszScene );
	Assert( entry );

	// Add to resource cache, too
	g_InstancedSceneResourceManager.FindOrAddScene( pszScene );

	int c = entry->GetSoundCount();
	for ( int i = 0; i < c; ++i )
	{
		CBaseEntity::PrecacheScriptSound( entry->GetSoundName( i ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CInstancedSceneEntity::DoThink( float frametime )
{
	CheckInterruptCompletion();

	if ( !m_pScene || !m_bIsPlayingBack || ( m_bHadOwner && m_hOwner == NULL ) )
	{
		UTIL_Remove( this );
		return;
	}

	if ( m_bPaused )
	{
		PauseThink();
		return;
	}

	float dt = frametime;

	m_pScene->SetSoundFileStartupLatency( GetSoundSystemLatency() );

	// Tell scene to go
	m_pScene->Think( m_flCurrentTime );

	// Drive simulation time for scene
	m_flCurrentTime += dt;

	// Did we get to the end
	if ( m_pScene->SimulationFinished() )
	{
		OnSceneFinished( false, false );

		UTIL_Remove( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Search for an actor by name, make sure it can do face poses
// Input  : *name - 
// Output : CBaseFlex
//-----------------------------------------------------------------------------
CBaseFlex *CInstancedSceneEntity::FindNamedActor( const char *name )
{
	if (m_hOwner != NULL)
	{
		CAI_BaseNPC	*npc = m_hOwner->MyNPCPointer();
		if (npc)
		{
			return npc;
		}
	}
	return BaseClass::FindNamedActor( name );
}


//-----------------------------------------------------------------------------
// Purpose: Search for an actor by name, make sure it can do face poses
// Input  : *name - 
// Output : CBaseFlex
//-----------------------------------------------------------------------------
CBaseEntity *CInstancedSceneEntity::FindNamedEntity( const char *name )
{
	CBaseEntity *pOther = NULL;

	if (m_hOwner != NULL)
	{
		CAI_BaseNPC	*npc = m_hOwner->MyNPCPointer();

		if (npc)
		{
			pOther = npc->FindNamedEntity( name );
		}
	}

	if (!pOther)
	{
		pOther = BaseClass::FindNamedEntity( name );
	}
	return pOther;
}


//-----------------------------------------------------------------------------
// Purpose: Suppress certain events when it's instanced since they're can cause odd problems
// Input  : actor
// Output : true - the event should happen, false - it shouldn't
//-----------------------------------------------------------------------------

bool CInstancedSceneEntity::PassThrough( CBaseFlex *actor )
{
	if (!actor)
		return false;

	CAI_BaseNPC *myNpc = actor->MyNPCPointer( );

	if (!myNpc)
		return false;

	if (myNpc->IsCurSchedule( SCHED_SCENE_GENERIC ))
	{
		return true;
	}

	if (myNpc->GetCurSchedule())
	{
		CAI_ScheduleBits testBits;
		myNpc->GetCurSchedule()->GetInterruptMask( &testBits );

		if (testBits.GetBit( COND_IDLE_INTERRUPT )) 
		{
			return true;
		}
	}

	Scene_Printf( "%s : event suppressed\n", STRING( m_iszSceneFile ) );

	return false;
}


//-----------------------------------------------------------------------------
void CInstancedSceneEntity::OnRestore()
{
	if ( m_bHadOwner && !m_hOwner )
	{
		// probably just came back from a level transition
		UTIL_Remove( this );
		return;
	}
	// reset background state
	if ( m_pScene )
	{
		m_pScene->SetBackground( m_bIsBackground );
	}
	BaseClass::OnRestore();
}




LINK_ENTITY_TO_CLASS( scene_manager, CSceneManager );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSceneManager::Think()
{
	// The manager is always thinking at 20 hz
	SetNextThink( gpGlobals->curtime + SCENE_THINK_INTERVAL );
	float frameTime = ( gpGlobals->curtime - GetLastThink() );
	frameTime = min( 0.1, frameTime );

	// stop if AI is diabled
	if (CAI_BaseNPC::m_nDebugBits & bits_debugDisableAI)
		return;

	bool needCleanupPass = false;
	int c = m_ActiveScenes.Count();
	for ( int i = 0; i < c; i++ )
	{
		CSceneEntity *scene = m_ActiveScenes[ i ].Get();
		if ( !scene )
		{
			needCleanupPass = true;
			continue;
		}

		scene->DoThink( frameTime );

		if ( m_ActiveScenes.Count() < c )
		{
			// Scene removed self while thinking. Adjust iteration.
			c = m_ActiveScenes.Count();
			i--;
		}
	}

	// Now delete any invalid ones
	if ( needCleanupPass )
	{
		for ( int i = c - 1; i >= 0; i-- )
		{
			CSceneEntity *scene = m_ActiveScenes[ i ].Get();
			if ( scene )
				continue;

			m_ActiveScenes.Remove( i );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSceneManager::ClearAllScenes()
{
	m_ActiveScenes.RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *scene - 
//-----------------------------------------------------------------------------
void CSceneManager::AddSceneEntity( CSceneEntity *scene )
{
	CHandle< CSceneEntity > h;
	
	h = scene;

	// Already added/activated
	if ( m_ActiveScenes.Find( h ) != m_ActiveScenes.InvalidIndex() )
	{
		return;
	}

	m_ActiveScenes.AddToTail( h );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *scene - 
//-----------------------------------------------------------------------------
void CSceneManager::RemoveSceneEntity( CSceneEntity *scene )
{
	CHandle< CSceneEntity > h;
	
	h = scene;

	m_ActiveScenes.FindAndRemove( h );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *player - 
//-----------------------------------------------------------------------------
void CSceneManager::OnClientActive( CBasePlayer *player )
{
	int c = m_QueuedSceneSounds.Count();
	for ( int i = 0; i < c; i++ )
	{
		CRestoreSceneSound *sound = &m_QueuedSceneSounds[ i ];

		if ( sound->actor == NULL )
			continue;

		// Blow off sounds more than 10 seconds in past
		if ( sound->time_in_past > 10.0f )
			continue;

		CPASAttenuationFilter filter( sound->actor );
		
		EmitSound_t es;
		es.m_nChannel = CHAN_VOICE;
		es.m_flVolume = 1;
		es.m_pSoundName = sound->soundname;
		es.m_SoundLevel = sound->soundlevel;
		es.m_flSoundTime = gpGlobals->curtime - sound->time_in_past;

		EmitSound( filter, sound->actor->entindex(), es );
	}

	m_QueuedSceneSounds.RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose: Stops scenes involving the specified actor
//-----------------------------------------------------------------------------
void CSceneManager::RemoveActorFromScenes( CBaseFlex *pActor, bool bInstancedOnly  )
{
	int c = m_ActiveScenes.Count();
	for ( int i = 0; i < c; i++ )
	{
		CSceneEntity *pScene = m_ActiveScenes[ i ].Get();
		if ( !pScene )
		{
			continue;
		}
		
		// If only stopping instanced scenes, then skip it if it can't cast to an instanced scene
		if ( bInstancedOnly && 
			( dynamic_cast< CInstancedSceneEntity * >( pScene ) == NULL ) )
		{
			continue;
		}

		if ( pScene->InvolvesActor( pActor ) )
		{
			pScene->CancelPlayback();
		}

	}
	
}

//-----------------------------------------------------------------------------
// Purpose: returns if there are scenes involving the specified actor
//-----------------------------------------------------------------------------
bool CSceneManager::IsRunningScriptedScene( CBaseFlex *pActor, bool bIgnoreInstancedScenes )
{
	int c = m_ActiveScenes.Count();
	for ( int i = 0; i < c; i++ )
	{
		CSceneEntity *pScene = m_ActiveScenes[ i ].Get();
		if ( !pScene || !pScene->IsPlayingBack() || ( bIgnoreInstancedScenes && dynamic_cast<CInstancedSceneEntity *>(pScene) != NULL ) )
		{
			continue;
		}
		
		if ( pScene->InvolvesActor( pActor ) )
		{
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pActor - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CSceneManager::IsRunningScriptedSceneWithSpeech( CBaseFlex *pActor, bool bIgnoreInstancedScenes )
{
	int c = m_ActiveScenes.Count();
	for ( int i = 0; i < c; i++ )
	{
		CSceneEntity *pScene = m_ActiveScenes[ i ].Get();
		if ( !pScene || !pScene->IsPlayingBack() || ( bIgnoreInstancedScenes && dynamic_cast<CInstancedSceneEntity *>(pScene) != NULL ) )
		{
			continue;
		}
		
		if ( pScene->InvolvesActor( pActor ) )
		{
			if ( pScene->HasUnplayedSpeech() )
				return true;
		}
	}
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *actor - 
//			*soundname - 
//			soundlevel - 
//			soundtime - 
//-----------------------------------------------------------------------------
void CSceneManager::QueueRestoredSound( CBaseFlex *actor, char const *soundname, soundlevel_t soundlevel, float time_in_past )
{
	CRestoreSceneSound e;
	e.actor = actor;
	Q_strncpy( e.soundname, soundname, sizeof( e.soundname ) );
	e.soundlevel = soundlevel;
	e.time_in_past = time_in_past;

	m_QueuedSceneSounds.AddToTail( e );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void RemoveActorFromScriptedScenes( CBaseFlex *pActor, bool instancedscenesonly )
{
	GetSceneManager()->RemoveActorFromScenes( pActor, instancedscenesonly );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool IsRunningScriptedScene( CBaseFlex *pActor, bool bIgnoreInstancedScenes )
{
	return GetSceneManager()->IsRunningScriptedScene( pActor, bIgnoreInstancedScenes );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool IsRunningScriptedSceneWithSpeech( CBaseFlex *pActor, bool bIgnoreInstancedScenes )
{
	return GetSceneManager()->IsRunningScriptedSceneWithSpeech( pActor, bIgnoreInstancedScenes );
}

