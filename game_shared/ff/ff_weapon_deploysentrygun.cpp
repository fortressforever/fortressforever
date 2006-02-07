//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_weapon_deploysentrygun.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 05/12/05
//	@brief A sentrygun construction slot
//
//	REVISIONS
//	---------
//	05/12/05, Mulchman: First created - basically a copy/paste of the
//		DeployDispenser set up
//
//	05/14/05, Mulchman:
//		Optimized as per Mirv's forum suggestion
//
//	09/20/2005,	Mulchman:
//		Added aiming ring.
//
//	09/24/2005,	Mulchman:
//		Added a model for the aiming ring.

#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"

#if defined( CLIENT_DLL )
	#define CFFWeaponDeploySentryGun C_FFWeaponDeploySentryGun
	#include "c_ff_player.h"
	#include "c_ff_buildableobjects.h"
	#include "ff_buildableobjects_shared.h"
	#include "ff_utils.h"
#else
	#include "ff_player.h"
	#include "ff_sentrygun.h"
#endif

//=============================================================================
// CFFWeaponDeploySentryGun
//=============================================================================

class CFFWeaponDeploySentryGun : public CFFWeaponBase
{
public:
	DECLARE_CLASS( CFFWeaponDeploySentryGun, CFFWeaponBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponDeploySentryGun();
#ifdef CLIENT_DLL 
	~CFFWeaponDeploySentryGun( void ) 
	{ 
		Cleanup(); 
		Cleanup_AimSphere();
	}
#endif

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void WeaponIdle();
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo );

	virtual FFWeaponID GetWeaponID( void ) const		{ return FF_WEAPON_DEPLOYSENTRYGUN; }

private:

	CFFWeaponDeploySentryGun( const CFFWeaponDeploySentryGun & );

#ifdef CLIENT_DLL
protected:
	int	m_spriteTexture;
	C_FFSentryGun *pSentryGun;
	C_FFSentryGun_AimSphere *pAimSphere;
#endif

	void Cleanup( void )
	{
#ifdef CLIENT_DLL
		if( pSentryGun )
		{
			pSentryGun->Remove( );
			pSentryGun = NULL;
		}
#endif
	}

	void Cleanup_AimSphere( void )
	{
#ifdef CLIENT_DLL
		if( pAimSphere )
		{
			pAimSphere->Remove();
			pAimSphere = NULL;
		}
#endif
	}
};

//=============================================================================
// CFFWeaponDeploySentryGun tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED( FFWeaponDeploySentryGun, DT_FFWeaponDeploySentryGun )

BEGIN_NETWORK_TABLE( CFFWeaponDeploySentryGun, DT_FFWeaponDeploySentryGun )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CFFWeaponDeploySentryGun )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( ff_weapon_deploysentrygun, CFFWeaponDeploySentryGun );
PRECACHE_WEAPON_REGISTER( ff_weapon_deploysentrygun );

//=============================================================================
// CFFWeaponDeploySentryGun implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponDeploySentryGun::CFFWeaponDeploySentryGun()
{
#ifdef CLIENT_DLL
	pSentryGun = NULL;
	pAimSphere = NULL;

	m_spriteTexture = PrecacheModel( "sprites/lgtning.vmt" );
#endif
}

//----------------------------------------------------------------------------
// Purpose: Handles whatever should be done when they fire (build, aim, etc)
//----------------------------------------------------------------------------
void CFFWeaponDeploySentryGun::PrimaryAttack( void )
{
	if( m_flNextPrimaryAttack < gpGlobals->curtime )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

		Cleanup();
		Cleanup_AimSphere();

#ifdef GAME_DLL
		CFFPlayer *player = GetPlayerOwner();
		CFFSentryGun *sg = dynamic_cast<CFFSentryGun *>(player->m_hSentryGun.Get());

		if (sg && sg->IsBuilt())
		{
			DevMsg("Aiming...!\n");

			Vector vecForward;
			AngleVectors(player->EyeAngles(), &vecForward);
            
			trace_t tr;
			UTIL_TraceLine(player->Weapon_ShootPosition(), player->Weapon_ShootPosition() + vecForward * MAX_TRACE_LENGTH, MASK_SHOT, player, COLLISION_GROUP_NONE, &tr);

            sg->SetFocusPoint(tr.endpos);
		}
		else
		{
			GetPlayerOwner( )->Command_BuildSentryGun( );
			Cleanup();
		}
#endif
	}
}

//----------------------------------------------------------------------------
// Purpose: Handles whatever should be done when they secondary fire
//----------------------------------------------------------------------------
void CFFWeaponDeploySentryGun::SecondaryAttack( void )
{
	if (gpGlobals->curtime > m_flNextSecondaryAttack + 0.1f && gpGlobals->curtime < m_flNextSecondaryAttack + 0.3f)
	{
#ifdef GAME_DLL
		CFFPlayer *ffplayer = ToFFPlayer(GetPlayerOwner());

		if (ffplayer)
		{
			CFFSentryGun *sg = dynamic_cast<CFFSentryGun *>(ffplayer->m_hSentryGun.Get());

			if (sg)
			{
				if ((ffplayer->GetAbsOrigin() - sg->GetAbsOrigin()).LengthSqr() < 6400.0f)
				{
					ffplayer->GiveAmmo(130.0f, AMMO_CELLS, true);
					sg->RemoveQuietly();
				}
				else
					sg->Detonate();

				// Bug #0000222: Removing SG with altfire(doubleclick) leaves teamcolored aimsphere.
				Cleanup_AimSphere();
			}
		}
#endif
	}

	m_flNextSecondaryAttack = gpGlobals->curtime;
}

//----------------------------------------------------------------------------
// Purpose: Checks validity of ground at this point or whatever
//----------------------------------------------------------------------------
void CFFWeaponDeploySentryGun::WeaponIdle()
{
	if( m_flTimeWeaponIdle < gpGlobals->curtime )
	{
		//SetWeaponIdleTime( gpGlobals->curtime + 0.1f );

#ifdef CLIENT_DLL 
		C_FFPlayer *pPlayer = GetPlayerOwner( );

		if (pPlayer->GetAmmoCount(AMMO_CELLS) < 130)
		{
			Cleanup();
		}
		// If we haven't built a sentrygun...
		else if( !pPlayer->m_hSentryGun.Get())
		{
			CFFBuildableInfo hBuildInfo(pPlayer, FF_BUILD_SENTRYGUN, FF_BUILD_SG_BUILD_DIST, FF_BUILD_SG_RAISE_VAL);

			if (hBuildInfo.BuildResult() == BUILD_ALLOWED)
			{
				if (pSentryGun)
				{
					// Sentry is currently hidden
					if (pSentryGun->GetEffects() & EF_NODRAW)
						pSentryGun->RemoveEffects(EF_NODRAW);

					// These were calculated in TryBuild()
					pSentryGun->SetAbsOrigin(hBuildInfo.GetBuildGroundOrigin());
					pSentryGun->SetAbsAngles(hBuildInfo.GetBuildGroundAngles());
				}
				else
					pSentryGun = C_FFSentryGun::CreateClientSideSentryGun(hBuildInfo.GetBuildGroundOrigin(), hBuildInfo.GetBuildGroundAngles());
			}
			// Unable to build, so hide buildable
			else if(pSentryGun && !(pSentryGun->GetEffects() & EF_NODRAW))
				pSentryGun->SetEffects(EF_NODRAW);
		}
		// If we have built a sentrygun...
		else
		{
			Cleanup( );

			// TODO: Draw range circle thing so the player can aim
			C_FFSentryGun *pActualSg = ( C_FFSentryGun * )pPlayer->m_hSentryGun.Get( );
			if( pActualSg )
			{
				// Player needs to be within x units of the SG & have line
				// of sight to the gun (just so you can't do this from another room
				// behind a wall or something...)
				
				Vector vecSGOrigin, vecPlayerOrigin;
				vecSGOrigin = pActualSg->GetAbsOrigin( );
				vecPlayerOrigin = pPlayer->GetAbsOrigin( );

				float flDist = vecSGOrigin.DistTo( vecPlayerOrigin );

				// Hopefully this trace will ignore stuff like players and things that don't
				// block LOS (as opposed to a WALL or something).
				trace_t tr;
				UTIL_TraceLine( vecPlayerOrigin + Vector( 0, 0, 48 ), vecSGOrigin + Vector( 0, 0, 48 ), MASK_OPAQUE, pPlayer, COLLISION_GROUP_NONE, &tr );

				bool bHaveLOS = false;

				// Not sure this stuff is needed actually, since the trace is always finishing (fraction = 1.0)
				// I guess I picked the right MASK_! :P Anyway, I'm leaving it in since it won't hurt
				// anything for now and I can always come back to it.
				if( tr.DidHit( ) && tr.m_pEnt )
				{
					// If the trace hit the SG and stopped
					if( ( C_FFSentryGun * )tr.m_pEnt == pActualSg )
						bHaveLOS = true;
				}
				else
				{
					// If the trace finished successfully (happened like 100% of the time
					// while testing - but I was just ducking behind walls and getting
					// a bot in my way. Dunno how it will work w/ grates and/or other ents).
					if( tr.fraction == 1.0 )
						bHaveLOS = true;
				}
				
				if(( flDist <= ( FF_SENTRYGUN_AIMSPHERE_VISIBLE ) ) && ( bHaveLOS ))
				{
					// Draw the aiming ring thing
					//DevMsg( "[Deploy SG] Trace passed, draw aiming ring thing\n" );

					if( !pAimSphere )
					{
						Color cColor;
						SetColorByTeam( pPlayer->GetTeamNumber( ) - 1, cColor );

						pAimSphere = C_FFSentryGun_AimSphere::CreateClientSentryGun_AimSphere( pActualSg->GetAbsOrigin(), QAngle( 0, 0, 0 ) );
						pAimSphere->SetRenderColorR( ( byte )cColor.r() );
						pAimSphere->SetRenderColorG( ( byte )cColor.g() );
						pAimSphere->SetRenderColorB( ( byte )cColor.b() );
					}
					
					/*
					CSingleUserRecipientFilter filter( pPlayer );
					te->BeamRingPoint( 
						filter, 1.0, vecSGOrigin,
						496, 
						512,	// twice the distance we're checking flDist to
						m_spriteTexture, 
						0,	// No halo
						0,
						5,	// framerate
						2,	// life
						8,	// width
						0,	// spread
						0,	// amplitude
						cColor.r( ),
						cColor.g( ),
						cColor.b( ),
						100,
						10
					);

					CSingleUserRecipientFilter filter2( pPlayer );
					te->BeamRingPoint( 
						filter2, 1.0, vecSGOrigin,
						240, 
						256,	// twice the distance we're checking flDist to
						m_spriteTexture, 
						0,	// No halo
						0,
						5,	// framerate
						2,	// life
						8,	// width
						0,	// spread
						0,	// amplitude
						cColor.r( ),
						cColor.g( ),
						cColor.b( ),
						100,
						10
					);
					*/
				}
				else
				{
					//DevMsg( "[Deploy SG] You gotta be standing closer to your gun and have line of sight to the gun!\n" );

					Cleanup_AimSphere();
				}
			}
		}
#endif
	}
}

bool CFFWeaponDeploySentryGun::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	Cleanup();
	Cleanup_AimSphere();

	return BaseClass::Holster( );
}
