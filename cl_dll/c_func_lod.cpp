//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "view.h"
#include "iviewrender.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern Vector g_vecRenderOrigin;
extern ConVar r_DoCovertTransitions;


class C_Func_LOD : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_Func_LOD, C_BaseEntity );
	DECLARE_CLIENTCLASS();

					C_Func_LOD();

// C_BaseEntity overrides.
public:

	virtual bool	LODTest();
	unsigned char	GetClientSideFade();

public:
	bool						m_bLastDrawn;			// Was this entity drawn last frame?
	enum {Visible,Invisible}	m_LastRegion;			// Which region was the entity last in?
														// This is used to determine the behavior while in the transition region
														// (ie: should it transition to visible or invisible?)

// Replicated vars from the server.
// These are documented in the server-side entity.
public:
	float			m_fDisappearDist;
};


ConVar lod_TransitionDist("lod_TransitionDist", "800");
ConVar lod_Enable("lod_Enable", "1");


// ------------------------------------------------------------------------- //
// Tables.
// ------------------------------------------------------------------------- //

// Datatable..
IMPLEMENT_CLIENTCLASS_DT(C_Func_LOD, DT_Func_LOD, CFunc_LOD)
	RecvPropFloat(RECVINFO(m_fDisappearDist)),
END_RECV_TABLE()



// ------------------------------------------------------------------------- //
// C_Func_LOD implementation.
// ------------------------------------------------------------------------- //

C_Func_LOD::C_Func_LOD()
{
	m_bLastDrawn = false;
	m_LastRegion = Invisible;
	m_fDisappearDist = 5000.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Calculate a fade.
//-----------------------------------------------------------------------------
unsigned char C_Func_LOD::GetClientSideFade()
{
	return UTIL_ComputeEntityFade( this, m_fDisappearDist, m_fDisappearDist + lod_TransitionDist.GetFloat(), 1.0f );
}

bool C_Func_LOD::LODTest()
{
	if(!lod_Enable.GetInt())
		return true;

	float transitionDist = m_fDisappearDist;
	float disappearDist  = m_fDisappearDist + lod_TransitionDist.GetFloat();

	float dist = (CurrentViewOrigin() - WorldSpaceCenter()).Length();
	// If they are zoomed in, shorten the distance by the ratio
	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();
	if ( local )
	{
		dist *= local->GetFOVDistanceAdjustFactor();
	}
	if(dist < transitionDist)
	{
		// Always visible between 0 and transitionDist.
		m_LastRegion = Visible;
		m_bLastDrawn = true;
	}
	else if(dist < disappearDist)
	{
		// In the transition range. First, figure out if we want to transition at all.
		bool bTransition = false;

		// See if they're rotating their view fast enough.
		if(r_DoCovertTransitions.GetInt())
		{
			bTransition = true;
		}
		else
		{
			// Since this is a brushmodel which isn't rotating, it should be close enough...
			Vector vecRenderMins, vecRenderMaxs;
			GetRenderBounds( vecRenderMins, vecRenderMaxs );
			Vector vCenter = (vecRenderMins + vecRenderMaxs) * 0.5f;
			float radius = (vecRenderMaxs - vCenter).Length();
			vCenter += GetRenderOrigin();
			bTransition = R_CullSphere(view->GetFrustum(), 5, &vCenter, radius);
		}
			
		if(bTransition)
		{
			// Ok, do the transition while they're not looking.
			if(m_LastRegion == Visible && m_bLastDrawn)
			{
				// Make it invisible.
				m_bLastDrawn = false;
			}
			else if(m_LastRegion == Invisible && !m_bLastDrawn)
			{
				// Make it visible.
				m_bLastDrawn = true;
			}
		}
	}
	else
	{
		// Never visible from disappearDist+.
		m_LastRegion = Invisible;
		m_bLastDrawn = false;
	}

	return m_bLastDrawn;
}

