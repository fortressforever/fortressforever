// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_radiotagdata.h
// @author Patrick O'Leary (Mulchman) 
// @date 9/20/2006
// @brief Radio tag data wrapper
//
// REVISIONS
// ---------
//	9/20/2006, Mulchman: 
//		First created

#ifndef FF_RADIOTAGDATA_H
#define FF_RADIOTAGDATA_H

#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

#ifdef CLIENT_DLL 
	#define CFFRadioTagData C_FFRadioTagData
#endif

//=============================================================================
//
// Class CFFRadioTagData
//
//=============================================================================
class CFFRadioTagData : public CBaseEntity
{
public:	
	DECLARE_CLASS( CFFRadioTagData, CBaseEntity );
#ifdef CLIENT_DLL 
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
#else
	DECLARE_SERVERCLASS();
#endif	

	CFFRadioTagData( void );
	~CFFRadioTagData( void );

	virtual void	Spawn( void );

#ifdef CLIENT_DLL
	virtual void	OnDataChanged( DataUpdateType_t updateType );
	bool			GetVisible( int iIndex ) const;
	int				GetClass( int iIndex ) const;
	int				GetTeam( int iIndex ) const;
	bool			GetDucking( int iIndex ) const;
	Vector			GetOrigin( int iIndex ) const;
#else
	virtual int		UpdateTransmitState( void );	
	virtual	int		ObjectCaps( void ) { return BaseClass::ObjectCaps() | FCAP_DONT_SAVE; }
	void			ClearVisible( void );
	void			Set( int iIndex, bool bVisible, int iClass, int iTeam, bool bDucking, const Vector& vecOrigin );
#endif

protected:
#ifdef CLIENT_DLL 
	bool	m_bVisible[ MAX_PLAYERS + 1 ];
	int		m_iClass[ MAX_PLAYERS + 1 ];
	int		m_iTeam[ MAX_PLAYERS + 1 ];
	bool	m_bDucking[ MAX_PLAYERS + 1 ];
	Vector	m_vecOrigin[ MAX_PLAYERS + 1 ];
#else
	CNetworkArray( int, m_bVisible, MAX_PLAYERS + 1 );
	CNetworkArray( int, m_iClass, MAX_PLAYERS + 1 );
	CNetworkArray( int, m_iTeam, MAX_PLAYERS + 1 );
	CNetworkArray( int, m_bDucking, MAX_PLAYERS + 1 );
	CNetworkArray( Vector, m_vecOrigin, MAX_PLAYERS + 1 );
#endif // CLIENT_DLL

};

#endif // FF_RADIOTAGDATA_H
