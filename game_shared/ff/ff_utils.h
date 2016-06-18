//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_utils.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date Unknown
//	@brief Utility functions
//
//	REVISIONS
//	---------
//	Unknown, Mulchman: 
//		First created
//
//	Unknown, Mulchman: 
//		Added a bunch of utility type stuff mainly to be used with the
//		Scout Radar
//
//	05/10/2005, Mulchman:
//		Moved some generic buildable stuff over here so it can be used
//		easily on both the client and server - mainly to comply
//		w/ the new buildable object slots (dispenser & sg slot on engy).
//		Modified the build code a bunch, too - completely overhauled.
//
//	06/08/2005, Mulchman:
//		Put more buildable object stuff in here to make it universal
//		and easier to update client/server stuff in one file. Ended
//		up moving buildable stuff out to a buildable shared file.
//	06/18/2005, L0ki:
//		Added a utility macro for sphere queries, mainly for the grenades
//
//	05/28/2006, Mulchman:
//		Added FF_DecalTrace

#ifndef FF_UTILS_H
#define FF_UTILS_H

#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
	#define CFFPlayer C_FFPlayer
	#include "c_ff_player.h"	

	// Once initialized, a pointer to
	// our client side Hud Hint class
	#include "ff_hud_hint.h"
	extern CHudHint *pHudHintHelper;

	// Jiggles: Testin' -- same thing
	#include "ff_hud_HintCenter.h"
	extern CHudHintCenter *g_pHintHelper;
	// end

#else
	#include "ff_player.h"
#endif


// Distance to trace from an entity outward when
// trying to hit solids for placing scorch marks
#define FF_DECALTRACE_TRACE_DIST	48.0f

enum HudElementType_t
{
	HUD_ICON = 0,
	HUD_BOX,
	HUD_TEXT,
	HUD_TIMER,
	HUD_REMOVE,
};

enum HudMessageType_t
{
	HUD_MESSAGE = 0,
	HUD_MESSAGE_DURATION,
	HUD_MESSAGE_COLOR,
	HUD_MESSAGE_COLOR_CUSTOM,
};

int Class_StringToInt( const char *szClassName );
const char *Class_IntToString( int iClassIndex );
const char *Class_IntToResourceString( int iClassIndex );
const char *Class_IntToPrintString( int iClassIndex );

const char *FF_GetPrimaryName( int iClassIndex );
const char *FF_GetSecondaryName( int iClassIndex );

const char *FF_GetDefaultWeapon( int iClassIndex );
const char *FF_GetDefaultWeapon( const char *classname );

void SetColorByTeam( int iTeam, Color& cColor );
Color ColorFade( int currentVal, int minVal, int maxVal, Color minColor, Color maxColor );

Color getIntensityColor( int iAmount, int iMaxAmount, int iColorSetting, int iAlpha, int iRed, int iOrange, int iYellow, int iGreen, bool invertScale  );

int FF_NumPlayersOnTeam( int iTeam );
int FF_GetPlayerOnTeam( int iTeam, int iNum );
int FF_NumPlayers( );
int FF_GetPlayer( int iNum );

void UTIL_GetTeamNumbers(char nTeamNumbers[4]);
void UTIL_GetTeamLimits(char nTeamLimits[4]);
int UTIL_GetTeamSpaces(char nSpacesRemaining[4]);

void UTIL_GetClassNumbers(int iTeamID, char nClassNumbers[10]);
void UTIL_GetClassLimits(int iTeamID, char nClassLimits[10]);
int UTIL_GetClassSpaces(int iTeamID, char nSpacesRemaining[10]);

bool IsPlayerRadioTagTarget( CFFPlayer *pPlayer, int iTeamDoingTargetting );

void FF_DecalTrace( CBaseEntity *pEntity, float flRadius, const char *pszDecalName );

#ifdef GAME_DLL
void FF_LuaHudIcon(CFFPlayer *pPlayer, const char *pszIdentifier, int x, int y, const char *pszImage, int iWidth, int iHeight, int iAlignX = -1, int iAlignY = -1);
void FF_LuaHudBox(CFFPlayer *pPlayer, const char *pszIdentifier, int x, int y, int iWidth, int iHeight, Color clr, Color clrBorder, int iBorderWidth, int iAlignX = -1, int iAlignY = -1);
void FF_LuaHudText(CFFPlayer *pPlayer, const char *pszIdentifier, int x, int y, const char *pszText, int iAlignX = -1, int iAlignY = -1, int iSize = -1);
void FF_LuaHudTimer(CFFPlayer *pPlayer, const char *pszIdentifier, int x, int y, float flStartValue, float flSpeed, int iAlignX = -1, int iAlignY = -1, int iSize = -1);
void FF_LuaHudRemove(CFFPlayer *pPlayer, const char *pszIdentifier);

int UTIL_PickRandomClass(int _curteam);
int UTIL_PickRandomTeam();
int UTIL_GetIntermissionData( Vector *pPosition, QAngle *pAngles );
#endif

bool FF_IsPlayerSpec( CFFPlayer *pPlayer );
bool FF_HasPlayerPickedClass( CFFPlayer *pPlayer );

extern ConVar ffdev_airshot_height_threshold;
#define FFDEV_AIRSHOT_HEIGHT_THRESHOLD ffdev_airshot_height_threshold.GetFloat()

bool FF_IsAirshot( CBaseEntity *pEntity, float flThresholdMultiplier=1.0f );

// Do a HudHint
void FF_HudHint(
#ifndef CLIENT_DLL 
				CFFPlayer *pPlayer,
#endif
				byte bType,
				unsigned short wID,
				const char *pszMessage,
				const char *pszSound = NULL);

// Jiggles: Testing Hint Center


void FF_SendHint(
#ifndef CLIENT_DLL 
				CFFPlayer *pPlayer,
#endif
				unsigned short wID,
				short	HintCount,
				short	HintPriority,
				const char *pszMessage);

// Hint Priority -- if we want, we can add more later
enum HintPriority
{
	PRIORITY_LOW,
	PRIORITY_NORMAL,
	PRIORITY_HIGH
};


// Unique Identifiers for all the hints.
// -- So we can tell whether we've received a hint before
// -- Yeah, this is lengthy, but it works :)
enum HintType
{ 	
	MAP_HINT,			// Generic!  Maps send their messages using the hint system

	// SCOUT
	SCOUT_SPAWN,		// Event: First spawn +5 seconds
	SCOUT_RADAR,		// Event: First radar use
	SCOUT_CONC1,		// Event: First thrown conc
	SCOUT_CONC2,		// Event: Conc hint #2
	SCOUT_TOUCHFLAG,	// Event: First initial touch on a flag
	SCOUT_DEFENDER,		// Event CTF: Scout does not leave his own base(his own teams colored rooms from the lua shit) for 60 seconds
	// SNIPER
	SNIPER_SPAWN,		// Event: First spawn +5 seconds
	SNIPER_SR,			// Event: select sniper rifle
	SNIPER_TAG,			// Event: Player shoots an enemy with a non-lethal shot and proceeds to die before the tagged enemy does 
	SNIPER_LEGSHOT,		// Event: Sniper legshots an enemy
	SNIPER_HEADSHOT,	// Event: Sniper headshots an enemy
	SNIPER_NOCHARGE,	// Event: 3 consecutive uncharged shots from sniper rifle
	// SOLDIER
	SOLDIER_SPAWN,		// Event: First spawn +5 seconds
	SOLDIER_LASERGREN,	// Event: Prime nail grenade
	SOLDIER_SENTRY,		// Event: Enemy sentrygun locks onto you
	SOLDIER_PLAYTIME,	// Event: Player logs 10 minutes of soldier class
	// DEMOMAN
	DEMOMAN_SPAWN,		// Event: First spawn +5 seconds
	DEMOMAN_PL,			// Event: select slot 5 (Pipe Launcher)
	DEMOMAN_FIREPL,		// Event: Firing green/yellow pipes + 10 seconds
	DEMOMAN_GL,			// Event: Select slot 4 (Grenade Launcher)
	DEMOMAN_DETPACK,	// Event: Select slot 6 (Detpack)
	DEMOMAN_SETDET,		// Event: Setting detpack
	// MEDIC
	MEDIC_SPAWN,		// Event: First spawn +5 seconds
	MEDIC_GOHEAL,		// Event: Allied player within 1000 units calls for medic for the first time
	MEDIC_CONC1,		// Event: First thrown conc(this hint should play only once for the player. See identical entry under Scout)
	MEDIC_CONC2,		// Event: Conc hint #2
	MEDIC_NOINFECT,		// Event: Medic tries to infect an enemy player who is Immune.
	// HW
	HWGUY_SPAWN,		// Event: First spawn +5 seconds
	HWGUY_OVERHEAT,		// Event: AC overheats
	HWGUY_SLOWFIELD,	// Event: First thrown slowfield grenade
	// PYRO
	PYRO_SPAWN,			// Event: First spawn +5 seconds
	PYRO_FLAMER,		// Event: Select flamethrower
	PYRO_IC,			// Event: Select IC
	PYRO_ROASTHW,		// Event: Taking AC damage
	PYRO_PLAYTIME,		// Event: + 5 minutes playing as pyro
	// SPY
	SPY_SPAWN,			// Event: First spawn +5 seconds
	SPY_NODISGUISE,		// Event: Player plays for 5 minutes without disguising.
	SPY_GANKDISGUISE,	// Event: Player successfully backstabs from disguise to steal enemy disguise 
	SPY_LOSEDISGUISE,	// Event: Player's disguise is removed by proximity to an enemy spy or scout
	SPY_LOSECLOAK,		// Event: Player's cloak is removed by proximity to an enemy spy
	SPY_SGCLOAK,		// Event: Player, while cloaked, is shot by an SG
	SPY_KNIFE,			// Event: Player selects the knife
	SPY_TRANQ,			// Event: Player selects the tranq gun
	SPY_SPLAT,			// Event: Player falls to a crunching thud(damage taking fall due to +duck not being held)
	// ENGY
	ENGY_SPAWN,			// Event: First spawn +5 seconds
	ENGY_BUILDSG,		// Event: Building a gun
	ENGY_BUILTSG,		// Event: Gun finished building
	ENGY_NOUPGRADE,		// Event: Hitting you or an allies gun with the wrench that is full health, and is upgradable while player has < 130 cells.
	ENGY_BUILDDISP,		// Event: Building a dispenser
	ENGY_BUILTDISP,		// Event: Dispenser finished building
	ENGY_SGDAMAGED,		// Event: Sentry gun damaged 
	ENGY_DISPDAMAGED,	// Event: Dispenser damaged
	ENGY_DISPENEMY,		// Event: Enemy touches dispenser
	ENGY_TEAMSG,		// Event: Teammate engineer building gun w/in 1000 radius 
	ENGY_GOSMACK,		// Event: Allied player within 1000 units calls for engineer/armor for the first time.
	// GLOBAL
	GLOBAL_DEFENDSG,	// Event: When playing as non-engy, teammate builds a sentrygun within 1000 units (same circumstance as an engy hint, class-specific hints should play over general hints if same event)
	GLOBAL_NOPRIME1,	// Event: 2 consecutive unprimed grenades thrown(with +gren1/gren2 commands used)
	GLOBAL_NOPRIME2,	// Event: 2 consecutive unprimed grenades thrown(using the double tap commands) 
	GLOBAL_NOLASTINV,	// Event: Player goes for 10 minutes without issuing the "lastinv" weapon switch command
	GLOBAL_EMPDEATH,	// Event: Player dies from emp explosion
	GLOBAL_GRENDEATH,	// Event: Player dies from a grenade (non-emp, non-projectile) explosion
	GLOBAL_MAP,			// Event: Player plays for 2  minutes (tell the player about using the Map key)
	GLOBAL_CTOC,		// Event: Player plays for 4 minutes (tell the player how to change team and class)

	// INTRO HINT
	INTRO_HINT			// Event: Player's first spawn (introduces the Hint Center)
};

// Jiggles: End Test

#define BEGIN_ENTITY_SPHERE_QUERY( origin, radius ) CBaseEntity *pEntity = NULL; \
	CFFPlayer *pPlayer = NULL; \
	for ( CEntitySphereQuery sphere( GetAbsOrigin(), radius); (pEntity = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity() ) \
	{ \
		if( pEntity->IsPlayer() ) \
			pPlayer = ToFFPlayer( pEntity );

#define END_ENTITY_SPHERE_QUERY( ) }

const char *FF_GetAmmoName(int i);

bool FF_TraceHitWorld( trace_t *pTrace );

#ifdef CLIENT_DLL
char *UTIL_GetFormattedMapName( void );
#endif

//-----------------------------------------------------------------------------
// Some wacky utility functions
//-----------------------------------------------------------------------------
bool FF_IsGrenade( CBaseEntity *pEntity );
bool FF_IsPlayer( CBaseEntity *pEntity );

//-----------------------------------------------------------------------------
// Purpose: This is a trace filter that ignores entities with a certain flag
//-----------------------------------------------------------------------------
class CTraceFilterIgnoreSingleFlag : public CTraceFilter
{
public:
	CTraceFilterIgnoreSingleFlag( unsigned int flag )
		: m_flag( flag )
	{
	}

	virtual void SetPassEntity( const IHandleEntity *pPassEntity ) { m_pPassEnt = pPassEntity; }
	const IHandleEntity *GetPassEntity( void ){ return m_pPassEnt;}

	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		// If the entity has this certain flag, ignore it!
		CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );

		if( pEntity && ( pEntity->GetFlags() & m_flag ) )
			return false;

		if ( pEntity == GetPassEntity() )
			return false;

		return true;
	}

private:
	unsigned int	m_flag;
	const IHandleEntity *m_pPassEnt;
};

class DevConVar
{
private:
	float m_flValue;
	float m_iValue;

	void Init(const char *value)
	{
		m_flValue = atof(value);
		m_iValue = (int) m_flValue;
	}

public:
	DevConVar(const char *name, const char *value, int flags, const char *description) { Init(value); }
	DevConVar(const char *name, const char *value, int flags) { Init(value); }

	int GetFloat() const { return m_flValue; }
	int GetInt() const { return m_iValue; }
	int GetBool() const { return m_iValue != 0; }
};

#ifdef GAME_DLL
// dexter: send the client a message saying yo, stop your primed nade timers (visual and sound)
void FF_SendStopGrenTimerMessage(CFFPlayer *target);
#endif

#endif // FF_UTILS_H
