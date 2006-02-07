//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef ANIMATIONLAYER_H
#define ANIMATIONLAYER_H
#ifdef _WIN32
#pragma once
#endif


#include "rangecheckedvar.h"
#include "lerp_functions.h"


class C_AnimationLayer
{
public:

	// This allows the datatables to access private members.
	ALLOW_DATATABLES_PRIVATE_ACCESS();

	C_AnimationLayer();

	void SetOrder( int order );

public:

	CRangeCheckedVar<int, -1, 1000, 0>	nSequence;
	CRangeCheckedVar<float, -2, 2, 0>	flPrevCycle;
	CRangeCheckedVar<float, -5, 5, 0>	flWeight;
	int		nOrder;

	// used for automatic crossfades between sequence changes
	CRangeCheckedVar<float, -50, 50, 1>		flPlaybackrate;
	CRangeCheckedVar<float, -2, 2, 0>		flCycle;

	float	flAnimtime;
	float	flFadeOuttime;
};
#ifdef CLIENT_DLL
	#define CAnimationLayer C_AnimationLayer
#endif


inline C_AnimationLayer::C_AnimationLayer()
{
	nSequence = 0;
	flPrevCycle = 0;
	flWeight = 0;
	flPlaybackrate = 0;
	flCycle = 0;
	flAnimtime = 0;
	flFadeOuttime = 0;
}


inline void C_AnimationLayer::SetOrder( int order )
{
	nOrder = order;
}


inline C_AnimationLayer LoopingLerp( float flPercent, C_AnimationLayer from, C_AnimationLayer to )
{
	C_AnimationLayer output;

	output.nSequence = to.nSequence;
	output.flCycle = LoopingLerp( flPercent, (float)from.flCycle, (float)to.flCycle );
	output.flPrevCycle = to.flPrevCycle;
	output.flWeight = Lerp( flPercent, from.flWeight, to.flWeight );
	output.nOrder = to.nOrder;

	output.flAnimtime = to.flAnimtime;
	output.flFadeOuttime = to.flFadeOuttime;
	return output;
}

inline C_AnimationLayer Lerp( float flPercent, const C_AnimationLayer& from, const C_AnimationLayer& to )
{
	C_AnimationLayer output;

	output.nSequence = to.nSequence;
	output.flCycle = Lerp( flPercent, from.flCycle, to.flCycle );
	output.flPrevCycle = to.flPrevCycle;
	output.flWeight = Lerp( flPercent, from.flWeight, to.flWeight );
	output.nOrder = to.nOrder;

	output.flAnimtime = to.flAnimtime;
	output.flFadeOuttime = to.flFadeOuttime;
	return output;
}

inline C_AnimationLayer LoopingLerp_Hermite( float flPercent, C_AnimationLayer prev, C_AnimationLayer from, C_AnimationLayer to )
{
	C_AnimationLayer output;

	output.nSequence = to.nSequence;
	output.flCycle = LoopingLerp_Hermite( flPercent, (float)prev.flCycle, (float)from.flCycle, (float)to.flCycle );
	output.flPrevCycle = to.flPrevCycle;
	output.flWeight = Lerp( flPercent, from.flWeight, to.flWeight );
	output.nOrder = to.nOrder;

	output.flAnimtime = to.flAnimtime;
	output.flFadeOuttime = to.flFadeOuttime;
	return output;
}

// YWB:  Specialization for interpolating euler angles via quaternions...
inline C_AnimationLayer Lerp_Hermite( float flPercent, const C_AnimationLayer& prev, const C_AnimationLayer& from, const C_AnimationLayer& to )
{
	C_AnimationLayer output;

	output.nSequence = to.nSequence;
	output.flCycle = Lerp_Hermite( flPercent, prev.flCycle, from.flCycle, to.flCycle );
	output.flPrevCycle = to.flPrevCycle;
	output.flWeight = Lerp( flPercent, from.flWeight, to.flWeight );
	output.nOrder = to.nOrder;

	output.flAnimtime = to.flAnimtime;
	output.flFadeOuttime = to.flFadeOuttime;
	return output;
}

inline void Lerp_Clamp( C_AnimationLayer &val )
{
	Lerp_Clamp( val.nSequence );
	Lerp_Clamp( val.flCycle );
	Lerp_Clamp( val.flPrevCycle );
	Lerp_Clamp( val.flWeight );
	Lerp_Clamp( val.nOrder );
	Lerp_Clamp( val.flAnimtime );
	Lerp_Clamp( val.flFadeOuttime );
}

#endif // ANIMATIONLAYER_H
