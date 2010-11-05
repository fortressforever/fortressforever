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

/////////////////////////////////////////////////////////////////////////////
// includes
#include "baseanimating.h"

/////////////////////////////////////////////////////////////////////////////
// defines
#define FLAG_MODEL "models/items/healthkit.mdl"

/////////////////////////////////////////////////////////////////////////////
// An info_ff_script has 3 position states
enum FF_ScriptPosState_e
{
	PS_RETURNED = 0,
	PS_CARRIED = 1,
	PS_DROPPED = 2,
	PS_REMOVED = 3
};

// An info_ff_script has 3 goal states
enum FF_ScriptGoalState_e
{
	GS_ACTIVE = 0,
	GS_INACTIVE = 1,
	GS_REMOVED = 2
};

enum FF_AllowFlags
{
	kAllowOnlyPlayers	= 1 << 0,
	kAllowBlueTeam		= 1 << 1,
	kAllowRedTeam		= 1 << 2,
	kAllowYellowTeam	= 1 << 3,
	kAllowGreenTeam		= 1 << 4,
	kAllowScout			= 1 << 5,
	kAllowSniper		= 1 << 6,
	kAllowSoldier		= 1 << 7,
	kAllowDemoman		= 1 << 8,
	kAllowMedic			= 1 << 9,
	kAllowHwguy			= 1 << 10,
	kAllowPyro			= 1 << 11,
	kAllowSpy			= 1 << 12,
	kAllowEngineer		= 1 << 13,
	kAllowCivilian		= 1 << 14,
};

/////////////////////////////////////////////////////////////////////////////
// Forward declaration
class CFFInfoScriptAnimator;

namespace luabind
{
	namespace adl
	{
		class object;
	}
}

/////////////////////////////////////////////////////////////////////////////
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
	virtual void	UpdateOnRemove( void );
	void			OnTouch( CBaseEntity *pEntity );
	void			OnOwnerDied( CBaseEntity *pEntity );
	void			OnOwnerForceRespawn( CBaseEntity *pEntity );
	void			OnThink( void );
	void			OnRespawn( void );

	virtual bool	IsPlayer( void ) { return false; }
	virtual bool	BlocksLOS( void ) { return false; }
	virtual bool	IsAlive( void ) { return false; }

	virtual int		ShouldTransmit( const CCheckTransmitInfo *pInfo );
	virtual int		UpdateTransmitState( void );

	// An info_ff_script's position state
	virtual bool	IsCarried( void );
	virtual bool	IsDropped( void );
	virtual bool	IsReturned( void );

	// An info_ff_script's goal state
	virtual bool	IsActive( void );
	virtual bool	IsInactive( void );
	virtual bool	IsRemoved( void );	

	// Expose these two
	virtual void	LUA_Remove( void );
	virtual void	LUA_Restore( void );

	CBaseEntity		*GetCarrier( void );
	CBaseEntity		*GetDropper( void );

	// Phsyics can be enabled on this object which
	// makes certain functions not work!
	Vector			LUA_GetOrigin( void ) const;
	void			LUA_SetOrigin( const Vector& vecOrigin );
	QAngle			LUA_GetAngles( void ) const;
	void			LUA_SetAngles( const QAngle& vecAngles );

	void			LUA_SetModel( const char *szModel );

	void			LUA_SetStartOrigin(const Vector& vecOrigin);
	void			LUA_SetStartAngles(const QAngle& vecAngles);

	void			Pickup( CBaseEntity *pEntity );
	void			Drop( float delay, Vector pos, Vector velocity );
	void			Drop( float delay, float speed = 0.0f );
	void			Respawn( float delay );
	void			Return( void );
	void			ForceReturn( void );

	void			RemoveThink( void );

	void			SetSpawnFlags( int flags );

	bool			HasAnimations( void ) const { return m_bHasAnims; }
	virtual Class_T	Classify( void ) { return CLASS_INFOSCRIPT; }

	void SetBotGoalInfo(int _type);
	void SpawnBot(const char *_name, int _team, int _class);

	// returns the criteria necessary for another entity to "touch" this entity
	int GetTouchFlags( void ) const { return m_allowTouchFlags; }
	// returns the criteria necessary for another entity to NOT "touch" this entity
	int GetDisallowTouchFlags( void ) const { return m_disallowTouchFlags; }

	// sets criteria for another entity to touch this entity
	void SetTouchFlags(const luabind::adl::object& table);
	// sets criteria for another entity to NOT touch this entity
	void SetDisallowTouchFlags(const luabind::adl::object& table);

	// returns true if a specified is allowed to touch this entity
	bool CanEntityTouch(CBaseEntity* pEntity);

	virtual void	ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity );

	// bot info accessors
	int GetBotTeamFlags() const { return m_BotTeamFlags; }
	int GetBotGoalType() const { return m_BotGoalType; }
protected:
	// Do not expose these to LUA!
	virtual void	SetActive( void );
	virtual void	SetInactive( void );
	virtual void	SetRemoved( void );
	virtual void	SetCarried( void );
	virtual void	SetReturned( void );
	virtual void	SetDropped( void );

	bool CreateItemVPhysicsObject( void );

	void PlayDroppedAnim( void );
	void PlayCarriedAnim( void );
	void PlayReturnedAnim( void	);
	void InternalPlayAnim( Activity hActivity );

protected:	

	bool m_atStart;
	CNetworkVar( unsigned int, m_iHasModel );

	bool m_bUsePhysics;
	
	bool m_bHasAnims;	
	CFFInfoScriptAnimator *m_pAnimator;

	Vector m_vStartOrigin;
	QAngle m_vStartAngles;
	Vector	m_vecMins;
	Vector	m_vecMaxs;

	CBaseEntity *m_pLastOwner;
	
	CNetworkVar(float, m_flThrowTime);

	CNetworkVector( m_vecOffset );
	
	CNetworkVar( unsigned int, m_iShadow );

	CNetworkVar( int, m_iGoalState );
	CNetworkVar( int, m_iPosState );

	float m_flSpawnTime;
	float m_flReturnTime;
	bool m_bFloatActive;

	// indicates some criteria limiting what will
	// be allowed to "touch" this entity
	int		m_allowTouchFlags;
	int		m_disallowTouchFlags;

	// cached information for bot use
	int		m_BotTeamFlags;
	int		m_BotGoalType;
};

/////////////////////////////////////////////////////////////////////////////
// This is a cheap hack. Basically, this just calls
// StudioFrameAdvance() on m_pFFScript.
class CFFInfoScriptAnimator : public CBaseAnimating
{
public:
	DECLARE_CLASS( CFFInfoScriptAnimator, CBaseAnimating );
	DECLARE_DATADESC();

	CFFInfoScriptAnimator( void ) : m_pFFScript(NULL) {}
	~CFFInfoScriptAnimator( void ) {}

	virtual void	Spawn( void );
	void			OnThink( void );

	CFFInfoScript *m_pFFScript;
};

/////////////////////////////////////////////////////////////////////////////
#endif //FF_ITEM_FLAG_H
