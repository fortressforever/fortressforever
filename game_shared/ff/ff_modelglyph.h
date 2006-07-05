/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file ff_modelglyph.h
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

#ifndef FF_MODELGLYPH_H
#define FF_MODELGLYPH_H

#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL 
	#define CFFModelGlyph C_FFModelGlyph
	#define CFFSaveMe C_FFSaveMe
#include "c_baseanimating.h"
	#else
#include "baseanimating.h"
#endif

//=============================================================================
//
//	class CFFModelGlyph
//
//=============================================================================
class CFFModelGlyph : public CBaseAnimating
{
public:
	DECLARE_CLASS( CFFModelGlyph, CBaseAnimating );
	DECLARE_NETWORKCLASS();	
	DECLARE_PREDICTABLE();

	// --> Shared code
	CFFModelGlyph( void );
	~CFFModelGlyph( void );

	virtual void	Precache( void );
	virtual void	Spawn( void );

	virtual bool	IsPlayer( void ) const { return false; }
	virtual bool	IsAlive( void ) { return false; }
	virtual bool	BlocksLOS( void ) { return false; }
	// <-- Shared code

#ifdef CLIENT_DLL 
	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual void	ClientThink( void );	
	virtual bool	ShouldDraw( void ) { return true; }
#else
	DECLARE_DATADESC();
	virtual void	OnObjectThink( void );
	virtual void	SetLifeTime( float flLifeTime ) { m_flLifeTime = flLifeTime; }

protected:
	float		m_flLifeTime;
#endif // CLIENT_DLL
};

//=============================================================================
//
//	class CFFSaveMe
//
//=============================================================================

#define FF_SAVEME_LIFETIME		10.0f
#define	FF_SAVEME_MODEL			"models/misc/saveme.mdl"

class CFFSaveMe : public CFFModelGlyph
{
public:
	DECLARE_CLASS( CFFSaveMe, CFFModelGlyph );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	
	// <-- Shared code
	virtual void	Precache( void );
	virtual void	Spawn( void );
	// --> Shared code

#ifdef CLIENT_DLL 
	virtual bool	ShouldDraw( void );
#endif
};

#endif // FF_MODELGLYPH_H
