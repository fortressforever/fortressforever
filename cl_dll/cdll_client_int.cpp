//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include <crtmemdebug.h>
#include "vgui_int.h"
#include "clientmode.h"
#include "cdll_convar.h"
#include "iinput.h"
#include "iviewrender.h"
#include "ivieweffects.h"
#include "ivmodemanager.h"
#include "prediction.h"
#include "clientsideeffects.h"
#include "particlemgr.h"
#include "initializer.h"
#include "smoke_fog_overlay.h"
#include "view.h"
#include "ienginevgui.h"
#include "iefx.h"
#include "enginesprite.h"
#include "networkstringtable_clientdll.h"
#include "voice_status.h"
#include "FileSystem.h"
#include "c_te_legacytempents.h"
#include "c_rope.h"
#include "engine/IShadowMgr.h"
#include "engine/IStaticPropMgr.h"
#include "hud_basechat.h"
#include "hud_crosshair.h"
#include "env_wind_shared.h"
#include "detailobjectsystem.h"
#include "clienteffectprecachesystem.h"
#include "soundEnvelope.h"
#include "c_basetempentity.h"
#include "materialsystem/imaterialsystemstub.h"
#include "vguimatsurface/IMatSystemSurface.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "c_soundscape.h"
#include "engine/IVDebugOverlay.h"
#include "vguicenterprint.h"
#include "iviewrender_beams.h"
#include "tier0/vprof.h"
#include "engine/IEngineTrace.h"
#include "engine/ivmodelinfo.h"
#include "physics.h"
#include "usermessages.h"
#include "gamestringpool.h"
#include "c_user_message_register.h"
#include "igameuifuncs.h"
#include "saverestoretypes.h"
#include "saverestore.h"
#include "physics_saverestore.h"
#include "igameevents.h"
#include "engine/IVEngineCache.h"
#include "kbutton.h"
#include "vstdlib/icommandline.h"
#include "gamerules_register.h"
#include "vgui_controls/AnimationController.h"
#include "tgawriter.h"
#include "c_world.h"
#include "perfvisualbenchmark.h"	
#include "soundemittersystem/isoundemittersystembase.h"
#include "hud_closecaption.h"
#include "physpropclientside.h"
#include "panelmetaclassmgr.h"
#include "c_vguiscreen.h"
#include "imessagechars.h"
#include "cl_dll/IGameClientExports.h"
#include "ragdoll_shared.h"

// BEG: Added by Mulchman for team menu at level start up
#include <cl_dll/iviewport.h>
// END: Added by Mulchman for team menu at level start up

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// IF YOU ADD AN INTERFACE, EXTERN IT IN THE HEADER FILE.
IVEngineClient	*engine = NULL;
IVModelRender *modelrender = NULL;
IVEfx *effects = NULL;
ICvar *cvar = NULL;
IVRenderView *render = NULL;
IVDebugOverlay *debugoverlay = NULL;
IMaterialSystem *materials = NULL;
IMaterialSystemStub *materials_stub = NULL;
IMaterialSystemHardwareConfig *g_pMaterialSystemHardwareConfig = NULL;
IVEngineCache *engineCache = NULL;
IVModelInfoClient *modelinfo = NULL;
IEngineVGui *enginevgui = NULL;
INetworkStringTableContainer *networkstringtable = NULL;
ISpatialPartition* partition = NULL;
IFileSystem *filesystem = NULL;
IShadowMgr *shadowmgr = NULL;
IStaticPropMgrClient *staticpropmgr = NULL;
IEngineSound *enginesound = NULL;
IUniformRandomStream *random = NULL;
static CGaussianRandomStream s_GaussianRandomStream;
CGaussianRandomStream *randomgaussian = &s_GaussianRandomStream;
IMatSystemSurface *g_pMatSystemSurface = NULL;
ISharedGameRules *sharedgamerules = NULL;
IEngineTrace *enginetrace = NULL;
IGameUIFuncs *gameuifuncs = NULL;
IGameEventManager2 *gameeventmanager = NULL;
ISoundEmitterSystemBase *soundemitterbase = NULL;

IGameSystem *SoundEmitterSystem();

// String tables
INetworkStringTable *g_StringTableEffectDispatch = NULL;
INetworkStringTable *g_StringTableVguiScreen = NULL;
INetworkStringTable *g_pStringTableMaterials = NULL;
INetworkStringTable *g_pStringTableInfoPanel = NULL;

static CGlobalVarsBase dummyvars( true );
// So stuff that might reference gpGlobals during DLL initialization won't have a NULL pointer.
// Once the engine calls Init on this DLL, this pointer gets assigned to the shared data in the engine
CGlobalVarsBase *gpGlobals = &dummyvars;

class CHudChat;

class CViewRender;
extern CViewRender g_DefaultViewRender;

static C_BaseEntityClassList *s_pClassLists = NULL;
C_BaseEntityClassList::C_BaseEntityClassList()
{
	m_pNextClassList = s_pClassLists;
	s_pClassLists = this;
}
C_BaseEntityClassList::~C_BaseEntityClassList()
{
}

// Any entities that want an OnDataChanged during simulation register for it here.
class CDataChangedEvent
{
public:
	CDataChangedEvent() {}
	CDataChangedEvent( IClientNetworkable *ent, DataUpdateType_t updateType, int *pStoredEvent )
	{
		m_pEntity = ent;
		m_UpdateType = updateType;
		m_pStoredEvent = pStoredEvent;
	}

	IClientNetworkable	*m_pEntity;
	DataUpdateType_t	m_UpdateType;
	int					*m_pStoredEvent;
};

ISaveRestoreBlockHandler *GetEntitySaveRestoreBlockHandler();
ISaveRestoreBlockHandler *GetViewEffectsRestoreBlockHandler();

CUtlLinkedList<CDataChangedEvent, unsigned short> g_DataChangedEvents;
ClientFrameStage_t g_CurFrameStage = FRAME_UNDEFINED;


class IMoveHelper;

void DispatchHudText( const char *pszName );

static ConVar s_CV_ShowParticleCounts("showparticlecounts", "0", 0, "Display number of particles drawn per frame");
static ConVar s_cl_team("cl_team", "default", FCVAR_USERINFO|FCVAR_ARCHIVE, "Default team when joining a game");
static ConVar s_cl_class("cl_class", "default", FCVAR_USERINFO|FCVAR_ARCHIVE, "Default class when joining a game");

// Console variable accessor.
static CDLL_ConVarAccessor g_ConVarAccessor;

// Physics system
bool g_bLevelInitialized;
bool g_bTextMode = false;

// --> Mirv: For the hud hints loading/saving
extern void HudHintLoad(const char *pMapName);
extern void HudHintSave();
// <-- Mirv

//-----------------------------------------------------------------------------
// Purpose: interface for gameui to modify voice bans
//-----------------------------------------------------------------------------
class CGameClientExports : public IGameClientExports
{
public:
	// ingame voice manipulation
	bool IsPlayerGameVoiceMuted(int playerIndex)
	{
		return GetClientVoiceMgr()->IsPlayerBlocked(playerIndex);
	}

	void MutePlayerGameVoice(int playerIndex)
	{
		GetClientVoiceMgr()->SetPlayerBlockedState(playerIndex, true);
	}

	void UnmutePlayerGameVoice(int playerIndex)
	{
		GetClientVoiceMgr()->SetPlayerBlockedState(playerIndex, false);
	}
};

EXPOSE_SINGLE_INTERFACE( CGameClientExports, IGameClientExports, GAMECLIENTEXPORTS_INTERFACE_VERSION );


//-----------------------------------------------------------------------------
// Helper interface for voice.
//-----------------------------------------------------------------------------
class CHLVoiceStatusHelper : public IVoiceStatusHelper
{
public:
	virtual void GetPlayerTextColor(int entindex, int color[3])
	{
		color[0] = color[1] = color[2] = 128;
	}

	virtual void UpdateCursorState()
	{
	}

	virtual bool			CanShowSpeakerLabels()
	{
		return true;
	}
};
static CHLVoiceStatusHelper g_VoiceStatusHelper;




//-----------------------------------------------------------------------------
// Purpose: engine to client .dll interface
//-----------------------------------------------------------------------------
class CHLClient : public IBaseClientDLL
{
public:
	CHLClient();

	virtual int						Init( CreateInterfaceFn appSystemFactory, CreateInterfaceFn physicsFactory, CGlobalVarsBase *pGlobals );

	virtual void					Shutdown( void );

	virtual void					LevelInitPreEntity( const char *pMapName );
	virtual void					LevelInitPostEntity();
	virtual void					LevelShutdown( void );

	virtual ClientClass				*GetAllClasses( void );

	virtual int						HudVidInit( void );
	virtual void					HudProcessInput( bool bActive );
	virtual void					HudUpdate( bool bActive );
	virtual void					HudReset( void );
	virtual void					HudText( const char * message );

	// Mouse Input Interfaces
	virtual void					IN_ActivateMouse( void );
	virtual void					IN_DeactivateMouse( void );
	virtual void					IN_MouseEvent ( int mstate, bool down );
	virtual void					IN_Accumulate ( void );
	virtual void					IN_ClearStates ( void );
	virtual bool					IN_IsKeyDown( const char *name, bool& isdown );
	// Raw signal
	virtual int						IN_KeyEvent( int eventcode, int keynum, const char *pszCurrentBinding );
	// Create movement command
	virtual void					CreateMove ( int sequence_number, float input_sample_frametime, bool active );
	virtual void					ExtraMouseSample( float frametime, bool active );
	virtual bool					WriteUsercmdDeltaToBuffer( bf_write *buf, int from, int to, bool isnewcommand );	
	virtual void					EncodeUserCmdToBuffer( bf_write& buf, int slot );
	virtual void					DecodeUserCmdFromBuffer( bf_read& buf, int slot );


	virtual void					View_Render( vrect_t *rect );
	virtual void					RenderView( const CViewSetup &view, bool drawViewmodel );
	virtual void					View_Fade( ScreenFade_t *pSF );
	
	virtual void					SetCrosshairAngle( const QAngle& angle );

	virtual void					InitSprite( CEngineSprite *pSprite, const char *loadname );
	virtual void					ShutdownSprite( CEngineSprite *pSprite );

	virtual int						GetSpriteSize( void ) const;

	virtual void					VoiceStatus( int entindex, qboolean bTalking );

	virtual void					InstallStringTableCallback( const char *tableName );

	virtual void					FrameStageNotify( ClientFrameStage_t curStage );

	virtual bool					DispatchUserMessage( int msg_type, bf_read &msg_data );

	// Save/restore system hooks
	virtual CSaveRestoreData  *SaveInit( int size );
	virtual void			SaveWriteFields( CSaveRestoreData *, const char *, void *, datamap_t *, typedescription_t *, int );
	virtual void			SaveReadFields( CSaveRestoreData *, const char *, void *, datamap_t *, typedescription_t *, int );
	virtual void			PreSave( CSaveRestoreData * );
	virtual void			Save( CSaveRestoreData * );
	virtual void			WriteSaveHeaders( CSaveRestoreData * );
	virtual void			ReadRestoreHeaders( CSaveRestoreData * );
	virtual void			Restore( CSaveRestoreData *, bool );
	virtual void			DispatchOnRestore();
	virtual void			WriteSaveGameScreenshot( const char *pFilename );

	// Given a list of "S(wavname) S(wavname2)" tokens, look up the localized text and emit
	//  the appropriate close caption if running with closecaption = 1
	virtual void			EmitSentenceCloseCaption( char const *tokenstream );
	virtual void			EmitCloseCaption( char const *captionname, float duration );

	virtual CStandardRecvProxies* GetStandardRecvProxies();

public:
	void PrecacheMaterial( const char *pMaterialName );

private:
	void UncacheAllMaterials( );

	CUtlVector< IMaterial * > m_CachedMaterials;
};


CHLClient gHLClient;
IBaseClientDLL *clientdll = &gHLClient;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CHLClient, IBaseClientDLL, CLIENT_DLL_INTERFACE_VERSION, gHLClient );


//-----------------------------------------------------------------------------
// Precaches a material
//-----------------------------------------------------------------------------
void PrecacheMaterial( const char *pMaterialName )
{
	gHLClient.PrecacheMaterial( pMaterialName );
}

//-----------------------------------------------------------------------------
// Converts a previously precached material into an index
//-----------------------------------------------------------------------------
int GetMaterialIndex( const char *pMaterialName )
{
	if (pMaterialName)
	{
		int nIndex = g_pStringTableMaterials->FindStringIndex( pMaterialName );
		Assert( nIndex >= 0 );
		if (nIndex >= 0)
			return nIndex;
	}

	// This is the invalid string index
	return 0;
}

//-----------------------------------------------------------------------------
// Converts precached material indices into strings
//-----------------------------------------------------------------------------
const char *GetMaterialNameFromIndex( int nIndex )
{
	if (nIndex != (g_pStringTableMaterials->GetMaxStrings() - 1))
	{
		return g_pStringTableMaterials->GetString( nIndex );
	}
	else
	{
		return NULL;
	}
}


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

CHLClient::CHLClient() 
{
	// Kinda bogus, but the logic in the engine is too convoluted to put it there
	g_bLevelInitialized = false;
}


extern IGameSystem *ViewportClientSystem();

//-----------------------------------------------------------------------------
// Purpose: Called when the DLL is first loaded.
// Input  : engineFactory - 
// Output : int
//-----------------------------------------------------------------------------
int CHLClient::Init( CreateInterfaceFn appSystemFactory, CreateInterfaceFn physicsFactory, CGlobalVarsBase *pGlobals )
{
	InitCRTMemDebug();
	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );

	// Hook up global variables
	gpGlobals = pGlobals;

	// We aren't happy unless we get all of our interfaces.
	if(
		(engine = (IVEngineClient *)appSystemFactory( VENGINE_CLIENT_INTERFACE_VERSION, NULL )) == NULL ||
		(modelrender = (IVModelRender *)appSystemFactory( VENGINE_HUDMODEL_INTERFACE_VERSION, NULL )) == NULL ||
		(effects = (IVEfx *)appSystemFactory( VENGINE_EFFECTS_INTERFACE_VERSION, NULL )) == NULL ||
		(cvar = (ICvar *)appSystemFactory( VENGINE_CVAR_INTERFACE_VERSION, NULL )) == NULL ||
		(enginetrace = (IEngineTrace *)appSystemFactory( INTERFACEVERSION_ENGINETRACE_CLIENT, NULL )) == NULL ||
		(render = (IVRenderView *)appSystemFactory( VENGINE_RENDERVIEW_INTERFACE_VERSION, NULL )) == NULL ||
		(debugoverlay = (IVDebugOverlay *)appSystemFactory( VDEBUG_OVERLAY_INTERFACE_VERSION, NULL )) == NULL ||
		(materials = (IMaterialSystem *)appSystemFactory( MATERIAL_SYSTEM_INTERFACE_VERSION, NULL )) == NULL ||
		(engineCache = (IVEngineCache*)appSystemFactory(VENGINE_CACHE_INTERFACE_VERSION, NULL )) == NULL ||
		(modelinfo = (IVModelInfoClient *)appSystemFactory(VMODELINFO_CLIENT_INTERFACE_VERSION, NULL )) == NULL ||
		(enginevgui = (IEngineVGui *)appSystemFactory(VENGINE_VGUI_VERSION, NULL )) == NULL ||
		(networkstringtable = (INetworkStringTableContainer *)appSystemFactory(INTERFACENAME_NETWORKSTRINGTABLECLIENT,NULL)) == NULL ||
		(partition = (ISpatialPartition *)appSystemFactory(INTERFACEVERSION_SPATIALPARTITION, NULL)) == NULL ||
		(shadowmgr = (IShadowMgr *)appSystemFactory(ENGINE_SHADOWMGR_INTERFACE_VERSION, NULL)) == NULL ||
		(staticpropmgr = (IStaticPropMgrClient *)appSystemFactory(INTERFACEVERSION_STATICPROPMGR_CLIENT, NULL)) == NULL ||
		(enginesound = (IEngineSound *)appSystemFactory(IENGINESOUND_CLIENT_INTERFACE_VERSION, NULL)) == NULL ||
		(filesystem = (IFileSystem *)appSystemFactory(FILESYSTEM_INTERFACE_VERSION, NULL)) == NULL ||
		(random = (IUniformRandomStream *)appSystemFactory(VENGINE_CLIENT_RANDOM_INTERFACE_VERSION, NULL)) == NULL ||
		(gameuifuncs = (IGameUIFuncs * )appSystemFactory( VENGINE_GAMEUIFUNCS_VERSION, NULL )) == NULL ||
		(gameeventmanager = (IGameEventManager2 *)appSystemFactory(INTERFACEVERSION_GAMEEVENTSMANAGER2,NULL)) == NULL ||
		(soundemitterbase = (ISoundEmitterSystemBase *)appSystemFactory(SOUNDEMITTERSYSTEM_INTERFACE_VERSION, NULL)) == NULL 
		)
	{
		return false;
	}

	// Yes, both the client and game .dlls will try to Connect, the soundemittersystem.dll will handle this gracefully
	if ( !soundemitterbase->Connect( appSystemFactory ) )
	{
		return false;
	}

	if ( CommandLine()->FindParm( "-textmode" ) )
		g_bTextMode = true;

	// Not fatal if the material system stub isn't around.
	materials_stub = (IMaterialSystemStub*)appSystemFactory( MATERIAL_SYSTEM_STUB_INTERFACE_VERSION, NULL );

	g_pMaterialSystemHardwareConfig = materials->GetHardwareConfig( MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION, NULL );
	if( !g_pMaterialSystemHardwareConfig )
	{
		return false;
	}

	SetScreenSize();

	// Hook up the gaussian random number generator
	s_GaussianRandomStream.AttachToStream( random );

	// Initialize the console variables.
	ConCommandBaseMgr::OneTimeInit(&g_ConVarAccessor);

	if(!Initializer::InitializeAllObjects())
		return false;

	if(!g_ParticleMgr.Init(MAX_TOTAL_PARTICLES, materials))
		return false;

	if(!VGui_Startup( appSystemFactory ))
		return false;

	g_pMatSystemSurface = (IMatSystemSurface*)vgui::surface()->QueryInterface( MAT_SYSTEM_SURFACE_INTERFACE_VERSION ); 
	if (!g_pMatSystemSurface)
		return false;

	// Add the client systems.	
	
	// Client Leaf System has to be initialized first, since DetailObjectSystem uses it
	IGameSystem::Add( GameStringSystem() );
	IGameSystem::Add( SoundEmitterSystem() );
	IGameSystem::Add( ClientLeafSystem() );
	IGameSystem::Add( DetailObjectSystem() );
	IGameSystem::Add( ViewportClientSystem() );
	IGameSystem::Add( ClientEffectPrecacheSystem() );
	IGameSystem::Add( g_pClientShadowMgr );
	IGameSystem::Add( ClientThinkList() );
	IGameSystem::Add( ClientSoundscapeSystem() );
	IGameSystem::Add( PerfVisualBenchmark() );

#if defined( CLIENT_DLL ) && defined( COPY_CHECK_STRESSTEST )
	IGameSystem::Add( GetPredictionCopyTester() );
#endif

	modemanager->Init( );

	g_pClientMode->InitViewport();

	gHUD.Init();

	g_pClientMode->Init();

	if( !IGameSystem::InitAllSystems() )
		return false;

	g_pClientMode->Enable();

	if( !view )
	{
		view = ( IViewRender * )&g_DefaultViewRender;
	}

	view->Init();
	vieweffects->Init();

	C_BaseTempEntity::PrecacheTempEnts();

	input->Init_All();

	VGui_CreateGlobalPanels();

	InitSmokeFogOverlay();

	// Register user messages..
	CUserMessageRegister::RegisterAll();

	ClientVoiceMgr_Init();

	TGAWriter::SetFileSystem( appSystemFactory );

	// Embed voice status icons inside chat element
	{
		vgui::VPANEL parent = enginevgui->GetPanel( PANEL_CLIENTDLL );
		GetClientVoiceMgr()->Init( &g_VoiceStatusHelper, parent );
	}

	if ( !PhysicsDLLInit( physicsFactory ) )
		return false;

	g_pGameSaveRestoreBlockSet->AddBlockHandler( GetEntitySaveRestoreBlockHandler() );
	g_pGameSaveRestoreBlockSet->AddBlockHandler( GetPhysSaveRestoreBlockHandler() );
	g_pGameSaveRestoreBlockSet->AddBlockHandler( GetViewEffectsRestoreBlockHandler() );

	ClientWorldFactoryInit();

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Called when the client .dll is being dismissed
//-----------------------------------------------------------------------------
void CHLClient::Shutdown( void )
{
	ClientWorldFactoryShutdown();

	g_pGameSaveRestoreBlockSet->RemoveBlockHandler( GetViewEffectsRestoreBlockHandler() );
	g_pGameSaveRestoreBlockSet->RemoveBlockHandler( GetPhysSaveRestoreBlockHandler() );
	g_pGameSaveRestoreBlockSet->RemoveBlockHandler( GetEntitySaveRestoreBlockHandler() );

	ClientVoiceMgr_Shutdown();

	Initializer::FreeAllObjects();

	g_pClientMode->Disable();
	g_pClientMode->Shutdown();

	input->Shutdown_All();
	C_BaseTempEntity::ClearDynamicTempEnts();
	TermSmokeFogOverlay();
	view->Shutdown();
	UncacheAllMaterials();

	IGameSystem::ShutdownAllSystems();

	gHUD.Shutdown();
	VGui_Shutdown();
	
	ClearKeyValuesCache();

	g_pMatSystemSurface = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//  Called when the game initializes
//  and whenever the vid_mode is changed
//  so the HUD can reinitialize itself.
// Output : int
//-----------------------------------------------------------------------------
int CHLClient::HudVidInit( void )
{
	gHUD.VidInit();
	GetClientVoiceMgr()->VidInit();

	return 1;
}

//-----------------------------------------------------------------------------
// Method used to allow the client to filter input messages before the 
// move record is transmitted to the server
//-----------------------------------------------------------------------------

void CHLClient::HudProcessInput( bool bActive )
{
	// Q: Why isn't this done be the engine when it deals with all other input?
	input->ControllerCommands();

	g_pClientMode->ProcessInput( bActive );
}

//-----------------------------------------------------------------------------
// Purpose: Called when shared data gets changed, allows dll to modify data
// Input  : bActive - 
//-----------------------------------------------------------------------------
void CHLClient::HudUpdate( bool bActive )
{
	float frametime = gpGlobals->frametime;
	SetScreenSize();

	GetClientVoiceMgr()->Frame( frametime );

	gHUD.UpdateHud( bActive );

	C_BaseAnimating::AllowBoneAccess( true, false ); 
	IGameSystem::UpdateAllSystems( frametime );
	C_BaseAnimating::AllowBoneAccess( false, false ); 

	// run vgui animations
	vgui::GetAnimationController()->UpdateAnimations( engine->Time() );

	// I don't think this is necessary any longer, but I will leave it until
	// I can check into this further.
	C_BaseTempEntity::CheckDynamicTempEnts();
}

//-----------------------------------------------------------------------------
// Purpose: Called to restore to "non"HUD state.
//-----------------------------------------------------------------------------
void CHLClient::HudReset( void )
{
	gHUD.VidInit();
	PhysicsReset();
}

//-----------------------------------------------------------------------------
// Purpose: Called to add hud text message
//-----------------------------------------------------------------------------
void CHLClient::HudText( const char * message )
{
	DispatchHudText( message );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : ClientClass
//-----------------------------------------------------------------------------
ClientClass *CHLClient::GetAllClasses( void )
{
	return g_pClientClassHead;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLClient::IN_ActivateMouse( void )
{
	input->ActivateMouse();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLClient::IN_DeactivateMouse( void )
{
	input->DeactivateMouse();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : mstate - 
//-----------------------------------------------------------------------------
void CHLClient::IN_MouseEvent ( int mstate, bool down )
{
	input->MouseEvent( mstate, down );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLClient::IN_Accumulate ( void )
{
	input->AccumulateMouse();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLClient::IN_ClearStates ( void )
{
	input->ClearStates();
}

//-----------------------------------------------------------------------------
// Purpose: Engine can query for particular keys
// Input  : *name - 
//-----------------------------------------------------------------------------
bool CHLClient::IN_IsKeyDown( const char *name, bool& isdown )
{
	kbutton_t *key = input->FindKey( name );
	if ( !key )
	{
		return false;
	}
	
	isdown = ( key->state & 1 ) ? true : false;

	// Found the key by name
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Engine can issue a key event
// Input  : eventcode - 
//			keynum - 
//			*pszCurrentBinding - 
// Output : int
//-----------------------------------------------------------------------------
int CHLClient::IN_KeyEvent( int eventcode, int keynum, const char *pszCurrentBinding )
{
	return input->KeyEvent( eventcode, keynum, pszCurrentBinding );
}

void CHLClient::ExtraMouseSample( float frametime, bool active )
{
	Assert( C_BaseEntity::IsAbsRecomputationsEnabled() );
	Assert( C_BaseEntity::IsAbsQueriesValid() );

	C_BaseAnimating::AllowBoneAccess( true, false ); 

	input->ExtraMouseSample( frametime, active );

	C_BaseAnimating::AllowBoneAccess( false, false );
}

//-----------------------------------------------------------------------------
// Purpose: Fills in usercmd_s structure based on current view angles and key/controller inputs
// Input  : frametime - timestamp for last frame
//			*cmd - the command to fill in
//			active - whether the user is fully connected to a server
//-----------------------------------------------------------------------------
void CHLClient::CreateMove ( int sequence_number, float input_sample_frametime, bool active )
{

	Assert( C_BaseEntity::IsAbsRecomputationsEnabled() );
	Assert( C_BaseEntity::IsAbsQueriesValid() );

	C_BaseAnimating::AllowBoneAccess( true, false ); 

	input->CreateMove( sequence_number, input_sample_frametime, active );

	C_BaseAnimating::AllowBoneAccess( false, false );

}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *buf - 
//			from - 
//			to - 
//-----------------------------------------------------------------------------
bool CHLClient::WriteUsercmdDeltaToBuffer( bf_write *buf, int from, int to, bool isnewcommand )
{
	return input->WriteUsercmdDeltaToBuffer( buf, from, to, isnewcommand );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : buf - 
//			buffersize - 
//			slot - 
//-----------------------------------------------------------------------------
void CHLClient::EncodeUserCmdToBuffer( bf_write& buf, int slot )
{
	input->EncodeUserCmdToBuffer( buf, slot );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : buf - 
//			buffersize - 
//			slot - 
//-----------------------------------------------------------------------------
void CHLClient::DecodeUserCmdFromBuffer( bf_read& buf, int slot )
{
	input->DecodeUserCmdFromBuffer( buf, slot );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLClient::View_Render( vrect_t *rect )
{
	VPROF( "View_Render" );

	// UNDONE: This gets hit at startup sometimes, investigate - will cause NaNs in calcs inside Render()
	if ( rect->width == 0 || rect->height == 0 )
		return;

	view->Render( rect );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSF - 
//-----------------------------------------------------------------------------
void CHLClient::View_Fade( ScreenFade_t *pSF )
{
	if ( pSF != NULL )
		vieweffects->Fade( *pSF );
}


//-----------------------------------------------------------------------------
// Purpose: Per level init
//-----------------------------------------------------------------------------
void CHLClient::LevelInitPreEntity( char const* pMapName )
{
	// HACK: Bogus, but the logic is too complicated in the engine
	if (g_bLevelInitialized)
		return;
	g_bLevelInitialized = true;

	vieweffects->LevelInit();
	
	// --> Mirv: Initialise hud hints
	HudHintLoad(pMapName);
	// <-- Mirv

	// Tell mode manager that map is changing
	modemanager->LevelInit( pMapName );

	C_BaseTempEntity::ClearDynamicTempEnts();
	clienteffects->Flush();
	view->LevelInit();
	tempents->LevelInit();

	IGameSystem::LevelInitPreEntityAllSystems(pMapName);

	ResetWindspeed();

	// don't do prediction if single player!
	// don't set direct because of FCVAR_USERINFO
	if ( (gpGlobals->maxClients > 1) && !engine->IsHLTV() )
	{
		engine->ClientCmd( "cl_predict 1" );
	}
	else
	{
		engine->ClientCmd( "cl_predict 0" );
	}

	// Check low violence settings for this map
	g_RagdollLVManager.SetLowViolence( pMapName );

	gHUD.LevelInit();
}


//-----------------------------------------------------------------------------
// Purpose: Per level init
//-----------------------------------------------------------------------------
void CHLClient::LevelInitPostEntity( )
{
	IGameSystem::LevelInitPostEntityAllSystems();
	C_PhysPropClientside::RecreateAll();
	internalCenterPrint->Clear();

   // BEG: Added by Mulchman for team menu
   // Show the team menu selection every time we start a level
   gViewPortInterface->ShowPanel( PANEL_TEAM, true );

   // Show the class menu selection every time we start a level
   //gViewPortInterface->ShowPanel( PANEL_CLASS, true );

   // Pop the info panel back up above the team change menu
   IViewPortPanel *pPanel = gViewPortInterface->FindPanelByName( PANEL_INFO );
   if( pPanel )
      pPanel->ShowPanel( true );
}


//-----------------------------------------------------------------------------
// Purpose: Per level de-init
//-----------------------------------------------------------------------------
void CHLClient::LevelShutdown( void )
{
	// HACK: Bogus, but the logic is too complicated in the engine
	if (!g_bLevelInitialized)
		return;
	g_bLevelInitialized = false;

	// Disable abs recomputations when everything is shutting down
	CBaseEntity::EnableAbsRecomputations( false );

	// Level shutdown sequence.
	// First do the pre-entity shutdown of all systems
	IGameSystem::LevelShutdownPreEntityAllSystems();

	C_PhysPropClientside::DestroyAll();

	modemanager->LevelShutdown();

	// Now release/delete the entities
	cl_entitylist->Release();

	C_BaseEntityClassList *pClassList = s_pClassLists;
	while ( pClassList )
	{
		pClassList->LevelShutdown();
		pClassList = pClassList->m_pNextClassList;
	}

	// Now do the post-entity shutdown of all systems
	IGameSystem::LevelShutdownPostEntityAllSystems();

	tempents->LevelShutdown();
	beams->ClearBeams();
	g_ParticleMgr.RemoveAllEffects();

	gHUD.LevelShutdown();

	// --> Mirv: Initialise hud hints
	HudHintSave();
	// <-- Mirv

	//-- Added by L0ki --
	// Hide these panels so the game doesnt crash on us
	gViewPortInterface->ShowPanel( PANEL_TEAM, false );
	gViewPortInterface->ShowPanel( PANEL_CLASS, false );

	internalCenterPrint->Clear();
	messagechars->Clear();
	UncacheAllMaterials();
}


//-----------------------------------------------------------------------------
// Purpose: Engine can directly ask to render a view ( timerefresh and envmap creation, e.g. )
// Input  : &vs - 
//			drawViewmodel - 
//-----------------------------------------------------------------------------
void CHLClient::RenderView( const CViewSetup &vs, bool drawViewmodel )
{
	view->RenderView( vs, drawViewmodel );
}


//-----------------------------------------------------------------------------
// Purpose: Engine received crosshair offset ( autoaim )
// Input  : angle - 
//-----------------------------------------------------------------------------
void CHLClient::SetCrosshairAngle( const QAngle& angle )
{
	CHudCrosshair *crosshair = GET_HUDELEMENT( CHudCrosshair );
	if ( crosshair )
	{
		crosshair->SetCrosshairAngle( angle );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Helper to initialize sprite from .spr semaphor
// Input  : *pSprite - 
//			*loadname - 
//-----------------------------------------------------------------------------
void CHLClient::InitSprite( CEngineSprite *pSprite, const char *loadname )
{
	if ( pSprite )
	{
		pSprite->Init( loadname );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSprite - 
//-----------------------------------------------------------------------------
void CHLClient::ShutdownSprite( CEngineSprite *pSprite )
{
	if ( pSprite )
	{
		pSprite->Shutdown();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Tells engine how much space to allocate for sprite objects
// Output : int
//-----------------------------------------------------------------------------
int CHLClient::GetSpriteSize( void ) const
{
	return sizeof( CEngineSprite );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : entindex - 
//			bTalking - 
//-----------------------------------------------------------------------------
void CHLClient::VoiceStatus( int entindex, qboolean bTalking )
{
	GetClientVoiceMgr()->UpdateSpeakerStatus( entindex, !!bTalking );
}


//-----------------------------------------------------------------------------
// Called when the string table for materials changes
//-----------------------------------------------------------------------------
void OnMaterialStringTableChanged( void *object, INetworkStringTable *stringTable, int stringNumber, const char *newString, void const *newData )
{
	// Make sure this puppy is precached
	gHLClient.PrecacheMaterial( newString );
}

//-----------------------------------------------------------------------------
// Called when the string table for VGUI changes
//-----------------------------------------------------------------------------
void OnVguiScreenTableChanged( void *object, INetworkStringTable *stringTable, int stringNumber, const char *newString, void const *newData )
{
	// Make sure this puppy is precached
	vgui::Panel *pPanel = PanelMetaClassMgr()->CreatePanelMetaClass( newString, 100, NULL, NULL );
	if ( pPanel )
		PanelMetaClassMgr()->DestroyPanelMetaClass( pPanel );
}

//-----------------------------------------------------------------------------
// Purpose: Hook up any callbacks here, the table definition has been parsed but 
//  no data has been added yet
//-----------------------------------------------------------------------------
void CHLClient::InstallStringTableCallback( const char *tableName )
{
	// Here, cache off string table IDs
	if (!Q_strcasecmp(tableName, "VguiScreen"))
	{
		// Look up the id 
		g_StringTableVguiScreen = networkstringtable->FindTable( tableName );

		// When the material list changes, we need to know immediately
		g_StringTableVguiScreen->SetStringChangedCallback( NULL, OnVguiScreenTableChanged );
	}
	else if (!Q_strcasecmp(tableName, "Materials"))
	{
		// Look up the id 
		g_pStringTableMaterials = networkstringtable->FindTable( tableName );

		// When the material list changes, we need to know immediately
		g_pStringTableMaterials->SetStringChangedCallback( NULL, OnMaterialStringTableChanged );
	}
	else if ( !Q_strcasecmp( tableName, "EffectDispatch" ) )
	{
		g_StringTableEffectDispatch = networkstringtable->FindTable( tableName );
	}
	else if ( !Q_strcasecmp( tableName, "InfoPanel" ) )
	{
		g_pStringTableInfoPanel = networkstringtable->FindTable( tableName );
	}


	InstallStringTableCallback_GameRules();
}


//-----------------------------------------------------------------------------
// Material precache
//-----------------------------------------------------------------------------
void CHLClient::PrecacheMaterial( const char *pMaterialName )
{
	Assert( pMaterialName );

	int nLen = Q_strlen( pMaterialName );
	char *pTempBuf = (char*)stackalloc( nLen + 1 );
	memcpy( pTempBuf, pMaterialName, nLen + 1 );
	char *pFound = Q_strstr( pTempBuf, ".vmt\0" );
	if ( pFound )
	{
		*pFound = 0;
	}
		
	IMaterial *pMaterial = materials->FindMaterial( pTempBuf, TEXTURE_GROUP_PRECACHED );
	if ( !IsErrorMaterial( pMaterial ) )
	{
		pMaterial->IncrementReferenceCount();
		m_CachedMaterials.AddToTail( pMaterial );
	}
}

void CHLClient::UncacheAllMaterials( )
{
	for (int i = m_CachedMaterials.Count(); --i >= 0; )
	{
		m_CachedMaterials[i]->DecrementReferenceCount();
	}
	m_CachedMaterials.RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pszName - 
//			iSize - 
//			*pbuf - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHLClient::DispatchUserMessage( int msg_type, bf_read &msg_data )
{
	return usermessages->DispatchUserMessage( msg_type, msg_data );
}


void SimulateEntities()
{
	input->CAM_Think();

	// Service timer events (think functions).
  	ClientThinkList()->PerformThinkFunctions();

	// TODO: make an ISimulateable interface so C_BaseNetworkables can simulate?
	C_BaseEntityIterator iterator;
	C_BaseEntity *pEnt;
	while ( (pEnt = iterator.Next()) != NULL )
	{
		pEnt->Simulate();
	}
}


bool AddDataChangeEvent( IClientNetworkable *ent, DataUpdateType_t updateType, int *pStoredEvent )
{
	Assert( ent );
	// Make sure we don't already have an event queued for this guy.
	if ( *pStoredEvent >= 0 )
	{
		Assert( g_DataChangedEvents[*pStoredEvent].m_pEntity == ent );

		// DATA_UPDATE_CREATED always overrides DATA_UPDATE_CHANGED.
		if ( updateType == DATA_UPDATE_CREATED )
			g_DataChangedEvents[*pStoredEvent].m_UpdateType = updateType;
	
		return false;
	}
	else
	{
		*pStoredEvent = g_DataChangedEvents.AddToTail( CDataChangedEvent( ent, updateType, pStoredEvent ) );
		return true;
	}
}


void ClearDataChangedEvent( int iStoredEvent )
{
	if ( iStoredEvent != -1 )
		g_DataChangedEvents.Remove( iStoredEvent );
}


void ProcessOnDataChangedEvents()
{
	FOR_EACH_LL( g_DataChangedEvents, i )
	{
		CDataChangedEvent *pEvent = &g_DataChangedEvents[i];

		// Reset their stored event identifier.		
		*pEvent->m_pStoredEvent = -1;

		// Send the event.
		IClientNetworkable *pNetworkable = pEvent->m_pEntity;
		pNetworkable->OnDataChanged( pEvent->m_UpdateType );
	}

	g_DataChangedEvents.Purge();
}


void UpdatePVSNotifiers()
{
	// At this point, all the entities that were rendered in the previous frame have INPVS_THISFRAME set
	// so we can tell the entities that aren't in the PVS anymore so.
	CUtlLinkedList<CClientEntityList::CPVSNotifyInfo,unsigned short> &theList = ClientEntityList().GetPVSNotifiers();
	FOR_EACH_LL( theList, i )
	{
		CClientEntityList::CPVSNotifyInfo *pInfo = &theList[i];

		// If this entity thinks it's in the PVS, but it wasn't in the PVS this frame, tell it so.
		if ( pInfo->m_InPVSStatus & INPVS_YES )
		{
			if ( pInfo->m_InPVSStatus & INPVS_THISFRAME )
			{
				// Clear it for the next time around.
				pInfo->m_InPVSStatus &= ~INPVS_THISFRAME;
			}
			else
			{
				pInfo->m_InPVSStatus &= ~INPVS_YES;
				pInfo->m_pNotify->OnPVSStatusChanged( false );
			}
		}
	}
}


void OnRenderStart()
{
	VPROF( "OnRenderStart" );
	CEngineCacheCriticalSection engineCacheCriticalSection( engineCache );

	partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, true );
	C_BaseEntity::SetAbsQueriesValid( false );

	Rope_ResetCounters();

	// Interpolate server entities and move aiments.
	C_BaseEntity::InterpolateServerEntities();

	// Invalidate any bone information.
	C_BaseAnimating::InvalidateBoneCaches();

	C_BaseEntity::SetAbsQueriesValid( true );
	C_BaseEntity::EnableAbsRecomputations( true );

	// Enable access to all model bones except view models.
	// This is necessary for aim-ent computation to occur properly
	C_BaseAnimating::AllowBoneAccess( true, false );

	// FIXME: This needs to be done before the player moves; it forces
	// aiments the player may be attached to to forcibly update their position
	C_BaseEntity::MarkAimEntsDirty();

	// This will place the player + the view models + all parent
	// entities	at the correct abs position so that their attachment points
	// are at the correct location
	view->OnRenderStart();

	// This will place all entities in the correct position in world space and in the KD-tree
	C_BaseAnimating::UpdateClientSideAnimations();

	partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, false );

	// Process OnDataChanged events.
	ProcessOnDataChangedEvents();

	// Reset the overlay alpha. Entities can change the state of this in their think functions.
	g_SmokeFogOverlayAlpha = 0;	

	// Simulate all the entities.
	SimulateEntities();
	PhysicsSimulate();

	// This creates things like temp entities.
	engine->FireEvents();

	// Update temp entities
	tempents->Update();

	// Update temp ent beams...
	beams->UpdateTempEntBeams();
	
	// Lock the frame from beam additions
	SetBeamCreationAllowed( false );

	// Update particle effects (eventually, the effects should use Simulate() instead of having
	// their own update system).
	{
		VPROF_BUDGET( "g_ParticleMgr.Update", VPROF_BUDGETGROUP_PARTICLE_RENDERING );
		g_ParticleMgr.Simulate( gpGlobals->frametime );
	}

	// Now that the view model's position is setup and aiments are marked dirty, update
	// their positions so they're in the leaf system correctly.
	C_BaseEntity::CalcAimEntPositions();

	// Finally, link all the entities into the leaf system right before rendering.
	C_BaseEntity::AddVisibleEntities();
}


void OnRenderEnd()
{
	// Disallow access to bones (access is enabled in CViewRender::SetUpView).
	C_BaseAnimating::AllowBoneAccess( false, false );

	UpdatePVSNotifiers();
}


void CHLClient::FrameStageNotify( ClientFrameStage_t curStage )
{
	g_CurFrameStage = curStage;

	switch( curStage )
	{
	default:
		break;

	case FRAME_RENDER_START:
		{
			VPROF( "CHLClient::FrameStageNotify FRAME_RENDER_START" );
			// Last thing before rendering, run simulation.
			OnRenderStart();
		}
		break;
		
	case FRAME_RENDER_END:
		{
			VPROF( "CHLClient::FrameStageNotify FRAME_RENDER_END" );
			OnRenderEnd();
		}
		break;
		
	case FRAME_NET_UPDATE_START:
		{
			VPROF( "CHLClient::FrameStageNotify FRAME_NET_UPDATE_START" );
			// disabled all recomputations while we update entities
			C_BaseEntity::EnableAbsRecomputations( false );
			C_BaseEntity::SetAbsQueriesValid( false );
			Interpolation_SetLastPacketTimeStamp( engine->GetLastTimeStamp() );
			partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, true );
		}
		break;
	case FRAME_NET_UPDATE_END:
		{
			// reenable abs recomputation since now all entities have been updated
			C_BaseEntity::EnableAbsRecomputations( true );
			C_BaseEntity::SetAbsQueriesValid( true );
			partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, false );
		}
		break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
		{
			VPROF( "CHLClient::FrameStageNotify FRAME_NET_UPDATE_POSTDATAUPDATE_START" );
		}
		break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
		{
			VPROF( "CHLClient::FrameStageNotify FRAME_NET_UPDATE_POSTDATAUPDATE_END" );
			// Let prediction copy off pristine data
			prediction->PostEntityPacketReceived();
		}
		break;
	case FRAME_START:
		{
			// Mark the frame as open for client fx additions
			SetFXCreationAllowed( true );
			SetBeamCreationAllowed( true );
		}
		break;
	}
}

CSaveRestoreData *SaveInit( int size );

// Save/restore system hooks
CSaveRestoreData  *CHLClient::SaveInit( int size )
{
	return ::SaveInit(size);
}

void CHLClient::SaveWriteFields( CSaveRestoreData *pSaveData, const char *pname, void *pBaseData, datamap_t *pMap, typedescription_t *pFields, int fieldCount )
{
	CSave saveHelper( pSaveData );
	saveHelper.WriteFields( pname, pBaseData, pMap, pFields, fieldCount );
}

void CHLClient::SaveReadFields( CSaveRestoreData *pSaveData, const char *pname, void *pBaseData, datamap_t *pMap, typedescription_t *pFields, int fieldCount )
{
	CRestore restoreHelper( pSaveData );
	restoreHelper.ReadFields( pname, pBaseData, pMap, pFields, fieldCount );
}

void CHLClient::PreSave( CSaveRestoreData *s )
{
	g_pGameSaveRestoreBlockSet->PreSave( s );
}

void CHLClient::Save( CSaveRestoreData *s )
{
	CSave saveHelper( s );
	g_pGameSaveRestoreBlockSet->Save( &saveHelper );
}

void CHLClient::WriteSaveHeaders( CSaveRestoreData *s )
{
	CSave saveHelper( s );
	g_pGameSaveRestoreBlockSet->WriteSaveHeaders( &saveHelper );
	g_pGameSaveRestoreBlockSet->PostSave();
}

void CHLClient::ReadRestoreHeaders( CSaveRestoreData *s )
{
	CRestore restoreHelper( s );
	g_pGameSaveRestoreBlockSet->PreRestore();
	g_pGameSaveRestoreBlockSet->ReadRestoreHeaders( &restoreHelper );
}

void CHLClient::Restore( CSaveRestoreData *s, bool b )
{
	CRestore restore(s);
	g_pGameSaveRestoreBlockSet->Restore( &restore, b );
	g_pGameSaveRestoreBlockSet->PostRestore();
}

static CUtlVector<EHANDLE> g_RestoredEntities;

void AddRestoredEntity( C_BaseEntity *pEntity )
{
	if ( !pEntity )
		return;

	g_RestoredEntities.AddToTail( EHANDLE(pEntity) );
}

void CHLClient::DispatchOnRestore()
{
	for ( int i = 0; i < g_RestoredEntities.Count(); i++ )
	{
		if ( g_RestoredEntities[i] != NULL )
		{
			engineCache->EnterCriticalSection();
			g_RestoredEntities[i]->OnRestore();
			engineCache->ExitCriticalSection();
		}
	}
	g_RestoredEntities.RemoveAll();
}

void CHLClient::WriteSaveGameScreenshot( const char *pFilename )
{
	view->WriteSaveGameScreenshot( pFilename );
}

// Given a list of "S(wavname) S(wavname2)" tokens, look up the localized text and emit
//  the appropriate close caption if running with closecaption = 1
void CHLClient::EmitSentenceCloseCaption( char const *tokenstream )
{
	extern ConVar closecaption;

	if ( !closecaption.GetBool() )
		return;

	CHudCloseCaption *hudCloseCaption = GET_HUDELEMENT( CHudCloseCaption );
	if ( hudCloseCaption )
	{
		hudCloseCaption->ProcessSentenceCaptionStream( tokenstream );
	}
}


void CHLClient::EmitCloseCaption( char const *captionname, float duration )
{
	extern ConVar closecaption;

	if ( !closecaption.GetBool() )
		return;

	CHudCloseCaption *hudCloseCaption = GET_HUDELEMENT( CHudCloseCaption );
	if ( hudCloseCaption )
	{
		hudCloseCaption->ProcessCaption( captionname, duration );
	}
}

CStandardRecvProxies* CHLClient::GetStandardRecvProxies()
{
	return &g_StandardRecvProxies;
}

