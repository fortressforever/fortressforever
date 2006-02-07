// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildableobjects.h
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

#ifndef C_FF_BUILDABLEOBJECTS_H
#define C_FF_BUILDABLEOBJECTS_H

#ifdef _WIN32
#pragma once
#endif

#include "c_AI_BaseNPC.h"

#include "ff_buildableobjects_shared.h"

// SG Aim Sphere
class C_FFSentryGun_AimSphere : public C_BaseAnimating
{
public:
	DECLARE_CLASS( C_FFSentryGun_AimSphere, C_BaseAnimating );

	C_FFSentryGun_AimSphere( void ) {};
	~C_FFSentryGun_AimSphere( void ) {};

	static C_FFSentryGun_AimSphere *CreateClientSentryGun_AimSphere( const Vector& vecOrigin, const QAngle& vecAngles )
	{
		C_FFSentryGun_AimSphere *pAimSphere = new C_FFSentryGun_AimSphere;

		if( !pAimSphere )
			return NULL;

		if( !pAimSphere->InitializeAsClientEntity( FF_SENTRYGUN_AIMSPHERE_MODEL, RENDER_GROUP_TRANSLUCENT_ENTITY ) )
		{
			pAimSphere->Release();

			return NULL;
		}

		// Lower the origin on the z-axis by the radius of the aim sphere model
		pAimSphere->SetAbsOrigin( vecOrigin - Vector( 0, 0, FF_SENTRYGUN_AIMSPHERE_RADIUS ) );
		pAimSphere->SetLocalAngles( vecAngles );
		pAimSphere->SetCollisionGroup( COLLISION_GROUP_NONE );
		pAimSphere->SetRenderMode( kRenderTransAlpha );
		pAimSphere->SetRenderColorA( ( byte )110 );

		return pAimSphere;
	}
};

//=============================================================================
//
//	class C_FFBuildableObject
//
//=============================================================================
class C_FFBuildableObject : public C_AI_BaseNPC
{
public:
	DECLARE_CLASS( C_FFBuildableObject, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();

	C_FFBuildableObject( void );
	~C_FFBuildableObject( void );

	virtual void OnDataChanged( DataUpdateType_t updateType );

	virtual bool IsAlive( void ) { return true; }
	virtual bool IsPlayer( void ) const { return false; }
	virtual int	GetHealth( void ) const { return m_iHealth; }
	virtual int	GetMaxHealth( void ) const { return m_iMaxHealth; }

	bool CheckForOwner( void ) { return ( m_hOwner.Get() ); }

public:
	CNetworkHandle( CBaseEntity, m_hOwner );
	bool	m_bBuilt;

};

//=============================================================================
//
//	class C_FFSevTest
//
//=============================================================================
class C_FFSevTest : public C_FFBuildableObject
{
public:
	DECLARE_CLASS( C_FFSevTest, C_FFBuildableObject );
	DECLARE_CLIENTCLASS( );

	C_FFSevTest( void );
	~C_FFSevTest( void );

	virtual void OnDataChanged( DataUpdateType_t updateType );

};

//=============================================================================
//
//	class C_FFDetpack
//
//=============================================================================
class C_FFDetpack : public C_FFBuildableObject
{
public:
	DECLARE_CLASS( C_FFDetpack, C_FFBuildableObject );
	DECLARE_CLIENTCLASS( );

	C_FFDetpack( void );
	~C_FFDetpack( void );

	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual Class_T Classify( void ) { return CLASS_DETPACK; }

	// Creates a client side ONLY detpack - used for the build slot
	static C_FFDetpack *CreateClientSideDetpack( const Vector& vecOrigin, const QAngle& vecAngles );

};

//=============================================================================
//
//	class C_FFDispenser
//
//=============================================================================
class C_FFDispenser : public C_FFBuildableObject
{
public:
	DECLARE_CLASS( C_FFDispenser, C_FFBuildableObject );
	DECLARE_CLIENTCLASS( );

	C_FFDispenser( void );
	~C_FFDispenser( void );

	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual Class_T Classify( void ) { return CLASS_DISPENSER; }

	// Creates a client side ONLY dispenser - used for build slot
	static C_FFDispenser *CreateClientSideDispenser( const Vector& vecOrigin, const QAngle& vecAngles );

public:
	// Network variables
	CNetworkVar( int, m_iCells );
	CNetworkVar( int, m_iShells );
	CNetworkVar( int, m_iNails );
	CNetworkVar( int, m_iRockets );
	CNetworkVar( int, m_iArmor );

};

//=============================================================================
//
//	class C_FFSentryGun
//	NOTE: Does not derive from C_FFBuildableObject
//=============================================================================
class C_FFSentryGun : public C_FFBuildableObject
{
public:
	DECLARE_CLASS( C_FFSentryGun, C_FFBuildableObject );
	DECLARE_CLIENTCLASS( );

	C_FFSentryGun( void );
	~C_FFSentryGun( void );

	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual Class_T Classify( void ) { return CLASS_SENTRYGUN; }

	// Creates a client side ONLY sentrygun - used for build slot
	static C_FFSentryGun *CreateClientSideSentryGun( const Vector& vecOrigin, const QAngle& vecAngles );

public:
	// Mirv: Just going to store the ammo percentage here, with the msb
	// holding the rocket state
	unsigned int m_iAmmoPercent;

	// Network variables
	CNetworkVar( float, m_flRange );
	CNetworkVar( int, m_iLevel );

};

#endif // C_FF_BUILDABLEOBJECTS_H
