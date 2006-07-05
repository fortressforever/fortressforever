//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ff_projectile_pipebomb.h"
#include "ff_weapon_base.h"
#include "ammodef.h"
#include "ai_debug_shared.h"
#include "shot_manipulator.h"

#ifdef CLIENT_DLL
	
	#include "c_ff_player.h"
	#define CRecipientFilter C_RecipientFilter	// |-- For PlayJumpSound

	extern void HudContextShow(bool visible);

#else

	#include "ff_player.h"
	#include "iservervehicle.h"
	#include "decals.h"

#endif

#include "gamevars_shared.h"
#include "takedamageinfo.h"
#include "effect_dispatch_data.h"
#include "engine/ivdebugoverlay.h"

ConVar sv_showimpacts("sv_showimpacts", "0", FCVAR_REPLICATED, "Shows client(red) and server(blue) bullet impact point");
ConVar sv_specchat("sv_spectatorchat", "0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Allows spectators to talk to players");

extern ConVar ai_debug_shoot_positions;

BEGIN_PREDICTION_DATA(CFFPlayer)
	DEFINE_PRED_FIELD(m_flCycle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK), 
	DEFINE_PRED_FIELD(m_iShotsFired, FIELD_INTEGER, FTYPEDESC_INSENDTABLE), 
END_PREDICTION_DATA()

void DispatchEffect(const char *pName, const CEffectData &data);

// Used to decide whether effects are allowed
static float g_flNextEffectAllowed[MAX_PLAYERS + 1];

bool AllowEffects(int iEntityIndex, float flNewDelay)
{
	if (iEntityIndex < 1 || iEntityIndex > MAX_PLAYERS)
		return true;

	if (g_flNextEffectAllowed[iEntityIndex - 1] < gpGlobals->curtime)
	{
		g_flNextEffectAllowed[iEntityIndex - 1] = gpGlobals->curtime + flNewDelay;
		return true;
	}
	return false;
}

void ClearAllowedEffects()
{
	memset(g_flNextEffectAllowed, 0, sizeof(g_flNextEffectAllowed));
}

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

			// Bug #0000671: Sniper rifle needs to cause more push upon hitting
			// Nothing fancy... 4.5 seemed to be about TFC's quick shot
			// and 8.5 seemed to be about TFC's full charge shot
			fScale = clamp( flSniperRifleCharge + 3.5f, 4.5f, 8.5f );

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
				// Bug #0000557: Teamplay 0 + sniper legshot slows allies
				// Don't apply the speed effect if the hit player is a teammate/ally

				// Slowed down by 10% - 60% depending on charge
				// Person hit by sniper rifle
				CFFPlayer *player = ToFFPlayer(tr.m_pEnt);

				// Person shooting the sniper rifle
				CFFPlayer *pShooter = ToFFPlayer(pevAttacker);

				// Bug #0000557: Teamplay 0 + sniper legshot slows allies
				// If they're not a teammate/ally then do the leg shot speed effect
				if (g_pGameRules->PlayerRelationship(pShooter, player) == GR_NOTTEAMMATE)
				{
					player->AddSpeedEffect(SE_LEGSHOT, 999, 0.9f - flSniperRifleCharge / 14.0f, SEM_ACCUMULATIVE|SEM_HEALABLE);

					// send them the status icon
					CSingleUserRecipientFilter user((CBasePlayer *) player);
					user.MakeReliable();
					UserMessageBegin(user, "StatusIconUpdate");
						WRITE_BYTE(FF_ICON_CALTROP);
						WRITE_FLOAT(15.0);
					MessageEnd();
				}
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

					// Revised further for
					// Bug: 0000620: Trace attacks aren't hitting walls

					// Mirv: Do impact traces no matter what
					if (pEntity /*&& pEntity->IsPlayer() */) //! (!friendlyfire.GetBool() && pEntity && pEntity->IsPlayer() && pEntity->GetTeamNumber() == GetTeamNumber()))
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
	// Remember last time idled
	m_flIdleTime = gpGlobals->curtime;

	if (!psurface)
		return;

	if (m_flJumpTime > gpGlobals->curtime)
		return;

	m_flJumpTime = gpGlobals->curtime + 0.4f;

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

	if (m_flFallTime > gpGlobals->curtime)
		return;

	m_flFallTime = gpGlobals->curtime + 0.4f;

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
	// Remember last time idled
	m_flIdleTime = gpGlobals->curtime;

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

int CFFPlayer::GetHealthPercentage()
{
	float flPerc;
	flPerc = ((float) GetHealth() / (float) GetMaxHealth()) * 100.0f;
	return (int) flPerc;
}

int CFFPlayer::GetArmorPercentage()
{
	float flPerc;
	flPerc = ((float) GetArmor() / (float) GetMaxArmor()) * 100.0f;
	return (int) flPerc;
}

/*
bool CFFPlayer::IsDisguised()
{
	return (GetClassSlot() == CLASS_SPY) && (m_iSpyDisguise != 0);
}

int CFFPlayer::GetDisguisedTeam()
{
	if (IsDisguised()) 	
		return (m_iSpyDisguise & 0x0000000F);

	return 0;
}

int CFFPlayer::GetDisguisedClass()
{
	if (IsDisguised())
		return ((m_iSpyDisguise & 0xFFFFFFF0) >> 4);

	return 0;
}
*/
/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.
================
*/
void CFFPlayer::FireBullets(const FireBulletsInfo_t &info)
{
	static int	tracerCount;
	trace_t		tr;
	CAmmoDef *	pAmmoDef	= GetAmmoDef();
	int			nDamageType	= pAmmoDef->DamageType(info.m_iAmmoType);
	int			nAmmoFlags	= pAmmoDef->Flags(info.m_iAmmoType);

	// Split the damage up into the number of shots
	float		flDmg = (info.m_iShots ? (float) info.m_iDamage / info.m_iShots : info.m_iDamage);

	// TODO: Should this be false in our mod too?
	bool bDoServerEffects = true;

	// This allows us to specify ourselves what damage we do to players
	int iPlayerDamage = info.m_iPlayerDamage;

	if (iPlayerDamage == 0)
	{
		if (nAmmoFlags & AMMO_INTERPRET_PLRDAMAGE_AS_DAMAGE_TO_PLAYER)
			iPlayerDamage = pAmmoDef->PlrDamage(info.m_iAmmoType);
	}

	// The default attacker is ourselves
	CBaseEntity *pAttacker = info.m_pAttacker ? info.m_pAttacker : this;

	// Make sure we don't have a dangling damage target from a recursive call
	if (g_MultiDamage.GetTarget() != NULL)
		ApplyMultiDamage();

	// Some cleanup stuff
	ClearMultiDamage();
	g_MultiDamage.SetDamageType(nDamageType | DMG_NEVERGIB);

	Vector vecDir, vecEnd;

	// Count bullets as projectiles so that they don't hit weapon bags
	CTraceFilterSkipTwoEntities traceFilter(this, info.m_pAdditionalIgnoreEnt, /*COLLISION_GROUP_NONE*/ COLLISION_GROUP_PROJECTILE);

	// Did bullet start underwater?
	bool bStartedInWater = (enginetrace->GetPointContents(info.m_vecSrc) & (CONTENTS_WATER|CONTENTS_SLIME)) != 0;

	int iSeed = 0;

	// Prediction is only usable on players
	if (IsPlayer())
		iSeed = CBaseEntity::GetPredictionRandomSeed() & 255;

	// Remember to enable this if we change bDoServerEffects
#if defined(HL2MP) && defined(GAME_DLL)
	int iEffectSeed = iSeed;
#endif

	//-----------------------------------------------------
	// Set up our shot manipulator.
	//-----------------------------------------------------
	CShotManipulator Manipulator(info.m_vecDirShooting);

	bool bDoImpacts = false;
	bool bDoTracers = false;

	bool bDoEffects = AllowEffects(entindex(), 0.3f);

	// Now simulate each shot
	for (int iShot = 0; iShot < info.m_iShots; iShot++)
	{
		bool bHitWater = false;
		bool bHitGlass = false;

		// Prediction is only usable on players
		// Init random system with this seed
		if (IsPlayer())
			RandomSeed(iSeed);

		// If we're firing multiple shots, and the first shot has to be bang on target, ignore spread
		// TODO: Possibly also dot his when m_iShots == 1
		if (iShot == 0 && info.m_iShots > 1 && (info.m_nFlags & FIRE_BULLETS_FIRST_SHOT_ACCURATE))
			vecDir = Manipulator.GetShotDirection();

		// Don't run the biasing code for the player at the moment.
		else
			vecDir = Manipulator.ApplySpread(info.m_vecSpread);

		vecEnd = info.m_vecSrc + vecDir * info.m_flDistance;

		if (IsPlayer() && /*info.m_iShots > 1 &&*/ (iShot % 2) == 0)
		{
			// Half of the shotgun pellets are hulls that make it easier to hit targets with the shotgun.
			AI_TraceHull(info.m_vecSrc, vecEnd, Vector(-3, -3, -3), Vector(3, 3, 3), MASK_SHOT, &traceFilter, &tr);
		}
		else
		{
			// But half aren't
			AI_TraceLine(info.m_vecSrc, vecEnd, MASK_SHOT, &traceFilter, &tr);
		}

#ifdef GAME_DLL
		// Handy debug stuff, I guess
		if (ai_debug_shoot_positions.GetBool())
			NDebugOverlay::Line(info.m_vecSrc, vecEnd, 255, 255, 255, false, .1);
#endif

		// Has this particular bullet hit water yet
		bHitWater = bStartedInWater;

		// Now hit all triggers along the ray that respond to shots...
		// Clip the ray to the first collided solid returned from traceline
		CTakeDamageInfo triggerInfo(pAttacker, pAttacker, /*info.m_iDamage */flDmg, nDamageType); // |-- Mirv: Split damage into shots
		CalculateBulletDamageForce(&triggerInfo, info.m_iAmmoType, vecDir, tr.endpos);
		triggerInfo.ScaleDamageForce(info.m_flDamageForceScale);
		triggerInfo.SetAmmoType(info.m_iAmmoType);
#ifdef GAME_DLL
		TraceAttackToTriggers(triggerInfo, tr.startpos, tr.endpos, vecDir);
#endif

		// Make sure given a valid bullet type
		if (info.m_iAmmoType == -1)
		{
			DevMsg("ERROR: Undefined ammo type!\n");
			return;
		}

		Vector vecTracerDest = tr.endpos;

		// Do damage, paint decals
		if (tr.fraction != 1.0)
		{
			// See if the bullet ended up underwater + started out of the water
			if (!bHitWater && (enginetrace->GetPointContents(tr.endpos) & (CONTENTS_WATER|CONTENTS_SLIME)))
			{
				// Only the first shot will do a splash effect, and only if effects
				// are enabled for this burst
				if (iShot == 0 && bDoEffects)
					bHitWater = HandleShotImpactingWater(info, vecEnd, &traceFilter, &vecTracerDest);

				// However, still do the test for bullet impacts
				else
				{
					trace_t	waterTrace;
					AI_TraceLine(info.m_vecSrc, vecEnd, (MASK_SHOT|CONTENTS_WATER|CONTENTS_SLIME), &traceFilter, &waterTrace);

					// See if this is the point we entered
					if ((enginetrace->GetPointContents(waterTrace.endpos - Vector(0, 0, 0.1f)) & (CONTENTS_WATER|CONTENTS_SLIME)) == 0)
						bHitWater = true;
				}
			}

			// Probably can move this
			float flActualDamage = /*info.m_iDamage */ flDmg;

			// If we hit a player, and we have player damage specified, use that instead
			// Adrian: Make sure to use the currect value if we hit a vehicle the player is currently driving.
			if (iPlayerDamage)
			{
				if (tr.m_pEnt->IsPlayer())
					flActualDamage = iPlayerDamage;
#ifdef GAME_DLL
				else if (tr.m_pEnt->GetServerVehicle())
				{
					if (tr.m_pEnt->GetServerVehicle()->GetPassenger() && tr.m_pEnt->GetServerVehicle()->GetPassenger()->IsPlayer())
						flActualDamage = iPlayerDamage;
				}
#endif
			}

			// Now some more damage stuff
			int nActualDamageType = nDamageType;
			if (flActualDamage == 0.0)
			{
				flActualDamage = g_pGameRules->GetAmmoDamage(pAttacker, tr.m_pEnt, info.m_iAmmoType);
			}
			else
			{
				nActualDamageType = nDamageType | ((flActualDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB);
			}

			// Now do the impacts from this shot
			if (!bHitWater || ((info.m_nFlags & FIRE_BULLETS_DONT_HIT_UNDERWATER) == 0))
			{
				// Damage specified by function parameter
				CTakeDamageInfo dmgInfo(this, pAttacker, flActualDamage, nActualDamageType);
				CalculateBulletDamageForce(&dmgInfo, info.m_iAmmoType, vecDir, tr.endpos);
				dmgInfo.ScaleDamageForce(info.m_flDamageForceScale);
				dmgInfo.SetAmmoType(info.m_iAmmoType);
				tr.m_pEnt->DispatchTraceAttack(dmgInfo, vecDir, &tr);

				if (bStartedInWater || !bHitWater || (info.m_nFlags & FIRE_BULLETS_ALLOW_WATER_SURFACE_IMPACTS))
				{
					if (bDoServerEffects)
					{
						// Is the entity valid, and the surface drawable on?
						if (tr.fraction < 1.0f && tr.m_pEnt && !(tr.surface.flags & (SURF_SKY|SURF_NODRAW)))
						{
							// Build the impact data
							CEffectData data;
							data.m_vOrigin = tr.endpos;
							data.m_vStart = tr.startpos;
							data.m_nSurfaceProp = tr.surface.surfaceProps;
							data.m_nDamageType = nDamageType;
							data.m_nHitBox = tr.hitbox;
							data.m_nEntIndex = tr.m_pEnt->entindex();

							// No impact effects for most of the shots or possible
							// this entire burst
							if (iShot > 2 || !bDoEffects)
								data.m_fFlags |= CEFFECT_EFFECTNOTNEEDED;

							// No sound for all but the first few
							if (iShot > 2)
								data.m_fFlags |= CEFFECT_SOUNDNOTNEEDED;

							// Send it off
							DispatchEffect("Impact", data);
						}
					}
					else
						bDoImpacts = true;
				}
				else
				{
					// We may not impact, but we DO need to affect ragdolls on the client
					CEffectData data;
					data.m_vStart = tr.startpos;
					data.m_vOrigin = tr.endpos;
					data.m_nDamageType = nDamageType;

					DispatchEffect("RagdollImpact", data);
				}

#ifdef GAME_DLL
				// Make sure if the player is holding this, he drops it
				if (nAmmoFlags & AMMO_FORCE_DROP_IF_CARRIED)
					Pickup_ForcePlayerToDropThisObject(tr.m_pEnt);		
#endif
			}
		}

		// See if we hit glass
		if (tr.m_pEnt != NULL)
		{
#ifdef GAME_DLL
			surfacedata_t *psurf = physprops->GetSurfaceData(tr.surface.surfaceProps);
			if ((psurf != NULL) && (psurf->game.material == CHAR_TEX_GLASS) && (tr.m_pEnt->ClassMatches("func_breakable")))
				bHitGlass = true;
#endif
		}

		// Do the tracers if required
		if ((info.m_iTracerFreq != 0) && (tracerCount++ % info.m_iTracerFreq) == 0 && (bHitGlass == false))
		{
			if (bDoServerEffects)
			{
				Vector vecTracerSrc = vec3_origin;
				ComputeTracerStartPosition(info.m_vecSrc, &vecTracerSrc);

				trace_t Tracer;
				Tracer = tr;
				Tracer.endpos = vecTracerDest;

				MakeTracer(vecTracerSrc, Tracer, pAmmoDef->TracerType(info.m_iAmmoType));
			}
			else
				bDoTracers = true;
		}

		// See if we should pass through glass
#ifdef GAME_DLL
		if (bHitGlass)
			HandleShotImpactingGlass(info, tr, vecDir, &traceFilter);
#endif

		iSeed++;
	}

	// Client side effects
#if defined(HL2MP) && defined(GAME_DLL)
	if (!bDoServerEffects && bDoEffects)
		TE_HL2MPFireBullets(entindex(), tr.startpos, info.m_vecDirShooting, info.m_iAmmoType, iEffectSeed, info.m_iShots, info.m_vecSpread.x, bDoTracers, bDoImpacts);
#endif

#ifdef GAME_DLL
	ApplyMultiDamage();
#endif
}

bool CFFPlayer::HandleShotImpactingWater(const FireBulletsInfo_t &info, const Vector &vecEnd, ITraceFilter *pTraceFilter, Vector *pVecTracerDest)
{
	trace_t	waterTrace;

	// Trace again with water enabled
	AI_TraceLine(info.m_vecSrc, vecEnd, (MASK_SHOT|CONTENTS_WATER|CONTENTS_SLIME), pTraceFilter, &waterTrace);

	// See if this is the point we entered
	if ((enginetrace->GetPointContents(waterTrace.endpos - Vector(0, 0, 0.1f)) & (CONTENTS_WATER|CONTENTS_SLIME)) == 0)
		return false;

	if (ShouldDrawWaterImpacts())
	{
		int	nMinSplashSize = GetAmmoDef()->MinSplashSize(info.m_iAmmoType);
		int	nMaxSplashSize = GetAmmoDef()->MaxSplashSize(info.m_iAmmoType);

		float flSplashModifier = 1.0f + info.m_iShots * 0.1f;	// |-- Mirv: Modify splashes by shot count

		CEffectData	data;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = random->RandomFloat(nMinSplashSize, nMaxSplashSize) * flSplashModifier;	// |-- Mirv: Modify splashes by shot count
		if (waterTrace.contents & CONTENTS_SLIME)
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}
		DispatchEffect("gunshotsplash", data);
	}

	*pVecTracerDest = waterTrace.endpos;
	return true;
}
