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
#include "IEffects.h"
#ifdef GAME_DLL
	#include "ff_player.h"
#endif

#define ROCKET_MODEL "models/projectiles/rocket/w_rocket.mdl"

ConVar ffdev_hook_range( "ffdev_hook_range", "1000.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Grappling hook range" );
#define HOOK_RANGE ffdev_hook_range.GetFloat()

ConVar ffdev_hook_closerange( "ffdev_hook_closerange", "80.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Grappling hook close range" );
#define HOOK_CLOSERANGE ffdev_hook_closerange.GetFloat()

ConVar ffdev_hook_firespeed( "ffdev_hook_firespeed", "1000.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Grappling hook fire speed" );
#define HOOK_FIRESPEED ffdev_hook_firespeed.GetFloat()

ConVar ffdev_hook_pullspeed( "ffdev_hook_pullspeed", "650.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Grappling hook pull speed" );
#define HOOK_PULLSPEED ffdev_hook_pullspeed.GetFloat()

#define ROPE_MATERIAL "cable/rope.vmt"

#ifdef GAME_DLL
	//#include "smoke_trail.h"
#else
	//#define RocketTrail C_RocketTrail
	//#include "c_smoke_trail.h"
	#include "tempentity.h"
	#include "iefx.h"

#endif


//ConVar ffdev_rocketsize("ffdev_rocketsize", "2.0", FCVAR_REPLICATED );
#define FFDEV_ROCKETSIZE 2.0f //ffdev_rocketsize.GetFloat() //2.0f
//#define PREDICTED_ROCKETS

//=============================================================================
// CFFProjectileRocket tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFProjectileHook, DT_FFProjectileHook)

BEGIN_NETWORK_TABLE(CFFProjectileHook, DT_FFProjectileHook)
#ifdef GAME_DLL
//SendPropEHandle(SENDINFO(m_hRocketTrail)),
#else
//RecvPropEHandle(RECVINFO(m_hRocketTrail)),
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
		//if (m_hRocketTrail)
		//	m_hRocketTrail->m_bEmit = false;

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
// Purpose: Creata a trail of smoke for the rocket
//----------------------------------------------------------------------------
	/*
void CFFProjectileHook::CreateSmokeTrail() 
{
#ifdef GAME_DLL
	// Smoke trail.
	if ((m_hRocketTrail = RocketTrail::CreateRocketTrail()) != NULL) 
	{
		m_hRocketTrail->m_Opacity = 0.2f;
		m_hRocketTrail->m_SpawnRate = 100;
		m_hRocketTrail->m_ParticleLifetime = 0.5f;
		m_hRocketTrail->m_StartColor.Init(0.65f, 0.65f , 0.65f);
		m_hRocketTrail->m_EndColor.Init(0.45f, 0.45f, 0.45f);
		m_hRocketTrail->m_StartSize = 6;
		m_hRocketTrail->m_EndSize = 10; // 24; // 32; Reduced a bit now
		m_hRocketTrail->m_SpawnRadius = 4;
		m_hRocketTrail->m_MinSpeed = 2;
		m_hRocketTrail->m_MaxSpeed = 16;
		
		m_hRocketTrail->SetLifetime(999);
		m_hRocketTrail->FollowEntity(this, "0");
	}
#endif
}
*/
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
	SetModel(ROCKET_MODEL);
	SetMoveType(MOVETYPE_FLY);
	SetSize(Vector(-(FFDEV_ROCKETSIZE), -(FFDEV_ROCKETSIZE), -(FFDEV_ROCKETSIZE)), Vector((FFDEV_ROCKETSIZE), (FFDEV_ROCKETSIZE), (FFDEV_ROCKETSIZE))); // smaller, cube bounding box so we rest on the ground
	SetSolid(SOLID_BBOX);	// So it will collide with physics props!
	SetSolidFlags(FSOLID_NOT_STANDABLE);

	// Set the correct think & touch for the nail
	SetTouch(&CFFProjectileHook::HookTouch); // No we're going to explode when we touch something
	SetThink(&CFFProjectileHook::HookThink);		// no thinking, yet!

	// Next think
	SetNextThink(gpGlobals->curtime);

	// Creates the smoke trail
	//CreateSmokeTrail();
}

//----------------------------------------------------------------------------
// Purpose: Precache the rocket model
//----------------------------------------------------------------------------
void CFFProjectileHook::Precache() 
{
	PrecacheModel(ROCKET_MODEL);
	PrecacheModel("cable/cable.vmt"); //rope material	
	PrecacheModel(ROPE_MATERIAL); //rope material	
	
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

		// Play body "thwack" sound
		EmitSound("Nail.HitBody");
	}

	// Now just remove the nail
	//Remove();
	if ( UTIL_PointContents( GetAbsOrigin() ) != CONTENTS_WATER)
	{
		g_pEffects->Sparks( GetAbsOrigin() );
	}

#ifdef GAME_DLL
	if ( m_hRope )
		m_hRope->SetupHangDistance( 0 );
#endif

	SetAbsVelocity(Vector(0,0,0));
	bHooked = true;
	//SetThink( &CFFProjectileHook::HookThink ); // start thinking! (pulling the owner towards you)
	//SetNextThink( gpGlobals->curtime );
}

//----------------------------------------------------------------------------
// Purpose: Think!
//----------------------------------------------------------------------------
void CFFProjectileHook::HookThink() 
{
#ifdef GAME_DLL
	DevMsg("Hook think!!  ");

	SetNextThink(gpGlobals->curtime + 1.0f);

	// Remove if we're nolonger in the world
	if (!IsInWorld())
	{
		RemoveHook();
		return;
	}

	CBaseEntity *pOwner = GetOwnerEntity();

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
		//CancelHook();
		RemoveHook();
		return;
	}

	// remove if we're too far from our owner
	float flDistance = WorldSpaceCenter().DistTo( pOwner->GetAbsOrigin() );
	if ( flDistance > HOOK_RANGE || ( bHooked && flDistance < HOOK_CLOSERANGE ) )
	{
		//CancelHook();
		RemoveHook();
		return;
	}

	// Bug #0000501: Doors can be blocked by shit that shouldn't block them.
	if( GetGroundEntity() && ( GetAbsVelocity() == vec3_origin ) )
	{
		if( GetGroundEntity()->GetMoveType() != MOVETYPE_PUSH )
			SetMoveType( MOVETYPE_NONE );
	}

	// This is the actual code for how the hook moves the player
	if ( bHooked )
	{
		Vector vecPushDir = GetAbsOrigin() - pOwner->GetAbsOrigin();
		VectorNormalize( vecPushDir );
		vecPushDir*= HOOK_PULLSPEED;
		//pOwner->VelocityPunch( vecPushDir );
		pOwner->SetAbsVelocity( vecPushDir );
	}

#ifdef GAME_DLL
	if ( m_hRope )
		m_hRope->RecalculateLength();
#endif

	/*
	void BeamDraw(IMaterial *pMaterial, const Vector &vecstart, const Vector &vecend, float widthstart, float widthend, float alphastart, float alphaend, const Vector &colorstart, const Vector &colorend);

			::BeamDraw(pMaterial, GetAbsOrigin() - (vecForward * 2), GetAbsOrigin() - (vecForward * 40),
						8.0f, 1.0f,
						1.0f, 0.1f,
						Vector(1.0f, 1.0f, 1.0f), Vector(0.6f, 0.6f, 0.6f));*/
/*
	BeamEntPoint( IRecipientFilter& filer, float delay,
		int	nStartEntity, const Vector *start, int nEndEntity, const Vector* end, 
		int modelindex, int haloindex, int startframe, int framerate,
		float life, float width, float endWidth, int fadeLength, float amplitude, 
		int r, int g, int b, int a, int speed ) = 0;

	BeamEnts( IRecipientFilter& filer, float delay,
		int	start, int end, int modelindex, int haloindex, int startframe, int framerate,
		float life, float width, float endWidth, int fadeLength, float amplitude, 
		int r, int g, int b, int a, int speed ) = 0;
*/

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

	Remove();
}
//----------------------------------------------------------------------------
// Purpose: Create a new rocket
//----------------------------------------------------------------------------
CFFProjectileHook * CFFProjectileHook::CreateHook(const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwner) 
{
	CFFProjectileHook *pHook;
	
#ifdef PREDICTED_ROCKETS
	if (pentOwner->IsPlayer()) 
	{
		pHook = (CFFProjectileHook *) CREATE_PREDICTED_ENTITY("ff_projectile_hook");
		pHook->SetPlayerSimulated(ToBasePlayer(pentOwner));
	}
	else
#endif
	{
		pHook = (CFFProjectileHook *) CreateEntityByName("ff_projectile_hook");
	}

	UTIL_SetOrigin(pHook, vecOrigin);
	pHook->SetAbsAngles(angAngles);
	pHook->Spawn();
	pHook->SetOwnerEntity(pentOwner);
	pHook->m_iSourceClassname = (pentOwner ? pentOwner->m_iClassname : NULL_STRING);

	Vector vecForward;
	AngleVectors(angAngles, &vecForward);

	// Set the speed and the initial transmitted velocity
	pHook->SetAbsVelocity(vecForward * HOOK_FIRESPEED);

#ifdef GAME_DLL
	pHook->SetupInitialTransmittedVelocity(vecForward * HOOK_FIRESPEED);


	//pHook->m_flDamage = iDamage;
	//pHook->m_DmgRadius = iDamageRadius;

	// Remove the old hook
	CFFPlayer *pPlayer = ToFFPlayer( pentOwner );
	if ( pPlayer )
	{
		if ( pPlayer->GetHook() )
		{

			pPlayer->GetHook()->Remove();
		}
	}

#endif
	pHook->bHooked = false;
	//pRocket->EmitSound("rocket.fly");
	// this is being swapped over to the client -mirv

	CPASFilter filter( vecOrigin );
	//te->ShowLine( filter, 0.0, &GetAbsOrigin(), &( pOwner->GetAbsOrigin() ) );

	// AfterShock: TODO: This doesnt work. I want to draw a line between hook + owner!
	/*
	te->BeamEntPoint ( filter, 0.0, 
		pHook->entindex(), &vecOrigin, pentOwner->entindex(), &( pentOwner->GetAbsOrigin() ),
		0, 0, 1, 1, 
		5.0f, 10.0f,  10.0f, 10, 10.0f,
		255, 200, 150, 150, 10 );
*/
#ifdef GAME_DLL
	pHook->m_hRope = CRopeKeyframe::Create( pHook, pentOwner, 1, 0, 2, ROPE_MATERIAL );
 
	if ( pHook->m_hRope )
	{
		pHook->m_hRope->m_Width = 2;
		pHook->m_hRope->m_nSegments = ROPE_MAX_SEGMENTS / 2;
		pHook->m_hRope->EnableWind( false );
	//	m_hRope->EnableCollision(); // Collision looks worse than no collision
		pHook->m_hRope->SetupHangDistance( 2.2f );
	}
#endif


	/*
	void BeamDraw(IMaterial *pMaterial, const Vector &vecstart, const Vector &vecend, float widthstart, float widthend, float alphastart, float alphaend, const Vector &colorstart, const Vector &colorend);

			::BeamDraw(pMaterial, GetAbsOrigin() - (vecForward * 2), GetAbsOrigin() - (vecForward * 40),
						8.0f, 1.0f,
						1.0f, 0.1f,
						Vector(1.0f, 1.0f, 1.0f), Vector(0.6f, 0.6f, 0.6f));*/
/*
	BeamEntPoint( IRecipientFilter& filer, float delay,
		int	nStartEntity, const Vector *start, int nEndEntity, const Vector* end, 
		int modelindex, int haloindex, int startframe, int framerate,
		float life, float width, float endWidth, int fadeLength, float amplitude, 
		int r, int g, int b, int a, int speed ) = 0;

	BeamEnts( IRecipientFilter& filer, float delay,
		int	start, int end, int modelindex, int haloindex, int startframe, int framerate,
		float life, float width, float endWidth, int fadeLength, float amplitude, 
		int r, int g, int b, int a, int speed ) = 0;
*/

	return pHook; 
}

