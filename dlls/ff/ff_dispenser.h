//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_dispenser.h
//	@author Patrick O'Leary (Mulchman)
//	@date 12/28/2005
//	@brief Dispenser class
//
//	REVISIONS
//	---------
//	12/28/2005, Mulchman: 
//		First created
//
//	05/09/2005, Mulchman:
//		Tons of little updates here and there
//
//	05/10/2005, Mulchman:
//		Added some stuff for regeneration of items inside
//
//	05/11/2005, Mulchman
//		Added some OnTouch stuff to give the player touching
//		us some stuff
//
//	05/13/2005, Mulchman
//		Fixed the OnTouch stuff to work correctly now, heh
//
//	05/17/2005, Mulchman
//		Added some code so the disp is ready
//		to calculate a new explosion magnitude
//		based on how full it is
//
//	08/25/2005, Mulchman
//		Removed physics orienting the weapon and am now doing
//		it manually

#ifndef FF_DISPENSER_H
#define FF_DISPENSER_H

#ifdef _WIN32
#pragma once
#endif

#include "ff_buildableobject.h"

class CFFDispenser : public CFFBuildableObject
{
public:
	DECLARE_CLASS( CFFDispenser, CFFBuildableObject )
	DECLARE_SERVERCLASS()
	DECLARE_DATADESC()

	CFFDispenser( void );
	~CFFDispenser( void );
		
	virtual void Spawn( void );
	void GoLive( void );

	void OnObjectTouch( CBaseEntity *pOther );
	void OnObjectThink( void );
	virtual void Event_Killed( const CTakeDamageInfo &info );

	// Generic function to send hud messages to players
	void SendMessageToPlayer( CFFPlayer *pPlayer, const char *pszMessage, bool bDispenserText = false );

	// Some functions for the custom dispenser text
	void SetText( const char *szCustomText ) { Q_strcpy( m_szCustomText, szCustomText ); }
	const char *GetText( void ) const { return m_szCustomText; }
    
	virtual Class_T	Classify( void ) { return CLASS_DISPENSER; }

	// Bug #0000135: Railgun shot makes dispenser bleed
	virtual int	BloodColor( void ) { return BLOOD_COLOR_MECH; }

	static CFFDispenser *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL );

protected:
	void SendStatsToBot();
//public:
	//IPhysicsConstraint *m_pConstraint;

public:
	// Network variables and shiz
	CNetworkVar( int, m_iCells );
	CNetworkVar( int, m_iShells );
	CNetworkVar( int, m_iNails );
	CNetworkVar( int, m_iRockets );
	CNetworkVar( int, m_iArmor );

protected:
	int		m_iMaxCells;
	int		m_iGiveCells;
	int		m_iMaxShells;
	int		m_iGiveShells;
	int		m_iMaxNails;
	int		m_iGiveNails;
	int		m_iMaxRockets;
	int		m_iGiveRockets;
	int		m_iMaxArmor;
	int		m_iGiveArmor;

	// Custom dispenser text string thing
	char	m_szCustomText[ FF_BUILD_DISP_STRING_LEN ];

	CFFPlayer	*m_pLastTouch;
	float		m_flLastTouch;

	// Actually give a player stuff
	void Dispense( CFFPlayer *pPlayer );

	// Calculates an adjustment to be made to the explosion
	// based on how much stuff is in the dispenser
	void CalcAdjExplosionVal( void );
	float	m_flOrigExplosionMagnitude;
};

#endif // FF_DISPENSER_H
