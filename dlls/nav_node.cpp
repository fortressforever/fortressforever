//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
// nav_node.cpp
// AI Navigation Nodes
// Author: Michael S. Booth (mike@turtlerockstudios.com), January 2003

#include "cbase.h"
#include "nav_node.h"
#include "nav_colors.h"
#include "nav_mesh.h"

NavDirType Opposite[ NUM_DIRECTIONS ] = { SOUTH, WEST, NORTH, EAST };

CNavNode *CNavNode::m_list = NULL;
unsigned int CNavNode::m_listLength = 0;


//--------------------------------------------------------------------------------------------------------------
/**
 * Constructor
 */
CNavNode::CNavNode( const Vector &pos, const Vector &normal, CNavNode *parent )
{
	m_pos = pos;
	m_normal = normal;

	static unsigned int nextID = 1;
	m_id = nextID++;

	for( int i=0; i<NUM_DIRECTIONS; i++ )
		m_to[ i ] = NULL;

	m_visited = 0;
	m_parent = parent;

	m_next = m_list;
	m_list = this;
	m_listLength++;

	m_isCovered = false;
	m_area = NULL;

	m_attributeFlags = 0;
}


//--------------------------------------------------------------------------------------------------------------
#if DEBUG_NAV_NODES
ConVar nav_show_node_id( "nav_show_node_id", "0" );
ConVar nav_test_node( "nav_test_node", "0" );
#endif // DEBUG_NAV_NODES


//--------------------------------------------------------------------------------------------------------------
void CNavNode::Draw( void )
{
#if DEBUG_NAV_NODES
	if ( m_isCovered )
	{
		NDebugOverlay::Cross3D( m_pos, 2, 255, 255, 255, true, 0.1f );
	}
	else
	{
		NDebugOverlay::Cross3D( m_pos, 2, 0, 0, 255, true, 0.1f );
		if ( nav_show_node_id.GetBool() )
		{
			char text[16];
			Q_snprintf( text, sizeof( text ), "%d", m_id );
			NDebugOverlay::Text( m_pos, text, true, 0.1f );
		}

		if ( (unsigned int)(nav_test_node.GetInt()) == m_id )
		{
			TheNavMesh->TestArea( this, 1, 1 );
			nav_test_node.SetValue( 0 );
		}
	}
#endif // DEBUG_NAV_NODES
}


//--------------------------------------------------------------------------------------------------------------
/**
 * Create a connection FROM this node TO the given node, in the given direction
 */
void CNavNode::ConnectTo( CNavNode *node, NavDirType dir )
{
	m_to[ dir ] = node;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return node at given position.
 * @todo Need a hash table to make this lookup fast
 */
CNavNode *CNavNode::GetNode( const Vector &pos )
{
	const float tolerance = 0.45f * GenerationStepSize;			// 1.0f

	for( CNavNode *node = m_list; node; node = node->m_next )
	{
		float dx = fabs( node->m_pos.x - pos.x );
		float dy = fabs( node->m_pos.y - pos.y );
		float dz = fabs( node->m_pos.z - pos.z );

		if (dx < tolerance && dy < tolerance && dz < tolerance)
			return node;
	}

	return NULL;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if this node is bidirectionally linked to 
 * another node in the given direction
 */
BOOL CNavNode::IsBiLinked( NavDirType dir ) const
{
	if (m_to[ dir ] && m_to[ dir ]->m_to[ Opposite[dir] ] == this)
	{
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return true if this node is the NW corner of a quad of nodes
 * that are all bidirectionally linked.
 */
BOOL CNavNode::IsClosedCell( void ) const
{
	if (IsBiLinked( SOUTH ) &&
		IsBiLinked( EAST ) &&
		m_to[ EAST ]->IsBiLinked( SOUTH ) &&
		m_to[ SOUTH ]->IsBiLinked( EAST ) &&
		m_to[ EAST ]->m_to[ SOUTH ] == m_to[ SOUTH ]->m_to[ EAST ])
	{
		return true;
	}

	return false;
}

