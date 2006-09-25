// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildableobjects.cpp
// @author Patrick O'Leary (Mulchman)
// @date 06/08/2005
// @brief Client side BuildableObject classes:
//			Dispenser, Detpack, & SentryGun
//
// REVISIONS
// ---------
//	12/28/2005, Mulchman: 
//		ClientSide Detpack first created
//
//	12/28/2005, Mulchman: 
//		ClientSide Dispenser first created
//
//	01/05/2005, Mulchman: 
//		ClientSide SentryGun first created
//
//	05/11/2005, Mulchman
//		Added the CreateClientSide stuff so that we spawn
//		a dispenser only on the client - mainly for the
//		dispenser weapon slot of the engineer
//
//	05/12/2005, Mulchman
//		Added the CreateClientSide stuff so that we spawn
//		a detpack only on the client - mainly for the
//		detpack weapon slot of the demoman
//
//	05/12/2005, Mulchman:
//		Added the CreateClientSide stuff so that we spawn
//		a sentrygun only on the client - mainly for the
//		sentrygun weapon slot of the engineer
//
//	06/08/2005, Mulchman:
//		Now derives from C_AI_BaseNPC for additional
//		(and easy) functionality
//
// 06/08/2005, Mulchman: 
//		This file First created - a container for all
//		ClientSide buildable object code
//
//	03/17/2006, Mulchman:
//		Removing aim sphere
//
//	05/10/2006, Mulchman:
//		Add radio tags to dispenser

#include "cbase.h"
#include "ff_buildableobjects_shared.h"
#include "c_ff_timers.h"

#include "materialsystem/IMaterialSystem.h"
#include "materialsystem/IMesh.h"
#include "ClientEffectPrecacheSystem.h"

// For DrawSprite
#include "beamdraw.h"

// Defines
#define FF_BUILD_ERROR_NOROOM	"sprites/ff_build_noroom"
#define FF_BUILD_ERROR_TOOSTEEP	"sprites/ff_build_toosteep"
#define FF_BUILD_ERROR_TOOFAR	"sprites/ff_build_toofar"
#define FF_BUILD_ERROR_OFFGROUND	"sprites/ff_build_offground"
#define FF_BUILD_ERROR_MOVEABLE		"sprites/ff_build_moveable"

// Define all the sprites to precache
CLIENTEFFECT_REGISTER_BEGIN( PrecacheBuildErrorNoRoom )
CLIENTEFFECT_MATERIAL( FF_BUILD_ERROR_NOROOM )
CLIENTEFFECT_REGISTER_END()

CLIENTEFFECT_REGISTER_BEGIN( PrecacheBuildErrorTooSteep )
CLIENTEFFECT_MATERIAL( FF_BUILD_ERROR_TOOSTEEP )
CLIENTEFFECT_REGISTER_END()

CLIENTEFFECT_REGISTER_BEGIN( PrecacheBuildErrorTooFar )
CLIENTEFFECT_MATERIAL( FF_BUILD_ERROR_TOOFAR )
CLIENTEFFECT_REGISTER_END()

CLIENTEFFECT_REGISTER_BEGIN( PrecacheBuildErrorOffGround )
CLIENTEFFECT_MATERIAL( FF_BUILD_ERROR_OFFGROUND )
CLIENTEFFECT_REGISTER_END()

CLIENTEFFECT_REGISTER_BEGIN( PrecacheBuildErrorMoveable )
CLIENTEFFECT_MATERIAL( FF_BUILD_ERROR_MOVEABLE )
CLIENTEFFECT_REGISTER_END()

//=============================================================================
//
//	class C_FFBuildableObject
//
//=============================================================================

#if defined( CFFBuildableObject )
	#undef CFFBuildableObject
#endif

IMPLEMENT_CLIENTCLASS_DT( C_FFBuildableObject, DT_FFBuildableObject, CFFBuildableObject )
	RecvPropEHandle( RECVINFO( m_hOwner ) ),
	RecvPropInt( RECVINFO( m_iHealth ) ),
	RecvPropInt( RECVINFO( m_iMaxHealth ) ),
	RecvPropInt( RECVINFO( m_bBuilt ) ),
END_RECV_TABLE( )

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_FFBuildableObject::C_FFBuildableObject( void )
{
	// Initialize
	m_bClientSideOnly = false;
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
C_FFBuildableObject::~C_FFBuildableObject( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFBuildableObject::OnDataChanged( DataUpdateType_t updateType )
{
	// NOTE: We MUST call the base classes' implementation of this function
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: To smooth the build helpers
//-----------------------------------------------------------------------------
void C_FFBuildableObject::ClientThink( void )
{
	// This is to "smooth" the build helper models
	if( m_bClientSideOnly )
	{
		C_FFPlayer *pPlayer = ToFFPlayer( m_hOwner.Get() );
		if( !pPlayer )
			return;

		float flBuildDist = 0.0f;

		switch( Classify() )
		{
			case CLASS_DISPENSER:
			flBuildDist = FF_BUILD_DISP_BUILD_DIST;
			break;

			case CLASS_SENTRYGUN:
			flBuildDist = FF_BUILD_SG_BUILD_DIST;
			break;

			case CLASS_DETPACK:
			flBuildDist = FF_BUILD_DET_BUILD_DIST;
			break;

			default: return; break;
		}

		Vector vecForward;
		pPlayer->EyeVectors( &vecForward );
		vecForward.z = 0.0f;
		VectorNormalize( vecForward );

		// Need to save off the z value before setting new origin
		Vector vecOrigin = GetAbsOrigin();

		// Compute a new origin in front of the player
		Vector vecNewOrigin = pPlayer->GetAbsOrigin() + ( vecForward * ( flBuildDist + 16.0f ) );
		vecNewOrigin.z = vecOrigin.z;

		SetAbsOrigin( vecNewOrigin );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Using this to draw any "can't build" type glyphs
//-----------------------------------------------------------------------------
int C_FFBuildableObject::DrawModel( int flags )
{
	if( m_bClientSideOnly )
	{
		// Draw our glyphs

		// See if there's even an error
		if( m_hBuildError != BUILD_ALLOWED )
		{
			float flOffset = 0.0f;

			// Get an offset for drawing (relative to GetAbsOrigin)
			switch( Classify() )
			{
				case CLASS_DISPENSER: flOffset = 32.0f; break;
				case CLASS_SENTRYGUN: flOffset = 32.0f; break;
				case CLASS_DETPACK: flOffset = 0.0f; break;
				default: return BaseClass::DrawModel( flags ); break;
			}

			const char *pszMaterial = NULL;

			// Find out which error we're showing
			switch( m_hBuildError )
			{
				case BUILD_NOROOM: pszMaterial = FF_BUILD_ERROR_NOROOM; break;
				case BUILD_TOOSTEEP: pszMaterial = FF_BUILD_ERROR_TOOSTEEP; break;
				case BUILD_TOOFAR: pszMaterial = FF_BUILD_ERROR_TOOFAR; break;
				case BUILD_PLAYEROFFGROUND: pszMaterial = FF_BUILD_ERROR_OFFGROUND; break;
				case BUILD_MOVEABLE: pszMaterial = FF_BUILD_ERROR_MOVEABLE; break;
			}

			// If a valid material...
			if( pszMaterial )
			{
				// Draw!
				IMaterial *pMaterial = materials->FindMaterial( pszMaterial, TEXTURE_GROUP_OTHER );
				if( pMaterial )
				{
					materials->Bind( pMaterial );
					color32 c = { 255, 0, 0, 255 };
					DrawSprite( Vector( GetAbsOrigin().x, GetAbsOrigin().y, EyePosition().z + flOffset ), 15.0f, 15.0f, c );
				}
			}

			// Finally, there was a build error, so don't actually draw the real model!
			return 0;
		}
	}
		
	return BaseClass::DrawModel( flags );
}

//=============================================================================
//
//	class C_FFSevTest
//
//=============================================================================

#if defined( CFFSevTest )
	#undef CFFSevTest
#endif

IMPLEMENT_CLIENTCLASS_DT( C_FFSevTest, DT_FFSevTest, CFFSevTest )
END_RECV_TABLE( )

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_FFSevTest::C_FFSevTest( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
C_FFSevTest::~C_FFSevTest( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFSevTest::OnDataChanged( DataUpdateType_t updateType )
{
	// NOTE: We MUST call the base classes' implementation of this function
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//=============================================================================
//
//	class C_FFDetpack
//
//=============================================================================

#if defined( CFFDetpack )
	#undef CFFDetpack
#endif

IMPLEMENT_CLIENTCLASS_DT( C_FFDetpack, DT_FFDetpack, CFFDetpack )
END_RECV_TABLE( )


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_FFDetpack::C_FFDetpack( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
C_FFDetpack::~C_FFDetpack( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFDetpack::OnDataChanged( DataUpdateType_t updateType )
{
	// NOTE: We MUST call the base classes' implementation of this function
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Creates a client side entity using the detpack model
//-----------------------------------------------------------------------------
C_FFDetpack *C_FFDetpack::CreateClientSideDetpack( const Vector& vecOrigin, const QAngle& vecAngles )
{
	C_FFDetpack *pDetpack = new C_FFDetpack;

	if( !pDetpack )
		return NULL;

	if( !pDetpack->InitializeAsClientEntity( FF_DETPACK_MODEL, RENDER_GROUP_TRANSLUCENT_ENTITY ) )
	{
		pDetpack->Release( );

		return NULL;
	}

	pDetpack->SetAbsOrigin( vecOrigin );
	pDetpack->SetLocalAngles( vecAngles );
	pDetpack->SetCollisionGroup( COLLISION_GROUP_DEBRIS );
	pDetpack->SetRenderMode( kRenderTransAlpha );
	pDetpack->SetRenderColorA( ( byte )110 );

	//kRenderTransAlphaAdd
	//kRender

	// Since this is client side only, give it an owner just in case
	// someone accesses the m_hOwner.Get() and wants to return something
	// that isn't NULL!
	pDetpack->m_hOwner = ( C_BaseEntity * )C_BasePlayer::GetLocalPlayer();
	pDetpack->SetClientSideOnly( true );
	pDetpack->SetNextClientThink( CLIENT_THINK_ALWAYS );

	return pDetpack;
}

//=============================================================================
//
//	class C_FFDispenser
//
//=============================================================================

#if defined( CFFDispenser )
	#undef CFFDispenser
#endif

IMPLEMENT_CLIENTCLASS_DT( C_FFDispenser, DT_FFDispenser, CFFDispenser )
	RecvPropInt( RECVINFO( m_iAmmoPercent ) ),
	RecvPropInt( RECVINFO( m_iCells ) ),
	RecvPropInt( RECVINFO( m_iShells ) ),
	RecvPropInt( RECVINFO( m_iNails ) ),
	RecvPropInt( RECVINFO( m_iRockets ) ),
	RecvPropInt( RECVINFO( m_iArmor ) ),
	RecvPropInt( RECVINFO( m_iRadioTags ) ),
END_RECV_TABLE( )

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
//C_FFDispenser::C_FFDispenser( void )
//{
//}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
//C_FFDispenser::~C_FFDispenser( void )
//{
//}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFDispenser::OnDataChanged( DataUpdateType_t updateType )
{
	// NOTE: We MUST call the base classes' implementation of this function
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Creates a client side entity using the dispenser model
//-----------------------------------------------------------------------------
C_FFDispenser *C_FFDispenser::CreateClientSideDispenser( const Vector& vecOrigin, const QAngle& vecAngles )
{
	C_FFDispenser *pDispenser = new C_FFDispenser;

	if( !pDispenser )
		return NULL;

	if( !pDispenser->InitializeAsClientEntity( FF_DISPENSER_MODEL, RENDER_GROUP_TRANSLUCENT_ENTITY ) )
	{
		pDispenser->Release( );

		return NULL;
	}

	pDispenser->SetAbsOrigin( vecOrigin );
	pDispenser->SetLocalAngles( vecAngles );
	pDispenser->SetCollisionGroup( COLLISION_GROUP_DEBRIS );
	pDispenser->SetRenderMode( kRenderTransAlpha );
	pDispenser->SetRenderColorA( ( byte )110 );

	// Since this is client side only, give it an owner just in case
	// someone accesses the m_hOwner.Get() and wants to return something
	// that isn't NULL!
	pDispenser->m_hOwner = ( C_BaseEntity * )C_BasePlayer::GetLocalPlayer();
	pDispenser->SetClientSideOnly( true );
	pDispenser->SetNextClientThink( CLIENT_THINK_ALWAYS );

	return pDispenser;
}

//=============================================================================
//
//	class C_FFSentryGun
//
//=============================================================================

#if defined( CFFSentryGun )
	#undef CFFSentryGun
#endif

IMPLEMENT_CLIENTCLASS_DT( C_FFSentryGun, DT_FFSentryGun, CFFSentryGun )
	RecvPropInt( RECVINFO( m_iAmmoPercent ) ),
	RecvPropFloat( RECVINFO( m_flRange ) ),
	RecvPropInt( RECVINFO( m_iLevel ) ),
	RecvPropInt( RECVINFO( m_iShells ) ),
	RecvPropInt( RECVINFO( m_iRockets ) ),
	RecvPropInt( RECVINFO( m_iMaxShells ) ),
	RecvPropInt( RECVINFO( m_iMaxRockets ) ),
END_RECV_TABLE( )

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_FFSentryGun::C_FFSentryGun( void )
{
	m_iLocalHallucinationIndex = -1;
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
C_FFSentryGun::~C_FFSentryGun( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFSentryGun::OnDataChanged( DataUpdateType_t updateType )
{
	// NOTE: We MUST call the base classes' implementation of this function
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_FFSentryGun::Upgrade( bool bUpgradelevel, int iCells, int iShells, int iRockets )
{
	if( ( m_iLevel < 3 ) && m_bBuilt )
		return true;
	else
		return false;
}

//-----------------------------------------------------------------------------
// Purpose: Creates a client side entity using the sentrygun model
//-----------------------------------------------------------------------------
C_FFSentryGun *C_FFSentryGun::CreateClientSideSentryGun( const Vector& vecOrigin, const QAngle& vecAngles )
{
	C_FFSentryGun *pSentryGun = new C_FFSentryGun;

	if( !pSentryGun )
		return NULL;

	if( !pSentryGun->InitializeAsClientEntity( FF_SENTRYGUN_MODEL, RENDER_GROUP_TRANSLUCENT_ENTITY ) )
	{
		pSentryGun->Release( );

		return NULL;
	}

	pSentryGun->SetAbsOrigin( vecOrigin );
	pSentryGun->SetLocalAngles( vecAngles );
	pSentryGun->SetCollisionGroup( COLLISION_GROUP_DEBRIS );
	pSentryGun->SetRenderMode( kRenderTransAlpha );
	pSentryGun->SetRenderColorA( ( byte )110 );

	// Since this is client side only, give it an owner just in case
	// someone accesses the m_hOwner.Get() and wants to return something
	// that isn't NULL!
	pSentryGun->m_hOwner = ( C_BaseEntity * )C_BasePlayer::GetLocalPlayer();

	// Mirv: Show up as the correct skin
	pSentryGun->m_nSkin = clamp(CBasePlayer::GetLocalPlayer()->GetTeamNumber() - TEAM_BLUE, 0, 3);
	pSentryGun->SetClientSideOnly( true );
	pSentryGun->SetNextClientThink( CLIENT_THINK_ALWAYS );

	return pSentryGun;
}

//-------------------------------------------------------------------------
// Purpose: Sentryguns will sometimes appear as the wrong model when
//			the local player is hallucinating
//-------------------------------------------------------------------------
int C_FFSentryGun::DrawModel(int flags)
{
	int nRet = BaseClass::DrawModel(flags);

	if (m_iLevel <= 0)
		return nRet;

	C_FFPlayer *pLocalPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

	// No hallucinations here. But check first if we have to reset
	if (!pLocalPlayer || !pLocalPlayer->m_iHallucinationIndex)
	{
		if (m_iLocalHallucinationIndex >= 0)
		{
			switch (m_iLevel)
			{
			case 1:
				SetModel(FF_SENTRYGUN_MODEL);
				break;
			case 2:
				SetModel(FF_SENTRYGUN_MODEL_LVL2);
				break;
			case 3:
				SetModel(FF_SENTRYGUN_MODEL_LVL3);
				break;
			}

			m_iLocalHallucinationIndex = -1;
		}

		return nRet;
	}

	int nNewLevel = entindex() + pLocalPlayer->m_iHallucinationIndex;

	nNewLevel = nNewLevel % 3; // ouch

	if (m_iLocalHallucinationIndex == nNewLevel)
		return nRet;

	switch (nNewLevel)
	{
	case 0:
		SetModel(FF_SENTRYGUN_MODEL);
		break;
	case 1:
		SetModel(FF_SENTRYGUN_MODEL_LVL2);
		break;
	case 2:
		SetModel(FF_SENTRYGUN_MODEL_LVL3);
		break;
	}

	m_iLocalHallucinationIndex = nNewLevel;

	return nRet;
}
