//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_FF_PLAYER_H
#define C_FF_PLAYER_H
#ifdef _WIN32
#pragma once
#endif

#include "ff_playeranimstate.h"
#include "c_baseplayer.h"
#include "ff_shareddefs.h"
#include "baseparticleentity.h"
#include "ff_esp_shared.h"
#include "ff_mapguide.h"
#include "ff_weapon_base.h"
#include "iviewrender_beams.h"
#include "Sprite.h"
#include "ff_fx_infection.h"
#include "ff_fx_immunity.h"
#include "ff_buildableobjects_shared.h"
#include "ff_radiotagdata.h"

class C_FFBuildableObject;
class C_FFDetpack;
class C_FFDispenser;
class C_FFSentryGun;
class C_FFSevTest;

#define FF_BUILD_NONE		0
#define FF_BUILD_DISPENSER	1
#define FF_BUILD_SENTRYGUN	2
#define FF_BUILD_DETPACK	3

// BEG: Added by Mulchman for team junk
#define FF_TEAM_UNASSIGNED	0
#define FF_TEAM_SPEC		1
#define FF_TEAM_BLUE		2
#define FF_TEAM_RED			3
#define FF_TEAM_YELLOW		4
#define FF_TEAM_GREEN		5
// END: Added by Mulchman for team junk

extern ConVar r_selfshadows;

void CC_PrimeOne(void);
void CC_PrimeTwo(void);
void CC_ThrowGren(void);

// --> Mirv: More gren priming functions
void CC_ToggleOne( void );
void CC_ToggleTwo( void );
// <-- Mirv: More gren priming functions

// Moved here from ff_shareddefs.h
typedef struct SpyInfo_s
{
	char	m_szName[ MAX_PLAYER_NAME_LENGTH ];	// Name we're using
	int		m_iTeam;	// Disguised team
	int		m_iClass;	// Disguised class

	void	Set( const char *pszName, int iTeam, int iClass )
	{
		Q_strcpy( m_szName, pszName );
		m_iTeam = iTeam;
		m_iClass = iClass;
	}

	void	SetTeam( int iTeam ) { m_iTeam = iTeam; }
	void	SetClass( int iClass ) { m_iClass = iClass; }
	void	SetName( const char *pszName ) { Q_strcpy( m_szName, pszName ); }

	bool	SameGuy( int iTeam, int iClass )
	{
		return( ( m_iTeam == iTeam ) && ( m_iClass == iClass ) );
	}

} CrosshairInfo_s;

//=============================================================================
//
// Class CFFRadioTagData
//
//=============================================================================
//-----------------------------------------------------------------------------
// Purpose: Silly class to encapsulate how many pipes a player has out currently
//-----------------------------------------------------------------------------
class CFFPipebombCounter
{
public:
	CFFPipebombCounter( void )
	{
		m_iPipes = -1;
	}

	void Increment( void )
	{
		m_iPipes = clamp( m_iPipes + 1, 0, 8 );
	}

	// Call this when the player dies
	void CFFPipebombCounter::Reset( void )
	{
		m_iPipes = -1;
	}

	// Get the number of pipes out
	int CFFPipebombCounter::GetPipes( void ) const
	{
		return m_iPipes;
	}

private:
	int m_iPipes;
};

class C_FFPlayer : public C_BasePlayer, public IFFPlayerAnimStateHelpers
{
public:
	DECLARE_CLASS( C_FFPlayer, C_BasePlayer );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();

	C_FFPlayer();
	~C_FFPlayer();

	static C_FFPlayer* GetLocalFFPlayer();

	virtual const QAngle& GetRenderAngles();
	virtual void UpdateClientSideAnimation();
	virtual void PostDataUpdate( DataUpdateType_t updateType );
	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual int  DrawModel( int flags );

	//--- Added by L0ki ---
	virtual void Simulate();
	//---------------------

protected:
	// Beg: Added by Mulchman for building objects and such
	CNetworkHandle( C_FFDispenser, m_hDispenser );
	CNetworkHandle( C_FFSentryGun, m_hSentryGun );
	CNetworkHandle( C_FFDetpack, m_hDetpack );

	// Used for seeing if a player is currently
	// trying to build a detpack, dispenser, or sentry gun
	CNetworkVar( bool, m_bBuilding );
	// Tells us what we are currently trying to build
	CNetworkVar( int, m_iCurBuild );

public:
	bool IsBuilding( void ) const;
	int GetCurBuild( void ) const;
	C_FFDetpack *GetDetpack( void ) const;
	C_FFDispenser *GetDispenser( void ) const;
	C_FFSentryGun *GetSentryGun( void ) const;
	C_FFBuildableObject *GetBuildable( int iBuildable ) const;
	// End: Added by Mulchman for building objects and such

	bool IsInfected( void ) const	{ return m_bInfected != 0; }
	// Two girls for every boy?
	CSmartPtr< CInfectionEmitter >	m_pInfectionEmitter1;
	CSmartPtr< CInfectionEmitter >	m_pInfectionEmitter2;

	bool IsImmune( void ) const		{ return m_bImmune != 0; }
	CSmartPtr< CImmunityEmitter >	m_pImmunityEmitter1;
	CSmartPtr< CImmunityEmitter >	m_pImmunityEmitter2;
private:
	unsigned int m_bInfected;
	unsigned int m_bImmune;

// Called by shared code.
public:
	
	// IFFPlayerAnimState overrides.
	virtual CFFWeaponBase* FFAnim_GetActiveWeapon();
	virtual bool FFAnim_CanMove();

	void DoAnimationEvent( PlayerAnimEvent_t event );
	bool ShouldDraw();

	IFFPlayerAnimState *m_PlayerAnimState;

	QAngle	m_angEyeAngles;
	CInterpolatedVar< QAngle >	m_iv_angEyeAngles;

	CNetworkVar( int, m_iShotsFired );	// number of shots fired recently

	EHANDLE	m_hRagdoll;

	CFFWeaponBase *GetActiveFFWeapon() const;

	C_BaseAnimating *BecomeRagdollOnClient( bool bCopyEntity);
	IRagdoll* C_FFPlayer::GetRepresentativeRagdoll() const;

	void FireBullet( 
		Vector vecSrc, 
		const QAngle &shootAngles, 
		float vecSpread, 
		float flDamage, 	// |-- Mirv: Float
		int iBulletType,
		CBaseEntity *pevAttacker,
		bool bDoEffects,
		float x,
		float y,
		float flSniperRifleCharge = 0.0f ); // added by Mulchman 9/20/2005
											// |-- Mirv: Modified a bit

	// --> Mirv: Proper sound effects
	void PlayJumpSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol );
	void PlayFallSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol );
	void PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );

	virtual void Precache();

	float	m_flJumpTime;
	float	m_flFallTime;
	// <-- Mirv: Proper sound effects

	// ---> added by billdoor
public:
	CNetworkVar(float, m_flArmorType);
	
	int NeedsArmor( void ) const { return GetMaxArmor() - GetArmor(); }

	float GetArmorType() const { return m_flArmorType; };
	// ---> end

	// --> mulch
	int GetHealthPercentage( void );
	int GetArmorPercentage( void );

	// This is so when we ID a disguised spy we use the same
	// player name until that spy disguises as something else
	SpyInfo_s m_hSpyTracking[ MAX_PLAYERS + 1 ];

	// The current guy you're ID'ing
	// This is mainly for if you want to
	// add %i lookup in messages
	CrosshairInfo_s m_hCrosshairInfo;

	virtual ShadowType_t ShadowCastType( void );

public:	
	SpyDisguiseWeapon m_DisguisedWeapons[11];
	int GetDisguisedClass( void );
	int GetDisguisedTeam( void );
	bool IsDisguised( void );
	CNetworkVar( int, m_iSpyDisguise );
	// <-- mulch

private:
	// ---> FF movecode stuff (billdoor)
	friend CFFGameMovement;
	void StartSkiing(void) { if(m_iSkiState == 0) m_iSkiState = 1; m_iLocalSkiState = 1; };
	void StopSkiing(void) { if(m_iSkiState == 1) m_iSkiState = 0; m_iLocalSkiState = 0; };
	int GetSkiState(void) { return m_iSkiState.Get(); };
	CNetworkVar(int, m_iSkiState);
	// this version of the ski state is not sent over the network, but is altered only by the movecode for the local player
	int m_iLocalSkiState;
	// ---> end

	// Beg: Added by L0ki for grenade stuff
public:
	CNetworkVar(int, m_iGrenadeState);
	CNetworkVar(float, m_flServerPrimeTime);
	CNetworkVar(int, m_iPrimary);
	CNetworkVar(int, m_iSecondary);

	float m_flPrimeTime;
	float m_flLatency;
	// End: Added by L0ki for grenade stuff

	// Beg: Added by FryGuy for status effect stuff
	CNetworkVar(float, m_flNextBurnTick);   // when the next burn tick should fire
	CNetworkVar(int, m_iBurnTicks);         // how many more ticks are left to fire
	CNetworkVar(float, m_flBurningDamage);  // how much total damage is left to take
	// End: Added by FryGuy

	// --> Mirv: Map guide stuff
	CNetworkHandle( CFFMapGuide, m_hNextMapGuide );
	CNetworkHandle( CFFMapGuide, m_hLastMapGuide );

	float		m_flNextMapGuideTime;
	// <-- Mirv: Map guide stuff

	// --> Mirv: Conc stuff
	float m_flConcTime;
	QAngle m_angConced, m_angConcedTest;
	
	virtual const QAngle &EyeAngles();
	virtual void CalcViewModelView( const Vector& eyeOrigin, const QAngle& eyeAngles);
	virtual void CalcView( Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov );
	
	virtual void ClientThink( void );
	virtual void PreThink( void );
	virtual void PostThink( void );
	// <-- Mirv: Conc stuff

	// --> Mirv: Hold some class info on the player side
	int m_iClassStatus;
	int GetClassSlot( void );

	void ClassSpecificSkill();
	void ClassSpecificSkill_Post();
	// <-- Mirv: Hold some class info on the player side

	float m_flNextClassSpecificSkill;

	int	  m_iSpawnInterpCounter;
	int	  m_iSpawnInterpCounterCache;

	void SwapToWeapon(FFWeaponID);
	void SwapToWeaponSlot(int iSlot);

	bool m_bFirstSpawn;
	virtual void Spawn( void );
	virtual void CreateMove(float flInputSampleTime, CUserCmd *pCmd);

	virtual void Death();

	// Mirv: In TFC the AbsOrigin is midway up the model. We need to take this into
	// account for various things. 
	Vector GetLegacyAbsOrigin();
	Vector GetWaistOrigin( void );
	Vector GetFeetOrigin( void );

	virtual void FireBullets(const FireBulletsInfo_t &info);
	virtual bool HandleShotImpactingWater(const FireBulletsInfo_t &info, const Vector &vecEnd, ITraceFilter *pTraceFilter, Vector *pVecTracerDest);

	virtual void AddEntity();
	void ReleaseFlashlight();
	virtual void NotifyShouldTransmit(ShouldTransmitState_t state);
	virtual bool ShouldReceiveProjectedTextures(int flags);

	Beam_t		*m_pFlashlightBeam;

	float		m_flIdleTime;

	bool Weapon_Switch(CBaseCombatWeapon *pWeapon, int viewmodelindex = 0);
	CBaseCombatWeapon *m_pOldActiveWeapon;

	float		m_flNextJumpTimeForDouble;
	bool		m_bCanDoubleJump;

	virtual		float GetFOV();

	float		m_flSpeedModifier;

	int			m_iHallucinationIndex;
	float		m_flHallucinationFinish;

public:
	Color GetTeamColor( void ) const { return m_clrTeamColor; }
protected:
	Color m_clrTeamColor;

	// **********************************
	// SaveMe stuffs
public:
	bool IsInSaveMe( void ) const { return m_iSaveMe != 0; }
protected:
	unsigned int m_iSaveMe;
	// **********************************

	// **********************************
	// EngyMe stuffs
public:
	bool IsInEngyMe( void ) const { return m_iEngyMe != 0; }
protected:
	unsigned int m_iEngyMe;
	// **********************************

private:
	C_FFPlayer( const C_FFPlayer & );

	// Local radio tag data
public:	
	C_FFRadioTagData *GetRadioTagData( void )	{ return m_hRadioTagData.Get(); }
private:
	CNetworkHandle( C_FFRadioTagData, m_hRadioTagData );

	// Pipebomb stuff
public:	
	CFFPipebombCounter *GetPipebombCounter( void ) { return &m_hPipebombCounter; }
private:
	CFFPipebombCounter m_hPipebombCounter;
};

// Just straight up copying the server version. Tired
// of this nonsense.
inline C_FFPlayer *ToFFPlayer( CBaseEntity *pEntity )
{
	if( !pEntity || !pEntity->IsPlayer() )
		return NULL;

#ifdef _DEBUG
	Assert( dynamic_cast< C_FFPlayer * >( pEntity ) != 0 );
#endif
	return static_cast< C_FFPlayer * >( pEntity );
}

#endif // C_FF_PLAYER_H
