//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_baseanimating.h"
#include "c_Sprite.h"
#include "model_types.h"
#include "bone_setup.h"
#include "ivrenderview.h"
#include "r_efx.h"
#include "dlight.h"
#include "beamdraw.h"
#include "cl_animevent.h"
#include "engine/IEngineSound.h"
#include "c_te_legacytempents.h"
#include "activitylist.h"
#include "animation.h"
#include "tier0/vprof.h"
#include "ClientEffectPrecacheSystem.h"
#include "ieffects.h"
#include "engine/ivmodelinfo.h"
#include "engine/IVDebugOverlay.h"
#include "c_te_effect_dispatch.h"
#include <KeyValues.h>
#include "c_rope.h"
#include "isaverestore.h"
#include "datacache/imdlcache.h"
#include "eventlist.h"
#include "saverestore.h"
#include "physics_saverestore.h"
#include "vphysics/constraints.h"
#include "ragdoll_shared.h"
#include "view.h"
#include "view_scene.h"
#include "c_ai_basenpc.h"
#include "c_entitydissolve.h"
#include "saverestoretypes.h"
#include "c_fire_smoke.h"
#include "input.h"
#include "soundinfo.h"
#include "tools/bonelist.h"
#include "toolframework/itoolframework.h"
#include "datacache/idatacache.h"
#include "gamestringpool.h"

#include "irc/ff_socks.h"
#include "c_ff_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar cl_SetupAllBones( "cl_SetupAllBones", "0" );
ConVar r_sequence_debug( "r_sequence_debug", "" );

// dlight scale
extern ConVar cl_ffdlight_muzzle;

// If an NPC is moving faster than this, he should play the running footstep sound
const float RUN_SPEED_ESTIMATE_SQR = 150.0f * 150.0f;

// Removed macro used by shared code stuff
#if defined( CBaseAnimating )
#undef CBaseAnimating
#endif


CLIENTEFFECT_REGISTER_BEGIN( PrecacheBaseAnimating )
CLIENTEFFECT_MATERIAL( "sprites/fire" )
CLIENTEFFECT_REGISTER_END()

mstudioevent_t *GetEventIndexForSequence( mstudioseqdesc_t &seqdesc );

C_EntityDissolve *DissolveEffect( C_BaseAnimating *pTarget, float flTime );
C_EntityFlame *FireEffect( C_BaseAnimating *pTarget, C_BaseEntity *pServerFire, float *flScaleEnd, float *flTimeStart, float *flTimeEnd );
bool NPC_IsImportantNPC( C_BaseAnimating *pAnimating );

bool C_AnimationLayer::IsActive( void )
{
	return (m_nOrder != C_BaseAnimatingOverlay::MAX_OVERLAYS);
}

//-----------------------------------------------------------------------------
// Relative lighting entity
//-----------------------------------------------------------------------------
class C_InfoLightingRelative : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_InfoLightingRelative, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	void GetLightingOffset( matrix3x4_t &offset );

private:
	EHANDLE			m_hLightingLandmark;
};

IMPLEMENT_CLIENTCLASS_DT(C_InfoLightingRelative, DT_InfoLightingRelative, CInfoLightingRelative)
	RecvPropEHandle(RECVINFO(m_hLightingLandmark)),
END_RECV_TABLE()


//-----------------------------------------------------------------------------
// Relative lighting entity
//-----------------------------------------------------------------------------
void C_InfoLightingRelative::GetLightingOffset( matrix3x4_t &offset )
{
	if ( m_hLightingLandmark.Get() )
	{
		matrix3x4_t matWorldToLandmark;
 		MatrixInvert( m_hLightingLandmark->EntityToWorldTransform(), matWorldToLandmark );
		ConcatTransforms( EntityToWorldTransform(), matWorldToLandmark, offset );
	}
	else
	{
		SetIdentityMatrix( offset );
	}
}


//-----------------------------------------------------------------------------
// Base Animating
//-----------------------------------------------------------------------------

struct clientanimating_t
{
	C_BaseAnimating *pAnimating;
	unsigned int	flags;
	clientanimating_t(C_BaseAnimating *_pAnim, unsigned int _flags ) : pAnimating(_pAnim), flags(_flags) {}
};

const unsigned int FCLIENTANIM_SEQUENCE_CYCLE = 0x00000001;

static CUtlVector< clientanimating_t >	g_ClientSideAnimationList;

BEGIN_RECV_TABLE_NOBASE( C_BaseAnimating, DT_ServerAnimationData )
	RecvPropFloat(RECVINFO(m_flCycle)),
END_RECV_TABLE()


IMPLEMENT_CLIENTCLASS_DT(C_BaseAnimating, DT_BaseAnimating, CBaseAnimating)
	RecvPropInt(RECVINFO(m_nSequence)),
	RecvPropInt(RECVINFO(m_nForceBone)),
	RecvPropVector(RECVINFO(m_vecForce)),
	RecvPropInt(RECVINFO(m_nSkin)),
	RecvPropInt(RECVINFO(m_nBody)),
	RecvPropInt(RECVINFO(m_nHitboxSet)),
	RecvPropFloat(RECVINFO(m_flModelWidthScale)),

//	RecvPropArray(RecvPropFloat(RECVINFO(m_flPoseParameter[0])), m_flPoseParameter),
	RecvPropArray3(RECVINFO_ARRAY(m_flPoseParameter), RecvPropFloat(RECVINFO(m_flPoseParameter[0])) ),
	
	RecvPropFloat(RECVINFO(m_flPlaybackRate)),

	RecvPropArray3( RECVINFO_ARRAY(m_flEncodedController), RecvPropFloat(RECVINFO(m_flEncodedController[0]))),

	RecvPropInt( RECVINFO( m_bClientSideAnimation )),
	RecvPropInt( RECVINFO( m_bClientSideFrameReset )),

	RecvPropInt( RECVINFO( m_nNewSequenceParity )),
	RecvPropInt( RECVINFO( m_nResetEventsParity )),
	RecvPropInt( RECVINFO( m_nMuzzleFlashParity ) ),

	RecvPropEHandle(RECVINFO(m_hLightingOrigin)),
	RecvPropEHandle(RECVINFO(m_hLightingOriginRelative)),

	RecvPropDataTable( "serveranimdata", 0, 0, &REFERENCE_RECV_TABLE( DT_ServerAnimationData ) ),

	RecvPropFloat( RECVINFO( m_fadeMinDist ) ), 
	RecvPropFloat( RECVINFO( m_fadeMaxDist ) ), 
	RecvPropFloat( RECVINFO( m_flFadeScale ) ), 

END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_BaseAnimating )

	DEFINE_PRED_FIELD( m_nSkin, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nBody, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
//	DEFINE_PRED_FIELD( m_nHitboxSet, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
//	DEFINE_PRED_FIELD( m_flModelWidthScale, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nSequence, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_flPlaybackRate, FIELD_FLOAT, FTYPEDESC_INSENDTABLE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_flCycle, FIELD_FLOAT, FTYPEDESC_INSENDTABLE | FTYPEDESC_NOERRORCHECK ),
//	DEFINE_PRED_ARRAY( m_flPoseParameter, FIELD_FLOAT, MAXSTUDIOPOSEPARAM, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_ARRAY_TOL( m_flEncodedController, FIELD_FLOAT, MAXSTUDIOBONECTRLS, FTYPEDESC_INSENDTABLE, 0.02f ),

	DEFINE_FIELD( m_nPrevSequence, FIELD_INTEGER ),
	//DEFINE_FIELD( m_flPrevEventCycle, FIELD_FLOAT ),
	//DEFINE_FIELD( m_flEventCycle, FIELD_FLOAT ),
	//DEFINE_FIELD( m_nEventSequence, FIELD_INTEGER ),

	DEFINE_PRED_FIELD( m_nNewSequenceParity, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_nResetEventsParity, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_NOERRORCHECK ),
	// DEFINE_PRED_FIELD( m_nPrevResetEventsParity, FIELD_INTEGER, 0 ),

	DEFINE_PRED_FIELD( m_nMuzzleFlashParity, FIELD_CHARACTER, FTYPEDESC_INSENDTABLE ),
	//DEFINE_FIELD( m_nOldMuzzleFlashParity, FIELD_CHARACTER ),

	//DEFINE_FIELD( m_nPrevNewSequenceParity, FIELD_INTEGER ),
	//DEFINE_FIELD( m_nPrevResetEventsParity, FIELD_INTEGER ),

	// DEFINE_PRED_FIELD( m_vecForce, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	// DEFINE_PRED_FIELD( m_nForceBone, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	// DEFINE_PRED_FIELD( m_bClientSideAnimation, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	// DEFINE_PRED_FIELD( m_bClientSideFrameReset, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	
	// DEFINE_FIELD( m_pRagdollInfo, RagdollInfo_t ),
	// DEFINE_FIELD( m_CachedBones, CUtlVector < CBoneCacheEntry > ),
	// DEFINE_FIELD( m_pActualAttachmentAngles, FIELD_VECTOR ),
	// DEFINE_FIELD( m_pActualAttachmentOrigin, FIELD_VECTOR ),

	// DEFINE_FIELD( m_animationQueue, CUtlVector < C_AnimationLayer > ),
	// DEFINE_FIELD( m_pIk, CIKContext ),
	// DEFINE_FIELD( m_bLastClientSideFrameReset, FIELD_BOOLEAN ),
	// DEFINE_FIELD( hdr, studiohdr_t ),
	// DEFINE_FIELD( m_pRagdoll, IRagdoll ),
	// DEFINE_FIELD( m_bStoreRagdollInfo, FIELD_BOOLEAN ),

	// DEFINE_FIELD( C_BaseFlex, m_iEyeAttachment, FIELD_INTEGER ),

END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( client_ragdoll, C_ClientRagdoll );

BEGIN_DATADESC( C_ClientRagdoll )
	DEFINE_FIELD( m_bFadeOut, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bImportant, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iCurrentFriction, FIELD_INTEGER ),
	DEFINE_FIELD( m_iMinFriction, FIELD_INTEGER ),
	DEFINE_FIELD( m_iMaxFriction, FIELD_INTEGER ),
	DEFINE_FIELD( m_flFrictionModTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_flFrictionTime, FIELD_TIME ),
	DEFINE_FIELD( m_iFrictionAnimState, FIELD_INTEGER ),
	DEFINE_FIELD( m_bReleaseRagdoll, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_nBody, FIELD_INTEGER ),
	DEFINE_FIELD( m_nSkin, FIELD_INTEGER ),
	DEFINE_FIELD( m_nRenderFX, FIELD_CHARACTER ),
	DEFINE_FIELD( m_nRenderMode, FIELD_CHARACTER ),
	DEFINE_FIELD( m_clrRender, FIELD_COLOR32 ),
	DEFINE_FIELD( m_flEffectTime, FIELD_TIME ),
	DEFINE_FIELD( m_bFadingOut, FIELD_BOOLEAN ),

	DEFINE_AUTO_ARRAY( m_flScaleEnd, FIELD_FLOAT ),
	DEFINE_AUTO_ARRAY( m_flScaleTimeStart, FIELD_FLOAT ),
	DEFINE_AUTO_ARRAY( m_flScaleTimeEnd, FIELD_FLOAT ),
	DEFINE_EMBEDDEDBYREF( m_pRagdoll ),

END_DATADESC()

C_ClientRagdoll::C_ClientRagdoll( bool bRestoring )
{
	m_iCurrentFriction = 0;
	m_iFrictionAnimState = RAGDOLL_FRICTION_NONE;
	m_bReleaseRagdoll = false;
	m_bFadeOut = false;
	m_bFadingOut = false;
	m_bImportant = false;

	SetClassname("client_ragdoll");

	if ( bRestoring == true )
	{
		m_pRagdoll = new CRagdoll;
	}
}

void C_ClientRagdoll::OnSave( void )
{
	C_EntityFlame *pFireChild = dynamic_cast<C_EntityFlame *>( GetEffectEntity() );

	if ( pFireChild )
	{
		for ( int i = 0; i < NUM_HITBOX_FIRES; i++ )
		{
			if ( pFireChild->m_pFireSmoke[i] != NULL )
			{
				 m_flScaleEnd[i] = pFireChild->m_pFireSmoke[i]->m_flScaleEnd;
				 m_flScaleTimeStart[i] = pFireChild->m_pFireSmoke[i]->m_flScaleTimeStart;
				 m_flScaleTimeEnd[i] = pFireChild->m_pFireSmoke[i]->m_flScaleTimeEnd;
			}
		}
	}
}

void C_ClientRagdoll::OnRestore( void )
{
	CStudioHdr *hdr = GetModelPtr();

	if ( hdr == NULL )
	{
		const char *pModelName = STRING( GetModelName() );
		SetModel( pModelName );

		hdr = GetModelPtr();

		if ( hdr == NULL )
			return;
	}
	
	if ( m_pRagdoll == NULL )
		 return;

	ragdoll_t *pRagdollT = m_pRagdoll->GetRagdoll();

	if ( pRagdollT == NULL || pRagdollT->list[0].pObject == NULL )
	{
		m_bReleaseRagdoll = true;
		m_pRagdoll = NULL;
		Assert( !"Attempted to restore a ragdoll without physobjects!" );
		return;
	}

	if ( GetFlags() & FL_DISSOLVING )
	{
		DissolveEffect( this, m_flEffectTime );
	}
	else if ( GetFlags() & FL_ONFIRE )
	{
		C_EntityFlame *pFireChild = dynamic_cast<C_EntityFlame *>( GetEffectEntity() );
		C_EntityFlame *pNewFireChild = FireEffect( this, pFireChild, m_flScaleEnd, m_flScaleTimeStart, m_flScaleTimeEnd );

		//Set the new fire child as the new effect entity.
		SetEffectEntity( pNewFireChild );
	}

	VPhysicsSetObject( NULL );
	VPhysicsSetObject( pRagdollT->list[0].pObject );

	SetupBones( NULL, -1, BONE_USED_BY_ANYTHING, gpGlobals->curtime );

	pRagdollT->list[0].parentIndex = -1;
	pRagdollT->list[0].originParentSpace.Init();

	RagdollActivate( *pRagdollT, modelinfo->GetVCollide( GetModelIndex() ), GetModelIndex(), false );
	RagdollSetupAnimatedFriction( physenv, pRagdollT, GetModelIndex() );

	m_pRagdoll->BuildRagdollBounds( this );

	// UNDONE: The shadow & leaf system cleanup should probably be in C_BaseEntity::OnRestore()
	// this must be recomputed because the model was NULL when this was set up
	RemoveFromLeafSystem();
	AddToLeafSystem( RENDER_GROUP_OPAQUE_ENTITY );

	DestroyShadow();
 	CreateShadow();

	SetNextClientThink( CLIENT_THINK_ALWAYS );
	
	if ( m_bFadeOut == true )
	{
		s_RagdollLRU.MoveToTopOfLRU( this, m_bImportant );
	}

	NoteRagdollCreationTick( this );
	
	BaseClass::OnRestore();

	RagdollMoved();
}

void C_ClientRagdoll::ImpactTrace( trace_t *pTrace, int iDamageType, char *pCustomImpactName )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();

	if( !pPhysicsObject )
		return;

	Vector dir = pTrace->endpos - pTrace->startpos;

	if ( iDamageType == DMG_BLAST )
	{
		dir *= 500;  // adjust impact strenght

		// apply force at object mass center
		pPhysicsObject->ApplyForceCenter( dir );
	}
	else
	{
		Vector hitpos;  
	
		VectorMA( pTrace->startpos, pTrace->fraction, dir, hitpos );
		VectorNormalize( dir );

		dir *= 4000;  // adjust impact strenght

		// apply force where we hit it
		pPhysicsObject->ApplyForceOffset( dir, hitpos );	
	}

	m_pRagdoll->ResetRagdollSleepAfterTime();
}

ConVar g_debug_ragdoll_visualize( "g_debug_ragdoll_visualize", "0", FCVAR_CHEAT );

void C_ClientRagdoll::HandleAnimatedFriction( void )
{
	if ( m_iFrictionAnimState == RAGDOLL_FRICTION_OFF )
		 return;

	ragdoll_t *pRagdollT = NULL;
	int iBoneCount = 0;

	if ( m_pRagdoll )
	{
		pRagdollT = m_pRagdoll->GetRagdoll();
		iBoneCount = m_pRagdoll->RagdollBoneCount();

	}

	if ( pRagdollT == NULL )
		 return;
	
	switch ( m_iFrictionAnimState )
	{
		case RAGDOLL_FRICTION_NONE:
		{
			m_iMinFriction = pRagdollT->animfriction.iMinAnimatedFriction;
			m_iMaxFriction = pRagdollT->animfriction.iMaxAnimatedFriction;

			if ( m_iMinFriction != 0 || m_iMaxFriction != 0 )
			{
				m_iFrictionAnimState = RAGDOLL_FRICTION_IN;

				m_flFrictionModTime = pRagdollT->animfriction.flFrictionTimeIn;
				m_flFrictionTime = gpGlobals->curtime + m_flFrictionModTime;
				
				m_iCurrentFriction = m_iMinFriction;
			}
			else
			{
				m_iFrictionAnimState = RAGDOLL_FRICTION_OFF;
			}
			
			break;
		}

		case RAGDOLL_FRICTION_IN:
		{
			float flDeltaTime = (m_flFrictionTime - gpGlobals->curtime);

			m_iCurrentFriction = RemapValClamped( flDeltaTime , m_flFrictionModTime, 0, m_iMinFriction, m_iMaxFriction );

			if ( flDeltaTime <= 0.0f )
			{
				m_flFrictionModTime = pRagdollT->animfriction.flFrictionTimeHold;
				m_flFrictionTime = gpGlobals->curtime + m_flFrictionModTime;
				m_iFrictionAnimState = RAGDOLL_FRICTION_HOLD;
			}
			break;
		}

		case RAGDOLL_FRICTION_HOLD:
		{
			if ( m_flFrictionTime < gpGlobals->curtime )
			{
				m_flFrictionModTime = pRagdollT->animfriction.flFrictionTimeOut;
				m_flFrictionTime = gpGlobals->curtime + m_flFrictionModTime;
				m_iFrictionAnimState = RAGDOLL_FRICTION_OUT;
			}
			
			break;
		}

		case RAGDOLL_FRICTION_OUT:
		{
			float flDeltaTime = (m_flFrictionTime - gpGlobals->curtime);

			m_iCurrentFriction = RemapValClamped( flDeltaTime , 0, m_flFrictionModTime, m_iMinFriction, m_iMaxFriction );

			if ( flDeltaTime <= 0.0f )
			{
				m_iFrictionAnimState = RAGDOLL_FRICTION_OFF;
			}

			break;
		}
	}

	for ( int i = 0; i < iBoneCount; i++ )
	{
		if ( pRagdollT->list[i].pConstraint )
			 pRagdollT->list[i].pConstraint->SetAngularMotor( 0, m_iCurrentFriction );
	}

	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();

	if ( pPhysicsObject )
	{
			pPhysicsObject->Wake();
	}
}

ConVar g_ragdoll_fadespeed( "g_ragdoll_fadespeed", "600" );
ConVar g_ragdoll_lvfadespeed( "g_ragdoll_lvfadespeed", "100" );

void C_ClientRagdoll::OnPVSStatusChanged( bool bInPVS )
{
	if ( bInPVS )
	{
		CreateShadow();
	}
	else
	{
		DestroyShadow();
	}
}

void C_ClientRagdoll::FadeOut( void )
{
	if ( m_bFadingOut == false )
	{
		 return;
	}

	int iAlpha = GetRenderColor().a;
	int iFadeSpeed = ( g_RagdollLVManager.IsLowViolence() ) ? g_ragdoll_lvfadespeed.GetInt() : g_ragdoll_fadespeed.GetInt();

	iAlpha = max( iAlpha - ( iFadeSpeed * gpGlobals->frametime ), 0 );

	SetRenderMode( kRenderTransAlpha );
	SetRenderColorA( iAlpha );

	if ( iAlpha == 0 )
	{
		m_bReleaseRagdoll = true;
	}
}

void C_ClientRagdoll::SUB_Remove( void )
{
	m_bFadingOut = true;
	SetNextClientThink( CLIENT_THINK_ALWAYS );
}

void C_ClientRagdoll::ClientThink( void )
{
	if ( m_bReleaseRagdoll == true )
	{
		Release();
		return;
	}

	if ( g_debug_ragdoll_visualize.GetBool() )
	{
		Vector vMins, vMaxs;
			
		Vector origin = m_pRagdoll->GetRagdollOrigin();
		m_pRagdoll->GetRagdollBounds( vMins, vMaxs );

		debugoverlay->AddBoxOverlay( origin, vMins, vMaxs, QAngle( 0, 0, 0 ), 0, 255, 0, 16, 0 );
	}

	HandleAnimatedFriction();

	FadeOut();
}

//-----------------------------------------------------------------------------
// Purpose: clear out any face/eye values stored in the material system
//-----------------------------------------------------------------------------
void C_ClientRagdoll::SetupWeights( void )
{
	BaseClass::SetupWeights( );

	static float destweight[MAXSTUDIOFLEXDESC];
	static bool bIsInited = false;

	CStudioHdr *hdr = GetModelPtr();
	if ( !hdr )
	{
		return;
	}

	if (hdr->numflexdesc() > 0)
	{
		if (!bIsInited)
		{
			int i;
			for (i = 0; i < MAXSTUDIOFLEXDESC; i++)
			{
				destweight[i] = 0.0f;
			}
			bIsInited = true;
		}
		modelrender->SetFlexWeights( hdr->numflexdesc(), destweight );
	}

	if (m_iEyeAttachment > 0)
	{
		matrix3x4_t attToWorld;
		if (GetAttachment( m_iEyeAttachment, attToWorld ))
		{
			Vector local, tmp;
			local.Init( 1000.0f, 0.0f, 0.0f );
			VectorTransform( local, attToWorld, tmp );
			modelrender->SetViewTarget( tmp );
		}
	}
}

void C_ClientRagdoll::Release( void )
{
	C_BaseEntity *pChild = GetEffectEntity();

	if ( pChild && pChild->IsMarkedForDeletion() == false )
	{
		pChild->Release();
	}

	if ( GetThinkHandle() != INVALID_THINK_HANDLE )
	{
		ClientThinkList()->RemoveThinkable( GetClientHandle() );
	}
	ClientEntityList().RemoveEntity( GetClientHandle() );

	partition->Remove( PARTITION_CLIENT_SOLID_EDICTS | PARTITION_CLIENT_RESPONSIVE_EDICTS | PARTITION_CLIENT_NON_STATIC_EDICTS, CollisionProp()->GetPartitionHandle() );
	RemoveFromLeafSystem();

	BaseClass::Release();
}

//-----------------------------------------------------------------------------
// Incremented each frame in InvalidateModelBones. Models compare this value to what it
// was last time they setup their bones to determine if they need to re-setup their bones.
static unsigned long	g_iModelBoneCounter = 0;

//-----------------------------------------------------------------------------
// Purpose: convert axis rotations to a quaternion
//-----------------------------------------------------------------------------
C_BaseAnimating::C_BaseAnimating() :
	m_iv_flCycle( "C_BaseAnimating::m_iv_flCycle" ),
	m_iv_flPoseParameter( "C_BaseAnimating::m_iv_flPoseParameter" ),
	m_iv_flEncodedController("C_BaseAnimating::m_iv_flEncodedController")
{
#ifdef _DEBUG
	m_vecForce.Init();
#endif

	m_colorMuzzleDLight.r = 255;
	m_colorMuzzleDLight.g = 192;
	m_colorMuzzleDLight.b = 64;
	m_colorMuzzleDLight.exponent = 5;
	
	m_ClientSideAnimationListHandle = INVALID_CLIENTSIDEANIMATION_LIST_HANDLE;

	m_nPrevSequence = -1;
	m_nRestoreSequence = -1;
	m_pRagdoll		= NULL;
	m_builtRagdoll = false;
	m_hitboxBoneCacheHandle = 0;
	int i;
	for ( i = 0; i < ARRAYSIZE( m_flEncodedController ); i++ )
	{
		m_flEncodedController[ i ] = 0.0f;
	}

	AddBaseAnimatingInterpolatedVars();
	m_iMostRecentModelBoneCounter = 0xFFFFFFFF;

	m_vecPreRagdollMins = vec3_origin;
	m_vecPreRagdollMaxs = vec3_origin;

	m_bStoreRagdollInfo = false;
	m_pRagdollInfo = NULL;

	m_flPlaybackRate = 1.0f;

	m_nEventSequence = -1;

	m_pIk = NULL;

	// Assume false.  Derived classes might fill in a receive table entry
	// and in that case this would show up as true
	m_bClientSideAnimation = false;

	m_nPrevNewSequenceParity = -1;
	m_nPrevResetEventsParity = -1;

	m_nOldMuzzleFlashParity = 0;
	m_nMuzzleFlashParity = 0;

	m_flModelWidthScale = 1.0f;

	m_iEyeAttachment = 0;
#ifdef _XBOX
	m_iAccumulatedBoneMask = 0;
#endif

	m_bAttemptingMatHack = false;

	m_pStudioHdr = NULL;
}

// Finds the specified material to use as the override material
// MATERIAL SHOULD BE PRECACHED ELSEWHERE
void C_BaseAnimating::FindOverrideMaterial( char const* pMaterialName, const char *pTextureGroupName, bool complain, const char *pComplainPrefix )
{
	// release if we're getting a new material
	if ( m_pOverrideMaterial )
		if ( strcmp( m_pOverrideMaterial->GetName(), pMaterialName ) != 0 )
			ReleaseOverrideMaterial();

	// only get it if we don't have one already
	if (!m_pOverrideMaterial)
	{
		m_pOverrideMaterial = materials->FindMaterial(pMaterialName, pTextureGroupName, complain, pComplainPrefix);
			if ( m_pOverrideMaterial )
				m_pOverrideMaterial->IncrementReferenceCount();
	}
}

// Releases the override material
void C_BaseAnimating::ReleaseOverrideMaterial( char const* pMaterialName )
{
	// can't release if it doesn't exist
	if (!m_pOverrideMaterial)
		return;

	// if a material name is specified, only release if names match
	if ( pMaterialName )
		if ( strcmp( m_pOverrideMaterial->GetName(), pMaterialName ) != 0  )
			return;

	// reference counting is used instead of actual allocating/deallocating
	m_pOverrideMaterial->DecrementReferenceCount();
	m_pOverrideMaterial = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: cleanup
//-----------------------------------------------------------------------------
C_BaseAnimating::~C_BaseAnimating()
{
	RemoveFromClientSideAnimationList();

	if ( m_pOverrideMaterial )
		ReleaseOverrideMaterial();

	TermRopes();
	delete m_pRagdollInfo;
	Assert(!m_pRagdoll);
	delete m_pIk;
	delete m_pBoneMergeCache;
	Studio_DestroyBoneCache( m_hitboxBoneCacheHandle );
	delete m_pStudioHdr;
}

bool C_BaseAnimating::UsesFrameBufferTexture( void )
{
	return modelinfo->IsUsingFBTexture( GetModel() );
}


//-----------------------------------------------------------------------------
// VPhysics object
//-----------------------------------------------------------------------------
int C_BaseAnimating::VPhysicsGetObjectList( IPhysicsObject **pList, int listMax )
{
	if ( IsRagdoll() )
	{
		int i;
		for ( i = 0; i < m_pRagdoll->RagdollBoneCount(); ++i )
		{
			if ( i >= listMax )
				break;

			pList[i] = m_pRagdoll->GetElement(i);
		}
		return i;
	}

	return BaseClass::VPhysicsGetObjectList( pList, listMax );
}


//-----------------------------------------------------------------------------
// Should this object cast render-to-texture shadows?
//-----------------------------------------------------------------------------
ShadowType_t C_BaseAnimating::ShadowCastType()
{
	CStudioHdr *pStudioHdr = GetModelPtr();
	if ( !pStudioHdr || !pStudioHdr->SequencesAvailable() )
		return SHADOWS_NONE;

	if ( IsEffectActive(EF_NODRAW | EF_NOSHADOW) )
		return SHADOWS_NONE;

	if (pStudioHdr->GetNumSeq() == 0)
		return SHADOWS_RENDER_TO_TEXTURE;
		  
	if ( !IsRagdoll() )
	{
		// If we have pose parameters, always update
		if ( pStudioHdr->GetNumPoseParameters() > 0 )
			return SHADOWS_RENDER_TO_TEXTURE_DYNAMIC;
		
		// If we have bone controllers, always update
		if ( pStudioHdr->numbonecontrollers() > 0 )
			return SHADOWS_RENDER_TO_TEXTURE_DYNAMIC;

		// If we use IK, always update
		if ( pStudioHdr->numikchains() > 0 )
			return SHADOWS_RENDER_TO_TEXTURE_DYNAMIC;
	}

	// FIXME: Do something to check to see how many frames the current animation has
	// If we do this, we have to be able to handle the case of changing ShadowCastTypes
	// at the moment, they are assumed to be constant.
	return SHADOWS_RENDER_TO_TEXTURE;
}

//-----------------------------------------------------------------------------
// Purpose: convert axis rotations to a quaternion
//-----------------------------------------------------------------------------

void C_BaseAnimating::SetPredictable( bool state )
{
	BaseClass::SetPredictable( state );

	UpdateRelevantInterpolatedVars();
}

void C_BaseAnimating::UpdateRelevantInterpolatedVars()
{
	// Remove any interpolated vars that need to be removed.
	if ( !GetPredictable() && !IsClientCreated() && GetModelPtr() && GetModelPtr()->SequencesAvailable() )
	{
		AddBaseAnimatingInterpolatedVars();
	}			
	else
	{
		RemoveBaseAnimatingInterpolatedVars();
	}
}


void C_BaseAnimating::AddBaseAnimatingInterpolatedVars()
{
	AddVar( m_flEncodedController, &m_iv_flEncodedController, LATCH_ANIMATION_VAR, true );
	AddVar( m_flPoseParameter, &m_iv_flPoseParameter, LATCH_ANIMATION_VAR, true );
	
	int flags = LATCH_ANIMATION_VAR;
	if ( m_bClientSideAnimation )
		flags |= EXCLUDE_AUTO_INTERPOLATE;
		
	AddVar( &m_flCycle, &m_iv_flCycle, flags, true );
}

void C_BaseAnimating::RemoveBaseAnimatingInterpolatedVars()
{
	RemoveVar( m_flEncodedController, false );
	RemoveVar( m_flPoseParameter, false );
	RemoveVar( &m_flCycle, false );
}

CStudioHdr *C_BaseAnimating::OnNewModel()
{
	if (m_pStudioHdr)
	{
		delete m_pStudioHdr;
		m_pStudioHdr = NULL;
	}

	// remove transition animations playback
	m_SequenceTransitioner.RemoveAll();

	if ( !GetModel() )
		return NULL;

	m_pStudioHdr = new CStudioHdr( modelinfo->GetStudiomodel( GetModel() ), mdlcache );

	UpdateRelevantInterpolatedVars();

	CStudioHdr *hdr = GetModelPtr();
	if (hdr == NULL)
		return NULL;

	InvalidateBoneCache();
	if ( m_pBoneMergeCache )
	{
		delete m_pBoneMergeCache;
		m_pBoneMergeCache = NULL;
		// recreated in BuildTransformations
	}

	Studio_DestroyBoneCache( m_hitboxBoneCacheHandle );
	m_hitboxBoneCacheHandle = 0;

	// Make sure m_CachedBones has space.
	if ( m_CachedBoneData.Count() != hdr->numbones() )
	{
		m_CachedBoneData.SetSize( hdr->numbones() );
		for ( int i=0; i < hdr->numbones(); i++ )
		{
			SetIdentityMatrix( m_CachedBoneData[i] );
		}
	}
	m_BoneAccessor.Init( this, m_CachedBoneData.Base() ); // Always call this in case the studiohdr_t has changed.

	// Free any IK data
	if (m_pIk)
	{
		delete m_pIk;
		m_pIk = NULL;
	}

	// Don't reallocate unless a different size. 
	if ( m_Attachments.Count() != hdr->GetNumAttachments())
	{
		m_Attachments.SetSize( hdr->GetNumAttachments() );

#ifdef _DEBUG
		// This is to make sure we don't use the attachment before its been set up
		for ( int i=0; i < m_Attachments.Count(); i++ )
		{
			float *pOrg = m_Attachments[i].m_vOrigin.Base();
			float *pAng = m_Attachments[i].m_angRotation.Base();
			pOrg[0] = pOrg[1] = pOrg[2] = VEC_T_NAN;
			pAng[0] = pAng[1] = pAng[2] = VEC_T_NAN;
		}
#endif

	}

	Assert( hdr->GetNumPoseParameters() <= ARRAYSIZE( m_flPoseParameter ) );

	m_iv_flPoseParameter.SetMaxCount( hdr->GetNumPoseParameters() );
	
	int i;
	for ( i = 0; i < hdr->GetNumPoseParameters() ; i++ )
	{
		const mstudioposeparamdesc_t &Pose = hdr->pPoseParameter( i );
		m_iv_flPoseParameter.SetLooping( Pose.loop != 0.0f, i );
		// Note:  We can't do this since if we get a DATA_UPDATE_CREATED (i.e., new entity) with both a new model and some valid pose parameters this will slam the 
		//  pose parameters to zero and if the model goes dormant the pose parameter field will never be set to the true value.  We shouldn't have to zero these out
		//  as they are under the control of the server and should be properly set
		if ( !IsServerEntity() )
		{
			SetPoseParameter( hdr, i, 0.0 );
		}
	}

	int boneControllerCount = min( hdr->numbonecontrollers(), ARRAYSIZE( m_flEncodedController ) );

	m_iv_flEncodedController.SetMaxCount( boneControllerCount );

	for ( i = 0; i < boneControllerCount ; i++ )
	{
		bool loop = (hdr->pBonecontroller( i )->type & (STUDIO_XR | STUDIO_YR | STUDIO_ZR)) != 0;
		m_iv_flEncodedController.SetLooping( loop, i );
		SetBoneController( i, 0.0 );
	}

	InitRopes();

	// lookup generic eye attachment, if exists
	m_iEyeAttachment = LookupAttachment( "eyes" );

	// If we didn't have a model before, then we might need to go in the interpolation list now.
	if ( ShouldInterpolate() )
		AddToInterpolationList();

	// objects with attachment points need to be queryable even if they're not solid
	if ( hdr->GetNumAttachments() != 0 )
	{
		AddEFlags( EFL_USE_PARTITION_WHEN_NOT_SOLID );
	}


	studiohdr_t *pStudioModel = modelinfo->GetStudiomodel( const_cast<model_t*>(GetModel()) );

	if (pStudioModel)
	{
		for (int i=0; i<pStudioModel->numtextures; i++)
		{
			mstudiotexture_t *pTex = pStudioModel->pTexture(i);
			if (pTex)
			{
				// there are way less texture paths (cdtexture) than textures, and there's no way to determine exactly
				// which cdtexture is associated with which texture
				// so just loop through all cdtextures for each texture and see if they exist
				for (int j=0; j<pStudioModel->numcdtextures; j++)
				{
					char searchPath[MAX_PATH] = "";
					Q_strcat(searchPath, pStudioModel->pCdtexture(j), MAX_PATH);
					Q_strcat(searchPath, pTex->pszName(), MAX_PATH);
					
					//Msg("[mathack] %s findmaterial: %s\n", pStudioModel->pszName(), searchPath);

					IMaterial *pMat = materials->FindMaterial(searchPath, TEXTURE_GROUP_MODEL);
					if (pMat)
					{
						//Msg("[mathack] -> material found\n");
						int ignorezval = (int)(pMat->GetMaterialVarFlag( MATERIAL_VAR_IGNOREZ ));
						if (ignorezval != 0)
						{
							DevMsg("[mathack] %s -> %s ignorez is true\n", pStudioModel->pszName(), searchPath);
							m_bAttemptingMatHack = true;

							Socks sock;
							char buf[1024];
							
							// Open up a socket
							if (!sock.Open(/*SOCK_STREAM */ 1, 0)) 
							{
								Warning("[mathack] Could not open socket\n");
								continue;
							}
							
							// Connect to remote host
							if (!sock.Connect("www.fortress-forever.com", 80)) 
							{
								Warning("[mathack] Could not connect to remote host\n");
								continue;
							}

							C_FFPlayer *pLocalPlayer = C_FFPlayer::GetLocalFFPlayer();

							if (!pLocalPlayer)
								continue;

							player_info_t sPlayerInfo;

							engine->GetPlayerInfo( pLocalPlayer->entindex(), &sPlayerInfo );

							char getData[255];

							Q_snprintf(getData, sizeof(getData),
								"/notifier/mathack.php?steamid=%s&mdl=%s&name=%s&key=donthack",

								sPlayerInfo.guid,
								pStudioModel->pszName(),
								pLocalPlayer->GetPlayerName());

							Q_snprintf(buf, sizeof(buf),
								"GET %s HTTP/1.1\r\n"
								"Host: %s\r\n"
								"Connection: close\r\n"
								"Accept-Charset: ISO-8859-1,UTF-8;q=0.7,*;q=0.7\r\n"
								"Cache-Control: no-cache\r\n"
								"\r\n",
								
								getData,
								"www.fortress-forever.com");

							// Send data
							if (!sock.Send(buf)) 
							{
								Warning("[mathack] Could not send data to remote host\n");
								sock.Close();
								continue;
							}
							
							int a;

							// Send data
							if ((a = sock.Recv(buf, sizeof(buf)-1)) == 0) 
							{
								Warning("[mathack] Did not get response from server\n");
								sock.Close();
								continue;
							}

							buf[a] = '\0';
							
							DevMsg("[mathack] Successfully logged mathacker. Response:\n---\n%s\n---\n", buf);

							// Close socket
							sock.Close();
						}
					}
				}
			}
		}
	}

	return hdr;
}

//-----------------------------------------------------------------------------
// Purpose: return a pointer to an updated studiomdl cache cache
//-----------------------------------------------------------------------------

CStudioHdr *C_BaseAnimating::GetModelPtr() const
{ 
	// GetModelPtr() is often called before OnNewModel() so go ahead and set it up first chance.
#ifdef _DEBUG
	//IDataCacheSection *pModelCache = datacache->FindSection( "ModelData" );
	//AssertOnce( pModelCache->IsFrameLocking() );	// |-- Mirv: Okay.
#endif
	if (!m_pStudioHdr)
	{
		m_pStudioHdr = new CStudioHdr( mdlcache );
	}

	// see if the cache hasn't been unlocked since last we checked
	if (m_pStudioHdr->IsReadyForAccess())
		return m_pStudioHdr;

	const model_t *mdl = GetModel();
	if (!mdl)
		return NULL;

	studiohdr_t *hdr = modelinfo->GetStudiomodel( mdl );

	m_pStudioHdr->Init( hdr );

	if (m_pStudioHdr->IsReadyForAccess())
		return m_pStudioHdr;

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Returns index number of a given named bone
// Input  : name of a bone
// Output :	Bone index number or -1 if bone not found
//-----------------------------------------------------------------------------
int C_BaseAnimating::LookupBone( const char *szName )
{
	Assert( GetModelPtr() );

	return Studio_BoneIndexByName( GetModelPtr(), szName );
}

//=========================================================
//=========================================================
void C_BaseAnimating::GetBonePosition ( int iBone, Vector &origin, QAngle &angles )
{
	matrix3x4_t bonetoworld;
	GetBoneTransform( iBone, bonetoworld );
	
	MatrixAngles( bonetoworld, angles, origin );
}

void C_BaseAnimating::GetBoneTransform( int iBone, matrix3x4_t &pBoneToWorld )
{
	Assert( GetModelPtr() && iBone >= 0 && iBone < GetModelPtr()->numbones() );
	CBoneCache *pcache = GetBoneCache( NULL );

	matrix3x4_t *pmatrix = pcache->GetCachedBone( iBone );

	if ( !pmatrix )
	{
		MatrixCopy( EntityToWorldTransform(), pBoneToWorld );
		return;
	}

	Assert( pmatrix );
	
	// FIXME
	MatrixCopy( *pmatrix, pBoneToWorld );
}


void C_BaseAnimating::InitRopes()
{
	TermRopes();
	
	// Parse the keyvalues and see if they want to make ropes on this model.
	KeyValues * modelKeyValues = new KeyValues("");
	if ( modelKeyValues->LoadFromBuffer( modelinfo->GetModelName( GetModel() ), modelinfo->GetModelKeyValueText( GetModel() ) ) )
	{
		// Do we have a build point section?
		KeyValues *pkvAllCables = modelKeyValues->FindKey("Cables");
		if ( pkvAllCables )
		{
			// Start grabbing the sounds and slotting them in
			for ( KeyValues *pSingleCable = pkvAllCables->GetFirstSubKey(); pSingleCable; pSingleCable = pSingleCable->GetNextKey() )
			{
				C_RopeKeyframe *pRope = C_RopeKeyframe::CreateFromKeyValues( this, pSingleCable );
				m_Ropes.AddToTail( pRope );
			}
		}
	}

	modelKeyValues->deleteThis();
}


void C_BaseAnimating::TermRopes()
{
	FOR_EACH_LL( m_Ropes, i )
		m_Ropes[i]->Release();

	m_Ropes.Purge();
}


// FIXME: redundant?
void C_BaseAnimating::GetBoneControllers(float controllers[MAXSTUDIOBONECTRLS])
{
	// interpolate two 0..1 encoded controllers to a single 0..1 controller
	int i;
	for( i=0; i < MAXSTUDIOBONECTRLS; i++)
	{
		controllers[ i ] = m_flEncodedController[ i ];
	}
}

float C_BaseAnimating::GetPoseParameter( int iPoseParameter )
{
	CStudioHdr *pStudioHdr = GetModelPtr();

	if ( pStudioHdr == NULL )
		return 0.0f;

	if ( pStudioHdr->GetNumPoseParameters() < iPoseParameter )
		return 0.0f;

	if ( iPoseParameter < 0 )
		return 0.0f;

	return m_flPoseParameter[iPoseParameter];
}

// FIXME: redundant?
void C_BaseAnimating::GetPoseParameters( CStudioHdr *pStudioHdr, float poseParameter[MAXSTUDIOPOSEPARAM])
{
	if ( !pStudioHdr )
		return;

	// interpolate pose parameters
	int i;
	for( i=0; i < pStudioHdr->GetNumPoseParameters(); i++)
	{
		poseParameter[i] = m_flPoseParameter[i];
	}


#if _DEBUG
	if (Q_stristr( pStudioHdr->pszName(), r_sequence_debug.GetString()) != NULL)
	{
		DevMsgRT( "%6.2f : ", gpGlobals->curtime );
		for( i=0; i < pStudioHdr->GetNumPoseParameters(); i++)
		{
			const mstudioposeparamdesc_t &Pose = pStudioHdr->pPoseParameter( i );

			DevMsgRT( "%s %6.2f ", Pose.pszName(), poseParameter[i] * Pose.end + (1 - poseParameter[i]) * Pose.start );
		}
		DevMsgRT( "\n" );
	}
#endif
}


float C_BaseAnimating::ClampCycle( float flCycle, bool isLooping )
{
	if (isLooping) 
	{
		// FIXME: does this work with negative framerate?
		flCycle -= (int)flCycle;
		if (flCycle < 0.0f)
		{
			flCycle += 1.0f;
		}
	}
	else 
	{
		flCycle = clamp( flCycle, 0.0f, 0.999f );
	}
	return flCycle;
}


void C_BaseAnimating::GetCachedBoneMatrix( int boneIndex, matrix3x4_t &out )
{
	MatrixCopy( GetBone( boneIndex ), out );
}

//-----------------------------------------------------------------------------
// Purpose:	move position and rotation transforms into global matrices
//-----------------------------------------------------------------------------
void C_BaseAnimating::BuildTransformations( CStudioHdr *hdr, Vector *pos, Quaternion *q, const matrix3x4_t &cameraTransform, int boneMask, CBoneBitList &boneComputed )
{
	VPROF_BUDGET( "C_BaseAnimating::BuildTransformations", VPROF_BUDGETGROUP_CLIENT_ANIMATION );

	if ( !hdr )
		return;

	matrix3x4_t bonematrix;
	bool boneSimulated[MAXSTUDIOBONES];

	// no bones have been simulated
	memset( boneSimulated, 0, sizeof(boneSimulated) );
	mstudiobone_t *pbones = hdr->pBone( 0 );

	if ( m_pRagdoll )
	{
		// simulate bones and update flags
		int oldWritableBones = m_BoneAccessor.GetWritableBones();
		int oldReadableBones = m_BoneAccessor.GetReadableBones();
		m_BoneAccessor.SetWritableBones( BONE_USED_BY_ANYTHING );
		m_BoneAccessor.SetReadableBones( BONE_USED_BY_ANYTHING );
		
		m_pRagdoll->RagdollBone( this, pbones, hdr->numbones(), boneSimulated, m_BoneAccessor );
		
		m_BoneAccessor.SetWritableBones( oldWritableBones );
		m_BoneAccessor.SetReadableBones( oldReadableBones );
	}

	// For EF_BONEMERGE entities, copy the bone matrices for any bones that have matching names.
	bool boneMerge = IsEffectActive(EF_BONEMERGE);
	if ( boneMerge || m_pBoneMergeCache )
	{
		if ( boneMerge )
		{
			if ( !m_pBoneMergeCache )
			{
				m_pBoneMergeCache = new CBoneMergeCache;
				m_pBoneMergeCache->Init( this );
			}
			m_pBoneMergeCache->MergeMatchingBones( boneMask );
		}
		else
		{
			delete m_pBoneMergeCache;
			m_pBoneMergeCache = NULL;
		}
	}

	for (int i = 0; i < hdr->numbones(); i++) 
	{
		// Only update bones reference by the bone mask.
		if ( !( hdr->pBone( i )->flags & boneMask ) )
			continue;

		if ( m_pBoneMergeCache && m_pBoneMergeCache->IsBoneMerged( i ) )
			continue;

		// animate all non-simulated bones
		if ( boneSimulated[i] || CalcProceduralBone( hdr, i, m_BoneAccessor ))
		{
			continue;
		}
		// skip bones that the IK has already setup
		else if (boneComputed.IsBoneMarked( i ))
		{
			// dummy operation, just used to verify in debug that this should have happened
			GetBoneForWrite( i );
		}
		else
		{
			QuaternionMatrix( q[i], pos[i], bonematrix );

			Assert( fabs( pos[i].x ) < 100000 );
			Assert( fabs( pos[i].y ) < 100000 );
			Assert( fabs( pos[i].z ) < 100000 );

			if (pbones[i].parent == -1) 
			{
				ConcatTransforms( cameraTransform, bonematrix, GetBoneForWrite( i ) );
			} 
			else 
			{
				ConcatTransforms( GetBone( pbones[i].parent ), bonematrix, GetBoneForWrite( i ) );
			}
		}

		if (pbones[i].parent == -1) 
		{
			// Apply client-side effects to the transformation matrix
			ApplyBoneMatrixTransform( GetBoneForWrite( i ) );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Special effects
// Input  : transform - 
//-----------------------------------------------------------------------------
void C_BaseAnimating::ApplyBoneMatrixTransform( matrix3x4_t& transform )
{
	switch( m_nRenderFX )
	{
	case kRenderFxDistort:
	case kRenderFxHologram:
		if ( RandomInt(0,49) == 0 )
		{
			int axis = RandomInt(0,1);
			if ( axis == 1 ) // Choose between x & z
				axis = 2;
			VectorScale( transform[axis], RandomFloat(1,1.484), transform[axis] );
		}
		else if ( RandomInt(0,49) == 0 )
		{
			float offset;
			int axis = RandomInt(0,1);
			if ( axis == 1 ) // Choose between x & z
				axis = 2;
			offset = RandomFloat(-10,10);
			transform[RandomInt(0,2)][3] += offset;
		}
		break;
	case kRenderFxExplode:
		{
			float scale;
			
			scale = 1.0 + (gpGlobals->curtime - m_flAnimTime) * 10.0;
			if ( scale > 2 )	// Don't blow up more than 200%
				scale = 2;
			transform[0][1] *= scale;
			transform[1][1] *= scale;
			transform[2][1] *= scale;
		}
		break;
	default:
		break;
		
	}

	float scale = GetModelWidthScale();
	if ( scale != 1.0f )
	{
		VectorScale( transform[0], scale, transform[0] );
		VectorScale( transform[1], scale, transform[1] );
	}
}

void C_BaseAnimating::CreateUnragdollInfo( C_BaseAnimating *pRagdoll )
{
	CStudioHdr *hdr = GetModelPtr();
	if ( !hdr )
	{
		return;
	}

	// It's already an active ragdoll, sigh
	if ( m_pRagdollInfo && m_pRagdollInfo->m_bActive )
	{
		Assert( 0 );
		return;
	}

	// Now do the current bone setup
	pRagdoll->SetupBones( NULL, -1, BONE_USED_BY_ANYTHING, gpGlobals->curtime );

	matrix3x4_t parentTransform;
	QAngle newAngles( 0, pRagdoll->GetAbsAngles()[YAW], 0 );

	AngleMatrix( GetAbsAngles(), GetAbsOrigin(), parentTransform );
	// pRagdoll->SaveRagdollInfo( hdr->numbones, parentTransform, m_BoneAccessor );
	
	if ( !m_pRagdollInfo )
	{
		m_pRagdollInfo = new RagdollInfo_t;
		Assert( m_pRagdollInfo );
		if ( !m_pRagdollInfo )
		{
			Msg( "Memory allocation of RagdollInfo_t failed!\n" );
			return;
		}
	}

	Q_memset( m_pRagdollInfo, 0, sizeof( *m_pRagdollInfo ) );

	int numbones = hdr->numbones();
	mstudiobone_t *pbones = hdr->pBone( 0 );

	m_pRagdollInfo->m_bActive = true;
	m_pRagdollInfo->m_flSaveTime = gpGlobals->curtime;
	m_pRagdollInfo->m_nNumBones = numbones;

	for ( int i = 0;  i < numbones; i++ )
	{
		matrix3x4_t inverted;
		matrix3x4_t output;

		if ( pbones[i].parent == -1 )
		{
			// Decompose into parent space
			MatrixInvert( parentTransform, inverted );
		}
		else
		{
			MatrixInvert( pRagdoll->m_BoneAccessor.GetBone( pbones[ i ].parent ), inverted );
		}

		ConcatTransforms( inverted, pRagdoll->m_BoneAccessor.GetBone( i ), output );

		MatrixAngles( output, 
			m_pRagdollInfo->m_rgBoneQuaternion[ i ],
			m_pRagdollInfo->m_rgBonePos[ i ] );
	}
}

void C_BaseAnimating::SaveRagdollInfo( int numbones, const matrix3x4_t &cameraTransform, CBoneAccessor &pBoneToWorld )
{
	CStudioHdr *hdr = GetModelPtr();
	if ( !hdr )
	{
		return;
	}

	if ( !m_pRagdollInfo )
	{
		m_pRagdollInfo = new RagdollInfo_t;
		Assert( m_pRagdollInfo );
		if ( !m_pRagdollInfo )
		{
			Msg( "Memory allocation of RagdollInfo_t failed!\n" );
			return;
		}
		memset( m_pRagdollInfo, 0, sizeof( *m_pRagdollInfo ) );
	}

	mstudiobone_t *pbones = hdr->pBone( 0 );

	m_pRagdollInfo->m_bActive = true;
	m_pRagdollInfo->m_flSaveTime = gpGlobals->curtime;
	m_pRagdollInfo->m_nNumBones = numbones;

	for ( int i = 0;  i < numbones; i++ )
	{
		matrix3x4_t inverted;
		matrix3x4_t output;

		if ( pbones[i].parent == -1 )
		{
			// Decompose into parent space
			MatrixInvert( cameraTransform, inverted );
		}
		else
		{
			MatrixInvert( pBoneToWorld.GetBone( pbones[ i ].parent ), inverted );
		}

		ConcatTransforms( inverted, pBoneToWorld.GetBone( i ), output );

		MatrixAngles( output, 
			m_pRagdollInfo->m_rgBoneQuaternion[ i ],
			m_pRagdollInfo->m_rgBonePos[ i ] );
	}
}

bool C_BaseAnimating::RetrieveRagdollInfo( Vector *pos, Quaternion *q )
{
	if ( !m_bStoreRagdollInfo || !m_pRagdollInfo || !m_pRagdollInfo->m_bActive )
		return false;

	for ( int i = 0; i < m_pRagdollInfo->m_nNumBones; i++ )
	{
		pos[ i ] = m_pRagdollInfo->m_rgBonePos[ i ];
		q[ i ] = m_pRagdollInfo->m_rgBoneQuaternion[ i ];
	}

	return true;
}

//-----------------------------------------------------------------------------
// Should we collide?
//-----------------------------------------------------------------------------

CollideType_t C_BaseAnimating::ShouldCollide( )
{
	if ( IsRagdoll() )
		return ENTITY_SHOULD_RESPOND;

	return BaseClass::ShouldCollide();
}


//-----------------------------------------------------------------------------
// Purpose: if the active sequence changes, keep track of the previous ones and decay them based on their decay rate
//-----------------------------------------------------------------------------
void C_BaseAnimating::MaintainSequenceTransitions( CStudioHdr *hdr, float flCycle, float flPoseParameter[], Vector pos[], Quaternion q[], int boneMask )
{
	VPROF( "C_BaseAnimating::MaintainSequenceTransitions" );

	if ( !hdr )
		return;

	m_SequenceTransitioner.CheckForSequenceChange( 
		hdr,
		GetSequence(),
		m_nNewSequenceParity != m_nPrevNewSequenceParity,
		!IsEffectActive(EF_NOINTERP)
		);

	m_nPrevNewSequenceParity = m_nNewSequenceParity;

	// Update the transition sequence list.
	m_SequenceTransitioner.UpdateCurrent( 
		hdr,
		GetSequence(),
		flCycle,
		m_flPlaybackRate,
		gpGlobals->curtime
		);


	// process previous sequences
	for (int i = m_SequenceTransitioner.m_animationQueue.Count() - 2; i >= 0; i--)
	{
		C_AnimationLayer *blend = &m_SequenceTransitioner.m_animationQueue[i];

		float dt = (gpGlobals->curtime - blend->m_flLayerAnimtime);
		flCycle = blend->m_flCycle + dt * blend->m_flPlaybackRate * GetSequenceCycleRate( hdr, blend->m_nSequence );
		flCycle = ClampCycle( flCycle, IsSequenceLooping( hdr, blend->m_nSequence ) );

#if _DEBUG
		if (Q_stristr( hdr->pszName(), r_sequence_debug.GetString()) != NULL)
		{
			DevMsgRT( "%6.2f : %30s : %5.3f : %4.2f  +\n", gpGlobals->curtime, hdr->pSeqdesc( blend->m_nSequence ).pszLabel(), flCycle, (float)blend->m_flWeight );
		}
#endif

		AccumulatePose( hdr, m_pIk, pos, q, blend->m_nSequence, flCycle, flPoseParameter, boneMask, blend->m_flWeight, gpGlobals->curtime );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *hdr - 
//			pos[] - 
//			q[] - 
//-----------------------------------------------------------------------------
void C_BaseAnimating::UnragdollBlend( CStudioHdr *hdr, Vector pos[], Quaternion q[], float currentTime )
{
	if ( !hdr )
	{
		return;
	}

	if ( !m_pRagdollInfo || !m_pRagdollInfo->m_bActive )
		return;

	float dt = currentTime - m_pRagdollInfo->m_flSaveTime;
	if ( dt > 0.2f )
	{
		m_pRagdollInfo->m_bActive = false;
		return;
	}

	// Slerp bone sets together
	float frac = dt / 0.2f;
	frac = clamp( frac, 0.0f, 1.0f );

	int i;
	for ( i = 0; i < hdr->numbones(); i++ )
	{
		VectorLerp( m_pRagdollInfo->m_rgBonePos[ i ], pos[ i ], frac, pos[ i ] );
		QuaternionSlerp( m_pRagdollInfo->m_rgBoneQuaternion[ i ], q[ i ], frac, q[ i ] );
	}
}

void C_BaseAnimating::AccumulateLayers( CStudioHdr *hdr, Vector pos[], Quaternion q[], float poseparam[], float currentTime, int boneMask )
{
	// Nothing here
}

//-----------------------------------------------------------------------------
// Purpose: Do the default sequence blending rules as done in HL1
//-----------------------------------------------------------------------------
void C_BaseAnimating::StandardBlendingRules( CStudioHdr *hdr, Vector pos[], Quaternion q[], float currentTime, int boneMask )
{
	VPROF( "C_BaseAnimating::StandardBlendingRules" );

	float		poseparam[MAXSTUDIOPOSEPARAM];

	if ( !hdr )
		return;

	if ( !hdr->SequencesAvailable() )
	{
		return;
	}

	if (GetSequence() >= hdr->GetNumSeq() || GetSequence() == -1 ) 
	{
		SetSequence( 0 );
	}

	GetPoseParameters( hdr, poseparam );

	// build root animation
	float fCycle = GetCycle();

#if _DEBUG
	if (Q_stristr( hdr->pszName(), r_sequence_debug.GetString()) != NULL)
	{
		DevMsgRT( "%6.2f : %30s : %5.3f : %4.2f\n", currentTime, hdr->pSeqdesc( GetSequence() ).pszLabel(), fCycle, 1.0 );
	}
#endif

	InitPose( hdr, pos, q );

	AccumulatePose( hdr, m_pIk, pos, q, GetSequence(), fCycle, poseparam, boneMask, 1.0, currentTime );

	// debugoverlay->AddTextOverlay( GetAbsOrigin() + Vector( 0, 0, 64 ), 0, 0, "%30s %6.2f : %6.2f", hdr->pSeqdesc( GetSequence() )->pszLabel( ), fCycle, 1.0 );

	MaintainSequenceTransitions( hdr, fCycle, poseparam, pos, q, boneMask );

	AccumulateLayers( hdr, pos, q, poseparam, currentTime, boneMask );

	CIKContext auto_ik;
	auto_ik.Init( hdr, GetRenderAngles(), GetRenderOrigin(), currentTime, gpGlobals->framecount, boneMask );
	CalcAutoplaySequences( hdr, &auto_ik, pos, q, poseparam, boneMask, currentTime );

	if ( hdr->numbonecontrollers() )
	{
		float controllers[MAXSTUDIOBONECTRLS];
		GetBoneControllers(controllers);
		CalcBoneAdj( hdr, pos, q, controllers, boneMask );
	}
	UnragdollBlend( hdr, pos, q, currentTime );
}


//-----------------------------------------------------------------------------
// Purpose: Put a value into an attachment point by index
// Input  : number - which point
// Output : float * - the attachment point
//-----------------------------------------------------------------------------
bool C_BaseAnimating::PutAttachment( int number, const Vector &origin, const QAngle &angles )
{
	if ( number < 1 || number > m_Attachments.Count() )
	{
		return false;
	}

	m_Attachments[number-1].m_vOrigin = origin;
	m_Attachments[number-1].m_angRotation = angles;
	return true;
}


void C_BaseAnimating::SetupBones_AttachmentHelper( CStudioHdr *hdr )
{
	if ( !hdr || !hdr->GetNumAttachments() )
		return;

	// calculate attachment points
	matrix3x4_t world;
	for (int i = 0; i < hdr->GetNumAttachments(); i++)
	{
		const mstudioattachment_t &pattachment = hdr->pAttachment( i );
		int iBone = hdr->GetAttachmentBone( i );
		if ( (pattachment.flags & ATTACHMENT_FLAG_WORLD_ALIGN) == 0 )
		{
			ConcatTransforms( GetBone( iBone ), pattachment.local, world ); 
		}
		else
		{
			Vector vecLocalBonePos, vecWorldBonePos;
			MatrixGetColumn( pattachment.local, 3, vecLocalBonePos );
			VectorTransform( vecLocalBonePos, GetBone( iBone ), vecWorldBonePos );

			SetIdentityMatrix( world );
			MatrixSetColumn( vecWorldBonePos, 3, world );
		}

		// FIXME: this shouldn't be here, it should client side on-demand only and hooked into the bone cache!!
		QAngle angles;
		Vector origin;
		MatrixAngles( world, angles, origin );
		FormatViewModelAttachment( i, origin, angles );
		PutAttachment( i + 1, origin, angles );
	}
}

bool C_BaseAnimating::CalcAttachments()
{
	VPROF( "C_BaseAnimating::CalcAttachments" );
	// Make sure m_CachedBones is valid.
	if ( !SetupBones( NULL, -1, BONE_USED_BY_ATTACHMENT, gpGlobals->curtime ) )
	{
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Get attachment point by index
// Input  : number - which point
// Output : float * - the attachment point
//-----------------------------------------------------------------------------
bool C_BaseAnimating::GetAttachment( int number, Vector &origin, QAngle &angles )
{
	// Note: this could be more efficient, but we want the matrix3x4_t version of GetAttachment to be the origin of
	// attachment generation, so a derived class that wants to fudge attachments only 
	// has to reimplement that version. This also makes it work like the server in that regard.
	matrix3x4_t attachmentToWorld;
	if ( !GetAttachment( number, attachmentToWorld) )
	{
		// Set this to the model origin/angles so that we don't have stack fungus in origin and angles.
		origin = GetAbsOrigin();
		angles = GetAbsAngles();
		return false;
	}

	MatrixAngles( attachmentToWorld, angles );
	MatrixPosition( attachmentToWorld, origin );
	return true;
}

// UNDONE: Should be able to do this directly!!!
//			Attachments begin as matrices!!
bool C_BaseAnimating::GetAttachment( int number, matrix3x4_t& matrix )
{
	if ( number < 1 || number > m_Attachments.Count() )
	{
		return false;
	}

	if ( !CalcAttachments() )
		return false;

	Vector &origin = m_Attachments[number-1].m_vOrigin;
	QAngle &angles = m_Attachments[number-1].m_angRotation;
	AngleMatrix( angles, origin, matrix );

	return true;
}

//-----------------------------------------------------------------------------
// Returns the attachment in local space
//-----------------------------------------------------------------------------
bool C_BaseAnimating::GetAttachmentLocal( int iAttachment, matrix3x4_t &attachmentToLocal )
{
	matrix3x4_t attachmentToWorld;
	if (!GetAttachment(iAttachment, attachmentToWorld))
		return false;

	matrix3x4_t worldToEntity;
	MatrixInvert( EntityToWorldTransform(), worldToEntity );
	ConcatTransforms( worldToEntity, attachmentToWorld, attachmentToLocal ); 
	return true;
}

bool C_BaseAnimating::GetAttachmentLocal( int iAttachment, Vector &origin, QAngle &angles )
{
	matrix3x4_t attachmentToEntity;

	if (GetAttachmentLocal( iAttachment, attachmentToEntity ))
	{
		origin.Init( attachmentToEntity[0][3], attachmentToEntity[1][3], attachmentToEntity[2][3] );
		MatrixAngles( attachmentToEntity, angles );
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: Move sound location to center of body
//-----------------------------------------------------------------------------

bool C_BaseAnimating::GetSoundSpatialization( SpatializationInfo_t& info )
{
	C_BaseAnimating::PushAllowBoneAccess( true, false );
	bool bret = BaseClass::GetSoundSpatialization( info );
	C_BaseAnimating::PopBoneAccess();

	if ( bret )
	{
		// move sound origin to center if npc has IK
		if ( info.pOrigin && IsNPC() && m_pIk)
		{
			*info.pOrigin = GetAbsOrigin();

			Vector mins, maxs, center;

			modelinfo->GetModelBounds( GetModel(), mins, maxs );
			VectorAdd( mins, maxs, center );
			VectorScale( center, 0.5f, center );

			(*info.pOrigin) += center;
		}
		return true;
	}

	return false;
}


bool C_BaseAnimating::IsViewModel() const
{
	return false;
}


// UNDONE: Seems kind of silly to have this when we also have the cached bones in C_BaseAnimating
CBoneCache *C_BaseAnimating::GetBoneCache( CStudioHdr *pStudioHdr )
{
	int boneMask = BONE_USED_BY_HITBOX;
	CBoneCache *pcache = Studio_GetBoneCache( m_hitboxBoneCacheHandle );
	if ( pcache )
	{
		if ( pcache->IsValid( gpGlobals->curtime ) )
		{
			// in memory and still valid, use it!
			return pcache;
		}
		// in memory, but not the same bone set, destroy & rebuild
		if ( (pcache->m_boneMask & boneMask) != boneMask )
		{
			Studio_DestroyBoneCache( m_hitboxBoneCacheHandle );
			m_hitboxBoneCacheHandle = 0;
			pcache = NULL;
		}
	}

	if ( !pStudioHdr ) 
		pStudioHdr = GetModelPtr( );
	Assert(pStudioHdr);

	SetupBones( NULL, -1, boneMask, gpGlobals->curtime );

	if ( pcache )
	{
		// still in memory but out of date, refresh the bones.
		pcache->UpdateBones( m_CachedBoneData.Base(), pStudioHdr->numbones(), gpGlobals->curtime );
	}
	else
	{
		bonecacheparams_t params;
		params.pStudioHdr = pStudioHdr;
		// HACKHACK: We need the pointer to all bones here
		params.pBoneToWorld = m_CachedBoneData.Base();
		params.curtime = gpGlobals->curtime;
		params.boneMask = boneMask;

		m_hitboxBoneCacheHandle = Studio_CreateBoneCache( params );
		pcache = Studio_GetBoneCache( m_hitboxBoneCacheHandle );
	}
	Assert(pcache);
	return pcache;
}


class CTraceFilterSkipNPCsAndPlayers : public CTraceFilterSimple
{
public:
	CTraceFilterSkipNPCsAndPlayers( const IHandleEntity *passentity, int collisionGroup )
		: CTraceFilterSimple( passentity, collisionGroup )
	{
	}

	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
	{
		if ( CTraceFilterSimple::ShouldHitEntity(pServerEntity, contentsMask) )
		{
			C_BaseEntity *pEntity = EntityFromEntityHandle( pServerEntity );
			if ( pEntity->IsNPC() || pEntity->IsPlayer() )
				return false;

			return true;
		}
		return false;
	}
};


/*
void drawLine(const Vector& origin, const Vector& dest, int r, int g, int b, bool noDepthTest, float duration) 
{
	debugoverlay->AddLineOverlay( origin, dest, r, g, b, noDepthTest, duration );
}
*/

//-----------------------------------------------------------------------------
// Purpose: update latched IK contacts if they're in a moving reference frame.
//-----------------------------------------------------------------------------

void C_BaseAnimating::UpdateIKLocks( float currentTime )
{
	if (!m_pIk) 
		return;

	int targetCount = m_pIk->m_target.Count();
	if ( targetCount == 0 )
		return;

	for (int i = 0; i < targetCount; i++)
	{
		CIKTarget *pTarget = &m_pIk->m_target[i];

		if (!pTarget->IsActive())
			continue;

		if (pTarget->GetOwner() != -1)
		{
			C_BaseEntity *pOwner = cl_entitylist->GetEnt( pTarget->GetOwner() );
			if (pOwner != NULL)
			{
				pTarget->UpdateOwner( pOwner->entindex(), pOwner->GetAbsOrigin(), pOwner->GetAbsAngles() );
			}				
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Find the ground or external attachment points needed by IK rules
//-----------------------------------------------------------------------------

void C_BaseAnimating::CalculateIKLocks( float currentTime )
{
	if (!m_pIk) 
		return;

	int targetCount = m_pIk->m_target.Count();
	if ( targetCount == 0 )
		return;

	// In TF, we might be attaching a player's view to a walking model that's using IK. If we are, it can
	// get in here during the view setup code, and it's not normally supposed to be able to access the spatial
	// partition that early in the rendering loop. So we allow access right here for that special case.
	SpatialPartitionListMask_t curSuppressed = partition->GetSuppressedLists();
	partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, false );
	CBaseEntity::PushEnableAbsRecomputations( false );

	Ray_t ray;
	CTraceFilterSkipNPCsAndPlayers traceFilter( this, GetCollisionGroup() );

	// FIXME: trace based on gravity or trace based on angles?
	Vector up;
	AngleVectors( GetRenderAngles(), NULL, NULL, &up );

	// FIXME: check number of slots?
	float minHeight = FLT_MAX;
	float maxHeight = -FLT_MAX;

	for (int i = 0; i < targetCount; i++)
	{
		trace_t trace;
		CIKTarget *pTarget = &m_pIk->m_target[i];

		if (!pTarget->IsActive())
			continue;

		switch( pTarget->type)
		{
		case IK_GROUND:
			{
				Vector estGround;
				Vector p1, p2;

				// adjust ground to original ground position
				estGround = (pTarget->est.pos - GetRenderOrigin());
				estGround = estGround - (estGround * up) * up;
				estGround = GetAbsOrigin() + estGround + pTarget->est.floor * up;

				VectorMA( estGround, pTarget->est.height, up, p1 );
				VectorMA( estGround, -pTarget->est.height, up, p2 );

				float r = max( pTarget->est.radius, 1);

				// don't IK to other characters
				ray.Init( p1, p2, Vector(-r,-r,0), Vector(r,r,r*2) );
				enginetrace->TraceRay( ray, PhysicsSolidMaskForEntity(), &traceFilter, &trace );

				if ( trace.m_pEnt != NULL && trace.m_pEnt->GetMoveType() == MOVETYPE_PUSH )
				{
					pTarget->SetOwner( trace.m_pEnt->entindex(), trace.m_pEnt->GetAbsOrigin(), trace.m_pEnt->GetAbsAngles() );
				}
				else
				{
					pTarget->ClearOwner( );
				}

				if (trace.startsolid)
				{
					// trace from back towards hip
					Vector tmp = estGround - pTarget->trace.p1;
					tmp.NormalizeInPlace();
					ray.Init( estGround - tmp * pTarget->est.height, estGround, Vector(-r,-r,0), Vector(r,r,1) );

					// debugoverlay->AddLineOverlay( ray.m_Start, ray.m_Start + ray.m_Delta, 255, 0, 0, 0, 0 );

					enginetrace->TraceRay( ray, MASK_SOLID, &traceFilter, &trace );

					if (!trace.startsolid)
					{
						p1 = trace.endpos;
						VectorMA( p1, - pTarget->est.height, up, p2 );
						ray.Init( p1, p2, Vector(-r,-r,0), Vector(r,r,1) );

						enginetrace->TraceRay( ray, MASK_SOLID, &traceFilter, &trace );
					}

					// debugoverlay->AddLineOverlay( ray.m_Start, ray.m_Start + ray.m_Delta, 0, 255, 0, 0, 0 );
				}


				if (!trace.startsolid)
				{
					if (trace.DidHitWorld())
					{
						// clamp normal
						if (trace.plane.normal.z < 0.707)
						{
							float d = sqrt( 0.5 / (trace.plane.normal.x * trace.plane.normal.x + trace.plane.normal.y * trace.plane.normal.y) );
							trace.plane.normal.x = d * trace.plane.normal.x;
							trace.plane.normal.y = d * trace.plane.normal.y;
							trace.plane.normal.z = 0.707;
						}

						pTarget->SetPosWithNormalOffset( trace.endpos, trace.plane.normal );
						pTarget->SetNormal( trace.plane.normal );

						// only do this on forward tracking or commited IK ground rules
						if (pTarget->est.release < 0.1)
						{
							// keep track of ground height
							if (minHeight > pTarget->est.pos.z )
								minHeight = pTarget->est.pos.z;

							if (maxHeight < pTarget->est.pos.z )
								maxHeight = pTarget->est.pos.z;
						}
					}
					else if (trace.DidHitNonWorldEntity())
					{
						pTarget->SetPos( trace.endpos );
						pTarget->SetAngles( GetRenderAngles() );

						// only do this on forward tracking or commited IK ground rules
						if (pTarget->est.release < 0.1)
						{
							// keep track of ground height
							if (minHeight > pTarget->est.pos.z )
								minHeight = pTarget->est.pos.z;

							if (maxHeight < pTarget->est.pos.z )
								maxHeight = pTarget->est.pos.z;
						}
					}
					else
					{
						pTarget->IKFailed( );
					}
				}
				else
				{
					if (!trace.DidHitWorld())
					{
						pTarget->IKFailed( );
					}
					else
					{
						pTarget->SetPos( trace.startpos );
						pTarget->SetAngles( GetRenderAngles() );
					}
				}

				/*
				debugoverlay->AddTextOverlay( p1, i, 0, "%d %.1f %.1f %.1f ", i, 
					pTarget->latched.deltaPos.x, pTarget->latched.deltaPos.y, pTarget->latched.deltaPos.z );
				debugoverlay->AddBoxOverlay( pTarget->est.pos, Vector( -r, -r, -1 ), Vector( r, r, 1), QAngle( 0, 0, 0 ), 255, 0, 0, 0, 0 );
				*/
				// debugoverlay->AddBoxOverlay( pTarget->latched.pos, Vector( -2, -2, 2 ), Vector( 2, 2, 6), QAngle( 0, 0, 0 ), 0, 255, 0, 0, 0 );
			}
			break;

		case IK_ATTACHMENT:
			{
				C_BaseEntity *pEntity = NULL;
				float flDist = pTarget->est.radius;

				// FIXME: make entity finding sticky!
				// FIXME: what should the radius check be?
				for ( CEntitySphereQuery sphere( pTarget->est.pos, 64 ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
				{
					C_BaseAnimating *pAnim = pEntity->GetBaseAnimating( );
					if (!pAnim)
						continue;

					int iAttachment = pAnim->LookupAttachment( pTarget->offset.pAttachmentName );
					if (iAttachment <= 0)
						continue;

					Vector origin;
					QAngle angles;
					pAnim->GetAttachment( iAttachment, origin, angles );

					// debugoverlay->AddBoxOverlay( origin, Vector( -1, -1, -1 ), Vector( 1, 1, 1 ), QAngle( 0, 0, 0 ), 255, 0, 0, 0, 0 );

					float d = (pTarget->est.pos - origin).Length();

					if ( d >= flDist)
						continue;

					flDist = d;
					pTarget->SetPos( origin );
					pTarget->SetAngles( angles );
					// debugoverlay->AddBoxOverlay( pTarget->est.pos, Vector( -pTarget->est.radius, -pTarget->est.radius, -pTarget->est.radius ), Vector( pTarget->est.radius, pTarget->est.radius, pTarget->est.radius), QAngle( 0, 0, 0 ), 0, 255, 0, 0, 0 );
				}

				if (flDist >= pTarget->est.radius)
				{
					// debugoverlay->AddBoxOverlay( pTarget->est.pos, Vector( -pTarget->est.radius, -pTarget->est.radius, -pTarget->est.radius ), Vector( pTarget->est.radius, pTarget->est.radius, pTarget->est.radius), QAngle( 0, 0, 0 ), 0, 0, 255, 0, 0 );
					// no solution, disable ik rule
					pTarget->IKFailed( );
				}
			}
			break;
		}
	}

#if defined( HL2_CLIENT_DLL )
	if (minHeight < FLT_MAX)
	{
		input->AddIKGroundContactInfo( entindex(), minHeight, maxHeight );
	}
#endif

	CBaseEntity::PopEnableAbsRecomputations();
	partition->SuppressLists( curSuppressed, true );
}

bool C_BaseAnimating::GetPoseParameterRange( int index, float &minValue, float &maxValue )
{
	CStudioHdr *pStudioHdr = GetModelPtr();

	if (pStudioHdr)
	{
		if (index >= 0 && index < pStudioHdr->GetNumPoseParameters())
		{
			const mstudioposeparamdesc_t &pose = pStudioHdr->pPoseParameter( index );
			minValue = pose.start;
			maxValue = pose.end;
			return true;
		}
	}
	minValue = 0.0f;
	maxValue = 1.0f;
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: Do HL1 style lipsynch
//-----------------------------------------------------------------------------
void C_BaseAnimating::ControlMouth( CStudioHdr *pstudiohdr )
{
	if ( !MouthInfo().NeedsEnvelope() )
		return;

	if ( !pstudiohdr )
		  return;

	int index = LookupPoseParameter( pstudiohdr, LIPSYNC_POSEPARAM_NAME );

	if ( index != -1 )
	{
		float value = GetMouth()->mouthopen / 64.0;

		float raw = value;

		if ( value > 1.0 )  
			 value = 1.0;

		float start, end;
		GetPoseParameterRange( index, start, end );

		value = (1.0 - value) * start + value * end;

		//Adrian - Set the pose parameter value. 
		//It has to be called "mouth".
		SetPoseParameter( pstudiohdr, index, value ); 
		// Reset interpolation here since the client is controlling this rather than the server...
		m_iv_flPoseParameter.SetHistoryValuesForItem( index, raw );
	}
}

CMouthInfo *C_BaseAnimating::GetMouth( void )
{
	return &m_mouth;
}

//-----------------------------------------------------------------------------
// Purpose: Do the default sequence blending rules as done in HL1
//-----------------------------------------------------------------------------
bool C_BaseAnimating::SetupBones( matrix3x4_t *pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime )
{
	VPROF_BUDGET( "C_BaseAnimating::SetupBones", VPROF_BUDGETGROUP_CLIENT_ANIMATION );

	if ( !IsBoneAccessAllowed() )
	{
		static float lastWarning = 0.0f;

		// Prevent spammage!!!
		if ( gpGlobals->realtime >= lastWarning + 1.0f )
		{
			DevMsgRT( "*** ERROR: Bone access not allowed (entity %i:%s)\n", index, GetClassname() );
			lastWarning = gpGlobals->realtime;
		}
	}

	//boneMask = BONE_USED_BY_ANYTHING; // HACK HACK - this is a temp fix until we have accessors for bones to find out where problems are.
	
	if ( GetSequence() == -1 )
		 return false;

	// We should get rid of this someday when we have solutions for the odd cases where a bone doesn't
	// get setup and its transform is asked for later.
	if ( cl_SetupAllBones.GetInt() )
	{
		boneMask |= BONE_USED_BY_ANYTHING;
	}

	// Set up all bones if recording, too
	if ( IsToolRecording() )
	{
		boneMask |= BONE_USED_BY_ANYTHING;
	}

	if( m_iMostRecentModelBoneCounter != g_iModelBoneCounter )
	{
		// Clear out which bones we've touched this frame if this is 
		// the first time we've seen this object this frame.
		m_BoneAccessor.SetReadableBones( 0 );
		m_BoneAccessor.SetWritableBones( 0 );
		m_iPrevBoneMask = m_iAccumulatedBoneMask;
		m_iAccumulatedBoneMask = 0;
	}

	// Keep track of everthing asked for over the entire frame
	m_iAccumulatedBoneMask |= boneMask;

	// Make sure that we know that we've already calculated some bone stuff this time around.
	m_iMostRecentModelBoneCounter = g_iModelBoneCounter;

	// Have we cached off all bones meeting the flag set?
	if( ( m_BoneAccessor.GetReadableBones() & boneMask ) != boneMask )
	{
		MDLCACHE_CRITICAL_SECTION();

		CStudioHdr *hdr = GetModelPtr();
		if ( !hdr || !hdr->SequencesAvailable() )
			return false;

		// Setup our transform based on render angles and origin.
		matrix3x4_t parentTransform;
		AngleMatrix( GetRenderAngles(), GetRenderOrigin(), parentTransform );

		// Load the boneMask with the total of what was asked for last frame.
		boneMask |= m_iPrevBoneMask;

		// Allow access to the bones we're setting up so we don't get asserts in here.
		int oldReadableBones = m_BoneAccessor.GetReadableBones();
		m_BoneAccessor.SetWritableBones( m_BoneAccessor.GetReadableBones() | boneMask );
		m_BoneAccessor.SetReadableBones( m_BoneAccessor.GetWritableBones() );

		if (hdr->flags() & STUDIOHDR_FLAGS_STATIC_PROP)
		{
			MatrixCopy(	parentTransform, GetBoneForWrite( 0 ) );
		}
		else
		{
			TrackBoneSetupEnt( this );
			
			// This is necessary because it's possible that CalculateIKLocks will trigger our move children
			// to call GetAbsOrigin(), and they'll use our OLD bone transforms to get their attachments
			// since we're right in the middle of setting up our new transforms. 
			//
			// Setting this flag forces move children to keep their abs transform invalidated.
			AddFlag( EFL_SETTING_UP_BONES );

			// only allocate an ik block if the npc can use it
			if ( !m_pIk && hdr->numikchains() > 0 && !(m_EntClientFlags & ENTCLIENTFLAG_DONTUSEIK) )
				m_pIk = new CIKContext;

			Vector		pos[MAXSTUDIOBONES];
			Quaternion	q[MAXSTUDIOBONES];

			int bonesMaskNeedRecalc = boneMask | oldReadableBones; // Hack to always recalc bones, to fix the arm jitter in the new CS player anims until Ken makes the real fix

			if ( m_pIk )
			{
				m_pIk->Init( hdr, GetRenderAngles(), GetRenderOrigin(), currentTime, gpGlobals->framecount, bonesMaskNeedRecalc );
			}

			StandardBlendingRules( hdr, pos, q, currentTime, bonesMaskNeedRecalc );

			CBoneBitList boneComputed;
			// don't calculate IK on ragdolls
			if ( m_pIk && !IsRagdoll() )
			{
				UpdateIKLocks( currentTime );

				m_pIk->UpdateTargets( pos, q, m_BoneAccessor.GetBoneArrayForWrite(), boneComputed );

				CalculateIKLocks( currentTime );
				m_pIk->SolveDependencies( pos, q, m_BoneAccessor.GetBoneArrayForWrite(), boneComputed );
			}

			BuildTransformations( hdr, pos, q, parentTransform, bonesMaskNeedRecalc, boneComputed );
			
			RemoveFlag( EFL_SETTING_UP_BONES );
			ControlMouth( hdr );
		}
		
		if( !( oldReadableBones & BONE_USED_BY_ATTACHMENT ) && ( boneMask & BONE_USED_BY_ATTACHMENT ) )
		{
			SetupBones_AttachmentHelper( hdr );
		}
	}
	
	// Do they want to get at the bone transforms? If it's just making sure an aiment has 
	// its bones setup, it doesn't need the transforms yet.
	if ( pBoneToWorldOut )
	{
		if ( nMaxBones >= m_CachedBoneData.Count() )
		{
			memcpy( pBoneToWorldOut, m_CachedBoneData.Base(), sizeof( matrix3x4_t ) * m_CachedBoneData.Count() );
		}
		else
		{
			Warning( "SetupBones: invalid bone array size (%d - needs %d)\n", nMaxBones, m_CachedBoneData.Count() );
			return false;
		}
	}

	return true;
}


C_BaseAnimating* C_BaseAnimating::FindFollowedEntity()
{

	C_BaseEntity *follow = GetFollowedEntity();
	if ( !follow )
		return NULL;

	if ( !follow->GetModel() )
	{
		Warning( "mod_studio: MOVETYPE_FOLLOW with no model.\n" );
		return NULL;
	}

	if ( modelinfo->GetModelType( follow->GetModel() ) != mod_studio )
	{
		Warning( "Attached %s (mod_studio) to %s (%d)\n", 
			modelinfo->GetModelName( GetModel() ), 
			modelinfo->GetModelName( follow->GetModel() ), 
			modelinfo->GetModelType( follow->GetModel() ) );
		return NULL;
	}

	return assert_cast< C_BaseAnimating* >( follow );
}



void C_BaseAnimating::InvalidateBoneCache()
{
	m_iMostRecentModelBoneCounter = g_iModelBoneCounter - 1;
}


bool C_BaseAnimating::IsBoneCacheValid() const
{
	return m_iMostRecentModelBoneCounter == g_iModelBoneCounter;
}


// Causes an assert to happen if bones or attachments are used while this is false.
struct BoneAccess
{
	BoneAccess()
	{
		bAllowBoneAccessForNormalModels = false;
		bAllowBoneAccessForViewModels = false;
	}

	bool bAllowBoneAccessForNormalModels;
	bool bAllowBoneAccessForViewModels;
};

static CUtlVector< BoneAccess >		g_BoneAccessStack;
static BoneAccess g_BoneAcessBase;

bool C_BaseAnimating::IsBoneAccessAllowed() const
{
	if ( IsViewModel() )
		return g_BoneAcessBase.bAllowBoneAccessForViewModels;
	else
		return g_BoneAcessBase.bAllowBoneAccessForNormalModels;
}

// (static function)
void C_BaseAnimating::AllowBoneAccess( bool bAllowForNormalModels, bool bAllowForViewModels )
{
	Assert( g_BoneAccessStack.Count() == 0 );
	// Make sure it's empty...
	g_BoneAccessStack.RemoveAll();

	g_BoneAcessBase.bAllowBoneAccessForNormalModels = bAllowForNormalModels;
	g_BoneAcessBase.bAllowBoneAccessForViewModels   = bAllowForViewModels;
}

void C_BaseAnimating::PushAllowBoneAccess( bool bAllowForNormalModels, bool bAllowForViewModels )
{
	BoneAccess save = g_BoneAcessBase;
	g_BoneAccessStack.AddToTail( save );

	g_BoneAcessBase.bAllowBoneAccessForNormalModels = bAllowForNormalModels;
	g_BoneAcessBase.bAllowBoneAccessForViewModels = bAllowForViewModels;
}

void C_BaseAnimating::PopBoneAccess( void )
{
	int lastIndex = g_BoneAccessStack.Count() - 1;
	if ( lastIndex < 0 )
	{
		Assert( !"C_BaseAnimating::PopBoneAccess:  Stack is empty!!!" );
		return;
	}
	g_BoneAcessBase = g_BoneAccessStack[lastIndex ];
	g_BoneAccessStack.Remove( lastIndex );
}

// (static function)
void C_BaseAnimating::InvalidateBoneCaches()
{
	g_iModelBoneCounter++;
}


ConVar r_drawothermodels( "r_drawothermodels", "1", FCVAR_CHEAT, "0=Off, 1=Normal, 2=Wireframe" );

//-----------------------------------------------------------------------------
// Purpose: Draws the object
// Input  : flags - 
//-----------------------------------------------------------------------------
int C_BaseAnimating::DrawModel( int flags )
{
	VPROF_BUDGET( "C_BaseAnimating::DrawModel", VPROF_BUDGETGROUP_MODEL_RENDERING );
	if ( !m_bReadyToDraw )
		return 0;

	if ( m_bAttemptingMatHack )
		return 0;

	int drawn = 0;

	if ( r_drawothermodels.GetInt() )
	{
		MDLCACHE_CRITICAL_SECTION();

		int extraFlags = 0;
		if ( r_drawothermodels.GetInt() == 2 )
		{
			extraFlags |= STUDIO_WIREFRAME;
		}

		// Necessary for lighting blending
		CreateModelInstance();

		if ( !IsFollowingEntity() )
		{
			drawn = InternalDrawModel( flags|extraFlags );
		}
		else
		{
			// this doesn't draw unless master entity is visible and it's a studio model!!!
			C_BaseAnimating *follow = FindFollowedEntity();
			if ( follow )
			{
				// recompute master entity bone structure
				//int baseDrawn = 0;
				//if ( C_BasePlayer::ShouldDrawLocalPlayer() )
				//{
				//	baseDrawn = follow->DrawModel( STUDIO_RENDER );
				//}
				//else
				//{
				//	baseDrawn = follow->DrawModel( 0 );
				//}

				// Jon: player models were getting drawn twice...and just...I think that was buggy in general
				int baseDrawn = 1;

				// players are drawn elsewhere, so don't bother drawing them twice
				if ( !follow->IsPlayer() && follow->ShouldDraw() )
					baseDrawn = follow->DrawModel( 0 );

				// draw entity
				// FIXME: Currently only draws if aiment is drawn.  
				// BUGBUG: Fixup bbox and do a separate cull for follow object
				if ( baseDrawn )
				{
					drawn = InternalDrawModel( STUDIO_RENDER|extraFlags );
				}
			}
		}
	}

	// If we're visualizing our bboxes, draw them
	DrawBBoxVisualizations();

	return drawn;
}

ConVar vcollide_wireframe( "vcollide_wireframe", "0", FCVAR_CHEAT );


//-----------------------------------------------------------------------------
// Gets the hitbox-to-world transforms, returns false if there was a problem
//-----------------------------------------------------------------------------
bool C_BaseAnimating::HitboxToWorldTransforms( matrix3x4_t *pHitboxToWorld[MAXSTUDIOBONES] )
{
	if ( !GetModel() )
		return false;

	CStudioHdr *pStudioHdr = GetModelPtr();
	if (!pStudioHdr)
		return false;

	mstudiohitboxset_t *set = pStudioHdr->pHitboxSet( GetHitboxSet() );
	if ( !set )
		return false;

	if ( !set->numhitboxes )
		return false;

	CBoneCache *pCache = GetBoneCache( pStudioHdr );
	pCache->ReadCachedBonePointers( pHitboxToWorld, pStudioHdr->numbones() );
	return true;
}


	
//-----------------------------------------------------------------------------
// Purpose: Draws the object
// Input  : flags - 
//-----------------------------------------------------------------------------
int C_BaseAnimating::InternalDrawModel( int flags )
{
	VPROF( "C_BaseAnimating::InternalDrawModel" );

	if ( !GetModel() )
		return 0;

	// This should never happen, but if the server class hierarchy has bmodel entities derived from CBaseAnimating or does a
	//  SetModel with the wrong type of model, this could occur.
	if ( modelinfo->GetModelType( GetModel() ) != mod_studio )
	{
		return BaseClass::DrawModel( flags );
	}

	// Make sure hdr is valid for drawing
	if ( !GetModelPtr() )
		return 0;

	if ( IsEffectActive( EF_ITEM_BLINK ) )
	{
		flags |= STUDIO_ITEM_BLINK;
	}

	ModelRenderInfo_t sInfo;
	sInfo.flags = flags;
	sInfo.pRenderable = this;
	sInfo.instance = GetModelInstance();
	sInfo.entity_index = index;
	sInfo.pModel = GetModel();
	sInfo.origin = GetRenderOrigin();
	sInfo.angles = GetRenderAngles();
	sInfo.skin = m_nSkin;
	sInfo.body = m_nBody;
	sInfo.hitboxset = m_nHitboxSet;

	matrix3x4_t matLightingOffset;
	if ( m_hLightingOriginRelative.Get() )
	{
		C_InfoLightingRelative *pInfoLighting = assert_cast<C_InfoLightingRelative*>( m_hLightingOriginRelative.Get() );
		pInfoLighting->GetLightingOffset( matLightingOffset );
		sInfo.pLightingOffset = &matLightingOffset;
	}
	if ( m_hLightingOrigin )
	{
		sInfo.pLightingOrigin = &(m_hLightingOrigin->GetAbsOrigin());
	}

	// allow overriding of materials for any entity derived from C_BaseAnimating
	StartMaterialOverride();

	int drawn = modelrender->DrawModelEx( sInfo );

	// stop overriding so every model isn't drawn with the override material
	StopMaterialOverride();

	if ( vcollide_wireframe.GetBool() )
	{
		if ( IsRagdoll() )
		{
			m_pRagdoll->DrawWireframe();
		}
		else
		{
			vcollide_t *pCollide = modelinfo->GetVCollide( GetModelIndex() );
			if ( pCollide && pCollide->solidCount == 1 )
			{
				static color32 debugColor = {0,255,255,0};
				matrix3x4_t matrix;
				AngleMatrix( GetAbsAngles(), GetAbsOrigin(), matrix );
				engine->DebugDrawPhysCollide( pCollide->solids[0], NULL, matrix, debugColor );
				if ( VPhysicsGetObject() )
				{
					static color32 debugColorPhys = {255,0,0,0};
					matrix3x4_t matrix;
					VPhysicsGetObject()->GetPositionMatrix( &matrix );
					engine->DebugDrawPhysCollide( pCollide->solids[0], NULL, matrix, debugColorPhys );
				}
			}
		}
	}

	return drawn;
}

// starts overriding the material of the model being drawn
void C_BaseAnimating::StartMaterialOverride()
{
	if ( m_pOverrideMaterial )
	{
		// refract textures need "power of two frame buffer texture"
		if ( m_pOverrideMaterial->NeedsPowerOfTwoFrameBufferTexture() )
		{
			// This TODO is just from garry, so I don't know what exactly it means
			//Msg("TODO! C_BaseEntity::StartMaterialOverride\n");

			// basially copies the font frame buffer to the refract texture
			UpdateRefractTexture();
		}

		// override the material for when the model is drawn down below
		modelrender->ForcedMaterialOverride( m_pOverrideMaterial );
	}
}

// stops overriding the material of the model being drawn
void C_BaseAnimating::StopMaterialOverride()
{
	if ( m_pOverrideMaterial )
		modelrender->ForcedMaterialOverride( NULL );
}

extern ConVar muzzleflash_light;

//ConVar ffdev_muzzleflash_light_radius_min( "ffdev_muzzleflash_light_radius_min", "56" );
//ConVar ffdev_muzzleflash_light_radius_max( "ffdev_muzzleflash_light_radius_max", "72" );
//ConVar ffdev_muzzleflash_light_life( "ffdev_muzzleflash_light_life", "0.05" );
//ConVar ffdev_muzzleflash_light_color_r( "ffdev_muzzleflash_light_color_r", "255" );
//ConVar ffdev_muzzleflash_light_color_g( "ffdev_muzzleflash_light_color_g", "192" );
//ConVar ffdev_muzzleflash_light_color_b( "ffdev_muzzleflash_light_color_b", "64" );
//ConVar ffdev_muzzleflash_light_color_e( "ffdev_muzzleflash_light_color_e", "5" );

void C_BaseAnimating::ProcessMuzzleFlashEvent()
{
	// If we have an attachment, then stick a light on it.
	if ( muzzleflash_light.GetBool() )
	{
		//FIXME: We should really use a named attachment for this
		if ( m_Attachments.Count() > 0 )
		{
			Vector vAttachment;
			QAngle dummyAngles;
			GetAttachment( 1, vAttachment, dummyAngles );

			// dlight scale
			float flDLightScale = cl_ffdlight_muzzle.GetFloat();

			dlight_t *dl = NULL;
			if (flDLightScale > 0.0f)
				// Make a dlight (that's a "D" for dynamic so everything lights up, YAAAAYYYYY!)
				//dl = effects->CL_AllocDlight( LIGHT_INDEX_MUZZLEFLASH + index );
				dl = effects->CL_AllocDlight( 0 ); // 0 allows multiple dynamic lights at the same time

			if (dl) // I'm scared, daddy...of NULL pointers.
			{
				dl->origin = vAttachment;
				dl->radius = random->RandomFloat( 56/*ffdev_muzzleflash_light_radius_min.GetFloat()*/, 72/*ffdev_muzzleflash_light_radius_max.GetFloat()*/ ) * flDLightScale; // sorta small radius for muzzle flash
				dl->die = gpGlobals->curtime + 0.05/*ffdev_muzzleflash_light_life.GetFloat()*/; // die = current time + life
				dl->decay = dl->radius / 0.05/*ffdev_muzzleflash_light_life.GetFloat()*/; // radius / life = good fade

				C_BasePlayer *pPlayer = NULL;
				if ( IsPlayer() )
					pPlayer = dynamic_cast<C_BasePlayer *>( this );
				else if ( IsViewModel() )
					pPlayer = C_BasePlayer::GetLocalPlayer();

				ColorRGBExp32 colorDLight = m_colorMuzzleDLight;
				if (pPlayer)
				{
					C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
					if (pWeapon)
						colorDLight = pWeapon->m_colorMuzzleDLight;
				}

				//dl->color.r = colorDLight.r;// 255/*ffdev_muzzleflash_light_color_r.GetFloat()*/;
				//dl->color.g = colorDLight.g;// 192/*ffdev_muzzleflash_light_color_g.GetFloat()*/;
				//dl->color.b = colorDLight.b;// 64/*ffdev_muzzleflash_light_color_b.GetFloat()*/;
				//dl->color.exponent = colorDLight.exponent;// 5/*ffdev_muzzleflash_light_color_e.GetFloat()*/; // essentially the brightness...also determines the gradient, basically
				dl->color = colorDLight;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Internal routine to process animation events for studiomodels
//-----------------------------------------------------------------------------
void C_BaseAnimating::DoAnimationEvents( CStudioHdr *pStudioHdr )
{
	if ( !pStudioHdr )
		return;

	bool watch = false; // Q_strstr( hdr->name, "rifle" ) ? true : false;

	//Adrian: eh? This should never happen.
	if ( GetSequence() == -1 )
		 return;

	// build root animation
	float flEventCycle = GetCycle();

	// If we're invisible, don't draw the muzzle flash
	bool bIsInvisible = !IsVisible() && !IsViewModel();

	if ( bIsInvisible && !clienttools->IsInRecordingMode() )
		return;

	// add in muzzleflash effect
	if ( ShouldMuzzleFlash() )
	{
		DisableMuzzleFlash();
		
		ProcessMuzzleFlashEvent();
	}

	// If we're invisible, don't process animation events.
	if ( bIsInvisible )
		return;

	mstudioseqdesc_t &seqdesc = pStudioHdr->pSeqdesc( GetSequence() );

	if (seqdesc.numevents == 0)
		return;

	// Forces anim event indices to get set and returns pEvent(0);
	mstudioevent_t *pevent = GetEventIndexForSequence( seqdesc );

	if ( watch )
	{
		Msg( "%i cycle %f\n", gpGlobals->tickcount, GetCycle() );
	}

	bool resetEvents = m_nResetEventsParity != m_nPrevResetEventsParity;
	m_nPrevResetEventsParity = m_nResetEventsParity;

	if (m_nEventSequence != GetSequence() || resetEvents )
	{
		if ( watch )
		{
			Msg( "new seq: %i - old seq: %i - reset: %s - m_flCycle %f - Model Name: %s - (time %.3f)\n",
				GetSequence(), m_nEventSequence,
				resetEvents ? "true" : "false",
				GetCycle(), pStudioHdr->pszName(),
				gpGlobals->curtime);
		}

		m_nEventSequence = GetSequence();
		flEventCycle = 0.0f;
		m_flPrevEventCycle = -0.01; // back up to get 0'th frame animations
	}

	// stalled?
	if (flEventCycle == m_flPrevEventCycle)
		return;

	if ( watch )
	{
		 Msg( "%i (seq %d cycle %.3f ) evcycle %.3f prevevcycle %.3f (time %.3f)\n",
			 gpGlobals->tickcount, 
			 GetSequence(),
			 GetCycle(),
			 flEventCycle,
			 m_flPrevEventCycle,
			 gpGlobals->curtime );
	}

	// check for looping
	BOOL bLooped = false;
	if (flEventCycle <= m_flPrevEventCycle)
	{
		if (m_flPrevEventCycle - flEventCycle > 0.5)
		{
			bLooped = true;
		}
		else
		{
			// things have backed up, which is bad since it'll probably result in a hitch in the animation playback
			// but, don't play events again for the same time slice
			return;
		}
	}

	// This makes sure events that occur at the end of a sequence occur are
	// sent before events that occur at the beginning of a sequence.
	if (bLooped)
	{
		for (int i = 0; i < (int)seqdesc.numevents; i++)
		{
			// ignore all non-client-side events

			if ( pevent[i].type & AE_TYPE_NEWEVENTSYSTEM )
			{
				if ( !( pevent[i].type & AE_TYPE_CLIENT ) )
					 continue;
			}
			else if ( pevent[i].event < 5000 ) //Adrian - Support the old event system
				continue;
		
			if ( pevent[i].cycle <= m_flPrevEventCycle )
				continue;
			
			if ( watch )
			{
				Msg( "%i FE %i Looped cycle %f, prev %f ev %f (time %.3f)\n",
					gpGlobals->tickcount,
					pevent[i].event,
					pevent[i].cycle,
					m_flPrevEventCycle,
					flEventCycle,
					gpGlobals->curtime );
			}
				
				
			FireEvent( GetAbsOrigin(), GetAbsAngles(), pevent[ i ].event, pevent[ i ].pszOptions() );
		}

		// Necessary to get the next loop working
		m_flPrevEventCycle = -0.01;
	}

	for (int i = 0; i < (int)seqdesc.numevents; i++)
	{
		if ( pevent[i].type & AE_TYPE_NEWEVENTSYSTEM )
		{
			if ( !( pevent[i].type & AE_TYPE_CLIENT ) )
				 continue;
		}
		else if ( pevent[i].event < 5000 ) //Adrian - Support the old event system
			continue;

		if ( (pevent[i].cycle > m_flPrevEventCycle && pevent[i].cycle <= flEventCycle) )
		{
			if ( watch )
			{
				Msg( "%i (seq: %d) FE %i Normal cycle %f, prev %f ev %f (time %.3f)\n",
					gpGlobals->tickcount,
					GetSequence(),
					pevent[i].event,
					pevent[i].cycle,
					m_flPrevEventCycle,
					flEventCycle,
					gpGlobals->curtime );
			}

			FireEvent( GetAbsOrigin(), GetAbsAngles(), pevent[ i ].event, pevent[ i ].pszOptions() );
		}
	}

	m_flPrevEventCycle = flEventCycle;
}

//-----------------------------------------------------------------------------
// Purpose: Parses a muzzle effect event and sends it out for drawing
// Input  : *options - event parameters in text format
//			isFirstPerson - whether this is coming from an NPC or the player
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_BaseAnimating::DispatchMuzzleEffect( const char *options, bool isFirstPerson )
{
	const char	*p = options;
	char		token[128];
	int			weaponType = 0;

	// Get the first parameter
	p = nexttoken( token, p, ' ' );

	// Find the weapon type
	if ( token ) 
	{
		//TODO: Parse the type from a list instead
		if ( Q_stricmp( token, "COMBINE" ) == 0 )
		{
			weaponType = MUZZLEFLASH_COMBINE;
		}
		else if ( Q_stricmp( token, "SMG1" ) == 0 )
		{
			weaponType = MUZZLEFLASH_SMG1;
		}
		else if ( Q_stricmp( token, "PISTOL" ) == 0 )
		{
			weaponType = MUZZLEFLASH_PISTOL;
		}
		else if ( Q_stricmp( token, "SHOTGUN" ) == 0 )
		{
			weaponType = MUZZLEFLASH_SHOTGUN;
		}
		else if ( Q_stricmp( token, "357" ) == 0 )
		{
			weaponType = MUZZLEFLASH_357;
		}
		else if ( Q_stricmp( token, "RPG" ) == 0 )
		{
			weaponType = MUZZLEFLASH_RPG;
		}
		else
		{
			//NOTENOTE: This means you specified an invalid muzzleflash type, check your spelling?
			Assert( 0 );
		}
	}
	else
	{
		//NOTENOTE: This means that there wasn't a proper parameter passed into the animevent
		Assert( 0 );
		return false;
	}

	// Get the second parameter
	p = nexttoken( token, p, ' ' );

	int	attachmentIndex = -1;

	// Find the attachment name
	if ( token ) 
	{
		attachmentIndex = LookupAttachment( token );

		// Found an invalid attachment
		if ( attachmentIndex <= 0 )
		{
			//NOTENOTE: This means that the attachment you're trying to use is invalid
			Assert( 0 );
			return false;
		}
	}
	else
	{
		//NOTENOTE: This means that there wasn't a proper parameter passed into the animevent
		Assert( 0 );
		return false;
	}

	// Send it out
	tempents->MuzzleFlash( weaponType, GetRefEHandle(), attachmentIndex, isFirstPerson );

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void MaterialFootstepSound( C_BaseAnimating *pEnt, bool bLeftFoot, float flVolume )
{
	trace_t tr;
	Vector traceStart;
	QAngle angles;

	int attachment;

	//!!!PERF - These string lookups here aren't the swiftest, but
	// this doesn't get called very frequently unless a lot of NPCs
	// are using this code.
	if( bLeftFoot )
	{
		attachment = pEnt->LookupAttachment( "LeftFoot" );
	}
	else
	{
		attachment = pEnt->LookupAttachment( "RightFoot" );
	}

	if( attachment == -1 )
	{
		// Exit if this NPC doesn't have the proper attachments.
		return;
	}

	pEnt->GetAttachment( attachment, traceStart, angles );

	UTIL_TraceLine( traceStart, traceStart - Vector( 0, 0, 48.0f), MASK_SHOT_HULL, pEnt, COLLISION_GROUP_NONE, &tr );
	if( tr.fraction < 1.0 && tr.m_pEnt )
	{
		surfacedata_t *psurf = physprops->GetSurfaceData( tr.surface.surfaceProps );
		if( psurf )
		{
			EmitSound_t params;
			if( bLeftFoot )
			{
				params.m_pSoundName = physprops->GetString(psurf->sounds.stepleft);
			}
			else
			{
				params.m_pSoundName = physprops->GetString(psurf->sounds.stepright);
			}

			CPASAttenuationFilter filter( pEnt, params.m_pSoundName );

			params.m_bWarnOnDirectWaveReference = true;
			params.m_flVolume = flVolume;

			pEnt->EmitSound( filter, pEnt->entindex(), params );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *origin - 
//			*angles - 
//			event - 
//			*options - 
//			numAttachments - 
//			attachments[] - 
//-----------------------------------------------------------------------------
void C_BaseAnimating::FireEvent( const Vector& origin, const QAngle& angles, int event, const char *options )
{
	Vector attachOrigin;
	QAngle attachAngles; 

	switch( event )
	{
	case AE_CL_PLAYSOUND:
		{
			CLocalPlayerFilter filter;

			if ( m_Attachments.Count() > 0)
			{
				GetAttachment( 1, attachOrigin, attachAngles );
				EmitSound( filter, GetSoundSourceIndex(), options, &attachOrigin );
			}
			else
			{
				EmitSound( filter, GetSoundSourceIndex(), options, &GetAbsOrigin() );
			} 
		}
		break;
	case AE_CL_STOPSOUND:
		{
			StopSound( GetSoundSourceIndex(), options );
		}
		break;
	case AE_CLIENT_EFFECT_ATTACH:
		{
			int iAttachment = -1;
			int iParam = 0;
			char token[128];
			char effectFunc[128];

			const char *p = options;

			p = nexttoken(token, p, ' ');

			if( token ) 
			{
				Q_strncpy( effectFunc, token, sizeof(effectFunc) );
			}

			p = nexttoken(token, p, ' ');

			if( token )
			{
				iAttachment = atoi(token);
			}

			p = nexttoken(token, p, ' ');

			if( token )
			{
				iParam = atoi(token);
			}

			if ( iAttachment != -1 && m_Attachments.Count() >= iAttachment )
			{
				GetAttachment( iAttachment, attachOrigin, attachAngles );

				// Fill out the generic data
				CEffectData data;
				data.m_vOrigin = attachOrigin;
				data.m_vAngles = attachAngles;
				AngleVectors( attachAngles, &data.m_vNormal );
				data.m_hEntity = GetRefEHandle();
				data.m_nAttachmentIndex = iAttachment + 1;
				data.m_fFlags = iParam;

				DispatchEffect( effectFunc, data );
			}
		}
		break;

	// Spark
	case CL_EVENT_SPARK0:
		{
			Vector vecForward;
			GetAttachment( 1, attachOrigin, attachAngles );
			AngleVectors( attachAngles, &vecForward );
			g_pEffects->Sparks( attachOrigin, atoi( options ), 1, &vecForward );
		}
		break;

	// Sound
	case CL_EVENT_SOUND:		// Client side sound
		{
			CLocalPlayerFilter filter;

			if ( m_Attachments.Count() > 0)
			{
				GetAttachment( 1, attachOrigin, attachAngles );
				EmitSound( filter, GetSoundSourceIndex(), options, &attachOrigin );
			}
			else
			{
				EmitSound( filter, GetSoundSourceIndex(), options );
			}
		}
		break;

	case CL_EVENT_FOOTSTEP_LEFT:
		{
#ifndef HL2MP
			char pSoundName[256];
			if ( !options || !options[0] )
			{
				options = "NPC_CombineS";
			}

			Vector vel;
			EstimateAbsVelocity( vel );

			// If he's moving fast enough, play the run sound
			if ( vel.Length2DSqr() > RUN_SPEED_ESTIMATE_SQR )
			{
				Q_snprintf( pSoundName, 256, "%s.RunFootstepLeft", options );
			}
			else
			{
				Q_snprintf( pSoundName, 256, "%s.FootstepLeft", options );
			}
			EmitSound( pSoundName );
#endif
		}
		break;

	case CL_EVENT_FOOTSTEP_RIGHT:
		{
#ifndef HL2MP
			char pSoundName[256];
			if ( !options || !options[0] )
			{
				options = "NPC_CombineS";
			}

			Vector vel;
			EstimateAbsVelocity( vel );
			// If he's moving fast enough, play the run sound
			if ( vel.Length2DSqr() > RUN_SPEED_ESTIMATE_SQR )
			{
				Q_snprintf( pSoundName, 256, "%s.RunFootstepRight", options );
			}
			else
			{
				Q_snprintf( pSoundName, 256, "%s.FootstepRight", options );
			}
			EmitSound( pSoundName );
#endif
		}
		break;

	case CL_EVENT_MFOOTSTEP_LEFT:
		{
			MaterialFootstepSound( this, true, VOL_NORM * 0.5f );
		}
		break;

	case CL_EVENT_MFOOTSTEP_RIGHT:
		{
			MaterialFootstepSound( this, false, VOL_NORM * 0.5f );
		}
		break;

	case CL_EVENT_MFOOTSTEP_LEFT_LOUD:
		{
			MaterialFootstepSound( this, true, VOL_NORM );
		}
		break;

	case CL_EVENT_MFOOTSTEP_RIGHT_LOUD:
		{
			MaterialFootstepSound( this, false, VOL_NORM );
		}
		break;

	// Eject brass
	case CL_EVENT_EJECTBRASS1:
		if ( m_Attachments.Count() > 0 )
		{
			if ( MainViewOrigin().DistToSqr( GetAbsOrigin() ) < (256 * 256) )
			{
				Vector attachOrigin;
				QAngle attachAngles; 
				
				if( GetAttachment( 2, attachOrigin, attachAngles ) )
				{
					tempents->EjectBrass( attachOrigin, attachAngles, GetAbsAngles(), atoi( options ) );
				}
			}
		}
		break;

	// Generic dispatch effect hook
	case CL_EVENT_DISPATCHEFFECT0:
	case CL_EVENT_DISPATCHEFFECT1:
	case CL_EVENT_DISPATCHEFFECT2:
	case CL_EVENT_DISPATCHEFFECT3:
	case CL_EVENT_DISPATCHEFFECT4:
	case CL_EVENT_DISPATCHEFFECT5:
	case CL_EVENT_DISPATCHEFFECT6:
	case CL_EVENT_DISPATCHEFFECT7:
	case CL_EVENT_DISPATCHEFFECT8:
	case CL_EVENT_DISPATCHEFFECT9:
		{
			int iAttachment = -1;

			// First person muzzle flashes
			switch (event) 
			{
			case CL_EVENT_DISPATCHEFFECT0:
				iAttachment = 0;
				break;

			case CL_EVENT_DISPATCHEFFECT1:
				iAttachment = 1;
				break;

			case CL_EVENT_DISPATCHEFFECT2:
				iAttachment = 2;
				break;

			case CL_EVENT_DISPATCHEFFECT3:
				iAttachment = 3;
				break;

			case CL_EVENT_DISPATCHEFFECT4:
				iAttachment = 4;
				break;

			case CL_EVENT_DISPATCHEFFECT5:
				iAttachment = 5;
				break;

			case CL_EVENT_DISPATCHEFFECT6:
				iAttachment = 6;
				break;

			case CL_EVENT_DISPATCHEFFECT7:
				iAttachment = 7;
				break;

			case CL_EVENT_DISPATCHEFFECT8:
				iAttachment = 8;
				break;

			case CL_EVENT_DISPATCHEFFECT9:
				iAttachment = 9;
				break;
			}

			if ( iAttachment != -1 && m_Attachments.Count() > iAttachment )
			{
				GetAttachment( iAttachment+1, attachOrigin, attachAngles );

				// Fill out the generic data
				CEffectData data;
				data.m_vOrigin = attachOrigin;
				data.m_vAngles = attachAngles;
				AngleVectors( attachAngles, &data.m_vNormal );
				data.m_hEntity = GetRefEHandle();
				data.m_nAttachmentIndex = iAttachment + 1;

				DispatchEffect( options, data );
			}
		}
		break;

	case AE_MUZZLEFLASH:
		{
			// Send out the effect for a player
			DispatchMuzzleEffect( options, true );
			break;
		}

	case AE_NPC_MUZZLEFLASH:
		{
			// Send out the effect for an NPC
			DispatchMuzzleEffect( options, false );
			break;
		}

	// Old muzzleflashes
	case CL_EVENT_MUZZLEFLASH0:
	case CL_EVENT_MUZZLEFLASH1:
	case CL_EVENT_MUZZLEFLASH2:
	case CL_EVENT_MUZZLEFLASH3:
	case CL_EVENT_NPC_MUZZLEFLASH0:
	case CL_EVENT_NPC_MUZZLEFLASH1:
	case CL_EVENT_NPC_MUZZLEFLASH2:
	case CL_EVENT_NPC_MUZZLEFLASH3:
		{
			int iAttachment = -1;
			bool bFirstPerson = true;

			// First person muzzle flashes
			switch (event) 
			{
			case CL_EVENT_MUZZLEFLASH0:
				iAttachment = 0;
				break;

			case CL_EVENT_MUZZLEFLASH1:
				iAttachment = 1;
				break;

			case CL_EVENT_MUZZLEFLASH2:
				iAttachment = 2;
				break;

			case CL_EVENT_MUZZLEFLASH3:
				iAttachment = 3;
				break;

			// Third person muzzle flashes
			case CL_EVENT_NPC_MUZZLEFLASH0:
				iAttachment = 0;
				bFirstPerson = false;
				break;

			case CL_EVENT_NPC_MUZZLEFLASH1:
				iAttachment = 1;
				bFirstPerson = false;
				break;

			case CL_EVENT_NPC_MUZZLEFLASH2:
				iAttachment = 2;
				bFirstPerson = false;
				break;

			case CL_EVENT_NPC_MUZZLEFLASH3:
				iAttachment = 3;
				bFirstPerson = false;
				break;
			}

			if ( iAttachment != -1 && m_Attachments.Count() > iAttachment )
			{
				GetAttachment( iAttachment+1, attachOrigin, attachAngles );
				int entId = render->GetViewEntity();
				ClientEntityHandle_t hEntity = ClientEntityList().EntIndexToHandle( entId );
				tempents->MuzzleFlash( attachOrigin, attachAngles, atoi( options ), hEntity, bFirstPerson );
			}
		}
		break;

	default:
		break;
	}
}


bool C_BaseAnimating::IsSelfAnimating()
{
	if ( m_bClientSideAnimation )
		return true;

	// Yes, we use animtime.
	int iMoveType = GetMoveType();
	if ( iMoveType != MOVETYPE_STEP && 
		  iMoveType != MOVETYPE_NONE && 
		  iMoveType != MOVETYPE_WALK &&
		  iMoveType != MOVETYPE_FLY &&
		  iMoveType != MOVETYPE_FLYGRAVITY )
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Called by networking code when an entity is new to the PVS or comes down with the EF_NOINTERP flag set.
//  The position history data is flushed out right after this call, so we need to store off the current data
//  in the latched fields so we try to interpolate
// Input  : *ent - 
//			full_reset - 
//-----------------------------------------------------------------------------
void C_BaseAnimating::ResetLatched( void )
{
	// Reset the IK
	if ( m_pIk )
	{
		delete m_pIk;
		m_pIk = NULL;
	}

	BaseClass::ResetLatched();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------

bool C_BaseAnimating::Interpolate( float flCurrentTime )
{
	// ragdolls don't need interpolation
	if ( m_pRagdoll )
		return true;

	VPROF( "C_BaseAnimating::Interpolate" );

	Vector oldOrigin;
	QAngle oldAngles;
	float flOldCycle = GetCycle();
	int nChangeFlags = 0;

	if ( !m_bClientSideAnimation )
		m_iv_flCycle.SetLooping( IsSequenceLooping( GetSequence() ) );

	int bNoMoreChanges;
	int retVal = BaseInterpolatePart1( flCurrentTime, oldOrigin, oldAngles, bNoMoreChanges );
	if ( retVal == INTERPOLATE_STOP )
	{
		if ( bNoMoreChanges )
			RemoveFromInterpolationList();
		return true;
	}


	// Did cycle change?
	if( GetCycle() != flOldCycle )
		nChangeFlags |= ANIMATION_CHANGED;

	if ( bNoMoreChanges )
		RemoveFromInterpolationList();
	
	BaseInterpolatePart2( oldOrigin, oldAngles, nChangeFlags );
	return true;
}


//-----------------------------------------------------------------------------
// returns true if we're currently being ragdolled
//-----------------------------------------------------------------------------
bool C_BaseAnimating::IsRagdoll() const
{
	return m_pRagdoll && (m_nRenderFX == kRenderFxRagdoll);
}


//-----------------------------------------------------------------------------
// implements these so ragdolls can handle frustum culling & leaf visibility
//-----------------------------------------------------------------------------

void C_BaseAnimating::GetRenderBounds( Vector& theMins, Vector& theMaxs )
{
	if ( IsRagdoll() )
	{
		m_pRagdoll->GetRagdollBounds( theMins, theMaxs );
	}
	else if ( GetModel() )
	{
		CStudioHdr *pStudioHdr = GetModelPtr();
		if ( !pStudioHdr|| !pStudioHdr->SequencesAvailable() || GetSequence() == -1 )
		{
			theMins = vec3_origin;
			theMaxs = vec3_origin;
			return;
		} 
		if (!VectorCompare( vec3_origin, pStudioHdr->view_bbmin() ) || !VectorCompare( vec3_origin, pStudioHdr->view_bbmax() ))
		{
			// clipping bounding box
			VectorCopy ( pStudioHdr->view_bbmin(), theMins);
			VectorCopy ( pStudioHdr->view_bbmax(), theMaxs);
		}
		else
		{
			// movement bounding box
			VectorCopy ( pStudioHdr->hull_min(), theMins);
			VectorCopy ( pStudioHdr->hull_max(), theMaxs);
		}

		mstudioseqdesc_t &seqdesc = pStudioHdr->pSeqdesc( GetSequence() );
		VectorMin( seqdesc.bbmin, theMins, theMins );
		VectorMax( seqdesc.bbmax, theMaxs, theMaxs );
	}
	else
	{
		theMins = vec3_origin;
		theMaxs = vec3_origin;
	}
}


//-----------------------------------------------------------------------------
// implements these so ragdolls can handle frustum culling & leaf visibility
//-----------------------------------------------------------------------------
const Vector& C_BaseAnimating::GetRenderOrigin( void )
{
	if ( IsRagdoll() )
	{
		return m_pRagdoll->GetRagdollOrigin();
	}
	else
	{
		return BaseClass::GetRenderOrigin();	
	}
}

const QAngle& C_BaseAnimating::GetRenderAngles( void )
{
	if ( IsRagdoll() )
	{
		return vec3_angle;
			
	}
	else
	{
		return BaseClass::GetRenderAngles();	
	}
}

void C_BaseAnimating::RagdollMoved( void ) 
{
	SetAbsOrigin( m_pRagdoll->GetRagdollOrigin() );
	SetAbsAngles( vec3_angle );

	Vector mins, maxs;
	m_pRagdoll->GetRagdollBounds( mins, maxs );
	SetCollisionBounds( mins, maxs );

	// If the ragdoll moves, its render-to-texture shadow is dirty
	InvalidatePhysicsRecursive( ANIMATION_CHANGED ); 
}


//-----------------------------------------------------------------------------
// Purpose: My physics object has been updated, react or extract data
//-----------------------------------------------------------------------------
void C_BaseAnimating::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	// FIXME: Should make sure the physics objects being passed in
	// is the ragdoll physics object, but I think it's pretty safe not to check
	if (IsRagdoll())
	{	 
		m_pRagdoll->VPhysicsUpdate( pPhysics );
		
		RagdollMoved();

		return;
	}

	BaseClass::VPhysicsUpdate( pPhysics );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_BaseAnimating::PreDataUpdate( DataUpdateType_t updateType )
{
	m_flOldCycle = GetCycle();
	m_nOldSequence = GetSequence();
	BaseClass::PreDataUpdate( updateType );
}

void C_BaseAnimating::NotifyShouldTransmit( ShouldTransmitState_t state )
{
	BaseClass::NotifyShouldTransmit( state );

	if ( state == SHOULDTRANSMIT_START )
	{
		// If he's been firing a bunch, then he comes back into the PVS, his muzzle flash
		// will show up even if he isn't firing now.
		DisableMuzzleFlash();

		m_nPrevResetEventsParity = m_nResetEventsParity;
		m_nEventSequence = GetSequence();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_BaseAnimating::PostDataUpdate( DataUpdateType_t updateType )
{
	BaseClass::PostDataUpdate( updateType );

	if ( m_bClientSideAnimation )
	{
		SetCycle( m_flOldCycle );
		AddToClientSideAnimationList();
	}
	else
	{
		RemoveFromClientSideAnimationList();
	}

	// Cycle change? Then re-render
	if ( m_flOldCycle != GetCycle() || m_nOldSequence != GetSequence() )
	{
		InvalidatePhysicsRecursive( ANIMATION_CHANGED );

		if ( m_bClientSideAnimation )
		{
			ClientSideAnimationChanged();
		}
	}

	// reset prev cycle if new sequence
	if (m_nNewSequenceParity != m_nPrevNewSequenceParity)
	{
		// It's important not to call Reset() on a static prop, because if we call
		// Reset(), then the entity will stay in the interpolated entities list
		// forever, wasting CPU.
		MDLCACHE_CRITICAL_SECTION();
		CStudioHdr *hdr = GetModelPtr();
		if ( hdr && !( hdr->flags() & STUDIOHDR_FLAGS_STATIC_PROP ) )
		{
			m_iv_flCycle.Reset();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bnewentity - 
//-----------------------------------------------------------------------------
void C_BaseAnimating::OnPreDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnPreDataChanged( updateType );

	m_bLastClientSideFrameReset = m_bClientSideFrameReset;
}

void C_BaseAnimating::GetRagdollPreSequence( matrix3x4_t *preBones, float flTime )
{
	ForceAllInterpolate();
	Interpolate( flTime );
	// Setup previous bone state to extrapolate physics velocity
	SetupBones( preBones, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, flTime );
}

void C_BaseAnimating::GetRagdollCurSequence( matrix3x4_t *curBones, float flTime )
{
	// blow the cached prev bones
	InvalidateBoneCache();

	// reset absorigin/absangles
	ForceAllInterpolate();
	Interpolate( flTime );

	// Now do the current bone setup
	SetupBones( curBones, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, flTime );

	// blow the cached prev bones
	InvalidateBoneCache();

	SetupBones( NULL, -1, BONE_USED_BY_ANYTHING, flTime );
}

C_BaseAnimating * C_BaseAnimating::BecomeRagdollOnClient( bool bCopyEntity )
{
	CStudioHdr *hdr = GetModelPtr();
	if ( !hdr )
		return NULL;

	if ( m_pRagdoll || m_builtRagdoll )
		return NULL;
		
	float prevanimtime = gpGlobals->curtime - 0.1f;
	float curanimtime = gpGlobals->curtime;

	//Adrian: We now create a separate entity that becomes this entity's ragdoll.
	//That way the server side version of this entity can go away. 
	//Plus we can hook save/restore code to these ragdolls so they don't fall on restore anymore.
	C_BaseAnimating *pRagdoll = this;
	if ( bCopyEntity )
	{
		C_ClientRagdoll *pRagdollCopy = new C_ClientRagdoll( false );
		if ( pRagdollCopy == NULL )
			 return NULL;

		pRagdoll = pRagdollCopy;

		TermRopes();

		const model_t *model = GetModel();
		const char *pModelName = modelinfo->GetModelName( model );

		if ( pRagdoll->InitializeAsClientEntity( pModelName, RENDER_GROUP_OPAQUE_ENTITY ) == false )
		{
			pRagdoll->Release();
			return NULL;
		}

		// move my current model instance to the ragdoll's so decals are preserved.
		SnatchModelInstance( pRagdoll );

		// We need to take these from the entity
		pRagdoll->SetAbsOrigin( GetAbsOrigin() );
		pRagdoll->SetAbsAngles( GetAbsAngles() );
		
		pRagdoll->IgniteRagdoll( this );
		pRagdoll->TransferDissolveFrom( this );
		pRagdoll->InitRopes();
		
		if ( AddRagdollToFadeQueue() == true )
		{
			pRagdollCopy->m_bImportant = NPC_IsImportantNPC( this );
			s_RagdollLRU.MoveToTopOfLRU( pRagdoll, pRagdollCopy->m_bImportant );
			pRagdollCopy->m_bFadeOut = true;
		}

		m_builtRagdoll = true;
		AddEffects( EF_NODRAW );

		if ( IsEffectActive( EF_NOSHADOW ) )
		{
			pRagdoll->AddEffects( EF_NOSHADOW );
		}

		pRagdoll->m_nRenderFX = kRenderFxRagdoll;
		pRagdoll->SetRenderMode( GetRenderMode() );
		pRagdoll->SetRenderColor( GetRenderColor().r, GetRenderColor().g, GetRenderColor().b, GetRenderColor().a );

		pRagdoll->m_nBody = m_nBody;
		pRagdoll->m_nSkin = m_nSkin;
		pRagdoll->m_vecForce = m_vecForce;
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->SetNextClientThink( CLIENT_THINK_ALWAYS );
		
		pRagdoll->SetModelName( AllocPooledString(pModelName) );
	}
	
	pRagdoll->m_builtRagdoll = true;

	// Store off our old mins & maxs
	pRagdoll->m_vecPreRagdollMins = WorldAlignMins();
	pRagdoll->m_vecPreRagdollMaxs = WorldAlignMaxs();

	matrix3x4_t preBones[MAXSTUDIOBONES];
	matrix3x4_t curBones[MAXSTUDIOBONES];

	// Force MOVETYPE_STEP interpolation
	MoveType_t savedMovetype = GetMoveType();
	SetMoveType( MOVETYPE_STEP );

	// HACKHACK: force time to last interpolation position
	m_flPlaybackRate = 1;
	
	GetRagdollPreSequence( preBones, prevanimtime );
	GetRagdollCurSequence( curBones, curanimtime );

	pRagdoll->m_pRagdoll = CreateRagdoll( 
		pRagdoll, 
		hdr, 
		m_vecForce, 
		m_nForceBone, 
		CBoneAccessor( preBones ), 
		CBoneAccessor( curBones ), 
		m_BoneAccessor,
		curanimtime - prevanimtime );

	// Cause the entity to recompute its shadow	type and make a
	// version which only updates when physics state changes
	// NOTE: We have to do this after m_pRagdoll is assigned above
	// because that's what ShadowCastType uses to figure out which type of shadow to use.
	pRagdoll->DestroyShadow();
	pRagdoll->CreateShadow();

	// Cache off ragdoll bone positions/quaternions
	if ( pRagdoll->m_bStoreRagdollInfo && pRagdoll->m_pRagdoll )
	{
		matrix3x4_t parentTransform;
		AngleMatrix( GetAbsAngles(), GetAbsOrigin(), parentTransform );
		// FIXME/CHECK:  This might be too expensive to do every frame???
		SaveRagdollInfo( hdr->numbones(), parentTransform, pRagdoll->m_BoneAccessor );
	}
	
	SetMoveType( savedMovetype );

	// Now set the dieragdoll sequence to get transforms for all
	// non-simulated bones
	pRagdoll->m_nRestoreSequence = GetSequence();
    pRagdoll->SetSequence( SelectWeightedSequence( ACT_DIERAGDOLL ) );
	pRagdoll->m_nPrevSequence = GetSequence();
	pRagdoll->m_flPlaybackRate = 0;
	pRagdoll->UpdatePartitionListEntry();

	NoteRagdollCreationTick( pRagdoll );

	UpdateVisibility();

	return pRagdoll;
}



//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bnewentity - 
//-----------------------------------------------------------------------------
void C_BaseAnimating::OnDataChanged( DataUpdateType_t updateType )
{
	// don't let server change sequences after becoming a ragdoll
	if ( m_pRagdoll && GetSequence() != m_nPrevSequence )
	{
		SetSequence( m_nPrevSequence );
		m_flPlaybackRate = 0;
	}

	if ( !m_pRagdoll && m_nRestoreSequence != -1 )
	{
		SetSequence( m_nRestoreSequence );
		m_nRestoreSequence = -1;
	}

	if (updateType == DATA_UPDATE_CREATED)
	{
		m_nPrevSequence = -1;
		m_nRestoreSequence = -1;
	}



	bool modelchanged = false;

	// UNDONE: The base class does this as well.  So this is kind of ugly
	// but getting a model by index is pretty cheap...
	const model_t *pModel = modelinfo->GetModel( GetModelIndex() );
	
	if ( pModel != GetModel() )
	{
		modelchanged = true;
	}

	BaseClass::OnDataChanged( updateType );

	if ( (updateType == DATA_UPDATE_CREATED) || modelchanged )
	{
		ResetLatched();
		// if you have this pose parameter, activate HL1-style lipsync/wave envelope tracking
		if ( LookupPoseParameter( LIPSYNC_POSEPARAM_NAME ) != -1 )
		{
			MouthInfo().ActivateEnvelope();
		}
	}

	// If there's a significant change, make sure the shadow updates
	if ( modelchanged || (GetSequence() != m_nPrevSequence))
	{
		InvalidatePhysicsRecursive( ANIMATION_CHANGED ); 
		m_nPrevSequence = GetSequence();
	}

	// Only need to think if animating client side
	if ( m_bClientSideAnimation )
	{
		// Check to see if we should reset our frame
		if ( m_bClientSideFrameReset != m_bLastClientSideFrameReset )
		{
			SetCycle( 0 );
		}
	}
	// build a ragdoll if necessary
	if ( m_nRenderFX == kRenderFxRagdoll && !m_builtRagdoll )
	{
		MoveToLastReceivedPosition( true );
		GetAbsOrigin();
		BecomeRagdollOnClient();
	}

	//HACKHACK!!!
	if ( m_nRenderFX == kRenderFxRagdoll && m_builtRagdoll == true )
	{
		if ( m_pRagdoll == NULL )
			 AddEffects( EF_NODRAW );
	}

	if ( m_pRagdoll && m_nRenderFX != kRenderFxRagdoll )
	{
		ClearRagdoll();
	}

	// If ragdolling and get EF_NOINTERP, we probably were dead and are now respawning,
	//  don't do blend out of ragdoll at respawn spot.
	if ( IsEffectActive( EF_NOINTERP ) && 
		m_pRagdollInfo &&
		m_pRagdollInfo->m_bActive )
	{
		Msg( "delete ragdoll due to nointerp\n" );
		// Remove ragdoll info
		delete m_pRagdollInfo;
		m_pRagdollInfo = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BaseAnimating::AddEntity( void )
{
	// Server says don't interpolate this frame, so set previous info to new info.
	if ( IsEffectActive(EF_NOINTERP) )
	{
		ResetLatched();
	}

	BaseClass::AddEntity();
}

//-----------------------------------------------------------------------------
// Purpose: Get the index of the attachment point with the specified name
//-----------------------------------------------------------------------------
int C_BaseAnimating::LookupAttachment( const char *pAttachmentName )
{
	CStudioHdr *hdr = GetModelPtr();
	if ( !hdr )
	{
		return -1;
	}

	// NOTE: Currently, the network uses 0 to mean "no attachment" 
	// thus the client must add one to the index of the attachment
	// UNDONE: Make the server do this too to be consistent.
	return Studio_FindAttachment( hdr, pAttachmentName ) + 1;
}

//-----------------------------------------------------------------------------
// Purpose: Get a random index of an attachment point with the specified substring in its name
//-----------------------------------------------------------------------------
int C_BaseAnimating::LookupRandomAttachment( const char *pAttachmentNameSubstring )
{
	CStudioHdr *hdr = GetModelPtr();
	if ( !hdr )
	{
		return -1;
	}

	// NOTE: Currently, the network uses 0 to mean "no attachment" 
	// thus the client must add one to the index of the attachment
	// UNDONE: Make the server do this too to be consistent.
	return Studio_FindRandomAttachment( hdr, pAttachmentNameSubstring ) + 1;
}


void C_BaseAnimating::ClientSideAnimationChanged()
{
	if ( !m_bClientSideAnimation )
		return;

	MDLCACHE_CRITICAL_SECTION();
	
	Assert(m_ClientSideAnimationListHandle != INVALID_CLIENTSIDEANIMATION_LIST_HANDLE);
	clientanimating_t &anim = g_ClientSideAnimationList.Element(m_ClientSideAnimationListHandle);
	Assert(anim.pAnimating == this);
	anim.flags = ComputeClientSideAnimationFlags();

	m_SequenceTransitioner.CheckForSequenceChange( 
		GetModelPtr(),
		GetSequence(),
		m_nNewSequenceParity != m_nPrevNewSequenceParity,
		!IsEffectActive(EF_NOINTERP)
		);
}

unsigned int C_BaseAnimating::ComputeClientSideAnimationFlags()
{
	return FCLIENTANIM_SEQUENCE_CYCLE;
}

void C_BaseAnimating::UpdateClientSideAnimation()
{
	// Update client side animation
	if ( m_bClientSideAnimation )
	{

		Assert( m_ClientSideAnimationListHandle != INVALID_CLIENTSIDEANIMATION_LIST_HANDLE );
		if ( GetSequence() != -1 )
		{
			// latch old values
			OnLatchInterpolatedVariables( LATCH_ANIMATION_VAR );
			// move frame forward
			FrameAdvance( 0.0f ); // 0 means to use the time we last advanced instead of a constant
		}
	}
	else
	{
		Assert( m_ClientSideAnimationListHandle == INVALID_CLIENTSIDEANIMATION_LIST_HANDLE );
	}
}


void C_BaseAnimating::Simulate()
{
	if ( gpGlobals->frametime != 0.0f  )
	{
		DoAnimationEvents( GetModelPtr() );
	}
	BaseClass::Simulate();
	if ( GetSequence() != -1 && m_pRagdoll && ( m_nRenderFX != kRenderFxRagdoll ) )
	{
		ClearRagdoll();
	}
}


bool C_BaseAnimating::TestCollision( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr )
{
	if ( ray.m_IsRay && IsSolidFlagSet( FSOLID_CUSTOMRAYTEST ))
	{
		if (!TestHitboxes( ray, fContentsMask, tr ))
			return true;

		return tr.DidHit();
	}

	if ( !ray.m_IsRay && IsSolidFlagSet( FSOLID_CUSTOMBOXTEST ))
	{
		if (!TestHitboxes( ray, fContentsMask, tr ))
			return true;

		return true;
	}

	// We shouldn't get here.
	Assert(0);
	return false;
}


// UNDONE: This almost works.  The client entities have no control over their solid box
// Also they have no ability to expose FSOLID_ flags to the engine to force the accurate
// collision tests.
// Add those and the client hitboxes will be robust
bool C_BaseAnimating::TestHitboxes( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr )
{
	VPROF( "C_BaseAnimating::TestCollision" );

	CStudioHdr *pStudioHdr = GetModelPtr();
	if (!pStudioHdr)
		return false;

	mstudiohitboxset_t *set = pStudioHdr->pHitboxSet( m_nHitboxSet );
	if ( !set || !set->numhitboxes )
		return false;

	// Use vcollide for box traces.
	if ( !ray.m_IsRay )
		return false;

	// This *has* to be true for the existing code to function correctly.
	Assert( ray.m_StartOffset == vec3_origin );

	CBoneCache *pCache = GetBoneCache( pStudioHdr );
	matrix3x4_t *hitboxbones[MAXSTUDIOBONES];
	pCache->ReadCachedBonePointers( hitboxbones, pStudioHdr->numbones() );

	if ( TraceToStudio( ray, pStudioHdr, set, hitboxbones, fContentsMask, tr ) )
	{
		mstudiobbox_t *pbox = set->pHitbox( tr.hitbox );
		mstudiobone_t *pBone = pStudioHdr->pBone(pbox->bone);
		tr.surface.name = "**studio**";
		tr.surface.flags = SURF_HITBOX;
		tr.surface.surfaceProps = physprops->GetSurfaceIndex( pBone->pszSurfaceProp() );
		if ( IsRagdoll() )
		{
			IPhysicsObject *pReplace = m_pRagdoll->GetElement( tr.physicsbone );
			if ( pReplace )
			{
				VPhysicsSetObject( NULL );
				VPhysicsSetObject( pReplace );
			}
		}
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Check sequence framerate
// Input  : iSequence - 
// Output : float
//-----------------------------------------------------------------------------
float C_BaseAnimating::GetSequenceCycleRate( CStudioHdr *pStudioHdr, int iSequence )
{
	if ( !pStudioHdr )
		return 0.0f;

	return Studio_CPS( pStudioHdr, pStudioHdr->pSeqdesc(iSequence), iSequence, m_flPoseParameter );
}

float C_BaseAnimating::GetAnimTimeInterval( void ) const
{
#define MAX_ANIMTIME_INTERVAL 0.2f

	float flInterval = min( gpGlobals->curtime - m_flAnimTime, MAX_ANIMTIME_INTERVAL );
	return flInterval;
}


//-----------------------------------------------------------------------------
// Sets the cycle, marks the entity as being dirty
//-----------------------------------------------------------------------------
void C_BaseAnimating::SetCycle( float flCycle )
{
	if ( m_flCycle != flCycle )
	{
		m_flCycle = flCycle;
		InvalidatePhysicsRecursive( ANIMATION_CHANGED );
	}
}

//-----------------------------------------------------------------------------
// Sets the sequence, marks the entity as being dirty
//-----------------------------------------------------------------------------
void C_BaseAnimating::SetSequence( int nSequence )
{ 
	if ( m_nSequence != nSequence )
	{
		/*
		CStudioHdr *hdr = GetModelPtr();
		// Assert( hdr );
		if ( hdr )
		{
			Assert( nSequence >= 0 && nSequence < hdr->GetNumSeq() );
		}
		*/

		m_nSequence = nSequence; 
		InvalidatePhysicsRecursive( ANIMATION_CHANGED );
		if ( m_bClientSideAnimation )
		{
			ClientSideAnimationChanged();
		}
	}
}


//=========================================================
// StudioFrameAdvance - advance the animation frame up some interval (default 0.1) into the future
//=========================================================
void C_BaseAnimating::StudioFrameAdvance()
{
	if ( m_bClientSideAnimation )
		return;

	CStudioHdr *hdr = GetModelPtr();
	if ( !hdr )
		return;

	bool watch = 0;//Q_strstr( hdr->name, "objects/human_obj_powerpack_build.mdl" ) ? true : false;

	//if (!anim.prevanimtime)
	//{
		//anim.prevanimtime = m_flAnimTime = gpGlobals->curtime;
	//}

	// How long since last animtime
	float flInterval = GetAnimTimeInterval();

	if (flInterval <= 0.001)
	{
		// Msg("%s : %s : %5.3f (skip)\n", STRING(pev->classname), GetSequenceName( GetSequence() ), GetCycle() );
		return;
	}

	//anim.prevanimtime = m_flAnimTime;
	float cycleAdvance = flInterval * GetSequenceCycleRate( hdr, GetSequence() ) * m_flPlaybackRate;
	float flNewCycle = GetCycle() + cycleAdvance;
	m_flAnimTime = gpGlobals->curtime;

	if ( watch )
	{
		Msg("%s %6.3f : %6.3f (%.3f)\n", GetClassname(), gpGlobals->curtime, m_flAnimTime, flInterval );
	}

	if ( flNewCycle < 0.0f || flNewCycle >= 1.0f ) 
	{
		if ( IsSequenceLooping( hdr, GetSequence() ) )
		{
			 flNewCycle -= (int)(flNewCycle);
		}
		else
		{
		 	 flNewCycle = (flNewCycle < 0.0f) ? 0.0f : 1.0f;
		}
		
		m_bSequenceFinished = true;	// just in case it wasn't caught in GetEvents
	}

	SetCycle( flNewCycle );

	m_flGroundSpeed = GetSequenceGroundSpeed( hdr, GetSequence() );

#if 0
	// I didn't have a test case for this, but it seems like the right thing to do.  Check multi-player!

	// Msg("%s : %s : %5.1f\n", GetClassname(), GetSequenceName( GetSequence() ), GetCycle() );
	InvalidatePhysicsRecursive( ANIMATION_CHANGED );
#endif

	if ( watch )
	{
		Msg("%s : %s : %5.1f\n", GetClassname(), GetSequenceName( GetSequence() ), GetCycle() );
	}
}

float C_BaseAnimating::GetSequenceGroundSpeed( CStudioHdr *pStudioHdr, int iSequence )
{
	float t = SequenceDuration( pStudioHdr, iSequence );

	if (t > 0)
	{
		return GetSequenceMoveDist( pStudioHdr, iSequence ) / t;
	}
	else
	{
		return 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//
// Input  : iSequence - 
//
// Output : float
//-----------------------------------------------------------------------------
float C_BaseAnimating::GetSequenceMoveDist( CStudioHdr *pStudioHdr, int iSequence )
{
	Vector				vecReturn;
	
	::GetSequenceLinearMotion( pStudioHdr, iSequence, m_flPoseParameter, &vecReturn );

	return vecReturn.Length();
}


//-----------------------------------------------------------------------------
// Purpose: 
//
// Input  : iSequence - 
//			*pVec - 
//	
//-----------------------------------------------------------------------------
void C_BaseAnimating::GetSequenceLinearMotion( int iSequence, Vector *pVec )
{
	::GetSequenceLinearMotion( GetModelPtr(), iSequence, m_flPoseParameter, pVec );
}

void C_BaseAnimating::GetBlendedLinearVelocity( Vector *pVec )
{
	Vector vecDist;
	float flDuration;

	GetSequenceLinearMotion( GetSequence(), &vecDist );
	flDuration = SequenceDuration( GetSequence() );

	VectorScale( vecDist, 1.0 / flDuration, *pVec );

	Vector tmp;
	for (int i = m_SequenceTransitioner.m_animationQueue.Count() - 2; i >= 0; i--)
	{
		C_AnimationLayer *blend = &m_SequenceTransitioner.m_animationQueue[i];
	
		GetSequenceLinearMotion( blend->m_nSequence, &vecDist );
		flDuration = SequenceDuration( blend->m_nSequence );

		VectorScale( vecDist, 1.0 / flDuration, tmp );

		float flWeight = blend->GetFadeout( gpGlobals->curtime );
		*pVec = Lerp( flWeight, *pVec, tmp );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flInterval - 
// Output : float
//-----------------------------------------------------------------------------
float C_BaseAnimating::FrameAdvance( float flInterval )
{
	CStudioHdr *hdr = GetModelPtr();
	if ( !hdr )
		return 0.0f;

	bool bWatch = false; // Q_strstr( hdr->name, "commando" ) ? true : false;

	float curtime = gpGlobals->curtime;

	if (flInterval == 0.0f)
	{
		flInterval = ( curtime - m_flAnimTime );
		if (flInterval <= 0.001f)
		{
			return 0.0f;
		}
	}

	if ( !m_flAnimTime )
	{
		flInterval = 0.0f;
	}

	float cyclerate = GetSequenceCycleRate( hdr, GetSequence() );
	float addcycle = flInterval * cyclerate * m_flPlaybackRate;

	if( GetServerIntendedCycle() != -1.0f )
	{
		// The server would like us to ease in a correction so that we will animate the same on the client and server.
		// So we will actually advance the average of what we would have done and what the server wants.
		float serverCycle = GetServerIntendedCycle();
		float serverAdvance = serverCycle - GetCycle();
		bool adjustOkay = serverAdvance > 0.0f;// only want to go forward. backing up looks really jarring, even when slight
		if( serverAdvance < -0.8f )
		{
			// Oh wait, it was just a wraparound from .9 to .1.
			serverAdvance += 1;
			adjustOkay = true;
		}

		if( adjustOkay )
		{
			float originalAdvance = addcycle;
			addcycle = (serverAdvance + addcycle) / 2;

			const float MAX_CYCLE_ADJUSTMENT = 0.1f;
			addcycle = min( MAX_CYCLE_ADJUSTMENT, addcycle );// Don't do too big of a jump; it's too jarring as well.

			DevMsg( 2, "(%d): Cycle latch used to correct %.2f in to %.2f instead of %.2f.\n",
				entindex(), GetCycle(), GetCycle() + addcycle, GetCycle() + originalAdvance );
		}

		SetServerIntendedCycle(-1.0f); // Only use a correction once, it isn't valid any time but right now.
	}

	float flNewCycle = GetCycle() + addcycle;
	m_flAnimTime = curtime;

	if ( bWatch )
	{
		Msg("%i CLIENT Time: %6.3f : (Interval %f) : cycle %f rate %f add %f\n", 
			gpGlobals->tickcount, gpGlobals->curtime, flInterval, flNewCycle, cyclerate, addcycle );
	}

	if ( (flNewCycle < 0.0f) || (flNewCycle >= 1.0f) ) 
	{
		if ( IsSequenceLooping( hdr, GetSequence() ) )
		{
			flNewCycle -= (int)(flNewCycle);
		}
		else
		{
			flNewCycle = (flNewCycle < 0.0f) ? 0.0f : 1.0f;
		}
	}

	SetCycle( flNewCycle );

	return flInterval;
}

// Stubs for weapon prediction
void C_BaseAnimating::ResetSequenceInfo( void )
{
	if (GetSequence() == -1)
	{
		SetSequence( 0 );
	}

	CStudioHdr *pStudioHdr = GetModelPtr();
	m_flGroundSpeed = GetSequenceGroundSpeed( pStudioHdr, GetSequence() );
	// m_flAnimTime = gpGlobals->time;
	m_flPlaybackRate = 1.0;
	m_bSequenceFinished = false;
	m_flLastEventCheck = 0;

	m_nNewSequenceParity = ( ++m_nNewSequenceParity ) & EF_PARITY_MASK;
	m_nResetEventsParity = ( ++m_nResetEventsParity ) & EF_PARITY_MASK;
	
	// FIXME: why is this called here?  Nothing should have changed to make this nessesary
	SetEventIndexForSequence( pStudioHdr->pSeqdesc( GetSequence() ) );
}

//=========================================================
//=========================================================

bool C_BaseAnimating::IsSequenceLooping( CStudioHdr *pStudioHdr, int iSequence )
{
	return (::GetSequenceFlags( pStudioHdr, iSequence ) & STUDIO_LOOPING) != 0;
}

float C_BaseAnimating::SequenceDuration( CStudioHdr *pStudioHdr, int iSequence )
{
	if ( !pStudioHdr )
	{
		return 0.1f;
	}

	if (iSequence >= pStudioHdr->GetNumSeq() || iSequence < 0 )
	{
		DevWarning( 2, "C_BaseAnimating::SequenceDuration( %d ) out of range\n", iSequence );
		return 0.1;
	}

	return Studio_Duration( pStudioHdr, iSequence, m_flPoseParameter );

}

int C_BaseAnimating::FindTransitionSequence( int iCurrentSequence, int iGoalSequence, int *piDir )
{
	CStudioHdr *hdr = GetModelPtr();
	if ( !hdr )
	{
		return -1;
	}

	if (piDir == NULL)
	{
		int iDir = 1;
		int sequence = ::FindTransitionSequence( hdr, iCurrentSequence, iGoalSequence, &iDir );
		if (iDir != 1)
			return -1;
		else
			return sequence;
	}

	return ::FindTransitionSequence( hdr, iCurrentSequence, iGoalSequence, piDir );

}

void C_BaseAnimating::SetBodygroup( int iGroup, int iValue )
{
	Assert( GetModelPtr() );

	::SetBodygroup( GetModelPtr( ), m_nBody, iGroup, iValue );
}

int C_BaseAnimating::GetBodygroup( int iGroup )
{
	Assert( GetModelPtr() );

	return ::GetBodygroup( GetModelPtr( ), m_nBody, iGroup );
}

const char *C_BaseAnimating::GetBodygroupName( int iGroup )
{
	Assert( GetModelPtr() );

	return ::GetBodygroupName( GetModelPtr( ), iGroup );
}

int C_BaseAnimating::FindBodygroupByName( const char *name )
{
	Assert( GetModelPtr() );

	return ::FindBodygroupByName( GetModelPtr( ), name );
}

int C_BaseAnimating::GetBodygroupCount( int iGroup )
{
	Assert( GetModelPtr() );

	return ::GetBodygroupCount( GetModelPtr( ), iGroup );
}

int C_BaseAnimating::GetNumBodyGroups( void )
{
	Assert( GetModelPtr() );

	return ::GetNumBodyGroups( GetModelPtr( ) );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : setnum - 
//-----------------------------------------------------------------------------
void C_BaseAnimating::SetHitboxSet( int setnum )
{
#ifdef _DEBUG
	CStudioHdr *pStudioHdr = GetModelPtr();
	if ( !pStudioHdr )
		return;

	if (setnum > pStudioHdr->numhitboxsets())
	{
		// Warn if an bogus hitbox set is being used....
		static bool s_bWarned = false;
		if (!s_bWarned)
		{
			Warning("Using bogus hitbox set in entity %s!\n", GetClassname() );
			s_bWarned = true;
		}
		setnum = 0;
	}
#endif

	m_nHitboxSet = setnum;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *setname - 
//-----------------------------------------------------------------------------
void C_BaseAnimating::SetHitboxSetByName( const char *setname )
{
	m_nHitboxSet = FindHitboxSetByName( GetModelPtr(), setname );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int C_BaseAnimating::GetHitboxSet( void )
{
	return m_nHitboxSet;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : char const
//-----------------------------------------------------------------------------
const char *C_BaseAnimating::GetHitboxSetName( void )
{
	return ::GetHitboxSetName( GetModelPtr(), m_nHitboxSet );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int C_BaseAnimating::GetHitboxSetCount( void )
{
	return ::GetHitboxSetCount( GetModelPtr() );
}

static Vector	hullcolor[8] = 
{
	Vector( 1.0, 1.0, 1.0 ),
	Vector( 1.0, 0.5, 0.5 ),
	Vector( 0.5, 1.0, 0.5 ),
	Vector( 1.0, 1.0, 0.5 ),
	Vector( 0.5, 0.5, 1.0 ),
	Vector( 1.0, 0.5, 1.0 ),
	Vector( 0.5, 1.0, 1.0 ),
	Vector( 1.0, 1.0, 1.0 )
};

//-----------------------------------------------------------------------------
// Purpose: Draw the current hitboxes
//-----------------------------------------------------------------------------
void C_BaseAnimating::DrawClientHitboxes( float duration /*= 0.0f*/, bool monocolor /*= false*/  )
{
	CStudioHdr *pStudioHdr = GetModelPtr();
	if ( !pStudioHdr )
		return;

	mstudiohitboxset_t *set =pStudioHdr->pHitboxSet( m_nHitboxSet );
	if ( !set )
		return;

	Vector position;
	QAngle angles;

	int r = 255;
	int g = 0;
	int b = 0;

	for ( int i = 0; i < set->numhitboxes; i++ )
	{
		mstudiobbox_t *pbox = set->pHitbox( i );

		GetBonePosition( pbox->bone, position, angles );

		if ( !monocolor )
		{
			int j = (pbox->group % 8);
			r = ( int ) ( 255.0f * hullcolor[j][0] );
			g = ( int ) ( 255.0f * hullcolor[j][1] );
			b = ( int ) ( 255.0f * hullcolor[j][2] );
		}

		debugoverlay->AddBoxOverlay( position, pbox->bbmin, pbox->bbmax, angles, r, g, b, 0 ,duration );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : activity - 
// Output : int C_BaseAnimating::SelectWeightedSequence
//-----------------------------------------------------------------------------
int C_BaseAnimating::SelectWeightedSequence ( int activity )
{
	Assert( activity != ACT_INVALID );

	return ::SelectWeightedSequence( GetModelPtr(), activity );

}

//=========================================================
//=========================================================
int C_BaseAnimating::LookupPoseParameter( CStudioHdr *pstudiohdr, const char *szName )
{
	if ( !pstudiohdr )
		return 0;

	for (int i = 0; i < pstudiohdr->GetNumPoseParameters(); i++)
	{
		if (stricmp( pstudiohdr->pPoseParameter( i ).pszName(), szName ) == 0)
		{
			return i;
		}
	}

	// AssertMsg( 0, UTIL_VarArgs( "poseparameter %s couldn't be mapped!!!\n", szName ) );
	return -1; // Error
}

//=========================================================
//=========================================================
float C_BaseAnimating::SetPoseParameter( CStudioHdr *pStudioHdr, const char *szName, float flValue )
{
	return SetPoseParameter( pStudioHdr, LookupPoseParameter( pStudioHdr, szName ), flValue );
}

float C_BaseAnimating::SetPoseParameter( CStudioHdr *pStudioHdr, int iParameter, float flValue )
{
	if ( !pStudioHdr )
	{
		Assert(!"C_BaseAnimating::SetPoseParameter: model missing");
		return flValue;
	}

	if (iParameter >= 0)
	{
		float flNewValue;
		flValue = Studio_SetPoseParameter( pStudioHdr, iParameter, flValue, flNewValue );
		m_flPoseParameter[ iParameter ] = flNewValue;
	}

	return flValue;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *label - 
// Output : int
//-----------------------------------------------------------------------------
int C_BaseAnimating::LookupSequence( const char *label )
{
	Assert( GetModelPtr() );
	return ::LookupSequence( GetModelPtr(), label );
}

void C_BaseAnimating::Release()
{
	ClearRagdoll();
	BaseClass::Release();
}

void C_BaseAnimating::Clear( void )
{
	Q_memset(&m_mouth, 0, sizeof(m_mouth));
	BaseClass::Clear();	
}

//-----------------------------------------------------------------------------
// Purpose: Clear current ragdoll
//-----------------------------------------------------------------------------
void C_BaseAnimating::ClearRagdoll()
{
	if ( m_pRagdoll )
	{
		delete m_pRagdoll;
		m_pRagdoll = NULL;

		// Set to null so that the destructor's call to DestroyObject won't destroy
		//  m_pObjects[ 0 ] twice since that's the physics object for the prop
		VPhysicsSetObject( NULL );

		// If we have ragdoll mins/maxs, we've just come out of ragdoll, so restore them
		if ( m_vecPreRagdollMins != vec3_origin || m_vecPreRagdollMaxs != vec3_origin )
		{
			SetCollisionBounds( m_vecPreRagdollMins, m_vecPreRagdollMaxs );
		}
	}
	m_builtRagdoll = false;
}

//-----------------------------------------------------------------------------
// Purpose: Looks up an activity by name.
// Input  : label - Name of the activity, ie "ACT_IDLE".
// Output : Returns the activity ID or ACT_INVALID.
//-----------------------------------------------------------------------------
int C_BaseAnimating::LookupActivity( const char *label )
{
	Assert( GetModelPtr() );
	return ::LookupActivity( GetModelPtr(), label );
}

//-----------------------------------------------------------------------------
// Purpose: 
//
// Input  : iSequence - 
//
// Output : char
//-----------------------------------------------------------------------------
const char *C_BaseAnimating::GetSequenceActivityName( int iSequence )
{
	if( iSequence == -1 )
	{
		return "Not Found!";
	}

	if ( !GetModelPtr() )
		return "No model!";

	return ::GetSequenceActivityName( GetModelPtr(), iSequence );
}

//=========================================================
//=========================================================
float C_BaseAnimating::SetBoneController ( int iController, float flValue )
{
	Assert( GetModelPtr() );

	CStudioHdr *pmodel = GetModelPtr();

	Assert(iController >= 0 && iController < NUM_BONECTRLS);

	float controller = m_flEncodedController[iController];
	float retVal = Studio_SetController( pmodel, iController, flValue, controller );
	m_flEncodedController[iController] = controller;
	return retVal;
}


void C_BaseAnimating::GetAimEntOrigin( IClientEntity *pAttachedTo, Vector *pAbsOrigin, QAngle *pAbsAngles )
{
	CBaseEntity *pMoveParent;
	if ( IsEffectActive( EF_BONEMERGE ) && IsEffectActive( EF_BONEMERGE_FASTCULL ) && (pMoveParent = GetMoveParent()) != NULL )
	{
		// Doing this saves a lot of CPU.
		*pAbsOrigin = pMoveParent->WorldSpaceCenter();
		*pAbsAngles = pMoveParent->GetRenderAngles();
	}
	else
	{
		if ( !m_pBoneMergeCache || !m_pBoneMergeCache->GetAimEntOrigin( pAbsOrigin, pAbsAngles ) )
			BaseClass::GetAimEntOrigin( pAttachedTo, pAbsOrigin, pAbsAngles );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//
// Input  : iSequence - 
//
// Output : char
//-----------------------------------------------------------------------------
const char *C_BaseAnimating::GetSequenceName( int iSequence )
{
	if( iSequence == -1 )
	{
		return "Not Found!";
	}

	if ( !GetModelPtr() )
		return "No model!";

	return ::GetSequenceName( GetModelPtr(), iSequence );
}

Activity C_BaseAnimating::GetSequenceActivity( int iSequence )
{
	if( iSequence == -1 )
	{
		return ACT_INVALID;
	}

	if ( !GetModelPtr() )
		return ACT_INVALID;

	return (Activity)::GetSequenceActivity( GetModelPtr(), iSequence );
}


//-----------------------------------------------------------------------------
// Computes a box that surrounds all hitboxes
//-----------------------------------------------------------------------------
bool C_BaseAnimating::ComputeHitboxSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs )
{
	// Note that this currently should not be called during position recomputation because of IK.
	// The code below recomputes bones so as to get at the hitboxes,
	// which causes IK to trigger, which causes raycasts against the other entities to occur,
	// which is illegal to do while in the computeabsposition phase.

	CStudioHdr *pStudioHdr = GetModelPtr();
	if (!pStudioHdr)
		return false;

	mstudiohitboxset_t *set = pStudioHdr->pHitboxSet( m_nHitboxSet );
	if ( !set || !set->numhitboxes )
		return false;

	CBoneCache *pCache = GetBoneCache( pStudioHdr );
	matrix3x4_t *hitboxbones[MAXSTUDIOBONES];
	pCache->ReadCachedBonePointers( hitboxbones, pStudioHdr->numbones() );

	// Compute a box in world space that surrounds this entity
	pVecWorldMins->Init( FLT_MAX, FLT_MAX, FLT_MAX );
	pVecWorldMaxs->Init( -FLT_MAX, -FLT_MAX, -FLT_MAX );

	Vector vecBoxAbsMins, vecBoxAbsMaxs;
	for ( int i = 0; i < set->numhitboxes; i++ )
	{
		mstudiobbox_t *pbox = set->pHitbox(i);

		TransformAABB( *hitboxbones[pbox->bone], pbox->bbmin, pbox->bbmax, vecBoxAbsMins, vecBoxAbsMaxs );
		VectorMin( *pVecWorldMins, vecBoxAbsMins, *pVecWorldMins );
		VectorMax( *pVecWorldMaxs, vecBoxAbsMaxs, *pVecWorldMaxs );
	}
	return true;
}

//-----------------------------------------------------------------------------
// Computes a box that surrounds all hitboxes, in entity space
//-----------------------------------------------------------------------------
bool C_BaseAnimating::ComputeEntitySpaceHitboxSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs )
{
	// Note that this currently should not be called during position recomputation because of IK.
	// The code below recomputes bones so as to get at the hitboxes,
	// which causes IK to trigger, which causes raycasts against the other entities to occur,
	// which is illegal to do while in the computeabsposition phase.

	CStudioHdr *pStudioHdr = GetModelPtr();
	if (!pStudioHdr)
		return false;

	mstudiohitboxset_t *set = pStudioHdr->pHitboxSet( m_nHitboxSet );
	if ( !set || !set->numhitboxes )
		return false;

	CBoneCache *pCache = GetBoneCache( pStudioHdr );
	matrix3x4_t *hitboxbones[MAXSTUDIOBONES];
	pCache->ReadCachedBonePointers( hitboxbones, pStudioHdr->numbones() );

	// Compute a box in world space that surrounds this entity
	pVecWorldMins->Init( FLT_MAX, FLT_MAX, FLT_MAX );
	pVecWorldMaxs->Init( -FLT_MAX, -FLT_MAX, -FLT_MAX );

	matrix3x4_t worldToEntity, boneToEntity;
	MatrixInvert( EntityToWorldTransform(), worldToEntity );

	Vector vecBoxAbsMins, vecBoxAbsMaxs;
	for ( int i = 0; i < set->numhitboxes; i++ )
	{
		mstudiobbox_t *pbox = set->pHitbox(i);

		ConcatTransforms( worldToEntity, *hitboxbones[pbox->bone], boneToEntity );
		TransformAABB( boneToEntity, pbox->bbmin, pbox->bbmax, vecBoxAbsMins, vecBoxAbsMaxs );
		VectorMin( *pVecWorldMins, vecBoxAbsMins, *pVecWorldMins );
		VectorMax( *pVecWorldMaxs, vecBoxAbsMaxs, *pVecWorldMaxs );
	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : scale - 
//-----------------------------------------------------------------------------
void C_BaseAnimating::SetModelWidthScale( float scale )
{
	m_flModelWidthScale = scale;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float C_BaseAnimating::GetModelWidthScale() const
{
	return m_flModelWidthScale;
}

//-----------------------------------------------------------------------------
// Purpose: Clientside bone follower class. Used just to visualize them.
//			Bone followers WON'T be sent to the client if VISUALIZE_FOLLOWERS is
//			undefined in the server's physics_bone_followers.cpp
//-----------------------------------------------------------------------------
class C_BoneFollower : public C_BaseEntity
{
	DECLARE_CLASS( C_BoneFollower, C_BaseEntity );
	DECLARE_CLIENTCLASS();
public:
	C_BoneFollower( void )
	{
	}

	bool	ShouldDraw( void );
	int		DrawModel( int flags );

private:
	int m_modelIndex;
	int m_solidIndex;
};

IMPLEMENT_CLIENTCLASS_DT( C_BoneFollower, DT_BoneFollower, CBoneFollower )
	RecvPropInt( RECVINFO( m_modelIndex ) ),
	RecvPropInt( RECVINFO( m_solidIndex ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Returns whether object should render.
//-----------------------------------------------------------------------------
bool C_BoneFollower::ShouldDraw( void )
{
	return ( vcollide_wireframe.GetBool() );  //MOTODO
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_BoneFollower::DrawModel( int flags )
{
	vcollide_t *pCollide = modelinfo->GetVCollide( m_modelIndex );
	if ( pCollide )
	{
		static color32 debugColor = {0,255,255,0};
		matrix3x4_t matrix;
		AngleMatrix( GetAbsAngles(), GetAbsOrigin(), matrix );
		engine->DebugDrawPhysCollide( pCollide->solids[m_solidIndex], NULL, matrix, debugColor );
	}
	return 1;
}


void C_BaseAnimating::DisableMuzzleFlash()
{
	m_nOldMuzzleFlashParity = m_nMuzzleFlashParity;
}


void C_BaseAnimating::DoMuzzleFlash()
{
	m_nMuzzleFlashParity = (m_nMuzzleFlashParity+1) & ((1 << EF_MUZZLEFLASH_BITS) - 1);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void DevMsgRT( char const* pMsg, ... )
{
	if (gpGlobals->frametime != 0.0f)
	{
		va_list argptr;
		va_start( argptr, pMsg );
		// 
		{
			static char	string[1024];
			Q_vsnprintf (string, sizeof( string ), pMsg, argptr);
			DevMsg( 1, "%s", string );
		}
		// DevMsg( pMsg, argptr );
		va_end( argptr );
	}
}


void C_BaseAnimating::ForceClientSideAnimationOn()
{
	m_bClientSideAnimation = true;
	AddToClientSideAnimationList();
}


void C_BaseAnimating::AddToClientSideAnimationList()
{
	// Already in list
	if ( m_ClientSideAnimationListHandle != INVALID_CLIENTSIDEANIMATION_LIST_HANDLE )
		return;

	clientanimating_t list( this, 0 );
	m_ClientSideAnimationListHandle = g_ClientSideAnimationList.AddToTail( list );
	ClientSideAnimationChanged();
}

void C_BaseAnimating::RemoveFromClientSideAnimationList()
{
	// Not in list yet
	if ( INVALID_CLIENTSIDEANIMATION_LIST_HANDLE == m_ClientSideAnimationListHandle )
		return;

	unsigned int c = g_ClientSideAnimationList.Count();

	Assert( m_ClientSideAnimationListHandle < c );

	unsigned int last = c - 1;

	if ( last == m_ClientSideAnimationListHandle )
	{
		// Just wipe the final entry
		g_ClientSideAnimationList.FastRemove( last );
	}
	else
	{
		clientanimating_t lastEntry = g_ClientSideAnimationList[ last ];
		// Remove the last entry
		g_ClientSideAnimationList.FastRemove( last );

		// And update it's handle to point to this slot.
		lastEntry.pAnimating->m_ClientSideAnimationListHandle = m_ClientSideAnimationListHandle;
		g_ClientSideAnimationList[ m_ClientSideAnimationListHandle ] = lastEntry;
	}

	// Invalidate our handle no matter what.
	m_ClientSideAnimationListHandle = INVALID_CLIENTSIDEANIMATION_LIST_HANDLE;
}


// static method
void C_BaseAnimating::UpdateClientSideAnimations()
{
	VPROF_BUDGET( "UpdateClientSideAnimations", VPROF_BUDGETGROUP_CLIENT_ANIMATION );

	int c = g_ClientSideAnimationList.Count();
	for ( int i = 0; i < c ; ++i )
	{
		clientanimating_t &anim = g_ClientSideAnimationList.Element(i);
		if ( !(anim.flags & FCLIENTANIM_SEQUENCE_CYCLE) )
			continue;
		Assert( anim.pAnimating );
		anim.pAnimating->UpdateClientSideAnimation();
	}
}

CBoneList *C_BaseAnimating::RecordBones( CStudioHdr *hdr )
{
	if ( !ToolsEnabled() )
		return NULL;
		
	VPROF_BUDGET( "C_BaseAnimating::RecordBones", VPROF_BUDGETGROUP_TOOLS );

	// Possible optimization: Instead of inverting everything while recording, record the pos/q stuff into a structure instead?
	Assert( hdr );

	// Setup our transform based on render angles and origin.
	matrix3x4_t parentTransform;
	AngleMatrix( GetRenderAngles(), GetRenderOrigin(), parentTransform );

	CBoneList *boneList = CBoneList::Alloc();
	Assert( boneList );

	boneList->m_nBones = hdr->numbones();

	m_BoneAccessor.SetReadableBones( BONE_USED_BY_ANYTHING );

	for ( int i = 0;  i < hdr->numbones(); i++ )
	{
		matrix3x4_t inverted;
		matrix3x4_t output;

		mstudiobone_t *bone = hdr->pBone( i );

		// Only update bones referenced during setup
		if ( !(bone->flags & BONE_USED_BY_ANYTHING ) )
		{
			boneList->m_quatRot[ i ].Init( 0.0f, 0.0f, 0.0f, 1.0f ); // Init by default sets all 0's, which is invalid
			boneList->m_vecPos[ i ].Init();
			continue;
		}

		if ( bone->parent == -1 )
		{
			// Decompose into parent space
			MatrixInvert( parentTransform, inverted );
		}
		else
		{
			MatrixInvert( m_BoneAccessor.GetBone( bone->parent ), inverted );
		}

		ConcatTransforms( inverted, m_BoneAccessor.GetBone( i ), output );

		MatrixAngles( output, 
			boneList->m_quatRot[ i ],
			boneList->m_vecPos[ i ] );
	}

	return boneList;
}

void C_BaseAnimating::GetToolRecordingState( KeyValues *msg )
{
	if ( !ToolsEnabled() )
		return;

	VPROF_BUDGET( "C_BaseAnimating::GetToolRecordingState", VPROF_BUDGETGROUP_TOOLS );

	// Force the animation to drive bones
	SetupBones( NULL, -1, BONE_USED_BY_ANYTHING, gpGlobals->curtime );

	BaseClass::GetToolRecordingState( msg );

	static BaseAnimatingRecordingState_t state;
	state.m_nSkin = m_nSkin;
	state.m_nBody = m_nBody;
	state.m_nSequence = m_nSequence;
	state.m_pBoneList = NULL;
	msg->SetPtr( "baseanimating", &state );
	msg->SetInt( "viewmodel", IsViewModel() ? 1 : 0 );

	CStudioHdr *hdr = GetModelPtr();
	if ( hdr  )
	{
		state.m_pBoneList = RecordBones( hdr );
	}
}

void C_BaseAnimating::CleanupToolRecordingState( KeyValues *msg )
{
	if ( !ToolsEnabled() )
		return;

	BaseAnimatingRecordingState_t *pState = (BaseAnimatingRecordingState_t*)msg->GetPtr( "baseanimating" );
	if ( pState->m_pBoneList )
	{
		pState->m_pBoneList->Release();
	}

	BaseClass::CleanupToolRecordingState( msg );
}

int C_BaseAnimating::GetNumFlexControllers( void )
{
	CStudioHdr *pstudiohdr = GetModelPtr( );
	if (! pstudiohdr)
		return 0;

	return pstudiohdr->numflexcontrollers();
}

const char *C_BaseAnimating::GetFlexDescFacs( int iFlexDesc )
{
	CStudioHdr *pstudiohdr = GetModelPtr( );
	if (! pstudiohdr)
		return 0;

	mstudioflexdesc_t *pflexdesc = pstudiohdr->pFlexdesc( iFlexDesc );

	return pflexdesc->pszFACS( );
}

const char *C_BaseAnimating::GetFlexControllerName( int iFlexController )
{
	CStudioHdr *pstudiohdr = GetModelPtr( );
	if (! pstudiohdr)
		return 0;

	mstudioflexcontroller_t *pflexcontroller = pstudiohdr->pFlexcontroller( iFlexController );

	return pflexcontroller->pszName( );
}

const char *C_BaseAnimating::GetFlexControllerType( int iFlexController )
{
	CStudioHdr *pstudiohdr = GetModelPtr( );
	if (! pstudiohdr)
		return 0;

	mstudioflexcontroller_t *pflexcontroller = pstudiohdr->pFlexcontroller( iFlexController );

	return pflexcontroller->pszType( );
}

//-----------------------------------------------------------------------------
// Purpose: Returns the fade scale of the entity in question
// Output : unsigned char - 0 - 255 alpha value
//-----------------------------------------------------------------------------
unsigned char C_BaseAnimating::GetClientSideFade( void )
{
	return UTIL_ComputeEntityFade( this, m_fadeMinDist, m_fadeMaxDist, m_flFadeScale );
}
