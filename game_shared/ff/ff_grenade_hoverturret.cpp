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
#include "ff_projectile_nail.h"

#ifdef GAME_DLL
	#include "ff_entity_system.h"
	#include "te_effect_dispatch.h"
	#include "ff_item_backpack.h"
#else
	#include "c_te_effect_dispatch.h"
#endif

#define GRENADE_BEAM_SPRITE2			"sprites/plasma.spr"
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
ConVar ffdev_hovergren_risespeed("ffdev_hovergren_risespeed", "236", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: speed it rises to off the floor ");
#define FFDEV_HOVERGREN_RISESPEED ffdev_hovergren_risespeed.GetFloat()
ConVar ffdev_hovergren_bulletdamage("ffdev_hovergren_bulletdamage", "6", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: Bullet damage ");
#define FFDEV_HOVERGREN_BULLETDAMAGE ffdev_hovergren_bulletdamage.GetFloat()
ConVar ffdev_hovergren_bulletpush("ffdev_hovergren_bulletpush", "3", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: Bullet damage ");
#define FFDEV_HOVERGREN_BULLETPUSH ffdev_hovergren_bulletpush.GetFloat()
ConVar ffdev_hovergren_rof("ffdev_hovergren_rof", "0.2", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: rate of fire ");

#define FFDEV_HOVERGREN_NAILDAMAGE ffdev_hovergren_naildamage.GetFloat()
ConVar ffdev_hovergren_naildamage("ffdev_hovergren_naildamage", "6", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: Nail damage ");
#define FFDEV_HOVERGREN_NAILSPEED ffdev_hovergren_nailspeed.GetFloat()
ConVar ffdev_hovergren_nailspeed("ffdev_hovergren_nailspeed", "1500", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: Nail speed ");
#define FFDEV_HOVERGREN_FIRENAILS ffdev_hovergren_firenails.GetBool()
ConVar ffdev_hovergren_firenails("ffdev_hovergren_firenails", "1", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: Firenails? If not, use bullets. ");

#define FFDEV_HOVERGREN_ROF ffdev_hovergren_rof.GetFloat()
ConVar ffdev_hovergren_endcells("ffdev_hovergren_endcells", "50", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: Cells to drop on expiry ");
#define FFDEV_HOVERGREN_ENDCELLS ffdev_hovergren_endcells.GetInt()
ConVar ffdev_hovergren_bagcells("ffdev_hovergren_bagcells", "30", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: Cells to drop in each mid-life bag ");
#define FFDEV_HOVERGREN_BAGCELLS ffdev_hovergren_bagcells.GetInt()
ConVar ffdev_hovergren_damagetodropbag("ffdev_hovergren_damagetodropbag", "50", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: How much damage to deal before dropping a mid-life bag ");
#define FFDEV_HOVERGREN_DAMAGETODROPBAG ffdev_hovergren_damagetodropbag.GetInt()
ConVar ffdev_hovergren_scansoundtime("ffdev_hovergren_scansoundtime", "1", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: Time between scan sounds ");
#define FFDEV_HOVERGREN_SCANSOUNDTIME ffdev_hovergren_scansoundtime.GetFloat()
ConVar ffdev_hovergren_bagthrowforce("ffdev_hovergren_bagthrowforce", "1", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: horizontal throw force on bags");
#define FFDEV_HOVERGREN_BAGTHROWFORCE ffdev_hovergren_bagthrowforce.GetFloat()
ConVar ffdev_hovergren_bagspawndist("ffdev_hovergren_bagspawndist", "1", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: distance below turret of bags spawning ");
#define FFDEV_HOVERGREN_BAGSPAWNDIST ffdev_hovergren_bagspawndist.GetFloat()
ConVar ffdev_hovergren_bagthrowforceup("ffdev_hovergren_bagthrowforceup", "400", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: vertical throw force on bags ");
#define FFDEV_HOVERGREN_BAGTHROWFORCEUP ffdev_hovergren_bagthrowforceup.GetFloat()

ConVar ffdev_hovergren_canbeshot("ffdev_hovergren_canbeshot", "0", FCVAR_REPLICATED /* FCVAR_REPLICATED | FCVAR_CHEAT */, "Hover turret grenade: Can you shoot it to destroy it? ");
#define FFDEV_HOVERGREN_CANBESHOT ffdev_hovergren_canbeshot.GetBool()

#ifdef CLIENT_DLL
	ConVar hud_hovergren_customColor_enable( "hud_hovergren_customColor_enable", "0", FCVAR_ARCHIVE, "Use custom laser colors (1 = use custom colour)");
	ConVar hud_hovergren_customColor_r( "hud_hovergren_customColor_r", "255", FCVAR_ARCHIVE, "Custom laser color - Red Component (0-255)");
	ConVar hud_hovergren_customColor_g( "hud_hovergren_customColor_g", "128", FCVAR_ARCHIVE, "Custom laser color - Green Component(0-255)");
	ConVar hud_hovergren_customColor_b( "hud_hovergren_customColor_b", "255", FCVAR_ARCHIVE, "Custom laser color - Blue Component(0-255)");
	ConVar ffdev_hovergren_widthcreate("ffdev_hovergren_widthcreate", "0.5", FCVAR_REPLICATED, "Width given in the constructor; not used");
	ConVar ffdev_hovergren_widthstart("ffdev_hovergren_widthstart", "4", FCVAR_REPLICATED, "Width at the start of the beam");
	ConVar ffdev_hovergren_widthend("ffdev_hovergren_widthend", "4", FCVAR_REPLICATED, "Width at the end of the beam");
	#define HOVERGREN_WIDTHCREATE ffdev_hovergren_widthcreate.GetFloat()
	#define HOVERGREN_WIDTHSTART ffdev_hovergren_widthstart.GetFloat()
	#define HOVERGREN_WIDTHEND ffdev_hovergren_widthend.GetFloat()
#endif


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
	virtual void ClientThink();
	virtual void OnDataChanged(DataUpdateType_t updateType);
	virtual void UpdateOnRemove( void );
protected:
	CBeam		*pBeam[1];
	IMaterial	*m_pMaterial;

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
	PrecacheModel(GRENADE_BEAM_SPRITE2);
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

		if ( FFDEV_HOVERGREN_CANBESHOT )
			m_takedamage = DAMAGE_YES;
		else
			m_takedamage = DAMAGE_NO;

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
			else if ( m_pTarget->IsCloaked() )
			{
				m_pTarget = NULL;
				DevMsg("Target cloaked\n");
			}
			
		}

		if ( m_pTarget )
		{
			// fire !

			if ( gpGlobals->curtime > m_flLastFireTime + FFDEV_HOVERGREN_ROF )
			{

				Vector vecDir = m_pTarget->GetAbsOrigin() - GetAbsOrigin();
				VectorNormalize( vecDir );

				if (!FFDEV_HOVERGREN_FIRENAILS) // fire bullets
				{
					FireBulletsInfo_t info;
					
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
							info.m_flDamageForceScale *= 5.0f; // see ff_sentrygun.cpp for updated numbers, 5 is correct at time of writing
					}

					FireBullets( info );
				}
				else // fire nails
				{
					QAngle vecAngles;
					VectorAngles( vecDir, vecAngles );

					CFFProjectileNail *pNail = CFFProjectileNail::CreateNail(this, GetAbsOrigin() + vecDir * 20, vecAngles,  ToFFPlayer(GetOwnerEntity()), FFDEV_HOVERGREN_NAILDAMAGE, FFDEV_HOVERGREN_NAILSPEED);
					//CFFProjectileDart *pDart = CFFProjectileDart::CreateDart(this, GetAbsOrigin() + vecDir * 20, vecAngles,  ToFFPlayer(GetOwnerEntity()), FFDEV_HOVERGREN_NAILDAMAGE, FFDEV_HOVERGREN_NAILSPEED);
				}

				EmitSound( "HoverTurret.Shoot" );

				m_flLastFireTime = gpGlobals->curtime;


				

				//int iAttachment = GetLevel() > 2 ? (m_bLeftBarrel ? m_iLBarrelAttachment : m_iRBarrelAttachment) : m_iMuzzleAttachment;

				trace_t tr;
				UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vecDir * (FFDEV_HOVERGREN_RANGE * 1.5), MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER, &tr);

				/* Couldnt get the muzzle flash working
				CEffectData data;
				data.m_vStart = GetAbsOrigin() + vecDir * 20;
				data.m_vOrigin = data.m_vStart;
				data.m_nEntIndex = GetBaseAnimating()->entindex();
				data.m_flScale = 4.0f;
				//data.m_nAttachmentIndex = iAttachment;
				data.m_fFlags = MUZZLEFLASH_TYPE_DEFAULT;

				DispatchEffect("MuzzleFlash", data);
	*/
/* Temporary commenting out of the tracer for hitscan shots
				CEffectData data2;
				data2.m_vStart = GetAbsOrigin();
				data2.m_vOrigin = tr.endpos;
				//data2.m_nEntIndex = GetBaseAnimating()->entindex();
				data2.m_flScale = 0.0f;
				//data2.m_nAttachmentIndex = iAttachment;
				DispatchEffect("AR2Tracer", data2);
				*/ 

				// Bags!

				if (FFDEV_HOVERGREN_FIRENAILS)
					m_iDamageDone += FFDEV_HOVERGREN_NAILDAMAGE; // Well technically we cant say that all nails have hit, so in this case it's more how many nails have been fired
				else
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

				if ( pPlayer->IsCloaked() )
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
		
		SetNextThink( gpGlobals->curtime + 0.01f );
	}

#else 

	//-----------------------------------------------------------------------------
	// Purpose: Emit gas.
	//-----------------------------------------------------------------------------
	void CFFGrenadeHoverTurret::ClientThink()
	{
		if ( m_bIsOn )
		{
			//EmitSound("NailGrenade.LaserLoop");

			Vector vecDirection;
			Vector vecOrigin = GetAbsOrigin();
			//QAngle angRadial = GetAbsAngles();

			float flSize = 20.0f;
			trace_t tr;
			//char i;

			CFFPlayer *pgrenOwner = ToFFPlayer( this->GetOwnerEntity() );
		
			if (!pgrenOwner)
				return;

			//float flDeltaAngle = 360.0f;

			//for( i = 0; i < laserbeams.GetInt(); i++ )
			//{
				//AngleVectors(angRadial, &vecDirection);
				//VectorNormalizeFast(vecDirection);
				vecDirection = Vector(0,0,-1);

				UTIL_TraceLine( vecOrigin + vecDirection * flSize, 
								vecOrigin + vecDirection * 2000.0f, 
								MASK_SHOT, this, COLLISION_GROUP_PLAYER, &tr );

				if( !pBeam[0] )
		{
					pBeam[0] = CBeam::BeamCreate( GRENADE_BEAM_SPRITE2, HOVERGREN_WIDTHCREATE );
					if (pBeam[0])
			{
						pBeam[0]->SetWidth( HOVERGREN_WIDTHSTART );
						pBeam[0]->SetEndWidth( HOVERGREN_WIDTHEND );
						pBeam[0]->LiveForTime( 1  );
						pBeam[0]->SetBrightness( 255 );
						if(hud_hovergren_customColor_enable.GetBool() == true)
							pBeam[0]->SetColor( hud_hovergren_customColor_r.GetInt(), hud_hovergren_customColor_g.GetInt(), hud_hovergren_customColor_b.GetInt() );						
						else if(pgrenOwner->GetTeamNumber() == TEAM_RED)
							pBeam[0]->SetColor( 255, 64, 64 );
						else if(pgrenOwner->GetTeamNumber() == TEAM_BLUE)
							pBeam[0]->SetColor( 64, 128, 255 );
						else if(pgrenOwner->GetTeamNumber() == TEAM_GREEN)
							pBeam[0]->SetColor( 153, 255, 153 );
						else if(pgrenOwner->GetTeamNumber() == TEAM_YELLOW)
							pBeam[0]->SetColor( 255, 178, 0 );
						else // just in case
							pBeam[0]->SetColor( 204, 204, 204 ); 
					}
				}
				if (pBeam[0])
					pBeam[0]->PointsInit( vecOrigin, tr.endpos );

				if ( tr.fraction == 1.0f )
					g_pEffects->MetalSparks( tr.endpos, vecDirection );
				else
					g_pEffects->MetalSparks( tr.endpos, -vecDirection );

			//}
				}
			}

	void CFFGrenadeHoverTurret::UpdateOnRemove( void )
		{
		//StopSound("NailGrenade.LaserLoop");
		//StopSound("NailGrenade.LaserDeploy");

		if( pBeam[0] )
		{
			delete pBeam[0];
		}

		//BaseClass::UpdateOnRemove();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Called when data changes on the server
	//-----------------------------------------------------------------------------
	void CFFGrenadeHoverTurret::OnDataChanged(DataUpdateType_t updateType)
	{
		// NOTE: We MUST call the base classes' implementation of this function
//		BaseClass::OnDataChanged(updateType);

		// Setup our entity's particle system on creation
		if (updateType == DATA_UPDATE_CREATED)
		{
			m_pMaterial = materials->FindMaterial("effects/blueblacklargebeam", TEXTURE_GROUP_CLIENT_EFFECTS);
			m_pMaterial->IncrementReferenceCount();

			// Call our ClientThink() function once every client frame
			SetNextClientThink(CLIENT_THINK_ALWAYS);
		}
	}

#endif