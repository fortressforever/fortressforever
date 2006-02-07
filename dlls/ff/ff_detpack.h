// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_detpack.h
// @author Patrick O'Leary (Mulchman)
// @date 12/28/2005
// @brief Detpack class
//
// REVISIONS
// ---------
//	12/28/2005, Mulchman: 
//		First created
//
//	05/09/2005, Mulchman:
//		Tons of little updates here and there
//
//	09/28/2005,	Mulchman:
//		Played with the think time and adding
//		support for whine sound when hit by
//		emp or 5 seconds left to go until
//		explode time

#ifndef FF_DETPACK_H
#define FF_DETPACK_H

#ifdef _WIN32
#pragma once
#endif

#include "ff_buildableobject.h"

class CFFDetpack : public CFFBuildableObject
{
public:
	DECLARE_CLASS( CFFDetpack, CFFBuildableObject )
	DECLARE_SERVERCLASS()
	DECLARE_DATADESC()

	CFFDetpack( void );
	~CFFDetpack( void );

	virtual void Spawn( void );
	void GoLive( void );

	void OnObjectTouch( CBaseEntity *pOther );
	void OnObjectThink( void );
	void SendStartTimerMessage( void );
	void SendStopTimerMessage( void );
	void OnEmpExplosion( void );

	virtual Class_T	Classify( void ) { return CLASS_DETPACK; }

	static CFFDetpack *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL );

public:	
	// Network variables and stuff

public:
	bool	m_bLive;
	int		m_iFuseTime;
	float	m_flDetonateTime;
	bool	m_bFiveSeconds;

};

#endif // FF_DETPACK_H
