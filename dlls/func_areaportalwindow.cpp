//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "func_areaportalwindow.h"
#include "entitylist.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// The server will still send entities through a window even after it opaque 
// to allow for net lag.
#define FADE_DIST_BUFFER	10


LINK_ENTITY_TO_CLASS( func_areaportalwindow, CFuncAreaPortalWindow );


IMPLEMENT_SERVERCLASS_ST( CFuncAreaPortalWindow, DT_FuncAreaPortalWindow )
	SendPropFloat( SENDINFO(m_flFadeDist), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO(m_flFadeStartDist), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO(m_flTranslucencyLimit), 0, SPROP_NOSCALE ),

	SendPropModelIndex(SENDINFO(m_iBackgroundModelIndex) ),
END_SEND_TABLE()


BEGIN_DATADESC( CFuncAreaPortalWindow )

	DEFINE_KEYFIELD( m_portalNumber, FIELD_INTEGER,	"portalnumber" ),
	DEFINE_KEYFIELD( m_flFadeStartDist,	FIELD_FLOAT,	"FadeStartDist" ),
	DEFINE_KEYFIELD( m_flFadeDist,	FIELD_FLOAT,	"FadeDist" ),
	DEFINE_KEYFIELD( m_flTranslucencyLimit,	FIELD_FLOAT,	"TranslucencyLimit" ),
	DEFINE_KEYFIELD( m_iBackgroundBModelName,FIELD_STRING,	"BackgroundBModel" ),
//	DEFINE_KEYFIELD( m_iBackgroundModelIndex,FIELD_INTEGER ),

END_DATADESC()




CFuncAreaPortalWindow::CFuncAreaPortalWindow()
{
	m_iBackgroundModelIndex = -1;
}


CFuncAreaPortalWindow::~CFuncAreaPortalWindow()
{
}


void CFuncAreaPortalWindow::Spawn()
{
	Precache();

	engine->SetAreaPortalState( m_portalNumber, 1 );
}


void CFuncAreaPortalWindow::Activate()
{
	BaseClass::Activate();
	
	// Find our background model.
	CBaseEntity *pBackground = gEntList.FindEntityByName( NULL, m_iBackgroundBModelName );
	if( pBackground )
	{
		m_iBackgroundModelIndex  = modelinfo->GetModelIndex( STRING( pBackground->GetModelName() ) );
		pBackground->AddEffects( EF_NODRAW ); // we will draw for it.
	}

	// Find our target and steal its bmodel.
	CBaseEntity *pTarget = gEntList.FindEntityByName( NULL, m_target );
	if( pTarget )
	{
		SetModel( STRING(pTarget->GetModelName()) );
		SetAbsOrigin( pTarget->GetAbsOrigin() );
		pTarget->AddEffects( EF_NODRAW ); // we will draw for it.
	}
}


bool CFuncAreaPortalWindow::IsWindowOpen( const Vector &vOrigin, float fovDistanceAdjustFactor )
{
	Vector vecLocalOrigin;
	CollisionProp()->WorldToCollisionSpace( vOrigin, &vecLocalOrigin );
	float flDist = CalcDistanceToAABB( CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), vecLocalOrigin );
	flDist *= fovDistanceAdjustFactor;
	if( flDist > (m_flFadeDist + FADE_DIST_BUFFER) )
	{
		return false;
	}
	else
	{
		return true;
	}
}


bool CFuncAreaPortalWindow::UpdateVisibility( const Vector &vOrigin, float fovDistanceAdjustFactor, bool &bIsOpenOnClient )
{
	if ( IsWindowOpen( vOrigin, fovDistanceAdjustFactor ) )
	{
		return BaseClass::UpdateVisibility( vOrigin, fovDistanceAdjustFactor, bIsOpenOnClient );
	}
	else
	{
		bIsOpenOnClient = false;
		return false;
	}
}

