//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ff_projectile_pipebomb.h"
#include "ff_weapon_base.h"

#ifdef CLIENT_DLL
	
	#include "c_ff_player.h"
	#define CRecipientFilter C_RecipientFilter	// |-- For PlayJumpSound

	extern void HudContextShow(bool visible);

#else

	#include "ff_player.h"

#endif

#include "gamevars_shared.h"
#include "takedamageinfo.h"
#include "effect_dispatch_data.h"
#include "engine/ivdebugoverlay.h"

ConVar sv_showimpacts("sv_showimpacts", "0", FCVAR_REPLICATED, "Shows client(red) and server(blue) bullet impact point");

BEGIN_PREDICTION_DATA(CFFPlayer) 
	DEFINE_PRED_FIELD(m_flCycle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK), 
	DEFINE_PRED_FIELD(m_iShotsFired, FIELD_INTEGER, FTYPEDESC_INSENDTABLE), 
END_PREDICTION_DATA() 

void DispatchEffect(const char *pName, const CEffectData &data);

CFFWeaponBase * CFFPlayer::FFAnim_GetActiveWeapon() 
{
	return GetActiveFFWeapon();
}

bool CFFPlayer::FFAnim_CanMove() 
{
	return true;
}

void CFFPlayer::FireBullet(
						   Vector vecSrc, 	// shooting postion
						   const QAngle &shootAngles, //shooting angle
						   float vecSpread, // spread vector
						   float flDamage, // base damage		// |-- Mirv: Floating damage
						   int iBulletType, // ammo type
						   CBaseEntity *pevAttacker, // shooter
						   bool bDoEffects, 	// create impact effect ?
						   float x, 	// spread x factor
						   float y, 	// spread y factor
						   float flSniperRifleCharge // added by Mulchman 9/20/2005
						) 
{
	float fCurrentDamage = flDamage;   // damage of the bullet at it's current trajectory
	float fScale = 1.0f;			// scale the force
	float flCurrentDistance = 0.0;  //distance that the bullet has traveled so far

	Vector vecDirShooting, vecRight, vecUp;
	AngleVectors(shootAngles, &vecDirShooting, &vecRight, &vecUp);

	if (!pevAttacker) 
		pevAttacker = this;  // the default attacker is ourselves

	// add the spray 
	Vector vecDir = vecDirShooting +
		x * vecSpread * vecRight +
		y * vecSpread * vecUp;

	VectorNormalize(vecDir);

	float flMaxRange = 8000;

	Vector vecEnd = vecSrc + vecDir * flMaxRange; // max bullet range is 10000 units

	trace_t tr; // main enter bullet trace

	UTIL_TraceLine(vecSrc, vecEnd, MASK_SOLID|CONTENTS_DEBRIS|CONTENTS_HITBOX, this, /*COLLISION_GROUP_NONE */ COLLISION_GROUP_PROJECTILE, &tr);	// |-- Mirv: Count bullets as projectiles so they don't hit weapon bags

	if (tr.fraction == 1.0f) 
		return; // we didn't hit anything, stop tracing shoot

	if (sv_showimpacts.GetBool()) 
	{
#ifdef CLIENT_DLL
		// draw red client impact markers
		debugoverlay->AddBoxOverlay(tr.endpos, Vector(-2, -2, -2), Vector(2, 2, 2), QAngle(0, 0, 0), 255, 0, 0, 127, 4);

		if (tr.m_pEnt && tr.m_pEnt->IsPlayer()) 
		{
			C_BasePlayer *player = ToBasePlayer(tr.m_pEnt);
			player->DrawClientHitboxes(4, true);
		}
#else
		// draw blue server impact markers
		NDebugOverlay::Box(tr.endpos, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 0, 255, 127, 4);

		if (tr.m_pEnt && tr.m_pEnt->IsPlayer()) 
		{
			CBasePlayer *player = ToBasePlayer(tr.m_pEnt);
			player->DrawServerHitboxes(4, true);
		}
#endif
	}

		//calculate the damage based on the distance the bullet travelled.
		flCurrentDistance += tr.fraction * flMaxRange;

		// damage get weaker of distance
		//fCurrentDamage *= pow(0.85f, (flCurrentDistance / 500));	// |-- Mirv: Distance doesnt affect sniper rifle

		// --> Mirv: Locational damage

		// Only if this is a charged shot
		if (flSniperRifleCharge) 
		{
			fCurrentDamage *= flSniperRifleCharge;
			fScale *= flSniperRifleCharge;

			if (tr.hitgroup == HITGROUP_HEAD) 
			{
				DevMsg("Headshot\n");
				fCurrentDamage *= 7.5f;
			}
			else if (tr.hitgroup == HITGROUP_LEFTLEG || tr.hitgroup == HITGROUP_RIGHTLEG) 
			{
				DevMsg("Legshot\n");
				fCurrentDamage *= 0.5f;

#ifdef GAME_DLL
				// Slowed down by 10% - 60% depending on charge
				CFFPlayer *player = ToFFPlayer(tr.m_pEnt);
				player->AddSpeedEffect(SE_LEGSHOT, 999, 0.9f - flSniperRifleCharge / 14.0f, SEM_ACCUMULATIVE|SEM_HEALABLE);

				// send them the status icon
				CSingleUserRecipientFilter user((CBasePlayer *) player);
				user.MakeReliable();
				UserMessageBegin(user, "StatusIconUpdate");
					WRITE_BYTE(FF_ICON_CALTROP);
					WRITE_FLOAT(15.0);
				MessageEnd();
#endif
			}			
		}
		// --> Mirv: Locational damage

		int iDamageType = DMG_BULLET | DMG_NEVERGIB;

		if (bDoEffects) 
		{
			// See if the bullet ended up underwater + started out of the water
			if (enginetrace->GetPointContents(tr.endpos) & (CONTENTS_WATER|CONTENTS_SLIME)) 
			{	
				trace_t waterTrace;
				UTIL_TraceLine(vecSrc, tr.endpos, (MASK_SHOT|CONTENTS_WATER|CONTENTS_SLIME), this, COLLISION_GROUP_NONE, &waterTrace);

				if (waterTrace.allsolid != 1) 
				{
					CEffectData	data;
					data.m_vOrigin = waterTrace.endpos;
					data.m_vNormal = waterTrace.plane.normal;
					data.m_flScale = random->RandomFloat(8, 12);

					if (waterTrace.contents & CONTENTS_SLIME) 
					{
						data.m_fFlags |= FX_WATER_IN_SLIME;
					}

					DispatchEffect("gunshotsplash", data);
				}
			}
			else
			{
				//Do Regular hit effects

				// Don't decal nodraw surfaces
				if (! (tr.surface.flags & (SURF_SKY|SURF_NODRAW|SURF_HINT|SURF_SKIP))) 
				{
					CBaseEntity *pEntity = tr.m_pEnt;
					if (! (!friendlyfire.GetBool() && pEntity && pEntity->IsPlayer() && pEntity->GetTeamNumber() == GetTeamNumber())) 
					{
						UTIL_ImpactTrace(&tr, iDamageType);
					}
				}
			}
		} // bDoEffects

		// add damage to entity that we hit

		ClearMultiDamage();

		CTakeDamageInfo info(pevAttacker, pevAttacker, fCurrentDamage, iDamageType);	// |-- Mirv: Modified this

		// for radio tagging and to make ammo type work in the DamageFunctions
		info.SetAmmoType(iBulletType);

		CalculateBulletDamageForce(&info, iBulletType, vecDir, tr.endpos, fScale);	// |-- Mirv: Modified this
		tr.m_pEnt->DispatchTraceAttack(info, vecDir, &tr);

		// Bug #0000168: Blood sprites for damage on players do not display
#ifdef GAME_DLL
		TraceAttackToTriggers(info, tr.startpos, tr.endpos, vecDir);
#endif

		ApplyMultiDamage();
}

// --> Mirv: Proper sounds
void CFFPlayer::PlayJumpSound(Vector &vecOrigin, surfacedata_t *psurface, float fvol) 
{
	if (!psurface) 
		return;

	// If we want different jump sounds for material...
//	IPhysicsSurfaceProps *physprops = MoveHelper()->GetSurfaceProps();
//	const char *pSoundName = physprops->GetString(stepSoundName);
//	CSoundParameters params;
//	if (!CBaseEntity::GetParametersForSound(pSoundName, params, NULL)) 
//		return;

	CRecipientFilter filter;
	filter.AddRecipientsByPAS(vecOrigin);

#ifndef CLIENT_DLL
	// Jump sounds done clientside for everybody who can see the models
	if (gpGlobals->maxClients > 1) 
		filter.RemoveRecipientsByPVS(vecOrigin);
#endif

	EmitSound_t ep;
	ep.m_nChannel = CHAN_BODY;
	ep.m_pSoundName = "Player.Jump"; //params.soundname;
	ep.m_flVolume = fvol;
	ep.m_SoundLevel = SNDLVL_70dB; // params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = PITCH_NORM; // params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound(filter, entindex(), ep);
}

void CFFPlayer::PlayFallSound(Vector &vecOrigin, surfacedata_t *psurface, float fvol) 
{
	if (!psurface) 
		return;

	// Kill sound if we're a falling spy
	if (GetClassSlot() == 8) 
		return;

	// If we want different jump sounds for material...
//	IPhysicsSurfaceProps *physprops = MoveHelper()->GetSurfaceProps();
//	const char *pSoundName = physprops->GetString(stepSoundName);
//	CSoundParameters params;
//	if (!CBaseEntity::GetParametersForSound(pSoundName, params, NULL)) 
//		return;

	CRecipientFilter filter;
	filter.AddRecipientsByPAS(vecOrigin);

#ifndef CLIENT_DLL
	// Jump sounds done clientside for everybody who can see the models
	if (gpGlobals->maxClients > 1) 
		filter.RemoveRecipientsByPVS(vecOrigin);
#endif

	EmitSound_t ep;
	ep.m_nChannel = CHAN_BODY;
	ep.m_pSoundName = "Player.fall"; //params.soundname;
	ep.m_flVolume = fvol;
	ep.m_SoundLevel = SNDLVL_70dB; // params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = PITCH_NORM; // params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound(filter, entindex(), ep);
}

void CFFPlayer::PlayStepSound(Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force) 
{
	// Don't play footsteps for spy
	if (GetClassSlot() != 8) 
		BaseClass::PlayStepSound(vecOrigin, psurface, fvol, force);
}
// <-- Mirv: Proper sounds

//-----------------------------------------------------------------------------
// Purpose: Handle all class specific skills
//-----------------------------------------------------------------------------
void CFFPlayer::ClassSpecificSkill() 
{
	if (m_flNextClassSpecificSkill > gpGlobals->curtime) 
		return;

	switch (GetClassSlot()) 
	{
#ifdef GAME_DLL
	case CLASS_DEMOMAN:
		CFFProjectilePipebomb::DestroyAllPipes(this);
		break;
#endif

#ifdef CLIENT_DLL
		case CLASS_HWGUY:
			SwapToWeapon(FF_WEAPON_ASSAULTCANNON);
			break;

		case CLASS_MEDIC:
			SwapToWeapon(FF_WEAPON_MEDKIT);
			break;

		case CLASS_PYRO:
			SwapToWeapon(FF_WEAPON_FLAMETHROWER);
			break;

		case CLASS_SOLDIER:
			engine->ClientCmd("+reload");
			break;

		case CLASS_ENGINEER:
		case CLASS_SPY:
			HudContextShow(true);
			break;

		case CLASS_SCOUT:
			engine->ClientCmd("radar");
			break;

#endif
		default:
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Anything to do after they've stopped pressing
//-----------------------------------------------------------------------------
void CFFPlayer::ClassSpecificSkill_Post() 
{
#ifdef CLIENT_DLL
	switch (GetClassSlot()) 
	{

	case CLASS_SOLDIER:
		engine->ClientCmd("-reload");
		break;

	case CLASS_ENGINEER:
	case CLASS_SPY:
		HudContextShow(false);
		break;

	default:
		break;
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: TFC style man!
//-----------------------------------------------------------------------------
Vector CFFPlayer::GetLegacyAbsOrigin()
{
	return GetAbsOrigin() + (FBitSet(GetFlags(), FL_DUCKING) ? Vector(0, 0, 16.0f) : Vector(0, 0, 36.0f));
}

int CFFPlayer::GetHealthPercentage( void )
{
	float flPerc;
	flPerc = ( ( float )GetHealth() / ( float )GetMaxHealth() ) * 100.0f;
	return ( int )flPerc;
}

int CFFPlayer::GetArmorPercentage( void )
{
	float flPerc;
	flPerc = ( ( float )GetArmor() / ( float )GetMaxArmor() ) * 100.0f;
	return ( int )flPerc;
}
