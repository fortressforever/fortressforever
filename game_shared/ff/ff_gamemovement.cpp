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

//static ConVar FF_JUMP_HEIGHT( "ffdev_jump_height", "27.5" );

static ConVar sv_trimpmultiplier("sv_trimpmultiplier", "0.3", FCVAR_REPLICATED);

class CBasePlayer;

class CFFGameMovement : public CGameMovement
{
public:
	virtual bool CheckJumpButton(void);
//	virtual void FullWalkMove(void);
//	virtual void WalkMove(void);
//	virtual void SkiMove(void);
//	virtual void AirMove(void);
//	virtual void StepMove(Vector &vecDestination, trace_t &trace);
//	virtual int TryPlayerMove(Vector *pFirstDest = NULL, trace_t *pFirstTrace = NULL);
//	virtual void Friction(void);
//	virtual void StartGravity(void);
//	virtual void FinishGravity(void);
	virtual void CheckWaterJump( void );

	virtual void FullNoClipMove( float factor, float maxacceleration ); // |-- Mirv: Map guides

	DECLARE_CLASS( CFFGameMovement, CGameMovement );

	CFFGameMovement();
};

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

	if ( mv->m_nOldButtons & IN_JUMP )
	{
		return false;		// don't pogo stick
	}

	// Don't allow jumping when the player is in a stasis field.
	if ( player->m_Local.m_bSlowMovement )
		return false;

	// Cannot jump will in the unduck transition.
	//if ( player->m_Local.m_bDucking && (  player->GetFlags() & FL_DUCKING ) )
	//	return false;

	// Still updating the eye position.
	//if ( player->m_Local.m_flDuckJumpTime > 0.0f )
	//	return false;

	// Now that we've allowed jumps to take place while unducking, we have to
	// immediately finish the unduck. This pretty much follows TFC behaviour
	// now.
	if (player->m_Local.m_bDucking && (player->GetFlags() & FL_DUCKING))
	{
		FinishUnDuck();
	}

	// In the air now.
	SetGroundEntity( (CBaseEntity *)NULL );

	// This following dynamic cap is documented here:
	//		http://www.madabouthats.org/code-tf2/viewtopic.php?t=2360

	const float baseline = /*1.9f*/ 1.52f * mv->m_flMaxSpeed;
	const float cap = /*2.0f*/ 1.6f * mv->m_flMaxSpeed;
	const float pcfactor = 0.5f;
	const float speed = FastSqrt(mv->m_vecVelocity[0] * mv->m_vecVelocity[0] + mv->m_vecVelocity[1] * mv->m_vecVelocity[1]);
	
	if (speed > cap)
	{
		float applied_cap = (speed - cap) * pcfactor + baseline;
		float multi = applied_cap / speed;

		mv->m_vecVelocity[0] *= multi;
		mv->m_vecVelocity[1] *= multi;

		Assert(multi <= 1.0f);
	}

	// Mirv: Play proper jump sounds
	//player->PlayStepSound( mv->m_vecAbsOrigin, player->m_pSurfaceData, 1.0, true );
	CFFPlayer *FFPlayer = dynamic_cast<CFFPlayer *> (player);
	FFPlayer->PlayJumpSound(mv->m_vecAbsOrigin, player->m_pSurfaceData, 1.0);

	// Mirv: This fixes the jump animation
	//MoveHelper()->PlayerSetAnimation( PLAYER_JUMP );
	ffplayer->DoAnimationEvent(PLAYERANIMEVENT_JUMP);

	float fGroundFactor = 1.0f;
	if (player->m_pSurfaceData)
	{
		fGroundFactor = player->m_pSurfaceData->game.jumpFactor; 
	}

	// --> Mirv: Trimp code v2.0!
	//float fMul = FF_MUL_CONSTANT;
	//float fMul = 268.3281573;
	float fMul = 268.6261342; //sqrt(2.0f * 800.0f * 45.1f);

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

		// They have to be at least moving a bit
		if (flHorizontalSpeed > 5.0f)
		{
			vecVelocity /= flHorizontalSpeed;

            float flDotProduct = DotProduct(vecVelocity, pm.plane.normal);

			// Don't do anything for flat ground or downwardly sloping (relative to motion)
			// Changed to 0.15f to make it a bit less trimpy on only slightly uneven ground
			if (flDotProduct < -0.15f || flDotProduct > 0.15f)
			{
				// This is one way to do it
				fMul += -flDotProduct * flHorizontalSpeed * sv_trimpmultiplier.GetFloat(); //0.6f;

				// This is another that'll give some different height results
				// UNDONE: Reverted back to the original way for now
				//Vector reflect = mv->m_vecVelocity + (-2.0f * pm.plane.normal * DotProduct(mv->m_vecVelocity, pm.plane.normal));
				//float flSpeedAmount = clamp((flLength - 400.0f) / 800.0f, 0, 1.0f);
				//fMul += reflect.z * flSpeedAmount;
			}
		}
	}
	// <-- Mirv: Trimp code v2.0!

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

	CFFPlayer *pPlayer = ToFFPlayer(player);

	// Double jump
	if (pPlayer->m_bCanDoubleJump)
	{
		float flElapsed = pPlayer->m_flNextJumpTimeForDouble - gpGlobals->curtime;

		if (flElapsed > 0 && flElapsed < 0.4f)
		{
			fMul *= 1.5f;

#ifdef GAME_DLL
			DevMsg("[S] Double jump %f!\n", fMul);
#else
			DevMsg("[C] Double jump %f!\n", fMul);
#endif

			pPlayer->m_bCanDoubleJump = false;
		}

		pPlayer->m_flNextJumpTimeForDouble = gpGlobals->curtime + 0.5f;
	}

	// --> Mirv: Add on new velocity
	if( mv->m_vecVelocity[2] < 0 )
		mv->m_vecVelocity[2] = 0;

	mv->m_vecVelocity[2] += fMul;
	// <-- Mirv: Add on new velocity

	FinishGravity();

//	mv->m_outJumpVel.z += mv->m_vecVelocity[2] - startz;
//	mv->m_outStepHeight += 0.15f;

	// Set jump time.
/*
	if ( gpGlobals->maxClients == 1 )
	{
		player->m_Local.m_flJumpTime = GAMEMOVEMENT_JUMP_TIME;
		player->m_Local.m_bInDuckJump = true;
	}
*/

	// Flag that we jumped.
	mv->m_nOldButtons |= IN_JUMP;	// don't jump again until released
	return true;
}

////-----------------------------------------------------------------------------
//// Purpose: 
////-----------------------------------------------------------------------------
//void CFFGameMovement::FullWalkMove(void)
//{
//	CFFPlayer *ffplayer = ToFFPlayer(player);
//	if(ffplayer == NULL)
//	{
//		BaseClass::FullWalkMove();
//		return;
//	}
//
//	if ( !CheckWater() ) 
//	{
//		StartGravity();
//	}
//
//	// If we are leaping out of the water, just update the counters.
//	if (player->m_flWaterJumpTime)
//	{
//		WaterJump();
//		TryPlayerMove();
//		// See if we are still in water?
//		CheckWater();
//		return;
//	}
//
//	// If we are swimming in the water, see if we are nudging against a place we can jump up out
//	//  of, and, if so, start out jump.  Otherwise, if we are not moving up, then reset jump timer to 0
//	if ( player->GetWaterLevel() >= WL_Waist ) 
//	{
//		if ( player->GetWaterLevel() == WL_Waist )
//		{
//			CheckWaterJump();
//		}
//
//		// If we are falling again, then we must not trying to jump out of water any more.
//		if ( mv->m_vecVelocity[2] < 0 && 
//			player->m_flWaterJumpTime )
//		{
//			player->m_flWaterJumpTime = 0;
//		}
//
//		// Was jump button pressed?
//		if (mv->m_nButtons & IN_JUMP)
//		{
//			CheckJumpButton();
//		}
//		else
//		{
//			if(ffplayer->m_iLocalSkiState == 1)
//			{
//				ffplayer->StopSkiing();
//			}
//			mv->m_nOldButtons &= ~IN_JUMP;
//		}
//
//		// Perform regular water movement
//		WaterMove();
//
//		// Redetermine position vars
//		CategorizePosition();
//
//		// If we are on ground, no downward velocity.
//		if ( player->GetGroundEntity() != NULL )
//		{
//			mv->m_vecVelocity[2] = 0;			
//		}
//	}
//	else
//		// Not fully underwater
//	{
//		// Was jump button pressed?
//		if (mv->m_nButtons & IN_JUMP)
//		{
//			CheckJumpButton();
//		}
//		else
//		{
//			if(ffplayer->m_iLocalSkiState == 1)
//			{
//				ffplayer->StopSkiing();
//			}
//			mv->m_nOldButtons &= ~IN_JUMP;
//		}
//
//		// Fricion is handled before we add in any base velocity. That way, if we are on a conveyor, 
//		//  we don't slow when standing still, relative to the conveyor.
//		if (player->GetGroundEntity() != NULL)
//		{
//			// --> Mirv: Removed Skiing for now
//			//if(ffplayer->m_iSkiState == 0)
//			//	mv->m_vecVelocity[2] = 0.0f;
//			// <-- Mirv: Removed Skiing for now
//			Friction();
//		}
//
//		// Make sure velocity is valid.
//		CheckVelocity();
//
//		if (player->GetGroundEntity() != NULL)
//		{
//			// --> Mirv: Removed Skiing for now
//			//if(ffplayer->m_iSkiState == 0)
//				WalkMove();
//			//else
//			//	SkiMove();
//			// <-- Mirv: Removed Skiing for now
//		}
//		else
//		{
//			AirMove();  // Take into account movement when in air.
//		}
//
//		// Set final flags.
//		CategorizePosition();
//
//		// Make sure velocity is valid.
//		CheckVelocity();
//
//		// Add any remaining gravitational component.
//		if ( !CheckWater() )
//		{
//			FinishGravity();
//		}
//
//		// If we are on ground, no downward velocity.
//		// --> Mirv: No skiing for now
//		//if (player->GetGroundEntity() != NULL && ffplayer->m_iLocalSkiState == 0)
//		if (player->GetGroundEntity() != NULL)
//		// <-- Mirv: No skiing for now
//		{
//			mv->m_vecVelocity[2] = 0.0f;
//		}
//		CheckFalling();
//	}
//
//	if  ( ( m_nOldWaterLevel == WL_NotInWater && player->GetWaterLevel() != WL_NotInWater ) ||
//		( m_nOldWaterLevel != WL_NotInWater && player->GetWaterLevel() == WL_NotInWater ) )
//	{
//		PlaySwimSound();
//	}
//}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFGameMovement::CheckWaterJump( void )
{
	Vector	flatforward;
	Vector forward;
	Vector	flatvelocity;
	float curspeed;

	AngleVectors( mv->m_vecViewAngles, &forward );  // Determine movement angles

	// Already water jumping.
	if (player->m_flWaterJumpTime)
		return;

	// Don't hop out if we just jumped in
	if (mv->m_vecVelocity[2] < 20)
		return; // only hop out if we are moving up

	// See if we are backing up
	flatvelocity[0] = mv->m_vecVelocity[0];
	flatvelocity[1] = mv->m_vecVelocity[1];
	flatvelocity[2] = 0;

	// Must be moving
	curspeed = VectorNormalize( flatvelocity );
	
	// see if near an edge
	flatforward[0] = forward[0];
	flatforward[1] = forward[1];
	flatforward[2] = 0;
	VectorNormalize (flatforward);

	// Are we backing into water from steps or something?  If so, don't pop forward
	if ( curspeed != 0.0 && ( DotProduct( flatvelocity, flatforward ) < 0.0 ) )
		return;

	Vector vecStart;
	// Start line trace at waist height (using the center of the player for this here)
	vecStart= mv->m_vecAbsOrigin + (GetPlayerMins() + GetPlayerMaxs() ) * 0.5;

	Vector vecEnd;
	VectorMA( vecStart, 24.0f, flatforward, vecEnd );
	
	trace_t tr;
	TracePlayerBBox( vecStart, vecEnd, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, tr );
	if ( tr.fraction < 1.0 )		// solid at waist
	{
		IPhysicsObject *pPhysObj = tr.m_pEnt->VPhysicsGetObject();
		if ( pPhysObj )
		{
			if ( pPhysObj->GetGameFlags() & FVPHYSICS_PLAYER_HELD )
				return;
		}

		vecStart.z = mv->m_vecAbsOrigin.z + player->GetViewOffset().z + WATERJUMP_HEIGHT; 
		VectorMA( vecStart, 24.0f, flatforward, vecEnd );
		VectorMA( vec3_origin, -50.0f, tr.plane.normal, player->m_vecWaterJumpVel );

		TracePlayerBBox( vecStart, vecEnd, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, tr );
		if ( tr.fraction == 1.0 )		// open at eye level
		{
			// Now trace down to see if we would actually land on a standable surface.
			VectorCopy( vecEnd, vecStart );
			vecEnd.z -= 1024.0f;
			TracePlayerBBox( vecStart, vecEnd, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, tr );
			if ( ( tr.fraction < 1.0f ) && ( tr.plane.normal.z >= 0.7 ) )
			{
				mv->m_vecVelocity[2] = 256.0f;			// Push up
				mv->m_nOldButtons |= IN_JUMP;		// Don't jump again until released
				player->AddFlag( FL_WATERJUMP );
				player->m_flWaterJumpTime = 2000.0f;	// Do this for 2 seconds
			}
		}
	}
}

////-----------------------------------------------------------------------------
//// Purpose: 
////-----------------------------------------------------------------------------
//void CFFGameMovement::WalkMove(void)
//{
//	CFFPlayer *ffplayer = ToFFPlayer(player);
//	if(ffplayer == NULL)
//	{
//		BaseClass::WalkMove();
//		return;
//	}
//
//	int i;
//
//	Vector wishvel;
//	float spd;
//	float fmove, smove;
//	Vector wishdir;
//	float wishspeed;
//
//	Vector dest;
//	trace_t pm;
//	Vector forward, right, up;
//
//	AngleVectors (mv->m_vecViewAngles, &forward, &right, &up);  // Determine movement angles
//
//	CHandle< CBaseEntity > oldground;
//	oldground = player->GetGroundEntity();
//
//	// Copy movement amounts
//	fmove = mv->m_flForwardMove;
//	smove = mv->m_flSideMove;
//
//	forward[2] = 0;
//	right[2]   = 0;
//
//	VectorNormalize (forward);  // Normalize remainder of vectors.
//	VectorNormalize (right);    // 
//
//	for (i=0 ; i<2 ; i++)       // Determine x and y parts of velocity
//		wishvel[i] = forward[i]*fmove + right[i]*smove;
//
//	wishvel[2] = 0;             // Zero out z part of velocity
//
//	VectorCopy (wishvel, wishdir);   // Determine maginitude of speed of move
//	wishspeed = VectorNormalize(wishdir);
//
//	//
//	// Clamp to server defined max speed
//	//
//	if ((wishspeed != 0.0f) && (wishspeed > mv->m_flMaxSpeed))
//	{
//		VectorScale (wishvel, mv->m_flMaxSpeed/wishspeed, wishvel);
//		wishspeed = mv->m_flMaxSpeed;
//	}
//
//	// Set pmove velocity
//	mv->m_vecVelocity[2] = 0;
//	Accelerate (wishdir, wishspeed, sv_accelerate.GetFloat());
//	mv->m_vecVelocity[2] = 0;
//
//	// Add in any base velocity to the current velocity.
//	VectorAdd (mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
//
//	spd = VectorLength( mv->m_vecVelocity );
//
//	if ( spd < 1.0f )
//	{
//		mv->m_vecVelocity.Init();
//		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
//		VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
//		return;
//	}
//
//	// first try just moving to the destination	
//	dest[0] = mv->m_vecAbsOrigin[0] + mv->m_vecVelocity[0]*gpGlobals->frametime;
//	dest[1] = mv->m_vecAbsOrigin[1] + mv->m_vecVelocity[1]*gpGlobals->frametime;	
//	dest[2] = mv->m_vecAbsOrigin[2];
//
//	// first try moving directly to the next spot
//	TracePlayerBBox( mv->m_vecAbsOrigin, dest, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm );
//
//	// If we made it all the way, then copy trace end as new player position.
//	mv->m_outWishVel += wishdir * wishspeed;
//
//	if ( pm.fraction == 1 )
//	{
//		VectorCopy( pm.endpos, mv->m_vecAbsOrigin );
//		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
//		VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
//		return;
//	}
//
//	// Don't walk up stairs if not on ground.
//	if ( oldground == NULL && player->GetWaterLevel()  == 0 )
//	{
//		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
//		VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
//		return;
//	}
//
//	// If we are jumping out of water, don't do anything more.
//	if ( player->m_flWaterJumpTime )         
//	{
//		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
//		VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
//		return;
//	}
//
//	StepMove(dest, pm);
//
//	// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
//	VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
//}
//
////-----------------------------------------------------------------------------
//// Purpose: 
////-----------------------------------------------------------------------------
//void CFFGameMovement::SkiMove(void)
//{
//	CFFPlayer *ffplayer = ToFFPlayer(player);
//	if(ffplayer == NULL)
//	{
//		BaseClass::WalkMove();
//		return;
//	}
//
//	int i;
//
//	Vector wishvel;
//	float spd;
//	float fmove, smove;
//	Vector wishdir;
//	float wishspeed;
//
//	Vector dest;
//	trace_t pm;
//	Vector forward, right, up;
//
//	AngleVectors (mv->m_vecViewAngles, &forward, &right, &up);  // Determine movement angles
//
//	CHandle< CBaseEntity > oldground;
//	oldground = player->GetGroundEntity();
//
//	// Copy movement amounts
//	fmove = 0.0f; //mv->m_flForwardMove;
//	smove = 0.0f; //mv->m_flSideMove;
//
//	forward[2] = 0;
//	right[2]   = 0;
//
//	VectorNormalize (forward);  // Normalize remainder of vectors.
//	VectorNormalize (right);    // 
//
//	for (i=0 ; i<2 ; i++)       // Determine x and y parts of velocity
//		wishvel[i] = forward[i]*fmove + right[i]*smove;
//
//	wishvel[2] = 0;             // Zero out z part of velocity
//
//	VectorCopy (wishvel, wishdir);   // Determine maginitude of speed of move
//	wishspeed = VectorNormalize(wishdir);
//
//	//
//	// Clamp to server defined max speed
//	//
//	if ((wishspeed != 0.0f) && (wishspeed > mv->m_flMaxSpeed))
//	{
//		VectorScale (wishvel, mv->m_flMaxSpeed/wishspeed, wishvel);
//		wishspeed = mv->m_flMaxSpeed;
//	}
//
//	// Add in any base velocity to the current velocity.
//	VectorAdd (mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
//
//	spd = VectorLength( mv->m_vecVelocity );
//
//	if ( spd < 1.0f )
//	{
//		mv->m_vecVelocity.Init();
//		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
//		VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
//		return;
//	}
//
//	// first try just moving to the destination	
//	dest[0] = mv->m_vecAbsOrigin[0] + mv->m_vecVelocity[0]*gpGlobals->frametime;
//	dest[1] = mv->m_vecAbsOrigin[1] + mv->m_vecVelocity[1]*gpGlobals->frametime;	
//	dest[2] = mv->m_vecAbsOrigin[2] + mv->m_vecVelocity[2]*gpGlobals->frametime;
//
//	// Set pmove velocity
//	// mv->m_vecVelocity[2] = 0;
//
//
//	// If we made it all the way, then copy trace end as new player position.
//	// mv->m_outWishVel += wishdir * wishspeed;
//
//	// first try moving directly to the next spot
//	TracePlayerBBox( mv->m_vecAbsOrigin, dest, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm );
//	if ( pm.fraction == 1 )
//	{
//		VectorCopy( pm.endpos, mv->m_vecAbsOrigin );
//		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
//		VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
//		return;
//	}
//
//	// Don't walk up stairs if not on ground.
//	if ( oldground == NULL && player->GetWaterLevel()  == 0 )
//	{
//		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
//		VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
//		return;
//	}
//
//	// If we are jumping out of water, don't do anything more.
//	if ( player->m_flWaterJumpTime )         
//	{
//		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
//		VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
//		return;
//	}
//
//	StepMove(dest, pm);
//
//	// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
//	VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
//}
//
////-----------------------------------------------------------------------------
//// Purpose: 
////-----------------------------------------------------------------------------
//void CFFGameMovement::AirMove(void)
//{
//	int			i;
//	Vector		wishvel;
//	float		fmove, smove;
//	Vector		wishdir;
//	float		wishspeed;
//	Vector forward, right, up;
//
//	AngleVectors (mv->m_vecViewAngles, &forward, &right, &up);  // Determine movement angles
//
//	// Copy movement amounts
//	fmove = mv->m_flForwardMove;
//	smove = mv->m_flSideMove;
//
//	// Zero out z components of movement vectors
//	forward[2] = 0;
//	right[2]   = 0;
//	VectorNormalize(forward);  // Normalize remainder of vectors
//	VectorNormalize(right);    // 
//
//	for (i=0 ; i<2 ; i++)       // Determine x and y parts of velocity
//		wishvel[i] = forward[i]*fmove + right[i]*smove;
//	wishvel[2] = 0;             // Zero out z part of velocity
//
//	VectorCopy (wishvel, wishdir);   // Determine maginitude of speed of move
//	wishspeed = VectorNormalize(wishdir);
//
//	//
//	// clamp to server defined max speed
//	//
//	if ( wishspeed != 0 && (wishspeed > mv->m_flMaxSpeed))
//	{
//		VectorScale (wishvel, mv->m_flMaxSpeed/wishspeed, wishvel);
//		wishspeed = mv->m_flMaxSpeed;
//	}
//
//	AirAccelerate( wishdir, wishspeed, sv_airaccelerate.GetFloat() );
//
//	// Add in any base velocity to the current velocity.
//	VectorAdd(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
//
//	TryPlayerMove();
//
//	// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
//	VectorSubtract( mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity );
//}
//
////-----------------------------------------------------------------------------
//// Purpose: Does the basic move attempting to climb up step heights.  It uses
////          the mv->m_vecAbsOrigin and mv->m_vecVelocity.  It returns a new
////          new mv->m_vecAbsOrigin, mv->m_vecVelocity, and mv->m_outStepHeight.
////-----------------------------------------------------------------------------
//void CFFGameMovement::StepMove(Vector &vecDestination, trace_t &trace)
//{
//	Vector vecEndPos;
//	VectorCopy( vecDestination, vecEndPos );
//
//	// Try sliding forward both on ground and up 16 pixels
//	//  take the move that goes farthest
//	Vector vecPos, vecVel;
//	VectorCopy( mv->m_vecAbsOrigin, vecPos );
//	VectorCopy( mv->m_vecVelocity, vecVel );
//
//	// Slide move down.
//	TryPlayerMove( &vecEndPos, &trace );
//
//	// Down results.
//	Vector vecDownPos, vecDownVel;
//	VectorCopy( mv->m_vecAbsOrigin, vecDownPos );
//	VectorCopy( mv->m_vecVelocity, vecDownVel );
//
//	// Reset original values.
//	VectorCopy( vecPos, mv->m_vecAbsOrigin );
//	VectorCopy( vecVel, mv->m_vecVelocity );
//
//	// Move up a stair height.
//	VectorCopy( mv->m_vecAbsOrigin, vecEndPos );
//	if ( player->m_Local.m_bAllowAutoMovement )
//	{
//		vecEndPos.z += player->m_Local.m_flStepSize;
//	}
//
//	TracePlayerBBox( mv->m_vecAbsOrigin, vecEndPos, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace );
//	if ( !trace.startsolid && !trace.allsolid )
//	{
//		VectorCopy( trace.endpos, mv->m_vecAbsOrigin );
//	}
//
//	// Slide move up.
//	TryPlayerMove();
//
//	// Move down a stair (attempt to).
//	VectorCopy( mv->m_vecAbsOrigin, vecEndPos );
//	if ( player->m_Local.m_bAllowAutoMovement )
//	{
//		vecEndPos.z -= player->m_Local.m_flStepSize;
//	}
//
//	TracePlayerBBox( mv->m_vecAbsOrigin, vecEndPos, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, trace );
//
//	// If we are not on the ground any more then use the original movement attempt.
//	if ( trace.plane.normal[2] < 0.7 )
//	{
//		VectorCopy( vecDownPos, mv->m_vecAbsOrigin );
//		VectorCopy( vecDownVel, mv->m_vecVelocity );
//		float flStepDist = mv->m_vecAbsOrigin.z - vecPos.z;
//		if ( flStepDist > 0.0f )
//		{
//			mv->m_outStepHeight += flStepDist;
//		}
//		return;
//	}
//
//	// If the trace ended up in empty space, copy the end over to the origin.
//	if ( !trace.startsolid && !trace.allsolid )
//	{
//		VectorCopy( trace.endpos, mv->m_vecAbsOrigin );
//	}
//
//	// Copy this origin to up.
//	Vector vecUpPos;
//	VectorCopy( mv->m_vecAbsOrigin, vecUpPos );
//
//	// decide which one went farther
//	float flDownDist = ( vecDownPos.x - vecPos.x ) * ( vecDownPos.x - vecPos.x ) + ( vecDownPos.y - vecPos.y ) * ( vecDownPos.y - vecPos.y );
//	float flUpDist = ( vecUpPos.x - vecPos.x ) * ( vecUpPos.x - vecPos.x ) + ( vecUpPos.y - vecPos.y ) * ( vecUpPos.y - vecPos.y );
//	if ( flDownDist > flUpDist )
//	{
//		VectorCopy( vecDownPos, mv->m_vecAbsOrigin );
//		VectorCopy( vecDownVel, mv->m_vecVelocity );
//	}
//	else 
//	{
//		// copy z value from slide move
//		mv->m_vecVelocity.z = vecDownVel.z;
//	}
//
//	float flStepDist = mv->m_vecAbsOrigin.z - vecPos.z;
//	if ( flStepDist > 0 )
//	{
//		mv->m_outStepHeight += flStepDist;
//	}
//}
//
////-----------------------------------------------------------------------------
//// Purpose: 
//// Output : int
////-----------------------------------------------------------------------------
//int CFFGameMovement::TryPlayerMove( Vector *pFirstDest, trace_t *pFirstTrace )
//{
//	int			bumpcount, numbumps;
//	Vector		dir;
//	float		d;
//	int			numplanes;
//	Vector		planes[MAX_CLIP_PLANES];
//	Vector		primal_velocity, original_velocity;
//	Vector      new_velocity;
//	int			i, j;
//	trace_t	pm;
//	Vector		end;
//	float		time_left, allFraction;
//	int			blocked;		
//
//	numbumps  = 4;           // Bump up to four times
//
//	blocked   = 0;           // Assume not blocked
//	numplanes = 0;           //  and not sliding along any planes
//
//	VectorCopy (mv->m_vecVelocity, original_velocity);  // Store original velocity
//	VectorCopy (mv->m_vecVelocity, primal_velocity);
//
//	allFraction = 0;
//	time_left = gpGlobals->frametime;   // Total time for this movement operation.
//
//	new_velocity.Init();
//
//	for (bumpcount=0 ; bumpcount < numbumps; bumpcount++)
//	{
//		if ( mv->m_vecVelocity.Length() == 0.0 )
//			break;
//
//		// Assume we can move all the way from the current origin to the
//		//  end point.
//		VectorMA( mv->m_vecAbsOrigin, time_left, mv->m_vecVelocity, end );
//
//		// See if we can make it from origin to end point.
//		TracePlayerBBox( mv->m_vecAbsOrigin, end, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm );
//
//		allFraction += pm.fraction;
//
//		// If we started in a solid object, or we were in solid space
//		//  the whole way, zero out our velocity and return that we
//		//  are blocked by floor and wall.
//		if (pm.allsolid)
//		{	
//			// entity is trapped in another solid
//			VectorCopy (vec3_origin, mv->m_vecVelocity);
//			return 4;
//		}
//
//		// If we moved some portion of the total distance, then
//		//  copy the end position into the pmove.origin and 
//		//  zero the plane counter.
//		if( pm.fraction > 0 )
//		{	
//			// actually covered some distance
//			VectorCopy (pm.endpos, mv->m_vecAbsOrigin);
//			VectorCopy (mv->m_vecVelocity, original_velocity);
//			numplanes = 0;
//		}
//
//		// If we covered the entire distance, we are done
//		//  and can return.
//		if (pm.fraction == 1)
//		{
//			break;		// moved the entire distance
//		}
//
//		// Save entity that blocked us (since fraction was < 1.0)
//		//  for contact
//		// Add it if it's not already in the list!!!
//		MoveHelper( )->AddToTouched( pm, mv->m_vecVelocity );
//
//		// If the plane we hit has a high z component in the normal, then
//		//  it's probably a floor
//		if (pm.plane.normal[2] > 0.7)
//		{
//			blocked |= 1;		// floor
//		}
//		// If the plane has a zero z component in the normal, then it's a 
//		//  step or wall
//		if (!pm.plane.normal[2])
//		{
//			blocked |= 2;		// step / wall
//		}
//
//		// Reduce amount of m_flFrameTime left by total time left * fraction
//		//  that we covered.
//		time_left -= time_left * pm.fraction;
//
//		// Did we run out of planes to clip against?
//		if (numplanes >= MAX_CLIP_PLANES)
//		{	
//			// this shouldn't really happen
//			//  Stop our movement if so.
//			VectorCopy (vec3_origin, mv->m_vecVelocity);
//			//Con_DPrintf("Too many planes 4\n");
//
//			break;
//		}
//
//		// Set up next clipping plane
//		VectorCopy (pm.plane.normal, planes[numplanes]);
//		numplanes++;
//
//		// modify original_velocity so it parallels all of the clip planes
//		//
//
//		// relfect player velocity 
//		// Only give this a try for first impact plane because you can get yourself stuck in an acute corner by jumping in place
//		//  and pressing forward and nobody was really using this bounce/reflection feature anyway...
//		if ( numplanes == 1 &&
//			player->GetMoveType() == MOVETYPE_WALK &&
//			player->GetGroundEntity() == NULL )	
//		{
//			for ( i = 0; i < numplanes; i++ )
//			{
//				if ( planes[i][2] > 0.7  )
//				{
//					// floor or slope
//					ClipVelocity( original_velocity, planes[i], new_velocity, 1 );
//					VectorCopy( new_velocity, original_velocity );
//				}
//				else
//				{
//					ClipVelocity( original_velocity, planes[i], new_velocity, 1.0 + sv_bounce.GetFloat() * (1 - player->m_surfaceFriction) );
//				}
//			}
//
//			VectorCopy( new_velocity, mv->m_vecVelocity );
//			VectorCopy( new_velocity, original_velocity );
//		}
//		else
//		{
//			for (i=0 ; i < numplanes ; i++)
//			{
//				ClipVelocity (
//					original_velocity,
//					planes[i],
//					mv->m_vecVelocity,
//					1);
//
//				for (j=0 ; j<numplanes ; j++)
//					if (j != i)
//					{
//						// Are we now moving against this plane?
//						if (mv->m_vecVelocity.Dot(planes[j]) < 0)
//							break;	// not ok
//					}
//					if (j == numplanes)  // Didn't have to clip, so we're ok
//						break;
//			}
//
//			// Did we go all the way through plane set
//			if (i != numplanes)
//			{	// go along this plane
//				// pmove.velocity is set in clipping call, no need to set again.
//				;  
//			}
//			else
//			{	// go along the crease
//				if (numplanes != 2)
//				{
//					VectorCopy (vec3_origin, mv->m_vecVelocity);
//					break;
//				}
//				CrossProduct (planes[0], planes[1], dir);
//				d = dir.Dot(mv->m_vecVelocity);
//				VectorScale (dir, d, mv->m_vecVelocity );
//			}
//
//			//
//			// if original velocity is against the original velocity, stop dead
//			// to avoid tiny occilations in sloping corners
//			//
//			d = mv->m_vecVelocity.Dot(primal_velocity);
//			if (d <= 0)
//			{
//				//Con_DPrintf("Back\n");
//				VectorCopy (vec3_origin, mv->m_vecVelocity);
//				break;
//			}
//		}
//	}
//
//	if ( allFraction == 0 )
//	{
//		VectorCopy (vec3_origin, mv->m_vecVelocity);
//	}
//
//	return blocked;
//}
//
////-----------------------------------------------------------------------------
//// Purpose: 
////-----------------------------------------------------------------------------
//void CFFGameMovement::Friction(void)
//{
//	CFFPlayer *ffplayer = ToFFPlayer(player);
//	if(ffplayer == NULL)
//	{
//		BaseClass::Friction();
//		return;
//	}
//
//	float	speed, newspeed, control;
//	float	friction;
//	float	drop;
//	Vector newvel;
//
//
//	// If we are in water jump cycle, don't apply friction
//	if (player->m_flWaterJumpTime)
//		return;
//
//	// Calculate speed
//	speed = VectorLength( mv->m_vecVelocity );
//
//	// If too slow, return
//	if (speed < 0.1f)
//	{
//		return;
//	}
//
//	drop = 0;
//
//	// apply ground friction
//	if (player->GetGroundEntity() != NULL)  // On an entity that is the ground
//	{
//		if(ffplayer->m_iSkiState == 0)
//		{
//			// REGULAR FRICTION
//
//            // this was only partially removed in the FF, I'm not sure why, but it's completely removed now
//			/*
//			Vector start, stop;
//			trace_t pm;
//
//			//
//			// NOTE: added a "1.0f" to the player minimum (bbox) value so that the 
//			//       trace starts just inside of the bounding box, this make sure
//			//       that we don't get any collision epsilon (on surface) errors.
//			//		 The significance of the 16 below is this is how many units out front we are checking
//			//		 to see if the player box would fall.  The 49 is the number of units down that is required
//			//		 to be considered a fall.  49 is derived from 1 (added 1 from above) + 48 the max fall 
//			//		 distance a player can fall and still jump back up.
//			//
//			//		 UNDONE: In some cases there are still problems here.  Specifically, no collision check is
//			//		 done so 16 units in front of the player could be inside a volume or past a collision point.
//			start[0] = stop[0] = mv->m_vecAbsOrigin[0] + (mv->m_vecVelocity[0]/speed)*16;
//			start[1] = stop[1] = mv->m_vecAbsOrigin[1] + (mv->m_vecVelocity[1]/speed)*16;
//			start[2] = mv->m_vecAbsOrigin[2] + ( GetPlayerMins()[2] + 1.0f );
//			stop[2] = start[2] - 49;
//
//			TracePlayerBBox( start, stop, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm ); 
//			*/
//
//			friction = sv_friction.GetFloat();
//
//			// Grab friction value.
//			friction *= player->m_surfaceFriction;  // player friction?
//
//			// Bleed off some speed, but if we have less than the bleed
//			//  threshhold, bleed the theshold amount.
//			control = (speed < sv_stopspeed.GetFloat()) ?
//				sv_stopspeed.GetFloat() : speed;
//
//			// Add the amount to the drop amount.
//			drop += control*friction*gpGlobals->frametime;
//		}
//		else
//		{
//			// SKIING FRICTION
//			// since skiing involves ramps a lot, we need to compute the correct friction
//			// for a body sliding on a ramp
//
//			Vector newvel;
//			Vector start, stop;
//			trace_t pm;
//			start[0] = mv->m_vecAbsOrigin[0];
//			start[1] = mv->m_vecAbsOrigin[1];
//			start[2] = mv->m_vecAbsOrigin[2] + (GetPlayerMins()[2] + 1.0f);
//			stop[2] = start[0];
//			stop[2] = start[1];
//			stop[2] = start[2] - 16.0f;
//			TracePlayerBBox(start, stop, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm); 
//			if(pm.fraction != 1.0f)
//			{
//				// skiing friction is far less than regular friction
//				friction = sv_friction.GetFloat();
//
//				// Grab friction value.
//				friction *= player->m_surfaceFriction * 0.2f * pm.plane.normal.z;  // player friction?
//
//				// Bleed off some speed, but if we have less than the bleed
//				//  threshhold, bleed the theshold amount.
//				control = (speed < sv_stopspeed.GetFloat()) ?
//					sv_stopspeed.GetFloat() : speed;
//
//				// Add the amount to the drop amount.
//				drop += control*friction*gpGlobals->frametime;
//			}
//			else
//			{
//				// skiing friction is far less than regular friction
//				friction = sv_friction.GetFloat();
//
//				// Grab friction value.
//				friction *= player->m_surfaceFriction * 0.2f;  // player friction?
//
//				// Bleed off some speed, but if we have less than the bleed
//				//  threshhold, bleed the theshold amount.
//				control = (speed < sv_stopspeed.GetFloat()) ?
//					sv_stopspeed.GetFloat() : speed;
//
//				// Add the amount to the drop amount.
//				drop += control*friction*gpGlobals->frametime;
//			}
//		}
//	}
//
//	// scale the velocity
//	newspeed = speed - drop;
//	if (newspeed < 0)
//		newspeed = 0;
//
//	// Determine proportion of old speed we are using.
//	newspeed /= speed;
//
//	// Adjust velocity according to proportion.
//	newvel[0] = mv->m_vecVelocity[0] * newspeed;
//	newvel[1] = mv->m_vecVelocity[1] * newspeed;
//	newvel[2] = mv->m_vecVelocity[2] * newspeed;
//
//	VectorCopy( newvel, mv->m_vecVelocity );
//	mv->m_outWishVel -= (1.f-newspeed) * mv->m_vecVelocity;
//}
//
////-----------------------------------------------------------------------------
//// Purpose: 
////-----------------------------------------------------------------------------
//void CFFGameMovement::StartGravity(void)
//{
//	CFFPlayer *ffplayer = ToFFPlayer(player);
//	if(ffplayer == NULL)
//	{
//		BaseClass::StartGravity();
//		return;
//	}
//
//	float ent_gravity;
//
//	if (player->GetGravity())
//		ent_gravity = player->GetGravity();
//	else
//		ent_gravity = 1.0;
//
//	// modify the player velocity depending on gravity and the ramp angle
//	// this is for skiing only
//	// trace down to get the normal of the surface we're on
//	if(ffplayer->m_iSkiState == 1 && player->GetGroundEntity() != NULL)
//	{
//		Vector start, stop;
//		trace_t pm;
//		start[0] = mv->m_vecAbsOrigin[0];
//		start[1] = mv->m_vecAbsOrigin[1];
//		start[2] = mv->m_vecAbsOrigin[2] + (GetPlayerMins()[2] + 1.0f);
//		stop[2] = start[0];
//		stop[2] = start[1];
//		stop[2] = start[2] - 16.0f;
//		TracePlayerBBox(start, stop, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm); 
//		if(pm.fraction != 1.0f)
//		{
//            // velocity change due to gravity when on a plane
//			Vector grav(0.0f, 0.0f, -ent_gravity * sv_gravity.GetFloat() * 0.5 * gpGlobals->frametime);
//			Vector newvel = mv->m_vecVelocity + grav + (pm.plane.normal * -DotProduct(grav, pm.plane.normal));
//			VectorCopy(newvel, mv->m_vecVelocity);
//		}
//	}
//	else
//	{
//		// Add gravity so they'll be in the correct position during movement
//		// yes, this 0.5 looks wrong, but it's not.  
//		mv->m_vecVelocity[2] -= (ent_gravity * sv_gravity.GetFloat() * 0.5 * gpGlobals->frametime );
//	}
//
//	mv->m_vecVelocity[2] += player->GetBaseVelocity()[2] * gpGlobals->frametime;
//	Vector temp = player->GetBaseVelocity();
//	temp[2] = 0;
//	player->SetBaseVelocity( temp );
//
//	CheckVelocity();
//}
//
////-----------------------------------------------------------------------------
//// Purpose: 
////-----------------------------------------------------------------------------
//void CFFGameMovement::FinishGravity(void)
//{
//	CFFPlayer *ffplayer = ToFFPlayer(player);
//	if(ffplayer == NULL)
//	{
//		BaseClass::FinishGravity();
//		return;
//	}
//
//	float ent_gravity;
//
//	if ( player->m_flWaterJumpTime )
//		return;
//
//	if ( player->GetGravity() )
//		ent_gravity = player->GetGravity();
//	else
//		ent_gravity = 1.0;
//
//	// modify the player velocity depending on gravity and the ramp angle
//	// this is for skiing only
//	// trace down to get the normal of the surface we're on
//	if(ffplayer->m_iSkiState == 1 && player->GetGroundEntity() != NULL)
//	{
//		Vector newvel;
//		Vector start, stop;
//		trace_t pm;
//		start[0] = mv->m_vecAbsOrigin[0];
//		start[1] = mv->m_vecAbsOrigin[1];
//		start[2] = mv->m_vecAbsOrigin[2] + (GetPlayerMins()[2] + 1.0f);
//		stop[2] = start[0];
//		stop[2] = start[1];
//		stop[2] = start[2] - 16.0f;
//		TracePlayerBBox(start, stop, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm); 
//		if(pm.fraction != 1.0f)
//		{
//			// velocity change due to gravity when on a plane
//			Vector grav(0.0f, 0.0f, -ent_gravity * sv_gravity.GetFloat() * 0.5 * gpGlobals->frametime);
//			Vector newvel = mv->m_vecVelocity + grav + (pm.plane.normal * -DotProduct(grav, pm.plane.normal));
//			VectorCopy(newvel, mv->m_vecVelocity);
//		}
//	}
//	else
//	{
//		// Get the correct velocity for the end of the dt 
//		mv->m_vecVelocity[2] -= (ent_gravity * sv_gravity.GetFloat() * gpGlobals->frametime * 0.5);
//	}
//
//	CheckVelocity();
//}


// Expose our interface.
static CFFGameMovement g_GameMovement;
IGameMovement *g_pGameMovement = ( IGameMovement * )&g_GameMovement;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CFFGameMovement, IGameMovement,INTERFACENAME_GAMEMOVEMENT, g_GameMovement );


// ---------------------------------------------------------------------------------------- //
// CFFGameMovement.
// ---------------------------------------------------------------------------------------- //

CFFGameMovement::CFFGameMovement()
{
	//m_vecViewOffsetNormal = FF_PLAYER_VIEW_OFFSET;
}

// --> Mirv: Map guides, client side part
void CFFGameMovement::FullNoClipMove( float factor, float maxacceleration )
{
#ifdef CLIENT_DLL
	//CFFPlayer *ffplayer = (CFFPlayer *) player;

	//if( ffplayer->m_flNextMapGuideTime > gpGlobals->curtime )
	//{
	//	CFFMapGuide *nextguide = ffplayer->m_hNextMapGuide;
	//	CFFMapGuide *lastguide = ffplayer->m_hLastMapGuide;

	//	float t = clamp( ( ffplayer->m_flNextMapGuideTime - gpGlobals->curtime ) / 10.0f, 0, 1.0f );

	//	Vector vecNewPos = t * lastguide->GetAbsOrigin() + ( 1 - t ) * nextguide->GetAbsOrigin();
	//	QAngle angNewDirection = t * lastguide->GetAbsAngles() + ( 1 - t ) * nextguide->GetAbsAngles();

	//	// Apply these here
	//}
	//else
#endif
		BaseClass::FullNoClipMove( factor, maxacceleration );
}
// <-- Mirv: Map guides, client side part