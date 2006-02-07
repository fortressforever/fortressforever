//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef BASEANIMATING_H
#define BASEANIMATING_H
#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"
#include "entityoutput.h"

struct animevent_t;
struct matrix3x4_t;
class CIKContext;
class KeyValues;
FORWARD_DECLARE_HANDLE( memhandle_t );

class CBaseAnimating : public CBaseEntity
{
public:
	DECLARE_CLASS( CBaseAnimating, CBaseEntity );

	CBaseAnimating();
	~CBaseAnimating();

	DECLARE_PREDICTABLE();

	enum
	{
		NUM_POSEPAREMETERS = 24,
		NUM_BONECTRLS = 4
	};

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	virtual void SetModel( const char *szModelName );
	virtual void Activate();
	virtual void SetTransmit( CCheckTransmitInfo *pInfo, bool bAlways );

	virtual void OnRestore();

	studiohdr_t *GetModelPtr( void );

	virtual CBaseAnimating*	GetBaseAnimating() { return this; }

	// Cycle access
	void SetCycle( float flCycle );
	float GetCycle() const;

	float	GetAnimTimeInterval( void ) const;

	// Call this in your constructor to tell it that you will not use animtime. Then the
	// interpolation will be done correctly on the client.
	// This defaults to off.
	void	UseClientSideAnimation();

	// Tells whether or not we're using client-side animation. Used for controlling
	// the transmission of animtime.
	bool	IsUsingClientSideAnimation()	{ return m_bClientSideAnimation; }


	// Basic NPC Animation functions
	virtual float	GetIdealSpeed( ) const;
	virtual float	GetIdealAccel( ) const;
	virtual void	StudioFrameAdvance(); // advance animation frame to some time in the future
	void StudioFrameAdvanceManual( float flInterval );
	bool	IsValidSequence( int iSequence );

	inline float					GetPlaybackRate();
	inline void						SetPlaybackRate( float rate );

	inline int GetSequence() { return m_nSequence; }
	inline void SetSequence(int nSequence) { m_nSequence = nSequence; }
	/* inline */ void ResetSequence(int nSequence);
	int		GetSequenceFlags( int iSequence );
	// FIXME: push transitions support down into CBaseAnimating?
	virtual bool IsActivityFinished( void ) { return m_bSequenceFinished; }
	inline bool IsSequenceFinished( void ) { return m_bSequenceFinished; }
	inline bool SequenceLoops( void ) { return m_bSequenceLoops; }
	inline float SequenceDuration( void ) { return SequenceDuration( m_nSequence ); }
	float	SequenceDuration( int iSequence );
	float	GetSequenceCycleRate( int iSequence );
	float	GetLastVisibleCycle( int iSequence );
	float	GetSequenceGroundSpeed( int iSequence );
	void	ResetActivityIndexes ( void );
	void    ResetEventIndexes ( void );
	int		SelectWeightedSequence ( Activity activity );
	int		SelectWeightedSequence ( Activity activity, int curSequence );
	int		SelectHeaviestSequence ( Activity activity );
	int		LookupActivity( const char *label );
	int		LookupSequence ( const char *label );
	KeyValues *GetSequenceKeyValues( int iSequence );

	float GetSequenceMoveYaw( int iSequence );
	float GetSequenceMoveDist( int iSequence );
	void  GetSequenceLinearMotion( int iSequence, Vector *pVec );
	const char *GetSequenceName( int iSequence );
	const char *GetSequenceActivityName( int iSequence );
	Activity GetSequenceActivity( int iSequence );

	void ResetSequenceInfo ( );
	// This will stop animation until you call ResetSequenceInfo() at some point in the future
	inline void StopAnimation( void ) { m_flPlaybackRate = 0; }

	virtual void ClampRagdollForce( const Vector &vecForceIn, Vector *vecForceOut ) { *vecForceOut = vecForceIn; } // Base class does nothing.
	virtual bool BecomeRagdollOnClient( const Vector &force );
	virtual bool IsRagdoll();
	virtual bool CanBecomeRagdoll( void ); //Check if this entity will ragdoll when dead.

	virtual	void GetSkeleton( Vector pos[], Quaternion q[], int boneMask );

	virtual void GetBoneTransform( int iBone, matrix3x4_t &pBoneToWorld );
	virtual void SetupBones( matrix3x4_t *pBoneToWorld, int boneMask );
	virtual void CalculateIKLocks( float currentTime );
	virtual void Teleport( const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity );

	bool HasAnimEvent( int nSequence, int nEvent );
	virtual	void DispatchAnimEvents ( CBaseAnimating *eventHandler ); // Handle events that have happend since last time called up until X seconds into the future
	virtual void HandleAnimEvent( animevent_t *pEvent ) { return; };

	int		LookupPoseParameter( const char *szName );
	float	SetPoseParameter( const char *szName, float flValue );
	float	SetPoseParameter( int iParameter, float flValue );
	float	GetPoseParameter( const char *szName );
	float	GetPoseParameter( int iParameter );
	bool	HasPoseParameter( int iSequence, const char *szName );
	bool	HasPoseParameter( int iSequence, int iParameter );
	float	EdgeLimitPoseParameter( int iParameter, float flValue, float flBase = 0.0f );

	int  LookupBone( const char *szName );
	void GetBonePosition( const char *szName, Vector &origin, QAngle &angles );
	void GetBonePosition( int iBone, Vector &origin, QAngle &angles );
	int	GetPhysicsBone( int boneIndex );

	int GetNumBones ( void );

	int  FindTransitionSequence( int iCurrentSequence, int iGoalSequence, int *piDir );
	int  GetEntryNode( int iSequence );
	int  GetExitNode( int iSequence );
	float  GetExitPhase( int iSequence );
	
	void GetEyeballs( Vector &origin, QAngle &angles ); // ?? remove ??

	int LookupAttachment( const char *szName );

	// These return the attachment in world space
	bool GetAttachment( const char *szName, Vector &absOrigin, QAngle &absAngles );
	bool GetAttachment( int iAttachment, Vector &absOrigin, QAngle &absAngles );
	virtual bool GetAttachment( int iAttachment, matrix3x4_t &attachmentToWorld );

	// These return the attachment in the space of the entity
	bool GetAttachmentLocal( const char *szName, Vector &origin, QAngle &angles );
	bool GetAttachmentLocal( int iAttachment, Vector &origin, QAngle &angles );
	bool GetAttachmentLocal( int iAttachment, matrix3x4_t &attachmentToLocal );
	
	// Non-angle versions of the attachments in world space
	bool GetAttachment(  const char *szName, Vector &absOrigin, Vector *forward = NULL, Vector *right = NULL, Vector *up = NULL );
	bool GetAttachment( int iAttachment, Vector &absOrigin, Vector *forward = NULL, Vector *right = NULL, Vector *up = NULL );

	void SetBodygroup( int iGroup, int iValue );
	int GetBodygroup( int iGroup );

	const char *GetBodygroupName( int iGroup );
	int FindBodygroupByName( const char *name );
	int GetBodygroupCount( int iGroup );
	int GetNumBodyGroups( void );

	void					SetHitboxSet( int setnum );
	void					SetHitboxSetByName( const char *setname );
	int						GetHitboxSet( void );
	char const				*GetHitboxSetName( void );
	int						GetHitboxSetCount( void );
	int						GetHitboxBone( int hitboxIndex );
	bool					LookupHitbox( const char *szName, int& outSet, int& outBox );

	// Computes a box that surrounds all hitboxes
	bool ComputeHitboxSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs );
	bool ComputeEntitySpaceHitboxSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs );
	
	// Clone a CBaseAnimating from another (copies model & sequence data)
	void CopyAnimationDataFrom( CBaseAnimating *pSource );

	int ExtractBbox( int sequence, Vector& mins, Vector& maxs );
	void SetSequenceBox( void );
	int RegisterPrivateActivity( const char *pszActivityName );

	void	ResetClientsideFrame( void );

// Controllers.
	virtual	void			InitBoneControllers ( void );
	
	// Return's the controller's angle/position in bone space.
	float					GetBoneController ( int iController );

	// Maps the angle/position value you specify into the bone's start/end and sets the specified controller to the value.
	float					SetBoneController ( int iController, float flValue );
	
	void					GetVelocity(Vector *vVelocity, AngularImpulse *vAngVelocity);

	// these two need to move somewhere else
	int GetNumFlexControllers( void );
	const char *GetFlexDescFacs( int iFlexDesc );
	const char *GetFlexControllerName( int iFlexController );
	const char *GetFlexControllerType( int iFlexController );

	virtual	Vector GetGroundSpeedVelocity( void );

	bool GetIntervalMovement( float flIntervalUsed, bool &bMoveSeqFinished, Vector &newPosition, QAngle &newAngles );
	bool GetSequenceMovement( int nSequence, float fromCycle, float toCycle, Vector &deltaPosition, QAngle &deltaAngles );
	float GetInstantaneousVelocity( float flInterval = 0.0 );
	float GetEntryVelocity( int iSequence );
	float GetExitVelocity( int iSequence );
	float GetMovementFrame( float flDist );
	bool HasMovement( int iSequence );

	void ReportMissingActivity( int iActivity );
	virtual bool TestCollision( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr );
	virtual bool TestHitboxes( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr );
	class CBoneCache *GetBoneCache( void );
	void InvalidateBoneCache();
	void InvalidateBoneCacheIfOlderThan( float deltaTime );
	virtual int DrawDebugTextOverlays( void );
	
	// See note in code re: bandwidth usage!!!
	virtual void		DrawServerHitboxes( float duration = 0.0f, bool monocolor = false );

	void				SetModelWidthScale( float scale, float change_duration = 0.0f );
	float				GetModelWidthScale() const;

	void				UpdateModelWidthScale();
	
	// also calculate IK on server? (always done on client)
	void EnableServerIK();
	void DisableServerIK();

	// for ragdoll vs. car
	int GetHitboxesFrontside( int *boxList, int boxMax, const Vector &normal, float dist );

	void	GetInputDispatchEffectPosition( const char *sInputString, Vector &pOrigin, QAngle &pAngles );

	virtual void	ModifyOrAppendCriteria( AI_CriteriaSet& set );

	// Send a muzzle flash event to the client for this entity.
	void DoMuzzleFlash();

	// Fire
	virtual void Ignite( float flFlameLifetime, bool bNPCOnly = true, float flSize = 0.0f, bool bCalledByLevelDesigner = false );
	virtual void Extinguish() { RemoveFlag( FL_ONFIRE ); }
	int IsOnFire() { return (GetFlags() & FL_ONFIRE); }
	void Scorch( int rate, int floor );
	void InputIgnite( inputdata_t &inputdata );

	// Dissolve, returns true if the ragdoll has been created
	bool Dissolve( const char *pMaterialName, float flStartTime, bool bNPCOnly = true, int nDissolveType = 0 );
	int IsDissolving() { return (GetFlags() & FL_DISSOLVING); }
	void TransferDissolveFrom( CBaseAnimating *pAnim );

	// animation needs
	float				m_flGroundSpeed;	// computed linear movement rate for current sequence
	float				m_flLastEventCheck;	// cycle index of when events were last checked

	virtual void SetLightingOrigin( CBaseEntity *pLightingOrigin );
	CBaseEntity *GetLightingOrigin();

	const float* GetPoseParameterArray() { return m_flPoseParameter.Base(); }
	const float* GetEncodedControllerArray() { return m_flEncodedController.Base(); }

private:
	void StudioFrameAdvanceInternal( float flInterval );
	void SetLightingOrigin( string_t strLightingOrigin );
	void InputSetLightingOriginHack( inputdata_t &inputdata );

public:
	CNetworkVar( int, m_nForceBone );
	CNetworkVector( m_vecForce );

	CNetworkVar( int, m_nSkin );
	CNetworkVar( int, m_nBody );
	CNetworkVar( int, m_nHitboxSet );

	// For making things thin during barnacle swallowing, e.g.
	CNetworkVar( float, m_flModelWidthScale );

	// was pev->framerate
	CNetworkVar( float, m_flPlaybackRate );

public:
	void SetIKGroundContactInfo( float minHeight, float maxHeight );
	void UpdateStepOrigin( void );

protected:
	float				m_flIKGroundContactTime;
	float				m_flIKGroundMinHeight;
	float				m_flIKGroundMaxHeight;

	float				m_flEstIkFloor; // debounced
	float				m_flEstIkOffset;

  	CIKContext			*m_pIk;
	int					m_iIKCounter;

public:
	Vector	GetStepOrigin( void ) const;
	QAngle	GetStepAngles( void ) const;

private:
	bool				m_bSequenceFinished;// flag set when StudioAdvanceFrame moves across a frame boundry
	bool				m_bSequenceLoops;	// true if the sequence loops
	float				m_flDissolveStartTime;

	// was pev->frame
	CNetworkVar( float, m_flCycle );
	CNetworkVar( int, m_nSequence );	
	CNetworkArray( float, m_flPoseParameter, NUM_POSEPAREMETERS );	// must be private so manual mode works!
	CNetworkArray( float, m_flEncodedController, NUM_BONECTRLS );		// bone controller setting (0..1)

	// Client-side animation (useful for looping animation objects)
	CNetworkVar( bool, m_bClientSideAnimation );
	CNetworkVar( bool, m_bClientSideFrameReset );

	CNetworkVar( int, m_nNewSequenceParity );
	CNetworkVar( int, m_nResetEventsParity );

	// Incremented each time the entity is told to do a muzzle flash.
	// The client picks up the change and draws the flash.
	CNetworkVar( unsigned char, m_nMuzzleFlashParity );

	CNetworkHandle( CBaseEntity, m_hLightingOrigin );
	string_t m_iszLightingOrigin;	// for reading from the file only
	memhandle_t		m_boneCacheHandle;

public:
	COutputEvent m_OnIgnite;

// FIXME: necessary so that cyclers can hack m_bSequenceFinished
friend class CFlexCycler;
friend class CCycler;
friend class CBlendingCycler;
};


//-----------------------------------------------------------------------------
// Purpose: Serves the 90% case of calling SetSequence / ResetSequenceInfo.
//-----------------------------------------------------------------------------

/*
inline void CBaseAnimating::ResetSequence(int nSequence)
{
	m_nSequence = nSequence;
	ResetSequenceInfo();
}
*/

inline float CBaseAnimating::GetPlaybackRate()
{
	return m_flPlaybackRate;
}

inline void CBaseAnimating::SetPlaybackRate( float rate )
{
	m_flPlaybackRate = rate;
}

inline void CBaseAnimating::SetLightingOrigin( CBaseEntity *pLightingOrigin )
{
	m_hLightingOrigin = pLightingOrigin;
}

inline CBaseEntity *CBaseAnimating::GetLightingOrigin()
{
	return m_hLightingOrigin;
}

//-----------------------------------------------------------------------------
// Cycle access
//-----------------------------------------------------------------------------
inline float CBaseAnimating::GetCycle() const
{
	return m_flCycle;
}

inline void CBaseAnimating::SetCycle( float flCycle )
{
	m_flCycle = flCycle;
}


EXTERN_SEND_TABLE(DT_BaseAnimating);



#define ANIMATION_SEQUENCE_BITS			10	// 1024 sequences
#define ANIMATION_SKIN_BITS				10	// 1024 body skin selections FIXME: this seems way high
#define ANIMATION_BODY_BITS				32	// body combinations
#define ANIMATION_HITBOXSET_BITS		2	// hit box sets 
#define ANIMATION_POSEPARAMETER_BITS	11	// pose parameter resolution
#define ANIMATION_PLAYBACKRATE_BITS		8	// default playback rate, only used on leading edge detect sequence changes

#endif // BASEANIMATING_H
