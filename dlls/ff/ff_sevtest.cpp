#include "cbase.h"
#include "ff_sevtest.h"

LINK_ENTITY_TO_CLASS( FF_SevTest_entity, CFFSevTest );
PRECACHE_REGISTER( FF_SevTest_entity );

IMPLEMENT_SERVERCLASS_ST( CFFSevTest, DT_FFSevTest )
END_SEND_TABLE( )

// Start of our data description for the class
BEGIN_DATADESC( CFFSevTest )
	DEFINE_THINKFUNC( OnObjectThink ),
END_DATADESC( )

// Array of char *'s to SevTest models
const char *g_pszFFSevTestModels[ ] =
{
	FF_SEVTEST_MODEL,
	NULL
};

// Array of char *'s to gib models
const char *g_pszFFSevTestGibModels[ ] =
{
	NULL
};

// Array of char *'s to sounds
const char *g_pszFFSevTestSounds[ ] =
{
	NULL
};

static ConVar sevtest_duration( "ffdev_sevtest_duration", "5" );

CFFSevTest::CFFSevTest( void )
{
	// Overwrite the base class stubs
	m_ppszModels = g_pszFFSevTestModels;
	m_ppszGibModels = g_pszFFSevTestGibModels;
	m_ppszSounds = g_pszFFSevTestSounds;
}

CFFSevTest::~CFFSevTest( void )
{
}

void CFFSevTest::OnObjectThink( void )
{
	// See if it's time to remove yet
	if( gpGlobals->curtime > ( sevtest_duration.GetInt( ) + m_flSpawnTime ))
		UTIL_Remove( this );	

	// Call base class think func to synchronize health
	CFFBuildableObject::OnObjectThink( );

	// Set the next time to call this function
	SetNextThink( gpGlobals->curtime + m_flThinkTime );
}

/**
@fn void Spawn( )
@brief Spawns a model - called from Create
@return void
*/ 
void CFFSevTest::Spawn( void )
{
	// Yeah, you can guess what this does!
	Precache( );

	// TODO: Modify the health to whatever value we decide
	m_iHealth = 150;

	m_flSpawnTime = gpGlobals->curtime;

	// Call baseclass spawn stuff
	CFFBuildableObject::Spawn( );

	// Set up a think function
	SetThink( &CFFSevTest::OnObjectThink );		// |-- Mirv: Account for GCC strictness
	SetNextThink( gpGlobals->curtime + m_flThinkTime );
}

/**
@fn void GoLive( )
@brief Object is built and ready to do it's thing
@return void
*/
void CFFSevTest::GoLive( void )
{
	// Call base class
	CFFBuildableObject::GoLive( );
}

/**
@fn CFFSevTest *Create( const Vector& vecOrigin, const QAngle& vecAngles, edict_t *pentOwner )
@brief Creates a new CFFSevTest at a particular location
@param vecOrigin - origin of the object to be created
@param vecAngles - view angles of the object to be created
@param pentOwner - edict_t of the owner creating the object (usually keep NULL)
@return a _new_ CFFSevTest
*/ 
CFFSevTest *CFFSevTest::Create( const Vector &vecOrigin, const QAngle &vecAngles, edict_t *pentOwner )
{
	// Create the object
	CFFSevTest *pObject = ( CFFSevTest * )CBaseEntity::Create( "FF_SevTest_entity", vecOrigin, vecAngles, CBaseEntity::Instance( pentOwner ) );

	pObject->SetOwnerEntity( Instance( pentOwner ) );

	// Spawn the object
	pObject->Spawn( );

	return pObject;
}