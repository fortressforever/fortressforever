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

#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"

#define CALTROPGRENADE_MODEL "models/weapons/w_eq_fraggrenade_thrown.mdl"

#ifdef CLIENT_DLL
	#define CFFGrenadeCaltrop C_FFGrenadeCaltrop
#else
	#include "ff_entity_system.h"
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
#endif

class CFFGrenadeCaltrop : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeCaltrop,CFFGrenadeBase)

	CNetworkVector(m_vInitialVelocity);

	virtual void Precache();

	virtual float		GetShakeAmplitude( void ) { return 0.0f; }	// remove the shake

#ifdef CLIENT_DLL
	CFFGrenadeCaltrop() {}
	CFFGrenadeCaltrop( const CFFGrenadeCaltrop& ) {}
#else
	virtual void Spawn();
	virtual void Explode(trace_t *pTrace, int bitsDamageType);
#endif
};

// mirrored in ff_caltrop.cpp
class CFFCaltrop : public CBaseAnimating
{
public:
	DECLARE_CLASS( CFFCaltrop, CBaseAnimating );

	CFFCaltrop();
	void Spawn( void );
	void Precache( void );
	void CaltropTouch ( CBaseEntity *pOther );
	void ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity );
	virtual const char *GetBounceSound() { return "CaltropGrenade.Bounce"; }

	DECLARE_DATADESC();
private:
	float m_flSpawnTime;
};

LINK_ENTITY_TO_CLASS( caltropgrenade, CFFGrenadeCaltrop );
PRECACHE_WEAPON_REGISTER( caltropgrenade );

#ifdef GAME_DLL
	void CFFGrenadeCaltrop::Spawn( void )
	{
		DevMsg("[Grenade Debug] CFFGrenadeCaltrop::Spawn\n");
		SetModel( CALTROPGRENADE_MODEL );
		BaseClass::Spawn();
	}
	void CFFGrenadeCaltrop::Explode(trace_t *pTrace, int bitsDamageType)
	{
		DevMsg("[Grenade Debug] CFFGrenadeCaltrop::Explode\n");
		CFFGrenadeBase::PreExplode( pTrace, NULL, NULL );

		// If the grenade is in a no gren area don't deploy caltrops
		if( FFScriptRunPredicates( this, "canexplode", true ) )
		{

			CFFPlayer *pOwner = ToFFPlayer( GetOwnerEntity() );
			DevMsg("[Grenade Debug] pOwner: 0x0%X\n",pOwner);

			// TODO: drop caltrops ala TFC
			DevMsg("[Grenade Debug] Dropping caltrops\n");
			for ( int i = 0; i < 6; i++ )
			{
				Vector vOrigin = GetAbsOrigin();
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
				vOrigin += vecVelocity*.1;

				QAngle angRotate;
				angRotate.x = RandomFloat(-360.0f, 360.0f);
				angRotate.y = RandomFloat(-360.0f, 360.0f);
				angRotate.z = 2.0*RandomFloat(-360.0f, 360.0f);

				UTIL_SetOrigin( pCaltrop, vOrigin );
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
		}

		CFFGrenadeBase::PostExplode();
	}
#endif

void CFFGrenadeCaltrop::Precache()
{
	DevMsg("[Grenade Debug] CFFGrenadeCaltrop::Precache\n");
	PrecacheModel( CALTROPGRENADE_MODEL );
	BaseClass::Precache();
}