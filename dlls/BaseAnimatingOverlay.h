//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

// #include "BaseAnimating.h"

class CAnimationLayer
{
public:	
	CAnimationLayer( void );
	void	Init( );

	// float	SetBlending( int iBlender, float flValue, CBaseAnimating *pOwner );
	void	StudioFrameAdvance( float flInterval, CBaseAnimating *pOwner );
	void	DispatchAnimEvents( CBaseAnimating *eventHandler, CBaseAnimating *pOwner );
	void	SetOrder( int nOrder );

public:	

#define ANIM_LAYER_ACTIVE		0x0001
#define ANIM_LAYER_AUTOKILL		0x0002
#define ANIM_LAYER_KILLME		0x0004
#define ANIM_LAYER_DONTRESTORE	0x0008
#define ANIM_LAYER_CHECKACCESS	0x0010
#define ANIM_LAYER_DYING		0x0020

	int		m_fFlags;

	bool	m_bSequenceFinished;
	bool	m_bLooping;
	int		m_nSequence;
	float	m_flCycle;
	float	m_flPrevCycle;
	float	m_flPlaybackRate;
	float	m_flWeight;

	float	m_flBlendIn; // start and end blend frac (0.0 for now blend)
	float	m_flBlendOut; 

	float	m_flKillRate;
	float	m_flKillDelay;

	// For checking for duplicates
	Activity	m_nActivity;

	// order of layering on client
	int		m_nPriority;
	int		m_nOrder;

	bool	IsActive( void ) { return ((m_fFlags & ANIM_LAYER_ACTIVE) != 0); }
	bool	IsAutokill( void ) { return ((m_fFlags & ANIM_LAYER_AUTOKILL) != 0); }
	bool	IsKillMe( void ) { return ((m_fFlags & ANIM_LAYER_KILLME) != 0); }
	bool	IsAutoramp( void ) { return (m_flBlendIn != 0.0 || m_flBlendOut != 0.0); }
	void	KillMe( void ) { m_fFlags |= ANIM_LAYER_KILLME; }
	void	Dying( void ) { m_fFlags |= ANIM_LAYER_DYING; }
	bool	IsDying( void ) { return ((m_fFlags & ANIM_LAYER_DYING) != 0); }
	void	Dead( void ) { m_fFlags &= ~ANIM_LAYER_DYING; }

	bool	IsAbandoned( void );
	void	MarkActive( void );

	float	m_flLastEventCheck;

	float	m_flLastAccess;

	DECLARE_SIMPLE_DATADESC();
};

inline void CAnimationLayer::SetOrder( int nOrder )
{
	m_nOrder = nOrder;
}



class CBaseAnimatingOverlay : public CBaseAnimating
{
	DECLARE_CLASS( CBaseAnimatingOverlay, CBaseAnimating );

public:
	enum 
	{
		MAX_OVERLAYS = 15,
	};

private:
	CUtlVector< CAnimationLayer	> m_AnimOverlay;
	//int				m_nActiveLayers;
	//int				m_nActiveBaseLayers;

public:
	
	virtual void	OnRestore();

	virtual void	StudioFrameAdvance();
	virtual	void	DispatchAnimEvents ( CBaseAnimating *eventHandler );
	virtual void	GetSkeleton( Vector pos[], Quaternion q[], int boneMask );

	int		AddGestureSequence( int sequence, bool autokill = true );
	int		AddGestureSequence( int sequence, float flDuration, bool autokill = true );
	int		AddGesture( Activity activity, bool autokill = true );
	int		AddGesture( Activity activity, float flDuration, bool autokill = true );
	bool	IsPlayingGesture( Activity activity );
	void	RestartGesture( Activity activity, bool addifmissing = true, bool autokill = true );
	void	RemoveGesture( Activity activity );
	void	RemoveAllGestures( void );

	int		AddLayeredSequence( int sequence, int iPriority );

	void	SetLayerPriority( int iLayer, int iPriority );

	bool	IsValidLayer( int iLayer );

	void	SetLayerDuration( int iLayer, float flDuration );
	float	GetLayerDuration( int iLayer );

	void	SetLayerCycle( int iLayer, float flCycle );
	void	SetLayerCycle( int iLayer, float flCycle, float flPrevCycle );
	float	GetLayerCycle( int iLayer );

	void	SetLayerPlaybackRate( int iLayer, float flPlaybackRate );
	void	SetLayerWeight( int iLayer, float flWeight );
	float	GetLayerWeight( int iLayer );
	void	SetLayerBlendIn( int iLayer, float flBlendIn );
	void	SetLayerBlendOut( int iLayer, float flBlendOut );
	void	SetLayerAutokill( int iLayer, bool bAutokill );
	void	SetLayerLooping( int iLayer, bool bLooping );
	void	SetLayerNoRestore( int iLayer, bool bNoRestore );

	Activity	GetLayerActivity( int iLayer );
	int			GetLayerSequence( int iLayer );
	
	int		FindGestureLayer( Activity activity );

	void	RemoveLayer( int iLayer, float flKillRate = 0.2, float flKillDelay = 0.0 );
	void	FastRemoveLayer( int iLayer );

	CAnimationLayer *GetAnimOverlay( int iIndex );
	int GetNumAnimOverlays() const;
	void SetNumAnimOverlays( int num );

	void VerifyOrder( void );

private:
	int		AllocateLayer( int iPriority = 0 ); // lower priorities are processed first

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_PREDICTABLE();
};

EXTERN_SEND_TABLE(DT_BaseAnimatingOverlay);

inline int CBaseAnimatingOverlay::GetNumAnimOverlays() const
{
	return m_AnimOverlay.Count();
}
