//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		A link that can be turned on and off.  Unlike normal links
//				dyanimc links must be entities so they can receive messages.
//				They update the state of the actual links.  Allows us to save
//				a lot of memory by not making all links into entities
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "collisionutils.h"
#include "ai_dynamiclink.h"
#include "ai_node.h"
#include "ai_link.h"
#include "ai_network.h"
#include "ai_networkmanager.h"
#include "saverestore_utlvector.h"
#include "editor_sendcommand.h"
#include "bitstring.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------

LINK_ENTITY_TO_CLASS(info_node_link_controller, CAI_DynamicLinkController);

BEGIN_DATADESC( CAI_DynamicLinkController )

	DEFINE_KEYFIELD( m_nLinkState, FIELD_INTEGER, "initialstate" ),
	DEFINE_KEYFIELD( m_strAllowUse, FIELD_STRING, "AllowUse" ),
	//				 m_ControlledLinks (rebuilt)

	DEFINE_INPUTFUNC( FIELD_VOID, "TurnOn", InputTurnOn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "TurnOff", InputTurnOff ),

END_DATADESC()


void CAI_DynamicLinkController::GenerateLinksFromVolume()
{
	Assert( m_ControlledLinks.Count() == 0 );

	int nNodes = g_pBigAINet->NumNodes();
	CAI_Node **ppNodes = g_pBigAINet->AccessNodes();

	const float MinDistCareSq = Square(MAX_NODE_LINK_DIST + 0.1);

	const Vector &origin = WorldSpaceCenter();
	Vector vAbsMins, vAbsMaxs;
	CollisionProp()->WorldSpaceAABB( &vAbsMins, &vAbsMaxs );
	vAbsMins -= Vector( 1, 1, 1 );
	vAbsMaxs += Vector( 1, 1, 1 );

	for ( int i = 0; i < nNodes; i++ )
	{
		CAI_Node *pNode = ppNodes[i];
		const Vector &nodeOrigin = pNode->GetOrigin();
		if ( origin.DistToSqr(nodeOrigin) < MinDistCareSq )
		{
			int nLinks = pNode->NumLinks();
			for ( int j = 0; j < nLinks; j++ )
			{
				CAI_Link *pLink = pNode->GetLinkByIndex( j );
				int iLinkDest = pLink->DestNodeID( i );
				if ( iLinkDest > i )
				{
					const Vector &originOther = ppNodes[iLinkDest]->GetOrigin();
					if ( origin.DistToSqr(originOther) < MinDistCareSq )
					{
						if ( IsBoxIntersectingRay( vAbsMins, vAbsMaxs, nodeOrigin, originOther - nodeOrigin ) )
						{
							Assert( IsBoxIntersectingRay( vAbsMins, vAbsMaxs, originOther, nodeOrigin - originOther ) );

							CAI_DynamicLink *pLink = (CAI_DynamicLink *)CreateEntityByName( "info_node_link" );
							pLink->m_nSrcID = i;
							pLink->m_nDestID = iLinkDest;
							pLink->m_nSrcEditID = g_pAINetworkManager->GetEditOps()->GetWCIdFromNodeId( pLink->m_nSrcID );
							pLink->m_nDestEditID = g_pAINetworkManager->GetEditOps()->GetWCIdFromNodeId( pLink->m_nDestID );
							pLink->m_nLinkState = m_nLinkState;
							pLink->m_strAllowUse = m_strAllowUse;
							pLink->m_bFixedUpIds = true;
							pLink->m_bNotSaved = true;

							pLink->Spawn();
							m_ControlledLinks.AddToTail( pLink );
						}
					}
				}
			}
		}
	}
}

void CAI_DynamicLinkController::InputTurnOn( inputdata_t &inputdata )
{
	for ( int i = 0; i < m_ControlledLinks.Count(); i++ )
	{
		if ( m_ControlledLinks[i] == NULL )
		{
			m_ControlledLinks.FastRemove(i);
			if ( i >= m_ControlledLinks.Count() )
				break;
		}
		m_ControlledLinks[i]->InputTurnOn( inputdata );
	}

	m_nLinkState = LINK_ON;
}

void CAI_DynamicLinkController::InputTurnOff( inputdata_t &inputdata )
{
	for ( int i = 0; i < m_ControlledLinks.Count(); i++ )
	{
		if ( m_ControlledLinks[i] == NULL )
		{
			m_ControlledLinks.FastRemove(i);
			if ( i >= m_ControlledLinks.Count() )
				break;
		}
		m_ControlledLinks[i]->InputTurnOff( inputdata );
	}

	m_nLinkState = LINK_OFF;
}

//-----------------------------------------------------------------------------

LINK_ENTITY_TO_CLASS(info_node_link, CAI_DynamicLink);

BEGIN_DATADESC( CAI_DynamicLink )

	//								m_pNextDynamicLink
	DEFINE_KEYFIELD( m_nLinkState, FIELD_INTEGER, "initialstate" ),
	DEFINE_KEYFIELD( m_nSrcEditID,	FIELD_INTEGER, "startnode" ),
	DEFINE_KEYFIELD( m_nDestEditID,	FIELD_INTEGER, "endnode" ),
	//				m_nSrcID (rebuilt)
	//				m_nDestID (rebuilt)
	DEFINE_KEYFIELD( m_strAllowUse, FIELD_STRING, "AllowUse" ),
	//				m_bFixedUpIds (part of rebuild)
	//				m_bNotSaved (rebuilt)

	DEFINE_INPUTFUNC( FIELD_VOID, "TurnOn", InputTurnOn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "TurnOff", InputTurnOff ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Init static variables
//-----------------------------------------------------------------------------
CAI_DynamicLink *CAI_DynamicLink::m_pAllDynamicLinks = NULL;
bool CAI_DynamicLink::gm_bInitialized;


//------------------------------------------------------------------------------

void CAI_DynamicLink::GenerateControllerLinks()
{
	CAI_DynamicLinkController *pController = NULL;
	while ( ( pController = gEntList.NextEntByClass( pController ) ) != NULL )
	{
		pController->GenerateLinksFromVolume();
	}
	
}

//------------------------------------------------------------------------------
// Purpose : Initializes src and dest IDs for all dynamic links
//			 	
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CAI_DynamicLink::InitDynamicLinks(void)
{
	if (!g_pAINetworkManager->GetEditOps()->m_pNodeIndexTable)
	{
		DevMsg("ERROR: Trying initialize links with no WC ID table!\n");
		return;
	}

	if ( gm_bInitialized )
		return;

	gm_bInitialized = true;

	bool bUpdateZones = false;
	
	GenerateControllerLinks();

	CAI_DynamicLink* pDynamicLink = CAI_DynamicLink::m_pAllDynamicLinks;

	while (pDynamicLink)
	{
		// -------------------------------------------------------------
		//  First convert this links WC IDs to engine IDs
		// -------------------------------------------------------------
		if ( !pDynamicLink->m_bFixedUpIds )
		{
			int	nSrcID = g_pAINetworkManager->GetEditOps()->GetNodeIdFromWCId( pDynamicLink->m_nSrcEditID );
			if (nSrcID == -1)
			{
				DevMsg( "ERROR: Dynamic link source WC node %d not found\n", pDynamicLink->m_nSrcEditID );
				nSrcID = NO_NODE;
			}

			int	nDestID = g_pAINetworkManager->GetEditOps()->GetNodeIdFromWCId( pDynamicLink->m_nDestEditID );
			if (nDestID == -1)
			{
				DevMsg( "ERROR: Dynamic link dest WC node %d not found\n", pDynamicLink->m_nDestEditID );
				nDestID = NO_NODE;
			}
			
			pDynamicLink->m_nSrcID  = nSrcID;
			pDynamicLink->m_nDestID  = nDestID;
			pDynamicLink->m_bFixedUpIds = true;
		}

		if ( pDynamicLink->m_nSrcID != NO_NODE && pDynamicLink->m_nDestID != NO_NODE )
		{
			if (  ( pDynamicLink->GetSpawnFlags() & bits_HULL_BITS_MASK ) != 0 )
			{
				CAI_Link *pLink = pDynamicLink->FindLink();
				if ( !pLink )
				{
					CAI_Node *pNode1, *pNode2;

					pNode1 = g_pBigAINet->GetNode( pDynamicLink->m_nSrcID );
					pNode2 = g_pBigAINet->GetNode( pDynamicLink->m_nDestID );

					if ( pNode1 && pNode2 )
					{
						pLink = g_pBigAINet->CreateLink( pDynamicLink->m_nSrcID, pDynamicLink->m_nDestID );
						if ( !pLink )
							DevMsg( "Failed to create dynamic link (%d <--> %d)\n", pDynamicLink->m_nSrcEditID, pDynamicLink->m_nDestEditID );
					}

				}

				if ( pLink )
				{
					bUpdateZones = true;

					int hullBits = ( pDynamicLink->GetSpawnFlags() & bits_HULL_BITS_MASK );
					for ( int i = 0; i < NUM_HULLS; i++ )
					{
						if ( hullBits & ( 1 << i ) )
							pLink->m_iAcceptedMoveTypes[i] = bits_CAP_MOVE_GROUND;
					}
				}
			}

			// Now set the link's state
			pDynamicLink->SetLinkState();

			// Go on to the next dynamic link
			pDynamicLink = pDynamicLink->m_pNextDynamicLink;
		}
		else
		{
			CAI_DynamicLink *pBadDynamicLink = pDynamicLink;

			// Go on to the next dynamic link
			pDynamicLink = pDynamicLink->m_pNextDynamicLink;

			UTIL_RemoveImmediate( pBadDynamicLink );
		}

	}

	if ( bUpdateZones )
	{
		g_AINetworkBuilder.InitZones( g_pBigAINet );
	}
}


//------------------------------------------------------------------------------
// Purpose : Goes through each dynamic link and updates the state of all
//			 AINetwork links	
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CAI_DynamicLink::ResetDynamicLinks(void)
{
	CAI_DynamicLink* pDynamicLink = CAI_DynamicLink::m_pAllDynamicLinks;

	while (pDynamicLink)
	{
		// Now set the link's state
		pDynamicLink->SetLinkState();

		// Go on to the next dynamic link
		pDynamicLink = pDynamicLink->m_pNextDynamicLink;
	}
}


//------------------------------------------------------------------------------
// Purpose : Goes through each dynamic link and checks to make sure that
//			 there is still a corresponding node link, if not removes it
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CAI_DynamicLink::PurgeDynamicLinks(void)
{
	CAI_DynamicLink* pDynamicLink = CAI_DynamicLink::m_pAllDynamicLinks;

	while (pDynamicLink)
	{
		if (!pDynamicLink->IsLinkValid())
		{
			// Didn't find the link, so remove it
#ifdef _WIN32
			int nWCSrcID = g_pAINetworkManager->GetEditOps()->m_pNodeIndexTable[pDynamicLink->m_nSrcID];
			int nWCDstID = g_pAINetworkManager->GetEditOps()->m_pNodeIndexTable[pDynamicLink->m_nDestID];
			int	status	 = Editor_DeleteNodeLink(nWCSrcID, nWCDstID, false);
			if (status == Editor_BadCommand)
			{
				DevMsg( "Worldcraft failed in PurgeDynamicLinks...\n" );
			}
#endif
			// Safe to remove it here as this happens only after I leave this function
			UTIL_Remove(pDynamicLink);
		}

		// Go on to the next dynamic link
		pDynamicLink = pDynamicLink->m_pNextDynamicLink;
	}
}

//------------------------------------------------------------------------------
// Purpose : Returns false if the dynamic link doesn't have a corresponding
//			 node link
// Input   :
// Output  :
//------------------------------------------------------------------------------
bool CAI_DynamicLink::IsLinkValid( void )
{
	CAI_Node *pNode = g_pBigAINet->GetNode(m_nSrcID);

	return ( pNode->GetLink( m_nDestID ) != NULL );
}


//------------------------------------------------------------------------------
// Purpose :
//------------------------------------------------------------------------------
void CAI_DynamicLink::InputTurnOn( inputdata_t &inputdata )
{
	if (m_nLinkState == LINK_OFF)
	{
		m_nLinkState = LINK_ON;
		CAI_DynamicLink::SetLinkState();
	}
}


//------------------------------------------------------------------------------
// Purpose :
//------------------------------------------------------------------------------
void CAI_DynamicLink::InputTurnOff( inputdata_t &inputdata )
{
	if (m_nLinkState == LINK_ON)
	{
		m_nLinkState = LINK_OFF;
		CAI_DynamicLink::SetLinkState();
	}
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CAI_Link *CAI_DynamicLink::FindLink()
{
	CAI_Node *	pSrcNode = g_pBigAINet->GetNode(m_nSrcID, false);
	if ( pSrcNode )
	{
		int	numLinks = pSrcNode->NumLinks();
		for (int i=0;i<numLinks;i++)
		{
			CAI_Link* pLink = pSrcNode->GetLinkByIndex(i);

			if (((pLink->m_iSrcID  == m_nSrcID )&&
				 (pLink->m_iDestID == m_nDestID))   ||

				((pLink->m_iSrcID  == m_nDestID)&&
				 (pLink->m_iDestID == m_nSrcID ))   )
			{
				return pLink;
			}
		}
	}
	return NULL;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int CAI_DynamicLink::ObjectCaps()
{
	int caps = BaseClass::ObjectCaps();

	if ( m_bNotSaved )
		caps |= FCAP_DONT_SAVE;

	return caps;
}

//------------------------------------------------------------------------------
// Purpose : Updates network link state if dynamic link state has changed
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CAI_DynamicLink::SetLinkState(void)
{
	if (m_nSrcID == NO_NODE || m_nDestID == NO_NODE)
	{
		DevMsg("ERROR: Dynamic link pointing to invalid node ID!!\n");
		return;
	}

	// ------------------------------------------------------------------
	// Now update the node links...
	//  Nodes share links so we only have to find the node from the src 
	//  For now just using one big AINetwork so find src node on that network
	// ------------------------------------------------------------------
	CAI_Node *	pSrcNode = g_pBigAINet->GetNode(m_nSrcID, false);
	if ( pSrcNode )
	{
		CAI_Link* pLink = FindLink();
		if ( pLink )
		{
			pLink->m_pDynamicLink = this;
			if (m_nLinkState == LINK_OFF)
			{
				pLink->m_LinkInfo |=  bits_LINK_OFF;
			}
			else
			{
				pLink->m_LinkInfo &= ~bits_LINK_OFF;
			}
		}
		else
		{
			DevMsg("Error: info_node_link unable to form between nodes %d and %d\n", m_nSrcID, m_nDestID );
		}
	}

}

//------------------------------------------------------------------------------
// Purpose : Given two node ID's return the related dynamic link if any or NULL
//			 	
// Input   :
// Output  :
//------------------------------------------------------------------------------
CAI_DynamicLink* CAI_DynamicLink::GetDynamicLink(int nSrcID, int nDstID)
{
	CAI_DynamicLink* pDynamicLink = CAI_DynamicLink::m_pAllDynamicLinks;

	while (pDynamicLink)
	{
		if ((nSrcID == pDynamicLink->m_nSrcID  && nDstID == pDynamicLink->m_nDestID) ||
			(nSrcID == pDynamicLink->m_nDestID && nDstID == pDynamicLink->m_nSrcID ) ) 
		{
			return pDynamicLink;
		}

		// Go on to the next dynamic link
		pDynamicLink = pDynamicLink->m_pNextDynamicLink;
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
// Input  :
// Output :
//-----------------------------------------------------------------------------
CAI_DynamicLink::CAI_DynamicLink(void)
{
	m_bFixedUpIds		= false;
	m_bNotSaved			= false;
	m_nSrcID			= NO_NODE;
	m_nDestID			= NO_NODE;
	m_nLinkState		= LINK_OFF;

	// -------------------------------------
	//  Add to linked list of dynamic links
	// -------------------------------------
	m_pNextDynamicLink = CAI_DynamicLink::m_pAllDynamicLinks;
	CAI_DynamicLink::m_pAllDynamicLinks = this;
};


//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
CAI_DynamicLink::~CAI_DynamicLink(void) {

	// ----------------------------------------------
	//  Remove from linked list of all dynamic links
	// ----------------------------------------------
	CAI_DynamicLink* pDynamicLink = CAI_DynamicLink::m_pAllDynamicLinks;
	if (pDynamicLink == this)
	{
		m_pAllDynamicLinks = pDynamicLink->m_pNextDynamicLink;
	}
	else
	{
		while (pDynamicLink)
		{
			if (pDynamicLink->m_pNextDynamicLink == this)
			{
				pDynamicLink->m_pNextDynamicLink = pDynamicLink->m_pNextDynamicLink->m_pNextDynamicLink;
				break;
			}
			pDynamicLink = pDynamicLink->m_pNextDynamicLink;
		}
	}
}

