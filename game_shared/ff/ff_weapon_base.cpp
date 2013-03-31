/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_base.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December ##, 2004
/// @brief All weapons derived from here
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation


#include "cbase.h"
#include "in_buttons.h"
#include "takedamageinfo.h"
#include "ff_weapon_base.h"
#include "ammodef.h"
#include "ff_buildableobjects_shared.h"
#include "ff_utils.h"

#ifdef CLIENT_DLL 
	#include "c_ff_player.h"
#else
	#include "ff_player.h"
	#include "eventqueue.h"
#endif

// All our weapons (no longer static)
const char *s_WeaponAliasInfo[] = 
{
	"none", 			// FF_WEAPON_NONE

	"crowbar", 			// FF_WEAPON_CROWBAR
	"knife", 			// FF_WEAPON_KNIFE
	"electricknife", 	// FF_WEAPON_ELECTRICKNIFE
	"medkit", 			// FF_WEAPON_MEDKIT
	"spanner", 			// FF_WEAPON_SPANNER
	"umbrella", 		// FF_WEAPON_UMBRELLA

	"shotgun", 			// FF_WEAPON_SHOTGUN
	"supershotgun", 	// FF_WEAPON_SUPERSHOTGUN

	"nailgun", 			// FF_WEAPON_NAILGUN
	"supernailgun", 	// FF_WEAPON_SUPERNAILGUN

	"grenadelauncher", 	// FF_WEAPON_GRENADELAUNCHER
	"pipelauncher", 	// FF_WEAPON_PIPELAUNCHER

	"autorifle", 		// FF_WEAPON_AUTORIFLE
	"sniperrifle", 		// FF_WEAPON_SNIPERRIFLE

	"flamethrower", 	// FF_WEAPON_FLAMETHROWER, 
	"incendiarycannon", // FF_WEAPON_INCENDIARYCANNON

	"railgun", 			// FF_WEAPON_RAILGUN

	"tranq",		 	// FF_WEAPON_TRANQUILISER
	"hookgun", 			// FF_WEAPON_HOOKGUN

	"assaultcannon", 	// FF_WEAPON_ASSAULTCANNON

	"rpg", 				// FF_WEAPON_RPG

	"tommygun",			// FF_WEAPON_TOMMYGUN

	"deploydispenser", 	// FF_WEAPON_DEPLOYDISPENSER
	"deploysentrygun", 	// FF_WEAPON_DEPLOYSENTRYGUN
	"deploydetpack", 	// FF_WEAPON_DEPLOYDETPACK
	"deploymancannon",	// FF_WEAPON_DEPLOYMANCANNON

	NULL, 				// FF_WEAPON_MAX
};

//=============================================================================
// Global functions
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Given an alias, return the associated weapon ID
//----------------------------------------------------------------------------
int AliasToWeaponID(const char *alias)
{
	if (alias)
	{
		for (int i=0; s_WeaponAliasInfo[i] != NULL; ++i)
		{
			if (!Q_stricmp(s_WeaponAliasInfo[i], alias))
				return i;
		}
	}

	return FF_WEAPON_NONE;
}

//----------------------------------------------------------------------------
// Purpose: Given a weapon ID, return its alias
//----------------------------------------------------------------------------
const char *WeaponIDToAlias(int id)
{
	if ((id >= FF_WEAPON_MAX) || (id < 0))
		return NULL;

	return s_WeaponAliasInfo[id];
}

//=============================================================================
// CFFWeaponBase tables.
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponBase, DT_FFWeaponBase)

BEGIN_NETWORK_TABLE(CFFWeaponBase, DT_FFWeaponBase)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CFFWeaponBase)
END_PREDICTION_DATA()

#ifdef GAME_DLL
	BEGIN_DATADESC(CFFWeaponBase)
		DEFINE_ENTITYFUNC( DropThink ),
	END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS(weapon_ff_base, CFFWeaponBase);

//=============================================================================
// CFFWeaponCSBase implementation. 
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponBase::CFFWeaponBase()
{
	SetPredictionEligible(true);
	AddSolidFlags(FSOLID_TRIGGER); // Nothing collides with these but it gets touches.

	// All FF weapons fire underwater
	m_bFiresUnderwater = true; 

	// Most weapons have a muzzle flash
	m_bMuzzleFlash = true;

	SetCollisionGroup(COLLISION_GROUP_WEAPON);
}

void CFFWeaponBase::Spawn()
{
	CBaseCombatWeapon::Spawn();
}

//----------------------------------------------------------------------------
// Purpose: All weapons use the same silence sound
//----------------------------------------------------------------------------
void CFFWeaponBase::WeaponSound(WeaponSound_t sound_type, float soundtime /* = 0.0f */)
{
#ifdef CLIENT_DLL

	// If we have some sounds from the weapon classname.txt file, play a random one of them
	const char *shootsound = GetWpnData().aShootSounds[sound_type]; 
	if (!shootsound || !shootsound[0])
		return;

	CBroadcastRecipientFilter filter; // this is client side only
	if (!te->CanPredict())
		return;

	// The GetAbsOrigin() is the reason for s_bAbsQueriesValid assert we keep getting.
	// Let's try doing this a different way.
	CBaseEntity::EmitSound(filter, GetPlayerOwner()->entindex(), shootsound, NULL, soundtime); 
#else
	BaseClass::WeaponSound(sound_type, soundtime);
#endif
}

//----------------------------------------------------------------------------
// Purpose: For weapon sounds that only the local player should hear
//----------------------------------------------------------------------------
void CFFWeaponBase::WeaponSoundLocal( WeaponSound_t sound_type, float soundtime )
{
	// If we have some sounds from the weapon classname.txt file, play a random one of them
	const char *shootsound = GetWpnData().aShootSounds[ sound_type ];
	if( !shootsound || !shootsound[0] )
		return;

	CSoundParameters params;

	if( !GetParametersForSound( shootsound, params, NULL ) )
		return;

	CSingleUserRecipientFilter filter( GetPlayerOwner() );

#ifdef CLIENT_DLL 
	if( GetPlayerOwner() == C_FFPlayer::GetLocalFFPlayer() )
	{
		// Not sure why we're checking tempents... but we did it above^^
		if( !te->CanPredict() )
			return;

		// The filter.UsePredictionRules() was throwing an assert previously. Hope
		// the sound doesn't play twice now (?)

		EmitSound( filter, GetPlayerOwner()->entindex(), shootsound, NULL, soundtime );
	}
#else
	EmitSound( filter, GetPlayerOwner()->entindex(), shootsound, NULL, soundtime );
#endif	
}

//----------------------------------------------------------------------------
// Purpose: Get CFFPlayer owner
//----------------------------------------------------------------------------
CFFPlayer * CFFWeaponBase::GetPlayerOwner() const
{
	//return dynamic_cast<CFFPlayer *> (GetOwner());
	return ToFFPlayer(GetOwner());
}

const char *CFFWeaponBase::GetWorldModel( void ) const
{
#ifdef CLIENT_DLL
	CFFPlayer *pFFPlayer = GetPlayerOwner();
	if(pFFPlayer && pFFPlayer->IsDisguised())
	{
		CFFWeaponBase *pWeapon = pFFPlayer->GetActiveFFWeapon();

		if(!pWeapon)
			return BaseClass::GetWorldModel();

		int iSlot = pWeapon->GetFFWpnData().iSlot;
		int iClass = pFFPlayer->GetDisguisedClass();

		if(pWeapon == this)
		{
			if(pFFPlayer->m_DisguisedWeapons[iClass].szWeaponModel[iSlot][0] != NULL)
				return pFFPlayer->m_DisguisedWeapons[iClass].szWeaponModel[iSlot];
			else if (pFFPlayer->GetDisguisedClass() == CLASS_CIVILIAN && pFFPlayer->m_DisguisedWeapons[CLASS_CIVILIAN].szWeaponModel[0] != NULL)
				return pFFPlayer->m_DisguisedWeapons[CLASS_CIVILIAN].szWeaponModel[0]; // always use umbrella
		}
	}
#endif
	return BaseClass::GetWorldModel();
}

int CFFWeaponBase::GetWorldModelIndex( void )
{
	CFFPlayer *pFFPlayer = GetPlayerOwner();
	if(pFFPlayer && pFFPlayer->IsDisguised())
		return modelinfo->GetModelIndex(GetWorldModel()); 
	else
#ifdef CLIENT_DLL
		return BaseClass::GetWorldModelIndex();
#else	//server should never use this code!
		Assert( false );
#endif
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Get script file weapon data
//----------------------------------------------------------------------------
const CFFWeaponInfo &CFFWeaponBase::GetFFWpnData() const
{
	const FileWeaponInfo_t *pWeaponInfo = &GetWpnData();
	const CFFWeaponInfo *pFFInfo;

	#ifdef _DEBUG
		pFFInfo = dynamic_cast<const CFFWeaponInfo *> (pWeaponInfo);
		Assert(pFFInfo);
	#else
		pFFInfo = static_cast<const CFFWeaponInfo *> (pWeaponInfo);
	#endif

	return *pFFInfo;
}

//----------------------------------------------------------------------------
// Purpose: Drop the weapon
//----------------------------------------------------------------------------
void CFFWeaponBase::Drop( const Vector& vecVelocity )
{
#if !defined( CLIENT_DLL )

	// Once somebody drops a gun, it's fair game for removal when/if
	// a game_weapon_manager does a cleanup on surplus weapons in the
	// world.
	SetRemoveable( true );

	//If it was dropped then there's no need to respawn it.
	AddSpawnFlags( SF_NORESPAWN );

	StopAnimation();
	StopFollowingEntity( );
	SetMoveType( MOVETYPE_FLYGRAVITY );
	// clear follow stuff, setup for collision
	SetGravity(1.0);
	m_iState = WEAPON_NOT_CARRIED;
	RemoveEffects( EF_NODRAW );
	FallInit();
	SetGroundEntity( NULL );
	AddEFlags( EFL_NO_WEAPON_PICKUP );
	SetThink( &CFFWeaponBase::DropThink );
	SetTouch( NULL );

	if( hl2_episodic.GetBool() )
	{
		RemoveSpawnFlags( SF_WEAPON_NO_PLAYER_PICKUP );
	}

	IPhysicsObject *pObj = VPhysicsGetObject();
	if ( pObj != NULL )
	{
		AngularImpulse	angImp( 200, 200, 200 );
		pObj->AddVelocity( &vecVelocity, &angImp );
	}
	else
	{
		SetAbsVelocity( vecVelocity );
	}

	SetNextThink( gpGlobals->curtime + 1.0f );
	SetOwnerEntity( NULL );
	SetOwner( NULL );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Remove after 30 seconds
//-----------------------------------------------------------------------------
void CFFWeaponBase::DropThink( void )
{
#if !defined( CLIENT_DLL )
	SetTouch( NULL );

	SetThink( &CBaseEntity::SUB_Remove );
	SetNextThink( gpGlobals->curtime + 30.0f );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: OVERRIDDEN from base sdk. Changed SequenceLength to MAX_DEPLOY_TIME for purposes of the first firing
// Input  : *szViewModel - 
//			*szWeaponModel - 
//			iActivity - 
//			*szAnimExt - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CFFWeaponBase::DefaultDeploy(char *szViewModel, char *szWeaponModel, int iActivity, char *szAnimExt)
{
	// We either use the default deploy time or the next time this gun is ready,
	// whichever is larger
	float flNextAttack = max(gpGlobals->curtime + GetFFWpnData().m_flDeployDelay, m_flNextPrimaryAttack);
	
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner)
	{
		// Dead men deploy no weapons
		if (pOwner->IsAlive() == false)
			return false;

		if( GetPlayerOwner()->IsStaticBuilding() )
			return false;

		pOwner->SetAnimationExtension(szAnimExt);

		SetViewModel();
		SendWeaponAnim(iActivity);

		pOwner->SetNextAttack(flNextAttack);
	}

	// Can't shoot again until we've finished deploying
	m_flNextPrimaryAttack	= flNextAttack;
	m_flNextSecondaryAttack = flNextAttack;

	SetWeaponVisible(true);

#ifndef CLIENT_DLL
	// Cancel any pending hide events
	g_EventQueue.CancelEventOn(this, "HideWeapon");
#endif

	return true;
}

//----------------------------------------------------------------------------
// Purpose: Stop selection of weapons while building
//----------------------------------------------------------------------------
bool CFFWeaponBase::CanBeSelected()
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	if (pPlayer->IsStaticBuilding())
		return false;
	//else if( pPlayer->IsCloaked() )
	//	return false;
	else
		return BaseClass::CanBeSelected();
}

//----------------------------------------------------------------------------
// Purpose: Do the weapon's recoil
//----------------------------------------------------------------------------
void CFFWeaponBase::WeaponRecoil()
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	QAngle angPunch = pPlayer->GetPunchAngle();
	angPunch.x += GetFFWpnData().m_flRecoilAmount;

	pPlayer->ViewPunch(angPunch);
}

//----------------------------------------------------------------------------
// Purpose: Return a weapon's death notice name
//----------------------------------------------------------------------------
char *CFFWeaponBase::GetDeathNoticeName()
{
#ifdef GAME_DLL
	return (char *) STRING(m_iClassname);
#else
	return "GetDeathNoticeName not implemented on client yet";
#endif
}

#ifdef CLIENT_DLL

//----------------------------------------------------------------------------
// Purpose: From HL2MP
//----------------------------------------------------------------------------
void CFFWeaponBase::OnDataChanged(DataUpdateType_t type)
{
	BaseClass::OnDataChanged(type);

	if (GetPredictable() && !ShouldPredict())
		ShutdownPredictable();
}


//----------------------------------------------------------------------------
// Purpose: From HL2MP
//----------------------------------------------------------------------------
bool CFFWeaponBase::ShouldPredict()
{
	if (GetOwner() && GetOwner() == C_BasePlayer::GetLocalPlayer())
		return true;

	return BaseClass::ShouldPredict();
}

//----------------------------------------------------------------------------
// Purpose: Draw differently if owner cloaked
//----------------------------------------------------------------------------
int CFFWeaponBase::DrawModel( int flags )
{
	C_FFPlayer *pPlayer = ToFFPlayer( GetOwner() );
	if( pPlayer )
	{
		if ( !pPlayer->ShouldDraw() )
			return 1;

		if( !pPlayer->IsCloaked() )
		{
			ReleaseOverrideMaterial(FF_CLOAK_MATERIAL);
		}
		else
		{
			// don't draw if cloaked and basically not moving
			if ( pPlayer->GetLocalVelocity().Length() < 1.0f )
				return 1;

			FindOverrideMaterial(FF_CLOAK_MATERIAL, FF_CLOAK_TEXTURE_GROUP);
		}
	}

	return BaseClass::DrawModel( flags );
}

//-----------------------------------------------------------------------------
// Purpose: cloaked models should be in the translucent renderables list
//-----------------------------------------------------------------------------
RenderGroup_t CFFWeaponBase::GetRenderGroup()
{
	C_FFPlayer *pPlayer = ToFFPlayer( GetOwner() );
	if( pPlayer )
	{
		if ( pPlayer->IsCloaked() )
			return RENDER_GROUP_TRANSLUCENT_ENTITY;
	}

	return BaseClass::GetRenderGroup();
}

//-----------------------------------------------------------------------------
// Purpose: Bug #0000508: Carried objects cast a shadow for the carrying player
//-----------------------------------------------------------------------------
ShadowType_t CFFWeaponBase::ShadowCastType( void )
{
	C_FFPlayer *pPlayer = ToFFPlayer( GetOwner() );
	if( pPlayer )
	{
		// Cloaked players have no shadows
		if( pPlayer->IsCloaked() )
			return SHADOWS_NONE;

		if( pPlayer == ToFFPlayer( C_BasePlayer::GetLocalPlayer() ) )
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

	return BaseClass::ShadowCastType();
}

#endif

//-----------------------------------------------------------------------------
// Purpose: Primary fire button attack for non-clip weapons
//			For clip weapons, see ff_weapon_baseclip.cpp
//-----------------------------------------------------------------------------
void CFFWeaponBase::PrimaryAttack()
{
	CANCEL_IF_BUILDING();
	CANCEL_IF_CLOAKED();

	// Only the player fires this way so we can cast
	CFFPlayer *pPlayer = ToFFPlayer(GetOwner());

	if (!pPlayer)
		return;

	// Undisguise
#ifdef GAME_DLL
	pPlayer->ResetDisguise();
#endif

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound(SINGLE);

	if (m_bMuzzleFlash)
		pPlayer->DoMuzzleFlash();

	SendWeaponAnim(GetPrimaryAttackActivity());

	// player "shoot" animation
	//pPlayer->SetAnimation(PLAYER_ATTACK1);
	pPlayer->DoAnimationEvent(PLAYERANIMEVENT_FIRE_GUN_PRIMARY);

#ifdef GAME_DLL
	int nShots = min(GetFFWpnData().m_iCycleDecrement, pPlayer->GetAmmoCount(m_iPrimaryAmmoType));
	pPlayer->RemoveAmmo(nShots, m_iPrimaryAmmoType);

#endif

	// Fire now
	Fire();

	pPlayer->m_flTrueAimTime = gpGlobals->curtime;

	m_flNextPrimaryAttack = gpGlobals->curtime + GetFFWpnData().m_flCycleTime;

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}

	//Add our view kick in
	pPlayer->ViewPunch(QAngle(-GetFFWpnData().m_flRecoilAmount * GetRecoilMultiplier(), 0, 0));
}

//-----------------------------------------------------------------------------
// Purpose: A modified ItemPostFrame to allow for different cycledecrements
//-----------------------------------------------------------------------------
void CFFWeaponBase::ItemPostFrame()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	//Track the duration of the fire
	//FIXME: Check for IN_ATTACK2 as well?
	//FIXME: What if we're calling ItemBusyFrame?
	m_fFireDuration = (pOwner->m_nButtons & IN_ATTACK) ? (m_fFireDuration + gpGlobals->frametime) : 0.0f;

	if (UsesClipsForAmmo1())
	{
		CheckReload();
	}

	bool bFired = false;

	// Secondary attack has priority
	if ((pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
	{
		if (UsesSecondaryAmmo() && pOwner->GetAmmoCount(m_iSecondaryAmmoType) <=0)
		{
			if (m_flNextEmptySoundTime < gpGlobals->curtime)
			{
				WeaponSound(EMPTY);
				m_flNextSecondaryAttack = m_flNextEmptySoundTime = gpGlobals->curtime + 0.5;
			}
		}
		else if (pOwner->GetWaterLevel() == 3 && m_bAltFiresUnderwater == false)
		{
			// This weapon doesn't fire underwater
			// Causing menu click underwater
			// WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			// FIXME: This isn't necessarily true if the weapon doesn't have a secondary fire!
			bFired = true;
			SecondaryAttack();

			// Secondary ammo doesn't have a reload animation
			if (UsesClipsForAmmo2())
			{
				// reload clip2 if empty
				if (m_iClip2 < 1)
				{
					pOwner->RemoveAmmo(1, m_iSecondaryAmmoType);
					m_iClip2 = m_iClip2 + 1;
				}
			}
		}
	}

	if (!bFired && (pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		// Clip empty? Or out of ammo on a no-clip weapon?
		if (!IsMeleeWeapon() &&  
			((UsesClipsForAmmo1() && m_iClip1 < GetFFWpnData().m_iCycleDecrement)
			|| (!UsesClipsForAmmo1()
			&& pOwner->GetAmmoCount(m_iPrimaryAmmoType) < GetFFWpnData().m_iCycleDecrement)))
		{
			HandleFireOnEmpty();
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f; //VOOGRU: #0000562 
		}
		else if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
		{
			// This weapon doesn't fire underwater
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			//NOTENOTE: There is a bug with this code with regards to the way machine guns catch the leading edge trigger
			//			on the player hitting the attack key.  It relies on the gun catching that case in the same frame.
			//			However, because the player can also be doing a secondary attack, the edge trigger may be missed.
			//			We really need to hold onto the edge trigger and only clear the condition when the gun has fired its
			//			first shot.  Right now that's too much of an architecture change -- jdw

			// If the firing button was just pressed, or the alt-fire just released, reset the firing time
			if ((pOwner->m_afButtonPressed & IN_ATTACK) || (pOwner->m_afButtonReleased & IN_ATTACK2))
			{
				m_flNextPrimaryAttack = gpGlobals->curtime;
			}

			PrimaryAttack();
		}
	}

	// -----------------------
	//  Reload pressed / Clip Empty
	// -----------------------
	if (pOwner->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload)
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
		m_fFireDuration = 0.0f;
	}

	// -----------------------
	//  No buttons down
	// -----------------------
	if (! ((pOwner->m_nButtons & IN_ATTACK) || /* (pOwner->m_nButtons & IN_ATTACK2) ||*/ (pOwner->m_nButtons & IN_RELOAD))) // |-- Mirv: Removed attack2 so things can continue while in menu
	{
		// no fire buttons down or reloading
		if (!ReloadOrSwitchWeapons() && (m_bInReload == false))
		{
			WeaponIdle();
		}
	}
}

//----------------------------------------------------------------------------
// Purpose: Don't leave a dangling pointer!
//----------------------------------------------------------------------------
CFFWeaponBase::~CFFWeaponBase()
{
#ifdef CLIENT_DLL
	C_FFPlayer *pOwner = GetPlayerOwner();

	if (pOwner && pOwner->m_pOldActiveWeapon == this)
	{
		pOwner->m_pOldActiveWeapon = NULL;
	}
#endif
}
