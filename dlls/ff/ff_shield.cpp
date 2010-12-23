// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_shield.cpp
// @author Greg "GreenMushy" Stefanakis
// @date 11/28/2010
// @brief Shield class
//

#include "cbase.h"
#include "ff_buildableobjects_shared.h"
#include "ff_scriptman.h"
#include "ff_luacontext.h"
#include "beam_flags.h"
#include "ff_gamerules.h"

#ifdef _DEBUG
#include "Color.h"
#endif
#include "ff_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( FF_Shield, CFFShield );
PRECACHE_REGISTER( FF_Shield );

IMPLEMENT_SERVERCLASS_ST( CFFShield, DT_FFShield )
END_SEND_TABLE( )

// Start of our data description for the class
BEGIN_DATADESC( CFFShield )
	DEFINE_ENTITYFUNC( OnObjectTouch ),
	DEFINE_THINKFUNC( OnObjectThink ),
END_DATADESC( )

extern const char *g_pszFFShieldModels[];

/**
@fn CFFShield
@brief Constructor
@return N/A
*/
CFFShield::CFFShield( void )
{
	// Overwrite the base class stubs
	m_ppszModels = g_pszFFShieldModels;

	m_bTakesDamage = false;

	// Override
	m_bTranslucent = false;
	m_bUsePhysics = true;
}

/**
@fn ~CFFShield
@brief Deconstructor
@return N/A
*/
CFFShield::~CFFShield( void )
{
}

/**
@fn void Spawn( )
@brief Spawns a model - called from Create
@return void
*/ 
void CFFShield::Spawn( void )
{
	VPROF_BUDGET( "CFFShield::Spawn", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Yeah, you can guess what this does!
	Precache();
	
	m_iHealth = 1;

	// Call baseclass spawn stuff
	CFFBuildableObject::Spawn();
}

/**
@fn void GoLive( )
@brief Object is built and ready to do it's thing
@return void
*/
void CFFShield::GoLive( void )
{
	VPROF_BUDGET( "CFFShield::GoLive", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Call base class
	CFFBuildableObject::GoLive();

	// Object is now built
	m_bBuilt = true;

	// Set up our touch function
	SetTouch( &CFFShield::OnObjectTouch );		// |-- Mirv: Account for GCC strictness

	// Set up a think function
	SetThink( &CFFShield::OnObjectThink );		// |-- Mirv: Account for GCC strictness
}

/**
@fn void OnTouch( )
@brief For when an entity touches our object
@param pOther - pointer to the entity that touched our object
@return void
*/
void CFFShield::OnObjectTouch( CBaseEntity *pOther )
{
	VPROF_BUDGET( "CFFShield::OnObjectTouch", VPROF_BUDGETGROUP_FF_BUILDABLE );

	CheckForOwner();

	if( !m_bBuilt )
		return;

	if( !pOther )
		return;

	if( !pOther->IsPlayer() )
		return;

	CFFPlayer *pPlayer = ToFFPlayer( pOther );

	// Skip the owner
	if( ( ( CFFPlayer * )m_hOwner.Get() ) == pPlayer )
		return;
			
	// Skip if spectator
	if( pPlayer->IsObserver() ) // make sure spectators aren't touching the object
		return;

	// Skip if on the same team
	if( FFGameRules()->IsTeam1AlliedToTeam2( pPlayer->GetTeamNumber(), ( ( CFFPlayer * )m_hOwner.Get() )->GetTeamNumber() ) == GR_TEAMMATE )
		return;
}

/**
@fn void OnThink
@brief Think function
@return void
*/
void CFFShield::OnObjectThink( void )
{
	VPROF_BUDGET( "CFFShield::OnObjectThink", VPROF_BUDGETGROUP_FF_BUILDABLE );

	CheckForOwner();

	SetNextThink( gpGlobals->curtime + 0.1f );
}

/**
@fn CFFShield *Create( const Vector& vecOrigin, const QAngle& vecAngles, edict_t *pentOwner )
@brief Creates a new CFFShield at a particular location
@param vecOrigin - origin of the object to be created
@param vecAngles - view angles of the object to be created
@param pentOwner - edict_t of the owner creating the object (usually keep NULL)
@return a _new_ CFFShield
*/ 
CFFShield *CFFShield::Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner )
{
	// Create the object
	CFFShield *pObject = ( CFFShield * )CBaseEntity::Create( "FF_Shield", vecOrigin, vecAngles, NULL );

	// Set our faux owner - see CFFBuildable::Create for the reason why
	pObject->m_hOwner.GetForModify() = pentOwner;

	pObject->VPhysicsInitNormal( SOLID_VPHYSICS, pObject->GetSolidFlags(), true );

	// Spawn the object
	pObject->Spawn( );

	DevMsg("Shield Created\n");

	return pObject;
}