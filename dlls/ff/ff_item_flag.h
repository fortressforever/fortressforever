/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file \Steam\SteamApps\SourceMods\FortressForeverCode\dlls\ff\ff_item_flag.h
/// @author Kevin Hjelden (FryGuy)
/// @date Jun. 29, 2005
/// @brief Flag Item (generic lua entity)
/// 
/// Implements a basic flag item for the lua scripting system to implement
/// 
/// Revisions
/// ---------
/// Jun. 29, 2005	FryGuy: Initial Creation

#ifndef FF_ITEM_FLAG_H
#define FF_ITEM_FLAG_H

#include "ammodef.h"
#include "items.h"
#include "ff_player.h"
#include "ff_weapon_base.h" //for ammo types

//#define FLAG_MODEL "models/items/backpack/backpack.mdl"
#define FLAG_MODEL "models/items/healthkit.mdl"
//#define FLAG_MODEL "models/items/ball/ball.mdl"

// Forward declaration
class CFFInfoScriptAnimator;

class CFFInfoScript : public CBaseAnimating
{
public:
	DECLARE_CLASS( CFFInfoScript, CBaseAnimating );
	DECLARE_SERVERCLASS();		// |-- Mirv: Added for client class
	DECLARE_DATADESC();

	CFFInfoScript( void );
	~CFFInfoScript( void );

	virtual void	Spawn( void );
	virtual void	Precache( void );
	void			OnTouch( CBaseEntity * );
	void			OnPlayerDied( CFFPlayer * );
	void			OnThink( void );
	void			TempThink( void );
	void			OnRespawn( void );

	virtual bool	IsPlayer( void ) { return false; }
	virtual bool	BlocksLOS( void ) { return false; }
	virtual bool	IsAlive( void ) { return false; }

	void			Pickup( CFFPlayer * );
	void			Drop( float delay, float speed = 0.0f );
	void			Respawn( float delay );
	CBaseEntity*	Return ( void );

	void			SetSpawnFlags( int flags );

	void			LUA_SetModel( const char *model );
	void			LUA_SetSkin( int skin );

	bool			HasAnimations( void ) const { return m_bHasAnims; }

protected:

	bool CreateItemVPhysicsObject( void );

	bool m_atStart;

	bool m_bUsePhysics;
	
	bool m_bHasAnims;
	int m_iSequence;
	Activity m_Activity;
	void PlayIdleAnim( void );
	void PlayActiveAnim( void );
	CFFInfoScriptAnimator *m_pAnimator;

	Vector m_vStartOrigin;
	QAngle m_vStartAngles;

	//CFFPlayer *m_pOwner;
	CFFPlayer *m_pLastOwner;
	
	CNetworkVar(float, m_flThrowTime);

	CNetworkVector( m_vecOffset );
};

// This is a cheap hack
class CFFInfoScriptAnimator : public CBaseAnimating
{
public:
	DECLARE_CLASS( CFFInfoScriptAnimator, CBaseAnimating );
	DECLARE_DATADESC();

	virtual void	Spawn( void );
	void			OnThink( void );

	CFFInfoScript *m_pFFScript;
};

#endif//FF_ITEM_FLAG_H