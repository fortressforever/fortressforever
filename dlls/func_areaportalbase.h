//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef FUNC_AREAPORTALBASE_H
#define FUNC_AREAPORTALBASE_H
#ifdef _WIN32
#pragma once
#endif


#include "baseentity.h"
#include "utllinkedlist.h"


// Shared stuff between door portals and window portals.
class CFuncAreaPortalBase : public CBaseEntity
{
	DECLARE_CLASS( CFuncAreaPortalBase, CBaseEntity );
public:
	DECLARE_DATADESC();

					CFuncAreaPortalBase();
	virtual			~CFuncAreaPortalBase();

	// Areaportals must be placed in each map for preprocess, they can't use transitions
	virtual int	ObjectCaps( void ) { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	
	// This is called each frame for each client to all portals to close 
	// when the viewer is far enough away, or on the backside.
	//
	// The default implementation closes the portal if the viewer (plus some padding)
	// is on the backside of the portal. Return false if you close the portal.
	virtual bool	UpdateVisibility( const Vector &vOrigin, float fovDistanceAdjustFactor );


public:
	
	int				m_portalNumber;


private:
	
	unsigned short	m_AreaPortalsElement;	// link into g_AreaPortals.
};


extern CUtlLinkedList<CFuncAreaPortalBase*, unsigned short> g_AreaPortals;



#endif // FUNC_AREAPORTALBASE_H
