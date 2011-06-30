//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "mathlib.h"
#include "util_shared.h"
#include "model_types.h"
#include "convar.h"
#include "IEffects.h"
#include "vphysics/object_hash.h"
#include "IceKey.H"
#include "checksum_crc.h"
#include "ff_triggerclip.h"
#include "ff_gamerules.h"

#ifdef CLIENT_DLL
	#include "c_ff_team.h"
	#include "c_ff_player.h"
	#include "c_te_effect_dispatch.h"
#else
	#include "ff_team.h"
	#include "ff_player.h"
	#include "te_effect_dispatch.h"
	#include "ff_item_flag.h"

bool NPC_CheckBrushExclude( CBaseEntity *pEntity, CBaseEntity *pBrush );
#endif


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar r_visualizetraces( "r_visualizetraces", "0", FCVAR_CHEAT );
ConVar developer("developer", "0", 0, "Set developer message level" ); // developer mode

//Adding ConVars to toggle grenades/pipes colliding with the enemy
ConVar ffdev_grenade_collidewithenemy("ffdev_grenade_collidewithenemy", "1", FCVAR_FF_FFDEV_REPLICATED, "If set to 0, grenades pass through enemies" );

ConVar ffdev_softclip_asdisguisedteam("ffdev_softclip_asdisguisedteam", "0", FCVAR_FF_FFDEV_REPLICATED, "If set to 1, spies soft clip as though they were on their disguised team");

float UTIL_VecToYaw( const Vector &vec )
{
	if (vec.y == 0 && vec.x == 0)
		return 0;
	
	float yaw = atan2( vec.y, vec.x );

	yaw = RAD2DEG(yaw);

	if (yaw < 0)
		yaw += 360;

	return yaw;
}


float UTIL_VecToPitch( const Vector &vec )
{
	if (vec.y == 0 && vec.x == 0)
	{
		if (vec.z < 0)
			return 180.0;
		else
			return -180.0;
	}

	float dist = vec.Length2D();
	float pitch = atan2( -vec.z, dist );

	pitch = RAD2DEG(pitch);

	return pitch;
}

float UTIL_VecToYaw( const matrix3x4_t &matrix, const Vector &vec )
{
	Vector tmp = vec;
	VectorNormalize( tmp );

	float x = matrix[0][0] * tmp.x + matrix[1][0] * tmp.y + matrix[2][0] * tmp.z;
	float y = matrix[0][1] * tmp.x + matrix[1][1] * tmp.y + matrix[2][1] * tmp.z;

	if (x == 0.0f && y == 0.0f)
		return 0.0f;
	
	float yaw = atan2( -y, x );

	yaw = RAD2DEG(yaw);

	if (yaw < 0)
		yaw += 360;

	return yaw;
}


float UTIL_VecToPitch( const matrix3x4_t &matrix, const Vector &vec )
{
	Vector tmp = vec;
	VectorNormalize( tmp );

	float x = matrix[0][0] * tmp.x + matrix[1][0] * tmp.y + matrix[2][0] * tmp.z;
	float z = matrix[0][2] * tmp.x + matrix[1][2] * tmp.y + matrix[2][2] * tmp.z;

	if (x == 0.0f && z == 0.0f)
		return 0.0f;
	
	float pitch = atan2( z, x );

	pitch = RAD2DEG(pitch);

	if (pitch < 0)
		pitch += 360;

	return pitch;
}

Vector UTIL_YawToVector( float yaw )
{
	Vector ret;
	
	ret.z = 0;
	float angle = DEG2RAD( yaw );
	SinCos( angle, &ret.y, &ret.x );

	return ret;
}

//-----------------------------------------------------------------------------
// Purpose: Helper function get get determinisitc random values for shared/prediction code
// Input  : seedvalue - 
//			*module - 
//			line - 
// Output : static int
//-----------------------------------------------------------------------------
static int SeedFileLineHash( int seedvalue, const char *sharedname, int additionalSeed )
{
	CRC32_t retval;

	CRC32_Init( &retval );

	CRC32_ProcessBuffer( &retval, (void *)&seedvalue, sizeof( int ) );
	CRC32_ProcessBuffer( &retval, (void *)&additionalSeed, sizeof( int ) );
	CRC32_ProcessBuffer( &retval, (void *)sharedname, Q_strlen( sharedname ) );
	
	CRC32_Final( &retval );

	return (int)( retval );
}

float SharedRandomFloat( const char *sharedname, float flMinVal, float flMaxVal, int additionalSeed /*=0*/ )
{
	Assert( CBaseEntity::GetPredictionRandomSeed() != -1 );

	int seed = SeedFileLineHash( CBaseEntity::GetPredictionRandomSeed(), sharedname, additionalSeed );
	RandomSeed( seed );
	return RandomFloat( flMinVal, flMaxVal );
}

int SharedRandomInt( const char *sharedname, int iMinVal, int iMaxVal, int additionalSeed /*=0*/ )
{
	Assert( CBaseEntity::GetPredictionRandomSeed() != -1 );

	int seed = SeedFileLineHash( CBaseEntity::GetPredictionRandomSeed(), sharedname, additionalSeed );
	RandomSeed( seed );
	return RandomInt( iMinVal, iMaxVal );
}

Vector SharedRandomVector( const char *sharedname, float minVal, float maxVal, int additionalSeed /*=0*/ )
{
	Assert( CBaseEntity::GetPredictionRandomSeed() != -1 );

	int seed = SeedFileLineHash( CBaseEntity::GetPredictionRandomSeed(), sharedname, additionalSeed );
	RandomSeed( seed );
	// HACK:  Can't call RandomVector/Angle because it uses rand() not vstlib Random*() functions!
	// Get a random vector.
	Vector random;
	random.x = RandomFloat( minVal, maxVal );
	random.y = RandomFloat( minVal, maxVal );
	random.z = RandomFloat( minVal, maxVal );
	return random;
}

QAngle SharedRandomAngle( const char *sharedname, float minVal, float maxVal, int additionalSeed /*=0*/ )
{
	Assert( CBaseEntity::GetPredictionRandomSeed() != -1 );

	int seed = SeedFileLineHash( CBaseEntity::GetPredictionRandomSeed(), sharedname, additionalSeed );
	RandomSeed( seed );

	// HACK:  Can't call RandomVector/Angle because it uses rand() not vstlib Random*() functions!
	// Get a random vector.
	Vector random;
	random.x = RandomFloat( minVal, maxVal );
	random.y = RandomFloat( minVal, maxVal );
	random.z = RandomFloat( minVal, maxVal );
	return QAngle( random.x, random.y, random.z );
}


//-----------------------------------------------------------------------------
//
// Shared client/server trace filter code
//
//-----------------------------------------------------------------------------
bool PassServerEntityFilter( const IHandleEntity *pTouch, const IHandleEntity *pPass ) 
{
	if ( !pPass )
		return true;

	if ( pTouch == pPass )
		return false;

	const CBaseEntity *pEntTouch = EntityFromEntityHandle( pTouch );
	const CBaseEntity *pEntPass = EntityFromEntityHandle( pPass );
	if ( !pEntTouch || !pEntPass )
		return true;

	// don't clip against own missiles
	if ( pEntTouch->GetOwnerEntity() == pEntPass )
		return false;
	
	// don't clip against owner
	if(!pEntPass->CanClipOwnerEntity())
	{
		if(pEntPass->GetOwnerEntity() == pEntTouch)
			return false;
	}

	if(!pEntPass->CanClipPlayer() && pEntTouch->IsPlayer())
		return false;

	return true;
}


//-----------------------------------------------------------------------------
// A standard filter to be applied to just about everything.
//-----------------------------------------------------------------------------
bool StandardFilterRules( IHandleEntity *pHandleEntity, int fContentsMask )
{
	CBaseEntity *pCollide = EntityFromEntityHandle( pHandleEntity );

	// Static prop case...
	if ( !pCollide )
		return true;

	SolidType_t solid = pCollide->GetSolid();
	const model_t *pModel = pCollide->GetModel();

	if ( ( modelinfo->GetModelType( pModel ) != mod_brush ) || (solid != SOLID_BSP && solid != SOLID_VPHYSICS) )
	{
		if ( (fContentsMask & CONTENTS_MONSTER) == 0 )
			return false;
	}

	// This code is used to cull out tests against see-thru entities
	if ( !(fContentsMask & CONTENTS_WINDOW) && pCollide->IsTransparent() )
		return false;

	// FIXME: this is to skip BSP models that are entities that can be 
	// potentially moved/deleted, similar to a monster but doors don't seem to 
	// be flagged as monsters
	// FIXME: the FL_WORLDBRUSH looked promising, but it needs to be set on 
	// everything that's actually a worldbrush and it currently isn't
	if ( !(fContentsMask & CONTENTS_MOVEABLE) && (pCollide->GetMoveType() == MOVETYPE_PUSH))// !(touch->flags & FL_WORLDBRUSH) )
		return false;

	return true;
}


//-----------------------------------------------------------------------------
// Simple trace filter
//-----------------------------------------------------------------------------
CTraceFilterSimple::CTraceFilterSimple( const IHandleEntity *passedict, int collisionGroup )
{
	m_pPassEnt = passedict;
	m_collisionGroup = collisionGroup;
}


//-----------------------------------------------------------------------------
// The trace filter!
//-----------------------------------------------------------------------------
bool CTraceFilterSimple::ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
{
	CBaseEntity *pHandle = EntityFromEntityHandle( pHandleEntity );

	const CBaseEntity *pPassEnt = NULL;
	if( m_pPassEnt )
		pPassEnt = EntityFromEntityHandle( m_pPassEnt );

	// --> hlstriker: No team collisions
	if( pPassEnt && pHandle )
	{
		if( pHandle->IsPlayer() )
		{
			////Team orient this collision group.  Includes things like rockets, and blue pipes -Green Mushy
			////Confusing: these projectiles use the interactive collision group, not the projectile collision group
			////they are things like rockets/blue pipes, or other entities that will collide with most anything
			//if( m_collisionGroup == COLLISION_GROUP_INTERACTIVE )
			//{
			//	//Make sure u dont collide the object with the owner
			//	if( ToFFPlayer(pHandle) == ToFFPlayer(pPassEnt->GetOwnerEntity()) )
			//	{
			//		return false;
			//	}
			//	//Now dont collide if the team numbers are the same
			//	else if( ToFFPlayer(pHandle)->GetTeamNumber() == ToFFPlayer(pPassEnt->GetOwnerEntity())->GetTeamNumber())
			//	{
			//		return false;
			//	}
			//	//Now collide normally after the owner and teammate checks are over
			//	else
			//	{
			//		//Do checks here for other exceptions and return true for collision
			//		return true;
			//	}
			//}

			//This collision group is for grenades and yellow pipes even though its called "projectile" 
			//They pass through the player collision group normally
			//adding an exception to collide with enemies here -Green Mushy
			if( m_collisionGroup == COLLISION_GROUP_PROJECTILE )
			{
				//Make sure u dont collide with the owner
				if( ToFFPlayer(pHandle) == ToFFPlayer(pPassEnt->GetOwnerEntity()) )
				{
					return false;
				}
				//Now check teams and dont collide if the team numbers are the same
				else if( ToFFPlayer(pHandle)->GetTeamNumber() == ToFFPlayer(pPassEnt->GetOwnerEntity())->GetTeamNumber())
				{
					return false;
				}
				//Now collide normally after the owner and teammate checks are over
				else
				{
					//Utilize the convar to toggle enemy collisions
					if( ffdev_grenade_collidewithenemy.GetBool() )
						return true;
					else
						return false;
				}
			}

			if( m_collisionGroup == COLLISION_GROUP_PLAYER_MOVEMENT )
			{
				if (pPassEnt == pHandle) 
					return false;

				// This allows players to pass through any team object (another player or entity), includes allies
				int iPassTeam = pPassEnt->GetTeamNumber();
				int iHandleTeam = pHandle->GetTeamNumber();

				// get disguised team if we should
				if (ffdev_softclip_asdisguisedteam.GetBool())
				{
					if (pPassEnt->IsPlayer())
					{
						CFFPlayer *pPassPlayer = static_cast< CFFPlayer * >( const_cast< CBaseEntity * >( pPassEnt ) );

						if (pPassPlayer && pPassPlayer->IsDisguised())
							iPassTeam = pPassPlayer->GetDisguisedTeam();
					}
					
					if (pHandle->IsPlayer())
					{
						CFFPlayer *pHandlePlayer = ToFFPlayer(pHandle);

						if (pHandlePlayer && pHandlePlayer->IsDisguised())
							iHandleTeam = pHandlePlayer->GetDisguisedTeam();
					}
				}
				
				if( pHandle->IsPlayer() && FFGameRules()->IsTeam1AlliedToTeam2( iPassTeam, iHandleTeam ) == GR_TEAMMATE )
				{
					// If player lands on top of a team entity make sure they hit
					Vector vecOrigin = pPassEnt->GetAbsOrigin();
					Vector vecTeamOrigin = pHandle->GetAbsOrigin();

					Vector vecMin;
					if( pPassEnt->IsPlayer() )
					{
						CFFPlayer *pPlayer = static_cast< CFFPlayer * >( const_cast< CBaseEntity * >( pPassEnt ) );
						vecMin = pPlayer->GetPlayerMins();
					}
					else
						vecMin = pPassEnt->WorldAlignMins();

					Vector vecTeamMax;
					if( pHandle->IsPlayer() )
					{
						CFFPlayer *pPlayer = static_cast< CFFPlayer * >( pHandle );
						vecTeamMax = pPlayer->GetPlayerMaxs();
					}
					else
						vecTeamMax = pHandle->WorldAlignMaxs();

					float fMinZ = vecMin[2];
					VectorAdd( vecMin, vecOrigin, vecMin );
					VectorAdd( vecTeamMax, vecTeamOrigin, vecTeamMax );

					// If players mins are greater than teams maxs - 2
					if( vecMin[2] >= vecTeamMax[2] )
						return true;
					if( vecMin[2] > vecTeamMax[2] - 5.0f )
					{
						// If a player is jumping through an entity to the top make sure they don't get stuck
						CBaseEntity *pEnt = const_cast< CBaseEntity * >( pPassEnt );
						pEnt->SetAbsOrigin( Vector( vecOrigin[0], vecOrigin[1], vecTeamMax[2] - fMinZ ) );
					}

					return false;
				}
			}
		}
	}
	// <--

	if( pHandle )
	{
		if( pHandle->Classify() == CLASS_TRIGGER_CLIP )
		{
			CFFTriggerClip *pTriggerClip = dynamic_cast< CFFTriggerClip * >( pHandle );
			if( pTriggerClip && pTriggerClip->GetClipMask() )
			{
				if( pPassEnt )
				{
					// Players
					if ( pPassEnt->IsPlayer() )
					{
						// Bullets from players
						if ( contentsMask == MASK_SHOT )
						{
							return ShouldFFTriggerClipBlock( pTriggerClip, pPassEnt->GetTeamNumber(), 
								LUA_CLIP_FLAG_BULLETS, LUA_CLIP_FLAG_NONPLAYERS,
								LUA_CLIP_FLAG_BULLETSBYTEAM | LUA_CLIP_FLAG_NONPLAYERSBYTEAM );
						}
						// Buildables trying to be built
						else if ( m_collisionGroup == COLLISION_GROUP_BUILDABLE_BUILDING )
						{
							return ShouldFFTriggerClipBlock( pTriggerClip, pPassEnt->GetTeamNumber(), 
								LUA_CLIP_FLAG_BUILDABLES, LUA_CLIP_FLAG_NONPLAYERS,
								LUA_CLIP_FLAG_BUILDABLESBYTEAM | LUA_CLIP_FLAG_NONPLAYERSBYTEAM );
						}

						// Players themselves
						return ShouldFFTriggerClipBlock( pTriggerClip, pPassEnt->GetTeamNumber(), 
							LUA_CLIP_FLAG_PLAYERS, 
							LUA_CLIP_FLAG_PLAYERSBYTEAM );
					}
					// Buildables
					else if ( FF_IsBuildableObject(const_cast< CBaseEntity * >( pPassEnt )) )
					{
						CFFBuildableObject *pBuildable = FF_ToBuildableObject( const_cast< CBaseEntity * >( pPassEnt ) );
						if (!pBuildable)
							return false;
						
						// Bullets from buildables
						if ( contentsMask == MASK_SHOT )
						{
							return ShouldFFTriggerClipBlock( pTriggerClip, pBuildable->GetTeamNumber(), 
								LUA_CLIP_FLAG_BUILDABLEWEAPONS, LUA_CLIP_FLAG_NONPLAYERS,
								LUA_CLIP_FLAG_BUILDABLEWEAPONSBYTEAM | LUA_CLIP_FLAG_NONPLAYERSBYTEAM );
						}

						// Buildables getting built don't send a buildable pointer (they send a player), so
						// any buildable getting sent is assumed to be doing damage of some sort
						// For sure, though, detpacks trace with MASK_SOLID for their explosion damage
						return ShouldFFTriggerClipBlock( pTriggerClip, pBuildable->GetTeamNumber(), 
							LUA_CLIP_FLAG_BUILDABLEWEAPONS, LUA_CLIP_FLAG_NONPLAYERS,
							LUA_CLIP_FLAG_BUILDABLEWEAPONSBYTEAM | LUA_CLIP_FLAG_NONPLAYERSBYTEAM );
					}
					// Grenades
					else if ( pPassEnt->GetFlags() & FL_GRENADE )
					{
						// Get owner so we can get its team number
						CBaseEntity *pOwner = pPassEnt->GetOwnerEntity();
						if( !pOwner )
							return false;

						return ShouldFFTriggerClipBlock( pTriggerClip, pOwner->GetTeamNumber(), 
							LUA_CLIP_FLAG_GRENADES, LUA_CLIP_FLAG_NONPLAYERS,
							LUA_CLIP_FLAG_GRENADESBYTEAM | LUA_CLIP_FLAG_NONPLAYERSBYTEAM );
					}
					// Backpacks
					else if ( const_cast< CBaseEntity * >( pPassEnt )->Classify() == CLASS_BACKPACK )
					{
						// Get owner so we can get its team number
						CBaseEntity *pOwner = pPassEnt->GetOwnerEntity();
						if( !pOwner )
							return false;
						
						// exploding backpack from an EMP
						if ( contentsMask == MASK_SHOT )
						{
							return ShouldFFTriggerClipBlock( pTriggerClip, pOwner->GetTeamNumber(), 
								LUA_CLIP_FLAG_GRENADES, LUA_CLIP_FLAG_NONPLAYERS,
								LUA_CLIP_FLAG_GRENADESBYTEAM | LUA_CLIP_FLAG_NONPLAYERSBYTEAM );
						}

						return ShouldFFTriggerClipBlock( pTriggerClip, pOwner->GetTeamNumber(), 
							LUA_CLIP_FLAG_BACKPACKS, LUA_CLIP_FLAG_NONPLAYERS,
							LUA_CLIP_FLAG_BACKPACKSBYTEAM | LUA_CLIP_FLAG_NONPLAYERSBYTEAM );
					}
					// Info_ff_scripts
					else if ( const_cast< CBaseEntity * >( pPassEnt )->Classify() == CLASS_INFOSCRIPT )
					{
						// Info scripts by team is a complicated mess that I can't be bothered to
						// deal with right now, so it's all-or-nothing for now - squeek
						// TODO: Block info_ff_scripts by team
						return ShouldFFTriggerClipBlock( pTriggerClip, 0, 
							LUA_CLIP_FLAG_INFOSCRIPTS, LUA_CLIP_FLAG_NONPLAYERS,
							0 /*LUA_CLIP_FLAG_INFOSCRIPTSBYTEAM | LUA_CLIP_FLAG_NONPLAYERSBYTEAM*/ );
					}
					// Respawn turrets
					else if ( const_cast< CBaseEntity * >( pPassEnt )->Classify() == CLASS_TURRET )
					{
						return ShouldFFTriggerClipBlock( pTriggerClip, pPassEnt->GetTeamNumber(), 
							LUA_CLIP_FLAG_SPAWNTURRETS, LUA_CLIP_FLAG_NONPLAYERS,
							LUA_CLIP_FLAG_SPAWNTURRETSBYTEAM | LUA_CLIP_FLAG_NONPLAYERSBYTEAM );
					}
					// Projectiles or any other player-owned entities
					else
					{
						// Get owner so we can get its team number
						CBaseEntity *pOwner = pPassEnt->GetOwnerEntity();
						if( !pOwner )
							return false;

						// Projectiles from players
						if( pOwner->IsPlayer() )
						{
							return ShouldFFTriggerClipBlock( pTriggerClip, pOwner->GetTeamNumber(), 
								LUA_CLIP_FLAG_PROJECTILES, LUA_CLIP_FLAG_NONPLAYERS,
								LUA_CLIP_FLAG_PROJECTILESBYTEAM | LUA_CLIP_FLAG_NONPLAYERSBYTEAM );
						}
						// Projectiles from buildables
						else if ( FF_IsBuildableObject(pOwner) )
						{
							return ShouldFFTriggerClipBlock( pTriggerClip, pOwner->GetTeamNumber(), 
								LUA_CLIP_FLAG_BUILDABLEWEAPONS, LUA_CLIP_FLAG_NONPLAYERS,
								LUA_CLIP_FLAG_BUILDABLEWEAPONSBYTEAM | LUA_CLIP_FLAG_NONPLAYERSBYTEAM );
						}
					}

					return false;
				}
			}
		}
	}

	if ( !StandardFilterRules( pHandleEntity, contentsMask ) )
		return false;

	if ( m_pPassEnt )
	{
		if ( !PassServerEntityFilter( pHandleEntity, m_pPassEnt ) )
			return false;
	}

	// Don't test if the game code tells us we should ignore this collision...
	if ( !pHandle->ShouldCollide( m_collisionGroup, contentsMask ) )
		return false;
	if ( pHandle && !g_pGameRules->ShouldCollide( m_collisionGroup, pHandle->GetCollisionGroup() ) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Trace filter that only hits NPCs and the player
//-----------------------------------------------------------------------------
bool CTraceFilterOnlyNPCsAndPlayer::ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
{
	if ( CTraceFilterSimple::ShouldHitEntity(pServerEntity, contentsMask) )
	{
		CBaseEntity *pEntity = EntityFromEntityHandle( pServerEntity );
#ifdef CSTRIKE_DLL
#ifndef CLIENT_DLL
		if ( pEntity->Classify() == CLASS_PLAYER_ALLY )
			return true; // CS hostages are CLASS_PLAYER_ALLY but not IsNPC()
#endif // !CLIENT_DLL
#endif // CSTRIKE_DLL
		return (pEntity->IsNPC() || pEntity->IsPlayer());
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Trace filter that only hits anything but NPCs and the player
//-----------------------------------------------------------------------------
bool CTraceFilterNoNPCsOrPlayer::ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
{
	if ( CTraceFilterSimple::ShouldHitEntity(pServerEntity, contentsMask) )
	{
		CBaseEntity *pEntity = EntityFromEntityHandle( pServerEntity );
#ifndef CLIENT_DLL
		if ( pEntity->Classify() == CLASS_PLAYER_ALLY )
			return false; // CS hostages are CLASS_PLAYER_ALLY but not IsNPC()
#endif
		return (!pEntity->IsNPC() && !pEntity->IsPlayer());
	}
	return false;
}

//-----------------------------------------------------------------------------
// Trace filter that skips two entities
//-----------------------------------------------------------------------------
CTraceFilterSkipTwoEntities::CTraceFilterSkipTwoEntities( const IHandleEntity *passentity, const IHandleEntity *passentity2, int collisionGroup ) :
	BaseClass( passentity, collisionGroup ), m_pPassEnt2(passentity2)
{
}

bool CTraceFilterSkipTwoEntities::ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
{
	Assert( pServerEntity );
	if ( !PassServerEntityFilter( pServerEntity, m_pPassEnt2 ) )
		return false;

	return BaseClass::ShouldHitEntity( pServerEntity, contentsMask );
}


//-----------------------------------------------------------------------------
// Trace filter that can take a list of entities to ignore
//-----------------------------------------------------------------------------
CTraceFilterSimpleList::CTraceFilterSimpleList( int collisionGroup ) :
	CTraceFilterSimple( NULL, collisionGroup )
{
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTraceFilterSimpleList::ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
{
	if ( m_PassEntities.Find(pHandleEntity) != m_PassEntities.InvalidIndex() )
		return false;

	return CTraceFilterSimple::ShouldHitEntity( pHandleEntity, contentsMask );
}


//-----------------------------------------------------------------------------
// Purpose: Add an entity to my list of entities to ignore in the trace
//-----------------------------------------------------------------------------
void CTraceFilterSimpleList::AddEntityToIgnore( IHandleEntity *pEntity )
{
	m_PassEntities.AddToTail( pEntity );
}


//-----------------------------------------------------------------------------
// Purpose: Custom trace filter used for NPC LOS traces
//-----------------------------------------------------------------------------
CTraceFilterLOS::CTraceFilterLOS( IHandleEntity *pHandleEntity, int collisionGroup, IHandleEntity *pHandleEntity2 ) :
		CTraceFilterSkipTwoEntities( pHandleEntity, pHandleEntity2, collisionGroup )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTraceFilterLOS::ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
{
	CBaseEntity *pEntity = (CBaseEntity *)pServerEntity;

	if ( !pEntity->BlocksLOS() )
		return false;

	return CTraceFilterSimple::ShouldHitEntity( pServerEntity, contentsMask );
}

//-----------------------------------------------------------------------------
// Sweeps against a particular model, using collision rules 
//-----------------------------------------------------------------------------
void UTIL_TraceModel( const Vector &vecStart, const Vector &vecEnd, const Vector &hullMin, 
					  const Vector &hullMax, CBaseEntity *pentModel, int collisionGroup, trace_t *ptr )
{
	// Cull it....
	if ( pentModel && pentModel->ShouldCollide( collisionGroup, MASK_ALL ) )
	{
		Ray_t ray;
		ray.Init( vecStart, vecEnd, hullMin, hullMax );
		enginetrace->ClipRayToEntity( ray, MASK_ALL, pentModel, ptr ); 
	}
	else
	{
		memset( ptr, 0, sizeof(trace_t) );
		ptr->fraction = 1.0f;
	}
}

bool UTIL_EntityHasMatchingRootParent( CBaseEntity *pRootParent, CBaseEntity *pEntity )
{
	if ( pRootParent )
	{
		// NOTE: Don't let siblings/parents collide.
		if ( pRootParent == pEntity->GetRootMoveParent() )
			return true;
		if ( pEntity->GetOwnerEntity() && pRootParent == pEntity->GetOwnerEntity()->GetRootMoveParent() )
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Sweep an entity from the starting to the ending position 
//-----------------------------------------------------------------------------
class CTraceFilterEntity : public CTraceFilterSimple
{
	DECLARE_CLASS( CTraceFilterEntity, CTraceFilterSimple );

public:
	CTraceFilterEntity( CBaseEntity *pEntity, int nCollisionGroup ) 
		: CTraceFilterSimple( pEntity, nCollisionGroup )
	{
		m_pRootParent = pEntity->GetRootMoveParent();
		m_pEntity = pEntity;
		m_checkHash = g_EntityCollisionHash->IsObjectInHash(pEntity);
	}

	bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		Assert( dynamic_cast<CBaseEntity*>(pHandleEntity) );
		CBaseEntity *pTestEntity = static_cast<CBaseEntity*>(pHandleEntity);

		// Check parents against each other
		// NOTE: Don't let siblings/parents collide.
		if ( UTIL_EntityHasMatchingRootParent( m_pRootParent, pTestEntity ) )
			return false;

		if ( m_checkHash )
		{
			if ( g_EntityCollisionHash->IsObjectPairInHash(m_pEntity, pTestEntity) )
				return false;
		}

#ifndef CLIENT_DLL
		if ( m_pEntity->IsNPC() )
		{
			if ( NPC_CheckBrushExclude( m_pEntity, pTestEntity ) == true )
				 return false;

		}
#endif

		return BaseClass::ShouldHitEntity( pHandleEntity, contentsMask );
	}

private:

	CBaseEntity *m_pRootParent;
	CBaseEntity *m_pEntity;
	bool		m_checkHash;
};

class CTraceFilterEntityIgnoreOther : public CTraceFilterEntity
{
	DECLARE_CLASS( CTraceFilterEntityIgnoreOther, CTraceFilterEntity );
public:
	CTraceFilterEntityIgnoreOther( CBaseEntity *pEntity, const IHandleEntity *pIgnore, int nCollisionGroup ) : 
		CTraceFilterEntity( pEntity, nCollisionGroup ), m_pIgnoreOther( pIgnore )
	{
	}

	bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		if ( pHandleEntity == m_pIgnoreOther )
			return false;

		return BaseClass::ShouldHitEntity( pHandleEntity, contentsMask );
	}

private:
	const IHandleEntity *m_pIgnoreOther;
};

//// Just something handy for testing
//class CTraceFilterInfoScriptEntity : public CTraceFilterSimple
//{
//	DECLARE_CLASS( CTraceFilterEntity, CTraceFilterSimple );
//
//public:
//	CTraceFilterInfoScriptEntity( CBaseEntity *pEntity, int nCollisionGroup )
//		: CTraceFilterSimple( pEntity, nCollisionGroup )
//	{
//		m_pEntity = pEntity;
//	}
//
//	bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
//	{
//#ifdef GAME_DLL
//		Assert( dynamic_cast<CBaseEntity *>( pHandleEntity ) );
//		CBaseEntity *pTestEntity = static_cast<CBaseEntity *>( pHandleEntity );
//
//		DevMsg( "Info Script hitting: %s\n", pTestEntity->GetClassname() );
//#endif
//
//		return BaseClass::ShouldHitEntity( pHandleEntity, contentsMask );
//	}
//
//private:
//	CBaseEntity *m_pEntity;
//};

//-----------------------------------------------------------------------------
// Sweeps a particular entity through the world 
//-----------------------------------------------------------------------------
void UTIL_TraceEntity( CBaseEntity *pEntity, const Vector &vecAbsStart, const Vector &vecAbsEnd, unsigned int mask, trace_t *ptr )
{
	ICollideable *pCollision = pEntity->GetCollideable();

	// Adding this assertion here so game code catches it, but really the assertion belongs in the engine
	// because one day, rotated collideables will work!
	Assert( pCollision->GetCollisionAngles() == vec3_angle );

	if( pEntity->Classify() == CLASS_INFOSCRIPT )
	{
//// Hey, this is some cool shit and shows just how often the shit is tested
//#ifdef GAME_DLL
//#include "ndebugoverlay.h"
//		static bool bFlipFlop = false;
//		if( bFlipFlop )
//			NDebugOverlay::Line( vecAbsStart, vecAbsEnd, 255, 0, 0, true, 60.0f );
//		else
//			NDebugOverlay::Line( vecAbsStart, vecAbsEnd, 0, 0, 255, true, 60.0f );
//		bFlipFlop = !bFlipFlop;
//#endif

		mask |= CONTENTS_PLAYERCLIP;

//		CTraceFilterInfoScriptEntity traceFilter( pEntity, iCollisionGroup );
//		enginetrace->SweepCollideable( pCollision, vecAbsStart, vecAbsEnd, pCollision->GetCollisionAngles(), mask, &traceFilter, ptr );

//#ifdef GAME_DLL
//		if( ptr && ptr->m_pEnt )
//		{
//			DevMsg( "Collide Ent: %s\n", ptr->m_pEnt->GetClassname() );
//		}
//#endif
	}

	CTraceFilterEntity traceFilter( pEntity, pCollision->GetCollisionGroup() );
	enginetrace->SweepCollideable( pCollision, vecAbsStart, vecAbsEnd, pCollision->GetCollisionAngles(), mask, &traceFilter, ptr );
}

void UTIL_TraceEntity( CBaseEntity *pEntity, const Vector &vecAbsStart, const Vector &vecAbsEnd, 
					  unsigned int mask, const IHandleEntity *pIgnore, int nCollisionGroup, trace_t *ptr )
{
	ICollideable *pCollision = pEntity->GetCollideable();

	// Adding this assertion here so game code catches it, but really the assertion belongs in the engine
	// because one day, rotated collideables will work!
	Assert( pCollision->GetCollisionAngles() == vec3_angle );

	CTraceFilterEntityIgnoreOther traceFilter( pEntity, pIgnore, nCollisionGroup );
	enginetrace->SweepCollideable( pCollision, vecAbsStart, vecAbsEnd, pCollision->GetCollisionAngles(), mask, &traceFilter, ptr );
}

void UTIL_TraceEntity( CBaseEntity *pEntity, const Vector &vecAbsStart, const Vector &vecAbsEnd, 
					  unsigned int mask, ITraceFilter *pFilter, trace_t *ptr )
{
	ICollideable *pCollision = pEntity->GetCollideable();

	// Adding this assertion here so game code catches it, but really the assertion belongs in the engine
	// because one day, rotated collideables will work!
	Assert( pCollision->GetCollisionAngles() == vec3_angle );

	enginetrace->SweepCollideable( pCollision, vecAbsStart, vecAbsEnd, pCollision->GetCollisionAngles(), mask, pFilter, ptr );
}

// ----
// This is basically a regular TraceLine that uses the FilterEntity filter.
void UTIL_TraceLineFilterEntity( CBaseEntity *pEntity, const Vector &vecAbsStart, const Vector &vecAbsEnd, 
					   unsigned int mask, int nCollisionGroup, trace_t *ptr )
{
	CTraceFilterEntity traceFilter( pEntity, nCollisionGroup );
	UTIL_TraceLine( vecAbsStart, vecAbsEnd, mask, &traceFilter, ptr );
}

void UTIL_ClipTraceToPlayers( const Vector& vecAbsStart, const Vector& vecAbsEnd, unsigned int mask, ITraceFilter *filter, trace_t *tr )
{
	trace_t playerTrace;
	Ray_t ray;
	float smallestFraction = tr->fraction;
	const float maxRange = 60.0f;

	ray.Init( vecAbsStart, vecAbsEnd );

	for ( int k = 1; k <= gpGlobals->maxClients; ++k )
	{
		CBasePlayer *player = UTIL_PlayerByIndex( k );

		if ( !player || !player->IsAlive() )
			continue;

#ifdef CLIENT_DLL
		if ( player->IsDormant() )
			continue;
#endif // CLIENT_DLL

		if ( filter && filter->ShouldHitEntity( player, mask ) == false )
			continue;

		float range = DistanceToRay( player->WorldSpaceCenter(), vecAbsStart, vecAbsEnd );
		if ( range < 0.0f || range > maxRange )
			continue;

		enginetrace->ClipRayToEntity( ray, mask|CONTENTS_HITBOX, player, &playerTrace );
		if ( playerTrace.fraction < smallestFraction )
		{
			// we shortened the ray - save off the trace
			*tr = playerTrace;
			smallestFraction = playerTrace.fraction;
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Make a tracer effect
//-----------------------------------------------------------------------------
void UTIL_Tracer( const Vector &vecStart, const Vector &vecEnd, int iEntIndex, 
				 int iAttachment, float flVelocity, bool bWhiz, const char *pCustomTracerName )
{
	CEffectData data;
	data.m_vStart = vecStart;
	data.m_vOrigin = vecEnd;
#ifdef CLIENT_DLL
	data.m_hEntity = ClientEntityList().EntIndexToHandle( iEntIndex );
#else
	data.m_nEntIndex = iEntIndex;
#endif
	data.m_flScale = flVelocity;

	// Flags
	if ( bWhiz )
	{
		data.m_fFlags |= TRACER_FLAG_WHIZ;
	}
	if ( iAttachment != TRACER_DONT_USE_ATTACHMENT )
	{
		data.m_fFlags |= TRACER_FLAG_USEATTACHMENT;
		// Stomp the start, since it's not going to be used anyway
		data.m_nAttachmentIndex = 1;
	}

	// Fire it off
	if ( pCustomTracerName )
	{
		DispatchEffect( pCustomTracerName, data );
	}
	else
	{
		DispatchEffect( "Tracer", data );
	}
}


void UTIL_BloodDrips( const Vector &origin, const Vector &direction, int color, int amount )
{
	if ( !UTIL_ShouldShowBlood( color ) )
		return;

	if ( color == DONT_BLEED || amount == 0 )
		return;

	if ( g_Language.GetInt() == LANGUAGE_GERMAN && color == BLOOD_COLOR_RED )
		color = 0;

	if ( g_pGameRules->IsMultiplayer() )
	{
		// scale up blood effect in multiplayer for better visibility
		amount *= 5;
	}

	if ( amount > 255 )
		amount = 255;

	if (color == BLOOD_COLOR_MECH)
	{
		g_pEffects->Sparks(origin);
		if (random->RandomFloat(0, 2) >= 1)
		{
			UTIL_Smoke(origin, random->RandomInt(10, 15), 10);
		}
	}
	else
	{
		// Normal blood impact
		UTIL_BloodImpact( origin, direction, color, amount );
	}
}	

//-----------------------------------------------------------------------------
// Purpose: Returns low violence settings
//-----------------------------------------------------------------------------
static ConVar	violence_hblood( "violence_hblood","1", 0, "Draw human blood" );
static ConVar	violence_hgibs( "violence_hgibs","1", 0, "Show human gib entities" );
static ConVar	violence_ablood( "violence_ablood","1", 0, "Draw alien blood" );
static ConVar	violence_agibs( "violence_agibs","1", 0, "Show alien gib entities" );

bool UTIL_IsLowViolence( void )
{
	if ( !violence_hblood.GetBool() || !violence_ablood.GetBool() || !violence_hgibs.GetBool() || !violence_agibs.GetBool() )
		return true;

	return false;
}

bool UTIL_ShouldShowBlood( int color )
{
	if ( color != DONT_BLEED )
	{
		if ( color == BLOOD_COLOR_RED )
		{
			return violence_hblood.GetBool();
		}
		else
		{
			return violence_ablood.GetBool();
		}
	}
	return false;
}


//------------------------------------------------------------------------------
// Purpose : Use trace to pass a specific decal type to the entity being decaled
// Input   :
// Output  :
//------------------------------------------------------------------------------
void UTIL_DecalTrace( trace_t *pTrace, char const *decalName )
{
	if (pTrace->fraction == 1.0)
		return;

	CBaseEntity *pEntity = pTrace->m_pEnt;
	pEntity->DecalTrace( pTrace, decalName );
}


void UTIL_BloodDecalTrace( trace_t *pTrace, int bloodColor )
{
	if ( UTIL_ShouldShowBlood( bloodColor ) )
	{
		if ( bloodColor == BLOOD_COLOR_RED )
		{
			UTIL_DecalTrace( pTrace, "Blood" );
		}
		else
		{
			UTIL_DecalTrace( pTrace, "YellowBlood" );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &pos - 
//			&dir - 
//			color - 
//			amount - 
//-----------------------------------------------------------------------------
void UTIL_BloodImpact( const Vector &pos, const Vector &dir, int color, int amount )
{
	CEffectData	data;

	data.m_vOrigin = pos;
	data.m_vNormal = dir;
	data.m_flScale = (float)amount;
	data.m_nColor = (unsigned char)color;

	DispatchEffect( "bloodimpact", data );
}

bool UTIL_IsSpaceEmpty( CBaseEntity *pMainEnt, const Vector &vMin, const Vector &vMax )
{
	Vector vHalfDims = ( vMax - vMin ) * 0.5f;
	Vector vCenter = vMin + vHalfDims;

	trace_t trace;
	UTIL_TraceHull( vCenter, vCenter, -vHalfDims, vHalfDims, MASK_SOLID, pMainEnt, COLLISION_GROUP_NONE, &trace );

	bool bClear = ( trace.fraction == 1 && trace.allsolid != 1 && (trace.startsolid != 1) );
	return bClear;
}

void UTIL_StringToFloatArray( float *pVector, int count, const char *pString )
{
	char *pstr, *pfront, tempString[128];
	int	j;

	Q_strncpy( tempString, pString, sizeof(tempString) );
	pstr = pfront = tempString;

	for ( j = 0; j < count; j++ )			// lifted from pr_edict.c
	{
		pVector[j] = atof( pfront );

		// skip any leading whitespace
		while ( *pstr && *pstr <= ' ' )
			pstr++;

		// skip to next whitespace
		while ( *pstr && *pstr > ' ' )
			pstr++;

		if (!*pstr)
			break;

		pstr++;
		pfront = pstr;
	}
	for ( j++; j < count; j++ )
	{
		pVector[j] = 0;
	}
}

void UTIL_StringToVector( float *pVector, const char *pString )
{
	UTIL_StringToFloatArray( pVector, 3, pString );
}

void UTIL_StringToIntArray( int *pVector, int count, const char *pString )
{
	char *pstr, *pfront, tempString[128];
	int	j;

	Q_strncpy( tempString, pString, sizeof(tempString) );
	pstr = pfront = tempString;

	for ( j = 0; j < count; j++ )			// lifted from pr_edict.c
	{
		pVector[j] = atoi( pfront );

		while ( *pstr && *pstr != ' ' )
			pstr++;
		if (!*pstr)
			break;
		pstr++;
		pfront = pstr;
	}

	for ( j++; j < count; j++ )
	{
		pVector[j] = 0;
	}
}

void UTIL_StringToColor32( color32 *color, const char *pString )
{
	int tmp[4];
	UTIL_StringToIntArray( tmp, 4, pString );
	color->r = tmp[0];
	color->g = tmp[1];
	color->b = tmp[2];
	color->a = tmp[3];
}

#ifndef _XBOX
void UTIL_DecodeICE( unsigned char * buffer, int size, const unsigned char *key)
{
	if ( !key )
		return;

	IceKey ice( 0 ); // level 0 = 64bit key
	ice.set( key ); // set key

	int blockSize = ice.blockSize();

	unsigned char *temp = (unsigned char *)_alloca( PAD_NUMBER( size, blockSize ) );
	unsigned char *p1 = buffer;
	unsigned char *p2 = temp;
				
	// encrypt data in 8 byte blocks
	int bytesLeft = size;
	while ( bytesLeft >= blockSize )
	{
		ice.decrypt( p1, p2 );
		bytesLeft -= blockSize;
		p1+=blockSize;
		p2+=blockSize;
	}

	// copy encrypted data back to original buffer
	Q_memcpy( buffer, temp, size-bytesLeft );
}
#endif

// work-around since client header doesn't like inlined gpGlobals->curtime
float IntervalTimer::Now( void ) const
{
	return gpGlobals->curtime;
}

// work-around since client header doesn't like inlined gpGlobals->curtime
float CountdownTimer::Now( void ) const
{
	return gpGlobals->curtime;
}


#ifdef CLIENT_DLL
	CBasePlayer *UTIL_PlayerByIndex( int entindex )
	{
		return ToBasePlayer( ClientEntityList().GetEnt( entindex ) );
	}
#endif
