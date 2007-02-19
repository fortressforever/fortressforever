/// =============== Fortress Forever ==============
/// ======== A modifcation for Half-Life 2 ========
///
/// @file ff_rail_effects.h
/// @author Jon "trepid_jon" Day
/// @date February 17, 2007
/// @brief Declaration of the client rail effects class
///
/// REVISIONS
/// ---------
///
/// 

#ifndef FF_RAIL_EFFECTS_H
#define FF_RAIL_EFFECTS_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "utlvector.h"

#define CFFRailEffects C_FFRailEffects

// this is mainly for changing colors at each bounce (and effects, I guess)
enum
{
	RAIL_KEYFRAME_TYPE_DEFAULT = 0,
	RAIL_KEYFRAME_TYPE_END,
	RAIL_KEYFRAME_TYPE_START,
	RAIL_KEYFRAME_TYPE_BOUNCE1,
	RAIL_KEYFRAME_TYPE_BOUNCE2,
};

struct RailKeyframe
{
	Vector pos;
	int type;

	RailKeyframe()
	{
		pos = Vector(0,0,0);
		type = RAIL_KEYFRAME_TYPE_DEFAULT;
	}

	RailKeyframe(Vector newVec, int newType)
	{
		pos = newVec;
		type = newType;
	}

	RailKeyframe& operator=( const RailKeyframe& src )
	{
		if ( this == &src )
			return *this;

		pos = src.pos;
		type = src.type;

		return *this;
	}
};

class CFFRailEffects : public CBaseEntity
{
public:
	DECLARE_CLIENTCLASS();
	DECLARE_CLASS( CFFRailEffects, CBaseEntity );

	CFFRailEffects();
	virtual void Release();
	virtual void Precache( void );
	virtual void Spawn( void );
	virtual bool ShouldDraw( void ) { return true; }
	virtual RenderGroup_t GetRenderGroup() { return RENDER_GROUP_TRANSLUCENT_ENTITY; }
	virtual void GetRenderBounds( Vector& mins, Vector& maxs );
	virtual int DrawModel( int flags );

	CUtlVector<RailKeyframe> m_Keyframes;

	bool m_bTimeToDie;
	float m_flDieTimer;
};

#endif // FF_RAIL_EFFECTS_H
