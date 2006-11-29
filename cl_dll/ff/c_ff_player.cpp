//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "input.h"
#include "c_ff_player.h"
#include "ff_weapon_base.h"
#include "ff_playerclass_parse.h"
#include "c_basetempentity.h"
#include "ff_buildableobjects_shared.h"
#include "ff_utils.h"
#include "engine/IEngineSound.h"
#include "in_buttons.h"
#include "filesystem.h"

#include "materialsystem/IMaterialSystem.h"
#include "materialsystem/IMesh.h"
#include "ClientEffectPrecacheSystem.h"

#include "iviewrender_beams.h"
#include "r_efx.h"
#include "dlight.h"
#include "beamdraw.h"
#include "fx.h"
#include "c_te_effect_dispatch.h"

#include "view.h"
#include "iviewrender.h"

#include "ff_hud_grenade1timer.h"
#include "ff_hud_grenade2timer.h"

#include "ff_gamerules.h"
#include "ff_vieweffects.h"
#include "c_fire_smoke.h"
#include "c_playerresource.h"

#if defined( CFFPlayer )
	#undef CFFPlayer
#endif

extern CHudGrenade1Timer *g_pGrenade1Timer;
extern CHudGrenade2Timer *g_pGrenade2Timer;

#include "c_gib.h"

#include "c_ff_timers.h"
#include "vguicenterprint.h"

#include "ff_fx_bloodstream.h"

// --> Mirv: Conc stuff
static ConVar horiz_speed( "ffdev_concuss_hspeed", "2.0", 0, "Horizontal speed" );
static ConVar horiz_mag( "ffdev_concuss_hmag", "2.0", 0, "Horizontal magnitude" );
static ConVar vert_speed( "ffdev_concuss_vspeed", "1.0", 0, "Vertical speed" );
static ConVar vert_mag( "ffdev_concuss_vmag", "2.0", 0, "Vertical magnitude" );
static ConVar conc_test( "ffdev_concuss_test", "0", 0, "Show conced decals" );
// <-- Mirv: Conc stuff

static ConVar render_mode( "ffdev_rendermode", "0", FCVAR_CLIENTDLL );

static ConVar decap_test("ffdev_decaptest", "0");
static ConVar gibcount("cl_gibcount", "6");

static ConVar cl_spawnweapon("cl_spawnslot", "0", FCVAR_ARCHIVE, "Weapon slot to spawn with");

ConVar r_selfshadows( "r_selfshadows", "0", FCVAR_CLIENTDLL, "Toggles player & player carried objects' shadows", true, 0, true, 1 );
static ConVar cl_classautokill( "cl_classautokill", "0", FCVAR_USERINFO | FCVAR_ARCHIVE, "Change class instantly");

static char g_szTimerFile[MAX_PATH];
void TimerChange_Callback(ConVar *var, char const *pOldString);
ConVar cl_timerwav("cl_grenadetimer", "default", FCVAR_ARCHIVE, "Timer file to use", TimerChange_Callback);

// Get around the ambiguous symbol problem
extern IFileSystem **pFilesystem;

// #0000331: impulse 81 not working (weapon_cubemap)
#include "../c_weapon__stubs.h"
#include "ff_weapon_base.h"

STUB_WEAPON_CLASS( weapon_cubemap, WeaponCubemap, C_BaseCombatWeapon );

CLIENTEFFECT_REGISTER_BEGIN( PrecacheSpySprite )
CLIENTEFFECT_MATERIAL( "sprites/ff_sprite_spy" )
CLIENTEFFECT_REGISTER_END()

CLIENTEFFECT_REGISTER_BEGIN( PrecacheSaveMeSprite )
CLIENTEFFECT_MATERIAL( "sprites/ff_sprite_saveme" )
CLIENTEFFECT_REGISTER_END()

CLIENTEFFECT_REGISTER_BEGIN( PrecacheEngyMeSprite )
CLIENTEFFECT_MATERIAL( "sprites/ff_sprite_engyme" )
CLIENTEFFECT_REGISTER_END()

CLIENTEFFECT_REGISTER_BEGIN( PrecacheTeamMate )
CLIENTEFFECT_MATERIAL( "sprites/ff_sprite_teammate" )
CLIENTEFFECT_REGISTER_END()

CLIENTEFFECT_REGISTER_BEGIN( PrecachePlayerCloakMaterial )
CLIENTEFFECT_MATERIAL( FF_CLOAK_MATERIAL )
CLIENTEFFECT_REGISTER_END()

bool g_StealMouseForAimSentry = false;
void SetStealMouseForAimSentry( bool bValue )
{
	g_StealMouseForAimSentry = bValue;
}
bool CanStealMouseForAimSentry( void )
{
	return g_StealMouseForAimSentry;
}

//bool g_StealMouseForCloak = false;
//void SetStealMouseForCloak( bool bValue )
//{
//	g_StealMouseForCloak = bValue;
//}
//bool CanStealMouseForCloak( void )
//{
//	return g_StealMouseForCloak;
//}

void OnTimerExpired(C_FFTimer *pTimer)
{
	string name = pTimer->GetTimerName();
	//DevMsg("OnTimerExpired(%s)\n",name.c_str());
	char buf[256];
	sprintf(buf,"OnTimerExpired(%s)\n",name.c_str());
	internalCenterPrint->SetTextColor( 255, 255, 255, 255 );
	internalCenterPrint->Print( buf );
}

// --> Mirv: Toggle grenades (requested by defrag)
void CC_ToggleOne()
{
	if (!engine->IsConnected() || !engine->IsInGame())
		return;

	C_FFPlayer *pLocalPlayer = C_FFPlayer::GetLocalFFPlayer();

	if (pLocalPlayer->m_iGrenadeState != 0)
		CC_ThrowGren();
	else
		CC_PrimeOne();
}

void CC_ToggleTwo()
{
	if (!engine->IsConnected() || !engine->IsInGame())
		return;

	C_FFPlayer *pLocalPlayer = C_FFPlayer::GetLocalFFPlayer();

	if (pLocalPlayer->m_iGrenadeState != 0)
		CC_ThrowGren();
	else
		CC_PrimeTwo();
}
// <-- Mirv: Toggle grenades (requested by defrag)

void CC_PrimeOne( void )
{
	if(!engine->IsConnected() || !engine->IsInGame())
		return;

	C_FFPlayer *pLocalPlayer = C_FFPlayer::GetLocalFFPlayer();

	// Don't want timers going when frozen
	if( pLocalPlayer->GetFlags() & FL_FROZEN )
		return;

	if( pLocalPlayer->IsBuilding() )
		return;

	// Bug #0000176: Sniper gren2 shouldn't trigger timer.wav
	// Bug #0000064: Civilian has primary & secondary grenade.
	if(pLocalPlayer->m_iPrimary <= 0)
	{
		//DevMsg("[Grenades] You are out of primary grenades!\n");
		return;
	}

	// Bug #0000169: Grenade timer is played when player is dead and primes a grenade
	if (!pLocalPlayer->IsAlive() || pLocalPlayer->GetTeamNumber() < TEAM_BLUE)
		return;

	// Bug #0000170: Grenade timer plays on gren2 command if the player is already priming a gren1
	// 0000818: Grenade timer not playing on second of double primes
	if ((pLocalPlayer->m_iGrenadeState != FF_GREN_NONE) &&
		(pLocalPlayer->m_flLastServerPrimeTime >= pLocalPlayer->m_flServerPrimeTime))
		return;

	// Bug #0000366: Spy's cloaking & grenade quirks
	// Spy shouldn't be able to prime grenades when feigned
	//if (pLocalPlayer->GetEffects() & EF_NODRAW)
	if( pLocalPlayer->IsCloaked() )
		return;

	// Make sure we can't insta-prime on the client either
	// This can be anything really so long as it's less than the real delay
	// This should be okay up to about ~400ms for the moment
	if (engine->Time() < pLocalPlayer->m_flPrimeTime + 0.4f)
		return;

	// 0000818: Grenade timer not playing on second of double primes
	pLocalPlayer->m_flLastServerPrimeTime = pLocalPlayer->m_flServerPrimeTime;

	pLocalPlayer->m_flPrimeTime = engine->Time();

/*	C_FFTimer *pTimer = g_FFTimers.Create("PrimeGren", 4.0f);
	if (pTimer)
	{
		pTimer->m_bRemoveWhenExpired = true;
		pTimer->StartTimer();				
	}*/

	//pLocalPlayer->EmitSound( "Grenade.Timer" );

	CPASAttenuationFilter filter(pLocalPlayer, g_szTimerFile);

	EmitSound_t params;
	params.m_pSoundName = g_szTimerFile;
	params.m_flSoundTime = 0.0f;
	params.m_pflSoundDuration = NULL;
	params.m_bWarnOnDirectWaveReference = false;

	pLocalPlayer->EmitSound(filter, pLocalPlayer->entindex(), params);

	Assert (g_pGrenade1Timer);
	g_pGrenade1Timer->SetTimer(4.0f);
}

void CC_PrimeTwo( void )
{
	if(!engine->IsConnected() || !engine->IsInGame())
		return;

	C_FFPlayer *pLocalPlayer = C_FFPlayer::GetLocalFFPlayer();

	// Don't want timers going when frozen
	if( pLocalPlayer->GetFlags() & FL_FROZEN )
		return;

	if( pLocalPlayer->IsBuilding() )
		return;

	// Bug #0000176: Sniper gren2 shouldn't trigger timer.wav
	// Bug #0000064: Civilian has primary & secondary grenade.
	if(pLocalPlayer->m_iSecondary <= 0)
	{
		//DevMsg("[Grenades] You are out of secondary grenades!\n");
		return;
	}

	// Bug #0000169: Grenade timer is played when player is dead and primes a grenade
	if (!pLocalPlayer->IsAlive() || pLocalPlayer->GetTeamNumber() < TEAM_BLUE)
		return;

	// Bug #0000170: Grenade timer plays on gren2 command if the player is already priming a gren1
	// 0000818: Grenade timer not playing on second of double primes
	if ((pLocalPlayer->m_iGrenadeState != FF_GREN_NONE) &&
		(pLocalPlayer->m_flLastServerPrimeTime >= pLocalPlayer->m_flServerPrimeTime))
		return;

	// Bug #0000366: Spy's cloaking & grenade quirks
	// Spy shouldn't be able to prime grenades when feigned
	//if (pLocalPlayer->GetEffects() & EF_NODRAW)
	if( pLocalPlayer->IsCloaked() )
		return;

	// Make sure we can't insta-prime on the client either
	// This can be anything really so long as it's less than the real delay
	// This should be okay up to about ~400ms for the moment
	if (engine->Time() < pLocalPlayer->m_flPrimeTime + 0.4f)
		return;

	// 0000818: Grenade timer not playing on second of double primes
	pLocalPlayer->m_flLastServerPrimeTime = pLocalPlayer->m_flServerPrimeTime;

	pLocalPlayer->m_flPrimeTime = engine->Time();

	/*C_FFTimer *pTimer = g_FFTimers.Create("PrimeGren", 4.0f);
	if (pTimer)
	{
		pTimer->m_bRemoveWhenExpired = true;
		pTimer->StartTimer();				
	}*/
	
	//pLocalPlayer->EmitSound( "Grenade.Timer" );

	CPASAttenuationFilter filter(pLocalPlayer, g_szTimerFile);

	EmitSound_t params;
	params.m_pSoundName = g_szTimerFile;
	params.m_flSoundTime = 0.0f;
	params.m_pflSoundDuration = NULL;
	params.m_bWarnOnDirectWaveReference = false;

	pLocalPlayer->EmitSound(filter, pLocalPlayer->entindex(), params);

	Assert (g_pGrenade2Timer);
	g_pGrenade2Timer->SetTimer(4.0f);
}
void CC_ThrowGren( void )
{
	if(!engine->IsConnected() || !engine->IsInGame())
		return;

	C_FFPlayer *pLocalPlayer = C_FFPlayer::GetLocalFFPlayer();
	if(
		((pLocalPlayer->m_iGrenadeState == FF_GREN_PRIMEONE) && (pLocalPlayer->m_iPrimary == 0))
		||
		((pLocalPlayer->m_iGrenadeState == FF_GREN_PRIMETWO) && (pLocalPlayer->m_iSecondary == 0))
		)
	{
		return;
	}
}
void CC_TestTimers( void )
{
	//DevMsg("[L0ki] CC_TestTimers\n");
	if(!engine->IsConnected() || !engine->IsInGame())
	{
		//DevMsg("[L0ki] \tNOT connected or NOT active!\n");
		return;
	}

	C_FFTimer *pTimer = g_FFTimers.Create("ClientTimer0", 3.0f);
	if(pTimer)
	{
		pTimer->SetExpiredCallback(OnTimerExpired, true);
		pTimer->StartTimer();
	}
}

ConCommand testtimers("cc_test_timers",CC_TestTimers,"Tests the basic timer classes.");

void CC_SpyCloak( void )
{
	if( !engine->IsConnected() || !engine->IsInGame() )
		return;

	C_FFPlayer *pLocalPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pLocalPlayer )
		return;

	if( !pLocalPlayer->IsAlive() )
		return;
	
	if( pLocalPlayer->GetClassSlot() != CLASS_SPY )
		return;

	pLocalPlayer->Command_SpyCloak();
}

void CC_SpySilentCloak( void )
{
	if( !engine->IsConnected() || !engine->IsInGame() )
		return;

	C_FFPlayer *pLocalPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pLocalPlayer )
		return;

	if( !pLocalPlayer->IsAlive() )
		return;

	if( pLocalPlayer->GetClassSlot() != CLASS_SPY )
		return;

	pLocalPlayer->Command_SpySilentCloak();
}

#define FF_PLAYER_MODEL "models/player/terror.mdl"

// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //

class C_TEPlayerAnimEvent : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEPlayerAnimEvent, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

	virtual void PostDataUpdate( DataUpdateType_t updateType )
	{
		// Create the effect.
		C_FFPlayer *pPlayer = dynamic_cast< C_FFPlayer* >( m_hPlayer.Get() );
		if ( pPlayer && !pPlayer->IsDormant() )
		{
			pPlayer->DoAnimationEvent( (PlayerAnimEvent_t)m_iEvent.Get() );
		}	
	}

public:
	CNetworkHandle( CBasePlayer, m_hPlayer );
	CNetworkVar( int, m_iEvent );
};

IMPLEMENT_CLIENTCLASS_EVENT( C_TEPlayerAnimEvent, DT_TEPlayerAnimEvent, CTEPlayerAnimEvent );

BEGIN_RECV_TABLE_NOBASE( C_TEPlayerAnimEvent, DT_TEPlayerAnimEvent )
	RecvPropEHandle( RECVINFO( m_hPlayer ) ),
	RecvPropInt( RECVINFO( m_iEvent ) )
END_RECV_TABLE()

void RecvProxy_PrimeTime( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	// Unpack the data.
	if(!engine->IsConnected() || !engine->IsInGame())
	{
		return;
	}
	C_FFPlayer *pLocalPlayer = C_FFPlayer::GetLocalFFPlayer();
	if(pLocalPlayer)
	{
		pLocalPlayer->m_flServerPrimeTime = pData->m_Value.m_Float;
		if(pLocalPlayer->m_flServerPrimeTime != 0.0f)
			pLocalPlayer->m_flLatency = engine->Time() - pLocalPlayer->m_flPrimeTime;
	}
}

BEGIN_RECV_TABLE_NOBASE( C_FFPlayer, DT_FFLocalPlayerExclusive )

#ifdef EXTRA_LOCAL_ORIGIN_ACCURACY
	RecvPropVector(RECVINFO_NAME(m_vecNetworkOrigin, m_vecOrigin)),
#endif

	RecvPropInt( RECVINFO( m_iShotsFired ) ),

	// Beg: Added by Mulchman for building objects and such
	RecvPropEHandle( RECVINFO( m_hDispenser ) ),
	RecvPropEHandle( RECVINFO( m_hSentryGun ) ),
	RecvPropEHandle( RECVINFO( m_hDetpack ) ),
	RecvPropInt( RECVINFO( m_bBuilding ) ),
	RecvPropInt( RECVINFO( m_iCurBuild ) ),
	// End: Added by Mulchman for building objects and such

	// ---> added by billdoor
	RecvPropFloat(RECVINFO(m_flArmorType)),

	RecvPropInt(RECVINFO(m_iSkiState)),
	// ---> end

	// Beg: Added by L0ki - Grenade related
	RecvPropInt( RECVINFO( m_iPrimary ) ),
	RecvPropInt( RECVINFO( m_iSecondary ) ),
	RecvPropInt( RECVINFO( m_iGrenadeState ) ),
	RecvPropFloat( RECVINFO( m_flServerPrimeTime ), 0, RecvProxy_PrimeTime ),
	// End: Added by L0ki

	// Beg: Added by FryGuy - Status Effects related
	RecvPropFloat( RECVINFO( m_flNextBurnTick ) ),
	RecvPropInt( RECVINFO( m_iBurnTicks ) ),
	RecvPropFloat( RECVINFO( m_flBurningDamage ) ),
	// End: Added by FryGuy

	RecvPropEHandle( RECVINFO( m_hNextMapGuide ) ),
	RecvPropEHandle( RECVINFO( m_hLastMapGuide ) ),
	RecvPropFloat( RECVINFO( m_flNextMapGuideTime ) ),

	RecvPropFloat(RECVINFO(m_flConcTime)),

	RecvPropFloat(RECVINFO(m_flSpeedModifier)),

	// Radiotag information the local client needs to know
	RecvPropEHandle( RECVINFO( m_hRadioTagData ) ),
	RecvPropInt( RECVINFO( m_bCloakable ) ),
	RecvPropInt( RECVINFO( m_bDisguisable ) ),
END_RECV_TABLE( )

#ifdef EXTRA_LOCAL_ORIGIN_ACCURACY
BEGIN_RECV_TABLE_NOBASE(C_FFPlayer, DT_NonLocalOrigin)
	RecvPropVector(RECVINFO_NAME(m_vecNetworkOrigin, m_vecOrigin)),
END_RECV_TABLE()
#endif

IMPLEMENT_CLIENTCLASS_DT( C_FFPlayer, DT_FFPlayer, CFFPlayer )
	RecvPropDataTable( "fflocaldata", 0, 0, &REFERENCE_RECV_TABLE(DT_FFLocalPlayerExclusive) ),

#ifdef EXTRA_LOCAL_ORIGIN_ACCURACY
	RecvPropDataTable("fforigin", 0, 0, &REFERENCE_RECV_TABLE(DT_NonLocalOrigin)),
#endif

	RecvPropFloat( RECVINFO( m_angEyeAngles[0] ) ),
	RecvPropFloat( RECVINFO( m_angEyeAngles[1] ) ),
	RecvPropEHandle( RECVINFO( m_hRagdoll ) ),

	RecvPropInt( RECVINFO( m_iClassStatus ) ),	
	RecvPropInt( RECVINFO( m_iSpyDisguise ) ),

	RecvPropInt(RECVINFO(m_iSpawnInterpCounter)),
	
	RecvPropInt( RECVINFO( m_iSaveMe ) ),
	RecvPropInt( RECVINFO( m_iEngyMe ) ),
	RecvPropInt( RECVINFO( m_bInfected ) ),
	RecvPropInt( RECVINFO( m_bImmune ) ),
	RecvPropInt( RECVINFO( m_iCloaked ) ),
	RecvPropFloat( RECVINFO( m_flCloakSpeed ) ),
END_RECV_TABLE( )

BEGIN_PREDICTION_DATA( C_FFPlayer )
	DEFINE_PRED_FIELD( m_flCycle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_iShotsFired, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),   
END_PREDICTION_DATA()

class C_FFRagdoll : public C_BaseAnimatingOverlay
{
public:
	DECLARE_CLASS( C_FFRagdoll, C_BaseAnimatingOverlay );
	DECLARE_CLIENTCLASS();

	C_FFRagdoll();
	~C_FFRagdoll();

	virtual void OnDataChanged( DataUpdateType_t type );

	// HACKHACK: Fix to allow laser beam to shine off ragdolls
	virtual CollideType_t ShouldCollide()
	{
		return ENTITY_SHOULD_COLLIDE_RESPOND;
	}

	virtual BloodColor() { return BLOOD_COLOR_RED; }

	int GetPlayerEntIndex() const;
	IRagdoll* GetIRagdoll() const;

	void ImpactTrace( trace_t *pTrace, int iDamageType, char *pCustomImpactName );

	void ClientThink();

private:

	C_FFRagdoll( const C_FFRagdoll & ) {}

	void Interp_Copy( C_BaseAnimatingOverlay *pSourceEntity );

	void CreateRagdoll();


private:

	EHANDLE	m_hPlayer;
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );

	CSmartPtr<CBloodStream>	m_pBloodStreamEmitter;

	int		m_fBodygroupState;
	int		m_nSkinIndex;
};


IMPLEMENT_CLIENTCLASS_DT_NOBASE( C_FFRagdoll, DT_FFRagdoll, CFFRagdoll )
	RecvPropVector( RECVINFO(m_vecRagdollOrigin) ),
	RecvPropEHandle( RECVINFO( m_hPlayer ) ),
	RecvPropInt( RECVINFO( m_nModelIndex ) ),
	RecvPropInt( RECVINFO(m_nForceBone) ),
	RecvPropVector( RECVINFO(m_vecForce) ),
	RecvPropVector( RECVINFO( m_vecRagdollVelocity ) ),

	RecvPropInt(RECVINFO(m_fBodygroupState)),
	RecvPropInt(RECVINFO(m_nSkinIndex)),
END_RECV_TABLE()


C_FFRagdoll::C_FFRagdoll()
{
	m_pBloodStreamEmitter = NULL;
}

C_FFRagdoll::~C_FFRagdoll()
{
	PhysCleanupFrictionSounds( this );
}

void C_FFRagdoll::Interp_Copy( C_BaseAnimatingOverlay *pSourceEntity )
{
	if ( !pSourceEntity )
		return;
	
	VarMapping_t *pSrc = pSourceEntity->GetVarMapping();
	VarMapping_t *pDest = GetVarMapping();
    	
	// Find all the VarMapEntry_t's that represent the same variable.
	for ( int i = 0; i < pDest->m_Entries.Count(); i++ )
	{
		VarMapEntry_t *pDestEntry = &pDest->m_Entries[i];
		for ( int j=0; j < pSrc->m_Entries.Count(); j++ )
		{
			VarMapEntry_t *pSrcEntry = &pSrc->m_Entries[j];
			if ( !Q_strcmp( pSrcEntry->watcher->GetDebugName(),
				pDestEntry->watcher->GetDebugName() ) )
			{
				pDestEntry->watcher->Copy( pSrcEntry->watcher );
				break;
			}
		}
	}
}

void C_FFRagdoll::ImpactTrace( trace_t *pTrace, int iDamageType, char *pCustomImpactName )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();

	if( !pPhysicsObject )
		return;

	// --> Mirv: [TODO] Return on impact to invisible bodygroup
	// This will need to wait until the #s of the hitgroups are finalised
	// <-- Mirv: [TODO] Return on impact to invisible bodygroup

	Vector dir = pTrace->endpos - pTrace->startpos;
	Vector hitpos;

	VectorMA(pTrace->startpos, pTrace->fraction, dir, hitpos);
	VectorNormalize(dir);

	UTIL_BloodDrips(pTrace->endpos, dir, BLOOD_COLOR_RED, 20);
	TraceBleed(20, dir, pTrace, DMG_BLAST);


	if ( iDamageType & DMG_BLAST )
	{
		dir *= 40000;  // adjust impact strength

		Vector vecVelocity, vecAngularVelocity;
		pPhysicsObject->CalculateVelocityOffset(dir, pTrace->endpos, &vecVelocity, &vecAngularVelocity);
		pPhysicsObject->AddVelocity(&vecVelocity, &vecAngularVelocity);
	}
	else
	{
		dir *= 4000;  // adjust impact strenght

		// apply force where we hit it
		pPhysicsObject->ApplyForceOffset( dir, hitpos );	
	}

	m_pRagdoll->ResetRagdollSleepAfterTime();
}


void C_FFRagdoll::CreateRagdoll()
{
	// First, initialize all our data. If we have the player's entity on our client,
	// then we can make ourselves start out exactly where the player is.
	C_FFPlayer *pPlayer = dynamic_cast< C_FFPlayer* >( m_hPlayer.Get() );

	if ( pPlayer && !pPlayer->IsDormant() )
	{
		// move my current model instance to the ragdoll's so decals are preserved.
		pPlayer->SnatchModelInstance( this );

		VarMapping_t *varMap = GetVarMapping();

		// Copy all the interpolated vars from the player entity.
		// The entity uses the interpolated history to get bone velocity.
		bool bRemotePlayer = (pPlayer != C_BasePlayer::GetLocalPlayer());			
		if ( bRemotePlayer )
		{
			Interp_Copy( pPlayer );

			SetAbsAngles( pPlayer->GetRenderAngles() );
			GetRotationInterpolator().Reset();

			m_flAnimTime = pPlayer->m_flAnimTime;
			SetSequence( pPlayer->GetSequence() );
			m_flPlaybackRate = pPlayer->GetPlaybackRate();
		}
		else
		{
			// This is the local player, so set them in a default
			// pose and slam their velocity, angles and origin
			SetAbsOrigin( m_vecRagdollOrigin );

			SetAbsAngles( pPlayer->GetRenderAngles() );

			SetAbsVelocity( m_vecRagdollVelocity );

			int iSeq = LookupSequence( "walk_lower" );
			if ( iSeq == -1 )
			{
				// 7/3/2006 - Mulchman:
				// Commented out because people are tired of getting this assert!
				Warning( "[C_FFRagdoll :: CreateRagdoll] Missing sequence walk_lower!\n" );

				// Mulch: to start knowing what asserts are popping up for when testing stuff
				// AssertMsg( false, "missing sequence walk_lower" ); 
				//Assert( false );	// missing walk_lower?				
				iSeq = 0;
			}

			SetSequence( iSeq );	// walk_lower, basic pose
			SetCycle( 0.0 );

			Interp_Reset( varMap );
		}

		/*
		// Now set the corpse alight if needed
		if (pPlayer->GetEffectEntity())
		{
			IgniteRagdoll(pPlayer);
			SetNextClientThink(gpGlobals->curtime + 1.0f);

			// This is giving the CL_CopyExistingEntity error
#if 0
			C_EntityFlame *pFlame = (C_EntityFlame *) pPlayer->GetEffectEntity();

			// Now make sure we kill our own flame
			if (pFlame)
			{
				pFlame->Remove();
			}
#endif
		}
		*/

		CFFWeaponBase *pWeapon = pPlayer->GetActiveFFWeapon();

		// We can also spawn a valid weapon
		if (pWeapon && pWeapon->GetWeaponID() < FF_WEAPON_DEPLOYDISPENSER)
		{
			C_Gib *pGib = C_Gib::CreateClientsideGib(pWeapon->GetFFWpnData().szWorldModel, pWeapon->GetAbsOrigin(), GetAbsVelocity(), Vector(0, 0, 0), 10.0f);

			if (pGib)
			{
				pGib->SetAbsAngles(pWeapon->GetAbsAngles());
			}
		}
	}
	else
	{
		// overwrite network origin so later interpolation will
		// use this position
		SetNetworkOrigin( m_vecRagdollOrigin );

		SetAbsOrigin( m_vecRagdollOrigin );
		SetAbsVelocity( m_vecRagdollVelocity );

		Interp_Reset( GetVarMapping() );

	}

	SetModelIndex( m_nModelIndex );
	m_nSkin = m_nSkinIndex;

	// TEMP to test blood streams!!
	if (decap_test.GetBool())
	{
		m_fBodygroupState = 0xFFFFFFFF;
	}

	// Remove the correct parts of the body
	if (m_fBodygroupState & DECAP_HEAD)
		SetBodygroup(1, 1);
	if (m_fBodygroupState & DECAP_LEFT_ARM)
		SetBodygroup(2, 1);
	if (m_fBodygroupState & DECAP_LEFT_LEG)
		SetBodygroup(3, 1);
	if (m_fBodygroupState & DECAP_RIGHT_ARM)
		SetBodygroup(4, 1);
	if (m_fBodygroupState & DECAP_RIGHT_LEG)
		SetBodygroup(5, 1);

	// Turn it into a ragdoll.
	// Make us a ragdoll..
	m_nRenderFX = kRenderFxRagdoll;

	BecomeRagdollOnClient( false );

	// HACKHACK: Fix to allow laser beam to shine off ragdolls
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetCollisionGroup(COLLISION_GROUP_WEAPON);

	m_pBloodStreamEmitter = CBloodStream::Create(this, "BloodStream");
	m_pBloodStreamEmitter->SetDieTime(gpGlobals->curtime + 25.0f);
}


void C_FFRagdoll::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		CreateRagdoll();
	
		IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
		if( pPhysicsObject )
		{
			AngularImpulse aVelocity(0,0,0);
			Vector vecExaggeratedVelocity = 3 * m_vecRagdollVelocity;
			pPhysicsObject->AddVelocity( &vecExaggeratedVelocity, &aVelocity );
		}

		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

IRagdoll* C_FFRagdoll::GetIRagdoll() const
{
	return m_pRagdoll;
}

C_BaseAnimating * C_FFPlayer::BecomeRagdollOnClient( bool bCopyEntity )
{
	// Let the C_CSRagdoll entity do this.
	// m_builtRagdoll = true;
	return NULL;
}


IRagdoll* C_FFPlayer::GetRepresentativeRagdoll() const
{
	if ( m_hRagdoll.Get() )
	{
		C_FFRagdoll *pRagdoll = (C_FFRagdoll*)m_hRagdoll.Get();

		return pRagdoll->GetIRagdoll();
	}
	else
	{
		return NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Check to see if we've entered the water and remove any flames
//			if we have.
//-----------------------------------------------------------------------------
void C_FFRagdoll::ClientThink( void )
{
	/*
	if( UTIL_PointContents( GetAbsOrigin() ) & MASK_WATER )
	{
		C_BaseEntity *pEntity = GetEffectEntity();

		if( pEntity )
		{
			C_BaseEntity *pEffectEntity = pEntity->GetEffectEntity();
			if( pEffectEntity )
			{
				Warning( "[Effect Entity Valid!]\n" );
			}			
		}

		C_EntityFlame *pFlame = ( C_EntityFlame * )GetEffectEntity();
		if( pFlame )
		{
			// Just hide drawing it on our client
			pFlame->SetRenderColorA( 0 );

			// Stop playing its burning sound as well
			pFlame->EmitSound( "General.StopBurning" );

			return;
		}
	}

	SetNextClientThink(gpGlobals->curtime + 0.3f);
	*/
}

const unsigned char *GetEncryptionKey( void )
{
	return NULL;
}


C_FFPlayer::C_FFPlayer() : 
	m_iv_angEyeAngles( "C_FFPlayer::m_iv_angEyeAngles" )
{
	m_PlayerAnimState = CreatePlayerAnimState( this, this, LEGANIM_9WAY, true );

	// Default
	m_clrTeamColor = Color( 255, 255, 255, 255 );

	m_flLastServerPrimeTime = 0.0f;
	m_angEyeAngles.Init();
	AddVar( &m_angEyeAngles, &m_iv_angEyeAngles, LATCH_SIMULATION_VAR );

	m_bFirstSpawn = true;

	m_iLocalSkiState = 0;

	m_flJumpTime = m_flFallTime = 0;

	m_iSpawnInterpCounter = 0;

	// BEG: Added by Mulchman
	m_iSpyDisguise = 0; // start w/ no disguise
	// END: Added by Mulchman
	
	m_pOldActiveWeapon = NULL;

	m_flConcTime = 0;

	m_flSpeedModifier = 1.0f;
	
	m_iHallucinationIndex = 0;

	for( int i = 0; i < MAX_PLAYERS; i++ )
	{
		// -1 = not set
		m_hSpyTracking[ i ].m_iClass = -1;
		m_hSpyTracking[ i ].m_iTeam = -1;
	}

	m_pFlashlightBeam = NULL;

	m_flNextJumpTimeForDouble = 0;

	//VOOGRU: I'll have bad nightmares if I don't do this.
	memset(&m_DisguisedWeapons, 0, sizeof(m_DisguisedWeapons));

	//Loop through all classes.
	for (int i=1; i < 10; i++)
	{
		PLAYERCLASS_FILE_INFO_HANDLE PlayerClassInfo;

		if (!ReadPlayerClassDataFromFileForSlot((*pFilesystem), Class_IntToString(i), &PlayerClassInfo, GetEncryptionKey()))
			return;

		CFFPlayerClassInfo *pPlayerClassInfo = GetFilePlayerClassInfoFromHandle(PlayerClassInfo);

		//Get weapons for the class they disguised as.
		WEAPON_FILE_INFO_HANDLE	WeaponInfo;

		//Loop through weapons, find a weapon in the same slot as the one they are using, and set their current weapon model to that.
		for ( int j = 0; j < pPlayerClassInfo->m_iNumWeapons; j++ )
		{
			if ( ReadWeaponDataFromFileForSlot( (*pFilesystem), pPlayerClassInfo->m_aWeapons[j], &WeaponInfo, GetEncryptionKey() ) )
			{
				CFFWeaponInfo *pWeaponInfo = NULL;
#ifdef _DEBUG
				pWeaponInfo = dynamic_cast<CFFWeaponInfo *> (GetFileWeaponInfoFromHandle(WeaponInfo));
				Assert(pWeaponInfo);
#else
				pWeaponInfo = static_cast<CFFWeaponInfo *> (GetFileWeaponInfoFromHandle(WeaponInfo));
#endif

				if(pWeaponInfo)
				{
					int iPlayerClass = i;
					int iSlot = pWeaponInfo->iSlot;
					bool bDone = false;

					//Correct slots in a 1/2/3/4 layout, so spy can select all available weapons.
					switch(iPlayerClass)
					{
					case 1:
						if(iSlot == 3) 
							iSlot = 2;
						break;
					case 3:
						if(iSlot == 4) 
							iSlot = 3;
						break;
					case 4:
						if(iSlot == 4) 
							iSlot = 2;
						break;
					case 6:
						if(iSlot == 4) 
							iSlot = 3;
						break;
					case 7:
						if(iSlot == 3) 
							iSlot = 2;
						else if(iSlot == 4) 
							iSlot = 3;
						break;
					case 9:
						if(iSlot > 2) //use shotgun for everything after the shotgun.
						{
							Q_strncpy(m_DisguisedWeapons[iPlayerClass].szWeaponModel[iSlot], 
								m_DisguisedWeapons[iPlayerClass].szWeaponModel[2], 
								sizeof(m_DisguisedWeapons[iPlayerClass].szWeaponModel[iSlot]));

							Q_strncpy(m_DisguisedWeapons[iPlayerClass].szAnimExt[iSlot], 
								m_DisguisedWeapons[iPlayerClass].szAnimExt[2], 
								sizeof(m_DisguisedWeapons[iPlayerClass].szAnimExt[iSlot]));
							bDone = true;
						}
						break;
					}

					if(bDone)
						continue;

					Q_strncpy(m_DisguisedWeapons[iPlayerClass].szWeaponModel[iSlot], 
						pWeaponInfo->szWorldModel, 
						sizeof(m_DisguisedWeapons[iPlayerClass].szWeaponModel[iSlot]));

					Q_strncpy(m_DisguisedWeapons[iPlayerClass].szAnimExt[iSlot], 
						pWeaponInfo->m_szAnimExtension, 
						sizeof(m_DisguisedWeapons[iPlayerClass].szAnimExt[iSlot]));
				}
			}
		}
	}
}

C_FFPlayer::~C_FFPlayer()
{
	m_PlayerAnimState->Release();

	ReleaseFlashlight();
}

C_FFPlayer* C_FFPlayer::GetLocalFFPlayer()
{
	// Assert thrown from here sometimes, validate 
	// C_BasePlayer::GetLocalPlayer() first
	if( C_BasePlayer::GetLocalPlayer() )
        return ToFFPlayer( C_BasePlayer::GetLocalPlayer() );
	else
		return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFPlayer::PreThink( void )
{
	if ((m_flConcTime > gpGlobals->curtime) || (m_flConcTime < 0))
	{
		//Warning( "[prethink] conctime: %i\n", m_flConcTime );
		float flLength = GetClassSlot() == CLASS_MEDIC ? 7.5f : 15.0f;
		float flConcAmount = 15.0f;
		if( m_flConcTime > 0 )
			flConcAmount *= (m_flConcTime - gpGlobals->curtime) / flLength;

		if (IsAlive())
		{
			// Our conc angles, this is also quite slow for now
			m_angConced = QAngle( flConcAmount * vert_mag.GetFloat() * sin(vert_speed.GetFloat() * gpGlobals->curtime), flConcAmount * horiz_mag.GetFloat() * sin(horiz_speed.GetFloat() * gpGlobals->curtime), 0 );

			float flTotalAngle = BaseClass::EyeAngles().x + m_angConced.x;

			// We need to make sure we're not looking down further than 90 as this
			// makes the viewmodel glitchy. Therefore remove any excess from the 
			// conced angles.
			// We have to do this here rather than on the viewmodel because the
			// viewmodel also has to be locked so that you can't see below it which
			// would conflict with also locking it above a certain point. So we have to
			// remove it from the conc'ed angle itself.
			if (flTotalAngle > 90.0f)
			{
				m_angConced.x -= flTotalAngle - 90.0f;
			}
		}
		else
		{
			m_angConced = vec3_angle;
		}
	}

	// Do we need to do a class specific skill?
	if (m_afButtonPressed & IN_ATTACK2)
		ClassSpecificSkill();

	else if (m_afButtonReleased & IN_ATTACK2)
		ClassSpecificSkill_Post();

	//if (m_afButtonPressed & IN_RELOAD && !IsAlive())
	//	engine->ClientCmd("-reload");

	// New possible fix
	if( ::input && !IsAlive() )
		::input->ClearInputButton( IN_RELOAD );

	// Clear hallucinations if time
	if (m_iHallucinationIndex && m_flHallucinationFinish < gpGlobals->curtime)
	{
		m_iHallucinationIndex = 0;
	}

	BaseClass::PreThink();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFPlayer::PostThink( void )
{
	BaseClass::PostThink();
}

void C_FFPlayer::Precache()
{
	for (int i = 1; i <= 8; i++)
	{
		PrecacheModel(VarArgs("models/gibs/gib%d.mdl", i));
	}
}

extern void ClearStatusIcons();

//-----------------------------------------------------------------------------
// Purpose: Spawn
//-----------------------------------------------------------------------------
void C_FFPlayer::Spawn( void )
{
	// Okay, not calling the base spawn when this was created
	// was breaking a lot of stuff.
	if (m_bFirstSpawn)
	{
		BaseClass::Spawn();
		m_bFirstSpawn = false;
	}

	m_flNextCloak = 0.0f;

	// Reset this on spawn
	SetStealMouseForAimSentry( false );

	// Clean up some stuff
	ffvieweffects->Reset();	
	ClearStatusIcons();

	// Default
	m_clrTeamColor = Color( 255, 255, 255, 255 );

	// Set our team color
	if( g_PR && ( GetTeamNumber() >= TEAM_BLUE ) && ( GetTeamNumber() <= TEAM_GREEN ) )
		m_clrTeamColor = g_PR->GetTeamColor( GetTeamNumber() );

	// Reset pipebomb counter!
	GetPipebombCounter()->Reset();

	// Stop grenade 1 timers if they're playing
	if( g_pGrenade1Timer && ( m_iGrenadeState != FF_GREN_PRIMEONE ) )
	{
		// TODO: Stop sound
		if( g_pGrenade1Timer->ActiveTimer() )
			g_pGrenade1Timer->ResetTimer();
	}

	// Stop grenade 2 timers if they're playing
	if( g_pGrenade2Timer && ( m_iGrenadeState != FF_GREN_PRIMETWO ) )
	{
		// TODO: Stop sound
		if( g_pGrenade2Timer->ActiveTimer() )
			g_pGrenade2Timer->ResetTimer();
	}

	char szCommand[128];

	// Execute the player config
	if (GetClassSlot() > 0)
	{
		Q_snprintf(szCommand, 127, "exec %.10s.cfg", Class_IntToString(GetClassSlot()));
		engine->ClientCmd(szCommand);
	}

	int iSpawnWeapon = cl_spawnweapon.GetInt();

	// Automatically call the slot specified by cl_spawnslot
	if (iSpawnWeapon > 0 && iSpawnWeapon <= MAX_WEAPON_SLOTS)
	{
		SwapToWeaponSlot(iSpawnWeapon);
	}

	// Stop any looping sounds - this too hack-ish?
	// Mirv: Might be better to ensure this happens only for local player, all players
	// will have this function called when they are first created and this might 
	// therefore stop sounds at 'random' times
	if (IsLocalPlayer())
	{
		//enginesound->StopAllSounds( true );
	}
	ViewPunchReset();
}

//-----------------------------------------------------------------------------
// Purpose: Local player has died, clean up various things here
//-----------------------------------------------------------------------------
void C_FFPlayer::Death()
{
	ffvieweffects->Reset();
	ClearStatusIcons();

	// Reset pipebomb counter!
	GetPipebombCounter()->Reset();

	// Reset this to false on death
	SetStealMouseForAimSentry( false );
}

// Stomp any movement if we're in mapguide mode
void C_FFPlayer::CreateMove(float flInputSampleTime, CUserCmd *pCmd)
{
	// Mapguides
	if (GetTeamNumber() == TEAM_SPECTATOR && m_hNextMapGuide)
	{
		pCmd->buttons = 0;
		pCmd->forwardmove = 0;
		pCmd->sidemove = 0;
		pCmd->upmove = 0;
	}
	else
	{
		// NOTE: Removed this for geekfeststarter

		// This caught my eye... do we want this in or is it alright
		// to override this function and then not call the baseclass?
		// Certainly remove it and ignore me if it's not needed :P - Mulch
		//BaseClass::CreateMove( flInputSampleTime, pCmd );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_FFPlayer::DrawModel( int flags )
{
	C_FFPlayer *pPlayer = GetLocalFFPlayer();

	float flOffset = 0.0f;
	
	if( pPlayer && ( this != pPlayer ) )
	{
		// --------------------------------
		// Check for team mate
		// --------------------------------
		if( FFGameRules()->IsTeam1AlliedToTeam2( pPlayer->GetTeamNumber(), ( IsDisguised() ? GetDisguisedTeam() : GetTeamNumber() ) ) == GR_TEAMMATE )
		{
			IMaterial *pMaterial = materials->FindMaterial( "sprites/ff_sprite_teammate", TEXTURE_GROUP_OTHER );
			if( pMaterial )
			{
				materials->Bind( pMaterial );
				color32 c = { 255, 255, 255, 255 };
				DrawSprite( Vector( GetAbsOrigin().x, GetAbsOrigin().y, EyePosition().z + 16.0f ), 15.0f, 15.0f, c );

				// Increment offset
				flOffset += 16.0f;
			}
		}

		// --------------------------------
		// Check for "saveme"
		// --------------------------------
		if( IsInSaveMe() && ( FFGameRules()->IsTeam1AlliedToTeam2( pPlayer->GetTeamNumber(), GetTeamNumber() ) == GR_TEAMMATE ) )
		{
			IMaterial *pMaterial = materials->FindMaterial( "sprites/ff_sprite_saveme", TEXTURE_GROUP_OTHER );
			if( pMaterial )
			{
				materials->Bind( pMaterial );
				color32 c = { 255, 0, 0, 255 };
				DrawSprite( Vector( GetAbsOrigin().x, GetAbsOrigin().y, EyePosition().z + 16.0f ), 15.0f, 15.0f, c );

				// Increment offset
				flOffset += 16.0f;
			}
		}

		// --------------------------------
		// Check for "engyme"
		// --------------------------------
		if( IsInEngyMe() && ( FFGameRules()->IsTeam1AlliedToTeam2( pPlayer->GetTeamNumber(), GetTeamNumber() ) == GR_TEAMMATE ) )
		{
			IMaterial *pMaterial = materials->FindMaterial( "sprites/ff_sprite_engyme", TEXTURE_GROUP_OTHER );
			if( pMaterial )
			{
				materials->Bind( pMaterial );

				// The color is based on the players real team
				int iTeam = GetTeamNumber();						
				Color clr = Color( 255, 255, 255, 255 );

				if( g_PR )
					clr.SetColor( g_PR->GetTeamColor( iTeam ).r(), g_PR->GetTeamColor( iTeam ).g(), g_PR->GetTeamColor( iTeam ).b(), 255 );
				
				color32 c = { clr.r(), clr.g(), clr.b(), 255 };
				DrawSprite( Vector( GetAbsOrigin().x, GetAbsOrigin().y, EyePosition().z + 16.0f + flOffset ), 15.0f, 15.0f, c );

				// Increment offset
				flOffset += 16.0f;
			}
		}

		// --------------------------------
		// Check for friendly spies
		// --------------------------------
		if( IsDisguised() && IsAlive() )
		{
			// See if the spy is a teammate or ally
			if( FFGameRules()->IsTeam1AlliedToTeam2( pPlayer->GetTeamNumber(), GetTeamNumber() ) == GR_TEAMMATE )
			{
				// Now, is the spy disguised as an enemy?
				if( FFGameRules()->IsTeam1AlliedToTeam2( pPlayer->GetTeamNumber(), GetDisguisedTeam() ) == GR_NOTTEAMMATE )
				{
					// Thanks mirv!
					IMaterial *pMaterial = materials->FindMaterial( "sprites/ff_sprite_spy", TEXTURE_GROUP_OTHER );
					if( pMaterial )
					{
						materials->Bind( pMaterial );

						// The color is based on the spies' real team
						int iTeam = GetTeamNumber();						
						Color clr = Color( 255, 255, 255, 255 );

						if( g_PR )
							clr.SetColor( g_PR->GetTeamColor( iTeam ).r(), g_PR->GetTeamColor( iTeam ).g(), g_PR->GetTeamColor( iTeam ).b(), 255 );
						
						color32 c = { clr.r(), clr.g(), clr.b(), clr.a() };
						DrawSprite( Vector( GetAbsOrigin().x, GetAbsOrigin().y, EyePosition().z + 16.0f + flOffset ), 15.0f, 15.0f, c );
					}
				}
			}
		}
	}

	// If we're hallucinating, players intermittently get swapped.  But only for
	// enemy players because we don't want the teamkills
	C_FFPlayer *pLocalPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

	if (pLocalPlayer && pLocalPlayer->m_iHallucinationIndex && !IsLocalPlayer())
	{
		if (pLocalPlayer->GetTeamNumber() != GetTeamNumber())
		{
			int nSkin = entindex() + pLocalPlayer->m_iHallucinationIndex;

			// It doesn't really matter if this is actually odd or even, 
			// we just need to differentiate between half of them
			if (nSkin & 1)
			{
				nSkin = pLocalPlayer->GetTeamNumber() - TEAM_BLUE;
			}
			else
			{
				nSkin = GetTeamNumber() - TEAM_BLUE;
			}

			// This player's skin needs changing
			if (m_nSkin != nSkin)
			{
				m_nSkin = nSkin;
			}
		}
	}
	else
	{
		int nRealSkin = (IsDisguised() ? GetDisguisedTeam() : GetTeamNumber()) - TEAM_BLUE;

		// Make sure hallucinations are reset!
		if (m_nSkin != nRealSkin)
		{
			m_nSkin = nRealSkin;
		}
	}

	// --------------------------------
	// Draw cloaked players special (and after we
	// might have changed skins from hallucinating)
	// --------------------------------
	if( pPlayer && ( pPlayer != this ) && flags )
	{
		// If guy we're drawing is cloaked
		if( IsCloaked() )
		{
			DRAWMODEL_CLOAKED();
		}		
	}

	return BaseClass::DrawModel( flags );
}

// Handy function to get the midpoint angle between two angles
float MidAngle(float target, float value, float amount) 
{
	target = anglemod(target);
	value = anglemod(value);

	float delta = target - value;

	if (delta < -180) 
		delta += 360;
	else if (delta > 180) 
		delta -= 360;

	delta *= amount;

	value += delta;

	if (value < -180)
		value += 360;
	else if (value > 360)
		value -= 360;

	return value;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const QAngle &C_FFPlayer::EyeAngles()
{
	// Mapguides
	if (GetTeamNumber() < TEAM_BLUE && m_hNextMapGuide)
	{
		float t = clamp((m_flNextMapGuideTime - gpGlobals->curtime) / m_hLastMapGuide->m_flTime, 0, 1.0f);
		t = SimpleSpline(t);

		static QAngle angDirection;

		// Dealing with 1-t here really, so swap Next/Last
		angDirection.x = MidAngle(m_hLastMapGuide->GetAbsAngles().x, m_hNextMapGuide->GetAbsAngles().x, t);
		angDirection.y = MidAngle(m_hLastMapGuide->GetAbsAngles().y, m_hNextMapGuide->GetAbsAngles().y, t);
		angDirection.z = MidAngle(m_hLastMapGuide->GetAbsAngles().z, m_hNextMapGuide->GetAbsAngles().z, t);

		return angDirection;
	}

	// Concussion
	if ((m_flConcTime > gpGlobals->curtime || m_flConcTime < 0) && conc_test.GetInt() != 0 )
	{
		m_angConcedTest = BaseClass::EyeAngles() + m_angConced;
		return m_angConcedTest;
	}
	else
		return BaseClass::EyeAngles();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFPlayer::CalcView( Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov )
{
	BaseClass::CalcView( eyeOrigin, eyeAngles, zNear, zFar, fov );

	if ((m_flConcTime > gpGlobals->curtime || m_flConcTime < 0) && conc_test.GetInt() == 0)
		eyeAngles += m_angConced;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFPlayer::CalcViewModelView(const Vector& eyeOrigin, const QAngle& eyeAngles)
{
	QAngle angEyeAngles = eyeAngles;

	if (m_flConcTime > gpGlobals->curtime || m_flConcTime < 0)
	{
		QAngle angConced = m_angConced;
		
		// Don't allow us to see underneath the weapon because then we'll see
		// arms stopping in the middle of nowhere and stuff like that
		if (angConced.x > 0.0f)
			angConced.x = 0.0f;

		angEyeAngles -= angConced;
	}

	// If we look up too far then we glitch
	if (angEyeAngles.x < -89.0f)
		angEyeAngles.x = -89.0f;

	BaseClass::CalcViewModelView(eyeOrigin, angEyeAngles);
}
// <-- Mirv: Conc angles

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const QAngle& C_FFPlayer::GetRenderAngles( void )
{
	if ( IsRagdoll() )
	{
		return vec3_angle;
	}
	else
	{
		return m_PlayerAnimState->GetRenderAngles();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFPlayer::UpdateClientSideAnimation()
{
	// Update the animation data. It does the local check here so this works when using
	// a third-person camera (and we don't have valid player angles).
	if ( this == C_FFPlayer::GetLocalFFPlayer() )
		m_PlayerAnimState->Update( EyeAngles()[YAW], EyeAngles()[PITCH] );	// |-- Mirv: 3rd person viewmodel fix
	else
		m_PlayerAnimState->Update( m_angEyeAngles[YAW], m_angEyeAngles[PITCH] );

	BaseClass::UpdateClientSideAnimation();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFPlayer::PostDataUpdate( DataUpdateType_t updateType )
{
	// C_BaseEntity assumes we're networking the entity's angles, so pretend that it
	// networked the same value we already have.
	SetNetworkAngles( GetLocalAngles() );

	// Move player directly to position
	if (m_iSpawnInterpCounter != m_iSpawnInterpCounterCache)
	{
		MoveToLastReceivedPosition(true);
		ResetLatched();
		m_iSpawnInterpCounterCache = m_iSpawnInterpCounter;

		// If this is the local player then do normal spawny stuff.
		if (IsLocalPlayer())
		{
			Spawn();
		}
	}
	
	BaseClass::PostDataUpdate( updateType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFPlayer::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
		m_pInfectionEmitter1 = NULL;
		m_pInfectionEmitter2 = NULL;
		m_pImmunityEmitter1 = NULL;
		m_pImmunityEmitter2 = NULL;
	}

	if (IsLocalPlayer())
	{
		//if( IsCloaked() )
		//	SetStealMouseForCloak( true );
		//else
        //    SetStealMouseForCloak( false );

		// Sometimes the server changes our weapon for us (eg. if we run out of ammo).
		// The client doesn't pick up on this and so weapons' holster and deploy aren't run.
		// This fixes it, hurrah.
		// Added extra guards to make this safer
		if (IsAlive() && GetTeamNumber() >= TEAM_BLUE && GetActiveWeapon() != m_pOldActiveWeapon)
		{
			if (m_pOldActiveWeapon)
				m_pOldActiveWeapon->Holster(GetActiveWeapon());

			if (GetActiveWeapon())
				GetActiveWeapon()->Deploy();

			m_pOldActiveWeapon = GetActiveWeapon();
		}

		// Also lets keep track of the lift state of the local player here too
		// That way we can find out if the local player has died and do any jobs
		// needed (such as clearing status icons or view effects).
		static char localLifeState = LIFE_DEAD;

		if (localLifeState == LIFE_ALIVE && m_lifeState > LIFE_ALIVE)
		{
			Death();
		}

		localLifeState = m_lifeState;
	}

	UpdateVisibility();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFPlayer::ClientThink( void )
{
	// Hopefully when the particles die the ::Create()
	// stuff gets removed automagically?

	// Update infection emitters
	if( IsAlive() && IsInfected() && !IsImmune() && !IsDormant() )
	{
		// Player is infected & emitter is NULL, start it up!
		if( !m_pInfectionEmitter1 )
			m_pInfectionEmitter1 = CInfectionEmitter::Create( "InfectionEmitter" );				

		if( !m_pInfectionEmitter2 )
			m_pInfectionEmitter2 = CInfectionEmitter::Create( "InfectionEmitter" );

		// Update emitter position & die time
		if( !!m_pInfectionEmitter1 )
		{
			m_pInfectionEmitter1->SetDieTime( gpGlobals->curtime + 5.0f );
			m_pInfectionEmitter1->UpdateEmitter( GetAbsOrigin() - Vector( 0, 0, 16 ), GetAbsVelocity() );
		}

		if( !!m_pInfectionEmitter2 )
		{
			m_pInfectionEmitter2->SetDieTime( gpGlobals->curtime + 5.0f );
			m_pInfectionEmitter2->UpdateEmitter( EyePosition() - Vector( 0, 0, 16 ), GetAbsVelocity() );
		}
	}
	else
	{
		// If player dead/non-infected but emitter still active, stop it
		if( !!m_pInfectionEmitter1 )
		{
			m_pInfectionEmitter1->SetDieTime( 0.0f );
			m_pInfectionEmitter1 = NULL;
		}

		if( !!m_pInfectionEmitter2 )
		{
			m_pInfectionEmitter2->SetDieTime( 0.0f );
			m_pInfectionEmitter2 = NULL;
		}
	}

	// Update immunity emitters
	if( IsAlive() && IsImmune() && !IsInfected() && !IsDormant() )
	{
		// Player is immune & emitter is NULL, start it up!
		if( !m_pImmunityEmitter1 )
			m_pImmunityEmitter1 = CImmunityEmitter::Create( "ImmunityEmitter" );				

		if( !m_pImmunityEmitter2 )
			m_pImmunityEmitter2 = CImmunityEmitter::Create( "ImmunityEmitter" );

		// Update emitter position & die time
		if( !!m_pImmunityEmitter1 )
		{
			m_pImmunityEmitter1->SetDieTime( gpGlobals->curtime + 5.0f );
			m_pImmunityEmitter1->UpdateEmitter( GetAbsOrigin() - Vector( 0, 0, 16 ), GetAbsVelocity() );
		}

		if( !!m_pImmunityEmitter2 )
		{
			m_pImmunityEmitter2->SetDieTime( gpGlobals->curtime + 5.0f );
			m_pImmunityEmitter2->UpdateEmitter( EyePosition() - Vector( 0, 0, 16 ), GetAbsVelocity() );
		}
	}
	else
	{
		// Cleanup
		if( !!m_pImmunityEmitter1 )
		{
			m_pImmunityEmitter1->SetDieTime( 0.0f );
			m_pImmunityEmitter1 = NULL;
		}

		if( !!m_pImmunityEmitter2 )
		{
			m_pImmunityEmitter2->SetDieTime( 0.0f );
			m_pImmunityEmitter2 = NULL;
		}
	}

	BaseClass::ClientThink();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFPlayer::Simulate()
{
	BaseClass::Simulate();

	g_FFTimers.SimulateTimers();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFPlayer::DoAnimationEvent( PlayerAnimEvent_t event )
{
	m_PlayerAnimState->DoAnimationEvent( event );
}

//-----------------------------------------------------------------------------
// Purpose: Bug #0000508: Carried objects cast a shadow for the carrying player
//-----------------------------------------------------------------------------
ShadowType_t C_FFPlayer::ShadowCastType( void )
{
	// Cloaked players have no shadows
	if( IsCloaked() )
		return SHADOWS_NONE;

	if( this == ToFFPlayer( C_BasePlayer::GetLocalPlayer() ) )
	{
		if( r_selfshadows.GetInt() )
			return SHADOWS_RENDER_TO_TEXTURE_DYNAMIC;

		return SHADOWS_NONE;
	}
	else
	{
		return SHADOWS_RENDER_TO_TEXTURE_DYNAMIC;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_FFPlayer::ShouldDraw( void )
{
	// If we're dead, our ragdoll will be drawn for us instead.
	if ( !IsAlive() )
		return false;

	if( GetTeamNumber() == TEAM_SPECTATOR )
		return false;

	if( IsLocalPlayer() && IsRagdoll() )
		return true;

	return BaseClass::ShouldDraw();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_FFPlayer::IsDisguised( void )
{
	return ( GetClassSlot() == CLASS_SPY ) && ( m_iSpyDisguise != 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_FFPlayer::GetDisguisedTeam( void )
{
	if( IsDisguised() )	
		return ( m_iSpyDisguise & 0x0000000F );

	return TEAM_UNASSIGNED;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_FFPlayer::GetDisguisedClass( void )
{
	if( IsDisguised() )
		return ( ( m_iSpyDisguise & 0xFFFFFFF0 ) >> 4 );

	return CLASS_NONE;
}

// --> Mirv: Get the class
int C_FFPlayer::GetClassSlot( void )
{
	return ( m_iClassStatus & 0x0000000F );
}
// <-- Mirv: Get the class

CFFWeaponBase* C_FFPlayer::GetActiveFFWeapon() const
{
	return dynamic_cast< CFFWeaponBase* >( GetActiveWeapon() );
}

void C_FFPlayer::SwapToWeapon(FFWeaponID weaponid)
{
	if (GetActiveFFWeapon() && GetActiveFFWeapon()->GetWeaponID() == weaponid)
		return;

	CFFWeaponBase *weap;

	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		weap = dynamic_cast<CFFWeaponBase *>(GetWeapon(i));
		if (weap && weap->GetWeaponID() == weaponid)
		{
			::input->MakeWeaponSelection(weap);
			break;
		}
	}	
}

void C_FFPlayer::SwapToWeaponSlot(int iSlot)
{
	Assert(iSlot > 0 && iSlot < MAX_WEAPON_SLOTS);

	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		C_FFWeaponBase *pWeapon = dynamic_cast<C_FFWeaponBase *> (GetWeapon(i));
		
		if (pWeapon && pWeapon->GetFFWpnData().iSlot == iSlot -1)
		{
			::input->MakeWeaponSelection(pWeapon);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: When the player selects a timer by changing this cvar, validate
//			and find the timer and ensure everything is okay.
//			HANDILY this is also called when the game first loads up
//-----------------------------------------------------------------------------
void TimerChange_Callback(ConVar *var, char const *pOldString)
{
	const char	*pszTimerString = var->GetString();
	int			nTimerStringChars = strlen(pszTimerString);

	// No need to do any checking if it's default because it should always
	// be there
	if (Q_strcmp(pszTimerString, "default") == 0)
	{
		Q_strcpy(g_szTimerFile, "timers/default.wav");
		return;
	}

	if (nTimerStringChars > 28)
	{
		Msg("Timer filename too large, must be 14 characters or less!\n");
		var->SetValue("default");
		return;
	}

	for (int i = 0; i < nTimerStringChars; i++)	
	{
		// Not valid alphanumeric (better way to check anyone?)
		if (!((pszTimerString[i] >= '0' && pszTimerString[i] <= '9') ||
			  (pszTimerString[i] >= 'A' && pszTimerString[i] <= 'Z') ||
			  (pszTimerString[i] >= 'a' && pszTimerString[i] <= 'z')))
		{
			Msg("Timer filename must only contain alphanumeric characters (0-9a-Z). Remember that file extension is not needed!\n");
			var->SetValue("default");
			return;
		}
	}
	
	// We've got this far so should be safe now
	char buf[MAX_PATH];
	Q_snprintf(buf, MAX_PATH - 1, "sound/timers/%s.*", var->GetString());

	// Find the file (extension will be found)
	FileFindHandle_t findHandle;
	const char *pFilename = (*pFilesystem)->FindFirstEx(buf, "MOD", &findHandle);

	(*pFilesystem)->FindClose(findHandle);
	
	// Timer not found so return to default
	if (!pFilename)
	{
		Msg("Timer not found.\n");
		var->SetValue("default");
	}

	Q_snprintf(g_szTimerFile, MAX_PATH - 1, "timers/%s", pFilename);

	(*pFilesystem)->FindClose(findHandle);
}

void C_FFPlayer::AddEntity()
{
	BaseClass::AddEntity();

	// Can probably get rid of this

	/*QAngle vTempAngles = GetLocalAngles();
	vTempAngles[PITCH] = m_angEyeAngles[PITCH];

	SetLocalAngles(vTempAngles);

	m_PlayerAnimState.Update();

	// Zero out model pitch, blending takes care of all of it.
	SetLocalAnglesDim(X_INDEX, 0);*/

	if (this != C_BasePlayer::GetLocalPlayer())
	{
		if (IsEffectActive(EF_DIMLIGHT))
		{
			int iAttachment = LookupAttachment("anim_attachment_RH");

			if (iAttachment < 0)
				return;

			Vector vecOrigin;
			QAngle eyeAngles = m_angEyeAngles;

			GetAttachment(iAttachment, vecOrigin, eyeAngles);

			Vector vForward;
			AngleVectors(eyeAngles, &vForward);

			trace_t tr;
			UTIL_TraceLine(vecOrigin, vecOrigin + (vForward * 200), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

			if (!m_pFlashlightBeam)
			{
				BeamInfo_t beamInfo;
				beamInfo.m_nType = TE_BEAMPOINTS;
				beamInfo.m_vecStart = tr.startpos;
				beamInfo.m_vecEnd = tr.endpos;
				beamInfo.m_pszModelName = "sprites/glow01.vmt";
				beamInfo.m_pszHaloName = "sprites/glow01.vmt";
				beamInfo.m_flHaloScale = 3.0;
				beamInfo.m_flWidth = 8.0f;
				beamInfo.m_flEndWidth = 35.0f;
				beamInfo.m_flFadeLength = 300.0f;
				beamInfo.m_flAmplitude = 0;
				beamInfo.m_flBrightness = 60.0;
				beamInfo.m_flSpeed = 0.0f;
				beamInfo.m_nStartFrame = 0.0;
				beamInfo.m_flFrameRate = 0.0;
				beamInfo.m_flRed = 255.0;
				beamInfo.m_flGreen = 255.0;
				beamInfo.m_flBlue = 255.0;
				beamInfo.m_nSegments = 8;
				beamInfo.m_bRenderable = true;
				beamInfo.m_flLife = 0.5;
				beamInfo.m_nFlags = FBEAM_FOREVER | FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;

				m_pFlashlightBeam = beams->CreateBeamPoints(beamInfo);
			}

			if (m_pFlashlightBeam)
			{
				BeamInfo_t beamInfo;
				beamInfo.m_vecStart = tr.startpos;
				beamInfo.m_vecEnd = tr.endpos;
				beamInfo.m_flRed = 255.0;
				beamInfo.m_flGreen = 255.0;
				beamInfo.m_flBlue = 255.0;

				beams->UpdateBeamInfo(m_pFlashlightBeam, beamInfo);

				dlight_t *el = effects->CL_AllocDlight(0);
				el->origin = tr.endpos;
				el->radius = 50; 
				el->color.r = 200;
				el->color.g = 200;
				el->color.b = 200;
				el->die = gpGlobals->curtime + 0.1;
			}
		}

		else if (m_pFlashlightBeam)
		{
			ReleaseFlashlight();
		}
	}
}

bool C_FFPlayer::ShouldReceiveProjectedTextures(int flags)
{
	Assert(flags & SHADOW_FLAGS_PROJECTED_TEXTURE_TYPE_MASK);

	if (IsEffectActive(EF_NODRAW))
		return false;

	if (flags & SHADOW_FLAGS_FLASHLIGHT)
	{
		return true;
	}

	return BaseClass::ShouldReceiveProjectedTextures(flags);
}

void C_FFPlayer::NotifyShouldTransmit(ShouldTransmitState_t state)
{
	if (state == SHOULDTRANSMIT_END)
	{
		if (m_pFlashlightBeam != NULL)
		{
			ReleaseFlashlight();
		}
	}

	BaseClass::NotifyShouldTransmit(state);
}

void C_FFPlayer::ReleaseFlashlight()
{
	if (m_pFlashlightBeam)
	{
		m_pFlashlightBeam->flags = 0;
		m_pFlashlightBeam->die = gpGlobals->curtime - 1;

		m_pFlashlightBeam = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Part of the weapon change bugfix. Ensures that the weapons'
//			deploy & holster method isn't called twice when the client switches
//-----------------------------------------------------------------------------
bool C_FFPlayer::Weapon_Switch(CBaseCombatWeapon *pWeapon, int viewmodelindex /*=0*/) 
{
	if (BaseClass::Weapon_Switch(pWeapon, viewmodelindex))
	{
		m_pOldActiveWeapon = pWeapon;
		return true;
	}

	return false;
}

extern ConVar default_fov;

//-----------------------------------------------------------------------------
// Purpose: Disable FOV and use weapon-specific stuff
//-----------------------------------------------------------------------------
float C_FFPlayer::GetFOV()
{
	C_FFWeaponBase *pWeapon = GetActiveFFWeapon();

	// There is a weapon specific FOV
	if (pWeapon)
	{
		float flFOV = pWeapon->GetFOV();

		if (flFOV > 0)
			return pWeapon->GetFOV();
	}

	return default_fov.GetFloat();
}

CON_COMMAND(ffdev_hallucinate, "hallucination!")
{
	C_FFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());
	pPlayer->m_iHallucinationIndex++;
}

CON_COMMAND(ffdev_hallucinatereset, "okay stop")
{
	C_FFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());
	pPlayer->m_iHallucinationIndex = 0;
}

//-----------------------------------------------------------------------------
// Purpose: This is a bit of a mis-use of the effects system
//-----------------------------------------------------------------------------
void Hallucination_Callback(const CEffectData &data)
{
	C_FFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

	Assert(pPlayer);

	// There is already a hallucination going on. We don't want to keep
	// swapping the index because that will look weird. Just extend the
	// current effect instead
	if (pPlayer->m_iHallucinationIndex)
	{
		pPlayer->m_flHallucinationFinish = gpGlobals->curtime + 15.0f;
		return;
	}
	
	pPlayer->m_iHallucinationIndex = random->RandomInt(1, 50);
	pPlayer->m_flHallucinationFinish = gpGlobals->curtime + 15.0f;
}

DECLARE_CLIENT_EFFECT("Hallucination", Hallucination_Callback);

void Gib_Callback(const CEffectData &data)
{
	C_FFPlayer *pPlayer = dynamic_cast<C_FFPlayer *> (data.GetEntity());

	Vector vecPosition = data.m_vOrigin;
	Vector vecOffset;
	const char *pszGibModel;

	// We can use the player origin here
	if (pPlayer && !pPlayer->IsDormant())
	{
		vecPosition = pPlayer->GetAbsOrigin();

		// We can also use this player to create a weapon model
		CFFWeaponBase *pWeapon = pPlayer->GetActiveFFWeapon();

		if (pWeapon && pWeapon->GetWeaponID() < FF_WEAPON_DEPLOYDISPENSER)
		{
			C_Gib * pGib = C_Gib::CreateClientsideGib(pWeapon->GetFFWpnData().szWorldModel, pWeapon->GetAbsOrigin(), pPlayer->GetAbsVelocity(), Vector(0, 0, 0), 10.0f);

			if (pGib)
			{
				pGib->SetAbsAngles(pWeapon->GetAbsAngles());
			}
		}
	}

	// Now spawn a number of gibs
	for (int i = 0; i < gibcount.GetInt(); i++)
	{
		vecOffset = vecPosition + Vector(0, 0, random->RandomFloat(-12, 12));

		// The first 3 gibs should be done only once, those after should be random
		int iGibNumber = (i < 3 ? i + 1 : random->RandomInt(4, 8));

		pszGibModel = VarArgs("models/gibs/gib%d.mdl", iGibNumber);

		C_Gib *pGib = C_Gib::CreateClientsideGib(pszGibModel, vecOffset, Vector(random->RandomFloat(-150, 150), random->RandomFloat(-150, 150), random->RandomFloat(100, 800)), Vector(0, 0, 0), 10.0f);

		if (pGib)
		{
			pGib->LeaveBloodDecal(true);
		}

		UTIL_BloodImpact(vecOffset, Vector(0, 0, 0), BLOOD_COLOR_RED, 512);
	}
}

DECLARE_CLIENT_EFFECT("Gib", Gib_Callback);
