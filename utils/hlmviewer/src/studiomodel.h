//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef INCLUDED_STUDIOMODEL
#define INCLUDED_STUDIOMODEL

#include "mathlib.h"
#include "studio.h"
#include "mouthinfo.h"
#include "UtlLinkedList.h"
#include "UtlSymbol.h"
#include "bone_setup.h"

#define DEFAULT_BLEND_TIME 0.2

typedef struct IFACE_TAG IFACE;
typedef struct IMESH_TAG IMESH;
typedef struct ResolutionUpdateTag ResolutionUpdate;
typedef struct FaceUpdateTag FaceUpdate;
class IMaterial;


class IStudioPhysics;
class CPhysmesh;
struct hlmvsolid_t;
struct constraint_ragdollparams_t;
class IStudioRender;


class AnimationLayer
{
public:
	float					m_cycle;			// 0 to 1 animation playback index
	int						m_sequence;			// sequence index
	float					m_weight;
	float					m_playbackrate;
	int						m_priority;			// lower priorities get layered first
};



class StudioModel
{
public:
	// memory handling, uses calloc so members are zero'd out on instantiation
    static void						*operator new( size_t stAllocateBlock );
	static void						operator delete( void *pMem );

	static void				Init( void );
	static void				Shutdown( void ); // garymcthack - need to call this.
	static void				UpdateViewState( const Vector& viewOrigin,
											 const Vector& viewRight,
											 const Vector& viewUp,
											 const Vector& viewPlaneNormal );

	static void						ReleaseStudioModel( void );
	static void						RestoreStudioModel( void );

	static void						UnloadGroupFiles();

	char const						*GetFileName( void ) { return m_pModelName; }

	IStudioRender				    *GetStudioRender() { return m_pStudioRender; }

	static void UpdateStudioRenderConfig( bool bFlat, bool bWireframe, bool bNormals );
	studiohdr_t						*getStudioHeader ( void ) const { return m_pstudiohdr; }
	studiohdr_t						*getAnimHeader (int i) const { return m_panimhdr[i]; }

	virtual void					ModelInit( void ) { }

	bool							IsModelLoaded() const { return m_pstudiohdr != 0; }

	void							FreeModel ();
	bool							LoadModel( const char *modelname );
	virtual bool					PostLoadModel ( const char *modelname );

	virtual int						DrawModel( bool mergeBones = false );

	virtual void					AdvanceFrame( float dt );
	float							GetInterval( void );
	float							GetCycle( void );
	float							GetFrame( void );
	int								GetMaxFrame( void );
	int								SetFrame( int frame );
	float							GetCycle( int iLayer );
	float							GetFrame( int iLayer );
	int								GetMaxFrame( int iLayer );
	int								SetFrame( int iLayer, int frame );

	void							ExtractBbox( Vector &mins, Vector &maxs );

	void							SetBlendTime( float blendtime );
	int								LookupSequence( const char *szSequence );
	int								SetSequence( int iSequence );
	int								SetSequence( const char *szSequence );
	void							ClearOverlaysSequences( void );
	void							ClearAnimationLayers( void );
	int								GetNewAnimationLayer( int iPriority = 0 );

	int								SetOverlaySequence( int iLayer, int iSequence, float flWeight );
	float							SetOverlayRate( int iLayer, float flCycle, float flFrameRate );
	int								GetOverlaySequence( int iLayer );
	float							GetOverlaySequenceWeight( int iLayer );
	void							StartBlending( void );

	float							GetTransitionAmount( void );
	int								GetSequence( void );
	void							GetSequenceInfo( int iSequence, float *pflFrameRate, float *pflGroundSpeed );
	void							GetSequenceInfo( float *pflFrameRate, float *pflGroundSpeed );
	float							GetFPS( int iSequence );
	float							GetFPS( );
	float							GetDuration( int iSequence );
	float							GetDuration( );
	int								GetNumFrames( int iSequence );
	bool							GetSequenceLoops( int iSequence );
	void							GetMovement( float prevCycle[5], Vector &vecPos, QAngle &vecAngles );
	void							GetMovement( int iSequence, float prevCycle, float currCycle, Vector &vecPos, QAngle &vecAngles );
	void							GetSeqAnims( int iSequence, mstudioanimdesc_t *panim[4], float *pweights );
	void							GetSeqAnims( mstudioanimdesc_t *panim[4], float *pweights );
	float							GetGroundSpeed( int iSequence );
	float							GetGroundSpeed( void );
	float							GetCurrentVelocity( void );
	bool							IsHidden( int iSequence );

	float							SetController( int iController, float flValue );

	int								LookupPoseParameter( char const *szName );
	float							SetPoseParameter( int iParameter, float flValue );
	float							SetPoseParameter( char const *szName, float flValue );
	float							GetPoseParameter( char const *szName );
	float							GetPoseParameter( int iParameter );
	bool 							GetPoseParameterRange( int iParameter, float *pflMin, float *pflMax );

	int								LookupAttachment( char const *szName );

	int								SetBodygroup( int iGroup, int iValue );
	int								SetSkin( int iValue );
	int								FindBone( const char *pName );

	int								LookupFlexController( char *szName );
	void							SetFlexController( char *szName, float flValue );
	void							SetFlexController( int iFlex, float flValue );
	float							GetFlexController( char *szName );
	float							GetFlexController( int iFlex );
	void							SetFlexControllerRaw( int iFlex, float flValue );
	float							GetFlexControllerRaw( int iFlex );

	// void							CalcBoneTransform( int iBone, Vector pos[], Quaternion q[], matrix3x4_t& bonematrix );

	void							UpdateBoneChain( Vector pos[], Quaternion q[], int iBone, matrix3x4_t *pBoneToWorld );
	void							SetViewTarget( void ); // ???
	void							GetBodyPoseParametersFromFlex( void );
	void							SetHeadPosition( Vector pos[], Quaternion q[] );
	float							SetHeadPosition( matrix3x4_t& attToWorld, Vector const &vTargetPos, float dt );

	int								GetNumLODs() const;
	float							GetLODSwitchValue( int lod ) const;
	void							SetLODSwitchValue( int lod, float switchValue );
	
	void							scaleMeshes (float scale);
	void							scaleBones (float scale);

	// Physics
	void							OverrideBones( bool *override );
	int								Physics_GetBoneCount( void );
	const char *					Physics_GetBoneName( int index );
	int								Physics_GetBoneIndex( const char *pName );
	void							Physics_GetData( int boneIndex, hlmvsolid_t *psolid, constraint_ragdollparams_t *pConstraint ) const;
	void							Physics_SetData( int boneIndex, const hlmvsolid_t *psolid, constraint_ragdollparams_t const *pConstraint );
	void							Physics_SetPreview( int previewBone, int axis, float t );
	float							Physics_GetMass( void );
	void							Physics_SetMass( float mass );
	char							*Physics_DumpQC( void );

public:
	// entity settings
	QAngle							m_angles;	// rot
	Vector							m_origin;	// trans

protected:
	int								m_bodynum;			// bodypart selection	
	int								m_skinnum;			// skin group selection
	float							m_controller[4];	// bone controllers

public:
	CMouthInfo						m_mouth;

protected:
	char							*m_pModelName;		// model file name

	// bool							m_owntexmodel;		// do we have a modelT.mdl ?

	// Previouse sequence data
	float							m_blendtime;
	float							m_sequencetime;
	int								m_prevsequence;
	float							m_prevcycle;

	float							m_dt;

	// Blending info

	// Gesture,Sequence layering state
#define MAXSTUDIOANIMLAYERS			8
	AnimationLayer					m_Layer[MAXSTUDIOANIMLAYERS];
	int								m_iActiveLayers;

public:
	float							m_cycle;			// 0 to 1 animation playback index
protected:
	int								m_sequence;			// sequence index
	float							m_poseparameter[MAXSTUDIOPOSEPARAM];		// intra-sequence blending
	float							m_weight;

	// internal data
	studiohdr_t						*m_pstudiohdr;
	mstudiomodel_t					*m_pmodel;
	studiohwdata_t					m_HardwareData;	

public:
	// I'm saving this as internal data because we may add or remove hitboxes
	// I'm using a utllinkedlist so hitbox IDs remain constant on add + remove
	typedef CUtlLinkedList<mstudiobbox_t, unsigned short> Hitboxes_t;
	CUtlVector< Hitboxes_t >		m_HitboxSets;
	struct hbsetname_s
	{
		char name[ 128 ];
	};

	CUtlVector< hbsetname_s >		m_HitboxSetNames;

	CUtlVector< CUtlSymbol >		m_SurfaceProps;
protected:

	// class data
	static IStudioRender			*m_pStudioRender;
	static Vector					*m_AmbientLightColors;

	// Added data
	// IMESH						*m_pimesh;
	// VertexUpdate					*m_pvertupdate;
	// FaceUpdate					*m_pfaceupdate;
	IFACE							*m_pface;

	// studiohdr_t					*m_ptexturehdr;
	studiohdr_t						*m_panimhdr[32];

	Vector4D						m_adj;				// FIX: non persistant, make static

public:
	IStudioPhysics					*m_pPhysics;
private:
	int								m_physPreviewBone;
	int								m_physPreviewAxis;
	float							m_physPreviewParam;
	float							m_physMass;

public:
	mstudioseqdesc_t				&GetSeqDesc( int seq );
private:
	mstudioanimdesc_t				&GetAnimDesc( int anim );
	mstudioanim_t					*GetAnim( int anim );

	void							Transform( Vector const &in1, mstudioboneweight_t const *pboneweight, Vector &out1 );
	void							Rotate( Vector const &in1, mstudioboneweight_t const *pboneweight, Vector &out1 );
	void							DrawPhysmesh( CPhysmesh *pMesh, int boneIndex, IMaterial *pMaterial, float *color );
	void							DrawPhysConvex( CPhysmesh *pMesh, IMaterial *pMaterial );

	void							SetupLighting( void );

	virtual void					SetupModel ( int bodypart );

private:
	float							m_flexweight[MAXSTUDIOFLEXCTRL];
public:
	virtual int						FlexVerts( mstudiomesh_t *pmesh );
	virtual void					RunFlexRules( void );
	virtual int						BoneMask( void );
	virtual void					SetUpBones ( bool mergeBones );

	int								GetLodUsed( void );
	float							GetLodMetric( void );

	const char						*GetKeyValueText( int iSequence );

private:
	// Drawing helper methods
	void DrawBones( );
	void DrawAttachments( );
	void DrawEditAttachment();
	void DrawHitboxes();
	void DrawPhysicsModel( );
	void DrawIllumPosition( );

public:
	// generic interface to rendering?
	void drawBox (Vector const *v, float const * color );
	void drawWireframeBox (Vector const *v, float const* color );
	void drawTransform( matrix3x4_t& m, float flLength = 4 );
	void drawLine( Vector const &p1, Vector const &p2, int r = 0, int g = 0, int b = 255 );
	void drawTransparentBox( Vector const &bbmin, Vector const &bbmax, const matrix3x4_t& m, float const *color, float const *wirecolor );

private:
	int						m_LodUsed;
	float					m_LodMetric;

public:

	void					SetSolveHeadTurn( int solve );
	int						GetSolveHeadTurn() const;

	//void					SetHeadTurn( float turn );
	//float					GetHeadTurn() const;
	void					ClearHeadTarget( void );

	void					SetHeadTarget( const Vector& target, float turn );
	void					GetHeadTarget( Vector& target ) const;

	void					SetEyeTarget( const Vector& target );
	void					GetEyeTarget( Vector& target ) const;

	void					SetHasLookTarget( bool hastarget );
	bool					GetHasLookTarget() const;

	void					SetModelYaw( float yaw );
	float					GetModelYaw( void ) const;
	void					SetBodyYaw( float yaw );
	float					GetBodyYaw( void ) const;
	void					SetSpineYaw( float yaw );
	float					GetSpineYaw( void ) const;

private:

	// 0 == no, 1 == based on dt, 2 == completely.
	int						m_nSolveHeadTurn; 
	float					m_flHeadTurn;
	Vector					m_vecHeadTarget;
	Vector					m_vecEyeTarget;
	bool					m_bHasLookTarget;

	float					m_flModelYaw;
	float					m_flBodyYaw;
	float					m_flSpineYaw;

public:
	bool					m_bIsTransparent;
	bool					m_bHasProxy;

	// necessary for accessing correct vertexes
	void					SetCurrentModel();

public:
	CIKContext				m_ik;
	float					m_prevGroundCycles[5];
	float					m_prevIKCycles[5];

public:
	void					IncrementFramecounter( void ) { m_iFramecounter++; };
private:
	int						m_iFramecounter;
};

extern Vector g_vright;		// needs to be set to viewer's right in order for chrome to work
extern StudioModel *g_pStudioModel;
extern StudioModel *g_pStudioExtraModel[4];

extern studiohdr_t *g_pCacheHdr;


#endif // INCLUDED_STUDIOMODEL