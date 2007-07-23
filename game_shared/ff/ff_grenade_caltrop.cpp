/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file ff_grenade_caltrop.cpp
/// @author Shawn Smith (L0ki)
/// @date Apr 23, 2005
/// @brief caltrop grenade class
/// 
/// Implementation of the CFFGrenadeCaltrop class. This is the primary grenade type for scout
/// 
/// Revisions
/// ---------
/// Apr. 23, 2005	L0ki: Initial Creation
/// Jul. 23, 2007	Defrag: Added checks to stop caltrops & cannister gibs getting stuck in ceiling

#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"

#define CALTROPGRENADE_MODEL_HOLSTER	"models/grenades/caltrop/caltrop_holster.mdl"

#ifdef CLIENT_DLL
	#define CFFGrenadeCaltrop C_FFGrenadeCaltrop
#else
	#include "ff_entity_system.h"
	#include "ff_caltrop.h"
#endif

#ifdef GAME_DLL
	ConVar caltrop_vel_min( "ffdev_caltrop_vel_min","100");
	ConVar caltrop_vel_max( "ffdev_caltrop_vel_max","250");
	ConVar caltrop_ang_x_min("ffdev_caltrop_ang_x_min","30.0",0,"Minimum pitch angle for caltroplets");
	ConVar caltrop_ang_x_max("ffdev_caltrop_ang_x_max","60.0",0,"Maximum pitch angle for caltroplets");
	ConVar caltrop_ang_y_seed("ffdev_caltrop_ang_y_seed","0",0,"");
	ConVar caltrop_ang_y_min("ffdev_caltrop_ang_y_min","0",0,"Minimum yaw angle for caltroplets");
	ConVar caltrop_ang_y_max("ffdev_caltrop_ang_y_max","360",0,"Maximum yaw angle for caltroplets");
	ConVar caltrop_ang_z_min("ffdev_caltrop_ang_z_min","0",0,"Minimum z spawn angle for caltroplets");
	ConVar caltrop_ang_z_max("ffdev_caltrop_ang_z_max","0",0,"Maximum z spawn angle for caltroplets");

	ConVar caltrop_number("ffdef_caltrop_number", "12");
#endif

class CFFGrenadeCaltrop : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeCaltrop,CFFGrenadeBase)

	virtual void Precache();
	virtual Class_T Classify( void ) { return CLASS_GREN_CALTROP; }

	virtual float		GetShakeAmplitude( void ) { return 0.0f; }	// remove the shake

#ifdef CLIENT_DLL
	CFFGrenadeCaltrop() {}
	CFFGrenadeCaltrop( const CFFGrenadeCaltrop& ) {}

	virtual void OnDataChanged( DataUpdateType_t updateType ) { BaseClass::OnDataChanged( updateType ); }
#else
	virtual void Spawn();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);
#endif
};

LINK_ENTITY_TO_CLASS( ff_grenade_caltrop, CFFGrenadeCaltrop );
PRECACHE_WEAPON_REGISTER( ff_grenade_caltrop );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFGrenadeCaltrop::Precache()
{
	PrecacheModel( CALTROPGRENADE_MODEL_HOLSTER );
	BaseClass::Precache();
}

#ifdef GAME_DLL

	//-----------------------------------------------------------------------------
	// Purpose: Set the model
	//-----------------------------------------------------------------------------
	void CFFGrenadeCaltrop::Spawn( void )
	{
		SetModel( CALTROPGRENADE_MODEL_HOLSTER );
		BaseClass::Spawn();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Deploy caltrops everywhere
	//-----------------------------------------------------------------------------
	void CFFGrenadeCaltrop::Explode(trace_t *pTrace, int bitsDamageType)
	{
		CFFPlayer *pOwner = ToFFPlayer( GetOwnerEntity() );

		// #0001281: Caltrops stick to crossover2 map ceiling -> Defrag
		// the caltrops are something like 12 units big and compiled with a scale of 0.6, so they've got a clearance of ~7.2 units.  Call it 8.

		// const var 'cause I cba changing this in 3 places if I need to tweak it
		const float kflCaltropCeilingOffset = 8.0f;

		Vector vCannisterOrigin = GetAbsOrigin();
		Vector vecBeg = vCannisterOrigin;

		// trace from cannister up into the ceiling
		Vector vecTarget = vCannisterOrigin + Vector( 0.0f, 0.0f, kflCaltropCeilingOffset );

		bool bWillSpawnInCeiling = false;
		CBaseEntity *pIgnore = this;

		trace_t tr;
		UTIL_TraceLine( vecBeg, vecTarget, MASK_SOLID, pIgnore, COLLISION_GROUP_NONE, &tr );

		// If the trace hit the world, then the caltrops are possibly going to spawn in the ceiling
		if( tr.DidHit() )
		{			
			if( FF_TraceHitWorld( &tr ) )
			{
				bWillSpawnInCeiling = true;
				DevMsg( "[Grenade Debug] Warning: Caltrops may spawn in ceiling" );			
			}
		}
		
		// Drop the grenade shell gibs
		for( int i = 0; i < 2; i++ )
		{
			Vector vShellOrigin = vCannisterOrigin;
			QAngle angSpawn;

			angSpawn.x = RandomFloat(caltrop_ang_x_min.GetFloat(),caltrop_ang_x_max.GetFloat());
			angSpawn.y = RandomFloat(caltrop_ang_y_min.GetFloat(),caltrop_ang_y_max.GetFloat());
			angSpawn.z = RandomFloat(caltrop_ang_z_min.GetFloat(),caltrop_ang_z_max.GetFloat());//0.0f;

			Vector vecVelocity;
			AngleVectors(angSpawn,&vecVelocity);
			vecVelocity *= RandomFloat(caltrop_vel_min.GetFloat(),caltrop_vel_max.GetFloat());

			// shift them a little so they don't stick on each other
			Vector vOffset = vecVelocity;

			// clamp vertical component of velocity to stop it flying to the moon and into a ceiling -> Defrag
			if( vOffset.z > 2.0f )
				vOffset.z = 2.0f;

			vShellOrigin += vOffset * 0.1f;

			// extra check.  If it's spawning just under a ceiling, move it down a little just to be safe.
			if( bWillSpawnInCeiling )
			{
				vShellOrigin.z -= kflCaltropCeilingOffset;
			}

			CFFCaltropGib *pCaltropGib = ( CFFCaltropGib * )CreateEntityByName( "caltropgib" );
			pCaltropGib->m_iGibModel = clamp( i + 1, 1, 2 );

			// shift them a little so they don't stick on each other
			vShellOrigin += vecVelocity*.1;

			QAngle angRotate;
			angRotate.x = RandomFloat(-360.0f, 360.0f);
			angRotate.y = RandomFloat(-360.0f, 360.0f);
			angRotate.z = 2.0*RandomFloat(-360.0f, 360.0f);

			pCaltropGib->Spawn();
			UTIL_SetOrigin( pCaltropGib, vShellOrigin );
			pCaltropGib->SetAbsAngles( QAngle( 0,0,0 ) ); //make the model stand on end
			pCaltropGib->SetLocalAngularVelocity( angRotate );				
			pCaltropGib->SetOwnerEntity( pOwner );

			// Set the speed and the initial transmitted velocity
			pCaltropGib->SetAbsVelocity( vecVelocity );
			pCaltropGib->SetElasticity( GetGrenadeElasticity() );
			pCaltropGib->ChangeTeam( pOwner->GetTeamNumber() );
			pCaltropGib->SetGravity( GetGrenadeGravity() + 0.2f );
			pCaltropGib->SetFriction( GetGrenadeFriction() );
		}

		// Drop the caltrops
		for ( int i = 0; i < caltrop_number.GetInt(); i++ )
		{			
			Vector vCaltropOrigin = vCannisterOrigin;
			QAngle angSpawn;

			angSpawn.x = RandomFloat(caltrop_ang_x_min.GetFloat(),caltrop_ang_x_max.GetFloat());
			angSpawn.y = RandomFloat(caltrop_ang_y_min.GetFloat(),caltrop_ang_y_max.GetFloat());
			angSpawn.z = RandomFloat(caltrop_ang_z_min.GetFloat(),caltrop_ang_z_max.GetFloat());//0.0f;

			Vector vecVelocity;
			AngleVectors(angSpawn,&vecVelocity);
			vecVelocity *= RandomFloat(caltrop_vel_min.GetFloat(),caltrop_vel_max.GetFloat());

			// So they don't begin moving down, I guess
			if (vecVelocity.z < 0)
				vecVelocity.z *= -1;

			CFFCaltrop *pCaltrop= (CFFCaltrop *)CreateEntityByName( "caltrop" );

			// shift them a little so they don't stick on each other
			Vector vOffset = vecVelocity;
			
			// clamp vertical component of velocity to stop it flying to the moon and into a ceiling -> Defrag
			if( vOffset.z > 2.0f )
				vOffset.z = 2.0f;

			vCaltropOrigin += vOffset * 0.1f;

			// extra check.  If it's spawning just under a ceiling, move it down a little just to be safe.
			if( bWillSpawnInCeiling )
			{
				vCaltropOrigin.z -= kflCaltropCeilingOffset;
			}

			QAngle angRotate;
			angRotate.x = RandomFloat(-360.0f, 360.0f);
			angRotate.y = RandomFloat(-360.0f, 360.0f);
			angRotate.z = 2.0*RandomFloat(-360.0f, 360.0f);

			UTIL_SetOrigin( pCaltrop, vCaltropOrigin );
			pCaltrop->SetAbsAngles(QAngle(0,0,0)); //make the model stand on end
			pCaltrop->SetLocalAngularVelocity(angRotate);
			pCaltrop->Spawn();
			pCaltrop->SetOwnerEntity( pOwner );

			// Set the speed and the initial transmitted velocity
			pCaltrop->SetAbsVelocity( vecVelocity );
			pCaltrop->SetElasticity( GetGrenadeElasticity() );
			pCaltrop->ChangeTeam( pOwner->GetTeamNumber() );
			pCaltrop->SetGravity( GetGrenadeGravity() + 0.2f );
			pCaltrop->SetFriction( GetGrenadeFriction() );
		}

		UTIL_Remove( this );
	}
#endif

