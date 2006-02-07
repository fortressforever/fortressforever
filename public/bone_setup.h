//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef BONE_SETUP_H
#define BONE_SETUP_H
#ifdef _WIN32
#pragma once
#endif


#include "studio.h"
#include "cmodel.h"
#include "bitvec.h"


class CBoneToWorld;
class CIKContext;
class CBoneAccessor;


// This provides access to networked arrays, so if this code actually changes a value, 
// the entity is marked as changed.
class IParameterAccess
{
public:
	virtual float GetParameter( int iParam ) = 0;
	virtual void SetParameter( int iParam, float flValue ) = 0;
};



class CBoneBitList : public CBitVec<MAXSTUDIOBONES>
{
public:
	inline void MarkBone(int iBone)
	{
		Set(iBone);
	}
	inline bool IsBoneMarked(int iBone)
	{
		return Get(iBone) != 0 ? true : false;
	}
};




//-----------------------------------------------------------------------------
// Purpose: blends together all the bones from two p:q lists
//
// p1 = p1 * (1 - s) + p2 * s
// q1 = q1 * (1 - s) + q2 * s
//-----------------------------------------------------------------------------
void SlerpBones( 
	const studiohdr_t *pStudioHdr,
	Quaternion q1[MAXSTUDIOBONES], 
	Vector pos1[MAXSTUDIOBONES], 
	int sequence, 
	const Quaternion q2[MAXSTUDIOBONES], 
	const Vector pos2[MAXSTUDIOBONES], 
	float s,
	int boneMask
	);


void InitPose(
	const studiohdr_t *pStudioHdr,
	Vector pos[], 
	Quaternion q[]
	);

void CalcPose(
	const studiohdr_t *pStudioHdr,
	CIKContext *pIKContext,			//optional
	Vector pos[], 
	Quaternion q[], 
	int sequence, 
	float cycle,
	const float poseParameter[],
	int boneMask,
	float flWeight = 1.0f,
	float flTime = 0.0f
	);

bool CalcPoseSingle(
	const studiohdr_t *pStudioHdr,
	Vector pos[], 
	Quaternion q[], 
	int sequence, 
	float cycle,
	const float poseParameter[],
	int boneMask,
	float flTime
	);

void AccumulatePose(
	const studiohdr_t *pStudioHdr,
	CIKContext *pIKContext,			//optional
	Vector pos[], 
	Quaternion q[], 
	int sequence, 
	float cycle,
	const float poseParameter[],
	int boneMask,
	float flWeight,
	float flTime
	);

// takes a "controllers[]" array normalized to 0..1 and adds in the adjustments to pos[], and q[].
void CalcBoneAdj(
	const studiohdr_t *pStudioHdr,
	Vector pos[], 
	Quaternion q[], 
	const float controllers[],
	int boneMask
	);


// Given two samples of a bone separated in time by dt, 
// compute the velocity and angular velocity of that bone
void CalcBoneDerivatives( Vector &velocity, AngularImpulse &angVel, const matrix3x4_t &prev, const matrix3x4_t &current, float dt );
// Give a derivative of a bone, compute the velocity & angular velocity of that bone
void CalcBoneVelocityFromDerivative( const QAngle &vecAngles, Vector &velocity, AngularImpulse &angVel, const matrix3x4_t &current );

// This function sets up the local transform for a single frame of animation. It doesn't handle
// pose parameters or interpolation between frames.
void SetupSingleBoneMatrix( 
	studiohdr_t *pOwnerHdr, 
	int nSequence, 
	int iFrame,
	int iBone, 
	matrix3x4_t &mBoneLocal );


// Purpose: build boneToWorld transforms for a specific bone
void BuildBoneChain(
	const studiohdr_t *pStudioHdr,
	matrix3x4_t &rootxform,
	Vector pos[], 
	Quaternion q[], 
	int	iBone,
	matrix3x4_t *pBoneToWorld );


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

// ik info
class CIKTarget
{
public:
	void SetPos( const Vector &pos );
	void SetAngles( const QAngle &angles );
	void SetQuaternion( const Quaternion &q );
	void SetNormal( const Vector &normal );
	void SetPosWithNormalOffset( const Vector &pos, const Vector &normal );

	bool IsActive( void );
	void IKFailed( void );
	int chain;
	int type;
	// accumulated offset from ideal footplant location
public:
	struct x2 {
		char		*pAttachmentName;
		Vector		pos;
		Quaternion	q;
	} offset;
private:
	struct x3 {
		Vector		pos;
		Quaternion	q;
	} ideal;
public:
	struct x4 {
		float		latched;
		float		release;
		float		height;
		float		floor;
		float		radius;
		float		flTime;
		float		flWeight;
		Vector		pos;
		Quaternion	q;
	} est; // estimate contact position
	struct x5 {
		Vector		p1;
		Vector		p2;
		Vector		p3;
	} trace;
private:
	// internally latched footset, position
	struct x1 {
		// matrix3x4_t		worldTarget;
		bool		bNeedsLatch;
		bool		bHasLatch;
		float		influence;
		int			iFramecounter;
		Vector		pos;
		Quaternion	q;
		Vector		deltaPos;	// acculated error
		Quaternion	deltaQ;
	} latched;

	friend class CIKContext;
};


struct ikchainresult_t
{
	// accumulated offset from ideal footplant location
	int			target;
	Vector		pos;
	Quaternion	q;
	float		flWeight;
};



struct ikcontextikrule_t
{
	int			index;

	int			type;
	int			chain;

	int			bone;

	int			slot;	// iktarget slot.  Usually same as chain.
	float		height;
	float		radius;
	float		floor;
	Vector		pos;
	Quaternion	q;

	float		start;	// beginning of influence
	float		peak;	// start of full influence
	float		tail;	// end of full influence
	float		end;	// end of all influence

	float		commit;		// frame footstep target should be committed
	float		release;	// frame ankle should end rotation from latched orientation

	float		flWeight;		// processed version of start-end cycle
	float		flRuleWeight;	// blending weight
	float		latched;		// does the IK rule use a latched value?
	char		*szLabel;

	Vector		kneeDir;
	Vector		kneePos;

	ikcontextikrule_t() {}

private:
	// No copy constructors allowed
	ikcontextikrule_t(const ikcontextikrule_t& vOther);
};


void Studio_AlignIKMatrix( matrix3x4_t &mMat, const Vector &vAlignTo );

bool Studio_SolveIK( int iThigh, int iKnee, int iFoot, Vector &targetFoot, matrix3x4_t* pBoneToWorld );

bool Studio_SolveIK( int iThigh, int iKnee, int iFoot, Vector &targetFoot, Vector &targetKneePos, Vector &targetKneeDir, matrix3x4_t* pBoneToWorld );

bool Studio_SolveIK( mstudioikchain_t *pikchain, Vector &targetFoot, matrix3x4_t* pBoneToWorld );

class CIKContext 
{
public:
	CIKContext( );
	void Init( const studiohdr_t *pStudioHdr, const QAngle &angles, const Vector &pos, float flTime, int iFramecounter, int boneMask );
	void AddDependencies(  int iSequence, float flCycle, const float poseParameters[], float flWeight = 1.0f );

	void ClearTargets( void );
	void UpdateTargets( Vector pos[], Quaternion q[], matrix3x4_t boneToWorld[], CBoneBitList &boneComputed );
	void SolveDependencies( Vector pos[], Quaternion q[], matrix3x4_t boneToWorld[], CBoneBitList &boneComputed );

	void AddAutoplayLocks( Vector pos[], Quaternion q[] );
	void SolveAutoplayLocks( Vector pos[], Quaternion q[] );

	void AddSequenceLocks( mstudioseqdesc_t &SeqDesc, Vector pos[], Quaternion q[] );
	void SolveSequenceLocks( mstudioseqdesc_t &SeqDesc, Vector pos[], 	Quaternion q[] );
	
	CUtlVector< CIKTarget >	m_target;

private:

	studiohdr_t const *m_pStudioHdr;

	bool Estimate( int iSequence, float flCycle, int iTarget, const float poseParameter[], float flWeight = 1.0f ); 
	void BuildBoneChain( Vector pos[], Quaternion q[], int iBone, matrix3x4_t *pBoneToWorld, CBoneBitList &boneComputed );

	// virtual IK rules, filtered and combined from each sequence
	CUtlVector< CUtlVector< ikcontextikrule_t > > m_ikChainRule;
	CUtlVector< ikcontextikrule_t > m_ikLock;
	matrix3x4_t m_rootxform;

	int m_iFramecounter;
	float m_flTime;
	int m_boneMask;
};


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

// takes a "poseparameters[]" array normalized to 0..1 and layers on the sequences driven by them
void CalcAutoplaySequences(
	const studiohdr_t *pStudioHdr,
	CIKContext *pIKContext,		//optional
	Vector pos[], 
	Quaternion q[], 
	const float poseParameters[],
	int boneMask,
	float time
	);

// replaces the bonetoworld transforms for all bones that are procedural
bool CalcProceduralBone(
	const studiohdr_t *pStudioHdr,
	int iBone,
	CBoneAccessor &bonetoworld
	);

void Studio_BuildMatrices(
	const studiohdr_t *pStudioHdr,
	const QAngle& angles, 
	const Vector& origin, 
	const Vector pos[],
	const Quaternion q[],
	int iBone,
	matrix3x4_t bonetoworld[MAXSTUDIOBONES],
	int boneMask
	);


// Get a bone->bone relative transform
void Studio_CalcBoneToBoneTransform( const studiohdr_t *pStudioHdr, int inputBoneIndex, int outputBoneIndex, matrix3x4_t &matrixOut );

// Given a bone rotation value, figures out the value you need to give to the controller
// to have the bone at that value.
// [in]  flValue  = the desired bone rotation value
// [out] ctlValue = the (0-1) value to set the controller t.
// return value   = flValue, unwrapped to lie between the controller's start and end.
float Studio_SetController( const studiohdr_t *pStudioHdr, int iController, float flValue, float &ctlValue );


// Given a 0-1 controller value, maps it into the controller's start and end and returns the bone rotation angle.
// [in] ctlValue  = value in controller space (0-1).
// return value   = value in bone space
float Studio_GetController( const studiohdr_t *pStudioHdr, int iController, float ctlValue );

float Studio_GetPoseParameter( const studiohdr_t *pStudioHdr, int iParameter, float ctlValue );
float Studio_SetPoseParameter( const studiohdr_t *pStudioHdr, int iParameter, float flValue, float &ctlValue );

// converts a global 0..1 pose parameter into the local sequences blending value
void Studio_LocalPoseParameter( const studiohdr_t *pStudioHdr, const float poseParameter[], int iSequence, int iLocalPose, float &flSetting, int &index );

void Studio_SeqAnims( const studiohdr_t *pStudioHdr, int iSequence, const float poseParameter[], mstudioanimdesc_t *panim[4], float *weight );
int Studio_MaxFrame( const studiohdr_t *pStudioHdr, int iSequence, const float poseParameter[] );
float Studio_FPS( const studiohdr_t *pStudioHdr, int iSequence, const float poseParameter[] );
float Studio_CPS( const studiohdr_t *pStudioHdr, int iSequence, const float poseParameter[] );
float Studio_Duration( const studiohdr_t *pStudioHdr, int iSequence, const float poseParameter[] );
void Studio_MovementRate( const studiohdr_t *pStudioHdr, int iSequence, const float poseParameter[], Vector *pVec );

// void Studio_Movement( const studiohdr_t *pStudioHdr, int iSequence, const float poseParameter[], Vector *pVec );

//void Studio_AnimPosition( mstudioanimdesc_t *panim, float flCycle, Vector &vecPos, Vector &vecAngle );
//void Studio_AnimVelocity( mstudioanimdesc_t *panim, float flCycle, Vector &vecVelocity );
//float Studio_FindAnimDistance( mstudioanimdesc_t *panim, float flDist );
bool Studio_AnimMovement( mstudioanimdesc_t *panim, float flCycleFrom, float flCycleTo, Vector &deltaPos, QAngle &deltaAngle );
bool Studio_SeqMovement( const studiohdr_t *pStudioHdr, int iSequence, float flCycleFrom, float flCycleTo, const float poseParameter[], Vector &deltaMovement, QAngle &deltaAngle );
bool Studio_SeqVelocity( const studiohdr_t *pStudioHdr, int iSequence, float flCycle, const float poseParameter[], Vector &vecVelocity );
float Studio_FindSeqDistance( const studiohdr_t *pStudioHdr, int iSequence, const float poseParameter[], float flDist );
float Studio_FindSeqVelocity( const studiohdr_t *pStudioHdr, int iSequence, const float poseParameter[], float flVelocity );
int Studio_FindAttachment( const studiohdr_t *pStudioHdr, const char *pAttachmentName );
int Studio_FindRandomAttachment( const studiohdr_t *pStudioHdr, const char *pAttachmentName );
int Studio_BoneIndexByName( const studiohdr_t *pStudioHdr, const char *pName );
const char *Studio_GetDefaultSurfaceProps( studiohdr_t *pstudiohdr );
float Studio_GetMass( studiohdr_t *pstudiohdr );
const char *Studio_GetKeyValueText( const studiohdr_t *pStudioHdr, int iSequence );

FORWARD_DECLARE_HANDLE( memhandle_t );
struct bonecacheparams_t
{
	studiohdr_t		*pStudioHdr;
	matrix3x4_t		*pBoneToWorld;
	float			curtime;
	int				boneMask;
};

class CBoneCache
{
public:

	// you must implement these static functions for the ResourceManager
	// -----------------------------------------------------------
	static CBoneCache *CreateResource( const bonecacheparams_t &params );
	static unsigned int EstimatedSize( const bonecacheparams_t &params );
	// -----------------------------------------------------------
	// member functions that must be present for the ResourceManager
	void			DestroyResource();
	CBoneCache		*GetData() { return this; }
	unsigned int	Size() { return m_size; }
	// -----------------------------------------------------------

					CBoneCache();

	// was constructor, but placement new is messy wrt memdebug - so cast & init instead
	void			Init( const bonecacheparams_t &params, unsigned int size, short *pStudioToCached, short *pCachedToStudio, int cachedBoneCount );
	
	void			UpdateBones( const matrix3x4_t *pBoneToWorld, int numbones, float curtime );
	matrix3x4_t		*GetCachedBone( int studioIndex );
	void			ReadCachedBones( matrix3x4_t *pBoneToWorld );
	void			ReadCachedBonePointers( matrix3x4_t **bones, int numbones );

	bool			IsValid( float time, float dt = 0.1f );

public:
	float			m_timeValid;
	int				m_boneMask;

private:
	matrix3x4_t		*BoneArray();
	short			*StudioToCached();
	short			*CachedToStudio();

	unsigned int	m_size;
	unsigned short	m_cachedBoneCount;
	unsigned short	m_matrixOffset;
	unsigned short	m_cachedToStudioOffset;
	unsigned short	m_boneOutOffset;
};

CBoneCache *Studio_GetBoneCache( memhandle_t cacheHandle );
memhandle_t Studio_CreateBoneCache( bonecacheparams_t &params );
void Studio_DestroyBoneCache( memhandle_t cacheHandle );
void Studio_InvalidateBoneCache( memhandle_t cacheHandle );

// Given a ray, trace for an intersection with this studiomodel.  Get the array of bones from StudioSetupHitboxBones
bool TraceToStudio( const Ray_t& ray, studiohdr_t *pStudioHdr, mstudiohitboxset_t *set, matrix3x4_t **hitboxbones, int fContentsMask, trace_t &trace );


void QuaternionSM( float s, const Quaternion &p, const Quaternion &q, Quaternion &qt );
void QuaternionMA( const Quaternion &p, float s, const Quaternion &q, Quaternion &qt );

#endif // BONE_SETUP_H
