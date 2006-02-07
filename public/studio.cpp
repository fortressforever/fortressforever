//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "studio.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

void virtualmodel_t::AppendSequences( int group, const studiohdr_t *pStudioHdr )
{
	int numCheck = m_seq.Count();

	int j, k;

	m_group[ group ].masterSeq.SetCount( pStudioHdr->numlocalseq );

	for (j = 0; j < pStudioHdr->numlocalseq; j++)
	{
		char *s1 = pStudioHdr->pLocalSeqdesc( j )->pszLabel();
		for (k = 0; k < numCheck; k++)
		{
			char *s2 = m_group[ m_seq[k].group ].GetStudioHdr()->pLocalSeqdesc( m_seq[k].index )->pszLabel();

			if (stricmp( s1, s2 ) == 0)
			{
				break;
			}
		}
		// no duplication
		if (k == numCheck)
		{
			virtualsequence_t tmp;
			tmp.group = group;
			tmp.index = j;
			tmp.flags = pStudioHdr->pLocalSeqdesc( j )->flags;
			tmp.activity = pStudioHdr->pLocalSeqdesc( j )->activity;
			k = m_seq.AddToTail( tmp );
		}
		else if (m_group[ m_seq[k].group ].GetStudioHdr()->pLocalSeqdesc( m_seq[k].index )->flags & STUDIO_OVERRIDE)
		{
			// the one in memory is a forward declared sequence, override it
			virtualsequence_t tmp;
			tmp.group = group;
			tmp.index = j;
			tmp.flags = pStudioHdr->pLocalSeqdesc( j )->flags;
			tmp.activity = pStudioHdr->pLocalSeqdesc( j )->activity;
			m_seq[k] = tmp;
		}
		else 
		{
			k = k;
		}

		m_group[ group ].masterSeq[ j ] = k;
	}
}


void virtualmodel_t::UpdateAutoplaySequences( const studiohdr_t *pStudioHdr )
{
	int autoplayCount = pStudioHdr->CountAutoplaySequences();
	m_autoplaySequences.SetCount( autoplayCount );
	pStudioHdr->CopyAutoplaySequences( m_autoplaySequences.Base(), autoplayCount );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

void virtualmodel_t::AppendAnimations( int group, const studiohdr_t *pStudioHdr )
{
	int numCheck = m_anim.Count();

	int j, k;

	m_group[ group ].masterAnim.SetCount( pStudioHdr->numlocalanim );

	for (j = 0; j < pStudioHdr->numlocalanim; j++)
	{
		char *s1 = pStudioHdr->pLocalAnimdesc( j )->pszName();
		for (k = 0; k < numCheck; k++)
		{
			char *s2 = m_group[ m_anim[k].group ].GetStudioHdr()->pLocalAnimdesc( m_anim[k].index )->pszName();

			if (stricmp( s1, s2 ) == 0)
			{
				break;
			}
		}
		// no duplication
		if (k == numCheck)
		{
			virtualgeneric_t tmp;
			tmp.group = group;
			tmp.index = j;
			k = m_anim.AddToTail( tmp );
		}

		m_group[ group ].masterAnim[ j ] = k;
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

void virtualmodel_t::AppendBonemap( int group, const studiohdr_t *pStudioHdr )
{
	const studiohdr_t *pBaseStudioHdr = m_group[ 0 ].GetStudioHdr( );

	m_group[ group ].boneMap.SetCount( pBaseStudioHdr->numbones );
	m_group[ group ].masterBone.SetCount( pStudioHdr->numbones );

	int j, k;

	if (group == 0)
	{
		for (j = 0; j < pStudioHdr->numbones; j++)
		{
			m_group[ group ].boneMap[ j ] = j;
			m_group[ group ].masterBone[ j ] = j;
		}
	}
	else
	{
		for (j = 0; j < pBaseStudioHdr->numbones; j++)
		{
			m_group[ group ].boneMap[ j ] = -1;
		}
		for (j = 0; j < pStudioHdr->numbones; j++)
		{
			for (k = 0; k < pBaseStudioHdr->numbones; k++)
			{
				if (stricmp( pStudioHdr->pBone( j )->pszName(), pBaseStudioHdr->pBone( k )->pszName() ) == 0)
				{
					break;
				}
			}
			if (k < pBaseStudioHdr->numbones)
			{
				m_group[ group ].masterBone[ j ] = k;
				m_group[ group ].boneMap[ k ] = j;

				// FIXME: these runtime messages don't display in hlmv
				if ((pStudioHdr->pBone( j )->parent == -1) || (pBaseStudioHdr->pBone( k )->parent == -1))
				{
					if ((pStudioHdr->pBone( j )->parent != -1) || (pBaseStudioHdr->pBone( k )->parent != -1))
					{
						Warning( "%s/%s : missmatched parent bones on \"%s\"\n", pBaseStudioHdr->name, pStudioHdr->name, pStudioHdr->pBone( j )->pszName() );
					}
				}
				else if (m_group[ group ].masterBone[ pStudioHdr->pBone( j )->parent ] != m_group[ 0 ].masterBone[ pBaseStudioHdr->pBone( k )->parent ])
				{
					Warning( "%s/%s : missmatched parent bones on \"%s\"\n", pBaseStudioHdr->name, pStudioHdr->name, pStudioHdr->pBone( j )->pszName() );
				}
			}
			else
			{
				m_group[ group ].masterBone[ j ] = -1;
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

void virtualmodel_t::AppendAttachments( int group, const studiohdr_t *pStudioHdr )
{
	int numCheck = m_attachment.Count();

	int j, k, n;

	m_group[ group ].masterAttachment.SetCount( pStudioHdr->numlocalattachments );

	for (j = 0; j < pStudioHdr->numlocalattachments; j++)
	{

		n = m_group[ group ].masterBone[ pStudioHdr->pLocalAttachment( j )->localbone ];
		
		// skip if the attachments bone doesn't exist in the root model
		if (n == -1)
		{
			continue;
		}
		
		
		char *s1 = pStudioHdr->pLocalAttachment( j )->pszName();
		for (k = 0; k < numCheck; k++)
		{
			char *s2 = m_group[ m_attachment[k].group ].GetStudioHdr()->pLocalAttachment( m_attachment[k].index )->pszName();

			if (stricmp( s1, s2 ) == 0)
			{
				break;
			}
		}
		// no duplication
		if (k == numCheck)
		{
			virtualgeneric_t tmp;
			tmp.group = group;
			tmp.index = j;
			k = m_attachment.AddToTail( tmp );

			// make sure bone flags are set so attachment calculates
			if ((m_group[ 0 ].GetStudioHdr()->pBone( n )->flags & BONE_USED_BY_ATTACHMENT) == 0)
			{
				while (n != -1)
				{
					m_group[ 0 ].GetStudioHdr()->pBone( n )->flags |= BONE_USED_BY_ATTACHMENT;
					n = m_group[ 0 ].GetStudioHdr()->pBone( n )->parent;
				}
				continue;
			}
		}

		m_group[ group ].masterAttachment[ j ] = k;
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

void virtualmodel_t::AppendPoseParameters( int group, const studiohdr_t *pStudioHdr )
{
	int numCheck = m_pose.Count();

	int j, k;

	m_group[ group ].masterPose.SetCount( pStudioHdr->numlocalposeparameters );

	for (j = 0; j < pStudioHdr->numlocalposeparameters; j++)
	{
		char *s1 = pStudioHdr->pLocalPoseParameter( j )->pszName();
		for (k = 0; k < numCheck; k++)
		{
			char *s2 = m_group[ m_pose[k].group ].GetStudioHdr()->pLocalPoseParameter( m_pose[k].index )->pszName();

			if (stricmp( s1, s2 ) == 0)
			{
				break;
			}
		}
		// no duplication
		if (k == numCheck)
		{
			virtualgeneric_t tmp;
			tmp.group = group;
			tmp.index = j;
			k = m_pose.AddToTail( tmp );
		}

		m_group[ group ].masterPose[ j ] = k;
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

void virtualmodel_t::AppendNodes( int group, const studiohdr_t *pStudioHdr )
{
	int numCheck = m_node.Count();

	int j, k;

	m_group[ group ].masterNode.SetCount( pStudioHdr->numlocalnodes );

	for (j = 0; j < pStudioHdr->numlocalnodes; j++)
	{
		char *s1 = pStudioHdr->pszLocalNodeName( j );
		for (k = 0; k < numCheck; k++)
		{
			char *s2 = m_group[ m_node[k].group ].GetStudioHdr()->pszLocalNodeName( m_node[k].index );

			if (stricmp( s1, s2 ) == 0)
			{
				break;
			}
		}
		// no duplication
		if (k == numCheck)
		{
			virtualgeneric_t tmp;
			tmp.group = group;
			tmp.index = j;
			k = m_node.AddToTail( tmp );
		}

		m_group[ group ].masterNode[ j ] = k;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------


void virtualmodel_t::AppendIKLocks( int group, const studiohdr_t *pStudioHdr )
{
	int numCheck = m_iklock.Count();

	int j, k;

	for (j = 0; j < pStudioHdr->numlocalikautoplaylocks; j++)
	{
		int chain1 = pStudioHdr->pLocalIKAutoplayLock( j )->chain;
		for (k = 0; k < numCheck; k++)
		{
			int chain2 = m_group[ m_iklock[k].group ].GetStudioHdr()->pLocalIKAutoplayLock( m_iklock[k].index )->chain;

			if (chain1 == chain2)
			{
				break;
			}
		}
		// no duplication
		if (k == numCheck)
		{
			virtualgeneric_t tmp;
			tmp.group = group;
			tmp.index = j;
			k = m_iklock.AddToTail( tmp );
		}
	}
}

	
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

void virtualmodel_t::AppendModels( int group, const studiohdr_t *pStudioHdr )
{
	AppendSequences( group, pStudioHdr );
	AppendAnimations( group, pStudioHdr );
	AppendBonemap( group, pStudioHdr );
	AppendAttachments( group, pStudioHdr );
	AppendPoseParameters( group, pStudioHdr );
	AppendNodes( group, pStudioHdr );
	AppendIKLocks( group, pStudioHdr );

	int j;
	for (j = 0; j < pStudioHdr->numincludemodels; j++)
	{
		void *tmp = NULL;
		const studiohdr_t *pTmpHdr = pStudioHdr->FindModel( &tmp, pStudioHdr->pModelGroup( j )->pszName() );

		if (pTmpHdr)
		{
			int group = m_group.AddToTail( );
			m_group[ group ].cache = tmp;
			AppendModels( group, pTmpHdr );
		}
	}
	UpdateAutoplaySequences( pStudioHdr );
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

mstudioanimdesc_t &studiohdr_t::pAnimdesc( int i ) const
{ 
	if (numincludemodels == 0)
	{
		return *pLocalAnimdesc( i );
	}

	virtualmodel_t *pVModel = (virtualmodel_t *)GetVirtualModel();
	Assert( pVModel );

	virtualgroup_t *pGroup = &pVModel->m_group[ pVModel->m_anim[i].group ];
	const studiohdr_t *pStudioHdr = pGroup->GetStudioHdr();
	Assert( pStudioHdr );

	return *pStudioHdr->pLocalAnimdesc( pVModel->m_anim[i].index );
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

mstudioanim_t *mstudioanimdesc_t::pAnim( void ) const
{
	if (animblock == 0)
	{
		return  (mstudioanim_t *)(((byte *)this) + animindex);
	}
	else
	{
		byte *pAnimBlocks = pStudiohdr()->GetAnimBlock( animblock );
		
		if ( !pAnimBlocks )
		{
			Error( "Error loading .ani file info block for '%s'\n",
				pStudiohdr()->name );
		}
		
		return (mstudioanim_t *)(pAnimBlocks + animindex);
	}

	return NULL;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

int studiohdr_t::GetNumSeq( void ) const
{
	if (numincludemodels == 0)
	{
		return numlocalseq;
	}

	virtualmodel_t *pVModel = (virtualmodel_t *)GetVirtualModel();
	Assert( pVModel );
	return pVModel->m_seq.Count();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

mstudioseqdesc_t &studiohdr_t::pSeqdesc( int i ) const
{
	if (numincludemodels == 0)
	{
		return *pLocalSeqdesc( i );
	}

	virtualmodel_t *pVModel = (virtualmodel_t *)GetVirtualModel();
	Assert( pVModel );

	virtualgroup_t *pGroup = &pVModel->m_group[ pVModel->m_seq[i].group ];
	const studiohdr_t *pStudioHdr = pGroup->GetStudioHdr();
	Assert( pStudioHdr );

	return *pStudioHdr->pLocalSeqdesc( pVModel->m_seq[i].index );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

int studiohdr_t::iRelativeAnim( int baseseq, int relanim ) const
{
	if (numincludemodels == 0)
	{
		return relanim;
	}

	virtualmodel_t *pVModel = (virtualmodel_t *)GetVirtualModel();
	Assert( pVModel );

	virtualgroup_t *pGroup = &pVModel->m_group[ pVModel->m_seq[baseseq].group ];

	return pGroup->masterAnim[ relanim ];
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

int studiohdr_t::iRelativeSeq( int baseseq, int relseq ) const
{
	if (numincludemodels == 0)
	{
		return relseq;
	}

	virtualmodel_t *pVModel = (virtualmodel_t *)GetVirtualModel();
	Assert( pVModel );

	virtualgroup_t *pGroup = &pVModel->m_group[ pVModel->m_seq[baseseq].group ];

	return pGroup->masterSeq[ relseq ];
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

int	studiohdr_t::GetNumPoseParameters( void ) const
{
	if (numincludemodels == 0)
	{
		return numlocalposeparameters;
	}

	virtualmodel_t *pVModel = (virtualmodel_t *)GetVirtualModel();
	Assert( pVModel );

	return pVModel->m_pose.Count();
}



//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

const mstudioposeparamdesc_t &studiohdr_t::pPoseParameter( int i ) const
{
	if (numincludemodels == 0)
	{
		return *pLocalPoseParameter( i );
	}

	virtualmodel_t *pVModel = (virtualmodel_t *)GetVirtualModel();
	Assert( pVModel );

	virtualgroup_t *pGroup = &pVModel->m_group[ pVModel->m_pose[i].group ];
	const studiohdr_t *pStudioHdr = pGroup->GetStudioHdr();
	Assert( pStudioHdr );

	return *pStudioHdr->pLocalPoseParameter( pVModel->m_pose[i].index );
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

int studiohdr_t::GetSharedPoseParameter( int iSequence, int iLocalPose ) const
{
	mstudioseqdesc_t &seqdesc = pSeqdesc( iSequence );

	if (numincludemodels == 0)
	{
		return seqdesc.paramindex[iLocalPose];
	}

	int iLocalIndex = seqdesc.paramindex[iLocalPose];
	if (iLocalIndex == -1)
		return iLocalIndex;

	virtualmodel_t *pVModel = (virtualmodel_t *)GetVirtualModel();
	Assert( pVModel );

	virtualgroup_t *pGroup = &pVModel->m_group[ pVModel->m_seq[iSequence].group ];

	return pGroup->masterPose[iLocalIndex];
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

int studiohdr_t::EntryNode( int iSequence ) const
{
	mstudioseqdesc_t &seqdesc = pSeqdesc( iSequence );

	if (numincludemodels == 0 || seqdesc.localentrynode == 0)
	{
		return seqdesc.localentrynode;
	}

	virtualmodel_t *pVModel = (virtualmodel_t *)GetVirtualModel();
	Assert( pVModel );

	virtualgroup_t *pGroup = &pVModel->m_group[ pVModel->m_seq[iSequence].group ];

	return pGroup->masterNode[seqdesc.localentrynode-1]+1;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------


int studiohdr_t::ExitNode( int iSequence ) const
{
	mstudioseqdesc_t &seqdesc = pSeqdesc( iSequence );

	if (numincludemodels == 0 || seqdesc.localexitnode == 0)
	{
		return seqdesc.localexitnode;
	}

	virtualmodel_t *pVModel = (virtualmodel_t *)GetVirtualModel();
	Assert( pVModel );

	virtualgroup_t *pGroup = &pVModel->m_group[ pVModel->m_seq[iSequence].group ];

	return pGroup->masterNode[seqdesc.localexitnode-1]+1;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

int	studiohdr_t::GetNumAttachments( void ) const
{
	if (numincludemodels == 0)
	{
		return numlocalattachments;
	}

	virtualmodel_t *pVModel = (virtualmodel_t *)GetVirtualModel();
	Assert( pVModel );

	return pVModel->m_attachment.Count();
}



//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

const mstudioattachment_t &studiohdr_t::pAttachment( int i ) const
{
	if (numincludemodels == 0)
	{
		return *pLocalAttachment( i );
	}

	virtualmodel_t *pVModel = (virtualmodel_t *)GetVirtualModel();
	Assert( pVModel );

	virtualgroup_t *pGroup = &pVModel->m_group[ pVModel->m_attachment[i].group ];
	const studiohdr_t *pStudioHdr = pGroup->GetStudioHdr();
	Assert( pStudioHdr );

	return *pStudioHdr->pLocalAttachment( pVModel->m_attachment[i].index );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

int	studiohdr_t::GetAttachmentBone( int i ) const
{
	const mstudioattachment_t &attachment = pAttachment( i );

	// remap bone
	virtualmodel_t *pVModel = GetVirtualModel();
	if (pVModel)
	{
		virtualgroup_t *pGroup = &pVModel->m_group[ pVModel->m_attachment[i].group ];
		int iBone = pGroup->masterBone[attachment.localbone];
		if (iBone == -1)
			return 0;
		return iBone;
	}
	return attachment.localbone;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

void studiohdr_t::SetAttachmentBone( int iAttachment, int iBone )
{
	mstudioattachment_t &attachment = (mstudioattachment_t &)pAttachment( iAttachment );

	// remap bone
	virtualmodel_t *pVModel = GetVirtualModel();
	if (pVModel)
	{
		virtualgroup_t *pGroup = &pVModel->m_group[ pVModel->m_attachment[iAttachment].group ];
		iBone = pGroup->boneMap[iBone];
	}
	attachment.localbone = iBone;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

char *studiohdr_t::pszNodeName( int iNode ) const
{
	if (numincludemodels == 0)
	{
		return pszLocalNodeName( iNode );
	}

	virtualmodel_t *pVModel = (virtualmodel_t *)GetVirtualModel();
	Assert( pVModel );

	if ( pVModel->m_node.Count() <= iNode-1 )
		return "Invalid node";

	return pVModel->m_group[ pVModel->m_node[iNode-1].group ].GetStudioHdr()->pszLocalNodeName( pVModel->m_node[iNode-1].index );
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

int studiohdr_t::GetTransition( int iFrom, int iTo ) const
{
	if (numincludemodels == 0)
	{
		return *pLocalTransition( (iFrom-1)*numlocalnodes + (iTo - 1) );
	}

	return iTo;
	/*
	FIXME: not connected
	virtualmodel_t *pVModel = (virtualmodel_t *)GetVirtualModel();
	Assert( pVModel );

	return pVModel->m_transition.Element( iFrom ).Element( iTo );
	*/
}


int	studiohdr_t::GetActivityListVersion( void ) const
{
	if (numincludemodels == 0)
	{
		return activitylistversion;
	}

	virtualmodel_t *pVModel = (virtualmodel_t *)GetVirtualModel();
	Assert( pVModel );

	int version = activitylistversion;

	int i;
	for (i = 1; i < pVModel->m_group.Count(); i++)
	{
		virtualgroup_t *pGroup = &pVModel->m_group[ i ];
		const studiohdr_t *pStudioHdr = pGroup->GetStudioHdr();

		Assert( pStudioHdr );

		version = min( version, pStudioHdr->activitylistversion );
	}

	return version;
}

void studiohdr_t::SetActivityListVersion( int version ) const
{
	activitylistversion = version;

	if (numincludemodels == 0)
	{
		return;
	}

	virtualmodel_t *pVModel = (virtualmodel_t *)GetVirtualModel();
	Assert( pVModel );

	int i;
	for (i = 1; i < pVModel->m_group.Count(); i++)
	{
		virtualgroup_t *pGroup = &pVModel->m_group[ i ];
		const studiohdr_t *pStudioHdr = pGroup->GetStudioHdr();

		Assert( pStudioHdr );

		pStudioHdr->SetActivityListVersion( version );
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------


int studiohdr_t::GetNumIKAutoplayLocks( void ) const
{
	if (numincludemodels == 0)
	{
		return numlocalikautoplaylocks;
	}

	virtualmodel_t *pVModel = (virtualmodel_t *)GetVirtualModel();
	Assert( pVModel );

	return pVModel->m_iklock.Count();
}

const mstudioiklock_t &studiohdr_t::pIKAutoplayLock( int i ) const
{
	if (numincludemodels == 0)
	{
		return *pLocalIKAutoplayLock( i );
	}

	virtualmodel_t *pVModel = (virtualmodel_t *)GetVirtualModel();
	Assert( pVModel );

	virtualgroup_t *pGroup = &pVModel->m_group[ pVModel->m_iklock[i].group ];
	const studiohdr_t *pStudioHdr = pGroup->GetStudioHdr();
	Assert( pStudioHdr );

	return *pStudioHdr->pLocalIKAutoplayLock( pVModel->m_iklock[i].index );
}

int	studiohdr_t::CountAutoplaySequences() const
{
	int count = 0;
	for (int i = 0; i < GetNumSeq(); i++)
	{
		mstudioseqdesc_t &seqdesc = pSeqdesc( i );
		if (seqdesc.flags & STUDIO_AUTOPLAY)
		{
			count++;
		}
	}
	return count;
}

int	studiohdr_t::CopyAutoplaySequences( unsigned short *pOut, int outCount ) const
{
	int outIndex = 0;
	for (int i = 0; i < GetNumSeq() && outIndex < outCount; i++)
	{
		mstudioseqdesc_t &seqdesc = pSeqdesc( i );
		if (seqdesc.flags & STUDIO_AUTOPLAY)
		{
			pOut[outIndex] = i;
			outIndex++;
		}
	}
	return outIndex;
}

//-----------------------------------------------------------------------------
// Purpose:	maps local sequence bone to global bone
//-----------------------------------------------------------------------------

int	studiohdr_t::RemapSeqBone( int iSequence, int iLocalBone ) const	
{
	// remap bone
	virtualmodel_t *pVModel = GetVirtualModel();
	if (pVModel)
	{
		const virtualgroup_t *pSeqGroup = pVModel->pSeqGroup( iSequence );
		return pSeqGroup->masterBone[iLocalBone];
	}
	return iLocalBone;
}

int	studiohdr_t::RemapAnimBone( int iAnim, int iLocalBone ) const
{
	// remap bone
	virtualmodel_t *pVModel = GetVirtualModel();
	if (pVModel)
	{
		const virtualgroup_t *pAnimGroup = pVModel->pAnimGroup( iAnim );
		return pAnimGroup->masterBone[iLocalBone];
	}
	return iLocalBone;
}

