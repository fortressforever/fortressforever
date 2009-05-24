// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_minecart.h
// @author Patrick O'Leary (Mulchman) 
// @date 5/21/2006
// @brief Mine cart (vphysics vrooom!!!)
//
// REVISIONS
// ---------
//	5/21/2006, Mulchman: 
//		First created

#ifndef FF_MINECART_H
#define FF_MINECART_H

#ifdef _WIN32
#pragma once
#endif

#include "baseanimating.h"

#define FF_MINECART_MODEL	"models/props/ff_dustbowl/minecart.mdl"

//=============================================================================
//
// Class CFFMineCart
//
//=============================================================================

class CFFMineCart : public CBaseAnimating
{
public:
	DECLARE_CLASS( CFFMineCart, CBaseAnimating );
	DECLARE_DATADESC();

	CFFMineCart( void );
	~CFFMineCart( void );

	virtual void Precache( void	);
	virtual void Spawn( void );
	virtual bool CreateVPhysics( void );
	virtual int ObjectCaps( void ) { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION | FCAP_IMPULSE_USE; }

	void OnUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );	

protected:
	bool PhysicsEnabled( void ) const;

protected:
	bool	m_bDisabled;
};

#endif // FF_MINECART_H
