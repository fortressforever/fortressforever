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
//#include "c_ff_buildableobjects.h"
#include "ff_buildableobjects_shared.h"
#include "c_ff_timers.h"

/*
#if defined( CFFBuildableDoorBlocker )
	#undef CFFBuildableDoorBlocker
#endif

IMPLEMENT_CLIENTCLASS_DT( C_FFBuildableDoorBlocker, DT_FFBuildableDoorBlocker, CFFBuildableDoorBlocker )
END_RECV_TABLE()

#if defined( CFFDispenserDoorBlocker )
	#undef CFFDispenserDoorBlocker
#endif

IMPLEMENT_CLIENTCLASS_DT( C_FFDispenserDoorBlocker, DT_FFDispenserDoorBlocker, CFFDispenserDoorBlocker )
END_RECV_TABLE()

#if defined( CFFSentryGunDoorBlocker )
	#undef CFFSentryGunDoorBlocker
#endif

IMPLEMENT_CLIENTCLASS_DT( C_FFSentryGunDoorBlocker, DT_FFSentryGunDoorBlocker, CFFSentryGunDoorBlocker )
END_RECV_TABLE()
*/

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
END_RECV_TABLE( )

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_FFSentryGun::C_FFSentryGun( void )
{
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

	return pSentryGun;
}
