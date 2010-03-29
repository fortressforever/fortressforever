/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_grenade_hoverturret.cpp
/// @author David "Mervaka" Cook
/// @date March 18, 2010
/// @brief The FF laser grenade code.
///
/// REVISIONS
/// ---------
/// Mar 29, 2005	Mervaka: First created, based on ff_grenade_nail.cpp by Mirvin_Monkey

//========================================================================
// Required includes
//========================================================================
#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"
#include "effect_dispatch_data.h"
#include "IEffects.h"
#include "beam_shared.h"

#ifdef GAME_DLL
	#include "ff_entity_system.h"
	#include "te_effect_dispatch.h"
	#include "ff_item_backpack.h"
#else
	#include "c_te_effect_dispatch.h"
#endif

#define GRENADE_BEAM_SPRITE			"effects/bluelaser1.vmt"
#define HOVERGRENADE_MODEL			"models/grenades/nailgren/nailgren.mdl"

#ifdef CLIENT_DLL
	#define CFFGrenadeHoverTurret C_FFGrenadeHoverTurret
#endif

ConVar ffdev_hovergren_range("ffdev_hovergren_range", "500", FCVAR_REPLICATED /*  | FCVAR_CHEAT */, "Hover turret grenade range ");
#define FFDEV_HOVERGREN_RANGE ffdev_hovergren_range.GetFloat()
ConVar ffdev_hovergren_lifetime("ffdev_hovergren_lifetime", "7", FCVAR_REPLICATED /*  | FCVAR_CHEAT */, "Hover turret grenade: life time ");
#define FFDEV_HOVERGREN_LIFETIME ffdev_hovergren_lifetime.GetFloat()
ConVar ffdev_hovergren_risetime("ffdev_hovergren_risetime", "0.3", FCVAR_REPLICATED /*  | FCVAR_CHEAT */, "Hover turret grenade: time it takes to rise to full height ");
#define FFDEV_HOVERGREN_RISETIME ffdev_hovergren_risetime.GetFloat()
ConVar ffdev_hovergren_risespeed("ffdev_hovergren_risespeed", "140", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: speed it rises to off the floor ");
#define FFDEV_HOVERGREN_RISESPEED ffdev_hovergren_risespeed.GetFloat()
ConVar ffdev_hovergren_bulletdamage("ffdev_hovergren_bulletdamage", "6", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: Bullet damage ");
#define FFDEV_HOVERGREN_BULLETDAMAGE ffdev_hovergren_bulletdamage.GetFloat()
ConVar ffdev_hovergren_bulletpush("ffdev_hovergren_bulletpush", "3", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: Bullet damage ");
#define FFDEV_HOVERGREN_BULLETPUSH ffdev_hovergren_bulletpush.GetFloat()
ConVar ffdev_hovergren_rof("ffdev_hovergren_rof", "0.3", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: rate of fire ");
#define FFDEV_HOVERGREN_ROF ffdev_hovergren_rof.GetFloat()
ConVar ffdev_hovergren_endcells("ffdev_hovergren_endcells", "50", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: Cells to drop on death ");
#define FFDEV_HOVERGREN_ENDCELLS ffdev_hovergren_endcells.GetInt()
ConVar ffdev_hovergren_bagcells("ffdev_hovergren_bagcells", "30", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: Cells to drop on death ");
#define FFDEV_HOVERGREN_BAGCELLS ffdev_hovergren_bagcells.GetInt()
ConVar ffdev_hovergren_damagetodropbag("ffdev_hovergren_damagetodropbag", "30", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: Cells to drop on death ");
#define FFDEV_HOVERGREN_DAMAGETODROPBAG ffdev_hovergren_damagetodropbag.GetInt()
ConVar ffdev_hovergren_scansoundtime("ffdev_hovergren_scansoundtime", "1", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: Time between scan sounds ");
#define FFDEV_HOVERGREN_SCANSOUNDTIME ffdev_hovergren_scansoundtime.GetFloat()
ConVar ffdev_hovergren_bagthrowforce("ffdev_hovergren_bagthrowforce", "1", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: horizontal throw force on bags");
#define FFDEV_HOVERGREN_BAGTHROWFORCE ffdev_hovergren_bagthrowforce.GetFloat()
ConVar ffdev_hovergren_bagspawndist("ffdev_hovergren_bagspawndist", "1", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: distance below turret of bags spawning ");
#define FFDEV_HOVERGREN_BAGSPAWNDIST ffdev_hovergren_bagspawndist.GetFloat()
ConVar ffdev_hovergren_bagthrowforceup("ffdev_hovergren_bagthrowforceup", "400", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: vertical throw force on bags ");
#define FFDEV_HOVERGREN_BAGTHROWFORCEUP ffdev_hovergren_bagthrowforceup.GetFloat()




class CFFGrenadeHoverTurret : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeHoverTurret, CFFGrenadeBase) 
	DECLARE_NETWORKCLASS(); 

	virtual void Precache();
	virtual const char *GetBounceSound() { return "NailGrenade.Bounce"; }

	virtual Class_T Classify( void ) { return CLASS_GREN_HOVERTURRET; } 

	virtual color32 GetColour() { color32 col = { 128, 225, 255, GREN_ALPHA_DEFAULT }; return col; }

#ifdef CLIENT_DLL
	CFFGrenadeHoverTurret() {}
	CFFGrenadeHoverTurret(const CFFGrenadeHoverTurret&) {}
//	virtual int DrawModel(int flags);
#else
	DECLARE_DATADESC(); // Since we're adding new thinks etc
	virtual void Spawn();
	virtual void HoverThink();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);

private:
	void ShootNail( const Vector& vecOrigin, const QAngle& vecAngles );

protected:
	//float	m_flNailSpit;
	//float	m_flAngleOffset;
	//CBeam	*pBeam[MAX_BEAMS];

	//CUtlVector< PseudoNail > m_NailsVector;

	//float	m_flLastThinkTime;
	CFFPlayer*	m_pTarget;
	float		m_flLastFireTime;
	float		m_flScanTime;
	int			m_iDamageDone;

#endif
};

#ifdef GAME_DLL
	BEGIN_DATADESC(CFFGrenadeHoverTurret) 
		DEFINE_THINKFUNC(HoverThink),
	END_DATADESC() 
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeHoverTurret, DT_FFGrenadeHoverTurret)

BEGIN_NETWORK_TABLE(CFFGrenadeHoverTurret, DT_FFGrenadeHoverTurret)
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(ff_grenade_hoverturret, CFFGrenadeHoverTurret);
PRECACHE_WEAPON_REGISTER(ff_grenade_hoverturret);

#define NAIL_BBOX 2.0f

//-----------------------------------------------------------------------------
// Purpose: Various precache things
//-----------------------------------------------------------------------------
void CFFGrenadeHoverTurret::Precache() 
{
	PrecacheModel(HOVERGRENADE_MODEL);
	PrecacheModel(GRENADE_BEAM_SPRITE);
	PrecacheScriptSound( "HoverTurret.Shoot" );
	PrecacheScriptSound( "HoverTurret.Scan" );

	BaseClass::Precache();
}

#ifdef GAME_DLL

	//-----------------------------------------------------------------------------
	// Purpose: Various spawny flag things
	//-----------------------------------------------------------------------------
	void CFFGrenadeHoverTurret::Spawn() 
	{
		SetModel(HOVERGRENADE_MODEL);
		BaseClass::Spawn();

		//m_flAngleOffset = 0.0f;
		//SetLocalAngularVelocity(QAngle(0, 15, 0));
		m_pTarget = NULL;
		m_flLastFireTime = 0.0f;
		m_flScanTime = 0.0f;
		m_iDamageDone = 0;
		m_takedamage = DAMAGE_YES;
		m_iHealth = m_iMaxHealth = 1;
		SetSize( -Vector(10,10,10), Vector(10,10,10) );
		SetCollisionGroup( COLLISION_GROUP_PLAYER );
		SetSolid( SOLID_BBOX );
		RemoveSolidFlags( FSOLID_NOT_SOLID );
		RemoveSolidFlags( FSOLID_NOT_STANDABLE );
		//AddSolidFlags( FSOLID_TRIGGER );
		//CreateVPhysics();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Instead of exploding, change to 
	//-----------------------------------------------------------------------------
	void CFFGrenadeHoverTurret::Explode(trace_t *pTrace, int bitsDamageType)
	{
		// Clumsy, will do for now
		if (GetMoveType() == MOVETYPE_FLY)
		{

			// AfterShock: Create bag when detonate
			CFFItemBackpack *pBackpack = (CFFItemBackpack *) CBaseEntity::Create( "ff_item_backpack", (GetAbsOrigin() + Vector(0.0f, 0.0f, 20.0f) ), QAngle(0, random->RandomFloat(0.0f, 359.0f), 0) );
			if( pBackpack )
			{
				pBackpack->SetAmmoCount( GetAmmoDef()->Index( AMMO_CELLS ), FFDEV_HOVERGREN_BAGCELLS );
				pBackpack->SetAbsVelocity( Vector(0.0f, 0.0f, 350.0f) );
			}

			BaseClass::Explode(pTrace, bitsDamageType);
			return;
		}

		SetDetonateTimerLength( FFDEV_HOVERGREN_LIFETIME );

		// Should this maybe be noclip?
		SetMoveType(MOVETYPE_FLY);
			
		SetDamage(80.0f);

		SetThink(&CFFGrenadeHoverTurret::HoverThink);
		SetNextThink(gpGlobals->curtime);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Spin round emitting lasers
	//-----------------------------------------------------------------------------
	void CFFGrenadeHoverTurret::HoverThink() 
	{

		// Blow up if we've reached the end of our fuse
		if (gpGlobals->curtime > m_flDetonateTime) 
		{
			Detonate(); // this should release bags etc
			return;
		}

		float flRisingspeed = 0.0f;
		// Lasts for 3 seconds, rise for 0.3, but only if not handheld
		if (m_flDetonateTime - gpGlobals->curtime > (FFDEV_HOVERGREN_LIFETIME - FFDEV_HOVERGREN_RISETIME) && !m_fIsHandheld)
		{
			flRisingspeed = FFDEV_HOVERGREN_RISESPEED;
		}
		SetAbsVelocity(Vector(0, 0, flRisingspeed));
		
		//SetAbsAngles(GetAbsAngles() + QAngle(0, laserangv.GetFloat(), 0));

		//SetAbsAngles( QAngle((GetAbsAngles().x + 30), 0, 90) ); // this spins and faces the player
		SetAbsAngles( QAngle(0, (GetAbsAngles().y + 50), 90) );

		if ( m_pTarget )
		{
			//fire at existingtarget

	// is this target still valid
			if ( !FVisible( m_pTarget ) )
			{
				m_pTarget = NULL;
				DevMsg("Cant see target\n");
			}
			else if( WorldSpaceCenter().DistTo( m_pTarget->GetAbsOrigin() ) > FFDEV_HOVERGREN_RANGE)
			{
				m_pTarget = NULL;
				DevMsg("Target gone out of range\n");
			}

			else if ( !m_pTarget->IsAlive() )
			{
				m_pTarget = NULL;
				DevMsg("Target dead\n");
			}
		}

		if ( m_pTarget )
		{
			// fire !

			if ( gpGlobals->curtime > m_flLastFireTime + FFDEV_HOVERGREN_ROF )
			{
				FireBulletsInfo_t info;
				Vector vecDir;
				vecDir = m_pTarget->GetAbsOrigin() - GetAbsOrigin();

				info.m_vecSrc = GetAbsOrigin();
				info.m_vecDirShooting = vecDir;
				info.m_iTracerFreq = 0;	// Mirv: Doing tracers down below now
				info.m_iShots = 1;
				// Jiggles: We want to fire more than 1 shot if the SG's think rate is too slow to keep up with the cycle time value
				//			Since we can't fire partial bullets, we have to accumulate them
				// But don't do it if it has been longer than the SG's think time

				//info.m_pAttacker = this;
				info.m_pAttacker = GetOwnerEntity();
				info.m_vecSpread = VECTOR_CONE_PRECALCULATED;
				info.m_flDistance = MAX_COORD_RANGE;
				info.m_iAmmoType = GetAmmoDef()->Index( AMMO_SHELLS );
				info.m_iDamage = FFDEV_HOVERGREN_BULLETDAMAGE;
				info.m_flDamageForceScale = FFDEV_HOVERGREN_BULLETPUSH;


				// Jiggles: A HACK to address the fact that it takes a lot more force to push players around on the ground than in the air
				if ( m_pTarget && (m_pTarget->GetFlags() & FL_ONGROUND) )
				{
						info.m_flDamageForceScale *= 5.0f; // see ff_sentrygun.cpp for updated numbers.. 5 is correct at time of writing
				}

				FireBullets( info );
				EmitSound( "HoverTurret.Shoot" );

				m_flLastFireTime = gpGlobals->curtime;

				// try normal tracers first

				QAngle vecAngles;
				VectorAngles( vecDir, vecAngles );

				//int iAttachment = GetLevel() > 2 ? (m_bLeftBarrel ? m_iLBarrelAttachment : m_iRBarrelAttachment) : m_iMuzzleAttachment;

				trace_t tr;
				UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vecDir * (FFDEV_HOVERGREN_RANGE * 1.5), MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER, &tr);

				/*
				CEffectData data;
				data.m_vStart = GetAbsOrigin() + vecDir * 20;
				data.m_vOrigin = data.m_vStart;
				data.m_nEntIndex = GetBaseAnimating()->entindex();
				data.m_flScale = 4.0f;
				//data.m_nAttachmentIndex = iAttachment;
				data.m_fFlags = MUZZLEFLASH_TYPE_DEFAULT;

				DispatchEffect("MuzzleFlash", data);
	*/

				CEffectData data2;
				data2.m_vStart = GetAbsOrigin();
				data2.m_vOrigin = tr.endpos;
				//data2.m_nEntIndex = GetBaseAnimating()->entindex();
				data2.m_flScale = 0.0f;
				//data2.m_nAttachmentIndex = iAttachment;
				DispatchEffect("AR2Tracer", data2);
				

				// Bags!

				m_iDamageDone += FFDEV_HOVERGREN_BULLETDAMAGE;
				if (m_iDamageDone >= FFDEV_HOVERGREN_DAMAGETODROPBAG)
				{
					// AfterShock: Create bag when enough damage is dealt
					CFFItemBackpack *pBackpack = (CFFItemBackpack *) CBaseEntity::Create( "ff_item_backpack", ( (GetAbsOrigin() - vecDir *0.1) + Vector(0.0f, 0.0f, -FFDEV_HOVERGREN_BAGSPAWNDIST) ), QAngle(0, random->RandomFloat(0.0f, 359.0f), 0) );
					if( pBackpack )
					{
						pBackpack->SetAmmoCount( GetAmmoDef()->Index( AMMO_CELLS ), FFDEV_HOVERGREN_BAGCELLS );
						//pBackpack->SetAbsVelocity( Vector( random->RandomFloat(-100.0f, 0.0f), random->RandomFloat(-100.0f, 100.0f), random->RandomFloat(20.0f, 100.0f) ) );
						pBackpack->SetAbsVelocity( Vector(-vecDir.x, -vecDir.y, FFDEV_HOVERGREN_BAGTHROWFORCEUP) * FFDEV_HOVERGREN_BAGTHROWFORCE );
					}
					m_iDamageDone -= FFDEV_HOVERGREN_DAMAGETODROPBAG;
				}
			}
		}
		else
		{
			// get a new target (players only)

			m_pTarget = NULL;

			if ( m_flScanTime < gpGlobals->curtime )
			{
				EmitSound( "HoverTurret.Scan" );
				m_flScanTime = gpGlobals->curtime + FFDEV_HOVERGREN_SCANSOUNDTIME;
			}

			float currentTargetDistance = 1000.0f;
			CBaseEntity *pEntity = NULL;
			for( CEntitySphereQuery sphere( GetAbsOrigin(), FFDEV_HOVERGREN_RANGE ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
			{
				if( !pEntity )
					continue;

				if( !pEntity->IsPlayer() )
					continue;

				CFFPlayer *pPlayer = ToFFPlayer( pEntity );
				//CFFPlayer *pSlower = ToFFPlayer( GetOwnerEntity() );

				if( !pPlayer || pPlayer->IsObserver() )
					continue;

				//if( !g_pGameRules->FCanTakeDamage( pPlayer, GetOwnerEntity() ) )
				//	continue;

				if ( g_pGameRules->PlayerRelationship( ToFFPlayer(GetOwnerEntity()), pPlayer ) == GR_TEAMMATE )
					continue;

				//if ( !FVisible( pPlayer ) )
				//	continue;
					
				Vector vecTarget = pPlayer->BodyTarget( WorldSpaceCenter(), false );

				if( WorldSpaceCenter().DistTo( vecTarget ) > FFDEV_HOVERGREN_RANGE)
					continue;

				if ( WorldSpaceCenter().DistTo( vecTarget ) < currentTargetDistance )
				{
					// Ok now do the more expensive check to see if we can actually hit them

					// Get our aiming position
					Vector vecOrigin = WorldSpaceCenter();

					// Can we trace to the target?
					trace_t tr;
					UTIL_TraceLine( vecOrigin, vecTarget, MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER, &tr );

					// What did our trace hit?
					if( tr.startsolid || /*( tr.fraction != 1.0f ) ||*/ !tr.m_pEnt || FF_TraceHitWorld( &tr ) )
						continue;

					if(tr.m_pEnt != pPlayer)
						continue;
					
					// Ok we can hit them!
					m_pTarget = pPlayer;
					currentTargetDistance = WorldSpaceCenter().DistTo( vecTarget );
				}
			}
		}

		/*
		Vector vecDirection;
		Vector vecOrigin = GetAbsOrigin();
		QAngle angRadial = GetAbsAngles();

		float flSize = 20.0f;
		trace_t tr;
		char i;

		float flDeltaAngle = 360.0f / laserbeams.GetInt();

		for( i = 0; i < laserbeams.GetInt(); i++ )
		{
			AngleVectors(angRadial, &vecDirection);
			VectorNormalizeFast(vecDirection);

			UTIL_TraceLine( vecOrigin + vecDirection * flSize, 
				vecOrigin + vecDirection * laserdistance.GetFloat(), MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER, &tr );
			
			pBeam[i]->PointsInit( vecOrigin, tr.endpos );

			if ( tr.m_pEnt )
				DoDamage( tr.m_pEnt );

			angRadial.y += flDeltaAngle;
		}
		*/
		
		SetNextThink( gpGlobals->curtime + 0.01f );
	}


/*
#else // client only drawing stuff here
// this was a (failed) attempt to put the grenade halos around the model even after it went live

	int CFFGrenadeHoverTurret::DrawModel(int flags)
	{
		int ret = BaseClass::DrawModel(flags);

		if (ret == 0)
			return 0;

		if (grenadetargets.GetBool() == false)
			return ret;

		float flSpeed = GetAbsVelocity().Length();
		
		float speed_max = target_speed_max.GetFloat();
		float speed_min = target_speed_min.GetFloat();

		// Safety check...
		if (speed_max == speed_min)
			speed_max += 0.1f;

		if (flSpeed > speed_max)
			return ret;

		color32 col = GetColour();

		if (m_flModelSize == 0.0f)
		{
			const model_t *pModel = GetModel();

			if (pModel)
			{
				studiohdr_t *pStudio = modelinfo->GetStudiomodel(pModel);

				if (pStudio)
				{
					Vector vecDimensions = pStudio->hull_max - pStudio->hull_min;

					// We could be cunning and project these with our projection matrix
					// in order to be more accurate, but lets try like this first
					m_flModelSize = vecDimensions.Length();
				}
			}
		}

		// Need to scale somewhere between speed_min and speed_max...
		if (flSpeed > speed_min)
		{
			float flScale = (flSpeed - speed_min) / (speed_max - speed_min);
			col.a *= 1.0f - flScale;
		}

		float flRemaining = target_time_remaining.GetFloat() - (gpGlobals->curtime - m_flSpawnTime);

		if (flRemaining < -0.1f)
		{
			flRemaining = target_time_remaining.GetFloat() - (gpGlobals - m_flDetonateTime);
		}

		float flSize = m_flModelSize * target_size_base.GetFloat() + target_size_multiplier.GetFloat() * flRemaining;
		flSize = clamp(flSize, target_clamp_min.GetFloat(), target_clamp_max.GetFloat());

		// The blur graphic now has everything all in one
		// TODO: Stop doing this every frame.
		//IMaterial *pMaterial = materials->FindMaterial("sprites/ff_target", TEXTURE_GROUP_CLIENT_EFFECTS);
		IMaterial *pMaterialBlur = materials->FindMaterial("sprites/ff_target_blur", TEXTURE_GROUP_CLIENT_EFFECTS);

//		float flRotation = gpGlobals->curtime * target_rotation.GetFloat() - anglemod(m_flSpawnTime);
		float flRotation = anglemod(gpGlobals->curtime  - m_flSpawnTime) * target_rotation.GetFloat();

		// Just display the blur material as that has all the stuff in one
		if (pMaterialBlur)
		{
			materials->Bind(pMaterialBlur);
			DrawSpriteRotated(GetAbsOrigin(), flSize, flSize, col, flRotation);
		}

		return ret;
	}
*/
#endif