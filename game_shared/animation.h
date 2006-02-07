//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef ANIMATION_H
#define ANIMATION_H

#define ACTIVITY_NOT_AVAILABLE		-1

struct animevent_t;
struct studiohdr_t;
struct mstudioseqdesc_t;

int ExtractBbox( studiohdr_t *pstudiohdr, int sequence, Vector& mins, Vector& maxs );

void IndexModelSequences( studiohdr_t *pstudiohdr );
void ResetActivityIndexes( studiohdr_t *pstudiohdr );
void VerifySequenceIndex( studiohdr_t *pstudiohdr );
int SelectWeightedSequence( studiohdr_t *pstudiohdr, int activity, int curSequence = -1 );
int SelectHeaviestSequence( studiohdr_t *pstudiohdr, int activity );
void SetEventIndexForSequence( mstudioseqdesc_t &seqdesc );
void BuildAllAnimationEventIndexes( studiohdr_t *pstudiohdr );
void ResetEventIndexes( studiohdr_t *pstudiohdr );

void GetEyePosition( studiohdr_t *pstudiohdr, Vector &vecEyePosition );

int LookupActivity( studiohdr_t *pstudiohdr, const char *label );
int LookupSequence( studiohdr_t *pstudiohdr, const char *label );

#define NOMOTION 99999
void GetSequenceLinearMotion( studiohdr_t *pstudiohdr, int iSequence, const float poseParameter[], Vector *pVec );

const char *GetSequenceName( studiohdr_t *pstudiohdr, int sequence );
const char *GetSequenceActivityName( studiohdr_t *pstudiohdr, int iSequence );

int GetSequenceFlags( studiohdr_t *pstudiohdr, int sequence );
int GetAnimationEvent( studiohdr_t *pstudiohdr, int sequence, animevent_t *pNPCEvent, float flStart, float flEnd, int index );
bool HasAnimationEventOfType( studiohdr_t *pstudiohdr, int sequence, int type );

int FindTransitionSequence( studiohdr_t *pstudiohdr, int iCurrentSequence, int iGoalSequence, int *piDir );

void SetBodygroup( studiohdr_t *pstudiohdr, int& body, int iGroup, int iValue );
int GetBodygroup( studiohdr_t *pstudiohdr, int body, int iGroup );

const char *GetBodygroupName( studiohdr_t *pstudiohdr, int iGroup );
int FindBodygroupByName( studiohdr_t *pstudiohdr, const char *name );
int GetBodygroupCount( studiohdr_t *pstudiohdr, int iGroup );
int GetNumBodyGroups( studiohdr_t *pstudiohdr );

int GetSequenceActivity( studiohdr_t *pstudiohdr, int sequence, int *pweight = NULL );

void GetAttachmentLocalSpace( studiohdr_t *pstudiohdr, int attachIndex, matrix3x4_t &pLocalToWorld );

float SetBlending( studiohdr_t *pstudiohdr, int sequence, int *pblendings, int iBlender, float flValue );

int FindHitboxSetByName( studiohdr_t *pstudiohdr, const char *name );
const char *GetHitboxSetName( studiohdr_t *pstudiohdr, int setnumber );
int GetHitboxSetCount( studiohdr_t *pstudiohdr );

#endif	//ANIMATION_H
