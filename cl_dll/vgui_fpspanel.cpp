//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ifpspanel.h"
#include <vgui_controls/Panel.h>
#include "view.h"
#include <vgui/IVGui.h>
#include "vguimatsurface/imatsystemsurface.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/IPanel.h>
#include "materialsystem/imaterialsystemhardwareconfig.h"
#ifdef _XBOX
#include "xbox/xbox_platform.h"
#include "xbox/xbox_win32stubs.h"
#include "xbox/xbox_core.h"
#endif
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar cl_showfps( "cl_showfps", "0", 0, "Draw fps meter at top of screen (1 = fps, 2 = smooth fps)" );
static ConVar cl_showpos( "cl_showpos", "0", 0, "Draw current position at top of screen (view angles only show with sv_cheats 1)" );

//-----------------------------------------------------------------------------
// Purpose: Framerate indicator panel
//-----------------------------------------------------------------------------
class CFPSPanel : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CFPSPanel, vgui::Panel );

public:
	CFPSPanel( vgui::VPANEL parent );
	virtual			~CFPSPanel( void );

	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void	Paint();
	virtual void	OnTick( void );

	virtual bool	ShouldDraw( void );

protected:
	MESSAGE_FUNC_INT_INT( OnScreenSizeChanged, "OnScreenSizeChanged", oldwide, oldtall );

private:
	void ComputeSize( void );

	vgui::HFont		m_hFont;
};

#define FPS_PANEL_WIDTH 300

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *parent - 
//-----------------------------------------------------------------------------
CFPSPanel::CFPSPanel( vgui::VPANEL parent ) : BaseClass( NULL, "CFPSPanel" )
{
	SetParent( parent );
	SetVisible( false );
	SetCursor( null );

	SetFgColor( Color( 0, 0, 0, 255 ) );
	SetPaintBackgroundEnabled( false );

	m_hFont = 0;

#if defined( _XBOX )
	if ( IsRetail() && XBX_NoXBDM() )
		return;
#endif
	ComputeSize();

	vgui::ivgui()->AddTickSignal( GetVPanel(), 250 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CFPSPanel::~CFPSPanel( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Updates panel to handle the new screen size
//-----------------------------------------------------------------------------
void CFPSPanel::OnScreenSizeChanged(int iOldWide, int iOldTall)
{
	BaseClass::OnScreenSizeChanged(iOldWide, iOldTall);
	ComputeSize();
}

//-----------------------------------------------------------------------------
// Purpose: Computes panel's desired size and position
//-----------------------------------------------------------------------------
void CFPSPanel::ComputeSize( void )
{
	int wide, tall;
	vgui::ipanel()->GetSize(GetVParent(), wide, tall );
#ifndef _XBOX
	SetPos( wide - FPS_PANEL_WIDTH, 0 );
#else
	SetPos( wide - FPS_PANEL_WIDTH - XBX_MAXBORDERSAFE, XBX_MAXBORDERSAFE );
#endif
	SetSize( FPS_PANEL_WIDTH, vgui::surface()->GetFontTall( m_hFont ) + 10 );
}

void CFPSPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_hFont = pScheme->GetFont( "DefaultFixedOutline" );
	Assert( m_hFont );

	ComputeSize();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFPSPanel::OnTick( void )
{
	SetVisible( ShouldDraw() );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CFPSPanel::ShouldDraw( void )
{
	if ( ( !cl_showfps.GetInt() || ( gpGlobals->absoluteframetime <= 0 ) ) &&
		 ( !cl_showpos.GetInt() ) )
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void GetFPSColor( int nFps, unsigned char ucColor[3] )
{
#ifndef _XBOX
	ucColor[0] = 255; ucColor[1] = 0; ucColor[2] = 0;

	if ( g_pMaterialSystemHardwareConfig->GetDXSupportLevel() >= 90 )
	{
		if ( nFps >= 60 )
		{
			ucColor[0] = 0; 
			ucColor[1] = 255;
		}
		else if ( nFps >= 50 )
		{
			ucColor[1] = 255;
		}
	}
	else if ( g_pMaterialSystemHardwareConfig->GetDXSupportLevel() >= 80 )
	{
		if ( nFps >= 30 )
		{
			ucColor[0] = 0; 
			ucColor[1] = 255;
		}
		else if ( nFps >= 25 )
		{
			ucColor[1] = 255;
		}
	}
	else
	{
		if ( nFps >= 20 )
		{
			ucColor[0] = 0; 
			ucColor[1] = 255;
		}
		else if ( nFps >= 15 )
		{
			ucColor[1] = 255;
		}
	}
#else
	ucColor[0] = 255;
	ucColor[1] = 255;
	ucColor[2] = 255;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : 
//-----------------------------------------------------------------------------
void CFPSPanel::Paint() 
{
	int i = 0;
	int x = 2;
	static float lastRealTime = -1.0f;

	float realFrameTime = gpGlobals->realtime - lastRealTime;

	if ( cl_showfps.GetInt() && realFrameTime > 0.0 )
	{
		if ( lastRealTime != -1.0f )
		{
#ifdef _XBOX
			g_pMatSystemSurface->DrawSetColor( 0, 0, 0, 255 );
			g_pMatSystemSurface->DrawFilledRect( 0, 0, FPS_PANEL_WIDTH, 4+vgui::surface()->GetFontTall( m_hFont ) );
#endif
			i++;
			static float AverageFPS = -1;
			static int high = -1;
			static int low = -1;
			int nFps = -1;
			unsigned char ucColor[3];
			if ( cl_showfps.GetInt() == 2 )
			{
				const float NewWeight  = 0.1f;
				float NewFrame = 1.0f / realFrameTime;

				if ( AverageFPS < 0.0f )
				{
					AverageFPS = NewFrame;
					high = (int)AverageFPS;
					low = (int)AverageFPS;
				} 
				else
				{				
					AverageFPS *= ( 1.0f - NewWeight ) ;
					AverageFPS += ( ( NewFrame ) * NewWeight );
				}
			
				int NewFrameInt = (int)NewFrame;
				if( NewFrameInt < low ) low = NewFrameInt;
				if( NewFrameInt > high ) high = NewFrameInt;	

				nFps = static_cast<int>( AverageFPS );
				GetFPSColor( nFps, ucColor );
				g_pMatSystemSurface->DrawColoredText( m_hFont, x, 2, ucColor[0], ucColor[1], ucColor[2], 255, "%3i fps (%3i, %3i) on %s", nFps, low, high, engine->GetLevelName() );
			} 
			else
			{
				AverageFPS = -1;
				nFps = static_cast<int>( 1.0f / realFrameTime );
				GetFPSColor( nFps, ucColor );
				g_pMatSystemSurface->DrawColoredText( m_hFont, x, 2, ucColor[0], ucColor[1], ucColor[2], 255, "%3i fps on %s", nFps, engine->GetLevelName() );
			}
		}
	}
	lastRealTime = gpGlobals->realtime;

	if ( cl_showpos.GetInt() )
	{
		g_pMatSystemSurface->DrawColoredText( m_hFont, x, 2+ i * ( vgui::surface()->GetFontTall( m_hFont ) + 2 ), 
			255, 255, 255, 255, 
			"pos:  %.2f %.2f %.2f", 
			MainViewOrigin().x, MainViewOrigin().y, MainViewOrigin().z );
		i++;

		// only draw angle if cheats is on
		if ( sv_cheats && sv_cheats->GetBool() )
		{
			g_pMatSystemSurface->DrawColoredText( m_hFont, x, 2 + i * ( vgui::surface()->GetFontTall( m_hFont ) + 2 ), 
				255, 255, 255, 255, 
				"ang:  %.2f %.2f %.2f", 
				MainViewAngles().x, MainViewAngles().y, MainViewAngles().z );
			i++;
		}

		Vector vel( 0, 0, 0 );
		C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
		if ( player )
		{
			vel = player->GetLocalVelocity();
		}

		g_pMatSystemSurface->DrawColoredText( m_hFont, x, 2 + i * ( vgui::surface()->GetFontTall( m_hFont ) + 2 ), 
			255, 255, 255, 255, 
			"vel:  %.2f", 
			vel.Length() );
	}
}

class CFPS : public IFPSPanel
{
private:
	CFPSPanel *fpsPanel;
public:
	CFPS( void )
	{
		fpsPanel = NULL;
	}

	void Create( vgui::VPANEL parent )
	{
		fpsPanel = new CFPSPanel( parent );
	}

	void Destroy( void )
	{
		if ( fpsPanel )
		{
			fpsPanel->SetParent( (vgui::Panel *)NULL );
			delete fpsPanel;
		}
	}
};

static CFPS g_FPSPanel;
IFPSPanel *fps = ( IFPSPanel * )&g_FPSPanel;

#if defined( TRACK_BLOCKING_IO ) && !defined( _RETAIL )

static ConVar cl_blocking_threshold( "cl_blocking_threshold", "0.000", 0, "If file ops take more than this amount of time, add to 'spewblocking' history list" );

void ShowBlockingChanged( ConVar *var, char const *pOldString )
{
	filesystem->EnableBlockingFileAccessTracking( var->GetBool() );
}

static ConVar cl_showblocking( "cl_showblocking", "0", 0, "Show blocking i/o on top of fps panel", ShowBlockingChanged );
static ConVar cl_blocking_recentsize( "cl_blocking_recentsize", "40", 0, "Number of items to store in recent spew history." );

//-----------------------------------------------------------------------------
// Purpose: blocking i/o indicator
//-----------------------------------------------------------------------------
class CBlockingFileIOPanel : public vgui::Panel
{
	typedef vgui::Panel BaseClass;
public:
	CBlockingFileIOPanel( vgui::VPANEL parent );
	virtual			~CBlockingFileIOPanel( void );

	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void	Paint();
	virtual void	OnTick( void );

	virtual bool	ShouldDraw( void );

	void			SpewRecent();

private:
	void			DrawIOTime( int x, int y, int w, int h, int slot, char const *label, const Color& clr );

	vgui::HFont		m_hFont;

	struct Graph_t
	{
		float			m_flCurrent;

		float			m_flHistory;
		float			m_flHistorySpike;
		float			m_flLatchTime;
		CUtlSymbol		m_LastFile;
	};

	Graph_t			m_History[ FILESYSTEM_BLOCKING_NUMBINS ];

	struct RecentPeaks_t
	{
		float		time;
		CUtlSymbol	fileName;
		float		elapsed;
		byte		reason;
		byte		ioType;
	};

	CUtlLinkedList< RecentPeaks_t, unsigned short >	m_Recent;

	void			SpewItem( const RecentPeaks_t& item );
};

#define IO_PANEL_WIDTH		400
#define IO_DECAY_FRAC		0.95f

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *parent - 
//-----------------------------------------------------------------------------
CBlockingFileIOPanel::CBlockingFileIOPanel( vgui::VPANEL parent ) : BaseClass( NULL, "CBlockingFileIOPanel" )
{
	SetParent( parent );
	int wide, tall;
	vgui::ipanel()->GetSize( parent, wide, tall );
#ifndef _XBOX
	SetPos( 2, 100 );
#else
	SetPos( XBX_MAXBORDERSAFE, XBX_MAXBORDERSAFE + 100 );
#endif
	SetSize( IO_PANEL_WIDTH, 140 );

	SetVisible( false );
	SetCursor( null );

	SetFgColor( Color( 0, 0, 0, 255 ) );
	SetPaintBackgroundEnabled( false );

	m_hFont = 0;

	vgui::ivgui()->AddTickSignal( GetVPanel(), 250 );
	SetZPos( 1000 );
	Q_memset( m_History, 0, sizeof( m_History ) );
	SetPaintBackgroundEnabled( false );
	SetPaintBorderEnabled( false );
	MakePopup();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBlockingFileIOPanel::~CBlockingFileIOPanel( void )
{
}

void CBlockingFileIOPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_hFont = pScheme->GetFont( "Default" );
	Assert( m_hFont );

	SetKeyBoardInputEnabled( false );
	SetMouseInputEnabled( false );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBlockingFileIOPanel::OnTick( void )
{
	SetVisible( ShouldDraw() );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBlockingFileIOPanel::ShouldDraw( void )
{
	if ( !cl_showblocking.GetInt() )
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : 
//-----------------------------------------------------------------------------
void CBlockingFileIOPanel::Paint() 
{
	int x = 2;
	
	int maxRecent = clamp( 0, cl_blocking_recentsize.GetInt(), 1000 );
	int bval = cl_showblocking.GetInt();
	if ( bval > 0 )
	{
		IBlockingFileItemList *list = filesystem->RetrieveBlockingFileAccessInfo();
		if ( list )
		{
			int i;
			int c = ARRAYSIZE( m_History );
			for ( i = 0; i < c; ++i )
			{
				m_History[ i ].m_flCurrent = 0.0f;
			}

			// Grab mutex (prevents async thread from filling in even more data...)
			list->LockMutex();
			{
				for ( int j = list->First() ; j != list->InvalidIndex(); j = list->Next( j ) )
				{
					const FileBlockingItem& item = list->Get( j );

					m_History[ item.m_ItemType ].m_flCurrent += item.m_flElapsed;

					RecentPeaks_t recent;
					recent.time = gpGlobals->realtime;
					recent.elapsed = item.m_flElapsed;
					recent.fileName = item.GetFileName();
					recent.reason = item.m_ItemType;
					recent.ioType = item.m_nAccessType;
					while ( m_Recent.Count() > maxRecent )
					{
						m_Recent.Remove( m_Recent.Head() );
					}

					m_Recent.AddToTail( recent );

					m_History[ item.m_ItemType ].m_LastFile = item.GetFileName();

					// Only care about time consuming synch or async blocking calls
					if ( item.m_ItemType == FILESYSTEM_BLOCKING_SYNCHRONOUS ||
						 item.m_ItemType == FILESYSTEM_BLOCKING_ASYNCHRONOUS_BLOCK )
					{
						if ( item.m_flElapsed > cl_blocking_threshold.GetFloat() )
						{
							SpewItem( recent );
						}
					}
				}
				list->Reset();
			}
			// Finished
			list->UnlockMutex();

			// Now draw some bars...
			int itemHeight = ( vgui::surface()->GetFontTall( m_hFont ) + 2 );

			int y = 2;
			int w = GetWide();

			DrawIOTime( x, y, w, itemHeight, FILESYSTEM_BLOCKING_SYNCHRONOUS, "Synchronous", Color( 255, 0, 0, 255 ) );
			y += 2*( itemHeight + 2 );
			DrawIOTime( x, y, w, itemHeight, FILESYSTEM_BLOCKING_ASYNCHRONOUS_BLOCK, "Async Block", Color( 255, 100, 0, 255 ) );
			y += 2*( itemHeight + 2 );
			DrawIOTime( x, y, w, itemHeight, FILESYSTEM_BLOCKING_CALLBACKTIMING, "Callback", Color( 255, 255, 0, 255 ) );
			y += 2*( itemHeight + 2 );
			DrawIOTime( x, y, w, itemHeight, FILESYSTEM_BLOCKING_ASYNCHRONOUS, "Asynchronous", Color( 0, 255, 0, 255 ) );

			for ( i = 0; i < c; ++i )
			{
				if ( m_History[ i ].m_flCurrent > m_History[ i ].m_flHistory )
				{
					m_History[ i ].m_flHistory = m_History[ i ].m_flCurrent;
					m_History[ i ].m_flHistorySpike = m_History[ i ].m_flCurrent;
					m_History[ i ].m_flLatchTime = gpGlobals->realtime;
				}
				else
				{
					// After this long, start to decay the previous history value
					if ( gpGlobals->realtime > m_History[ i ].m_flLatchTime + 1.0f )
					{
						m_History[ i ].m_flHistory = m_History[ i ].m_flHistory * IO_DECAY_FRAC + ( 1.0f - IO_DECAY_FRAC ) * m_History[ i ].m_flCurrent;
					}
				}
			}
		}
	}
}

static ConVar cl_blocking_msec( "cl_blocking_msec", "100", 0, "Vertical scale of blocking graph in milliseconds" );

static const char *GetBlockReason( int reason )
{
	switch ( reason )
	{
	case FILESYSTEM_BLOCKING_SYNCHRONOUS:
		return "Synchronous";
	case FILESYSTEM_BLOCKING_ASYNCHRONOUS:
		return "Asynchronous";
	case FILESYSTEM_BLOCKING_CALLBACKTIMING:
		return "Async Callback";
	case FILESYSTEM_BLOCKING_ASYNCHRONOUS_BLOCK:
		return "Async Blocked";
	}
	return "???";
}

static const char *GetIOType( int iotype )
{
	if ( FileBlockingItem::FB_ACCESS_APPEND == iotype )
	{
		return "Append";
	}
	else if ( FileBlockingItem::FB_ACCESS_CLOSE == iotype )
	{
		return "Close";
	}
	else if ( FileBlockingItem::FB_ACCESS_OPEN == iotype)
	{
		return "Open";
	}
	else if ( FileBlockingItem::FB_ACCESS_READ == iotype)
	{
		return "Read";
	}
	else if ( FileBlockingItem::FB_ACCESS_SIZE == iotype)
	{
		return "Size";
	}
	else if ( FileBlockingItem::FB_ACCESS_WRITE == iotype)
	{
		return "Write";
	}
	return "???";
}

void CBlockingFileIOPanel::SpewItem( const RecentPeaks_t& item )
{
	switch ( item.reason )
	{
	default:
		Assert( 0 );
		// break; -- intentionally fall through
	case FILESYSTEM_BLOCKING_ASYNCHRONOUS:
	case FILESYSTEM_BLOCKING_CALLBACKTIMING:
		Msg( "%8.3f %16.16s i/o [%6.6s] took %8.3f msec:  %33.33s\n", 
			item.time, 
			GetBlockReason( item.reason ), 
			GetIOType( item.ioType ),
			item.elapsed * 1000.0f, 
			item.fileName.String()
			);
		break;
	case FILESYSTEM_BLOCKING_SYNCHRONOUS:
	case FILESYSTEM_BLOCKING_ASYNCHRONOUS_BLOCK:
		Warning( "%8.3f %16.16s i/o [%6.6s] took %8.3f msec:  %33.33s\n", 
			item.time, 
			GetBlockReason( item.reason ), 
			GetIOType( item.ioType ),
			item.elapsed * 1000.0f, 
			item.fileName.String()
			);
		break;
	}
}

void CBlockingFileIOPanel::SpewRecent()
{
	FOR_EACH_LL( m_Recent, i )
	{
		const RecentPeaks_t& item = m_Recent[ i ];
		SpewItem( item );
	}
}

void  CBlockingFileIOPanel::DrawIOTime( int x, int y, int w, int h, int slot, char const *label, const Color& clr )
{
	float t = m_History[ slot ].m_flCurrent;
	float history = m_History[ slot ].m_flHistory;
	float latchedtime = m_History[ slot ].m_flLatchTime;
	float historyspike = m_History[ slot ].m_flHistorySpike;

	// 250 msec is considered a huge spike
	float maxTime = cl_blocking_msec.GetFloat() * 0.001f;
	if ( maxTime < 0.000001f )
		return;
	float frac = clamp( t / maxTime, 0.0f, 1.0f );
	float hfrac = clamp( history / maxTime, 0.0f, 1.0f );
	float spikefrac = clamp( historyspike / maxTime, 0.0f, 1.0f );

	g_pMatSystemSurface->DrawColoredText( m_hFont, x + 2, y + 1, 
			clr[0], clr[1], clr[2], clr[3], 
			"%s", 
			label );

	int textWidth = 95;

	x += textWidth;
	w -= ( textWidth + 5 );

	int prevFileWidth = 140;
	w -= prevFileWidth;

	bool bDrawHistorySpike = false;

	if ( m_History[ slot ].m_LastFile.IsValid() && 
		( gpGlobals->realtime < latchedtime + 10.0f ) )
	{
		bDrawHistorySpike = true;
		g_pMatSystemSurface->DrawColoredText( m_hFont, x + w + 5, y + 1, 
				255, 255, 255, 200, "[%8.3f ms]", m_History[ slot ].m_flHistorySpike * 1000.0f );
		g_pMatSystemSurface->DrawColoredText( m_hFont, x, y + h + 1, 
				255, 255, 255, 200, "%s", m_History[ slot ].m_LastFile.String() );
	}

	y += 2;
	h -= 4;

	int barWide = ( int )( w * frac + 0.5f );
	int historyWide = ( int ) ( w * hfrac + 0.5f );
	int spikeWide = ( int ) ( w * spikefrac + 0.5f );

	int useWide = max( barWide, historyWide );

	vgui::surface()->DrawSetColor( Color( 0, 0, 0, 31 ) );
	vgui::surface()->DrawFilledRect( x, y, x + w, y + h );
	vgui::surface()->DrawSetColor( Color( 255, 255, 255, 128 ) );
	vgui::surface()->DrawOutlinedRect( x, y, x + w, y + h );
	vgui::surface()->DrawSetColor( clr );
	vgui::surface()->DrawFilledRect( x+1, y+1, x + useWide, y + h -1 );
	if ( bDrawHistorySpike )
	{
		vgui::surface()->DrawSetColor( Color( 255, 255, 255, 192 ) );
		vgui::surface()->DrawFilledRect( x + spikeWide, y + 1, x + spikeWide + 1, y + h - 1 );
	}
}

class CBlockingFileIO : public IShowBlockingPanel
{
private:
	CBlockingFileIOPanel *ioPanel;
public:
	CBlockingFileIO( void )
	{
		ioPanel = NULL;
	}

	void Create( vgui::VPANEL parent )
	{
		ioPanel = new CBlockingFileIOPanel( parent );
	}

	void Destroy( void )
	{
		if ( ioPanel )
		{
			ioPanel->SetParent( (vgui::Panel *)NULL );
			delete ioPanel;
		}
	}

	void Spew()
	{
		if ( ioPanel )
		{
			ioPanel->SpewRecent();
		}
	}
};

static CBlockingFileIO g_IOPanel;
IShowBlockingPanel *iopanel = ( IShowBlockingPanel * )&g_IOPanel;

CON_COMMAND( spewblocking, "Spew current blocking file list." )
{
	g_IOPanel.Spew();
}

#endif // TRACK_BLOCKING_IO