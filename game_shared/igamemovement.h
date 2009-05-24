//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#if !defined( IGAMEMOVEMENT_H )
#define IGAMEMOVEMENT_H

#ifdef _WIN32
#pragma once
#endif

#include "vector.h"
#include "interface.h"
#include "imovehelper.h"
#include "const.h"

//-----------------------------------------------------------------------------
// Name of the class implementing the game movement.
//-----------------------------------------------------------------------------

#define INTERFACENAME_GAMEMOVEMENT	"GameMovement001"

//-----------------------------------------------------------------------------
// Forward declarations.
//-----------------------------------------------------------------------------

class IMoveHelper;

//-----------------------------------------------------------------------------
// Purpose: Encapsulated input parameters to player movement.
//-----------------------------------------------------------------------------

class CMoveData
{
public:
	bool			m_bFirstRunOfFunctions;

	EntityHandle_t	m_nPlayerHandle;	// edict index on server, client entity handle on client

	int				m_nImpulseCommand;	// Impulse command issued.
	QAngle			m_vecViewAngles;	// Command view angles (local space)
	QAngle			m_vecAbsViewAngles;	// Command view angles (world space)
	int				m_nButtons;			// Attack buttons.
	int				m_nOldButtons;		// From host_client->oldbuttons;
	float			m_flForwardMove;
	float			m_flSideMove;
	float			m_flUpMove;
	
	float			m_flMaxSpeed;
	float			m_flClientMaxSpeed;

	// Variables from the player edict (sv_player) or entvars on the client.
	// These are copied in here before calling and copied out after calling.
	Vector			m_vecVelocity;		// edict::velocity		// Current movement direction.
	QAngle			m_vecAngles;		// edict::angles
	QAngle			m_vecOldAngles;
	Vector			m_vecAbsOrigin;		// edict::origin
	
// Output only
	float			m_outStepHeight;	// how much you climbed this move
	Vector			m_outWishVel;		// This is where you tried 
	Vector			m_outJumpVel;		// This is your jump velocity

	// Movement constraints	(radius 0 means no constraint)
	Vector			m_vecConstraintCenter;
	float			m_flConstraintRadius;
	float			m_flConstraintWidth;
	float			m_flConstraintSpeedFactor;
};


//-----------------------------------------------------------------------------
// Purpose: The basic player movement interface
//-----------------------------------------------------------------------------

abstract_class IGameMovement
{
public:
	virtual			~IGameMovement( void ) {}
	
	// Process the current movement command
	virtual void	ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMove ) = 0;		

	// Allows other parts of the engine to find out the normal and ducked player bbox sizes
	virtual Vector const&	GetPlayerMins( bool ducked ) const = 0;
	virtual Vector const&	GetPlayerMaxs( bool ducked ) const = 0;
	virtual Vector const&   GetPlayerViewOffset( bool ducked ) const = 0;
};


#endif // IGAMEMOVEMENT_H
