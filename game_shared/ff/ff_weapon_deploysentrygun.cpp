//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_weapon_deploysentrygun.cpp
//	@author Patrick O'Leary(Mulchman) 
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
//	09/20/2005, 	Mulchman:
//		Added aiming ring.
//
//	09/24/2005, 	Mulchman:
//		Added a model for the aiming ring.
//
//	03/17/06, Mulchman:
//		Bug #0000333: Buildable Behavior (non build slot) while building fixes
//		Removing Aim Sphere

#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponDeploySentryGun C_FFWeaponDeploySentryGun
	#define CFFSentryGun C_FFSentryGun

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
	DECLARE_CLASS(CFFWeaponDeploySentryGun, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponDeploySentryGun();
#ifdef CLIENT_DLL 
	~CFFWeaponDeploySentryGun() 
	{ 
		Cleanup(); 
		//Cleanup_AimSphere();
	}
#endif

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void WeaponIdle();
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual bool CanBeSelected();

	virtual FFWeaponID GetWeaponID() const		{ return FF_WEAPON_DEPLOYSENTRYGUN; }

private:

	CFFWeaponDeploySentryGun(const CFFWeaponDeploySentryGun &);

#ifdef CLIENT_DLL
protected:
	//int	m_spriteTexture;
	CFFSentryGun *pSentryGun;
	//C_FFSentryGun_AimSphere *pAimSphere;
#endif

	void Cleanup() 
	{
#ifdef CLIENT_DLL
		if (pSentryGun) 
		{
			pSentryGun->Remove();
			pSentryGun = NULL;
		}
#endif
	}

//	void Cleanup_AimSphere() 
//	{
//#ifdef CLIENT_DLL
//		if (pAimSphere) 
//		{
//			pAimSphere->Remove();
//			pAimSphere = NULL;
//		}
//#endif
//	}
};

//=============================================================================
// CFFWeaponDeploySentryGun tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponDeploySentryGun, DT_FFWeaponDeploySentryGun) 

BEGIN_NETWORK_TABLE(CFFWeaponDeploySentryGun, DT_FFWeaponDeploySentryGun) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponDeploySentryGun) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_deploysentrygun, CFFWeaponDeploySentryGun);
PRECACHE_WEAPON_REGISTER(ff_weapon_deploysentrygun);

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
	//pAimSphere = NULL;

	//m_spriteTexture = PrecacheModel("sprites/lgtning.vmt");
#endif
}

//----------------------------------------------------------------------------
// Purpose: Handles whatever should be done when they fire(build, aim, etc) 
//----------------------------------------------------------------------------
void CFFWeaponDeploySentryGun::PrimaryAttack() 
{
	if (m_flNextPrimaryAttack < gpGlobals->curtime) 
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

		Cleanup();
		//Cleanup_AimSphere();

#ifdef GAME_DLL
		//CFFPlayer *player = GetPlayerOwner();
		//CFFSentryGun *sg = dynamic_cast<CFFSentryGun *> (player->m_hSentryGun.Get());

		//if (!sg) 
			GetPlayerOwner()->Command_BuildSentryGun();
#endif
	}
}

//----------------------------------------------------------------------------
// Purpose: Handles whatever should be done when they secondary fire
//----------------------------------------------------------------------------
void CFFWeaponDeploySentryGun::SecondaryAttack() 
{
	if( m_flNextSecondaryAttack < gpGlobals->curtime )
	{
		m_flNextSecondaryAttack = gpGlobals->curtime;
	}
}

//----------------------------------------------------------------------------
// Purpose: Checks validity of ground at this point or whatever
//----------------------------------------------------------------------------
void CFFWeaponDeploySentryGun::WeaponIdle() 
{
	if (m_flTimeWeaponIdle < gpGlobals->curtime) 
	{
		//SetWeaponIdleTime(gpGlobals->curtime + 0.1f);

#ifdef CLIENT_DLL 
		C_FFPlayer *pPlayer = GetPlayerOwner();

		if (pPlayer->GetAmmoCount(AMMO_CELLS) < 130) 
		{
			Cleanup();
		}
		// If we haven't built a sentrygun...
		else if (!pPlayer->m_hSentryGun.Get()) 
		{
			//Cleanup_AimSphere();

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
					pSentryGun = CFFSentryGun::CreateClientSideSentryGun(hBuildInfo.GetBuildGroundOrigin(), hBuildInfo.GetBuildGroundAngles());
			}
			// Unable to build, so hide buildable
			else if (pSentryGun && ! (pSentryGun->GetEffects() & EF_NODRAW)) 
				pSentryGun->SetEffects(EF_NODRAW);
		}
		// If we have built a sentrygun...
		else
		{
			Cleanup();

			// Bug #0000333: Buildable Behavior (non build slot) while building
			pPlayer->SwapToWeapon( FF_WEAPON_SPANNER );

			// TODO: Draw range circle thing so the player can aim
//			CFFSentryGun *pActualSg = (CFFSentryGun *) pPlayer->m_hSentryGun.Get();
//
//			if (pActualSg) 
//			{
//				// Player needs to be within x units of the SG & have line
//				// of sight to the gun(just so you can't do this from another room
//				// behind a wall or something...) 
//				
//				Vector vecSGOrigin, vecPlayerOrigin;
//				vecSGOrigin = pActualSg->GetAbsOrigin();
//				vecPlayerOrigin = pPlayer->GetAbsOrigin();
//
//				float flDist = vecSGOrigin.DistTo(vecPlayerOrigin);
//
//				// Hopefully this trace will ignore stuff like players and things that don't
//				// block LOS(as opposed to a WALL or something).
//				trace_t tr;
//				UTIL_TraceLine(vecPlayerOrigin + Vector(0, 0, 48), vecSGOrigin + Vector(0, 0, 48), MASK_OPAQUE, pPlayer, COLLISION_GROUP_NONE, &tr);
//
//				bool bHaveLOS = false;
//
//				// Not sure this stuff is needed actually, since the trace is always finishing(fraction = 1.0) 
//				// I guess I picked the right MASK_! :P Anyway, I'm leaving it in since it won't hurt
//				// anything for now and I can always come back to it.
//				if (tr.DidHit() && tr.m_pEnt) 
//				{
//					// If the trace hit the SG and stopped
//					if ((CFFSentryGun *) tr.m_pEnt == pActualSg) 
//						bHaveLOS = true;
//				}
//				else
//				{
//					// If the trace finished successfully(happened like 100% of the time
//					// while testing - but I was just ducking behind walls and getting
//					// a bot in my way. Dunno how it will work w/ grates and/or other ents).
//					if (tr.fraction == 1.0) 
//						bHaveLOS = true;
//				}
//				
//				if ((flDist <= (FF_SENTRYGUN_AIMSPHERE_VISIBLE)) && (bHaveLOS)) 
//				{
//					if (!pAimSphere) 
//					{
//						Color cColor;
//						SetColorByTeam(pPlayer->GetTeamNumber() - 1, cColor);
//
//						pAimSphere = C_FFSentryGun_AimSphere::CreateClientSentryGun_AimSphere(pActualSg->GetAbsOrigin(), QAngle(0, 0, 0));
//						pAimSphere->SetRenderColorR((byte) cColor.r());
//						pAimSphere->SetRenderColorG((byte) cColor.g());
//						pAimSphere->SetRenderColorB((byte) cColor.b());
//					}
//				}
//				else
//					Cleanup_AimSphere();
//			}
		}
#endif
	}
}

bool CFFWeaponDeploySentryGun::Holster(CBaseCombatWeapon *pSwitchingTo) 
{
	Cleanup();
	//Cleanup_AimSphere();

	return BaseClass::Holster( pSwitchingTo );
}

bool CFFWeaponDeploySentryGun::CanBeSelected()
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	if (pPlayer && ((CFFSentryGun *) pPlayer->m_hSentryGun.Get()))
		return false;
	// Bug #0000333: Buildable Behavior (non build slot) while building
	else if( pPlayer->m_bBuilding )
		return false;
	// Bug #0000333: Buildable Behavior (non build slot) while building
	else if( pPlayer->GetAmmoCount( AMMO_CELLS ) < 130 )
		return false;
	else
		return BaseClass::CanBeSelected();
}

#ifdef GAME_DLL
	//=============================================================================
	// Commands
	//=============================================================================
	CON_COMMAND(aimsentry, "Aim sentrygun")
	{
		CFFPlayer *pPlayer = ToFFPlayer(UTIL_GetCommandClient());

		if (!pPlayer) 
			return;

		// Bug #0000333: Buildable Behavior (non build slot) while building
		if( pPlayer->m_bBuilding && ( pPlayer->m_iCurBuild == FF_BUILD_SENTRYGUN ) )
			return;

		CFFSentryGun *pSentry = dynamic_cast<CFFSentryGun *> (pPlayer->m_hSentryGun.Get());

		if (!pSentry) 
			return;

		Vector vecForward;
		AngleVectors(pPlayer->EyeAngles(), &vecForward);

		trace_t tr;
		UTIL_TraceLine(pPlayer->Weapon_ShootPosition(), pPlayer->Weapon_ShootPosition() + vecForward * MAX_TRACE_LENGTH, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

		pSentry->SetFocusPoint(tr.endpos);
	}

	CON_COMMAND(dismantlesentry, "Dismantle sentrygun") 
	{
		CFFPlayer *pPlayer = ToFFPlayer(UTIL_GetCommandClient());

		if (!pPlayer) 
			return;

		// Bug #0000333: Buildable Behavior (non build slot) while building
		if( pPlayer->m_bBuilding && ( pPlayer->m_iCurBuild == FF_BUILD_SENTRYGUN ) )
			return;

		CFFSentryGun *pSentry = dynamic_cast<CFFSentryGun *> (pPlayer->m_hSentryGun.Get());

		if (!pSentry) 
			return;

		// Close enough to dismantle
		if ((pPlayer->GetAbsOrigin() - pSentry->GetAbsOrigin()).LengthSqr() < 6400.0f) 
		{
			pPlayer->GiveAmmo(pSentry->GetLevel() * 65, AMMO_CELLS, true);
			pSentry->RemoveQuietly();
		}
		else
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_TOOFARAWAY");
	}

	CON_COMMAND(detsentry, "Detonates sentrygun") 
	{
		CFFPlayer *pPlayer = ToFFPlayer(UTIL_GetCommandClient());

		if (!pPlayer) 
			return;

		// Bug #0000333: Buildable Behavior (non build slot) while building
		if( pPlayer->m_bBuilding && ( pPlayer->m_iCurBuild == FF_BUILD_SENTRYGUN ) )
			return;

		CFFSentryGun *pSentry = dynamic_cast<CFFSentryGun *> (pPlayer->m_hSentryGun.Get());

		if (!pSentry) 
			return;

		pSentry->Detonate();
	}

	CON_COMMAND(detdismantlesentry, "Dismantles or detonate sentrygun depending on distance") 
	{
		CFFPlayer *pPlayer = ToFFPlayer(UTIL_GetCommandClient());

		if (!pPlayer) 
			return;

		// Bug #0000333: Buildable Behavior (non build slot) while building
		if( pPlayer->m_bBuilding && ( pPlayer->m_iCurBuild == FF_BUILD_SENTRYGUN ) )
			return;

		CFFSentryGun *pSentry = dynamic_cast<CFFSentryGun *> (pPlayer->m_hSentryGun.Get());

		if (!pSentry) 
			return;

		// Close enough to dismantle
		if ((pPlayer->GetAbsOrigin() - pSentry->GetAbsOrigin()).LengthSqr() < 6400.0f) 
		{
			pPlayer->GiveAmmo(pSentry->GetLevel() * 65, AMMO_CELLS, true);
			pSentry->RemoveQuietly();
		}
		else
			pSentry->Detonate();
	}
#endif