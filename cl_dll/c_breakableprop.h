//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_BREAKABLEPROP_H
#define C_BREAKABLEPROP_H
#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_BreakableProp : public C_BaseAnimating
{
	typedef C_BaseAnimating BaseClass;
public:
	DECLARE_CLIENTCLASS();

	C_BreakableProp();
	
	virtual void SetFadeMinMax( float fademin, float fademax );

	// Allow entities to perform client-side fades
	virtual unsigned char GetClientSideFade();

	// Copy fade from another breakable prop
	void CopyFadeFrom( C_BreakableProp *pSource );

protected:
	// Networked vars.
	float m_fadeMinDist;
	float m_fadeMaxDist;
	float m_flFadeScale;
};

#endif // C_BREAKABLEPROP_H
