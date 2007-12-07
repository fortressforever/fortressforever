// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_mancannon.h
// @author Patrick O'Leary (Mulchman)
// @date 12/6/2007
// @brief Man cannon thing
//
// REVISIONS
// ---------
// 12/6/2007, Mulchman: 
//		First created

#include "cbase.h"
#include "ff_buildableobjects_shared.h"
#include "const.h"
#include "ff_gamerules.h"
#include "ff_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar ffdev_mancannon_push_foward( "ffdev_mancannon_push_forward", "1000", FCVAR_NONE );
ConVar ffdev_mancannon_push_up( "ffdev_mancannon_push_up", "50", FCVAR_NONE );

//=============================================================================
//
//	class CFFManCannon
//
//=============================================================================
LINK_ENTITY_TO_CLASS( FF_ManCannon, CFFManCannon );
PRECACHE_REGISTER( FF_ManCannon );

IMPLEMENT_SERVERCLASS_ST( CFFManCannon, DT_FFManCannon )
END_SEND_TABLE()

// Start of our data description for the class
BEGIN_DATADESC( CFFManCannon )
	DEFINE_ENTITYFUNC( OnObjectTouch ),
END_DATADESC()

// Array of char *'s to dispenser models
const char *g_pszFFManCannonModels[] =
{
	FF_MANCANNON_MODEL,
	NULL
};

// Array of char *'s to gib models
const char *g_pszFFManCannonGibModels[] =
{
	NULL
};

// Array of char *'s to sounds
const char *g_pszFFManCannonSounds[] =
{
	FF_MANCANNON_BUILD_SOUND,
	FF_MANCANNON_EXPLODE_SOUND,
	NULL
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFManCannon::Spawn( void )
{
	VPROF_BUDGET( "CFFManCannon::Spawn", VPROF_BUDGETGROUP_FF_BUILDABLE );

	Precache();
	CFFBuildableObject::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFManCannon::GoLive( void )
{
	VPROF_BUDGET( "CFFManCannon::GoLive", VPROF_BUDGETGROUP_FF_BUILDABLE );

	CFFBuildableObject::GoLive();

	m_bBuilt = true;

	SetTouch( &CFFManCannon::OnObjectTouch );

	// Take away what it cost to build
	CFFPlayer *pOwner = GetOwnerPlayer();
	if( pOwner )
		pOwner->RemoveAmmo( 1, AMMO_MANCANNON );
}

//-----------------------------------------------------------------------------
// Purpose: Launch guy
//-----------------------------------------------------------------------------
void CFFManCannon::OnObjectTouch( CBaseEntity *pOther )
{
	VPROF_BUDGET( "CFFManCannon::OnObjectTouch", VPROF_BUDGETGROUP_FF_BUILDABLE );

	CheckForOwner();

	if( !IsBuilt() )
		return;

	if( !pOther )
		return;

	if( !pOther->IsPlayer() )
		return;

	CFFPlayer *pPlayer = ToFFPlayer( pOther );
	if( !pPlayer )
		return;
	
	// Launch the guy
	QAngle vecAngles = pPlayer->EyeAngles();
	vecAngles.z = 0.0f;

	Vector vecForward;
	AngleVectors( vecAngles, &vecForward );
	VectorNormalize( vecForward );

	// Shoot forward & up-ish
	pPlayer->ApplyAbsVelocityImpulse( (vecForward * ffdev_mancannon_push_foward.GetFloat()) + Vector( 0, 0, ffdev_mancannon_push_foward.GetFloat() ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CFFManCannon *CFFManCannon::Create( const Vector& vecOrigin, const QAngle& vecAngles, CBaseEntity *pentOwner )
{
	CFFManCannon *pObject = (CFFManCannon *)CBaseEntity::Create( "FF_ManCannon", vecOrigin, vecAngles, NULL );

	pObject->m_hOwner.GetForModify() = pentOwner;
	pObject->VPhysicsInitNormal( SOLID_VPHYSICS, pObject->GetSolidFlags(), true );
	pObject->Spawn();

	return pObject;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFManCannon::Detonate( void )
{
	VPROF_BUDGET( "CFFManCannon::Detonate", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Fire an event.
	IGameEvent *pEvent = gameeventmanager->CreateEvent( "mancannon_detonated" );
	if( pEvent )
	{
		CFFPlayer *pOwner = GetOwnerPlayer();
		if( pOwner )
		{
			pEvent->SetInt( "userid", pOwner->GetUserID() );
			gameeventmanager->FireEvent( pEvent, true );
		}		
	}

	CFFBuildableObject::Detonate();
}

//-----------------------------------------------------------------------------
// Purpose: Carry out the radius damage for this buildable
//-----------------------------------------------------------------------------
void CFFManCannon::DoExplosionDamage( void )
{
	VPROF_BUDGET( "CFFManCannon::DoExplosionDamage", VPROF_BUDGETGROUP_FF_BUILDABLE );

	float flDamage = 140.0f;

	if( m_hOwner.Get() )
	{
		CTakeDamageInfo info( this, m_hOwner, vec3_origin, GetAbsOrigin(), flDamage, DMG_BLAST );
		RadiusDamage( info, GetAbsOrigin(), 625, CLASS_NONE, NULL );
		
		UTIL_ScreenShake( GetAbsOrigin(), flDamage * 0.0125f, 150.0f, m_flExplosionDuration, 620.0f, SHAKE_START );
	}
}
