/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file ff_modelglyph.cpp
/// @author Kevin Hjelden (FryGuy)
/// @date 29 Dec 2005
/// @brief Class for drawing models attached to objects
/// 
/// Revisions
/// ---------
/// 29 Dec 2005: Initial Creation
//
//	7/4/2006: Mulchman
//		Made this OOPy

#include "cbase.h"
#include "ff_modelglyph.h"

#ifdef CLIENT_DLL 
	#include "c_ff_player.h"
	#include "ff_gamerules.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
//
// Class CFFModelGlyph tables
//
//=============================================================================
IMPLEMENT_NETWORKCLASS_ALIASED( FFModelGlyph, DT_FFModelGlyph ) 

BEGIN_NETWORK_TABLE( CFFModelGlyph, DT_FFModelGlyph )
#ifdef CLIENT_DLL 
#else
#endif
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA( CFFModelGlyph )
END_PREDICTION_DATA()

#ifdef GAME_DLL
// Datatable
BEGIN_DATADESC( CFFModelGlyph )
	DEFINE_THINKFUNC( OnObjectThink ),
END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( ff_modelglyph, CFFModelGlyph );
PRECACHE_REGISTER( ff_modelglyph );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFModelGlyph::CFFModelGlyph( void )
{
#ifdef GAME_DLL
	m_flLifeTime = -1;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
CFFModelGlyph::~CFFModelGlyph( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Precache
//-----------------------------------------------------------------------------
void CFFModelGlyph::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Set's the lifetime
//-----------------------------------------------------------------------------
void CFFModelGlyph::Spawn( void )
{
#ifdef GAME_DLL
	SetBlocksLOS( false );

	SetThink( &CFFModelGlyph::OnObjectThink );
	SetNextThink( gpGlobals->curtime );
#endif

	BaseClass::Spawn();
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: Server thinking
//-----------------------------------------------------------------------------
void CFFModelGlyph::OnObjectThink( void )
{
	if( ( gpGlobals->curtime >= m_flLifeTime ) && ( m_flLifeTime != -1 ) )
	{
		UTIL_Remove( this );
	}
	else
	{
		SetNextThink( gpGlobals->curtime );
	}
}
#endif

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Make the client think always
//-----------------------------------------------------------------------------
void CFFModelGlyph::OnDataChanged( DataUpdateType_t updateType ) 
{
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED ) 
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}	
}

//-----------------------------------------------------------------------------
// Purpose: Position the model
//-----------------------------------------------------------------------------
void CFFModelGlyph::ClientThink( void )
{
}
#endif

//=============================================================================
//
// Class CFFSaveMe tables
//
//=============================================================================
IMPLEMENT_NETWORKCLASS_ALIASED( FFSaveMe, DT_FFSaveMe ) 

BEGIN_NETWORK_TABLE( CFFSaveMe, DT_FFSaveMe )
#ifdef CLIENT_DLL 
#else
#endif
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA( CFFSaveMe )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( ff_saveme, CFFSaveMe );
PRECACHE_REGISTER( ff_saveme );

//-----------------------------------------------------------------------------
// Purpose: Precache
//-----------------------------------------------------------------------------
void CFFSaveMe::Precache( void )
{
	PrecacheModel( FF_SAVEME_MODEL );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Spawn
//-----------------------------------------------------------------------------
void CFFSaveMe::Spawn( void )
{
	Precache();
	SetModel( FF_SAVEME_MODEL );

	BaseClass::Spawn();

	ResetSequenceInfo();
	SetSequence( SelectWeightedSequence( ( Activity )ACT_IDLE ) );
}

//-----------------------------------------------------------------------------
// Purpose: See if the model should be drawn
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
bool CFFSaveMe::ShouldDraw( void )
{
	if( IsEffectActive( EF_NODRAW ) )
		return false;

	if( C_BasePlayer::GetLocalPlayer() == NULL )
		return false;

	if( GetOwnerEntity() == NULL )
		return false;

	CFFPlayer *pLocalPlayer = ToFFPlayer( C_BasePlayer::GetLocalPlayer() );
	CFFPlayer *pOwner = ToFFPlayer( GetOwnerEntity() );

	// Shouldn't happen
	if( !pLocalPlayer )
		return false;

	// Just in case someone attaches this to a non-player
	if( !pOwner || !pOwner->IsPlayer() )
		return false;

	if( pLocalPlayer->IsObserver() || pOwner->IsObserver() || ( pOwner->GetTeamNumber() < TEAM_BLUE ) || ( pLocalPlayer->GetTeamNumber() < TEAM_BLUE ) )
		return false;

	// Hide from player if not the right team
	if( pLocalPlayer && pOwner )
	{
		if( FFGameRules()->IsTeam1AlliedToTeam2( pLocalPlayer->GetTeamNumber(), pOwner->GetTeamNumber() ) == GR_NOTTEAMMATE )
		{
			AddEffects( EF_NODRAW );
			return false;
		}
	}

	return true;
}
#endif
