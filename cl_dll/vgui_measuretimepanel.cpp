//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "iprofiling.h"
#include "measure_section.h"
#include "vgui_BasePanel.h"
#include <vgui/IVgui.h>
#include <vgui/IScheme.h>
#include "vguimatsurface/imatsystemsurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

enum TimeMode
{
	Cycles=0,
	Microseconds,
	Milliseconds,
	Seconds,
	Frametime,
	NUM_TIME_MODES
};


ConVar timemode(
	"timemode",		// name
	"4",			// default (% frametime)
	0,				// flags
	"Show timings in:\n    0 = cycles\n    1 = microseconds\n    2 = milliseconds\n    3 = seconds\n    4 = %% frametime", // description
	true,			// bMin
	0,				// minVal
	true,			// bMax
	NUM_TIME_MODES-1// maxVal
	);

ConVar host_speeds("host_speeds", "0");

void FormatTimeModeString(TimeMode mode, const CCycleCount &elapsed, char *str)
{
	switch(mode)
	{
		case Cycles:
		{
			Q_snprintf(str, sizeof( str ), "%lu cycles", elapsed.GetCycles());
		}
		break;

		case Microseconds:
		{
			Q_snprintf(str, sizeof( str ), "%.1fu", elapsed.GetMicrosecondsF());
		}
		break;

		case Milliseconds:
		{
			Q_snprintf(str, sizeof( str ), "%.3fms", elapsed.GetMillisecondsF());
		}
		break;
		
		case Seconds:
		{
			Q_snprintf(str, sizeof( str ), "%.4f seconds", elapsed.GetSeconds());
		}
		break;

		case Frametime:
		{
			// Put two %%'s to deal with recursive call into format specifier resolution
			Q_snprintf(str, sizeof( str ), "%.3f%%%%", elapsed.GetSeconds() * 100.0 / gpGlobals->frametime);
		}
		break;

		default:
		{
			Q_strncpy(str, "<INVALID TIMEMODE>", sizeof( str ) );
		}
		break;
	}
}

float GetRealTime()
{
	return engine->Time();
}

//-----------------------------------------------------------------------------
// Purpose: The system profiling panel
//-----------------------------------------------------------------------------
class CMeasureTimePanel : public vgui::Panel
{
	typedef vgui::Panel BaseClass;
public:
					CMeasureTimePanel( vgui::VPANEL parent );
	virtual			~CMeasureTimePanel( void );

	virtual void	ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void	Paint();
	virtual void	OnTick( void );

	virtual bool	ShouldDraw( void );

private:
	vgui::HFont		m_hFont;
};

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *parent - 
//-----------------------------------------------------------------------------
CMeasureTimePanel::CMeasureTimePanel( vgui::VPANEL parent ) : 
	BaseClass( NULL, "CMeasureTimePanel" )
{
	SetParent( parent );
	SetSize( ScreenWidth(), ScreenHeight() );
	SetPos( 0, 0 );
	SetVisible( false );
	SetCursor( null );

	m_hFont = 0;

	SetFgColor( Color( 0, 0, 0, 255 ) );
	SetPaintBackgroundEnabled( false );

	vgui::ivgui()->AddTickSignal( GetVPanel(), 100 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CMeasureTimePanel::~CMeasureTimePanel( void )
{
}

void CMeasureTimePanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_hFont = pScheme->GetFont( "DefaultVerySmall" );
	assert( m_hFont );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMeasureTimePanel::OnTick( void )
{
	SetVisible( ShouldDraw() );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CMeasureTimePanel::ShouldDraw( void )
{
	if ( host_speeds.GetInt() < 3 )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : 
//-----------------------------------------------------------------------------
void CMeasureTimePanel::Paint() 
{
	int col0 = 20;
	int	col1 = 200;

	int row = 1;
	int rowheight = 15;

	int y = 20;

	// Draw the engine's first
	for(int iList=0; iList < 2; iList++)
	{
		CMeasureSection *p = iList == 0 ? engine->GetMeasureSectionList() : CMeasureSection::GetList();
		while ( p )
		{
			CCycleCount cTime = p->GetTime();
			CCycleCount cMaxTime = p->GetMaxTime();

			g_pMatSystemSurface->DrawColoredText( m_hFont, col0, y + row * rowheight, 255, 255, 255, 255, "%s : ", p->GetName() );
			
			char str[256];

			FormatTimeModeString((TimeMode)timemode.GetInt(), cTime, str);

			char peakstring[256];

			FormatTimeModeString((TimeMode)timemode.GetInt(), cMaxTime, peakstring);

			g_pMatSystemSurface->DrawColoredText( m_hFont, col1, y + row * rowheight, 255, 255, 255, 255, VarArgs( "%10s : %10s", str, peakstring ) );

			row++;

			p->UpdateMax();

			p = p->GetNext();
		}
	}
}

class CProfiling : public IProfiling
{
private:
	CMeasureTimePanel *measureTimePanel;
public:
	CProfiling( void )
	{
		measureTimePanel = NULL;
	}
	void Create( vgui::VPANEL parent )
	{
		measureTimePanel = new CMeasureTimePanel( parent );
	}

	void Destroy( void )
	{
		if ( measureTimePanel )
		{
			measureTimePanel->SetParent( (vgui::Panel *)NULL );
			delete measureTimePanel;
		}
	}
};

static CProfiling g_Profiling;
IProfiling *profiling = ( IProfiling * )&g_Profiling;
