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
#include "ff_mapguide.h"		// |-- Mirv: Map guides
#include "ff_weapon_base.h"

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

	//--- Added by L0ki ---
	virtual void Simulate();
	//---------------------

public:
	// Beg: Added by Mulchman for building objects and such
	//EHANDLE m_hDispenser; // Shared network handle for the dispenser
	//EHANDLE m_hSentryGun; // Shared network handle for the sentry gun
	//EHANDLE m_hDetpack; // Shared network handle for the detpack
	CNetworkHandle( CBaseEntity, m_hDispenser );
	CNetworkHandle( C_AI_BaseNPC, m_hSentryGun );
	CNetworkHandle( CBaseEntity, m_hDetpack );

	// Used for seeing if a player is currently
	// trying to build a detpack, dispenser, or sentry gun
	CNetworkVar( bool, m_bBuilding );
	// Tells us what we are currently trying to build
	CNetworkVar( int, m_iCurBuild );
	// Tells us if the player cancelled building
	//CNetworkVar( bool, m_bCancelledBuild );

	bool	m_bClientBuilding;
	// End: Added by Mulchman for building objects and such

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

	float	m_flJumpTime;
	float	m_flFallTime;
	// <-- Mirv: Proper sound effects

	// ---> added by billdoor
public:
	CNetworkVar(float, m_fArmorType);

	float GetArmorType() const { return m_fArmorType; };
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
	
	virtual void PreThink( void );
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

	virtual void CreateMove(float flInputSampleTime, CUserCmd *pCmd);

	// Mirv: In TFC the AbsOrigin is midway up the model. We need to take this into
	// account for various things. 
	Vector GetLegacyAbsOrigin();

	virtual void FireBullets(const FireBulletsInfo_t &info);
	virtual bool HandleShotImpactingWater(const FireBulletsInfo_t &info, const Vector &vecEnd, ITraceFilter *pTraceFilter, Vector *pVecTracerDest);

private:
	C_FFPlayer( const C_FFPlayer & );
};


inline C_FFPlayer* ToFFPlayer( CBaseEntity *pPlayer )
{
	Assert( dynamic_cast< C_FFPlayer* >( pPlayer ) != NULL );
	return static_cast< C_FFPlayer* >( pPlayer );
}

#endif // C_FF_PLAYER_H
