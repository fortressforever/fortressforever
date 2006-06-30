// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_caltrop.h
// @author Patrick O'Leary (Mulchman) 
// @date 6/30/2006
// @brief Caltrop & Gib
//
// REVISIONS
// ---------
//	6/30/2006, Mulchman: 
//		First created

#ifndef FF_MINECART_H
#define FF_MINECART_H

#ifdef _WIN32
#pragma once
#endif

#include "baseanimating.h"

#define CALTROPGRENADE_MODEL_CALTROP	"models/grenades/caltrop/caltrop.mdl"
#define CALTROPGRENADE_MODEL_GIB1		"models/grenades/caltrop/caltrop_holster_gib1.mdl"
#define CALTROPGRENADE_MODEL_GIB2		"models/grenades/caltrop/caltrop_holster_gib2.mdl"

//=============================================================================
//
// Class CFFCaltropGib
//
//=============================================================================
class CFFCaltropGib : public CBaseAnimating
{
public:
	DECLARE_CLASS( CFFCaltropGib, CBaseAnimating );

	CFFCaltropGib( void );
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual void ResolveFlyCollisionCustom( trace_t& trace, Vector& vecVelocity );

	int   m_iGibModel;	// 1 or 2

private:
	float m_flSpawnTime;	
};

//=============================================================================
//
// Class CFFCaltrop
//
//=============================================================================
class CFFCaltrop : public CBaseAnimating
{
public:
	DECLARE_CLASS( CFFCaltrop, CBaseAnimating );

	CFFCaltrop();
	void Spawn( void );
	void Precache( void );
	void CaltropTouch ( CBaseEntity *pOther );
	void ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity );

	DECLARE_DATADESC();
private:
	float m_flSpawnTime;
};

#endif // FF_CALTROP_H
