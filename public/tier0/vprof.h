//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Real-Time Hierarchical Profiling
//
// $NoKeywords: $
//=============================================================================//

#ifndef VPROF_H
#define VPROF_H

#include "tier0/dbg.h"
#include "tier0/fasttimer.h"
#include "tier0/l2cache.h"
#include "tier0/threadtools.h"

// VProf is enabled by default in all configurations -except- XBOX Retail.
#if !( defined(_XBOX) && defined(_RETAIL) )
#define VPROF_ENABLED
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif

//-----------------------------------------------------------------------------
//
// Profiling instrumentation macros
//

#define MAXCOUNTERS 256


#ifdef VPROF_ENABLED

#define VPROF_VTUNE_GROUP

#define	VPROF( name )						VPROF_(name, 1, VPROF_BUDGETGROUP_OTHER_UNACCOUNTED, false, 0)
#define	VPROF_ASSERT_ACCOUNTED( name )		VPROF_(name, 1, VPROF_BUDGETGROUP_OTHER_UNACCOUNTED, true, 0)
#define	VPROF_( name, detail, group, bAssertAccounted, budgetFlags )		VPROF_##detail(name,group, bAssertAccounted, budgetFlags)

#define VPROF_BUDGET( name, group )					VPROF_BUDGET_FLAGS(name, group, BUDGETFLAG_OTHER)
#define VPROF_BUDGET_FLAGS( name, group, flags )	VPROF_(name, 0, group, false, flags)

#define VPROF_SCOPE_BEGIN( tag )	do { VPROF( tag )
#define VPROF_SCOPE_END()			} while (0)

#define VPROF_ONLY( expression )	expression

#define VPROF_ENTER_SCOPE( name )			g_VProfCurrentProfile.EnterScope( name, 1, VPROF_BUDGETGROUP_OTHER_UNACCOUNTED, false, 0 )
#define VPROF_EXIT_SCOPE()					g_VProfCurrentProfile.ExitScope()

#define VPROF_BUDGET_GROUP_ID_UNACCOUNTED 0


// Budgetgroup flags. These are used with VPROF_BUDGET_FLAGS.
// These control which budget panels the groups show up in.
// If a budget group uses VPROF_BUDGET, it gets the default 
// which is BUDGETFLAG_OTHER.
#define BUDGETFLAG_CLIENT	(1<<0)		// Shows up in the client panel.
#define BUDGETFLAG_SERVER	(1<<1)		// Shows up in the server panel.
#define BUDGETFLAG_OTHER	(1<<2)		// Unclassified (the client shows these but the dedicated server doesn't).
#define BUDGETFLAG_ALL		0xFFFF


// NOTE: You can use strings instead of these defines. . they are defined here and added
// in vprof.cpp so that they are always in the same order.
#define VPROF_BUDGETGROUP_OTHER_UNACCOUNTED			_T("Unaccounted")
#define VPROF_BUDGETGROUP_WORLD_RENDERING			_T("World Rendering")
#define VPROF_BUDGETGROUP_DISPLACEMENT_RENDERING	_T("Displacement Rendering")
#define VPROF_BUDGETGROUP_GAME						_T("Game")
#define VPROF_BUDGETGROUP_NPCS						_T("NPCs")
#define VPROF_BUDGETGROUP_SERVER_ANIM				_T("Server Animation")
#define VPROF_BUDGETGROUP_PHYSICS					_T("Physics")
#define VPROF_BUDGETGROUP_STATICPROP_RENDERING		_T("Static Prop Rendering")
#define VPROF_BUDGETGROUP_MODEL_RENDERING			_T("Other Model Rendering")
#define VPROF_BUDGETGROUP_BRUSHMODEL_RENDERING		_T("Brush Model Rendering")
#define VPROF_BUDGETGROUP_SHADOW_RENDERING			_T("Shadow Rendering")
#define VPROF_BUDGETGROUP_DETAILPROP_RENDERING		_T("Detail Prop Rendering")
#define VPROF_BUDGETGROUP_PARTICLE_RENDERING		_T("Particle/Effect Rendering")
#define VPROF_BUDGETGROUP_ROPES						_T("Ropes")
#define VPROF_BUDGETGROUP_DLIGHT_RENDERING			_T("Dynamic Light Rendering")
#define VPROF_BUDGETGROUP_OTHER_NETWORKING			_T("Networking")
#define VPROF_BUDGETGROUP_CLIENT_ANIMATION			_T("Client Animation")
#define VPROF_BUDGETGROUP_OTHER_SOUND				_T("Sound")
#define VPROF_BUDGETGROUP_OTHER_VGUI				_T("VGUI")
#define VPROF_BUDGETGROUP_OTHER_FILESYSTEM			_T("FileSystem")
#define VPROF_BUDGETGROUP_PREDICTION				_T("Prediction")
#define VPROF_BUDGETGROUP_INTERPOLATION				_T("Interpolation")
#define VPROF_BUDGETGROUP_SWAP_BUFFERS				_T("Swap Buffers")
#define VPROF_BUDGETGROUP_PLAYER					_T("Player")
#define VPROF_BUDGETGROUP_OCCLUSION					_T("Occlusion")
#define VPROF_BUDGETGROUP_OVERLAYS					_T("Overlays")
#define VPROF_BUDGETGROUP_TOOLS						_T("Tools")
#define VPROF_BUDGETGROUP_LIGHTCACHE				_T("Light Cache")
#define VPROF_BUDGETGROUP_DISP_RAYTRACES			_T("Displacement Ray Traces")
#define VPROF_BUDGETGROUP_DISP_HULLTRACES			_T("Displacement Hull Traces")
#define VPROF_BUDGETGROUP_TEXTURE_CACHE				_T("Texture Cache")

// FF SPECIFIC VPROF ENTRIES
#define VPROF_BUDGETGROUP_FF_BUILDABLE					_T("FF Buildable Objects")
	
#ifdef _XBOX
// update flags
#define VPROF_UPDATE_BUDGET				0x01	// send budget data every frame
#define VPROF_UPDATE_TEXTURE_GLOBAL		0x02	// send global texture data every frame
#define VPROF_UPDATE_TEXTURE_PERFRAME	0x04	// send perframe texture data every frame
#endif

//-------------------------------------

#ifndef VPROF_LEVEL
#define VPROF_LEVEL 0
#endif

#define	VPROF_0(name,group,assertAccounted,budgetFlags)	CVProfScope VProf_(name, 0, group, assertAccounted, budgetFlags);

#if VPROF_LEVEL > 0 
#define	VPROF_1(name,group,assertAccounted,budgetFlags)	CVProfScope VProf_(name, 1, group, assertAccounted, budgetFlags);
#else
#define	VPROF_1(name,group,assertAccounted,budgetFlags)	((void)0)
#endif

#if VPROF_LEVEL > 1 
#define	VPROF_2(name,group,assertAccounted,budgetFlags)	CVProfScope VProf_(name, 2, group, assertAccounted, budgetFlags);
#else
#define	VPROF_2(name,group,assertAccounted,budgetFlags)	((void)0)
#endif

#if VPROF_LEVEL > 2 
#define	VPROF_3(name,group,assertAccounted,budgetFlags)	CVProfScope VProf_(name, 3, group, assertAccounted, budgetFlags);
#else
#define	VPROF_3(name,group,assertAccounted,budgetFlags)	((void)0)
#endif

#if VPROF_LEVEL > 3 
#define	VPROF_4(name,group,assertAccounted,budgetFlags)	CVProfScope VProf_(name, 4, group, assertAccounted, budgetFlags);
#else
#define	VPROF_4(name,group,assertAccounted,budgetFlags)	((void)0)
#endif

//-------------------------------------

#define VPROF_INCREMENT_COUNTER(name,amount)			do { static CVProfCounter _counter( name ); _counter.Increment( amount ); } while( 0 )
#define VPROF_INCREMENT_GROUP_COUNTER(name,group,amount)			do { static CVProfCounter _counter( name, group ); _counter.Increment( amount ); } while( 0 )

#else

#define	VPROF( name )									((void)0)
#define	VPROF_ASSERT_ACCOUNTED( name )					((void)0)
#define	VPROF_( name, detail, group, bAssertAccounted )	((void)0)
#define VPROF_BUDGET( name, group )						((void)0)
#define VPROF_BUDGET_FLAGS( name, group, flags )		((void)0)

#define VPROF_SCOPE_BEGIN( tag )	do {
#define VPROF_SCOPE_END()			} while (0)

#define VPROF_ONLY( expression )	((void)0)

#define VPROF_ENTER_SCOPE( name )
#define VPROF_EXIT_SCOPE()

#define VPROF_INCREMENT_COUNTER(name,amount)			((void)0)
#define VPROF_INCREMENT_GROUP_COUNTER(name,group,amount) ((void)0)

#endif
 
//-----------------------------------------------------------------------------

#ifdef VPROF_ENABLED

//-----------------------------------------------------------------------------
//
// A node in the call graph hierarchy
//

class DBG_CLASS CVProfNode 
{
friend class CVProfRecorder;
friend class CVProfile;

public:
	CVProfNode( const tchar * pszName, int detailLevel, CVProfNode *pParent, const tchar *pBudgetGroupName, int budgetFlags );
	~CVProfNode();
	
	CVProfNode *GetSubNode( const tchar *pszName, int detailLevel, const tchar *pBudgetGroupName, int budgetFlags );
	CVProfNode *GetSubNode( const tchar *pszName, int detailLevel, const tchar *pBudgetGroupName );
	CVProfNode *GetParent();
	CVProfNode *GetSibling();		
	CVProfNode *GetPrevSibling();	
	CVProfNode *GetChild();		
	
	void MarkFrame();
	void ResetPeak();
	
	void Pause();
	void Resume();
	void Reset();

	void EnterScope();
	bool ExitScope();

	const tchar *GetName();

	int GetBudgetGroupID()
	{
		return m_BudgetGroupID;
	}

	// Only used by the record/playback stuff.
	void SetBudgetGroupID( int id )
	{
		m_BudgetGroupID = id;
	}

	int	GetCurCalls();
	double GetCurTime();		
	int GetPrevCalls();
	double GetPrevTime();
	int	GetTotalCalls();
	double GetTotalTime();		
	double GetPeakTime();		

	double GetCurTimeLessChildren();
	double GetPrevTimeLessChildren();
	double GetTotalTimeLessChildren();

	void ClearPrevTime();

	int GetL2CacheMisses();

	// Not used in the common case...
	void SetCurFrameTime( unsigned long milliseconds );
	
	void SetClientData( int iClientData )	{ m_iClientData = iClientData; }
	int GetClientData() const				{ return m_iClientData; }

#ifdef DBGFLAG_VALIDATE
	void Validate( CValidator &validator, tchar *pchName );		// Validate our internal structures
#endif // DBGFLAG_VALIDATE


// Used by vprof record/playback.
private:

	void SetUniqueNodeID( int id )
	{
		m_iUniqueNodeID = id;
	}

	int GetUniqueNodeID() const
	{
		return m_iUniqueNodeID;
	}

	static int s_iCurrentUniqueNodeID;


private:
	const tchar *m_pszName;
	CFastTimer	m_Timer;

#ifndef _XBOX	
	// L2 Cache data.
	CL2Cache	m_L2Cache;
	int			m_iCurL2CacheMiss;
	int			m_iTotalL2CacheMiss;
#endif

	int			m_nRecursions;
	
	unsigned	m_nCurFrameCalls;
	CCycleCount	m_CurFrameTime;
	
	unsigned	m_nPrevFrameCalls;
	CCycleCount	m_PrevFrameTime;

	unsigned	m_nTotalCalls;
	CCycleCount	m_TotalTime;

	CCycleCount	m_PeakTime;

	CVProfNode *m_pParent;
	CVProfNode *m_pChild;
	CVProfNode *m_pSibling;

	int m_BudgetGroupID;
	
	int m_iClientData;
	int m_iUniqueNodeID;
	
#if !defined(_WIN32) || defined(_XBOX)
	void *operator new( size_t );
	void operator delete( void * );
#endif

};

//-----------------------------------------------------------------------------
//
// Coordinator and root node of the profile hierarchy tree
//

enum VProfReportType_t
{
	VPRT_SUMMARY									= ( 1 << 0 ),
	VPRT_HIERARCHY									= ( 1 << 1 ),
	VPRT_HIERARCHY_TIME_PER_FRAME_AND_COUNT_ONLY	= ( 1 << 2 ),
	VPRT_LIST_BY_TIME								= ( 1 << 3 ),
	VPRT_LIST_BY_TIME_LESS_CHILDREN					= ( 1 << 4 ),
	VPRT_LIST_BY_AVG_TIME							= ( 1 << 5 ),	
	VPRT_LIST_BY_AVG_TIME_LESS_CHILDREN				= ( 1 << 6 ),
	VPRT_LIST_BY_PEAK_TIME							= ( 1 << 7 ),
	VPRT_LIST_BY_PEAK_OVER_AVERAGE					= ( 1 << 8 ),
	VPRT_LIST_TOP_ITEMS_ONLY						= ( 1 << 9 ),

	VPRT_FULL = (0xffffffff & ~(VPRT_HIERARCHY_TIME_PER_FRAME_AND_COUNT_ONLY|VPRT_LIST_TOP_ITEMS_ONLY)),
};

enum CounterGroup_t
{
	COUNTER_GROUP_DEFAULT=0,
	COUNTER_GROUP_NO_RESET,				// The engine doesn't reset these counters. Usually, they are used 
										// like global variables that can be accessed across modules.
	COUNTER_GROUP_TEXTURE_GLOBAL,		// Global texture usage counters (totals for what is currently in memory).
	COUNTER_GROUP_TEXTURE_PER_FRAME		// Per-frame texture usage counters.
}; 

class DBG_CLASS CVProfile 
{
public:
	CVProfile();
	~CVProfile();

	void Term();
	
	//
	// Runtime operations
	//
	
	void Start();
	void Stop();

#ifdef _XBOX
	// piggyback to profiler
	void VXProfileStart();
	void VXProfileUpdate();
	void VXEnableUpdateMode(int event, bool bEnable);
#endif

	void EnterScope( const tchar *pszName, int detailLevel, const tchar *pBudgetGroupName, bool bAssertAccounted );
	void EnterScope( const tchar *pszName, int detailLevel, const tchar *pBudgetGroupName, bool bAssertAccounted, int budgetFlags );
	void ExitScope();

	void MarkFrame();
	void ResetPeaks();
	
	void Pause();
	void Resume();
	void Reset();
	
	bool IsEnabled() const;
	int GetDetailLevel() const;

	bool AtRoot() const;

	//
	// Queries
	//

#ifdef VPROF_VTUNE_GROUP
#	define MAX_GROUP_STACK_DEPTH 1024

	void EnableVTuneGroup( const tchar *pGroupName )
	{
		m_nVTuneGroupID = BudgetGroupNameToBudgetGroupID( pGroupName );
		m_bVTuneGroupEnabled = true;
	}
	void DisableVTuneGroup( void )
	{
		m_bVTuneGroupEnabled = false;
	}
	
	inline void PushGroup( int nGroupID );
	inline void PopGroup( void );
#endif
	
	int NumFramesSampled()	{ return m_nFrames; }
	double GetPeakFrameTime();
	double GetTotalTimeSampled();
	double GetTimeLastFrame();
	
	CVProfNode *GetRoot();
	CVProfNode *FindNode( CVProfNode *pStartNode, const tchar *pszNode );

	void OutputReport( int type = VPRT_FULL, const tchar *pszStartNode = NULL, int budgetGroupID = -1 );

	const tchar *GetBudgetGroupName( int budgetGroupID );
	int GetBudgetGroupFlags( int budgetGroupID ) const;	// Returns a combination of BUDGETFLAG_ defines.
	int GetNumBudgetGroups( void );
	void GetBudgetGroupColor( int budgetGroupID, int &r, int &g, int &b, int &a );
	int BudgetGroupNameToBudgetGroupID( const tchar *pBudgetGroupName );
	int BudgetGroupNameToBudgetGroupID( const tchar *pBudgetGroupName, int budgetFlagsToORIn );
	void RegisterNumBudgetGroupsChangedCallBack( void (*pCallBack)(void) );

	int *FindOrCreateCounter( const tchar *pName, CounterGroup_t eCounterGroup=COUNTER_GROUP_DEFAULT  );
	void ResetCounters( CounterGroup_t eCounterGroup );
	
	int GetNumCounters( void ) const;
	
	const tchar *GetCounterName( int index ) const;
	int GetCounterValue( int index ) const;
	const tchar *GetCounterNameAndValue( int index, int &val ) const;
	CounterGroup_t GetCounterGroup( int index ) const;

#ifndef _XBOX
	// Performance monitoring events.
	void PMEInitialized( bool bInit )		{ m_bPMEInit = bInit; }
	void PMEEnable( bool bEnable )			{ m_bPMEEnabled = bEnable; }
	bool UsePME( void )						{ return ( m_bPMEInit && m_bPMEEnabled ); }
#endif

#ifdef DBGFLAG_VALIDATE
	void Validate( CValidator &validator, tchar *pchName );		// Validate our internal structures
#endif // DBGFLAG_VALIDATE

protected:

	void FreeNodes_R( CVProfNode *pNode );

#ifdef VPROF_VTUNE_GROUP
	bool VTuneGroupEnabled()
	{ 
		return m_bVTuneGroupEnabled; 
	}
	int VTuneGroupID() 
	{ 
		return m_nVTuneGroupID; 
	}
#endif

	void SumTimes( const tchar *pszStartNode, int budgetGroupID );
	void SumTimes( CVProfNode *pNode, int budgetGroupID );
	void DumpNodes( CVProfNode *pNode, int indent, bool bAverageAndCountOnly );
	int FindBudgetGroupName( const tchar *pBudgetGroupName );
	int AddBudgetGroupName( const tchar *pBudgetGroupName, int budgetFlags );

#ifdef VPROF_VTUNE_GROUP
	bool		m_bVTuneGroupEnabled;
	int			m_nVTuneGroupID;
	int			m_GroupIDStack[MAX_GROUP_STACK_DEPTH];
	int			m_GroupIDStackDepth;
#endif
	int 		m_enabled;
	bool		m_fAtRoot; // tracked for efficiency of the "not profiling" case
	CVProfNode *m_pCurNode;
	CVProfNode	m_Root;
	int			m_nFrames;
	int			m_ProfileDetailLevel;
	int			m_pausedEnabledDepth;
	
	class CBudgetGroup
	{
	public:
		tchar *m_pName;
		int m_BudgetFlags;
	};
	
	CBudgetGroup	*m_pBudgetGroups;
	int			m_nBudgetGroupNamesAllocated;
	int			m_nBudgetGroupNames;
	void		(*m_pNumBudgetGroupsChangedCallBack)(void);

#ifndef _XBOX
	// Performance monitoring events.
	bool		m_bPMEInit;
	bool		m_bPMEEnabled;
#endif

	int m_Counters[MAXCOUNTERS];
	char m_CounterGroups[MAXCOUNTERS]; // (These are CounterGroup_t's).
	tchar *m_CounterNames[MAXCOUNTERS];
	int m_NumCounters;

#ifdef _XBOX
	int m_UpdateMode;
#endif
};

//-------------------------------------

DBG_INTERFACE CVProfile g_VProfCurrentProfile;

//-----------------------------------------------------------------------------

#ifdef VPROF_VTUNE_GROUP
inline void CVProfile::PushGroup( int nGroupID )
{
	// There is always at least one item on the stack since we force 
	// the first element to be VPROF_BUDGETGROUP_OTHER_UNACCOUNTED.
	Assert( m_GroupIDStackDepth > 0 );
	Assert( m_GroupIDStackDepth < MAX_GROUP_STACK_DEPTH );
	m_GroupIDStack[m_GroupIDStackDepth] = nGroupID;
	m_GroupIDStackDepth++;
	if( m_GroupIDStack[m_GroupIDStackDepth-2] != nGroupID && 
		VTuneGroupEnabled() &&
		nGroupID == VTuneGroupID() )
	{
		vtune( true );
	}
}
#endif // VPROF_VTUNE_GROUP

#ifdef VPROF_VTUNE_GROUP
inline void CVProfile::PopGroup( void )
{
	m_GroupIDStackDepth--;
	// There is always at least one item on the stack since we force 
	// the first element to be VPROF_BUDGETGROUP_OTHER_UNACCOUNTED.
	Assert( m_GroupIDStackDepth > 0 );
	if(	m_GroupIDStack[m_GroupIDStackDepth] != m_GroupIDStack[m_GroupIDStackDepth+1] && 
		VTuneGroupEnabled() &&
		m_GroupIDStack[m_GroupIDStackDepth+1] == VTuneGroupID() )
	{
		vtune( false );
	}
}
#endif // VPROF_VTUNE_GROUP

//-----------------------------------------------------------------------------

class CVProfScope
{
public:
	CVProfScope( const tchar * pszName, int detailLevel, const tchar *pBudgetGroupName, bool bAssertAccounted, int budgetFlags );
	~CVProfScope();
};

//-----------------------------------------------------------------------------
//
// CVProfNode, inline methods
//

inline CVProfNode::CVProfNode( const tchar * pszName, int detailLevel, CVProfNode *pParent, const tchar *pBudgetGroupName, int budgetFlags )
 :	m_pszName( pszName ),
	m_nCurFrameCalls( 0 ),
	m_nPrevFrameCalls( 0 ),
	m_nRecursions( 0 ),
	m_pParent( pParent ),
	m_pChild( NULL ),
	m_pSibling( NULL ),
	m_iClientData( -1 )
{
	m_iUniqueNodeID = s_iCurrentUniqueNodeID++;

	if ( m_iUniqueNodeID > 0 )
	{
		m_BudgetGroupID = g_VProfCurrentProfile.BudgetGroupNameToBudgetGroupID( pBudgetGroupName, budgetFlags );
	}
	else
	{
		m_BudgetGroupID = 0; // "m_Root" can't call BudgetGroupNameToBudgetGroupID because g_VProfCurrentProfile not yet initialized
	}

	Reset();

	if( m_pParent && ( m_BudgetGroupID == VPROF_BUDGET_GROUP_ID_UNACCOUNTED ) )
	{
		m_BudgetGroupID = m_pParent->GetBudgetGroupID();
	}
}

//-------------------------------------

inline CVProfNode::~CVProfNode()
{
#ifndef _WIN32
	delete m_pChild;
	delete m_pSibling;
#endif
}

//-------------------------------------

inline CVProfNode *CVProfNode::GetParent()		
{ 
	Assert( m_pParent );
	return m_pParent; 
}

//-------------------------------------

inline CVProfNode *CVProfNode::GetSibling()		
{ 
	return m_pSibling; 
}

//-------------------------------------
// Hacky way to the previous sibling, only used from vprof panel at the moment,
// so it didn't seem like it was worth the memory waste to add the reverse
// link per node.

inline CVProfNode *CVProfNode::GetPrevSibling()		
{ 
	CVProfNode* p = GetParent();

	if(!p) 
		return NULL;

	CVProfNode* s;
	for( s = p->GetChild(); 
	     s && ( s->GetSibling() != this ); 
		 s = s->GetSibling() )
		;

	return s;	
}

//-------------------------------------

inline CVProfNode *CVProfNode::GetChild()			
{ 
	return m_pChild; 
}

//-------------------------------------

inline const tchar *CVProfNode::GetName()				
{ 
	Assert( m_pszName );
	return m_pszName; 
}

//-------------------------------------

inline int	CVProfNode::GetTotalCalls()		
{ 
	return m_nTotalCalls; 
}

//-------------------------------------

inline double CVProfNode::GetTotalTime()		
{ 
	return m_TotalTime.GetMillisecondsF();
}

//-------------------------------------

inline int	CVProfNode::GetCurCalls()		
{ 
	return m_nCurFrameCalls; 
}

//-------------------------------------

inline double CVProfNode::GetCurTime()		
{ 
	return m_CurFrameTime.GetMillisecondsF();
}

//-------------------------------------

inline int CVProfNode::GetPrevCalls()
{
	return m_nPrevFrameCalls;
}

//-------------------------------------

inline double CVProfNode::GetPrevTime()		
{ 
	return m_PrevFrameTime.GetMillisecondsF();
}

//-------------------------------------

inline double CVProfNode::GetPeakTime()		
{ 
	return m_PeakTime.GetMillisecondsF();
}

//-------------------------------------

inline double CVProfNode::GetTotalTimeLessChildren()
{
	double result = GetTotalTime();
	CVProfNode *pChild = GetChild();
	while ( pChild )
	{
		result -= pChild->GetTotalTime();
		pChild = pChild->GetSibling();
	}
	return result;
}

//-------------------------------------

inline double CVProfNode::GetCurTimeLessChildren()
{
	double result = GetCurTime();
	CVProfNode *pChild = GetChild();
	while ( pChild )
	{
		result -= pChild->GetCurTime();
		pChild = pChild->GetSibling();
	}
	return result;
}


//-----------------------------------------------------------------------------
inline double CVProfNode::GetPrevTimeLessChildren()
{
	double result = GetPrevTime();
	CVProfNode *pChild = GetChild();
	while ( pChild )
	{
		result -= pChild->GetPrevTime();
		pChild = pChild->GetSibling();
	}
	return result;
}

//-----------------------------------------------------------------------------
inline void CVProfNode::ClearPrevTime()
{
	m_PrevFrameTime.Init();
}

//-----------------------------------------------------------------------------
inline int CVProfNode::GetL2CacheMisses( void )
{ 
#ifndef _XBOX
	return m_L2Cache.GetL2CacheMisses(); 
#else
	return 0;
#endif
}

//-----------------------------------------------------------------------------
//
// CVProfile, inline methods
//

//-------------------------------------

inline bool CVProfile::IsEnabled() const	
{ 
	return ( m_enabled != 0 ); 
}

//-------------------------------------

inline int CVProfile::GetDetailLevel() const	
{ 
	return m_ProfileDetailLevel; 
}

	
//-------------------------------------

inline bool CVProfile::AtRoot() const
{
	return m_fAtRoot;
}
	
//-------------------------------------

inline void CVProfile::Start()	
{ 
	if ( ++m_enabled == 1 )
	{
		m_Root.EnterScope();
#ifdef _XBOX
		VXProfileStart();
#endif
	}
}

//-------------------------------------

inline void CVProfile::Stop()		
{ 
	if ( --m_enabled == 0 )
		m_Root.ExitScope();
}

//-------------------------------------

inline void CVProfile::EnterScope( const tchar *pszName, int detailLevel, const tchar *pBudgetGroupName, bool bAssertAccounted, int budgetFlags )
{
	if ( ( m_enabled != 0 || !m_fAtRoot ) && ThreadInMainThread() ) // if became disabled, need to unwind back to root before stopping
	{
		// Only account for vprof stuff on the primary thread.
		//if( !Plat_IsPrimaryThread() )
		//	return;

		if ( pszName != m_pCurNode->GetName() ) 
		{
			m_pCurNode = m_pCurNode->GetSubNode( pszName, detailLevel, pBudgetGroupName, budgetFlags );
		}
		m_pBudgetGroups[m_pCurNode->GetBudgetGroupID()].m_BudgetFlags |= budgetFlags;
#ifdef _DEBUG
		if( bAssertAccounted )
		{
			// FIXME
			AssertOnce( m_pCurNode->GetBudgetGroupID() != 0 );
		}
#endif
		m_pCurNode->EnterScope();
		m_fAtRoot = false;
	}
}

inline void CVProfile::EnterScope( const tchar *pszName, int detailLevel, const tchar *pBudgetGroupName, bool bAssertAccounted )
{
	EnterScope( pszName, detailLevel, pBudgetGroupName, bAssertAccounted, BUDGETFLAG_OTHER );
}

//-------------------------------------

inline void CVProfile::ExitScope()
{
	if ( ( !m_fAtRoot || m_enabled != 0 ) && ThreadInMainThread() )
	{
		// Only account for vprof stuff on the primary thread.
		//if( !Plat_IsPrimaryThread() )
		//	return;

		// ExitScope will indicate whether we should back up to our parent (we may
		// be profiling a recursive function)
		if (m_pCurNode->ExitScope()) 
		{
			m_pCurNode = m_pCurNode->GetParent();
		}
		m_fAtRoot = ( m_pCurNode == &m_Root );
	}
}

//-------------------------------------

inline void CVProfile::Pause()
{
	m_pausedEnabledDepth = m_enabled;
	m_enabled = 0;
	if ( !AtRoot() )
		m_Root.Pause(); 
}

//-------------------------------------

inline void CVProfile::Resume()
{
	m_enabled = m_pausedEnabledDepth;
	if ( !AtRoot() )
		m_Root.Resume(); 
}

//-------------------------------------

inline void CVProfile::Reset()
{
	m_Root.Reset(); 
	m_nFrames = 0;
}

//-------------------------------------

inline void CVProfile::ResetPeaks()
{
	m_Root.ResetPeak(); 
}

//-------------------------------------

inline void CVProfile::MarkFrame()
{
	if ( m_enabled )
	{
		++m_nFrames;
		m_Root.ExitScope();
		m_Root.MarkFrame(); 
		m_Root.EnterScope();
	}
}

//-------------------------------------

inline double CVProfile::GetTotalTimeSampled()
{
	return m_Root.GetTotalTime();
}

//-------------------------------------

inline double CVProfile::GetPeakFrameTime()
{
	return m_Root.GetPeakTime();
}

//-------------------------------------

inline double CVProfile::GetTimeLastFrame()
{
	return m_Root.GetCurTime();
}
	
//-------------------------------------

inline CVProfNode *CVProfile::GetRoot()
{
	return &m_Root;
}


inline const tchar *CVProfile::GetBudgetGroupName( int budgetGroupID )
{
	Assert( budgetGroupID >= 0 && budgetGroupID < m_nBudgetGroupNames );
	return m_pBudgetGroups[budgetGroupID].m_pName;
}

inline int CVProfile::GetBudgetGroupFlags( int budgetGroupID ) const
{
	Assert( budgetGroupID >= 0 && budgetGroupID < m_nBudgetGroupNames );
	return m_pBudgetGroups[budgetGroupID].m_BudgetFlags;
}


//-----------------------------------------------------------------------------

inline CVProfScope::CVProfScope( const tchar * pszName, int detailLevel, const tchar *pBudgetGroupName, bool bAssertAccounted, int budgetFlags )
{ 
	g_VProfCurrentProfile.EnterScope( pszName, detailLevel, pBudgetGroupName, bAssertAccounted, budgetFlags ); 
}

//-------------------------------------

inline CVProfScope::~CVProfScope()					
{ 
	g_VProfCurrentProfile.ExitScope(); 
}

class CVProfCounter
{
public:
	CVProfCounter( const tchar *pName, CounterGroup_t group=COUNTER_GROUP_DEFAULT )
	{
		m_pCounter = g_VProfCurrentProfile.FindOrCreateCounter( pName, group );
		Assert( m_pCounter );
	}
	~CVProfCounter()
	{
	}
	void Increment( int val ) 
	{ 
		Assert( m_pCounter );
		*m_pCounter += val; 
	}
private:
	int *m_pCounter;
};

#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif

//=============================================================================
