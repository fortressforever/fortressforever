//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "base_playeranimstate.h"
#include "tier0/vprof.h"
#include "animation.h"
#include "studio.h"
#include "apparent_velocity_helper.h"
#include "utldict.h"
#include "in_buttons.h"

#include "ff_playeranimstate.h"
#include "ff_weapon_base.h"

#ifdef CLIENT_DLL
	#include "c_ff_player.h"
	#include "bone_setup.h"
	#include "interpolatedvar.h"
#else
	#include "ff_player.h"
#endif

#define ANIM_TOPSPEED_WALK			100
#define ANIM_TOPSPEED_RUN			250
#define ANIM_TOPSPEED_RUN_CROUCH	85

#define ANGLE_BEFORE_TURN_LEGS		60 // changed from 30 -> 60 for rebo

#define DEFAULT_IDLE_NAME "idle_upper_"
#define DEFAULT_CROUCH_IDLE_NAME "crouch_idle_upper_"
#define DEFAULT_CROUCH_WALK_NAME "crouch_walk_upper_"
#define DEFAULT_WALK_NAME "walk_upper_"
#define DEFAULT_RUN_NAME "run_upper_"
#define DEFAULT_SLIDE_NAME "slide_upper_"

#define DEFAULT_FIRE_IDLE_NAME "idle_shoot_"
#define DEFAULT_FIRE_CROUCH_NAME "crouch_idle_shoot_"
#define DEFAULT_FIRE_CROUCH_WALK_NAME "crouch_walk_shoot_"
#define DEFAULT_FIRE_WALK_NAME "walk_shoot_"
#define DEFAULT_FIRE_RUN_NAME "run_shoot_"


#define FIRESEQUENCE_LAYER		(AIMSEQUENCE_LAYER+NUM_AIMSEQUENCE_LAYERS)
#define RELOADSEQUENCE_LAYER	(FIRESEQUENCE_LAYER + 1)
#define NUM_LAYERS_WANTED		(RELOADSEQUENCE_LAYER + 1)



// ------------------------------------------------------------------------------------------------ //
// CFFPlayerAnimState declaration.
// ------------------------------------------------------------------------------------------------ //

class CFFPlayerAnimState : public CBasePlayerAnimState, public IFFPlayerAnimState
{
public:
	DECLARE_CLASS( CFFPlayerAnimState, CBasePlayerAnimState );
	friend IFFPlayerAnimState* CreatePlayerAnimState( CBaseAnimatingOverlay *pEntity, IFFPlayerAnimStateHelpers *pHelpers, LegAnimType_t legAnimType, bool bUseAimSequences );

	CFFPlayerAnimState();

	void InitFF( CBaseAnimatingOverlay *pPlayer, IFFPlayerAnimStateHelpers *pHelpers, LegAnimType_t legAnimType, bool bUseAimSequences );

	virtual bool	CanThePlayerMove();
	virtual float	GetCurrentMaxGroundSpeed();
	virtual Activity CalcMainActivity();
	virtual void	DebugShowAnimState( int iStartLine );
	virtual void	ClearAnimationLayers();

	virtual void	DoAnimationEvent( PlayerAnimEvent_t event );

	virtual void	ComputeSequences();
	
	virtual int		CalcAimLayerSequence( float *flCycle, float *flAimSequenceWeight, bool bForceIdle );
	virtual void	ClearAnimationState();
	
protected:

	int		CalcFireLayerSequence(PlayerAnimEvent_t event);
	void	ComputeFireSequence();

	int		CalcReloadLayerSequence();
	void	ComputeReloadSequence();

	const char* GetWeaponSuffix();

	bool HandleJumping();

	void UpdateLayerSequenceGeneric( int iLayer, bool &bEnabled, float &flCurCycle, int &iSequence, bool bWaitAtEnd );

private:

	bool	m_bJumping;				// Set on a jump event.
	bool	m_bFirstJumpFrame;		// Is this the first frame of the jump
	float	m_flJumpStartTime;		// To keep track of when this jump started

	bool	m_bReloading;			// Set on a reload event
	int		m_iReloadSequence;		// Sequence number
	float	m_flReloadCycle;		// Where in the sequence are we
	
	bool	m_bFiring;				// Set on any fire layer animation
	int		m_iFireSequence;		// Sequence number
	float	m_flFireCycle;			// Where in the sequence are we
	
	// Easy access to some player information
	IFFPlayerAnimStateHelpers *m_pHelpers;
};


IFFPlayerAnimState* CreatePlayerAnimState( CBaseAnimatingOverlay *pEntity, IFFPlayerAnimStateHelpers *pHelpers, LegAnimType_t legAnimType, bool bUseAimSequences )
{
	CFFPlayerAnimState *pRet = new CFFPlayerAnimState;
	pRet->InitFF( pEntity, pHelpers, legAnimType, bUseAimSequences );
	return pRet;
}

// ------------------------------------------------------------------------------------------------ //
// CFFPlayerAnimState implementation.
// ------------------------------------------------------------------------------------------------ //

CFFPlayerAnimState::CFFPlayerAnimState()
{
	m_pOuter = NULL;
	m_bReloading = false;
}


void CFFPlayerAnimState::InitFF( CBaseAnimatingOverlay *pEntity, IFFPlayerAnimStateHelpers *pHelpers, LegAnimType_t legAnimType, bool bUseAimSequences )
{
	CModAnimConfig config;
	config.m_flMaxBodyYawDegrees = ANGLE_BEFORE_TURN_LEGS;
	config.m_LegAnimType = legAnimType;
	config.m_bUseAimSequences = bUseAimSequences;

	m_pHelpers = pHelpers;

	BaseClass::Init( pEntity, config );
}


void CFFPlayerAnimState::ClearAnimationState()
{
	m_bJumping = false;
	m_bFiring = false;
	m_bReloading = false;
	
	BaseClass::ClearAnimationState();
}


void CFFPlayerAnimState::DoAnimationEvent( PlayerAnimEvent_t event )
{
	if ( event == PLAYERANIMEVENT_FIRE_GUN_PRIMARY || 
		 event == PLAYERANIMEVENT_FIRE_GUN_SECONDARY )
	{
		// Regardless of what we're doing in the fire layer, restart it.
		m_flFireCycle = 0;
		m_iFireSequence = CalcFireLayerSequence( event );
		m_bFiring = m_iFireSequence != -1;
	}
	else if ( event == PLAYERANIMEVENT_JUMP )
	{
		// Play the jump animation.
		m_bJumping = true;
		m_bFirstJumpFrame = true;
		m_flJumpStartTime = gpGlobals->curtime;
	}
	else if ( event == PLAYERANIMEVENT_RELOAD )
	{
		m_iReloadSequence = CalcReloadLayerSequence();
		if ( m_iReloadSequence != -1 )
		{
			m_bReloading = true;
			m_flReloadCycle = 0;
		}
	}
	else
	{
		Assert( !"CFFPlayerAnimState::DoAnimationEvent" );
	}
}

int CFFPlayerAnimState::CalcReloadLayerSequence()
{
	const char *pSuffix = GetWeaponSuffix();
	if ( !pSuffix )
		return -1;

	CFFWeaponBase *pWeapon = m_pHelpers->FFAnim_GetActiveWeapon();
	if ( !pWeapon )
		return -1;

	// First, look for reload_<weapon name>.
	char szName[512];
	Q_snprintf( szName, sizeof( szName ), "reload_%s", pSuffix );
	int iReloadSequence = m_pOuter->LookupSequence( szName );
	if ( iReloadSequence != -1 )
		return iReloadSequence;

	//FFTODO
/*
	// Ok, look for generic categories.. pistol, shotgun, rifle, etc.
	if ( pWeapon->GetFFWpnData().m_WeaponType == WEAPONTYPE_PISTOL )
	{
		Q_snprintf( szName, sizeof( szName ), "reload_pistol" );
		iReloadSequence = m_pOuter->LookupSequence( szName );
		if ( iReloadSequence != -1 )
			return iReloadSequence;
	}
	*/
			
	// Fall back to reload_m4.
	iReloadSequence = CalcSequenceIndex( "reload_m4" );
	if ( iReloadSequence > 0 )
		return iReloadSequence;

	return -1;
}


#ifdef CLIENT_DLL
	void CFFPlayerAnimState::UpdateLayerSequenceGeneric( int iLayer, bool &bEnabled, float &flCurCycle, int &iSequence, bool bWaitAtEnd )
	{
		if ( !bEnabled )
			return;

		// Increment the fire sequence's cycle.
		flCurCycle += m_pOuter->GetSequenceCycleRate( iSequence ) * gpGlobals->frametime;
		if ( flCurCycle > 1 )
		{
			if ( bWaitAtEnd )
			{
				flCurCycle = 1;
			}
			else
			{
				// Not firing anymore.
				bEnabled = false;
				iSequence = 0;
				return;
			}
		}

		// Now dump the state into its animation layer.
		C_AnimationLayer *pLayer = m_pOuter->GetAnimOverlay( iLayer );

		pLayer->flCycle = flCurCycle;
		pLayer->nSequence = iSequence;

		pLayer->flPlaybackrate = 1.0;
		pLayer->flWeight = 1.0f;
		pLayer->nOrder = iLayer;
	}
#endif

void CFFPlayerAnimState::ComputeReloadSequence()
{
#ifdef CLIENT_DLL
	UpdateLayerSequenceGeneric( RELOADSEQUENCE_LAYER, m_bReloading, m_flReloadCycle, m_iReloadSequence, false );
#else
	// Server doesn't bother with different fire sequences.
#endif
}

int CFFPlayerAnimState::CalcAimLayerSequence( float *flCycle, float *flAimSequenceWeight, bool bForceIdle )
{
	const char *pSuffix = GetWeaponSuffix();
	if ( !pSuffix )
		return 0;

	if ( bForceIdle )
	{
		switch ( GetCurrentMainSequenceActivity() )
		{
			case ACT_CROUCHIDLE:
				return CalcSequenceIndex( "%s%s", DEFAULT_CROUCH_IDLE_NAME, pSuffix );

			default:
				return CalcSequenceIndex( "%s%s", DEFAULT_IDLE_NAME, pSuffix );
		}
	}
	else
	{
		switch ( GetCurrentMainSequenceActivity() )
		{
			case ACT_HL2MP_JUMP_SLAM:
				return CalcSequenceIndex( "%s%s", DEFAULT_SLIDE_NAME, pSuffix );

			case ACT_RUN:
				return CalcSequenceIndex( "%s%s", DEFAULT_RUN_NAME, pSuffix );

			case ACT_WALK:
			case ACT_RUNTOIDLE:
			case ACT_IDLETORUN:
				return CalcSequenceIndex( "%s%s", DEFAULT_WALK_NAME, pSuffix );

			case ACT_CROUCHIDLE:
				return CalcSequenceIndex( "%s%s", DEFAULT_CROUCH_IDLE_NAME, pSuffix );

			case ACT_RUN_CROUCH:
				return CalcSequenceIndex( "%s%s", DEFAULT_CROUCH_WALK_NAME, pSuffix );

			case ACT_IDLE:
			default:
				return CalcSequenceIndex( "%s%s", DEFAULT_IDLE_NAME, pSuffix );
		}
	}
}



const char* CFFPlayerAnimState::GetWeaponSuffix()
{
	// Figure out the weapon suffix.
	CFFWeaponBase *pWeapon = m_pHelpers->FFAnim_GetActiveWeapon();
	if ( !pWeapon )
		return 0;

	const char *pSuffix = pWeapon->GetFFWpnData().m_szAnimExtension;

	//Disguised? override.
#ifdef CLIENT_DLL
	CFFPlayer *pFFPlayer = ToFFPlayer(pWeapon->GetOwner());

	if(pFFPlayer && pFFPlayer->IsDisguised())
	{
		if(pFFPlayer->m_DisguisedWeapons[pFFPlayer->GetDisguisedClass()].szAnimExt[pWeapon->GetFFWpnData().iSlot][0] != NULL)
			pSuffix = pFFPlayer->m_DisguisedWeapons[pFFPlayer->GetDisguisedClass()].szAnimExt[pWeapon->GetFFWpnData().iSlot];
	}
#endif
	return pSuffix;
}



int CFFPlayerAnimState::CalcFireLayerSequence(PlayerAnimEvent_t event)
{
	// Figure out the weapon suffix.
	CFFWeaponBase *pWeapon = m_pHelpers->FFAnim_GetActiveWeapon();
	if ( !pWeapon )
		return 0;

	const char *pSuffix = GetWeaponSuffix();
	if ( !pSuffix )
		return 0;
		
	switch ( GetCurrentMainSequenceActivity() )
	{
		case ACT_PLAYER_RUN_FIRE:
		case ACT_RUN:
			return CalcSequenceIndex( "%s%s", DEFAULT_FIRE_RUN_NAME, pSuffix );

		case ACT_PLAYER_WALK_FIRE:
		case ACT_WALK:
			return CalcSequenceIndex( "%s%s", DEFAULT_FIRE_WALK_NAME, pSuffix );

		case ACT_PLAYER_CROUCH_FIRE:
		case ACT_CROUCHIDLE:
			return CalcSequenceIndex( "%s%s", DEFAULT_FIRE_CROUCH_NAME, pSuffix );

		case ACT_PLAYER_CROUCH_WALK_FIRE:
		case ACT_RUN_CROUCH:
			return CalcSequenceIndex( "%s%s", DEFAULT_FIRE_CROUCH_WALK_NAME, pSuffix );

		default:
		case ACT_PLAYER_IDLE_FIRE:
			return CalcSequenceIndex( "%s%s", DEFAULT_FIRE_IDLE_NAME, pSuffix );
	}
}


bool CFFPlayerAnimState::CanThePlayerMove()
{
	return m_pHelpers->FFAnim_CanMove();
}


float CFFPlayerAnimState::GetCurrentMaxGroundSpeed()
{
	Activity currentActivity = 	m_pOuter->GetSequenceActivity( m_pOuter->GetSequence() );
	if ( currentActivity == ACT_WALK || currentActivity == ACT_IDLE )
		return ANIM_TOPSPEED_WALK;
	else if ( currentActivity == ACT_RUN )
		return ANIM_TOPSPEED_RUN;
	else if ( currentActivity == ACT_RUN_CROUCH )
		return ANIM_TOPSPEED_RUN_CROUCH;
	else
		return 0;
}


bool CFFPlayerAnimState::HandleJumping()
{
	if ( m_bJumping )
	{
		if ( m_bFirstJumpFrame )
		{
			m_bFirstJumpFrame = false;
			RestartMainSequence();	// Reset the animation.
		}

		// Don't check if he's on the ground for a sec.. sometimes the client still has the
		// on-ground flag set right when the message comes in.
		if ( gpGlobals->curtime - m_flJumpStartTime > 0.2f )
		{
			if ( m_pOuter->GetFlags() & FL_ONGROUND )
			{
				m_bJumping = false;
				RestartMainSequence();	// Reset the animation.
			}
		}
	}

	// Are we still jumping? If so, keep playing the jump animation.
	return m_bJumping;
}


Activity CFFPlayerAnimState::CalcMainActivity()
{
	float flOuterSpeed = GetOuterXYSpeed();

	if ( HandleJumping() )
	{
		return ACT_HOP;
	}
	else
	{
		Activity idealActivity = ACT_IDLE;

		if ( m_pOuter->GetFlags() & FL_DUCKING )
		{
			if ( flOuterSpeed > MOVING_MINIMUM_SPEED )
				idealActivity = ACT_RUN_CROUCH;
			else
				idealActivity = ACT_CROUCHIDLE;
		}
		else
		{
			if ( flOuterSpeed > MOVING_MINIMUM_SPEED )
			{
				if ( flOuterSpeed > ARBITRARY_RUN_SPEED )
					idealActivity = ACT_RUN;
				else
					idealActivity = ACT_WALK;
				
				/* commenting this out cause it borks the animations - fryguy
				// --> Mirv: Slide anim test
				CFFPlayer *player = dynamic_cast< CFFPlayer* >(m_pOuter);

				if( ( player->m_nButtons & IN_FORWARD ) == FALSE )
					idealActivity = ACT_HL2MP_JUMP_SLAM;
				// <-- Mirv: Slide anim test
				*/
			}
			else
			{
				idealActivity = ACT_IDLE;
			}
		}

		return idealActivity;
	}
}


void CFFPlayerAnimState::DebugShowAnimState( int iStartLine )
{
#ifdef CLIENT_DLL
	engine->Con_NPrintf( iStartLine++, "fire  : %s, cycle: %.2f\n", m_bFiring ? GetSequenceName( m_pOuter->GetModelPtr(), m_iFireSequence ) : "[not firing]", m_flFireCycle );
	engine->Con_NPrintf( iStartLine++, "reload: %s, cycle: %.2f\n", m_bReloading ? GetSequenceName( m_pOuter->GetModelPtr(), m_iReloadSequence ) : "[not reloading]", m_flReloadCycle );
	BaseClass::DebugShowAnimState( iStartLine );
#endif
}


void CFFPlayerAnimState::ComputeSequences()
{
	BaseClass::ComputeSequences();

	ComputeFireSequence();
	ComputeReloadSequence();
}


void CFFPlayerAnimState::ClearAnimationLayers()
{
	if ( !m_pOuter )
		return;

	m_pOuter->SetNumAnimOverlays( NUM_LAYERS_WANTED );
	for ( int i=0; i < m_pOuter->GetNumAnimOverlays(); i++ )
	{
		m_pOuter->GetAnimOverlay( i )->SetOrder( CBaseAnimatingOverlay::MAX_OVERLAYS );
	}
}


void CFFPlayerAnimState::ComputeFireSequence()
{
#ifdef CLIENT_DLL
	UpdateLayerSequenceGeneric( FIRESEQUENCE_LAYER, m_bFiring, m_flFireCycle, m_iFireSequence, false );
#else
	// Server doesn't bother with different fire sequences.
#endif
}
