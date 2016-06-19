//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#include "gamemovement.h"
#include "ff_gamerules.h"
#include "ff_shareddefs.h"
#include "in_buttons.h"
#include "movevars_shared.h"
#include "ff_mapguide.h"

#define	STOP_EPSILON		0.1
#define	MAX_CLIP_PLANES		5

#ifdef CLIENT_DLL
	#include "c_ff_player.h"
#else
	#include "ff_player.h"
#endif

// When changing jump height, recalculate the FF_MUL_CONSTANT!!!
#define FF_JUMP_HEIGHT 27.5f // Modified by Mulch 10/20/2005 so we could jump on 63 but not 64 unit high stuff
#define FF_MUL_CONSTANT 209.76177f //sqrt(2.0f * 800.0f * FF_JUMP_HEIGHT);
#define EXTINGUISH_FIRE_SPEED 500.0f
//static ConVar FF_JUMP_HEIGHT( "ffdev_jump_height", "27.5", FCVAR_FF_FFDEV );

//static ConVar sv_trimpmultiplier("sv_trimpmultiplier", "1.4", FCVAR_REPLICATED | FCVAR_CHEAT);
#define SV_TRIMPMULTIPLIER 1.4f
//static ConVar sv_trimpdownmultiplier("sv_trimpdownmultiplier", "1.2", FCVAR_REPLICATED | FCVAR_CHEAT);
#define SV_TRIMPDOWNMULTIPLIER 1.2f
//static ConVar sv_trimpmax("sv_trimpmax", "5000", FCVAR_REPLICATED);
#define SV_TRIMPMAX 5000.0f
//static ConVar sv_trimptriggerspeed("sv_trimptriggerspeed", "550", FCVAR_REPLICATED | FCVAR_CHEAT);
#define SV_TRIMPTRIGGERSPEED 550.0f
//static ConVar sv_trimptriggerspeeddown("sv_trimptriggerspeeddown", "50", FCVAR_REPLICATED | FCVAR_CHEAT);
#define SV_TRIMPTRIGGERSPEEDDOWN 50.0f

// from ff_player_shared.cpp
//extern ConVar ffdev_overpressure_slide_duration;
//extern ConVar ffdev_overpressure_slide_friction;
//extern ConVar ffdev_overpressure_slide_airaccel;
//extern ConVar ffdev_overpressure_slide_accel;
//extern ConVar ffdev_overpressure_slide_wearsoff;
//extern ConVar ffdev_overpressure_slide_wearsoff_bias;
#define OVERPRESSURE_SLIDE_DURATION 1.0f
#define OVERPRESSURE_SLIDE_FRICTION 0.0f
#define OVERPRESSURE_SLIDE_AIRACCEL 1.0f
#define OVERPRESSURE_SLIDE_ACCEL 1.0f
#define OVERPRESSURE_SLIDE_WEARSOFF 1.0f
#define OVERPRESSURE_SLIDE_WEARSOFF_BIAS 0.2f

extern bool g_bMovementOptimizations;

#ifdef CLIENT_DLL
ConVar cl_bunnyhop_disablepogojump( "cl_jumpqueue", "0.0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Enables jump queue (have to let go and press jump in between concurrent jumps) if set to 1" );
#endif

class CBasePlayer;

//=============================================================================
// Overrides some CGameMovement stuff
//=============================================================================
class CFFGameMovement : public CGameMovement
{
	DECLARE_CLASS(CFFGameMovement, CGameMovement);

public:
	// CGameMovement
	virtual bool CheckJumpButton();
	virtual bool CanAccelerate();
	virtual void CheckVelocity( void );
	virtual void CategorizePosition( void );

	// CFFGameMovement
	virtual void FullBuildMove( void );	
	virtual void WalkMove();
	virtual void AirMove();
	virtual void Friction();
	bool IsRampSliding( CFFPlayer *pPlayer );

	CFFGameMovement() {};
};

//static ConVar bhop_cap_soft("ffdev_bhop_cap_soft", "1.4", FCVAR_FF_FFDEV_REPLICATED); // bhop_cap_soft.GetFloat()
#define BHOP_CAP_SOFT 1.4f // also defined in ff_hud_speedometer - change it there too! 
//static ConVar bhop_cap_mid("ffdev_bhop_cap_mid", "1.55", FCVAR_FF_FFDEV_REPLICATED); // bhop_cap_mid.GetFloat()
#define BHOP_CAP_MID 1.55f //bhop_cap_mid.GetFloat() // also defined in ff_hud_speedometer - change it there too! // This slows the player more if they go above this cap
//static ConVar bhop_cap_hard("ffdev_bhop_cap_hard", "1.71", FCVAR_FF_FFDEV_REPLICATED); // bhop_cap_hard.GetFloat()
#define BHOP_CAP_HARD 1.71f // also defined in ff_hud_speedometer - change it there too!
//static ConVar bhop_pcfactor("ffdev_bhop_pcfactor", "0.65", FCVAR_FF_FFDEV_REPLICATED); // bhop_pcfactor.GetFloat()
#define BHOP_PCFACTOR 0.65f //bhop_pcfactor.GetFloat()
//static ConVar bhop_pcfactor_mid("ffdev_bhop_pcfactor_mid", "0.4", FCVAR_FF_FFDEV_REPLICATED); // bhop_pcfactor_mid.GetFloat()
#define BHOP_PCFACTOR_MID 0.4f //bhop_pcfactor_mid.GetFloat()

//-----------------------------------------------------------------------------
// Purpose: Provides TFC jump heights, trimping, doublejumps
//-----------------------------------------------------------------------------
bool CFFGameMovement::CheckJumpButton(void)
{
	CFFPlayer *ffplayer = ToFFPlayer(player);
	if(ffplayer == NULL)
	{
		return BaseClass::CheckJumpButton();
	}

	if (player->pl.deadflag)
	{
		mv->m_nOldButtons |= IN_JUMP ;	// don't jump again until released
		return false;
	}

	// See if we are waterjumping.  If so, decrement count and return.
	if (player->m_flWaterJumpTime)
	{
		player->m_flWaterJumpTime -= gpGlobals->frametime;
		if (player->m_flWaterJumpTime < 0)
			player->m_flWaterJumpTime = 0;

		return false;
	}

	// If we are in the water most of the way...	
	if ( player->GetWaterLevel() >= 2 )
	{
		// swimming, not jumping
		SetGroundEntity( (CBaseEntity *)NULL );

		if(player->GetWaterType() == CONTENTS_WATER)    // We move up a certain amount
			mv->m_vecVelocity[2] = 100;
		else if (player->GetWaterType() == CONTENTS_SLIME)
			mv->m_vecVelocity[2] = 80;

		// play swiming sound
		if ( player->m_flSwimSoundTime <= 0 )
		{
			// Don't play sound again for 1 second
			player->m_flSwimSoundTime = 1000;
			PlaySwimSound();
		}

		return false;
	}

	// No more effect
	if (player->GetGroundEntity() == NULL)
	{
		//mv->m_nOldButtons |= IN_JUMP;

		// hack to enable Q3-style jumping
		// To jump as soon as you hit the ground, the jump button must be pressed and held in the downward part
		// of your jump only -- if you start holding it during the upward part of your previous jump, it won't work.
		// This hack just clears the current state of the jump button as long as jump was not held at the time that
		// holding the jump button became "legal" again.
		// Thus, with this hack, mv->m_nOldButtons & IN_JUMP will never be set in the air unless it has been set
		// constantly ever since the player initially jumped or unless the player has re-pressed the jump button in the
		// upwards phase of their jump.
		if(!(mv->m_nOldButtons & IN_JUMP))
		{
			/*
			if(mv->m_vecVelocity[2] < 0.0f)
			{
				mv->m_nButtons &= ~IN_JUMP;
			}
			*/

			// 06/13/2005 - Mulchman (as per Defrag)
			// Clear the jump regardless of our
			// vertical velocity - so we can
			// hit jump on the upward or downward
			// part of the jump
			mv->m_nButtons &= ~IN_JUMP;
		}
//		else
//		{
//			if(ffplayer->m_iLocalSkiState == 0)
//			{
//				ffplayer->StartSkiing();
//			}
//		}
		return false;		// in air, so no effect
	}

	// caes: do pogo stick!
	//if ( mv->m_nOldButtons & IN_JUMP )
	//{
	//	return false;		// don't pogo stick
	//}

	
#ifdef CLIENT_DLL
	if ( mv->m_nOldButtons & IN_JUMP && cl_bunnyhop_disablepogojump.GetBool() )
	{
		return false;		// don't pogo stick
	}
#else
	if ( mv->m_nOldButtons & IN_JUMP && (Q_atoi( engine->GetClientConVarValue( player->entindex(), "cl_jumpqueue" ) ) ) )
	{
		//DevMsg("Jumpqueue is %i\n",(Q_atoi( engine->GetClientConVarValue( player->entindex(), "cl_jumpqueue" ) ) ) );
		return false;		// don't pogo stick
	}
#endif


	// Don't allow jumping when the player is in a stasis field.
	if ( player->m_Local.m_bSlowMovement )
		return false;

	// Cannot jump will in the unduck transition.
	//if ( player->m_Local.m_bDucking && (  player->GetFlags() & FL_DUCKING ) )
	//	return false;

	// Still updating the eye position.
	//if ( player->m_Local.m_flDuckJumpTime > 0.0f )
	//	return false;

	// UNDONE: Now that we've allowed jumps to take place while unducking, we have to
	// immediately finish the unduck. This pretty much follows TFC behaviour
	// now.
	//if (player->m_Local.m_bDucking && (player->GetFlags() & FL_DUCKING))
	//{
	//	FinishUnDuck();
	//}

	// In the air now.
	SetGroundEntity( (CBaseEntity *)NULL );

	// Mirv: Play proper jump sounds
	//player->PlayStepSound( mv->m_vecAbsOrigin, player->m_pSurfaceData, 1.0, true );
	ffplayer->PlayJumpSound(mv->m_vecAbsOrigin, player->m_pSurfaceData, 1.0);

	// Mirv: This fixes the jump animation
	//MoveHelper()->PlayerSetAnimation( PLAYER_JUMP );
	ffplayer->DoAnimationEvent(PLAYERANIMEVENT_JUMP);

	float fGroundFactor = 1.0f;
	if (player->m_pSurfaceData)
	{
		fGroundFactor = player->m_pSurfaceData->game.jumpFactor; 
	}

	// This following dynamic cap is documented here:
	//		http://www.madabouthats.org/code-tf2/viewtopic.php?t=2360

	const float cap_soft = BHOP_CAP_SOFT * mv->m_flMaxSpeed;
	const float cap_mid = BHOP_CAP_MID * mv->m_flMaxSpeed;
	const float cap_hard = BHOP_CAP_HARD * mv->m_flMaxSpeed;
	float pcfactor = BHOP_PCFACTOR;
	float speed = FastSqrt(mv->m_vecVelocity[0] * mv->m_vecVelocity[0] + mv->m_vecVelocity[1] * mv->m_vecVelocity[1]);

	if (speed > cap_soft) // apply soft cap
	{
		if (speed > cap_mid) // Slow down even more if above mid cap
			pcfactor = BHOP_PCFACTOR_MID;

		float applied_cap = (speed - cap_soft) * pcfactor + cap_soft;
		float multi = applied_cap / speed;

		mv->m_vecVelocity[0] *= multi;
		mv->m_vecVelocity[1] *= multi;

		Assert(multi <= 1.0f);
	}

	// --> Mirv: Trimp code v2.0!
	//float fMul = FF_MUL_CONSTANT;
	//float fMul = 268.3281573;
	// This is the base jump height before adding trimp/doublejump height
	float fMul = 268.6261342; //sqrt(2.0f * 800.0f * 45.1f);

	bool bTrimped = false;
	bool bDownTrimped = false;

	trace_t pm;

	// Adjusted for TFC bboxes.
	// TODO: Look at this later
	Vector vecStart = mv->m_vecAbsOrigin; // + Vector(0, 0, GetPlayerMins()[2] + 1.0f);
	Vector vecStop = vecStart - Vector(0, 0, 60.0f);
	
	TracePlayerBBox(vecStart, vecStop, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm);

	// Found the floor
	if(pm.fraction != 1.0f)
	{
		// Take the lateral velocity
		Vector vecVelocity = mv->m_vecVelocity * Vector(1.0f, 1.0f, 0.0f);
		float flHorizontalSpeed = vecVelocity.Length();

		// If building, don't let them trimp!
		if( ffplayer->IsStaticBuilding() )
			flHorizontalSpeed = 0.0f;

		if (flHorizontalSpeed > 0)
			vecVelocity /= flHorizontalSpeed;

        float flDotProduct = DotProduct(vecVelocity, pm.plane.normal);
		float flRampSlideDotProduct = DotProduct(mv->m_vecVelocity, pm.plane.normal);

		// They have to be at least moving a bit
		if (flHorizontalSpeed > SV_TRIMPTRIGGERSPEED)
		{
			// Don't do anything for flat ground or downwardly sloping (relative to motion)
			// Changed to 0.15f to make it a bit less trimpy on only slightly uneven ground
			//if (flDotProduct < -0.15f || flDotProduct > 0.15f)
			if (flDotProduct < -0.15f)
			{
				// This is one way to do it
				fMul += -flDotProduct * flHorizontalSpeed * SV_TRIMPMULTIPLIER; //0.6f;
				DevMsg("[S] Trimp %f! Dotproduct:%f. Horizontal speed:%f. Rampslide dot.p.:%f\n", fMul, flDotProduct, flHorizontalSpeed, flRampSlideDotProduct);

				bTrimped = true;

				if (SV_TRIMPMULTIPLIER > 0)
				{
					mv->m_vecVelocity[0] *= (1.0f / SV_TRIMPMULTIPLIER );
					mv->m_vecVelocity[1] *= (1.0f / SV_TRIMPMULTIPLIER );
				}
				// This is another that'll give some different height results
				// UNDONE: Reverted back to the original way for now
				//Vector reflect = mv->m_vecVelocity + (-2.0f * pm.plane.normal * DotProduct(mv->m_vecVelocity, pm.plane.normal));
				//float flSpeedAmount = clamp((flLength - 400.0f) / 800.0f, 0, 1.0f);
				//fMul += reflect.z * flSpeedAmount;
			}
		}
		// trigger downwards trimp at any speed
		if (flHorizontalSpeed > SV_TRIMPTRIGGERSPEEDDOWN)
		{
			if (flDotProduct > 0.15f) // AfterShock: travelling downwards onto a downward ramp - give boost horizontally
			{
				// This is one way to do it
				//mv->m_vecVelocity[1] += -flDotProduct * mv->m_vecVelocity[2] * sv_trimpmultiplier.GetFloat(); //0.6f;
				//mv->m_vecVelocity[0] += -flDotProduct * mv->m_vecVelocity[2] * sv_trimpmultiplier.GetFloat(); //0.6f;
				//mv->m_vecVelocity[1] += -flDotProduct * fMul * sv_trimpmultiplier.GetFloat(); //0.6f;
				//mv->m_vecVelocity[0] += -flDotProduct * fMul * sv_trimpmultiplier.GetFloat(); //0.6f;
				mv->m_vecVelocity[1] *= SV_TRIMPDOWNMULTIPLIER; //0.6f;
				mv->m_vecVelocity[0] *= SV_TRIMPDOWNMULTIPLIER; //0.6f;
				DevMsg("[S] Down Trimp %f! Dotproduct:%f, upwards vel:%f, vel 1:%f, vel 0:%f\n", fMul, flDotProduct,mv->m_vecVelocity[2],mv->m_vecVelocity[1],mv->m_vecVelocity[0]);

				bDownTrimped = true;

				if (SV_TRIMPMULTIPLIER > 0)
				{
					fMul *= (1.0f / SV_TRIMPDOWNMULTIPLIER );
				}
				// This is another that'll give some different height results
				// UNDONE: Reverted back to the original way for now
				//Vector reflect = mv->m_vecVelocity + (-2.0f * pm.plane.normal * DotProduct(mv->m_vecVelocity, pm.plane.normal));
				//float flSpeedAmount = clamp((flLength - 400.0f) / 800.0f, 0, 1.0f);
				//fMul += reflect.z * flSpeedAmount;
			}
		}
	}
	// <-- Mirv: Trimp code v2.0!

	if (!bTrimped)
	{
		speed = FastSqrt(mv->m_vecVelocity[0] * mv->m_vecVelocity[0] + mv->m_vecVelocity[1] * mv->m_vecVelocity[1]);

		// apply skim cap
		if (speed > cap_hard )
		{
			float applied_cap = BHOP_CAP_HARD * mv->m_flMaxSpeed;; 
			float multi = applied_cap / speed;

			mv->m_vecVelocity[0] *= multi;
			mv->m_vecVelocity[1] *= multi;

			Assert(multi <= 1.0f);
		}
	}

	//// Acclerate upward
	//// If we are ducking...
	//float startz = mv->m_vecVelocity[2];
	//if ((player->m_Local.m_bDucking ) || (player->GetFlags() & FL_DUCKING))
	//{
	//	// d = 0.5 * g * t^2		- distance traveled with linear accel
	//	// t = sqrt(2.0 * 45 / g)	- how long to fall 45 units
	//	// v = g * t				- velocity at the end (just invert it to jump up that high)
	//	// v = g * sqrt(2.0 * 45 / g )
	//	// v^2 = g * g * 2.0 * 45 / g
	//	// v = sqrt( g * 2.0 * 45 )
	//	mv->m_vecVelocity[2] = fGroundFactor * fMul;  // 2 * gravity * height
	//}
	//else
	//{
	//	//mv->m_vecVelocity[2] += flGroundFactor * flMul;  // 2 * gravity * height
	//	mv->m_vecVelocity[2] = fGroundFactor * fMul;  // 2 * gravity * height
	//}

	// Double jump - but don't allow double jumps while building, please!
	if( ffplayer->m_bCanDoubleJump && !ffplayer->IsStaticBuilding() )
	{
		float flElapsed = ffplayer->m_flNextJumpTimeForDouble - gpGlobals->curtime;

		if (flElapsed > 0 && flElapsed < 0.4f)
		{
			// AfterShock: Add a set amount for a double jump (dont multiply)
			fMul += 190.0f;

#ifdef GAME_DLL
			DevMsg("[S] Double jump %f!\n", fMul);
#else
			//DevMsg("[C] Double jump %f!\n", fMul);
#endif

			ffplayer->m_bCanDoubleJump = false;
		}

		ffplayer->m_flNextJumpTimeForDouble = gpGlobals->curtime + 0.5f;
	}

	// --> Mirv: Add on new velocity
	if( mv->m_vecVelocity[2] < 0 )
		mv->m_vecVelocity[2] = 0;
//	if (fMul > 500)
//		DevMsg("[S] vert velocity_old %f!\n", mv->m_vecVelocity[2]);
		
	// This is commented out because trimp code was getting called twice and doubling the effect sometimes.
	//mv->m_vecVelocity[2] += fMul;
	mv->m_vecVelocity[2] = fMul;

//	if (fMul > 500)
//		DevMsg("[S] vert velocity %f!\n", mv->m_vecVelocity[2]);
				
	mv->m_vecVelocity[2] = min(mv->m_vecVelocity[2], SV_TRIMPMAX);
//	if (fMul > 500)
//		DevMsg("[S] vert velocity2 %f!\n", mv->m_vecVelocity[2]);
	
	// <-- Mirv: Add on new velocity

	FinishGravity();

	// Flag that we jumped.
	mv->m_nOldButtons |= IN_JUMP;	// don't jump again until released
	return true;
}

void CFFGameMovement::CategorizePosition()
{
	BaseClass::CategorizePosition();

#ifdef GAME_DLL
	CFFPlayer *pFFPlayer = ToFFPlayer(player);
	if (pFFPlayer->m_flMancannonTime > 0.0f && player->GetGroundEntity() != NULL || player->GetWaterLevel() > WL_Feet)
	{
		// reset jump pad time so that it stops conc speed limiting
		// once you're firmly on the ground or in water
		pFFPlayer->m_flMancannonTime = 0.0f;
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Movement while building in Fortress Forever
//-----------------------------------------------------------------------------
void CFFGameMovement::FullBuildMove( void )
{
	CFFPlayer *pPlayer = ToFFPlayer( player );
	if( !pPlayer )
		return;

	// Don't care if dead or not building...
	if( !pPlayer->IsStaticBuilding() || !pPlayer->IsAlive() )
		return;
	
	// Don't care if under water...
	if( pPlayer->GetWaterLevel() > WL_Feet )
		return;

	// Finally, allow jumping

	StartGravity();

	// Was jump button pressed?
	if( mv->m_nButtons & IN_JUMP )
	{
		CheckJumpButton();
	}
	else
	{
		mv->m_nOldButtons &= ~IN_JUMP;
	}

	// Reset these so we stay in place
	mv->m_flSideMove = 0.0f;
	mv->m_flForwardMove = 0.0f;
	mv->m_vecVelocity[ 0 ] = 0.0f;
	mv->m_vecVelocity[ 1 ] = 0.0f;

	CheckVelocity();

	if( pPlayer->GetGroundEntity() != NULL )
	{
		WalkMove();
	}
	else
	{
		AirMove();  // Take into account movement when in air.
	}

	CategorizePosition();

	FinishGravity();

	// Does nothing, but makes me feel happy.
	BaseClass::FullBuildMove();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFGameMovement::WalkMove( void )
{
	Vector wishvel;
	float spd;
	float fmove, smove;
	Vector wishdir;
	float wishspeed;

	Vector dest;
	trace_t pm;
	Vector forward, right, up;
	
	CFFPlayer *pFFPlayer = ToFFPlayer( player );
	if( !pFFPlayer )
		return;

	AngleVectors (mv->m_vecViewAngles, &forward, &right, &up);  // Determine movement angles

	CHandle< CBaseEntity > oldground;
	oldground = player->GetGroundEntity();
	
	// Copy movement amounts
	fmove = mv->m_flForwardMove;
	smove = mv->m_flSideMove;
	
	// Zero out z components of movement vectors
	if ( g_bMovementOptimizations )
	{
		if ( forward[2] != 0 )
		{
			forward[2] = 0;
			VectorNormalize( forward );
		}

		if ( right[2] != 0 )
		{
			right[2] = 0;
			VectorNormalize( right );
		}
	}
	else
	{
		forward[2] = 0;
		right[2]   = 0;
		
		VectorNormalize (forward);  // Normalize remainder of vectors.
		VectorNormalize (right);    // 
	}

	for (int i=0 ; i<2 ; i++)       // Determine x and y parts of velocity
		wishvel[i] = forward[i]*fmove + right[i]*smove;
	
	wishvel[2] = 0;             // Zero out z part of velocity

	VectorCopy (wishvel, wishdir);   // Determine maginitude of speed of move
	wishspeed = VectorNormalize(wishdir);

	//
	// Clamp to server defined max speed
	//
	if ((wishspeed != 0.0f) && (wishspeed > mv->m_flMaxSpeed))
	{
		VectorScale (wishvel, mv->m_flMaxSpeed/wishspeed, wishvel);
		wishspeed = mv->m_flMaxSpeed;
	}

	// Set pmove velocity
	mv->m_vecVelocity[2] = 0;
	if (pFFPlayer->IsSliding())
	{
		float accel = OVERPRESSURE_SLIDE_ACCEL;
		if (OVERPRESSURE_SLIDE_WEARSOFF)
		{
			float dt = OVERPRESSURE_SLIDE_DURATION - ( pFFPlayer->m_flSlidingTime - gpGlobals->curtime );
			float percent = clamp(dt / OVERPRESSURE_SLIDE_DURATION, 0.0f, 1.0f);
			percent = Bias( percent, OVERPRESSURE_SLIDE_WEARSOFF_BIAS );
			accel = Lerp( percent, min(OVERPRESSURE_SLIDE_ACCEL,sv_accelerate.GetFloat()), max(OVERPRESSURE_SLIDE_ACCEL,sv_accelerate.GetFloat()) );
		}

		Accelerate ( wishdir, wishspeed, accel );
	}
	else
	{
		Accelerate( wishdir, wishspeed, sv_accelerate.GetFloat() );
	}
	mv->m_vecVelocity[2] = 0;

	// Add in any base velocity to the current velocity.
	VectorAdd (mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );

	spd = VectorLength( mv->m_vecVelocity );

	if ( spd < 1.0f )
	{
		mv->m_vecVelocity.Init();
		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
		VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
		return;
	}

	// first try just moving to the destination	
	dest[0] = mv->m_vecAbsOrigin[0] + mv->m_vecVelocity[0]*gpGlobals->frametime;
	dest[1] = mv->m_vecAbsOrigin[1] + mv->m_vecVelocity[1]*gpGlobals->frametime;	
	dest[2] = mv->m_vecAbsOrigin[2];

	// first try moving directly to the next spot
	TracePlayerBBox( mv->m_vecAbsOrigin, dest, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, pm );

	// If we made it all the way, then copy trace end as new player position.
	mv->m_outWishVel += wishdir * wishspeed;

	if ( pm.fraction == 1 )
	{
		VectorCopy( pm.endpos, mv->m_vecAbsOrigin );
		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
		VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );

		StayOnGround();
		return;
	}

	// Don't walk up stairs if not on ground.
	if ( oldground == NULL && player->GetWaterLevel()  == 0 )
	{
		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
		VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
		return;
	}

	// If we are jumping out of water, don't do anything more.
	if ( player->m_flWaterJumpTime )         
	{
		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
		VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
		return;
	}

	StepMove( dest, pm );

	// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
	VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );

	StayOnGround();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFGameMovement::AirMove( void )
{
	int			i;
	Vector		wishvel;
	float		fmove, smove;
	Vector		wishdir;
	float		wishspeed;
	Vector forward, right, up;

	CFFPlayer *pFFPlayer = ToFFPlayer( player );
	if( !pFFPlayer )
		return;

	AngleVectors (mv->m_vecViewAngles, &forward, &right, &up);  // Determine movement angles
	
	// Copy movement amounts
	fmove = mv->m_flForwardMove;
	smove = mv->m_flSideMove;
	
	// Zero out z components of movement vectors
	forward[2] = 0;
	right[2]   = 0;
	VectorNormalize(forward);  // Normalize remainder of vectors
	VectorNormalize(right);    // 

	for (i=0 ; i<2 ; i++)       // Determine x and y parts of velocity
		wishvel[i] = forward[i]*fmove + right[i]*smove;
	wishvel[2] = 0;             // Zero out z part of velocity

	VectorCopy (wishvel, wishdir);   // Determine maginitude of speed of move
	wishspeed = VectorNormalize(wishdir);

	//
	// clamp to server defined max speed
	//
	if ( wishspeed != 0 && (wishspeed > mv->m_flMaxSpeed))
	{
		VectorScale (wishvel, mv->m_flMaxSpeed/wishspeed, wishvel);
		wishspeed = mv->m_flMaxSpeed;
	}
	
	if (pFFPlayer->IsSliding())
	{
		float accel = OVERPRESSURE_SLIDE_AIRACCEL;
		if (OVERPRESSURE_SLIDE_WEARSOFF)
		{
			float dt = OVERPRESSURE_SLIDE_DURATION - ( pFFPlayer->m_flSlidingTime - gpGlobals->curtime );
			float percent = clamp(dt / OVERPRESSURE_SLIDE_DURATION, 0.0f, 1.0f);
			percent = Bias( percent, OVERPRESSURE_SLIDE_WEARSOFF_BIAS );
			accel = Lerp( percent, min(OVERPRESSURE_SLIDE_AIRACCEL,sv_airaccelerate.GetFloat()), max(OVERPRESSURE_SLIDE_AIRACCEL,sv_airaccelerate.GetFloat()) );
		}

		AirAccelerate ( wishdir, wishspeed, accel );
	}
	else
	{
		AirAccelerate( wishdir, wishspeed, sv_airaccelerate.GetFloat() );
	}

	// Add in any base velocity to the current velocity.
	VectorAdd(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );

	TryPlayerMove();

	// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
	VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFGameMovement::Friction( void )
{
	float	speed, newspeed, control;
	float	friction;
	float	drop;
	Vector newvel;

	CFFPlayer *pFFPlayer = ToFFPlayer( player );
	if( !pFFPlayer )
		return;
	
	// If we are in water jump cycle, don't apply friction
	if (player->m_flWaterJumpTime)
		return;

	// Calculate speed
	speed = VectorLength( mv->m_vecVelocity );
	
	// If too slow, return
	if (speed < 0.1f)
	{
		return;
	}

	drop = 0;

	// apply ground friction
	if (player->GetGroundEntity() != NULL)  // On an entity that is the ground
	{
		Vector start, stop;
		trace_t pm;

		//
		// NOTE: added a "1.0f" to the player minimum (bbox) value so that the 
		//       trace starts just inside of the bounding box, this make sure
		//       that we don't get any collision epsilon (on surface) errors.
		//		 The significance of the 16 below is this is how many units out front we are checking
		//		 to see if the player box would fall.  The 49 is the number of units down that is required
		//		 to be considered a fall.  49 is derived from 1 (added 1 from above) + 48 the max fall 
		//		 distance a player can fall and still jump back up.
		//
		//		 UNDONE: In some cases there are still problems here.  Specifically, no collision check is
		//		 done so 16 units in front of the player could be inside a volume or past a collision point.
		start[0] = stop[0] = mv->m_vecAbsOrigin[0] + (mv->m_vecVelocity[0]/speed)*16;
		start[1] = stop[1] = mv->m_vecAbsOrigin[1] + (mv->m_vecVelocity[1]/speed)*16;
		start[2] = mv->m_vecAbsOrigin[2] + ( GetPlayerMins()[2] + 1.0f );
		stop[2] = start[2] - 49;

		if ( g_bMovementOptimizations )
		{
			// We don't actually need this trace.
		}
		else
		{
			TracePlayerBBox( start, stop, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, pm ); 
		}

		if (pFFPlayer->IsSliding())
		{
			friction = OVERPRESSURE_SLIDE_FRICTION;
			if (OVERPRESSURE_SLIDE_WEARSOFF)
			{
				float dt = OVERPRESSURE_SLIDE_DURATION - ( pFFPlayer->m_flSlidingTime - gpGlobals->curtime );
				float percent = clamp(dt / OVERPRESSURE_SLIDE_DURATION, 0.0f, 1.0f);
				percent = Bias( percent, OVERPRESSURE_SLIDE_WEARSOFF_BIAS );
				friction = Lerp( percent, min(OVERPRESSURE_SLIDE_FRICTION,sv_friction.GetFloat()), max(OVERPRESSURE_SLIDE_FRICTION,sv_friction.GetFloat()) );
			}
		}
		else
			friction = sv_friction.GetFloat();

		// Grab friction value.
		friction *= pFFPlayer->GetFriction() * /*player->m_surfaceFriction*/ 1.0f;	// |-- Mirv: More TFC Feeling (tm) friction

		// Bleed off some speed, but if we have less than the bleed
		//  threshhold, bleed the theshold amount.
#ifdef _XBOX 
		if( player->m_Local.m_bDucked )
		{
			control = (speed < sv_stopspeed.GetFloat()) ? sv_stopspeed.GetFloat() : speed;
		}
		else
		{
			control = (speed < sv_stopspeed.GetFloat()) ? (sv_stopspeed.GetFloat() * 2.0f) : speed;
		}
#else
		control = (speed < sv_stopspeed.GetFloat()) ?
			sv_stopspeed.GetFloat() : speed;
#endif //_XBOX

		// Add the amount to the drop amount.
		drop += control*friction*gpGlobals->frametime;
	}

	// scale the velocity
	newspeed = speed - drop;
	if (newspeed < 0)
		newspeed = 0;

	// Determine proportion of old speed we are using.
	newspeed /= speed;

	// Adjust velocity according to proportion.
	newvel[0] = mv->m_vecVelocity[0] * newspeed;
	newvel[1] = mv->m_vecVelocity[1] * newspeed;
	newvel[2] = mv->m_vecVelocity[2] * newspeed;

	VectorCopy( newvel, mv->m_vecVelocity );
 	mv->m_outWishVel -= (1.f-newspeed) * mv->m_vecVelocity;
}

//-----------------------------------------------------------------------------
// Purpose: Allow spectators to move
//-----------------------------------------------------------------------------
bool CFFGameMovement::CanAccelerate()
{
	// Observers can accelerate
	if (player->IsObserver())
		return true;

	// Dead players don't accelerate.
	if (player->pl.deadflag)
		return false;

	// If waterjumping, don't accelerate
	if (player->m_flWaterJumpTime)
		return false;

	return true;
}

bool CFFGameMovement::IsRampSliding( CFFPlayer *pPlayer )
{
	if (pPlayer->GetGroundEntity() == NULL)  // Rampsliding occurs when normal ground detection fails
	{
		// Take the lateral velocity
		Vector vecVelocity = mv->m_vecVelocity * Vector(1.0f, 1.0f, 0.0f);
		float flHorizontalSpeed = vecVelocity.Length();

		if (flHorizontalSpeed > SV_TRIMPTRIGGERSPEED)
		{
			trace_t pm;

			Vector vecStart = mv->m_vecAbsOrigin;
			Vector vecStop = vecStart - Vector(0, 0, 0.1f);
			
			TracePlayerBBox(vecStart, vecStop, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm); // but actually you are on the ground

			// Found the floor
			if(pm.fraction != 1.0f)
			{
				if (flHorizontalSpeed > 0)
					vecVelocity /= flHorizontalSpeed;

				float flDotProduct = DotProduct(vecVelocity, pm.plane.normal);
				if (flDotProduct < -0.15f) // On an upwards ramp
				{
					return true;
				}
			}
		}
		
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Check player velocity & clamp if cloaked
//-----------------------------------------------------------------------------
void CFFGameMovement::CheckVelocity( void )
{
	// Let regular movement clamp us within movement limits
	BaseClass::CheckVelocity();

	// Clamp further only if we're cloaked
	
	CFFPlayer *pPlayer = ToFFPlayer( player );
	if( !pPlayer )
		return;

	pPlayer->SetRampsliding(IsRampSliding(pPlayer));
#ifdef GAME_DLL
	if ( pPlayer->GetMovementSpeed() > EXTINGUISH_FIRE_SPEED)
	{
		pPlayer->Extinguish();
	}
#endif
	if( !pPlayer->IsCloaked() )
		return;

	float flMaxCloakSpeed = SPY_MAXCLOAKSPEED;

	// Going over speed limit, need to clamp so we don't uncloak
	if( mv->m_vecVelocity.LengthSqr() > ( flMaxCloakSpeed * flMaxCloakSpeed ) )
	{
		mv->m_vecVelocity.x *= 0.5f;
		mv->m_vecVelocity.y *= 0.5f;
	}
}
// Expose our interface.
static CFFGameMovement g_GameMovement;
IGameMovement *g_pGameMovement = ( IGameMovement * )&g_GameMovement;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CFFGameMovement, IGameMovement,INTERFACENAME_GAMEMOVEMENT, g_GameMovement );
