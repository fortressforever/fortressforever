// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_radiotagdata.cpp
// @author Patrick O'Leary (Mulchman) 
// @date 9/20/2006
// @brief Radio tag data wrapper
//
// REVISIONS
// ---------
//	9/20/2006, Mulchman: 
//		First created

#include "cbase.h"
#include "ff_radiotagdata.h"

#ifdef CLIENT_DLL 
#undef CFFRadioTagData
IMPLEMENT_CLIENTCLASS_DT_NOBASE( C_FFRadioTagData, DT_FFRadioTagData, CFFRadioTagData )
	RecvPropArray3( RECVINFO_ARRAY( m_bVisible ), RecvPropInt( RECVINFO( m_bVisible[ 0 ] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_iClass ), RecvPropInt( RECVINFO( m_iClass[ 0 ] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_iTeam ), RecvPropInt( RECVINFO( m_iTeam[ 0 ] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_bDucking ), RecvPropInt( RECVINFO( m_bDucking[ 0 ] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_vecOrigin ), RecvPropVector( RECVINFO( m_vecOrigin[ 0 ] ) ) ),
END_RECV_TABLE()
#define CFFRadioTagData C_FFRadioTagData
#else
IMPLEMENT_SERVERCLASS_ST_NOBASE( CFFRadioTagData, DT_FFRadioTagData )
	SendPropArray3( SENDINFO_ARRAY3( m_bVisible ), SendPropInt( SENDINFO_ARRAY( m_bVisible ), 1, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_iClass ), SendPropInt( SENDINFO_ARRAY( m_iClass ), 5 ) ), // 10 classes fits into 4 bits, plus i think we use -1, need to check though
	SendPropArray3( SENDINFO_ARRAY3( m_iTeam ), SendPropInt( SENDINFO_ARRAY( m_iTeam ), 4 ) ), // teams go from 0-5: none,spec,blue,red,green,yellow, plus i think we use -1, need to check though
	SendPropArray3( SENDINFO_ARRAY3( m_bDucking ), SendPropInt( SENDINFO_ARRAY( m_bDucking ), 1, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_vecOrigin ), SendPropVector( SENDINFO_ARRAY( m_vecOrigin ), 32, SPROP_CHANGES_OFTEN | SPROP_COORD ) ),
END_SEND_TABLE()
#endif

LINK_ENTITY_TO_CLASS( ff_radiotagdata, CFFRadioTagData );

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA_NO_BASE( CFFRadioTagData )

	DEFINE_PRED_ARRAY( m_vecOrigin, FIELD_VECTOR, MAX_PLAYERS + 1, FTYPEDESC_INSENDTABLE ),

END_PREDICTION_DATA()
#endif

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFRadioTagData::CFFRadioTagData( void )
{
#ifdef CLIENT_DLL
	for( int i = 0; i < MAX_PLAYERS + 1; i++ )
	{
		m_bVisible[ i ] = 0;
		m_iClass[ i ] = 0;
		m_iTeam[ i ] = 0;
		m_bDucking[ i ] = 0;
		m_vecOrigin[ i ].Init();
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CFFRadioTagData::~CFFRadioTagData( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Initialize stuff
//-----------------------------------------------------------------------------
void CFFRadioTagData::Spawn( void )
{
#ifdef GAME_DLL
	for( int i = 0; i < MAX_PLAYERS + 1; i++ )
	{
		m_bVisible.Set( i, 0 );
		m_iClass.Set( i, 0 );
		m_iTeam.Set( i, 0 );
		m_bDucking.Set( i, 0 );
		m_vecOrigin.Set( i, Vector( 0, 0, 0 ) );
	}
#endif
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFRadioTagData::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED ) 
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CFFRadioTagData::GetVisible( int iIndex ) const
{
	return m_bVisible[ iIndex ];
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CFFRadioTagData::GetClass( int iIndex ) const
{
	return m_iClass[ iIndex ];
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CFFRadioTagData::GetTeam( int iIndex ) const
{
	return m_iTeam[ iIndex ];
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CFFRadioTagData::GetDucking( int iIndex ) const
{
	return m_bDucking[ iIndex ];
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Vector CFFRadioTagData::GetOrigin( int iIndex ) const
{
	return m_vecOrigin[ iIndex ];
}

#else
//-----------------------------------------------------------------------------
// Purpose: Always transmit to clients
//-----------------------------------------------------------------------------
int CFFRadioTagData::UpdateTransmitState( void )
{
	// always transmit to the client
	return SetTransmitState( FL_EDICT_ALWAYS );
}

//-----------------------------------------------------------------------------
// Purpose: Set visible to false
//-----------------------------------------------------------------------------
void CFFRadioTagData::ClearVisible( void )
{
	for( int i = 0; i < MAX_PLAYERS + 1; i++ )
	{
		m_bVisible.Set( i, 0 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set data for an index
//-----------------------------------------------------------------------------
void CFFRadioTagData::Set( int iIndex, bool bVisible, int iClass, int iTeam, bool bDucking, const Vector& vecOrigin )
{
	m_bVisible.Set( iIndex, bVisible );
	m_iClass.Set( iIndex, iClass );
	m_iTeam.Set( iIndex, iTeam );
	m_bDucking.Set( iIndex, bDucking );
	m_vecOrigin.Set( iIndex, vecOrigin );
}
#endif
