/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_rocket.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF rocket projectile code.
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation logged


#include "cbase.h"
#include "ff_projectile_hook.h"
#include "ff_weapon_hookgun.h"
#include "IEffects.h"
#include "movevars_shared.h"
#ifdef GAME_DLL
	#include "ff_player.h"
#endif

#define HOOK_MODEL "models/grenades/caltrop/caltrop.mdl"

ConVar ffdev_hook_range( "ffdev_hook_range", "1000.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Grappling hook range" );
#define HOOK_RANGE ffdev_hook_range.GetFloat()
ConVar ffdev_hook_closerange( "ffdev_hook_closerange", "80.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Grappling hook close range" );
#define HOOK_CLOSERANGE ffdev_hook_closerange.GetFloat()
ConVar ffdev_hook_firespeed( "ffdev_hook_firespeed", "1500.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Grappling hook fire speed" );
#define HOOK_FIRESPEED ffdev_hook_firespeed.GetFloat()
ConVar ffdev_hook_pullspeed( "ffdev_hook_pullspeed", "650.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Grappling hook pull speed" );
#define HOOK_PULLSPEED ffdev_hook_pullspeed.GetFloat()



#define ROPE_MATERIAL			"cable/rope_b.vmt"
#define ROPE_MATERIAL_BLUE		"cable/rope_b.vmt"
#define ROPE_MATERIAL_RED		"cable/rope_r.vmt"
#define ROPE_MATERIAL_YELLOW	"cable/rope_y.vmt"
#define ROPE_MATERIAL_GREEN		"cable/rope_g.vmt"

#ifdef CLIENT_DLL
	#include "tempentity.h"
	#include "iefx.h"
#endif


//ConVar ffdev_rocketsize("ffdev_rocketsize", "2.0", FCVAR_REPLICATED );
#define FFDEV_HOOKSIZE 1.0f //ffdev_rocketsize.GetFloat() //2.0f
//ConVar ffdev_hook_attachment("ffdev_hook_attachment", "23.0", FCVAR_REPLICATED );
#define FFDEV_HOOK_ATTACHMENT 23 //ffdev_hook_attachment.GetInt() //2.0f  // 0=feet, 1=eyes, 2=shoulder, 3=knee... 23=left hand with floppy rope. Dont ask me.
//ConVar ffdev_hook_rope_hangdistance("ffdev_hook_rope_hangdistance", "2.2", FCVAR_REPLICATED );
#define FFDEV_HOOK_ROPE_HANGDISTANCE 2.2f //ffdev_hook_rope_hangdistance.GetFloat() //2.0f
ConVar ffdev_hook_rope_segments("ffdev_hook_rope_segments", "3", FCVAR_REPLICATED );
#define FFDEV_HOOK_ROPE_SEGMENTS ffdev_hook_rope_segments.GetInt()

// caes: testing
ConVar ffdev_hook_end_on_jump( "ffdev_hook_end_on_jump", "1", FCVAR_REPLICATED, "end hook if pressing jump and have ever had jump not pressed since last on ground" );
ConVar ffdev_hook_swing( "ffdev_hook_swing", "1", FCVAR_REPLICATED, "[0/1/2] - winch system 1: pull speed falls off linearly as force on rope increases; rope can't extend. winch system 2: applies constant force on rope when in air; rope can't extend; max pull speed capped; when on ground sets you to max pull speed if pull is horizontal and gives small kick if pull is vertical." );
ConVar ffdev_hook_swing_break( "ffdev_hook_swing_break", "0.8", FCVAR_REPLICATED, "end hook " );
ConVar ffdev_hook_swing1_speed( "ffdev_hook_swing1_speed", "750.0", FCVAR_REPLICATED, "pull speed when no force on rope" );
ConVar ffdev_hook_swing1_falloff( "ffdev_hook_swing1_falloff", "0.0001", FCVAR_REPLICATED, "rate pull speed falls off as force on rope increases" );
ConVar ffdev_hook_swing1_falloff_power( "ffdev_hook_swing1_falloff_power", "2.0", FCVAR_REPLICATED, "" );
ConVar ffdev_hook_swing2_speed( "ffdev_hook_swing2_speed", "750.0", FCVAR_REPLICATED, "max pull speed cap (and horizontal pull speed when on ground)" );
ConVar ffdev_hook_swing2_speed_v( "ffdev_hook_swing2_speed_v", "100.0", FCVAR_REPLICATED, "vertical pull speed when on ground" );
ConVar ffdev_hook_swing2_force( "ffdev_hook_swing2_force", "2000.0", FCVAR_REPLICATED, "constant force applied on rope when in air" );

ConVar ffdev_hook_swing2_elasticity( "ffdev_hook_swing2_elasticity", "1.2", FCVAR_REPLICATED | FCVAR_CHEAT, "Grappling hook swing2 system elasticity - lower numbers will snap the rope easier if you pull on it" );
#define FFDEV_HOOK_SWING2_ELASTICITY ffdev_hook_swing2_elasticity.GetFloat()
// caes

//#define PREDICTED_ROCKETS

//=============================================================================
// CFFProjectileHook tables
//=============================================================================

class CRecvProxyData;
extern void RecvProxy_LocalVelocityX(const CRecvProxyData *pData, void *pStruct, void *pOut);
extern void RecvProxy_LocalVelocityY(const CRecvProxyData *pData, void *pStruct, void *pOut);
extern void RecvProxy_LocalVelocityZ(const CRecvProxyData *pData, void *pStruct, void *pOut);

IMPLEMENT_NETWORKCLASS_ALIASED(FFProjectileHook, DT_FFProjectileHook)

BEGIN_NETWORK_TABLE(CFFProjectileHook, DT_FFProjectileHook)
#ifdef CLIENT_DLL
	RecvPropFloat		( RECVINFO(m_vecVelocity[0]), 0, RecvProxy_LocalVelocityX ),
	RecvPropFloat		( RECVINFO(m_vecVelocity[1]), 0, RecvProxy_LocalVelocityY ),
	RecvPropFloat		( RECVINFO(m_vecVelocity[2]), 0, RecvProxy_LocalVelocityZ ),
#else
	// Increased range for this because it goes at a mad speed
	SendPropExclude("DT_BaseGrenade", "m_vecVelocity[0]"),
	SendPropExclude("DT_BaseGrenade", "m_vecVelocity[1]"),
	SendPropExclude("DT_BaseGrenade", "m_vecVelocity[2]"),
	SendPropFloat( SENDINFO_VECTORELEM(m_vecVelocity, 0), 16, SPROP_ROUNDDOWN, -4096.0f, 4096.0f ),
	SendPropFloat( SENDINFO_VECTORELEM(m_vecVelocity, 1), 16, SPROP_ROUNDDOWN, -4096.0f, 4096.0f ),
	SendPropFloat( SENDINFO_VECTORELEM(m_vecVelocity, 2), 16, SPROP_ROUNDDOWN, -4096.0f, 4096.0f ),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(ff_projectile_hook, CFFProjectileHook);
PRECACHE_WEAPON_REGISTER(ff_projectile_hook);


//=============================================================================
// CFFProjectileHook implementation
//=============================================================================

#ifdef GAME_DLL
	// Bug #0000436: Need to truncate Rocket travel sound on impact.
	BEGIN_DATADESC( CFFProjectileHook )
		// Function Pointers
		DEFINE_THINKFUNC( HookThink ), 
		DEFINE_ENTITYFUNC( HookTouch ),
	END_DATADESC()
#endif


	//----------------------------------------------------------------------------
	// Purpose: Client constructor
	//----------------------------------------------------------------------------
	CFFProjectileHook::CFFProjectileHook()
	{
#ifdef GAME_DLL
		m_hRope				= NULL;
#endif
	}

#ifdef CLIENT_DLL
	//-----------------------------------------------------------------------------
	// Purpose: Remove the rocket trail
	//-----------------------------------------------------------------------------
	void CFFProjectileHook::CleanUp()
	{
		BaseClass::CleanUp();
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	bool CFFProjectileHook::ShouldPredict()
	{
#ifdef PREDICTED_ROCKETS
		if (GetOwnerEntity() && GetOwnerEntity() == C_BasePlayer::GetLocalPlayer())
			return true;
#endif
		return BaseClass::ShouldPredict();
	}

	extern short	g_sModelIndexFireball;		// (in combatweapon.cpp) holds the index for the fireball 
	extern short	g_sModelIndexWExplosion;	// (in combatweapon.cpp) holds the index for the underwater explosion

	//-----------------------------------------------------------------------------
	// Purpose: Called when data changes on the server
	//-----------------------------------------------------------------------------
	void CFFProjectileHook::OnDataChanged( DataUpdateType_t updateType )
	{
		// NOTE: We MUST call the base classes' implementation of this function
		BaseClass::OnDataChanged( updateType );

		// Setup our entity's particle system on creation
		if ( updateType == DATA_UPDATE_CREATED )
		{
			// Call our ClientThink() function once every client frame
			SetNextClientThink( CLIENT_THINK_ALWAYS );
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Client-side think function for the entity
	//-----------------------------------------------------------------------------
	void CFFProjectileHook::ClientThink( void )
	{

	}

#endif

//----------------------------------------------------------------------------
// Purpose: Spawn a rocket, set up model, size, etc
//----------------------------------------------------------------------------
void CFFProjectileHook::Spawn() 
{
	BaseClass::Spawn();

#if defined(CLIENT_DLL) && !defined(PREDICTED_ROCKETS)
	return;
#endif

	// Setup
	SetModel(HOOK_MODEL);
	SetMoveType(MOVETYPE_FLY);
	SetSize(Vector(-(FFDEV_HOOKSIZE), -(FFDEV_HOOKSIZE), -(FFDEV_HOOKSIZE)), Vector((FFDEV_HOOKSIZE), (FFDEV_HOOKSIZE), (FFDEV_HOOKSIZE))); // smaller, cube bounding box so we rest on the ground
	SetSolid(SOLID_BBOX);	// So it will collide with physics props!
	SetSolidFlags(FSOLID_NOT_STANDABLE);
	SetCollisionGroup( COLLISION_GROUP_DEBRIS );

	// Set the correct think & touch for the nail
	SetTouch(&CFFProjectileHook::HookTouch); // No we're going to explode when we touch something
	SetThink(&CFFProjectileHook::HookThink);		// no thinking, yet!

	// Next think
	SetNextThink(gpGlobals->curtime);
}

//----------------------------------------------------------------------------
// Purpose: Precache the rocket model
//----------------------------------------------------------------------------
void CFFProjectileHook::Precache() 
{
	PrecacheModel(HOOK_MODEL);
	PrecacheModel(ROPE_MATERIAL);
	PrecacheModel(ROPE_MATERIAL_BLUE);
	PrecacheModel(ROPE_MATERIAL_RED);
	PrecacheModel(ROPE_MATERIAL_YELLOW);
	PrecacheModel(ROPE_MATERIAL_GREEN);	

	PrecacheScriptSound("rocket.fly");

	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: What happens when the hook touches something
//----------------------------------------------------------------------------
void CFFProjectileHook::HookTouch(CBaseEntity *pOther) 
{
	// The projectile has not hit anything valid so far
	if (!pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS) || !g_pGameRules->ShouldCollide(GetCollisionGroup(), pOther->GetCollisionGroup())) 
		return;

//#ifdef GAME_DLL
//	NDebugOverlay::EntityBounds(this, 0, 0, 255, 100, 5.0f);
//#endif

	trace_t	tr;
	tr = BaseClass::GetTouchTrace();

	// Remove the hook if it hits the sky
	if(tr.surface.flags & SURF_SKY)
	{
		RemoveHook();
		return;
	}

	// This entity can take damage, so deal it out
	if (pOther->m_takedamage != DAMAGE_NO) 
	{
#ifdef GAME_DLL
		Vector	vecNormalizedVel = GetAbsVelocity();
		VectorNormalize(vecNormalizedVel);

		ClearMultiDamage();

		CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), m_flDamage, DMG_BULLET | DMG_NEVERGIB);
		//CalculateBulletDamageForce(&dmgInfo, GetAmmoDef()->Index("AMMO_NAILS"), vecNormalizedVel, tr.endpos);
		//dmgInfo.SetDamagePosition(tr.endpos);

		//if (pOther->IsPlayer())
		//{
		//	dmgInfo.ScaleDamageForce( FF_NAIL_PUSHMULTIPLIER );
		//}
		//else if( ( pOther->Classify() == CLASS_SENTRYGUN ) && m_bNailGrenadeNail )
		//{
			// Modify the damage +- cvar value
			dmgInfo.SetDamage( 5.0f );
		//}

		pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);

		ApplyMultiDamage();
#endif

		// Keep going through the glass.
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS) 
			 return;
	}

	// Play body "thwack" sound
	EmitSound("Nail.HitBody");

	if ( UTIL_PointContents( GetAbsOrigin() ) != CONTENTS_WATER)
	{
		g_pEffects->Sparks( GetAbsOrigin() );
	}

#ifdef GAME_DLL
	if ( m_hRope )
		m_hRope->SetupHangDistance( 0 );
#endif

	SetLocalAngularVelocity(QAngle(0, 0, 0));	// stop spinning
	SetAbsVelocity(Vector(0,0,0));				// stop moving
	bHooked = true;
	// caes: end hook if pressing jump and have ever had jump not pressed since last on ground
	bBeenNotJumping = false;
	// caes
}

//----------------------------------------------------------------------------
// Purpose: Think!
//----------------------------------------------------------------------------
void CFFProjectileHook::HookThink() 
{
#ifdef GAME_DLL
	//DevMsg("Hook think!!  ");


	SetNextThink(gpGlobals->curtime + 1.0f);

	// Remove if we're nolonger in the world
	if (!IsInWorld())
	{
		RemoveHook();
		return;
	}

	CBaseEntity *pOwner = GetOwnerEntity(); // Get player

	if ( !pOwner )
	{
		RemoveHook();
		return;
	}
	if ( !pOwner->IsAlive() )
	{
		RemoveHook();
		return;
	}
	/*
	CFFPlayer *pOwner = dynamic_cast< CFFPlayer* > ( GetOwnerEntity() );

	// remove if we let go of attack
	if ( !( pOwner->m_nButtons & IN_ATTACK ) )
	{
		//CancelHook();
		RemoveHook();
		return;
	}
	*/

	// remove if we can't see our owner any more
	if ( !FVisible( pOwner->GetAbsOrigin() ) )
	{
		EmitSound("hookgun.rope_snap");		
		RemoveHook();
		return;
	}

	// remove if we're too far from our owner
	float flDistance = WorldSpaceCenter().DistTo( pOwner->GetAbsOrigin() );
	if ( flDistance > HOOK_RANGE || ( bHooked && flDistance < HOOK_CLOSERANGE ) )
	{
		EmitSound("hookgun.rope_snap");
		RemoveHook();
		return;
	}

	// Bug #0000501: Doors can be blocked by shit that shouldn't block them.
	if( GetGroundEntity() && ( GetAbsVelocity() == vec3_origin ) )
	{
		if( GetGroundEntity()->GetMoveType() != MOVETYPE_PUSH )
			SetMoveType( MOVETYPE_NONE );
	}

	// hook attached
	if ( bHooked )
	{
		// caes: end hook if pressing jump and have ever had jump not pressed since last on ground
		if ( ffdev_hook_end_on_jump.GetBool() )
		{
			CFFPlayer *pPlayer = ToFFPlayer( pOwner );
			if ( !pPlayer->IsOnGround() )
			{
				if ( pPlayer->m_nButtons & IN_JUMP )
				{
					if ( bBeenNotJumping )
					{
						EmitSound("hookgun.rope_snap");
						RemoveHook();
						return;
					}
				}
				else if ( !bBeenNotJumping )
				{
					bBeenNotJumping = true;
				}
			}
			else
			{
				bBeenNotJumping = false;
			}
		}
		// caes


		// caes: testing different movement systems
		if ( ffdev_hook_swing.GetInt() == 0 )
		{
			// This is the actual code for how the hook moves the player
			Vector vecPullDir = GetAbsOrigin() - pOwner->GetAbsOrigin();
			VectorNormalize( vecPullDir );
			vecPullDir*= HOOK_PULLSPEED;
			pOwner->SetAbsVelocity( vecPullDir );
		}

		else
		{
			// get direction from player to hook
			Vector vecPullDir = GetAbsOrigin() - pOwner->GetAbsOrigin();
			VectorNormalize( vecPullDir );

			// get player velocity
			Vector vecVel;
			pOwner->GetVelocity( &vecVel );

			// calculate swing direction
			Vector vecSwingDir = CrossProduct( CrossProduct( vecPullDir, vecVel ), vecPullDir );
			VectorNormalize( vecSwingDir );

			// current speed in swing and radial directions
			float flSwingSpeed = DotProduct( vecVel, vecSwingDir );
			float flRadialSpeed = DotProduct( vecVel, vecPullDir );

			// radial accelerations needed for rope to stay the same length
			float flCentripetalAccel = flSwingSpeed * flSwingSpeed / flDistance;
			float flRadialGravityAccel = sv_gravity.GetFloat() * DotProduct( Vector(0.0f,0.0f,1.0f), vecPullDir );

			// just here while testing
			float flPullSpeed = 0.0f;

			// winch system 1: pull speed falls off linearly as force on rope increases;rope can't extend.
			if ( ffdev_hook_swing.GetInt() == 1 )
			{
				// positive radial acceleration needed from tension in the rope for it to stay the same length
				float flRadialAccel = max( 0.0f, flCentripetalAccel + flRadialGravityAccel );
				// winch pull speed falls off linearly as the tension in the rope increases (mass is constant so within the cvar) and can't be negative
				flPullSpeed = max( 0.0f, ffdev_hook_swing1_speed.GetFloat() - ffdev_hook_swing1_falloff.GetFloat() * pow( flRadialAccel, ffdev_hook_swing1_falloff_power.GetFloat() ) );
				// stop gravity changing radial speed during the next tick (we already took gravity into account above)
				flPullSpeed += flRadialGravityAccel * gpGlobals->interval_per_tick;
			}

			// winch system 2: applies constant force on rope when in air; rope can't extend; max pull speed capped; when on ground sets you to max pull speed if pull is horizontal and gives small kick if pull is vertical.
			else if ( ffdev_hook_swing.GetInt() == 2 )
			{
				CFFPlayer *pPlayer = ToFFPlayer( pOwner );
				if ( !pPlayer->IsOnGround() )
				{
					// radial acceleration available to counter gravity and pull player in (mass is constant so within the cvar)
					float flRadialAccel = ffdev_hook_swing2_force.GetFloat() - flCentripetalAccel;
					// apply radial acceleration to player's radial speed
					flPullSpeed = flRadialSpeed + flRadialAccel * gpGlobals->interval_per_tick;
					// make sure pull speed is enough for gravity not to extend the rope in the next tick
					flPullSpeed = max( flPullSpeed, flRadialGravityAccel * gpGlobals->interval_per_tick * FFDEV_HOOK_SWING2_ELASTICITY );
					// cap maximum speed winch can pull you in at
					flPullSpeed = min( flPullSpeed, ffdev_hook_swing2_speed.GetFloat() );
				}
				else
				{
					// calculate forwards direction
					Vector vecForwardDir = CrossProduct( CrossProduct( Vector(0.0f,0.0f,1.0f), vecPullDir ), Vector(0.0f,0.0f,1.0f) );
					VectorNormalize( vecForwardDir );
					// calculate ground kick vector
					Vector vecGroundKick = vecForwardDir*ffdev_hook_swing2_speed.GetFloat() + Vector(0.0f,0.0f,1.0f)*ffdev_hook_swing2_speed_v.GetFloat();
					// project ground kick onto pull direction
					flPullSpeed = DotProduct( vecPullDir, vecGroundKick );
				}
			}

			// rope can only do tension, so don't decrease player's radial speed
			flPullSpeed = max( flPullSpeed, flRadialSpeed );
			// set resultant velocity (gravity is applied by the game later)
			Vector VecNewVel = vecPullDir*flPullSpeed + vecSwingDir*flSwingSpeed;
			pOwner->SetAbsVelocity( VecNewVel );

			// testing end hook ideas
			VectorNormalize( VecNewVel );
			float flNewUp = DotProduct( VecNewVel, Vector(0.0f,0.0f,1.0f) );
			if ( ( flSwingSpeed > flPullSpeed ) && ( flNewUp >= ffdev_hook_swing_break.GetFloat() ) )
			{
				RemoveHook();
				EmitSound("hookgun.rope_snap");
				return;
			}
		}
		// caes
	}

#ifdef GAME_DLL
	if ( m_hRope )
		m_hRope->RecalculateLength();
#endif

#endif
	// Next think straight away
	SetNextThink(gpGlobals->curtime + 0.01f); // think every tick (since it shouldnt multiply the effect, just will stop gravity doing odd things inbetween ticks
}

//----------------------------------------------------------------------------
// Purpose: Remove the hook from world
//----------------------------------------------------------------------------
void CFFProjectileHook::RemoveHook() 
{
#ifdef GAME_DLL
	if ( m_hRope )
	{
		UTIL_Remove( m_hRope );
		m_hRope = NULL;
	}
#endif

	
	CFFWeaponHookGun *pOwnerGun = dynamic_cast< CFFWeaponHookGun * >( m_pOwnerGun );
	if ( pOwnerGun )
		pOwnerGun->m_pHook = NULL;
		
	Remove();
}
//----------------------------------------------------------------------------
// Purpose: Create a new hook (hacked from ff_projectile_rocket.cpp)
//----------------------------------------------------------------------------
CFFProjectileHook * CFFProjectileHook::CreateHook(CBaseEntity *pOwnerGun, const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwnerPlayer) 
{
	CFFProjectileHook *pHook;
	
#ifdef PREDICTED_ROCKETS
	if (pentOwnerPlayer->IsPlayer()) 
	{
		pHook = (CFFProjectileHook *) CREATE_PREDICTED_ENTITY("ff_projectile_hook");
		pHook->SetPlayerSimulated(ToBasePlayer(pentOwnerPlayer));
	}
	else
#endif
	{
		pHook = (CFFProjectileHook *) CreateEntityByName("ff_projectile_hook");
	}

	UTIL_SetOrigin(pHook, vecOrigin);
	pHook->SetAbsAngles(angAngles);
	pHook->Spawn();
	pHook->SetOwnerEntity(pentOwnerPlayer);
	pHook->m_iSourceClassname = (pentOwnerPlayer ? pentOwnerPlayer->m_iClassname : NULL_STRING);
	pHook->m_pOwnerGun = pOwnerGun;

	Vector vecForward;
	AngleVectors(angAngles, &vecForward);

	// Set the speed and the initial transmitted velocity
	pHook->SetAbsVelocity(vecForward * HOOK_FIRESPEED);

#ifdef GAME_DLL
	pHook->SetupInitialTransmittedVelocity(vecForward * HOOK_FIRESPEED);

	//pHook->m_flDamage = iDamage;
	//pHook->m_DmgRadius = iDamageRadius;
#endif

	pHook->bHooked = false;
	//pRocket->EmitSound("rocket.fly");
	// this is being swapped over to the client -mirv

	CPASFilter filter( vecOrigin );
	
#ifdef GAME_DLL

	switch( pentOwnerPlayer->GetTeamNumber() )
	{
		case TEAM_BLUE: pHook->m_hRope = CRopeKeyframe::Create( pHook, pentOwnerPlayer, 1, FFDEV_HOOK_ATTACHMENT, 2, ROPE_MATERIAL_BLUE ); break;
		case TEAM_RED: pHook->m_hRope = CRopeKeyframe::Create( pHook, pentOwnerPlayer, 1, FFDEV_HOOK_ATTACHMENT, 2, ROPE_MATERIAL_RED ); break;
		case TEAM_YELLOW: pHook->m_hRope = CRopeKeyframe::Create( pHook, pentOwnerPlayer, 1, FFDEV_HOOK_ATTACHMENT, 2, ROPE_MATERIAL_YELLOW ); break;
		case TEAM_GREEN: pHook->m_hRope = CRopeKeyframe::Create( pHook, pentOwnerPlayer, 1, FFDEV_HOOK_ATTACHMENT, 2, ROPE_MATERIAL_GREEN ); break;
	}
 
	if ( pHook->m_hRope )
	{
		pHook->m_hRope->m_Width = 2;
		pHook->m_hRope->m_nSegments = FFDEV_HOOK_ROPE_SEGMENTS; // This is like 10+9*8 segments = 41, which is a lot, could probably cut this down.
		pHook->m_hRope->EnableWind( false );
	//	pHook->m_hRope->EnableCollision(); // Collision looks worse than no collision
		pHook->m_hRope->SetupHangDistance( FFDEV_HOOK_ROPE_HANGDISTANCE ); 
		pHook->m_hRope->SetParent( pHook, 1 );
	}
#endif

	return pHook; 
}

