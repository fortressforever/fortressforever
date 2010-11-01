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

ConVar ffdev_mancannon_push_foward( "ffdev_mancannon_push_forward", "1024", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar ffdev_mancannon_push_up( "ffdev_mancannon_push_up", "512", FCVAR_REPLICATED | FCVAR_CHEAT );

ConVar ffdev_mancannon_health( "ffdev_mancannon_health", "300", FCVAR_REPLICATED );
ConVar ffdev_mancannon_lifetime( "ffdev_mancannon_lifetime", "60.0", FCVAR_REPLICATED );

// Jiggles: Sorry, but I'm not using the "mancannon" nomenclature; Bungie didn't invent the jump pad!
//#define JUMPPAD_INITIAL_DEPLOY	0	
#define JUMPPAD_ACTIVATE		1
#define JUMPPAD_POWERDOWN		2
#define JUMPPAD_REMOVE			3

#define JUMPPAD_WARMUP_TIME		1.0f
#define JUMPPAD_LIFESPAN		ffdev_mancannon_lifetime.GetFloat()
#define JUMPPAD_POWERDOWN_TIME	5.0f

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
	DEFINE_THINKFUNC( OnJumpPadThink ),
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
	//"JumpPad.WarmUp",
	"JumpPad.PowerDown",
	"JumpPad.Fire",
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

	//Sets the team color -GreenMushy
	CFFPlayer *pOwner = ToFFPlayer( m_hOwner.Get() ); //static_cast< CFFPlayer * >( m_hOwner.Get() );
	if( pOwner ) 
		m_nSkin = ( pOwner->GetTeamNumber() - 1 ); 
		
	m_iJumpPadState = JUMPPAD_ACTIVATE;
	m_bTakesDamage = true;//Making the jumppad take damage -GreenMushy
	m_iHealth = ffdev_mancannon_health.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFManCannon::GoLive( void )
{
	VPROF_BUDGET( "CFFManCannon::GoLive", VPROF_BUDGETGROUP_FF_BUILDABLE );

	CFFBuildableObject::GoLive();

	m_bBuilt = true;
	SetCollisionGroup( COLLISION_GROUP_PUSHAWAY );
	AddSolidFlags(FSOLID_TRIGGER);
	CollisionProp()->UseTriggerBounds(true, 5);
	SetTouch( &CFFManCannon::OnObjectTouch );

	// Take away what it cost to build
	CFFPlayer *pOwner = GetOwnerPlayer();
	if( pOwner )
		pOwner->RemoveAmmo( 1, AMMO_MANCANNON );

	// tell the client when it expires
	CSingleUserRecipientFilter user(pOwner);
	user.MakeReliable();


	UserMessageBegin(user, "ManCannonMsg");
		WRITE_FLOAT(gpGlobals->curtime + JUMPPAD_LIFESPAN + JUMPPAD_POWERDOWN_TIME);
	MessageEnd();
	

	// start thinking
	SetThink( &CFFManCannon::OnJumpPadThink );
	// Stagger our starting times
	SetNextThink( gpGlobals->curtime + random->RandomFloat( 0.1f, 0.3f ) );
}




//-----------------------------------------------------------------------------
// Purpose: Generic think function
//-----------------------------------------------------------------------------
void CFFManCannon::OnJumpPadThink( void )
{
	switch ( m_iJumpPadState )
	{
	//case JUMPPAD_INITIAL_DEPLOY:
	//	// Start warmup sound
	//	EmitSound( "JumpPad.WarmUp" );
	//	SetNextThink( gpGlobals->curtime + JUMPPAD_WARMUP_TIME );
	//	m_iJumpPadState++;
	//	break;
	case JUMPPAD_ACTIVATE:
		// Play activate sound
		//EmitSound("JumpPad.Activate");
		SetNextThink( gpGlobals->curtime + JUMPPAD_LIFESPAN );
		m_iJumpPadState++; 
		break;
	case JUMPPAD_POWERDOWN:
		//EmitSound("JumpPad.PowerDown");
		SetNextThink( gpGlobals->curtime + JUMPPAD_POWERDOWN_TIME );
		m_iJumpPadState++;
		break;
	case JUMPPAD_REMOVE:
		//Removing this bit so it doesnt tell you it expired -GreenMushy
		/*
		CFFPlayer *pOwner = GetOwnerPlayer();
		if ( pOwner )
			ClientPrint( pOwner, HUD_PRINTCENTER, "#FF_MANCANNON_TIMEOUT" );
		Detonate();
		*/
		break;
	}
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

	if ( m_iJumpPadState < JUMPPAD_ACTIVATE + 1 )
		return;

	if( !pOther )
		return;

	if( !pOther->IsPlayer() )
		return;

	CFFPlayer *pPlayer = ToFFPlayer( pOther );

	if( g_pGameRules->PlayerRelationship( GetOwnerPlayer(), pPlayer ) == GR_NOTTEAMMATE )//Team orients it -GreenMushy
		return;

	if( !pPlayer )
		return;
	
	// can only use it once per second
	if (gpGlobals->curtime < pPlayer->m_flMancannonTime + 1.0f)
	{
		//DevMsg("Mancannon ready in %f\n", (gpGlobals->curtime - (pPlayer->m_flMancannonTime + 1.0f)));
		return;
	}

	// Only trigger when the player hits his jump key
	if ( !(pPlayer->m_nButtons & IN_JUMP) || pPlayer->m_nButtons & IN_DUCK )//So u dont activate when u duck -GreenMushy
		return;

	// Launch the guy
	QAngle vecAngles = pPlayer->EyeAngles();
	//vecAngles.z = 0.0f;

	Vector vecForward;
	AngleVectors( vecAngles, &vecForward );
	vecForward.z = 0.f;
	VectorNormalize( vecForward );

	// Shoot forward & up-ish
	// pPlayer->ApplyAbsVelocityImpulse( (vecForward * ffdev_mancannon_push_foward.GetFloat()) + Vector( 0, 0, ffdev_mancannon_push_foward.GetFloat() ) );

	// add an amount to their horizontal + vertical velocity (dont multiply cos slow classes wouldnt go anywhere!)
	//pPlayer->ApplyAbsVelocityImpulse( (vecForward * ffdev_mancannon_push_foward.GetFloat()) + Vector( 0, 0, ffdev_mancannon_push_up.GetFloat() ) );

	pPlayer->SetAbsVelocity((vecForward * ffdev_mancannon_push_foward.GetFloat()) + Vector( 0, 0, ffdev_mancannon_push_up.GetFloat() ) );
	
	//Vector vecVelocity = pPlayer->GetAbsVelocity();
	//Vector vecLatVelocity = vecVelocity * Vector(1.0f, 1.0f, 0.0f);

	//pPlayer->SetAbsVelocity(Vector(vecVelocity.x * ffdev_mancannon_push_foward.GetFloat(), vecVelocity.y  * ffdev_mancannon_push_foward.GetFloat(), vecVelocity.z + ffdev_mancannon_push_up.GetFloat() ) );
	//DevMsg("Mancannon boost! X vel: %f, Y vel: %f, Z vel: %f\n", vecVelocity.x * ffdev_mancannon_push_foward.GetFloat(), vecVelocity.y  * ffdev_mancannon_push_foward.GetFloat(), vecVelocity.z + ffdev_mancannon_push_up.GetFloat() );
	EmitSound("JumpPad.Fire");

	pPlayer->m_flMancannonTime = gpGlobals->curtime;
	//Detonate();
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
	//Jiggles: Actually, we'd rather this not do any damage
	float flDamage = 140.0f;

	//if( m_hOwner.Get() )
	//{
	//	CTakeDamageInfo info( this, m_hOwner, vec3_origin, GetAbsOrigin(), flDamage, DMG_BLAST );
	//	RadiusDamage( info, GetAbsOrigin(), 625, CLASS_NONE, NULL );
		
		UTIL_ScreenShake( GetAbsOrigin(), flDamage * 0.0125f, 150.0f, m_flExplosionDuration, 620.0f, SHAKE_START );
	//}
}
