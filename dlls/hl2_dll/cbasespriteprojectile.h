//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Base class for simple projectiles
//
// $NoKeywords: $
//=============================================================================//

#ifndef CBASESPRITEPROJECTILE_H
#define CBASESPRITEPROJECTILE_H
#ifdef _WIN32
#pragma once
#endif

#include "Sprite.h"

enum MoveType_t;
enum MoveCollide_t;


//=============================================================================
//=============================================================================
class CBaseSpriteProjectile : public CSprite
{
	DECLARE_DATADESC();
	DECLARE_CLASS( CBaseSpriteProjectile, CSprite );

public:
	void Touch( CBaseEntity *pOther );

	void CBaseSpriteProjectile::Spawn(	char *pszModel,
									const Vector &vecOrigin,
									const Vector &vecVelocity,
									edict_t *pOwner,
									MoveType_t	iMovetype,
									MoveCollide_t nMoveCollide,
									int	iDamage,
									int iDamageType );

	virtual void Precache( void ) {};

	int	m_iDmg;
	int m_iDmgType;
};

#endif // CBASESPRITEPROJECTILE_H
