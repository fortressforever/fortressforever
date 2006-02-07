//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "vcollide_parse.h"
#include "vehicle_base.h"
#include "npc_vehicledriver.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "soundenvelope.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "saverestore_utlvector.h"
#include "KeyValues.h"
#include "studio.h"
#include "bone_setup.h"
#include "collisionutils.h"
#include "animation.h"
#include "env_player_surface_trigger.h"

#ifdef HL2_DLL
	#include "hl2_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar g_debug_vehiclesound( "g_debug_vehiclesound", "0", FCVAR_CHEAT );
ConVar g_debug_vehicleexit( "g_debug_vehicleexit", "0", FCVAR_CHEAT );

#define HITBOX_SET	2

//-----------------------------------------------------------------------------
// Save/load
//-----------------------------------------------------------------------------
BEGIN_DATADESC_NO_BASE( vehicle_gear_t )

	DEFINE_FIELD( flMinSpeed,			FIELD_FLOAT ),
	DEFINE_FIELD( flMaxSpeed,			FIELD_FLOAT ),
	DEFINE_FIELD( flSpeedApproachFactor,FIELD_FLOAT ),

END_DATADESC()

BEGIN_DATADESC_NO_BASE( vehicle_crashsound_t )
	DEFINE_FIELD( flMinSpeed,			FIELD_FLOAT ),
	DEFINE_FIELD( flMinDeltaSpeed,		FIELD_FLOAT ),
	DEFINE_FIELD( iszCrashSound,		FIELD_STRING ),
	DEFINE_FIELD( gearLimit,			FIELD_INTEGER ),
END_DATADESC()

BEGIN_DATADESC_NO_BASE( vehiclesounds_t )

	DEFINE_AUTO_ARRAY( iszSound,		FIELD_STRING ),
	DEFINE_UTLVECTOR( pGears,			FIELD_EMBEDDED ),
	DEFINE_UTLVECTOR( crashSounds,		FIELD_EMBEDDED ),
	DEFINE_AUTO_ARRAY( iszStateSounds,	FIELD_STRING ),
	DEFINE_AUTO_ARRAY( minStateTime,	FIELD_FLOAT ),

END_DATADESC()

BEGIN_DATADESC_NO_BASE( CBaseServerVehicle )

// These are reset every time by the constructor of the owning class 
//	DEFINE_FIELD( m_pVehicle, FIELD_CLASSPTR ),
//	DEFINE_FIELD( m_pDrivableVehicle; ??? ),

	// Controls
	DEFINE_FIELD( m_nNPCButtons,		FIELD_INTEGER ),
	DEFINE_FIELD( m_nPrevNPCButtons,	FIELD_INTEGER ),
	DEFINE_FIELD( m_flTurnDegrees,		FIELD_FLOAT ),
	DEFINE_FIELD( m_flVehicleVolume,	FIELD_FLOAT ),

	// We're going to reparse this data from file in Precache
	DEFINE_EMBEDDED( m_vehicleSounds ),

	DEFINE_FIELD( m_iSoundGear,			FIELD_INTEGER ),
	DEFINE_FIELD( m_flSpeedPercentage,	FIELD_FLOAT ),
	DEFINE_SOUNDPATCH( m_pStateSound ),
	DEFINE_SOUNDPATCH( m_pStateSoundFade ),
	DEFINE_FIELD( m_soundState,			FIELD_INTEGER ),
	DEFINE_FIELD( m_soundStateStartTime, FIELD_TIME ),
	DEFINE_FIELD( m_lastSpeed, FIELD_FLOAT ),
	
// NOT SAVED
//	DEFINE_FIELD( m_EntryAnimations, CUtlVector ),
//	DEFINE_FIELD( m_ExitAnimations, CUtlVector ),
//	DEFINE_FIELD( m_bParsedAnimations,	FIELD_BOOLEAN ),

	DEFINE_FIELD( m_iCurrentExitAnim,	FIELD_INTEGER ),
	DEFINE_FIELD( m_vecCurrentExitEndPoint, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_chPreviousTextureType, FIELD_CHARACTER ),

	DEFINE_FIELD( m_savedViewOffset, FIELD_VECTOR ),
	DEFINE_FIELD( m_hExitBlocker, FIELD_EHANDLE ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: Base class for drivable vehicle handling. Contain it in your 
//			drivable vehicle.
//-----------------------------------------------------------------------------
CBaseServerVehicle::CBaseServerVehicle( void )
{
	m_pVehicle = NULL;
	m_pDrivableVehicle = NULL;
	m_nNPCButtons = 0;
	m_nPrevNPCButtons = 0;
	m_flTurnDegrees = 0;

	m_bParsedAnimations = false;
	m_iCurrentExitAnim = 0;
	m_vecCurrentExitEndPoint = vec3_origin;

	m_flVehicleVolume = 0.5;
	m_iSoundGear = 0;
	m_pStateSound = NULL;
	m_pStateSoundFade = NULL;
	m_soundState = SS_NONE;
	m_flSpeedPercentage = 0;

	for ( int i = 0; i < VS_NUM_SOUNDS; ++i )
	{
		m_vehicleSounds.iszSound[i] = NULL_STRING;
	}
	for ( i = 0; i < SS_NUM_STATES; i++ )
	{
		m_vehicleSounds.iszStateSounds[i] = NULL_STRING;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseServerVehicle::~CBaseServerVehicle( void )
{
	SoundShutdown(0);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::Precache( void )
{
	int i;
	// Precache our other sounds
	for ( i = 0; i < VS_NUM_SOUNDS; i++ )
	{
		if ( m_vehicleSounds.iszSound[i] != NULL_STRING )
		{
			CBaseEntity::PrecacheScriptSound( STRING(m_vehicleSounds.iszSound[i]) );
		}
	}
	for ( i = 0; i < m_vehicleSounds.crashSounds.Count(); i++ )
	{
		if ( m_vehicleSounds.crashSounds[i].iszCrashSound != NULL_STRING )
		{
			CBaseEntity::PrecacheScriptSound( STRING(m_vehicleSounds.crashSounds[i].iszCrashSound) );
		}
	}

	for ( i = 0; i < SS_NUM_STATES; i++ )
	{
		if ( m_vehicleSounds.iszStateSounds[i] != NULL_STRING )
		{
			CBaseEntity::PrecacheScriptSound( STRING(m_vehicleSounds.iszStateSounds[i]) );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Parses the vehicle's script for the vehicle sounds
//-----------------------------------------------------------------------------
bool CBaseServerVehicle::Initialize( const char *pScriptName )
{
	byte *pFile = UTIL_LoadFileForMe( pScriptName, NULL );
	if ( !pFile )
		return false;

	IVPhysicsKeyParser *pParse = physcollision->VPhysicsKeyParserCreate( (char *)pFile );
	CVehicleSoundsParser soundParser;
	while ( !pParse->Finished() )
	{
		const char *pBlock = pParse->GetCurrentBlockName();
		if ( !strcmpi( pBlock, "vehicle_sounds" ) )
		{
			pParse->ParseCustom( &m_vehicleSounds, &soundParser );
		}
		else
		{
			pParse->SkipBlock();
		}
	}
	physcollision->VPhysicsKeyParserDestroy( pParse );
	UTIL_FreeFile( pFile );

	Precache();

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::SetVehicle( CBaseEntity *pVehicle )
{ 
	m_pVehicle = pVehicle; 
	m_pDrivableVehicle = dynamic_cast<IDrivableVehicle*>(m_pVehicle);
	Assert( m_pDrivableVehicle );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
IDrivableVehicle *CBaseServerVehicle::GetDrivableVehicle( void )
{
	Assert( m_pDrivableVehicle );
	return m_pDrivableVehicle;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the driver. Unlike GetPassenger(VEHICLE_DRIVER), it will return
//			the NPC driver if it has one.
//-----------------------------------------------------------------------------
CBaseEntity	*CBaseServerVehicle::GetDriver( void )
{
	return GetPassenger(VEHICLE_DRIVER);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBasePlayer	*CBaseServerVehicle::GetPassenger( int nRole ) 
{ 
	Assert( nRole == VEHICLE_DRIVER ); 
	return ToBasePlayer( GetDrivableVehicle()->GetDriver() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseServerVehicle::GetPassengerRole( CBasePlayer *pPassenger )
{
	if (pPassenger == GetDrivableVehicle()->GetDriver())
		return VEHICLE_DRIVER;
	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: Get and set the current driver. Use PassengerRole_t enum in shareddefs.h for adding passengers
//-----------------------------------------------------------------------------
void CBaseServerVehicle::SetPassenger( int nRole, CBasePlayer *pPassenger )
{
	// Baseclass only handles vehicles with a single passenger
	Assert( nRole == VEHICLE_DRIVER ); 

	// Getting in? or out?
	if ( pPassenger )
	{
		m_savedViewOffset = pPassenger->GetViewOffset();
		pPassenger->SetViewOffset( vec3_origin );
		pPassenger->ShowCrosshair( false );

		GetDrivableVehicle()->EnterVehicle( pPassenger );

#ifdef HL2_DLL
		// Stop the player sprint and flashlight.
		CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player*>( pPassenger );
		if ( pHL2Player )
		{
			if ( pHL2Player->IsSprinting() )
			{
				pHL2Player->StopSprinting();
			}

			if ( pHL2Player->FlashlightIsOn() )
			{
				pHL2Player->FlashlightTurnOff();
			}
		}
#endif
	}
	else
	{
		// Restore the exiting player's view offset
		CBasePlayer *pPlayer = dynamic_cast<CBasePlayer *>(GetDriver());
		if ( pPlayer )
		{
			pPlayer->SetViewOffset( m_savedViewOffset );
			pPlayer->ShowCrosshair( true );
		}

		GetDrivableVehicle()->ExitVehicle( nRole );
		GetDrivableVehicle()->SetVehicleEntryAnim( false );
		UTIL_Remove( m_hExitBlocker );
	}
}
	
//-----------------------------------------------------------------------------
// Purpose: Get a position in *world space* inside the vehicle for the player to start at
//-----------------------------------------------------------------------------
void CBaseServerVehicle::GetPassengerStartPoint( int nRole, Vector *pPoint, QAngle *pAngles )
{
	Assert( nRole == VEHICLE_DRIVER ); 

	// NOTE: We don't set the angles, which causes them to remain the same
	// as they were before entering the vehicle

	CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating *>(m_pVehicle);
	if ( pAnimating )
	{
		char pAttachmentName[32];
		Q_snprintf( pAttachmentName, sizeof( pAttachmentName ), "vehicle_feet_passenger%d", nRole );
		int nFeetAttachmentIndex = pAnimating->LookupAttachment(pAttachmentName);
		if ( nFeetAttachmentIndex > 0 )
		{
			QAngle vecAngles;
			pAnimating->GetAttachment( nFeetAttachmentIndex, *pPoint, vecAngles );
			return;
		}
	}

	// Couldn't find the attachment point, so just use the origin
	*pPoint = m_pVehicle->GetAbsOrigin();
}

//---------------------------------------------------------------------------------
// Check Exit Point for leaving vehicle.
//
// Input: yaw/roll from vehicle angle to check for exit
//		  distance from origin to drop player (allows for different shaped vehicles
// Output: returns true if valid location, pEndPoint
//         updated with actual exit point
//---------------------------------------------------------------------------------
bool CBaseServerVehicle::CheckExitPoint( float yaw, int distance, Vector *pEndPoint )
{
	QAngle vehicleAngles = m_pVehicle->GetLocalAngles();
  	Vector vecStart = m_pVehicle->GetAbsOrigin();
  	Vector vecDir;
   
  	vecStart.z += 12;		// always 12" from ground
  	vehicleAngles[YAW] += yaw;	
  	AngleVectors( vehicleAngles, NULL, &vecDir, NULL );
	// Vehicles are oriented along the Y axis
	vecDir *= -1;
  	*pEndPoint = vecStart + vecDir * distance;
  
  	trace_t tr;
  	UTIL_TraceHull( vecStart, *pEndPoint, VEC_HULL_MIN, VEC_HULL_MAX, MASK_PLAYERSOLID, m_pVehicle, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction < 1.0 )
		return false;
  
  	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Where does this passenger exit the vehicle?
//-----------------------------------------------------------------------------
bool CBaseServerVehicle::GetPassengerExitPoint( int nRole, Vector *pExitPoint, QAngle *pAngles )
{ 
	Assert( nRole == VEHICLE_DRIVER ); 

	// First, see if we've got an attachment point
	CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating *>(m_pVehicle);
	if ( pAnimating )
	{
		Vector vehicleExitOrigin;
		QAngle vehicleExitAngles;
		if ( pAnimating->GetAttachment( "vehicle_driver_exit", vehicleExitOrigin, vehicleExitAngles ) )
		{
			// Make sure it's clear
			trace_t tr;
			UTIL_TraceHull( vehicleExitOrigin + Vector(0, 0, 12), vehicleExitOrigin, VEC_HULL_MIN, VEC_HULL_MAX, MASK_PLAYERSOLID, m_pVehicle, COLLISION_GROUP_NONE, &tr );
			if ( !tr.startsolid )
			{
				*pAngles = vehicleExitAngles;
				*pExitPoint = tr.endpos;
				return true;
			}
		}
	}

	// left side 
	if( CheckExitPoint( 90, 90, pExitPoint ) )	// angle from car, distance from origin, actual exit point
		return true;

	// right side
	if( CheckExitPoint( -90, 90, pExitPoint ) )
		return true;

	// front
	if( CheckExitPoint( 0, 100, pExitPoint ) )
		return true;

	// back
	if( CheckExitPoint( 180, 170, pExitPoint ) )
		return true;

	// All else failed, try popping them out the top.
	Vector vecWorldMins, vecWorldMaxs;
	m_pVehicle->CollisionProp()->WorldSpaceAABB( &vecWorldMins, &vecWorldMaxs );
	pExitPoint->x = (vecWorldMins.x + vecWorldMaxs.x) * 0.5f;
	pExitPoint->y = (vecWorldMins.y + vecWorldMaxs.y) * 0.5f;
	pExitPoint->z = vecWorldMaxs.z + 50.0f;

	// Make sure it's clear
	trace_t tr;
	UTIL_TraceHull( m_pVehicle->CollisionProp()->WorldSpaceCenter(), *pExitPoint, VEC_HULL_MIN, VEC_HULL_MAX, MASK_PLAYERSOLID, m_pVehicle, COLLISION_GROUP_NONE, &tr );
	if ( !tr.startsolid )
	{
		return true;
	}

	// No clear exit point available!
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::ParseExitAnim( KeyValues *pkvExitList, bool bEscapeExit )
{
	// Look through the entry animations list
	KeyValues *pkvExitAnim = pkvExitList->GetFirstSubKey();
	while ( pkvExitAnim )
	{
		// Add 'em to our list
		int iIndex = m_ExitAnimations.AddToTail();
		Q_strncpy( m_ExitAnimations[iIndex].szAnimName, pkvExitAnim->GetName(), sizeof(m_ExitAnimations[iIndex].szAnimName) );
		m_ExitAnimations[iIndex].bEscapeExit = bEscapeExit;
		if ( !Q_strncmp( pkvExitAnim->GetString(), "upsidedown", 10 ) )
		{
			m_ExitAnimations[iIndex].bUpright = false;
		}
		else 
		{
			m_ExitAnimations[iIndex].bUpright = true;
		}

		CBaseAnimating *pAnimating = (CBaseAnimating *)m_pVehicle;
		m_ExitAnimations[iIndex].iAttachment = pAnimating->LookupAttachment( m_ExitAnimations[iIndex].szAnimName );

		pkvExitAnim = pkvExitAnim->GetNextKey();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::ParseEntryExitAnims( void )
{
	// Try and find the right animation to play in the model's keyvalues
	KeyValues *modelKeyValues = new KeyValues("");
	if ( modelKeyValues->LoadFromBuffer( modelinfo->GetModelName( m_pVehicle->GetModel() ), modelinfo->GetModelKeyValueText( m_pVehicle->GetModel() ) ) )
	{
		// Do we have an entry section?
		KeyValues *pkvEntryList = modelKeyValues->FindKey("vehicle_entry");
		if ( pkvEntryList )
		{
			// Look through the entry animations list
			KeyValues *pkvEntryAnim = pkvEntryList->GetFirstSubKey();
			while ( pkvEntryAnim )
			{
				// Add 'em to our list
				int iIndex = m_EntryAnimations.AddToTail();
				Q_strncpy( m_EntryAnimations[iIndex].szAnimName, pkvEntryAnim->GetName(), sizeof(m_EntryAnimations[iIndex].szAnimName) );
				m_EntryAnimations[iIndex].iHitboxGroup = pkvEntryAnim->GetInt();

				pkvEntryAnim = pkvEntryAnim->GetNextKey();
			}
		}

		// Do we have an exit section?
		KeyValues *pkvExitList = modelKeyValues->FindKey("vehicle_exit");
		if ( pkvExitList )
		{
			ParseExitAnim( pkvExitList, false );
		}

		// Do we have an exit section?
		pkvExitList = modelKeyValues->FindKey("vehicle_escape_exit");
		if ( pkvExitList )
		{
			ParseExitAnim( pkvExitList, true );
		}
	}

	modelKeyValues->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::HandlePassengerEntry( CBasePlayer *pPlayer, bool bAllowEntryOutsideZone )
{
	CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating *>(m_pVehicle);
	if ( !pAnimating )
		return;

	// Find out which hitbox the player's eyepoint is within
	int iEntryAnim = GetEntryAnimForPoint( pPlayer->EyePosition() );

	// Are we in an entrypoint zone? 
	if ( iEntryAnim == ACTIVITY_NOT_AVAILABLE )
	{
		// Normal get in refuses to allow entry
		if ( !bAllowEntryOutsideZone )
			return;

		// We failed to find a valid entry anim, but we've got to get back in because the player's
		// got stuck exiting the vehicle. For now, just use the first get in anim
		// UNDONE: We need a better solution for this.
		iEntryAnim = pAnimating->LookupSequence( m_EntryAnimations[0].szAnimName );
	}

	// Check to see if this vehicle can be controlled or if it's locked
	if ( GetDrivableVehicle()->CanEnterVehicle(pPlayer) )
	{
		pPlayer->GetInVehicle( this, VEHICLE_DRIVER);

		// Setup the "enter" vehicle sequence and skip the animation if it isn't present.
		pAnimating->SetCycle( 0 );
		pAnimating->m_flAnimTime = gpGlobals->curtime;
		pAnimating->ResetSequence( iEntryAnim );
		pAnimating->ResetClientsideFrame();
		GetDrivableVehicle()->SetVehicleEntryAnim( true );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseServerVehicle::HandlePassengerExit( CBasePlayer *pPlayer )
{
	// Clear hud hints
	UTIL_HudHintText( pPlayer, "" );

	vbs_sound_update_t params;
	InitSoundParams(params);
	params.bExitVehicle = true;
	SoundState_Update( params );

	// Find the right exit anim to use based on available exit points.
	Vector vecExitPoint;
	bool bAllPointsBlocked;
	int iSequence = GetExitAnimToUse( vecExitPoint, bAllPointsBlocked );

	// If all exit points were blocked and this vehicle doesn't allow exiting in
	// these cases, bail.
	Vector vecNewPos = pPlayer->GetAbsOrigin();
	QAngle angNewAngles = pPlayer->GetAbsAngles();

	int nRole = GetPassengerRole( pPlayer );
	if ( ( bAllPointsBlocked ) || ( iSequence == ACTIVITY_NOT_AVAILABLE ) )
	{
		// Animation-driven exit points are all blocked, or we have none. Fall back to the more simple static exit points.
		if ( !GetPassengerExitPoint( nRole, &vecNewPos, &angNewAngles ) && !GetDrivableVehicle()->AllowBlockedExit( pPlayer, nRole ) )
		{
			return false;
		}
	}

	// Now we either have an exit sequence to play, a valid static exit position, or we don't care
	// whether we're blocked or not. We're getting out, one way or another.
	GetDrivableVehicle()->PreExitVehicle( pPlayer, nRole );

	if ( iSequence > ACTIVITY_NOT_AVAILABLE )
	{
		CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating *>(m_pVehicle);
		if ( pAnimating )
		{
			pAnimating->SetCycle( 0 );
			pAnimating->m_flAnimTime = gpGlobals->curtime;
			pAnimating->ResetSequence( iSequence );
			pAnimating->ResetClientsideFrame();
			GetDrivableVehicle()->SetVehicleExitAnim( true, vecExitPoint );

			// Re-deploy our weapon
			if ( pPlayer && pPlayer->IsAlive() )
			{
				if ( pPlayer->GetActiveWeapon() )
				{
					pPlayer->GetActiveWeapon()->Deploy();
					pPlayer->ShowCrosshair( true );
				}
			}

			// To prevent anything moving into the volume the player's going to occupy at the end of the exit
			// NOTE: Set the player as the blocker's owner so the player is allowed to intersect it
			Vector vecExitFeetPoint = vecExitPoint - VEC_VIEW;
			m_hExitBlocker = CEntityBlocker::Create( vecExitFeetPoint, VEC_HULL_MIN, VEC_HULL_MAX, pPlayer, true );
			return true;
		}
	}

	// Couldn't find an animation, so exit immediately
	pPlayer->LeaveVehicle( vecNewPos, angNewAngles );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CBaseServerVehicle::GetEntryAnimForPoint( const Vector &vecEyePoint )
{
	// Parse the vehicle animations the first time they get in the vehicle
	if ( !m_bParsedAnimations )
	{
		// Load the entry/exit animations from the vehicle
		ParseEntryExitAnims();
		m_bParsedAnimations = true;
	}

	// No entry anims? Vehicles with no entry anims are always enterable.
	if ( !m_EntryAnimations.Count() )
		return 0;

	// Figure out which entrypoint hitbox the player is in
	CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating *>(m_pVehicle);
	if ( !pAnimating )
		return 0;

	studiohdr_t *pStudioHdr = pAnimating->GetModelPtr();
	if (!pStudioHdr)
		return 0;
	int iHitboxSet = FindHitboxSetByName( pStudioHdr, "entryboxes" );
	mstudiohitboxset_t *set = pStudioHdr->pHitboxSet( iHitboxSet );
	if ( !set || !set->numhitboxes )
		return 0;

	// Loop through the hitboxes and find out which one we're in
	for ( int i = 0; i < set->numhitboxes; i++ )
	{
		mstudiobbox_t *pbox = set->pHitbox( i );

		Vector vecPosition;
		QAngle vecAngles;
		pAnimating->GetBonePosition( pbox->bone, vecPosition, vecAngles );

		// Build a rotation matrix from orientation
		matrix3x4_t fRotateMatrix;
		AngleMatrix( vecAngles, vecPosition, fRotateMatrix);

		Vector localEyePoint;
		VectorITransform( vecEyePoint, fRotateMatrix, localEyePoint );
		if ( IsPointInBox( localEyePoint, pbox->bbmin, pbox->bbmax ) )
		{
			// Find the entry animation for this hitbox
			int iCount = m_EntryAnimations.Count();
			for ( int entry = 0; entry < iCount; entry++ )
			{
				if ( m_EntryAnimations[entry].iHitboxGroup == pbox->group )
				{
					// Get the sequence for the animation
					return pAnimating->LookupSequence( m_EntryAnimations[entry].szAnimName );
				}
			}
		}
	}

	// Fail
	return ACTIVITY_NOT_AVAILABLE;
}

//-----------------------------------------------------------------------------
// Purpose: Find an exit animation that'll get the player to a valid position
// Input  : vecEyeExitEndpoint - Returns with the final eye position after exiting.
//			bAllPointsBlocked - Returns whether all exit points were found to be blocked.
// Output : 
//-----------------------------------------------------------------------------
int CBaseServerVehicle::GetExitAnimToUse( Vector &vecEyeExitEndpoint, bool &bAllPointsBlocked )
{
	bAllPointsBlocked = false;
	
	// Parse the vehicle animations the first time they get in the vehicle
	if ( !m_bParsedAnimations )
	{
		// Load the entry/exit animations from the vehicle
		ParseEntryExitAnims();
		m_bParsedAnimations = true;
	}

	// No exit anims? 
	if ( !m_ExitAnimations.Count() )
		return ACTIVITY_NOT_AVAILABLE;

	// Figure out which entrypoint hitbox the player is in
	CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating *>(m_pVehicle);
	if ( !pAnimating )
		return ACTIVITY_NOT_AVAILABLE;

	studiohdr_t *pStudioHdr = pAnimating->GetModelPtr();
	if (!pStudioHdr)
		return ACTIVITY_NOT_AVAILABLE;

	bool bUpright = IsVehicleUpright();

	// Loop through the exit animations and find one that ends in a clear position
	// Also attempt to choose the animation which brings you closest to your view direction.
	CBasePlayer *pPlayer = dynamic_cast<CBasePlayer *>(GetDriver());
	if (!pPlayer)
		return ACTIVITY_NOT_AVAILABLE;

	int nRole = GetPassengerRole( pPlayer );

	int nBestExitAnim = -1;
	bool bBestExitIsEscapePoint = true;
	Vector vecViewDirection, vecViewOrigin, vecBestExitPoint( 0, 0, 0 );
	vecViewOrigin = pPlayer->EyePosition();
	pPlayer->EyeVectors( &vecViewDirection );
	vecViewDirection.z = 0.0f;
	VectorNormalize( vecViewDirection );

	float flMaxCosAngleDelta = -2.0f;

	int iCount = m_ExitAnimations.Count();
	for ( int i = 0; i < iCount; i++ )
	{
		if ( m_ExitAnimations[i].bUpright != bUpright )
			continue;

		// Don't use an escape point if we found a non-escape point already
		if ( !bBestExitIsEscapePoint && m_ExitAnimations[i].bEscapeExit )
			continue;

		trace_t tr;
		Vector vehicleExitOrigin;
		QAngle vehicleExitAngles;

		// Ensure the endpoint is clear by dropping a point down from above
		pAnimating->GetAttachment( m_ExitAnimations[i].iAttachment, vehicleExitOrigin, vehicleExitAngles );

		// Don't bother checking points which are farther from our view direction.
		Vector vecDelta;
		VectorSubtract( vehicleExitOrigin, vecViewOrigin, vecDelta );
		vecDelta.z = 0.0f;
		VectorNormalize( vecDelta );
		float flCosAngleDelta = DotProduct( vecDelta, vecViewDirection );

		// But always check non-escape exits if our current best exit is an escape exit.
		if ( !bBestExitIsEscapePoint || m_ExitAnimations[i].bEscapeExit )
		{
			if ( flCosAngleDelta < flMaxCosAngleDelta )
				continue;
		}

		// The attachment points are where the driver's eyes will end up, so we subtract the view offset
		// to get the actual exit position.
		vehicleExitOrigin -= VEC_VIEW;

		Vector vecMove(0,0,64);
		Vector vecStart = vehicleExitOrigin + vecMove;
		Vector vecEnd = vehicleExitOrigin - vecMove;

		// First try a zero-length ray to check for being partially stuck in displacement surface.
	  	UTIL_TraceHull( vecStart, vecStart, VEC_HULL_MIN, VEC_HULL_MAX, MASK_PLAYERSOLID, NULL, COLLISION_GROUP_NONE, &tr );
		if ( !tr.startsolid )
		{
			// Now trace down to find the ground (or water), where we will put the player.
	  		UTIL_TraceHull( vecStart, vecEnd, VEC_HULL_MIN, VEC_HULL_MAX, MASK_WATER | MASK_PLAYERSOLID, NULL, COLLISION_GROUP_NONE, &tr );
		}

		if ( tr.startsolid )
		{
			// Started in solid, try again starting at the exit point itself (might be under an overhang).
			vecStart = vehicleExitOrigin;

			// First try a zero-length ray to check for being partially stuck in displacement surface.
		  	UTIL_TraceHull( vecStart, vecStart, VEC_HULL_MIN, VEC_HULL_MAX, MASK_PLAYERSOLID, NULL, COLLISION_GROUP_NONE, &tr );
			if ( !tr.startsolid )
			{
				// Now trace down to find the ground (or water), where we will put the player.
			  	UTIL_TraceHull( vecStart, vecEnd, VEC_HULL_MIN, VEC_HULL_MAX, MASK_WATER | MASK_PLAYERSOLID, NULL, COLLISION_GROUP_NONE, &tr );
			}

			if ( g_debug_vehicleexit.GetBool() )
			{
				NDebugOverlay::Box( vecStart, VEC_HULL_MIN, VEC_HULL_MAX, 0,255,0, 8, 10 );
				NDebugOverlay::Box( vecEnd, VEC_HULL_MIN, VEC_HULL_MAX, 255,255,255, 8, 10 );
			}
		}
		else if ( g_debug_vehicleexit.GetBool() )
		{
			NDebugOverlay::Box( vecStart, VEC_HULL_MIN, VEC_HULL_MAX, 0,255,0, 8, 10 );
			NDebugOverlay::Box( vecEnd, VEC_HULL_MIN, VEC_HULL_MAX, 255,255,255, 8, 10 );
		}

		// Disallow exits at blocked exit points or where we can't find the ground below the exit point,
		// so that we don't exit off of a cliff (although some vehicles allow this).
		if ( tr.startsolid || ( ( tr.fraction == 1.0 ) && !GetDrivableVehicle()->AllowMidairExit( pPlayer, nRole ) ) )
		{
			if ( g_debug_vehicleexit.GetBool() )
			{
				NDebugOverlay::Box( tr.endpos, VEC_HULL_MIN, VEC_HULL_MAX, 255,0,0, 64, 10 );
			}
			continue;
		}

		// Calculate the exit endpoint & viewpoint
		Vector vecExitEndPoint;
		VectorLerp( vecStart, vecEnd, tr.fraction, vecExitEndPoint );

		// Make sure we can trace to the center of the exit point
		UTIL_TraceLine( vecViewOrigin, vecExitEndPoint, MASK_PLAYERSOLID, pAnimating, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction != 1.0 )
		{
			if ( g_debug_vehicleexit.GetBool() )
			{
				NDebugOverlay::Line( vecViewOrigin, vecExitEndPoint, 255,0,0, true, 10 );
			}
			continue;
		}

		bBestExitIsEscapePoint = m_ExitAnimations[i].bEscapeExit;
		vecBestExitPoint = vecExitEndPoint;
		nBestExitAnim = i;
		flMaxCosAngleDelta = flCosAngleDelta;
	}

	if ( nBestExitAnim >= 0 )
	{
		m_vecCurrentExitEndPoint = vecBestExitPoint;

		if ( g_debug_vehicleexit.GetBool() )
		{
			NDebugOverlay::Cross3D( m_vecCurrentExitEndPoint, 16, 0, 255, 0, true, 10 );
			NDebugOverlay::Box( m_vecCurrentExitEndPoint, VEC_HULL_MIN, VEC_HULL_MAX, 255,255,255, 8, 10 );
		}

		vecEyeExitEndpoint = vecBestExitPoint + VEC_VIEW;
		m_iCurrentExitAnim = nBestExitAnim;
		return pAnimating->LookupSequence( m_ExitAnimations[m_iCurrentExitAnim].szAnimName );
	}

	// Fail, all exit points were blocked.
	bAllPointsBlocked = true;
	return ACTIVITY_NOT_AVAILABLE;	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::HandleEntryExitFinish( bool bExitAnimOn, bool bResetAnim )
{
	// Parse the vehicle animations. This is needed because they may have 
	// saved, and loaded during exit anim, which would clear the exit anim.
	if ( !m_bParsedAnimations )
	{
		// Load the entry/exit animations from the vehicle
		ParseEntryExitAnims();
		m_bParsedAnimations = true;
	}

	// Figure out which entrypoint hitbox the player is in
	CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating *>(m_pVehicle);
	if ( !pAnimating )
		return;
		
	// Did the entry anim just finish?
	if ( bExitAnimOn )
	{
		// The exit animation just finished
		CBasePlayer *pPlayer = dynamic_cast<CBasePlayer *>(GetDriver());
		if ( pPlayer )
		{
			Vector vecEyes;
			QAngle vecEyeAng;
			if ( m_iCurrentExitAnim >= 0 && m_iCurrentExitAnim < m_ExitAnimations.Count() )
			{
				pAnimating->GetAttachment( m_ExitAnimations[m_iCurrentExitAnim].szAnimName, vecEyes, vecEyeAng );

				// Use the endpoint we figured out when we exited
				vecEyes = m_vecCurrentExitEndPoint;
			}
			else
			{
				pAnimating->GetAttachment( "vehicle_driver_eyes", vecEyes, vecEyeAng );
			}

			if ( g_debug_vehicleexit.GetBool() )
			{
				NDebugOverlay::Box( vecEyes, -Vector(2,2,2), Vector(2,2,2), 255,0,0, 64, 10.0 );
			}

			// If the end point isn't clear, get back in the vehicle
			/*
			trace_t tr;
			UTIL_TraceHull( vecEyes, vecEyes, VEC_HULL_MIN, VEC_HULL_MAX, MASK_SOLID, NULL, COLLISION_GROUP_NONE, &tr );
			if ( tr.startsolid && tr.fraction < 1.0 )
			{
				pPlayer->LeaveVehicle( vecEyes, vecEyeAng );
				m_pVehicle->Use( pPlayer, pPlayer, USE_TOGGLE, 1 );
				return;
			}
			*/

			pPlayer->LeaveVehicle( vecEyes, vecEyeAng );
		}
	}

	// Only reset the animation if we're told to
	if ( bResetAnim )
	{
		// Start the vehicle idling again
		int iSequence = pAnimating->SelectWeightedSequence( ACT_IDLE );
		if ( iSequence > ACTIVITY_NOT_AVAILABLE )
		{
			pAnimating->SetCycle( 0 );
			pAnimating->m_flAnimTime = gpGlobals->curtime;
			pAnimating->ResetSequence( iSequence );
			pAnimating->ResetClientsideFrame();
		}
	}

	GetDrivableVehicle()->SetVehicleEntryAnim( false );
	GetDrivableVehicle()->SetVehicleExitAnim( false, vec3_origin );
}

//-----------------------------------------------------------------------------
// Purpose: Where does the passenger see from?
//-----------------------------------------------------------------------------
void CBaseServerVehicle::GetVehicleViewPosition( int nRole, Vector *pAbsOrigin, QAngle *pAbsAngles )
{
	Assert( nRole == VEHICLE_DRIVER );
	CBasePlayer *pPlayer = GetPassenger( VEHICLE_DRIVER );
	Assert( pPlayer );

	*pAbsAngles = pPlayer->EyeAngles();
	*pAbsOrigin = m_pVehicle->GetAbsOrigin();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::SetupMove( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move )
{
	GetDrivableVehicle()->SetupMove( player, ucmd, pHelper, move );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMoveData )
{
	GetDrivableVehicle()->ProcessMovement( pPlayer, pMoveData );

	trace_t	tr;
	UTIL_TraceLine( pPlayer->GetAbsOrigin(), pPlayer->GetAbsOrigin() - Vector( 0, 0, 256 ), MASK_PLAYERSOLID, GetVehicleEnt(), COLLISION_GROUP_NONE, &tr );

	// If our gamematerial has changed, tell any player surface triggers that are watching
	IPhysicsSurfaceProps *physprops = MoveHelper()->GetSurfaceProps();
	surfacedata_t *pSurfaceProp = physprops->GetSurfaceData( tr.surface.surfaceProps );
	char cCurrGameMaterial = pSurfaceProp->game.material;

	// Changed?
	if ( m_chPreviousTextureType != cCurrGameMaterial )
	{
		CEnvPlayerSurfaceTrigger::SetPlayerSurface( pPlayer, cCurrGameMaterial );
	}

	m_chPreviousTextureType = cCurrGameMaterial;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::FinishMove( CBasePlayer *player, CUserCmd *ucmd, CMoveData *move )
{
	GetDrivableVehicle()->FinishMove( player, ucmd, move );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::ItemPostFrame( CBasePlayer *player )
{
	Assert( player == GetDriver() );

	GetDrivableVehicle()->ItemPostFrame( player );

	if ( player->m_afButtonPressed & IN_USE )
	{
		if ( GetDrivableVehicle()->CanExitVehicle(player) )
		{
			if ( !HandlePassengerExit( player ) && ( player != NULL ) )
			{
				player->PlayUseDenySound();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::NPC_ThrottleForward( void )
{
	m_nNPCButtons |= IN_FORWARD;
	m_nNPCButtons &= ~IN_BACK;
	m_nNPCButtons &= ~IN_JUMP;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::NPC_ThrottleReverse( void )
{
	m_nNPCButtons |= IN_BACK;
	m_nNPCButtons &= ~IN_FORWARD;
	m_nNPCButtons &= ~IN_JUMP;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::NPC_ThrottleCenter( void )
{
	m_nNPCButtons &= ~IN_FORWARD;
	m_nNPCButtons &= ~IN_BACK;
	m_nNPCButtons &= ~IN_JUMP;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::NPC_Brake( void )
{
	m_nNPCButtons &= ~IN_FORWARD;
	m_nNPCButtons &= ~IN_BACK;
	m_nNPCButtons |= IN_JUMP;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::NPC_TurnLeft( float flDegrees )
{
	m_nNPCButtons |= IN_MOVELEFT;
	m_nNPCButtons &= ~IN_MOVERIGHT;
	m_flTurnDegrees = -flDegrees;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::NPC_TurnRight( float flDegrees )
{
	m_nNPCButtons |= IN_MOVERIGHT;
	m_nNPCButtons &= ~IN_MOVELEFT;
	m_flTurnDegrees = flDegrees;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::NPC_TurnCenter( void )
{
	m_nNPCButtons &= ~IN_MOVERIGHT;
	m_nNPCButtons &= ~IN_MOVELEFT;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::NPC_PrimaryFire( void )
{
	m_nNPCButtons |= IN_ATTACK;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::NPC_SecondaryFire( void )
{
	m_nNPCButtons |= IN_ATTACK2;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::Weapon_PrimaryRanges( float *flMinRange, float *flMaxRange )
{
	*flMinRange = 64;
	*flMaxRange = 1024;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::Weapon_SecondaryRanges( float *flMinRange, float *flMaxRange )
{
	*flMinRange = 64;
	*flMaxRange = 1024;
}

//-----------------------------------------------------------------------------
// Purpose: Return the time at which this vehicle's primary weapon can fire again
//-----------------------------------------------------------------------------
float CBaseServerVehicle::Weapon_PrimaryCanFireAt( void )
{
	return gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: Return the time at which this vehicle's secondary weapon can fire again
//-----------------------------------------------------------------------------
float CBaseServerVehicle::Weapon_SecondaryCanFireAt( void )
{
	return gpGlobals->curtime;
}

const char *pSoundStateNames[] =
{
	"SS_NONE",
	"SS_SHUTDOWN",
	"SS_SHUTDOWN_WATER",
	"SS_START_WATER",
	"SS_START_IDLE",
	"SS_IDLE",
	"SS_GEAR_0",
	"SS_GEAR_1",
	"SS_GEAR_2",
	"SS_GEAR_3",
	"SS_GEAR_4",
	"SS_SLOWDOWN",
	"SS_SLOWDOWN_HIGHSPEED",
	"SS_GEAR_0_RESUME",
	"SS_GEAR_1_RESUME",
	"SS_GEAR_2_RESUME",
	"SS_GEAR_3_RESUME",
	"SS_GEAR_4_RESUME",
	"SS_TURBO",
	"SS_REVERSE",
};


static int SoundStateIndexFromName( const char *pName )
{
	for ( int i = 0; i < SS_NUM_STATES; i++ )
	{
		Assert( i < ARRAYSIZE(pSoundStateNames) );
		if ( !strcmpi( pSoundStateNames[i], pName ) )
			return i;
	}
	return -1;
}

static const char *SoundStateNameFromIndex( int index )
{
	index = clamp(index, 0, SS_NUM_STATES-1 );
	return pSoundStateNames[index];
}

void CBaseServerVehicle::PlaySound( const char *pSound )
{
	if ( !pSound || !pSound[0] )
		return;

	if ( g_debug_vehiclesound.GetInt() )
	{
		Msg("Playing non-looping vehicle sound: %s\n", pSound );
	}
	m_pVehicle->EmitSound( pSound );
}

void CBaseServerVehicle::StopLoopingSound( float fadeTime )
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	if ( m_pStateSoundFade )
	{
		controller.SoundDestroy( m_pStateSoundFade );
		m_pStateSoundFade = NULL;
	}
	if ( m_pStateSound )
	{
		m_pStateSoundFade = m_pStateSound;
		m_pStateSound = NULL;
		controller.SoundFadeOut( m_pStateSoundFade, fadeTime, false );
	}
}

void CBaseServerVehicle::PlayLoopingSound( const char *pSoundName )
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	CPASAttenuationFilter filter( m_pVehicle );
	CSoundPatch *pNewSound = NULL;
	if ( pSoundName && pSoundName[0] )
	{
		pNewSound = controller.SoundCreate( filter, m_pVehicle->entindex(), CHAN_STATIC, pSoundName, ATTN_NORM );
	}

	if ( m_pStateSound && pNewSound && controller.SoundGetName( pNewSound ) == controller.SoundGetName( m_pStateSound ) )
	{
		// if the sound is the same, don't play this, just re-use the old one
		controller.SoundDestroy( pNewSound );
		pNewSound = m_pStateSound;
		controller.SoundChangeVolume( pNewSound, 1.0f, 0.0f );
		m_pStateSound = NULL;
	}
	else if ( g_debug_vehiclesound.GetInt() )
	{
		const char *pStopSound = m_pStateSound ?  controller.SoundGetName( m_pStateSound ).ToCStr() : "NULL";
		const char *pStartSound = pNewSound ?  controller.SoundGetName( pNewSound ).ToCStr() : "NULL";
		Msg("Stop %s, start %s\n", pStopSound, pStartSound );
	}

	StopLoopingSound();
	m_pStateSound = pNewSound;
	if ( m_pStateSound )
	{
		controller.Play( m_pStateSound, 1.0f, 100 );
	}
}

static sound_states MapGearToState( vbs_sound_update_t &params, int gear )
{
	switch( gear )
	{
	case 0: return params.bReverse ? SS_REVERSE : SS_GEAR_0;
	case 1: return SS_GEAR_1;
	case 2: return SS_GEAR_2;
	case 3: return SS_GEAR_3;
	default:case 4: return SS_GEAR_4;
	}
}

static sound_states MapGearToMidState( vbs_sound_update_t &params, int gear )
{
	switch( gear )
	{
	case 0: return params.bReverse ? SS_REVERSE : SS_GEAR_0_RESUME;
	case 1: return SS_GEAR_1_RESUME;
	case 2: return SS_GEAR_2_RESUME;
	case 3: return SS_GEAR_3_RESUME;
	default:case 4: return SS_GEAR_4_RESUME;
	}
}

bool CBaseServerVehicle::PlayCrashSound( float speed )
{
	int i;
	float delta = 0;
	float absSpeed = fabs(speed);
	float absLastSpeed = fabs(m_lastSpeed);
	if ( absLastSpeed > absSpeed )
	{
		delta = fabs(m_lastSpeed - speed);
	}

	for ( i = 0; i < m_vehicleSounds.crashSounds.Count(); i++ )
	{
		const vehicle_crashsound_t &crash = m_vehicleSounds.crashSounds[i];
		if ( !crash.gearLimit )
			continue;

		if ( m_iSoundGear <= crash.gearLimit )
		{
			if ( delta > crash.flMinDeltaSpeed && absLastSpeed > crash.flMinSpeed )
			{
				PlaySound( crash.iszCrashSound.ToCStr() );
				return true;
			}
		}
	}

	for ( i = m_vehicleSounds.crashSounds.Count()-1; i >= 0; --i )
	{
		const vehicle_crashsound_t &crash = m_vehicleSounds.crashSounds[i];
		if ( delta > crash.flMinDeltaSpeed && absLastSpeed > crash.flMinSpeed )
		{
			PlaySound( crash.iszCrashSound.ToCStr() );
			return true;
		}
	}
	return false;
}


bool CBaseServerVehicle::CheckCrash( vbs_sound_update_t &params )
{
	if ( params.bVehicleInWater )
		return false;

	bool bCrashed = PlayCrashSound( params.flWorldSpaceSpeed );
	if ( bCrashed )
	{
		if ( g_debug_vehiclesound.GetInt() )
		{
			Msg("Crashed!: speed %.2f, lastSpeed %.2f\n", params.flWorldSpaceSpeed, m_lastSpeed );
		}
	}
	m_lastSpeed = params.flWorldSpaceSpeed;
	return bCrashed;
}


sound_states CBaseServerVehicle::SoundState_ChooseState( vbs_sound_update_t &params )
{
	float timeInState = gpGlobals->curtime - m_soundStateStartTime;
	bool bInStateForMinTime = timeInState > m_vehicleSounds.minStateTime[m_soundState] ? true : false;

	sound_states stateOut = m_soundState;

	// exit overrides everything else
	if ( params.bExitVehicle )
	{
		switch ( m_soundState )
		{
		case SS_NONE:
		case SS_SHUTDOWN:
		case SS_SHUTDOWN_WATER:
			return m_soundState;
		}
		return SS_SHUTDOWN;
	}

	// check global state in states that don't mask them
	switch( m_soundState )
	{
	// global states masked for these states.
	case SS_NONE:
	case SS_START_IDLE:
	case SS_SHUTDOWN:
		break;
	case SS_START_WATER:
	case SS_SHUTDOWN_WATER:
		if ( !params.bVehicleInWater )
			return SS_START_IDLE;
		break;

	case SS_TURBO:
		if ( params.bVehicleInWater )
			return SS_SHUTDOWN_WATER;
		if ( CheckCrash(params) )
			return SS_IDLE;
		break;
	
	case SS_IDLE:
		if ( params.bVehicleInWater )
			return SS_SHUTDOWN_WATER;
		break;

	case SS_REVERSE:
	case SS_GEAR_0:
	case SS_GEAR_1:
	case SS_GEAR_2:
	case SS_GEAR_3:
	case SS_GEAR_4:
	case SS_SLOWDOWN:
	case SS_SLOWDOWN_HIGHSPEED:
	case SS_GEAR_0_RESUME:
	case SS_GEAR_1_RESUME:
	case SS_GEAR_2_RESUME:
	case SS_GEAR_3_RESUME:
	case SS_GEAR_4_RESUME:
		if ( params.bVehicleInWater )
		{
			return SS_SHUTDOWN_WATER;
		}
		if ( params.bTurbo )
		{
			return SS_TURBO;
		}
		if ( CheckCrash(params) )
			return SS_IDLE;
		break;
	}

	switch( m_soundState )
	{
	case SS_START_IDLE:
		return SS_IDLE;
	case SS_IDLE:
		if ( bInStateForMinTime && params.bThrottleDown )
		{
			if ( params.bTurbo )
				return SS_TURBO;
			return params.bReverse ? SS_REVERSE : SS_GEAR_0;
		}
		break;

	case SS_GEAR_0_RESUME:
	case SS_GEAR_0:
		if ( (bInStateForMinTime && !params.bThrottleDown) || params.bReverse )
		{
			return SS_IDLE;
		}
		if ( m_iSoundGear > 0 )
		{
			return SS_GEAR_1;
		}
		break;
	case SS_GEAR_1_RESUME:
	case SS_GEAR_1:
		if ( bInStateForMinTime )
		{
			if ( !params.bThrottleDown )
				return SS_SLOWDOWN;
		}
		if ( m_iSoundGear != 1 )
			return MapGearToState( params, m_iSoundGear);
		break;

	case SS_GEAR_2_RESUME:
	case SS_GEAR_2:
		if ( bInStateForMinTime )
		{
			if ( !params.bThrottleDown )
				return SS_SLOWDOWN;
			else if ( m_iSoundGear != 2 )
				return MapGearToState(params, m_iSoundGear);
		}
		break;

	case SS_GEAR_3_RESUME:
	case SS_GEAR_3:
		if ( bInStateForMinTime )
		{
			if ( !params.bThrottleDown )
				return SS_SLOWDOWN;
			else if ( m_iSoundGear != 3 )
				return MapGearToState(params, m_iSoundGear);
		}
		break;

	case SS_GEAR_4_RESUME:
	case SS_GEAR_4:
		if ( bInStateForMinTime && !params.bThrottleDown )
		{
			return SS_SLOWDOWN;
		}
		if ( m_iSoundGear != 4 )
		{
			return MapGearToMidState(params, m_iSoundGear);
		}
		break;
	case SS_REVERSE:
		if ( bInStateForMinTime && !params.bReverse )
		{
			return SS_SLOWDOWN;
		}
		break;

	case SS_SLOWDOWN_HIGHSPEED:
	case SS_SLOWDOWN:
		if ( params.bThrottleDown )
		{
			// map gears
			return MapGearToMidState(params, m_iSoundGear);
		}
		if ( m_iSoundGear == 0 )
		{
			return SS_IDLE;
		}
		break;

	case SS_NONE:
		stateOut = params.bVehicleInWater ? SS_START_WATER : SS_START_IDLE;
		break;
	case SS_TURBO:
		if ( bInStateForMinTime && !params.bTurbo )
		{
			return MapGearToMidState(params, m_iSoundGear);
		}
		break;
	default:
		break;
	}

	return stateOut;
}

const char *CBaseServerVehicle::StateSoundName( sound_states state )
{
	return m_vehicleSounds.iszStateSounds[state].ToCStr();
}

void CBaseServerVehicle::SoundState_OnNewState( sound_states lastState )
{
	if ( g_debug_vehiclesound.GetInt() )
	{
		int index = m_soundState;
		Msg("Switched to state: %d (%s)\n", m_soundState, SoundStateNameFromIndex(index) );
	}

	switch ( m_soundState )
	{
	case SS_SHUTDOWN:
	case SS_SHUTDOWN_WATER:
	case SS_START_WATER:
		StopLoopingSound();
		PlaySound( StateSoundName(m_soundState) );
		break;
	case SS_IDLE:
		m_lastSpeed = -1;
		PlayLoopingSound( StateSoundName(m_soundState) );
		break;
	case SS_START_IDLE:
	case SS_REVERSE:
	case SS_GEAR_0:
	case SS_GEAR_0_RESUME:
	case SS_GEAR_1:
	case SS_GEAR_1_RESUME:
	case SS_GEAR_2:
	case SS_GEAR_2_RESUME:
	case SS_GEAR_3:
	case SS_GEAR_3_RESUME:
	case SS_GEAR_4:
	case SS_GEAR_4_RESUME:
	case SS_TURBO:
		PlayLoopingSound( StateSoundName(m_soundState) );
		break;

	case SS_SLOWDOWN_HIGHSPEED:
	case SS_SLOWDOWN:
		if ( m_iSoundGear < 2 )
		{
			PlayLoopingSound( StateSoundName( SS_SLOWDOWN ) );
		}
		else
		{
			PlayLoopingSound( StateSoundName( SS_SLOWDOWN_HIGHSPEED ) );
		}
		break;
		
	default:break;
	}

	m_soundStateStartTime = gpGlobals->curtime;
}


void CBaseServerVehicle::SoundState_Update( vbs_sound_update_t &params )
{
	sound_states newState = SoundState_ChooseState( params );
	if ( newState != m_soundState )
	{
		sound_states lastState = m_soundState;
		m_soundState = newState;
		SoundState_OnNewState( lastState );
	}

	switch( m_soundState )
	{
	case SS_SHUTDOWN:
	case SS_SHUTDOWN_WATER:
	case SS_START_WATER:
	case SS_START_IDLE:
	case SS_IDLE:
	case SS_REVERSE:
	case SS_GEAR_0:
	case SS_GEAR_4:
	case SS_SLOWDOWN_HIGHSPEED:
	case SS_SLOWDOWN:
	case SS_GEAR_0_RESUME:
	case SS_GEAR_4_RESUME:
		break;
	default:break;
	}
}

void CBaseServerVehicle::InitSoundParams( vbs_sound_update_t &params )
{
	params.Defaults();
	params.bVehicleInWater = IsVehicleBodyInWater();
}

//-----------------------------------------------------------------------------
// Purpose: Vehicle Sound Start
//-----------------------------------------------------------------------------
void CBaseServerVehicle::SoundStart()
{
	m_soundState = SS_NONE;
	vbs_sound_update_t params;
	InitSoundParams(params);

	SoundState_Update( params );
}

// vehicle is starting up disabled, but in some cases you still want to play a sound
// HACK: handle those here.
void CBaseServerVehicle::SoundStartDisabled()
{
	m_soundState = SS_NONE;
	vbs_sound_update_t params;
	InitSoundParams(params);
	sound_states newState = SoundState_ChooseState( params );

	switch( newState )
	{
	case SS_START_WATER:
		PlaySound( StateSoundName(newState) );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::SoundShutdown( float flFadeTime )
{
	// Stop any looping sounds that may be running, as the following stop sound may not exist
	// and thus leave a looping sound playing after the user gets out.
	for ( int i = 0; i < NUM_SOUNDS_TO_STOP_ON_EXIT; i++ )
	{
		StopSound( g_iSoundsToStopOnExit[i] );
	}

	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	if ( m_pStateSoundFade )
	{
		controller.SoundFadeOut( m_pStateSoundFade, flFadeTime, true );
		m_pStateSoundFade = NULL;
	}
	if ( m_pStateSound )
	{
		controller.SoundFadeOut( m_pStateSound, flFadeTime, true );
		m_pStateSound = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseServerVehicle::SoundUpdate( vbs_sound_update_t &params )
{
	if ( g_debug_vehiclesound.GetInt() > 1 )
	{
		Msg("Throttle: %s, Reverse: %s\n", params.bThrottleDown?"on":"off", params.bReverse?"on":"off" );
	}

	float flCurrentSpeed = params.flCurrentSpeedFraction;
	if ( g_debug_vehiclesound.GetInt() > 1 )
	{
		Msg("CurrentSpeed: %.3f  ", flCurrentSpeed );
	}

	// Figure out our speed for the purposes of sound playing.
	// We slow the transition down a little to make the gear changes slower.
	if ( m_vehicleSounds.pGears.Count() > 0 )
	{
		if ( flCurrentSpeed > m_flSpeedPercentage )
		{
			// don't accelerate when the throttle isn't down
			if ( !params.bThrottleDown )
			{
				flCurrentSpeed = m_flSpeedPercentage;
			}
			flCurrentSpeed = Approach( flCurrentSpeed, m_flSpeedPercentage, params.flFrameTime * m_vehicleSounds.pGears[m_iSoundGear].flSpeedApproachFactor );
		}
	}
	m_flSpeedPercentage = clamp( flCurrentSpeed, 0.0f, 1.0f );

	if ( g_debug_vehiclesound.GetInt() > 1 )
	{
		Msg("Sound Speed: %.3f\n", m_flSpeedPercentage );
	}

	// Only do gear changes when the throttle's down
	RecalculateSoundGear( params );

	SoundState_Update( params );
}

//-----------------------------------------------------------------------------
// Purpose: Play a non-gear based vehicle sound
//-----------------------------------------------------------------------------
void CBaseServerVehicle::PlaySound( vehiclesound iSound )
{
	if ( m_vehicleSounds.iszSound[iSound] != NULL_STRING )
	{
		CPASAttenuationFilter filter( m_pVehicle );

		EmitSound_t ep;
		ep.m_nChannel = CHAN_VOICE;
		ep.m_pSoundName = STRING(m_vehicleSounds.iszSound[iSound]);
		ep.m_flVolume = m_flVehicleVolume;
		ep.m_SoundLevel = SNDLVL_NORM;

		CBaseEntity::EmitSound( filter, m_pVehicle->entindex(), ep );
		if ( g_debug_vehiclesound.GetInt() )
		{
			Msg("Playing vehicle sound: %s\n", ep.m_pSoundName );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Stop a non-gear based vehicle sound
//-----------------------------------------------------------------------------
void CBaseServerVehicle::StopSound( vehiclesound iSound )
{
	if ( m_vehicleSounds.iszSound[iSound] != NULL_STRING )
	{
		CBaseEntity::StopSound( m_pVehicle->entindex(), CHAN_VOICE, STRING(m_vehicleSounds.iszSound[iSound]) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Calculate the gear we should be in based upon the vehicle's current speed
//-----------------------------------------------------------------------------
void CBaseServerVehicle::RecalculateSoundGear( vbs_sound_update_t &params )
{
	int iNumGears = m_vehicleSounds.pGears.Count();
	for ( int i = (iNumGears-1); i >= 0; i-- )
	{
		if ( m_flSpeedPercentage > m_vehicleSounds.pGears[i].flMinSpeed )
		{
			m_iSoundGear = i;
			break;
		}
	}

	// If we're going in reverse, we want to stay in first gear
	if ( params.bReverse )
	{
		m_iSoundGear = 0;
	}
}

//===========================================================================================================
// Vehicle Sounds
//===========================================================================================================
// These are sounds that are to be automatically stopped whenever the vehicle's driver leaves it
vehiclesound g_iSoundsToStopOnExit[] =
{
	VS_ENGINE2_START,
	VS_ENGINE2_STOP,
};

char *vehiclesound_parsenames[VS_NUM_SOUNDS] =
{
	"skid_lowfriction",
	"skid_normalfriction",
	"skid_highfriction",
	"engine2_start",
	"engine2_stop",
	"misc1",
	"misc2",
	"misc3",
	"misc4",
};

CVehicleSoundsParser::CVehicleSoundsParser( void )
{
	// UNDONE: Revisit this pattern - move sub-block processing ideas into the parser architecture
	m_iCurrentGear = -1;
	m_iCurrentState = -1;
	m_iCurrentCrashSound = -1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CVehicleSoundsParser::ParseKeyValue( void *pData, const char *pKey, const char *pValue )
{
	vehiclesounds_t *pSounds = (vehiclesounds_t *)pData;
	// New gear?
	if ( !strcmpi( pKey, "gear" ) )
	{
		// Create, initialize, and add a new gear to our list
		int iNewGear = pSounds->pGears.AddToTail();
		pSounds->pGears[iNewGear].flMaxSpeed = 0;
		pSounds->pGears[iNewGear].flSpeedApproachFactor = 1.0;

		// Set our min speed to the previous gear's max
		if ( iNewGear == 0 )
		{
			// First gear, so our minspeed is 0
			pSounds->pGears[iNewGear].flMinSpeed = 0;
		}
		else
		{
			pSounds->pGears[iNewGear].flMinSpeed = pSounds->pGears[iNewGear-1].flMaxSpeed;
		}

		// Remember which gear we're reading data from
		m_iCurrentGear = iNewGear;
	}
	else if ( !strcmpi( pKey, "state" ) )
	{
		m_iCurrentState = 0;
	}
	else if ( !strcmpi( pKey, "crashsound" ) )
	{
		m_iCurrentCrashSound = pSounds->crashSounds.AddToTail();
		pSounds->crashSounds[m_iCurrentCrashSound].flMinSpeed = 0;
		pSounds->crashSounds[m_iCurrentCrashSound].flMinDeltaSpeed = 0;
		pSounds->crashSounds[m_iCurrentCrashSound].iszCrashSound = NULL_STRING;
	}
	else
	{
		int i;

		// Are we currently in a gear block?
		if ( m_iCurrentGear >= 0 )
		{
			Assert( m_iCurrentGear < pSounds->pGears.Count() );

			// Check gear keys
			if ( !strcmpi( pKey, "max_speed" ) )
			{
				pSounds->pGears[m_iCurrentGear].flMaxSpeed = atof(pValue);
				return;
			}
			if ( !strcmpi( pKey, "speed_approach_factor" ) )
			{
				pSounds->pGears[m_iCurrentGear].flSpeedApproachFactor = atof(pValue);
				return;
			}
		}
		// We're done reading a gear, so stop checking them.
		m_iCurrentGear = -1;

		if ( m_iCurrentState >= 0 )
		{
			if ( !strcmpi( pKey, "name" ) )
			{
				m_iCurrentState = SoundStateIndexFromName( pValue );
				pSounds->iszStateSounds[m_iCurrentState] = NULL_STRING;
				pSounds->minStateTime[m_iCurrentState] = 0.0f;
				return;
			}
			else if ( !strcmpi( pKey, "sound" ) )
			{
				pSounds->iszStateSounds[m_iCurrentState] = AllocPooledString(pValue);
				return;
			}
			else if ( !strcmpi( pKey, "min_time" ) )
			{
				pSounds->minStateTime[m_iCurrentState] = atof(pValue);
				return;
			}
		}
		// 
		m_iCurrentState = -1;

		if ( m_iCurrentCrashSound >= 0 )
		{
			if ( !strcmpi( pKey, "min_speed" ) )
			{
				pSounds->crashSounds[m_iCurrentCrashSound].flMinSpeed = atof(pValue);
				return;
			}
			else if ( !strcmpi( pKey, "sound" ) )
			{
				pSounds->crashSounds[m_iCurrentCrashSound].iszCrashSound = AllocPooledString(pValue);
				return;
			}
			else if ( !strcmpi( pKey, "min_speed_change" ) )
			{
				pSounds->crashSounds[m_iCurrentCrashSound].flMinDeltaSpeed = atof(pValue);
				return;
			}
			else if ( !strcmpi( pKey, "gear_limit" ) )
			{
				pSounds->crashSounds[m_iCurrentCrashSound].gearLimit = atoi(pValue);
				return;
			}
		}
		m_iCurrentCrashSound = -1;

		for ( i = 0; i < VS_NUM_SOUNDS; i++ )
		{
			if ( !strcmpi( pKey, vehiclesound_parsenames[i] ) )
			{
				pSounds->iszSound[i] = AllocPooledString(pValue);
				return;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CVehicleSoundsParser::SetDefaults( void *pData ) 
{
	vehiclesounds_t *pSounds = (vehiclesounds_t *)pData;
	for ( int i = 0; i < VS_NUM_SOUNDS; i++ )
	{
		pSounds->iszSound[i] = NULL_STRING;
	}

	pSounds->pGears.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: Parse just the vehicle_sound section of a vehicle file.
//-----------------------------------------------------------------------------
void CVehicleSoundsParser::ParseVehicleSounds( const char *pScriptName, vehiclesounds_t *pSounds )
{
	byte *pFile = UTIL_LoadFileForMe( pScriptName, NULL );
	if ( !pFile )
		return;

	IVPhysicsKeyParser *pParse = physcollision->VPhysicsKeyParserCreate( (char *)pFile );
	while ( !pParse->Finished() )
	{
		const char *pBlock = pParse->GetCurrentBlockName();
		if ( !strcmpi( pBlock, "vehicle_sounds" ) )
		{
			pParse->ParseCustom( &pSounds, this );
		}
		else
		{
			pParse->SkipBlock();
		}
	}
	physcollision->VPhysicsKeyParserDestroy( pParse );

	UTIL_FreeFile( pFile );
}
