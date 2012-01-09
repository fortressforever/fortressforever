/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_pipebomb.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 24, 2004
/// @brief The FF pipebomb projectile code.
///
/// REVISIONS
/// ---------
/// Dec 24, 2004 Mirv: First created

//KNOWN BUG:  Pipes dont stop playing sound after their det goes off

//TODO: Think about slightly randomizing the vector location inside the player so pipes dont clump up
//		Play a sound sound somewhere, either player or pipe to denote magnetism
//		Display somewhere on the hud how many pipes are attached
//		Indicate if the pipes are close to the target by sound or visual effect( hud or otherwise )

#include "cbase.h"
#include "ff_projectile_pipebomb.h"
#include "ff_utils.h"

#ifdef GAME_DLL
	#include "ff_entity_system.h"
	#include "soundent.h"	
#endif

extern short	g_sModelIndexFireball;		// (in combatweapon.cpp) holds the index for the fireball 
extern short	g_sModelIndexWExplosion;	// (in combatweapon.cpp) holds the index for the underwater explosion
extern short	g_sModelIndexSmoke;			// (in combatweapon.cpp) holds the index for the smoke cloud

//=============================================================================
// CFFProjectilePipebomb tables
//=============================================================================

#ifdef GAME_DLL
BEGIN_DATADESC(CFFProjectilePipebomb)
	DEFINE_THINKFUNC(PipebombThink), 
END_DATADESC() 
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFProjectilePipebomb, DT_FFProjectilePipebomb) 

BEGIN_NETWORK_TABLE(CFFProjectilePipebomb, DT_FFProjectilePipebomb) 
END_NETWORK_TABLE() 

LINK_ENTITY_TO_CLASS(ff_projectile_pl, CFFProjectilePipebomb);
PRECACHE_WEAPON_REGISTER(ff_projectile_pl);


ConVar ffdev_pipe_friction("ffdev_pipe_friction", "0.375", FCVAR_REPLICATED | FCVAR_CHEAT, "256 for sticky pipes.  0.375 for normal");
//0001279: Need convar for pipe det delay
ConVar pipebomb_time_till_live("ffdev_pipe_detdelay", "0.55", FCVAR_REPLICATED | FCVAR_CHEAT);
#define PIPE_DET_DELAY 0.55	// this is mirrored in ff_player_shared.cpp(97) and ff_player.cpp

ConVar ffdev_pipebomb_arm_delay("ffdev_pipe_arm_delay", "0.0", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define PIPE_ARM_DELAY ffdev_pipebomb_arm_delay.GetFloat()

ConVar ffdev_pipebomb_follow_speed_factor( "ffdev_pipe_follow_speed_factor", "0.8", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define PIPE_FOLLOW_SPEED_FACTOR ffdev_pipebomb_follow_speed_factor.GetFloat()

ConVar ffdev_pipebomb_magnet_radius( "ffdev_pipe_magnet_radius", "128", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define PIPE_MAGNET_RADIUS ffdev_pipebomb_magnet_radius.GetInt()

//My own det delay so i can revert easily, and i think mine is a little different
ConVar ffdev_pipebomb_detdelay( "ffdev_pipe_detdelay", "0.1", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define PIPE_DETONATE_DELAY ffdev_pipebomb_detdelay.GetFloat()

//Convar for the det time on magnetic pipes
ConVar ffdev_pipebomb_magnetic_detdelay( "ffdev_pipe_magnetic_detdelay", "0.5", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define PIPE_MAGNETIC_DETONATE_DELAY ffdev_pipebomb_magnetic_detdelay.GetFloat()

//Pipes max follow distance
ConVar ffdev_pipebomb_maxfollowdist( "ffdev_pipe_maxfollowdist", "128", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define PIPE_MAX_FOLLOW_DIST ffdev_pipebomb_maxfollowdist.GetInt()

//Pipes min pecentage base speed of player to activate the magnet
ConVar ffdev_pipebomb_min_activation_speed_factor( "ffdev_pipe_min_activation_speed_factor", "0.6", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define PIPE_MIN_ACTIVATION_SPEED_FACTOR ffdev_pipebomb_min_activation_speed_factor.GetFloat()

//Radius that pipes randomize when they stop
ConVar ffdev_pipebomb_randomize_radius( "ffdev_pipe_randomize_radius", "16", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define PIPE_RANDOMIZE_RADIUS ffdev_pipebomb_randomize_radius.GetInt()

//Speed that pipes move when they randomize
ConVar ffdev_pipebomb_randomize_speed( "ffdev_pipe_randomize_speed", "128", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define PIPE_RANDOMIZE_SPEED ffdev_pipebomb_randomize_speed.GetInt()

//Allow the player to attach to himself for easier testing/debugging
ConVar ffdev_pipebomb_friendly_attach( "ffdev_pipe_friendly_attach", "0", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define PIPE_FRIENDLY_ATTACH ffdev_pipebomb_friendly_attach.GetBool()

//Allow for different pipe modes
ConVar ffdev_pipebomb_mode( "ffdev_pipe_mode", "0", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define PIPE_MODE ffdev_pipebomb_mode.GetInt()

//Radius for Proximity pipes
ConVar ffdev_pipebomb_proximity_radius( "ffdev_pipe_proximity_radius", "128", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define PIPE_PROXIMITY_RADIUS  ffdev_pipebomb_proximity_radius.GetInt()

//Time after activation to detonate via proximity
ConVar ffdev_pipebomb_proximity_det_delay( "ffdev_pipe_proximity_det_delay", "0.75", FCVAR_REPLICATED | FCVAR_NOTIFY );
#define PIPE_PROXIMITY_DETONATE_DELAY ffdev_pipebomb_proximity_det_delay.GetFloat()

//Speed required to activate pipe behavior
ConVar ffdev_pipebomb_activation_speed( "ffdev_pipebomb_activation_speed", "4.0f", FCVAR_REPLICATED | FCVAR_NOTIFY, "Max speed to activate magnet/autodet");
#define PIPE_ACTIVATION_SPEED ffdev_pipebomb_activation_speed.GetFloat()

//=============================================================================
// CFFProjectilePipebomb implementation
//=============================================================================

#ifdef GAME_DLL

	void CFFProjectilePipebomb::CreateProjectileEffects()
	{
		if( PIPE_MODE == 0 )
		{
			// Start up the eye glow
			m_hMainGlow = CSprite::SpriteCreate( "sprites/redglow1.vmt", GetLocalOrigin(), false );

			int nAttachment = LookupAttachment( "fuse" );

			if ( m_hMainGlow != NULL )
			{
				m_hMainGlow->FollowEntity( this );
				m_hMainGlow->SetAttachment( this, nAttachment );
				m_hMainGlow->SetTransparency( kRenderGlow, 255, 255, 255, 200, kRenderFxNoDissipation );
				m_hMainGlow->SetScale( 0.2f );
				m_hMainGlow->SetGlowProxySize( 4.0f );
				m_hMainGlow->SetThink(&CBaseEntity::SUB_Remove);
				m_hMainGlow->SetNextThink(gpGlobals->curtime + PIPE_DET_DELAY);
			}
		}

		int nAttachment = LookupAttachment( "fuse" );

		// Start up the eye trail
		m_hGlowTrail = CSpriteTrail::SpriteTrailCreate( "sprites/bluelaser1.vmt", GetLocalOrigin(), false );

		if ( m_hGlowTrail != NULL )
		{
			m_hGlowTrail->FollowEntity( this );
			m_hGlowTrail->SetAttachment( this, nAttachment );
			m_hGlowTrail->SetTransparency( kRenderTransAdd, 255, 0, 0, 255, kRenderFxNone );
			m_hGlowTrail->SetStartWidth( 10.0f );
			m_hGlowTrail->SetEndWidth( 5.0f );
			m_hGlowTrail->SetLifeTime( 0.5f );
		}
	}

	//Sets the time to start glowing the sprite, denoting the pipe is going to blow up soon
	void CFFProjectilePipebomb::StartGlowEffect()
	{
		// Start up the eye glow
		m_hMainGlow = CSprite::SpriteCreate( "sprites/redglow1.vmt", GetLocalOrigin(), false );

		int nAttachment = LookupAttachment( "fuse" );

		if ( m_hMainGlow != NULL )
		{
			m_hMainGlow->FollowEntity( this );
			m_hMainGlow->SetAttachment( this, nAttachment );
			m_hMainGlow->SetTransparency( kRenderGlow, 255, 255, 255, 200, kRenderFxNoDissipation );
			m_hMainGlow->SetScale( 0.2f );
			m_hMainGlow->SetGlowProxySize( 4.0f );
			m_hMainGlow->SetThink(&CBaseEntity::SUB_Remove);
			if( PIPE_MODE == 0 )
			{
				m_hMainGlow->SetNextThink(gpGlobals->curtime + PIPE_DET_DELAY);
			}
			else if( PIPE_MODE == 1 )
			{
				m_hMainGlow->SetNextThink(gpGlobals->curtime + PIPE_MAGNETIC_DETONATE_DELAY);
			}
			else if( PIPE_MODE == 2 )
			{
				m_hMainGlow->SetNextThink(gpGlobals->curtime + PIPE_PROXIMITY_DETONATE_DELAY);
			}
		}
	}

	//----------------------------------------------------------------------------
	// Purpose: Detonate the pipebomb(pOther = optional triggerer) 
	//----------------------------------------------------------------------------
	void CFFProjectilePipebomb::DetonatePipe(bool force, CBaseEntity *pOther) 
	{	
		// This is currently live
		//if (!force && gpGlobals->curtime < m_flSpawnTime + pipebomb_time_till_live.GetFloat()) 
		//	return;

		// Transfer ownership before exploding 
		//	eg. if an engineer dets these instead with emp
		if (pOther)
		{
			SetThrower((CBaseCombatCharacter *)pOther);
			SetOwnerEntity(pOther);
		}

		// Detonate!
		SetDetonated(true);
		SetThink(&CFFProjectilePipebomb::Detonate);
		SetNextThink(gpGlobals->curtime);
	}
#endif

void CFFProjectilePipebomb::Precache( void ) 
{

	BaseClass::Precache();

	//Precache the pipe noises
	PrecacheScriptSound("Pipe.Pre_Detonate");

	PrecacheModel("sprites/bluelaser1.vmt");
}

//----------------------------------------------------------------------------
// Purpose: Spawn like a normal grenade but replace skin
//----------------------------------------------------------------------------
void CFFProjectilePipebomb::Spawn() 
{
	BaseClass::Spawn();
	m_nSkin = 0;			// Green skin(#1) 

	m_flSpawnTime = gpGlobals->curtime;

	//Initialize pipe specific data members
	m_bArmed = false;
	m_bMagnetArmed = false;
	m_pMagnetTarget = NULL;
	m_bShouldDetonate = false;
	m_flDetonateTime = 0.0f;
	m_bMagnetActive = false;

	//Set the Pipebomb collision so it doesnt hit players -Green Mushy
	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);

#ifdef CLIENT_DLL
	// Rebo you are quite mean with this tomfoolery!!!!!!!!!!
	player_info_t pinfo;

	if (CBasePlayer::GetLocalPlayer())
	{
		engine->GetPlayerInfo(C_BasePlayer::GetLocalPlayer()->entindex(), &pinfo);
		fAltSkin = ! (pinfo.friendsID & 1);
	}
#else
	// Set the think
	SetThink(&CFFProjectilePipebomb::PipebombThink);		// |-- Mirv: Account for GCC strictness
	SetNextThink(gpGlobals->curtime);
#endif
}

// Added so that grenades aren't using projectiles explode code.
// Grenades might need to look in more places than just below
// them to see if scorch marks can be drawn.
void CFFProjectilePipebomb::Explode( trace_t *pTrace, int bitsDamageType )
{
#ifdef GAME_DLL
	SetModelName( NULL_STRING );//invisible
	AddSolidFlags( FSOLID_NOT_SOLID );

	m_takedamage = DAMAGE_NO;

	if( FFScriptRunPredicates( this, "onexplode", true ) )
	{
		// Pull out of the wall a bit
		if( pTrace->fraction != 1.0 )
			SetLocalOrigin( pTrace->endpos + ( pTrace->plane.normal * 32 ) );

		Vector vecAbsOrigin = GetAbsOrigin();
		int contents = UTIL_PointContents( vecAbsOrigin );

		if( pTrace->fraction != 1.0 ) 
		{
			Vector vecNormal = pTrace->plane.normal;
			surfacedata_t *pdata = physprops->GetSurfaceData( pTrace->surface.surfaceProps );	
			CPASFilter filter( vecAbsOrigin );
			te->Explosion( filter, -1.0, // don't apply cl_interp delay
				&vecAbsOrigin, 
				! ( contents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion, 
				m_flDamage / 128, 
				25, 
				TE_EXPLFLAG_NONE, 
				m_DmgRadius, 
				m_flDamage, 
				&vecNormal, 
				( char )pdata->game.material );

			// Normal decals since trace hit something
			UTIL_DecalTrace( pTrace, "Scorch" );
		}
		else
		{
			CPASFilter filter( vecAbsOrigin );
			te->Explosion( filter, -1.0, // don't apply cl_interp delay
				&vecAbsOrigin, 
				! ( contents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion, 
				m_flDamage / 128, 
				25, 
				TE_EXPLFLAG_NONE, 
				m_DmgRadius, 
				m_flDamage );

			// Trace hit nothing so do custom scorch mark finding
			FF_DecalTrace( this, FF_DECALTRACE_TRACE_DIST, "Scorch" );
		}

		CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );

		CBaseEntity *pThrower = GetThrower();
		// Use the grenade's position as the reported position
		Vector vecReported = pTrace->endpos;
		CTakeDamageInfo info( this, pThrower, GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, 0, &vecReported );
		RadiusDamage( info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );

		EmitSound( "BaseGrenade.Explode" );
	}

	SetThink( &CBaseGrenade::SUB_Remove );
	SetTouch( NULL );

	AddEffects( EF_NODRAW );
	SetAbsVelocity( vec3_origin );
	SetNextThink( gpGlobals->curtime );
#endif
}

#ifdef CLIENT_DLL
//----------------------------------------------------------------------------
// Purpose: Draw model with different skin
//----------------------------------------------------------------------------
int CFFProjectilePipebomb::DrawModel(int flags) 
{
	if (fAltSkin) 
		m_nSkin = 2;		// Yellow skin(#3) 

	return BaseClass::DrawModel(flags);
}
#endif

//----------------------------------------------------------------------------
// Purpose: Destroy all pipes belonging to a player
//----------------------------------------------------------------------------
void CFFProjectilePipebomb::DestroyAllPipes(CBaseEntity *pOwner, bool force) 
{
#ifdef GAME_DLL
	// tell the client to reset the count for the hud
	CFFPlayer *pPipeOwner = ToFFPlayer(pOwner);
	CSingleUserRecipientFilter user(pPipeOwner);
	user.MakeReliable();

	UserMessageBegin(user, "PipeMsg");
		WRITE_BYTE(0);
	MessageEnd();

	// Detonate all the pipes belonging to us
	CFFProjectilePipebomb *pPipe = NULL; 

	// Detonate any pipes belonging to us
	while ((pPipe = (CFFProjectilePipebomb *) gEntList.FindEntityByClassT(pPipe, CLASS_PIPEBOMB)) != NULL) 
	{
		//Check if the pipe is valid and assert if it isnt
		if( pPipe == NULL )
		{
			Assert( "A pipe on detonation was null." );
			return;
		}

		if( pOwner == NULL )
		{
			Assert( "The owner of a pipe was null on detonation." );
			return;
		}

		if( PIPE_MODE == 0 || PIPE_MODE == 2 )
		{
			if (pPipe->GetOwnerEntity() == pOwner && !pPipe->IsDetonated()) 
			{
				pPipe->DetonatePipe(force);
				continue;
			}
		}

		//If it is magnetic mode 1
		if( PIPE_MODE == 1 )
		{
			if (pPipe->GetOwnerEntity() == pOwner)
			{
				//Use this "force" boolean to mean: "Force all pipes to detonate immediately"
				if( force == true )
				{
					// Detonate!				
					pPipe->SetShouldDetonate(true);
					pPipe->SetDetonateTime(gpGlobals->curtime);
				
					//Continue the while loop to the next pipe
					continue;
				}
			

				//Check if the pipe hasnt already been detted, and it is detable
				//Also check if the magnet is armed, meaning it has impacted a solid surface since its creation
				if( pPipe->GetShouldDetonate() == false && pPipe->GetArmed() == true )
				{
					//Emit the sound from each pipe
					pPipe->EmitSoundShared( "Pipe.Pre_Detonate" );

					//Set to true so you cant detonate it again before it explodes
					pPipe->SetShouldDetonate(true);

					//Set the pipes detonation time in the future: MAGNETIC PIPES
					pPipe->SetDetonateTime(gpGlobals->curtime + PIPE_MAGNETIC_DETONATE_DELAY);

					//Start glowing this pipe
					pPipe->StartGlowEffect();
				}
			}
		}
	}
#endif
}

//----------------------------------------------------------------------------
// Purpose: Create a new pipebomb
//----------------------------------------------------------------------------
CFFProjectilePipebomb * CFFProjectilePipebomb::CreatePipebomb(const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iDamageRadius, const int iSpeed) 
{
	CFFProjectilePipebomb *pPipebomb = (CFFProjectilePipebomb *) CreateEntityByName("ff_projectile_pl");

	if( !pPipebomb )
		return NULL;

	UTIL_SetOrigin(pPipebomb, vecOrigin);
	pPipebomb->SetAbsAngles(angAngles);
	pPipebomb->Spawn();
	pPipebomb->SetOwnerEntity(pentOwner);
	pPipebomb->m_iSourceClassname = (pSource ? pSource->m_iClassname : NULL_STRING);

	Vector vecForward;
	AngleVectors(angAngles, &vecForward);

	// Set the speed and the initial transmitted velocity
	pPipebomb->SetAbsVelocity(vecForward * iSpeed);

#ifdef GAME_DLL
	pPipebomb->SetupInitialTransmittedVelocity(vecForward * iSpeed);
	
	pPipebomb->SetDetonateTimerLength(120);

	pPipebomb->SetElasticity(GetGrenadeElasticity());

	pPipebomb->SetDetonated(false);
#endif
	
	//Null out the magnet target
	pPipebomb->SetMagnetTarget(NULL);

	pPipebomb->SetDamage(iDamage);
	pPipebomb->SetDamageRadius(iDamageRadius);

	pPipebomb->m_bIsLive = false;

	pPipebomb->SetThrower(pentOwner); 

	pPipebomb->SetGravity(GetGrenadeGravity());
	pPipebomb->SetFriction(GetGrenadeFriction());

	pPipebomb->SetLocalAngularVelocity(RandomAngle(-400, 400));

#ifdef GAME_DLL
	CFFProjectilePipebomb *pPipe = NULL, *pOldestPipe = NULL;
	int i = 0;

	// Make sure there aren't already too many pipes
	while ((pPipe = (CFFProjectilePipebomb *) gEntList.FindEntityByClassT(pPipe, CLASS_PIPEBOMB)) != NULL) 
	{
		if (pPipe->GetOwnerEntity() == pPipebomb->GetOwnerEntity()) 
		{
			i++;

			if (!pOldestPipe) 
				pOldestPipe = pPipe;

			if (pPipe->m_flSpawnTime < pOldestPipe->m_flSpawnTime) 
				pOldestPipe = pPipe;
		}
	}

	// Too many pipes
	if (i > 8)
		pOldestPipe->DetonatePipe();
	else {
		// tell the client to increment the count for the hud
		CFFPlayer *pPipeOwner = ToFFPlayer(pPipebomb->GetOwnerEntity());
		CSingleUserRecipientFilter user(pPipeOwner);
		user.MakeReliable();

		UserMessageBegin(user, "PipeMsg");
			WRITE_BYTE(1);
		MessageEnd();
	}
#endif

	return pPipebomb; 
}

//----------------------------------------------------------------------------
// Purpose: Grenade think function
//----------------------------------------------------------------------------
void CFFProjectilePipebomb::DecrementHUDCount() 
{
#ifdef GAME_DLL
	// tell the client (demoman) to decrement the hud pipe count
	CFFPlayer *pPipeOwner = ToFFPlayer(this->GetOwnerEntity());
	CSingleUserRecipientFilter user(pPipeOwner);
	user.MakeReliable();

	UserMessageBegin(user, "PipeMsg");
		WRITE_BYTE(2);
	MessageEnd();
#endif
}

//----------------------------------------------------------------------------
// Purpose: Pipe think function
//----------------------------------------------------------------------------
void CFFProjectilePipebomb::PipebombThink() 
{
	if( PIPE_MODE == 0 )
	{
		BaseClass::GrenadeThink();
		return;
	}
	else if( PIPE_MODE == 1 )
	{
		//First check if the pipe has hit a surface yet
		if( m_bMagnetArmed )
		{
			//If the pipe has not yet magnetized to a target
			if( m_bMagnetActive == false )
			{
				//Attempting to check for players in a radius by way of a sphere query
				CBaseEntity *pEntity = NULL;
				for( CEntitySphereQuery sphere( GetAbsOrigin(), PIPE_MAGNET_RADIUS ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
				{
					//If this is not an entity, move on
					if( !pEntity )
						continue;

					//If this is not a player, move on
					if( !pEntity->IsPlayer() )
						continue;

					//Should be a player now ->

					//If the #define says attach to teammates self or teammates
					if( PIPE_FRIENDLY_ATTACH == false )
					{
						//just dont bother magnetizing to the owner, move on
						if( pEntity == GetOwnerEntity() )
							continue;
					}

					//dont magnetize if the player is moving slower then base movespeed
					CFFPlayer* pTarget = ToFFPlayer(pEntity);
					if( pTarget->GetAbsVelocity().Length() < ( pTarget->MaxSpeed() * PIPE_MIN_ACTIVATION_SPEED_FACTOR ))
						continue;

					//If the #define says attach to teammates self or teammates
					if( PIPE_FRIENDLY_ATTACH == false )
					{
						//dont magnetize to teammates of this pipe's owner
						if( pTarget->GetTeam() == GetOwnerEntity()->GetTeam() )
							continue;
					}

					//Checks complete, target acquired->

					//Set the magnet target in the pipe to this entity
					SetMagnetTarget( pEntity );
					
					//Set the magnet bool to true so it doesnt sphere query anymore
					m_bMagnetActive = true;

					DevMsg("Magnet acquired target\n");
					//break out of the loop
					break;
				}
			}

			//If the pipe is currently set to magnetize to somewhere
			if( m_bMagnetActive == true )
			{
				//Call this function to handle the vectors and stuff for magnetizing
				Magnetize( GetMagnetTarget() );
			}
		}

		//Check if this pipe was armed and should detonate
		if( m_bShouldDetonate == true )
		{
			//If the detonate time is up
			if( gpGlobals->curtime > m_flDetonateTime )
			{
				//Stop the pre-det sound: This doesnt work! -GreenMushy
				StopSound("Pipe.Pre-Detonate");

#ifdef GAME_DLL
				//Blow this pipe up
				DetonatePipe( false, GetOwnerEntity() );
#endif
			}
		}

		//Check if the pipe's explosive is active yet
		if( m_bArmed == false )
		{
			//if its been in the world long enough
			if( gpGlobals->curtime > m_flSpawnTime + PIPE_ARM_DELAY )
			{
				//set this bool to true so it can not be detonated
				m_bArmed = true;
			}
		}
	}
	else if( PIPE_MODE == 2 )
	{
#ifdef GAME_DLL
		//Check if this pipe was armed and should detonate
		if( m_bShouldDetonate == true )
		{
			//If the detonate time is up
			if( gpGlobals->curtime > m_flDetonateTime )
			{
				//Stop the pre-det sound: This doesnt work! -GreenMushy
				StopSound("Pipe.Pre-Detonate");

				//Blow this pipe up
				DetonatePipe( false, GetOwnerEntity() );

				//Tell the hud there is 1 less pipe
				DecrementHUDCount();

				//Bail out now
				return;
			}
		}

		//Attempting to check for players in a radius by way of a sphere query
		CBaseEntity *pEntity = NULL;
		for( CEntitySphereQuery sphere( GetAbsOrigin(), PIPE_PROXIMITY_RADIUS ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			//If this is not an entity, move on
			if( !pEntity )
				continue;

			//If this is not a player, move on
			if( !pEntity->IsPlayer() )
				continue;

			//just dont bother magnetizing to the owner, move on
			if( pEntity == GetOwnerEntity() )
			{
				continue;
			}

			//Should be a player now ->
			CFFPlayer* pTarget = ToFFPlayer(pEntity);

			//Find the distance from this pipe to the target
			Vector vDist = pTarget->GetAbsOrigin() - GetAbsOrigin();
			if( vDist.Length() > PIPE_PROXIMITY_RADIUS  )
			{
				//Outside the radius, continue to next target
				continue;
			}

			//dont magnetize to teammates of this pipe's owner
			if( pTarget->GetTeam() == GetOwnerEntity()->GetTeam() )
			{
				continue;
			}
		
			//Check if it is able to detonate
			if( GetShouldDetonate() == false && GetArmed() == true)
			{
				//Arm the pipe and detonate in the future
				//Emit the sound from each pipe
				EmitSoundShared( "Pipe.Pre_Detonate" );

				//This pipe is in detonate mode now
				SetShouldDetonate(true);

				//Set the pipes detonation time in the future
				SetDetonateTime(gpGlobals->curtime + PIPE_PROXIMITY_DETONATE_DELAY );

				//Start glowing this pipe
				StartGlowEffect();
			}
		}
#endif		
	}

	BaseClass::GrenadeThink();
}

//----------------------------------------------------------------------------
// Purpose: Magnetizes the pipe toward a target function
//----------------------------------------------------------------------------
void CFFProjectilePipebomb::Magnetize( CBaseEntity* _pTarget )
{
	//If there is no valid target, gtfo
	if( _pTarget == NULL )
		return;

	//Target is dead, reset the magnet and physics then gtfo
	if( !(_pTarget->IsAlive()) )
	{
		//Set the magnet active to false so it stops moving
		m_bMagnetActive = false;

		//null out the target
		m_pMagnetTarget = NULL;

		//null out the ground so gravity will start applying
		SetGroundEntity( (CBaseEntity *)NULL );

		//Reset the move type to the default grenade type so gravity effects it again 
		// keep the speed so it does more realistic falling
		SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
		
		//Send msg for debugging
		DevMsg("Pipebomb target died.  Detaching\n");
		
		//gtfo this function
		return;
	}

	//Declare the direction the pipe will magnetize to
	Vector vMoveDir;

	//Get the vector that is the difference between this pipe and the target
	vMoveDir = _pTarget->GetAbsOrigin() - GetAbsOrigin();

	//If the vector between the target and pipe are too large
	if( vMoveDir.Length() > PIPE_MAX_FOLLOW_DIST )
	{
		//Set the magnet active to false so it stops moving
		m_bMagnetActive = false;

		//null out the target
		m_pMagnetTarget = NULL;

		//null out the ground so gravity will start applying
		SetGroundEntity( (CBaseEntity *)NULL );

		//Reset the move type to the default grenade type so gravity effects it again 
		// keep the speed so it does more realistic falling
		SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
		
		//Send msg for debugging
		DevMsg("Pipebomb reached chase limit.. Detaching\n");
		
		//gtfo this function
		return;
	}

	//If the player is really slow, the pipes catch up, and they keep wierdly magnetizing into the player's origin
	// so if the player is slow, just set the pipe to abide by gravity again
	CFFPlayer* pPlayer = ToFFPlayer(_pTarget);
	if( pPlayer->GetAbsVelocity().Length() < ( pPlayer->MaxSpeed() * PIPE_MIN_ACTIVATION_SPEED_FACTOR ))
	{
		//Set the magnet active to false to stop moving
		m_bMagnetActive = false;

		//null out the target
		m_pMagnetTarget = NULL;

		//null out the ground so gravity will start applying
		SetGroundEntity( (CBaseEntity *)NULL );

		//Reset the move type to the default grenade type so gravity effects it again 
		// keep the speed so it does more realistic falling
		SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);

		//Send msg for debugging
		DevMsg("Pipebomb target moving too slow.. Detaching\n");

		//Send the pipes to randomize their velocity to spread out
		RandomizePipeVelocity();

		//gtfo this function
		return;
	}

	//Normalize the vector 
	VectorNormalize(vMoveDir);

	//Set the movement type to fly without gravity while magnetizing, and slide collisions to help round corners(hopefully)
	SetMoveType( MOVETYPE_FLY, MOVECOLLIDE_FLY_SLIDE );

	//Set the velocity to the targets move speed + a factor we feel is good
	SetAbsVelocity( (vMoveDir * _pTarget->GetAbsVelocity().Length()) * PIPE_FOLLOW_SPEED_FACTOR );
}

//----------------------------------------------------------------------------
// Purpose: Override this virtual function to arm the magnet on impact
//----------------------------------------------------------------------------
void CFFProjectilePipebomb::ResolveFlyCollisionCustom(trace_t &trace, Vector &vecVelocity)
{
	if( vecVelocity.Length() < (float)PIPE_ACTIVATION_SPEED )
	{
		if( PIPE_MODE == 1 )
		{
			//Arm the magnet upon impact
			m_bMagnetArmed = true;

			//Send dev msg for debugging
			DevMsg("Magnet Armed\n");
		}
		else if( PIPE_MODE == 2 )
		{
			//Use this armed variable for this mode
			m_bArmed = true;
		}
	}

	//Now do the normal base class collision stuff
	BaseClass::ResolveFlyCollisionCustom( trace, vecVelocity );
}

//----------------------------------------------------------------------------
//Purpose: Randomize the location of a pipe 
//----------------------------------------------------------------------------
void CFFProjectilePipebomb::RandomizePipeVelocity()
{
	//Declare the static that will behave as the seed for all pipe randomizing
	static int pipe_seed = 0;

	//Declare the direction vector
	Vector vRandomDir;

	// init random system with this seed
	RandomSeed( pipe_seed );

	// increment the seed for later use
	pipe_seed++;

	// Get circular gaussian spread.
	int x = RandomInt( -PIPE_RANDOMIZE_RADIUS, PIPE_RANDOMIZE_RADIUS );
	int y = RandomInt( -PIPE_RANDOMIZE_RADIUS, PIPE_RANDOMIZE_RADIUS );

	//Plug the x and y values in.  I dont want a z direction
	vRandomDir = Vector(x,y,0);

	//Normalize the vector 
	VectorNormalize(vRandomDir);

	//Set the velocity
	SetAbsVelocity( vRandomDir * PIPE_RANDOMIZE_SPEED );
}
