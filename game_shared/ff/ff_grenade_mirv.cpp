/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file ff_grenade_concussion.cpp
/// @author Shawn Smith (L0ki)
/// @date Jan. 29, 2005
/// @brief mirv grenade class
/// 
/// Implementation of the CFFGrenadeMirv class. This is the secondary grenade type for the demoman and hwguy classes.
/// 
/// Revisions
/// ---------
/// Jan. 29, 2005	L0ki: Initial Creation
/// Apr. 23, 2005	L0ki: removed header file, moved everything to a single cpp file

#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"

#define MIRVGRENADE_MODEL "models/grenades/mirv/mirv.mdl"

#ifdef CLIENT_DLL
	#define CFFGrenadeMirv C_FFGrenadeMirv
#else
	#include "ff_entity_system.h"
#endif

#ifdef GAME_DLL
	static ConVar mirv_vel_min( "ffdev_mirv_vel_min","250");
	static ConVar mirv_vel_max( "ffdev_mirv_vel_max","350");
	static ConVar mirv_ang_x_min("ffdev_mirv_ang_x_min","45.0",0,"Minimum x spawn angle for mirvlets");
	static ConVar mirv_ang_x_max("ffdev_mirv_ang_x_max","90.0",0,"Maximum x spawn angle for mirvlets");
	static ConVar mirv_ang_y_seed("ffdev_mirv_ang_y_seed","0",0,"");
	static ConVar mirv_ang_y_min("ffdev_mirv_ang_y_min","0",0,"Minimum y spawn angle for mirvlets");
	static ConVar mirv_ang_y_max("ffdev_mirv_ang_y_max","360",0,"Maximum y spawn angle for mirvlets");
	static ConVar mirv_ang_z_min("ffdev_mirv_ang_z_min","0",0,"Minimum x spawn angle for mirvlets");
	static ConVar mirv_ang_z_max("ffdev_mirv_ang_z_max","0",0,"Maximum x spawn angle for mirvlets");
	static ConVar mirvlet_dmg("ffdev_mirvlet_dmg","180.0",0,"Damage a single mirvlet does");
#endif

class CFFGrenadeMirv : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeMirv,CFFGrenadeBase)

	virtual void Precache();
	virtual const char *GetBounceSound() { return "MirvGrenade.Bounce"; }
	virtual Class_T Classify( void ) { return CLASS_GREN_MIRV; }

#ifdef CLIENT_DLL
	CFFGrenadeMirv() {}
	CFFGrenadeMirv( const CFFGrenadeMirv& ) {}
#else
	virtual void Spawn();
	virtual void Explode( trace_t *pTrace, int bitsDamageType );

#endif
};

class CFFGrenadeMirvlet : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeMirvlet,CFFGrenadeBase)

	virtual void Precache();
	virtual Class_T Classify( void ) { return CLASS_GREN_MIRVLET; }

#ifdef CLIENT_DLL
	CFFGrenadeMirvlet() {}
	CFFGrenadeMirvlet( const CFFGrenadeMirvlet& ) {}
#else
	virtual void Spawn();
#endif
};

LINK_ENTITY_TO_CLASS( ff_grenade_mirv, CFFGrenadeMirv);
PRECACHE_WEAPON_REGISTER( ff_grenade_mirv );

#ifdef GAME_DLL

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFGrenadeMirv::Spawn( void )
{
	SetModel( MIRVGRENADE_MODEL );
	BaseClass::Spawn();

	SetLocalAngularVelocity(QAngle(0, RandomFloat(-5, 5), 0));
}

//-----------------------------------------------------------------------------
// Purpose: Normal explosion, but also spawn mirvlets
//-----------------------------------------------------------------------------
void CFFGrenadeMirv::Explode( trace_t *pTrace, int bitsDamageType )
{
	BaseClass::Explode( pTrace, bitsDamageType );

	CBaseEntity *pOwner = GetOwnerEntity();

	//random starting y axis rotation
	// after the first mirvlet, each of the remaining mirvletss will be rotated another 90 degrees
	float y_ang_seed = RandomFloat(0.0f,360.0f);
	for ( int i = 0; i < 4; i++ )
	{
		Vector vecSrc = GetAbsOrigin();
		QAngle angSpawn;

		angSpawn.x = RandomFloat(45.0f,90.0f);
		angSpawn.y = y_ang_seed + (i*90.0f);
		angSpawn.z = RandomFloat(mirv_ang_z_min.GetFloat(),mirv_ang_z_max.GetFloat());//0.0f;

		Vector vecVelocity;
		AngleVectors(angSpawn,&vecVelocity);
		vecVelocity *= RandomFloat(mirv_vel_min.GetFloat(),mirv_vel_max.GetFloat());

		// So they don't begin moving down, I guess
		if (vecVelocity.z < 0)
			vecVelocity.z *= -1;

		CFFGrenadeMirvlet *pMirvlet = (CFFGrenadeMirvlet *)CreateEntityByName( "ff_grenade_mirvlet" );

		// I dunno... just try again i suppose
		if (!pMirvlet)
			continue;

		QAngle angRotate;
		angRotate.x = RandomFloat(-360.0f, 360.0f);
		angRotate.y = RandomFloat(-360.0f, 360.0f);
		angRotate.z = 2.0*RandomFloat(-360.0f, 360.0f);

		UTIL_SetOrigin( pMirvlet, vecSrc );
		pMirvlet->SetAbsAngles(QAngle(0,0,0)); //make the model stand on end
		pMirvlet->SetLocalAngularVelocity(angRotate);
		pMirvlet->Spawn();
		pMirvlet->SetOwnerEntity( pOwner );

		// Set the speed and the initial transmitted velocity
		// L0ki: changed this to a cvar to tweak mirvlet damage
		pMirvlet->SetDamage( mirvlet_dmg.GetFloat() );
		pMirvlet->m_DmgRadius = CFFGrenadeBase::GetGrenadeRadius();
		pMirvlet->SetAbsVelocity( vecVelocity );
		pMirvlet->SetupInitialTransmittedVelocity( vecVelocity );
		pMirvlet->SetDetonateTimerLength( RandomFloat(2.0f,3.0f) );
		pMirvlet->SetElasticity( GetGrenadeElasticity() );

		pMirvlet->ChangeTeam( pOwner->GetTeamNumber() );
		pMirvlet->SetThrower( (CBaseCombatCharacter*)pOwner ); 
		pMirvlet->SetGravity( GetGrenadeGravity() + 0.2f );
		pMirvlet->SetFriction( GetGrenadeFriction() );
	}
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFGrenadeMirv::Precache()
{
	PrecacheModel( MIRVGRENADE_MODEL );
	BaseClass::Precache();
}