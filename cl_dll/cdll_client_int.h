//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef CDLL_CLIENT_INT_H
#define CDLL_CLIENT_INT_H
#ifdef _WIN32
#pragma once
#endif

#include "iclientnetworkable.h"
#include "utllinkedlist.h"
#include "cdll_int.h"


class IVModelRender;
class IVEngineClient;
class IVModelRender;
class IVEfx;
class IVRenderView;
class IVDebugOverlay;
class IMaterialSystem;
class IMaterialSystemStub;
class IDataCache;
class IMDLCache;
class IVModelInfoClient;
class IEngineVGui;
class ISpatialPartition;
class IBaseClientDLL;
class ISpatialPartition;
class IFileSystem;
class IStaticPropMgrClient;
class IShadowMgr;
class IUniformRandomStream;
class CGaussianRandomStream;
class IEngineSound;
class IMatSystemSurface;
class IMaterialSystemHardwareConfig;
class ISharedGameRules;
class IEngineTrace;
class IGameUIFuncs;
class IGameEventManager2;
class IPhysicsGameTrace;
class CGlobalVarsBase;
class IClientTools;
class C_BaseAnimating;
class IColorCorrectionSystem;
class IInputSystem;
class ISceneFileCache;
class IAvi;

extern IVModelRender *modelrender;
extern IVEngineClient	*engine;
extern IVModelRender *modelrender;
extern IVEfx *effects;
extern IVRenderView *render;
extern IVDebugOverlay *debugoverlay;
extern IMaterialSystem *materials;
extern IMaterialSystemStub *materials_stub;
extern IMaterialSystemHardwareConfig *g_pMaterialSystemHardwareConfig;
extern IDataCache *datacache;
extern IMDLCache *mdlcache;
extern IVModelInfoClient *modelinfo;
extern IEngineVGui *enginevgui;
extern ISpatialPartition* partition;
extern IBaseClientDLL *clientdll;
extern IFileSystem *filesystem;
extern IStaticPropMgrClient *staticpropmgr;
extern IShadowMgr *shadowmgr;
extern IUniformRandomStream *random;
extern CGaussianRandomStream *randomgaussian;
extern IEngineSound *enginesound;
extern IMatSystemSurface *g_pMatSystemSurface;
extern IEngineTrace *enginetrace;
extern IGameUIFuncs *gameuifuncs;
extern IGameEventManager2 *gameeventmanager;
extern IPhysicsGameTrace *physgametrace;
extern CGlobalVarsBase *gpGlobals;
extern IClientTools *clienttools;
extern IColorCorrectionSystem *colorcorrection;
extern IInputSystem *inputsystem;
extern ISceneFileCache *scenefilecache;
extern IAvi *avi;

// Set to true between LevelInit and LevelShutdown.
extern bool	g_bLevelInitialized;
extern bool g_bTextMode;

//-----------------------------------------------------------------------------
// Purpose: engine to client .dll interface
//-----------------------------------------------------------------------------
// FF: Class definition moved here from the .cpp so that we can inherit from it
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
	virtual void					IN_MouseEvent( int mstate, bool down );
	virtual void					IN_Accumulate( void );
	virtual void					IN_ClearStates( void );
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
	virtual void					RenderView( const CViewSetup &view, int nClearFlags, bool drawViewmodel );
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

	virtual bool			CanRecordDemo( char *errorMsg, int length ) const;

	// save game screenshot writing
	virtual void			WriteSaveGameScreenshotOfSize( const char *pFilename, int width, int height );

	// See RenderViewInfo_t
	virtual void			RenderViewEx( const CViewSetup &view, int nClearFlags, int whatToDraw );

	// Gets the location of the player viewpoint
	virtual bool			GetPlayerView( CViewSetup &playerView );

public:
	void PrecacheMaterial( const char *pMaterialName );

private:
	void UncacheAllMaterials( );

	CUtlVector< IMaterial * > m_CachedMaterials;
};

// Returns true if a new OnDataChanged event is registered for this frame.
bool AddDataChangeEvent( IClientNetworkable *ent, DataUpdateType_t updateType, int *pStoredEvent );

void ClearDataChangedEvent( int iStoredEvent );

//-----------------------------------------------------------------------------
// Precaches a material
//-----------------------------------------------------------------------------
void PrecacheMaterial( const char *pMaterialName );

//-----------------------------------------------------------------------------
// Converts a previously precached material into an index
//-----------------------------------------------------------------------------
int GetMaterialIndex( const char *pMaterialName );

//-----------------------------------------------------------------------------
// Converts precached material indices into strings
//-----------------------------------------------------------------------------
const char *GetMaterialNameFromIndex( int nIndex );

//-----------------------------------------------------------------------------
// Called during bone setup to test perf
//-----------------------------------------------------------------------------
void TrackBoneSetupEnt( C_BaseAnimating *pEnt );


#endif // CDLL_CLIENT_INT_H
