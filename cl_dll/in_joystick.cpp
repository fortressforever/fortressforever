//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Joystick handling function
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include <windows.h>
#include "basehandle.h"
#include "utlvector.h"
#include "cdll_client_int.h"
#include "cdll_util.h"
#include "kbutton.h"
#include "usercmd.h"
#include "keydefs.h"
#include "input.h"
#include "iviewrender.h"
#include "convar.h"
#include "vstdlib/icommandline.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Control like a joystick
#define JOY_ABSOLUTE_AXIS	0x00000000		
// Control like a mouse, spinner, trackball
#define JOY_RELATIVE_AXIS	0x00000010		

#define JOY_POVFWDRIGHT		( ( JOY_POVFORWARD + JOY_POVRIGHT ) >> 1 )  // 4500
#define JOY_POVRIGHTBACK	( ( JOY_POVRIGHT + JOY_POVBACKWARD ) >> 1 ) // 13500
#define JOY_POVFBACKLEFT	( ( JOY_POVBACKWARD + JOY_POVLEFT ) >> 1 ) // 22500
#define JOY_POVLEFTFWD		( ( JOY_POVLEFT + JOY_POVFORWARD ) >> 1 ) // 31500

// Joystick info from windows multimedia system.
static JOYINFOEX ji;

static ConVar joy_name( "joy_name", "joystick", FCVAR_ARCHIVE );
static ConVar joy_advanced( "joy_advanced", "0", 0 );
static ConVar joy_advaxisx( "joy_advaxisx", "0", 0 );
static ConVar joy_advaxisy( "joy_advaxisy", "0", 0 );
static ConVar joy_advaxisz( "joy_advaxisz", "0", 0 );
static ConVar joy_advaxisr( "joy_advaxisr", "0", 0 );
static ConVar joy_advaxisu( "joy_advaxisu", "0", 0 );
static ConVar joy_advaxisv( "joy_advaxisv", "0", 0 );
static ConVar joy_forwardthreshold( "joy_forwardthreshold", "0.15", FCVAR_ARCHIVE );
static ConVar joy_sidethreshold( "joy_sidethreshold", "0.15", FCVAR_ARCHIVE );
static ConVar joy_pitchthreshold( "joy_pitchthreshold", "0.15", FCVAR_ARCHIVE );
static ConVar joy_yawthreshold( "joy_yawthreshold", "0.15", FCVAR_ARCHIVE );
static ConVar joy_forwardsensitivity( "joy_forwardsensitivity", "-1", FCVAR_ARCHIVE );
static ConVar joy_sidesensitivity( "joy_sidesensitivity", "-1", FCVAR_ARCHIVE );
static ConVar joy_pitchsensitivity( "joy_pitchsensitivity", "1", FCVAR_ARCHIVE );
static ConVar joy_yawsensitivity( "joy_yawsensitivity", "-1", FCVAR_ARCHIVE );
static ConVar joy_wwhack1( "joy_wingmanwarrier_centerhack", "0", FCVAR_ARCHIVE, "Wingman warrior centering hack." );
static ConVar joy_wwhack2( "joy_wingmanwarrier_turnhack", "0", FCVAR_ARCHIVE, "Wingman warrior hack related to turn axes." );

static ConVar joy_diagonalpov( "joy_diagonalpov", "0", FCVAR_ARCHIVE, "POV manipulator operates on diagonal axes, too." );

extern ConVar lookspring;
extern ConVar cl_forwardspeed;
extern ConVar lookstrafe;
extern ConVar in_joystick;
extern ConVar m_pitch;
extern ConVar l_pitchspeed;
extern ConVar cl_sidespeed;
extern ConVar cl_yawspeed;
extern ConVar cl_pitchdown;
extern ConVar cl_pitchup;
extern ConVar cl_pitchspeed;
 
//-----------------------------------------------------------------------------
// Purpose: Init_Joystick 
//-----------------------------------------------------------------------------
void CInput::Init_Joystick( void ) 
{ 
	int			numdevs;
	JOYCAPS		jc;
	MMRESULT	mmr;
 
	m_rgAxes[ JOY_AXIS_X ].AxisFlags = JOY_RETURNX;
	m_rgAxes[ JOY_AXIS_Y ].AxisFlags = JOY_RETURNY;
	m_rgAxes[ JOY_AXIS_Z ].AxisFlags = JOY_RETURNZ;
	m_rgAxes[ JOY_AXIS_R ].AxisFlags = JOY_RETURNR;
	m_rgAxes[ JOY_AXIS_U ].AxisFlags = JOY_RETURNU;
	m_rgAxes[ JOY_AXIS_V ].AxisFlags = JOY_RETURNV;

 	// assume no joystick
	m_fJoystickAvailable = false; 

	// abort startup if user requests no joystick
	if ( CommandLine()->FindParm("-nojoy" ) ) 
	{
		return; 
	}
 
	// verify joystick driver is present
	numdevs = joyGetNumDevs();
	if ( numdevs <= 0)
	{
		DevMsg( 1, "joystick not found -- driver not present\n");
		return;
	}

	// Error if nothing gets assigned..
	mmr = JOYERR_NOERROR+1; 
	// cycle through the joysticks looking for the first valid one
	for (m_nJoystickID=0 ; m_nJoystickID<numdevs ; m_nJoystickID++)
	{
		Q_memset( &ji, 0, sizeof( ji ) );
		ji.dwSize = sizeof(ji);
		ji.dwFlags = JOY_RETURNCENTERED;
		mmr = joyGetPosEx( m_nJoystickID, &ji );
		if ( mmr == JOYERR_NOERROR )
		{
			// Found one
			break;
		}
	} 

	// Abort startup if we didn't find a valid joystick
	if (mmr != JOYERR_NOERROR)
	{
		return;
	}

	// get the capabilities of the selected joystick
	// abort startup if command fails
	Q_memset( &jc, 0, sizeof( jc ) );
	mmr = joyGetDevCaps( m_nJoystickID, &jc, sizeof( jc ) );
	if ( mmr != JOYERR_NOERROR )
	{
		DevWarning( 1, "joystick not found -- invalid joystick capabilities (%x)\n\n", mmr); 
		return;
	}

	// save the joystick's number of buttons and POV status
	m_nJoystickButtons = (int)jc.wNumButtons;
	m_fJoystickHasPOVControl = ( jc.wCaps & JOYCAPS_HASPOV ) ? true : false;

	// old button and POV states default to no buttons pressed
	m_nJoystickOldButtons	= 0;
	m_nJoystickOldPOVState	= 0;

	// Mark the joystick as available and advanced initialization not completed
	// this is needed as correctly set cvars are not available this early during initialization
	// FIXME:  Is this still the case?
	Msg( "joystick found\n" ); 
	m_fJoystickAvailable = true; 
	m_fJoystickAdvancedInit = false;
}

//-----------------------------------------------------------------------------
// Purpose: Get raw joystick sample along axis
// Input  : axis - 
// Output : unsigned long
//-----------------------------------------------------------------------------
unsigned long *CInput::RawValuePointer( int axis )
{
	switch (axis)
	{
	case JOY_AXIS_X:
		return &ji.dwXpos;
	case JOY_AXIS_Y:
		return &ji.dwYpos;
	case JOY_AXIS_Z:
		return &ji.dwZpos;
	case JOY_AXIS_R:
		return &ji.dwRpos;
	case JOY_AXIS_U:
		return &ji.dwUpos;
	case JOY_AXIS_V:
		return &ji.dwVpos;
	}
	// FIX: need to do some kind of error
	return &ji.dwXpos;
}

//-----------------------------------------------------------------------------
// Purpose: Advanced joystick setup
//-----------------------------------------------------------------------------
void CInput::Joystick_Advanced(void)
{
	// called once by ReadJoystick() and by user whenever an update is needed
	int	i;
	DWORD dwTemp;

	// Initialize all the maps
	for ( i = 0; i < JOY_MAX_AXES; i++ )
	{
		m_rgAxes[i].AxisMap = AxisNada;
		m_rgAxes[i].ControlMap = JOY_ABSOLUTE_AXIS;
		m_rgAxes[i].pRawValue = RawValuePointer(i);
	}

	if ( !joy_advanced.GetBool() )
	{
		// default joystick initialization
		// 2 axes only with joystick control
		m_rgAxes[JOY_AXIS_X].AxisMap = AxisTurn;
		m_rgAxes[JOY_AXIS_Y].AxisMap = AxisForward;
	}
	else
	{
		if ( Q_stricmp( joy_name.GetString(), "joystick") != 0 )
		{
			// notify user of advanced controller
			Msg( "Using joystick '%s' configuration\n", joy_name.GetString() );
		}

		// advanced initialization here
		// data supplied by user via joy_axisn cvars
		dwTemp = (DWORD)joy_advaxisx.GetInt();
		m_rgAxes[JOY_AXIS_X].AxisMap = dwTemp & 0x0000000f;
		m_rgAxes[JOY_AXIS_X].ControlMap = dwTemp & JOY_RELATIVE_AXIS;

		DescribeJoystickAxis( "JOY_AXIS_X", &m_rgAxes[JOY_AXIS_X] );

		dwTemp = (DWORD)joy_advaxisy.GetInt();
		m_rgAxes[JOY_AXIS_Y].AxisMap = dwTemp & 0x0000000f;
		m_rgAxes[JOY_AXIS_Y].ControlMap = dwTemp & JOY_RELATIVE_AXIS;

		DescribeJoystickAxis( "JOY_AXIS_Y", &m_rgAxes[JOY_AXIS_Y] );

		dwTemp = (DWORD)joy_advaxisz.GetInt();
		m_rgAxes[JOY_AXIS_Z].AxisMap = dwTemp & 0x0000000f;
		m_rgAxes[JOY_AXIS_Z].ControlMap = dwTemp & JOY_RELATIVE_AXIS;

		DescribeJoystickAxis( "JOY_AXIS_Z", &m_rgAxes[JOY_AXIS_Z] );

		dwTemp = (DWORD)joy_advaxisr.GetInt();
		m_rgAxes[JOY_AXIS_R].AxisMap = dwTemp & 0x0000000f;
		m_rgAxes[JOY_AXIS_R].ControlMap = dwTemp & JOY_RELATIVE_AXIS;

		DescribeJoystickAxis( "JOY_AXIS_R", &m_rgAxes[JOY_AXIS_R] );

		dwTemp = (DWORD)joy_advaxisu.GetInt();
		m_rgAxes[JOY_AXIS_U].AxisMap = dwTemp & 0x0000000f;
		m_rgAxes[JOY_AXIS_U].ControlMap = dwTemp & JOY_RELATIVE_AXIS;

		DescribeJoystickAxis( "JOY_AXIS_U", &m_rgAxes[JOY_AXIS_U] );

		dwTemp = (DWORD)joy_advaxisv.GetInt();
		m_rgAxes[JOY_AXIS_V].AxisMap = dwTemp & 0x0000000f;
		m_rgAxes[JOY_AXIS_V].ControlMap = dwTemp & JOY_RELATIVE_AXIS;

		DescribeJoystickAxis( "JOY_AXIS_V", &m_rgAxes[JOY_AXIS_V] );

		Msg( "Advanced Joystick settings initialized\n" );
	}

	// Compute the axes to collect from driver
	m_nJoystickFlags = JOY_RETURNCENTERED | JOY_RETURNBUTTONS | JOY_RETURNPOV;
	for ( i = 0; i < JOY_MAX_AXES; i++ )
	{
		if ( m_rgAxes[i].AxisMap != AxisNada )
		{
			m_nJoystickFlags |= m_rgAxes[i].AxisFlags;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : index - 
// Output : char const
//-----------------------------------------------------------------------------
char const *CInput::DescribeAxis( int index )
{
	switch ( index )
	{
	default:
	case AxisNada:
		return "Unknown";
	case AxisForward:
		return "Forward";
	case AxisLook:
		return "Look";
	case AxisSide:
		return "Side";
	case AxisTurn:
		return "Turn";
	}

	return "Unknown";
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *axis - 
//			*mapping - 
//-----------------------------------------------------------------------------
void CInput::DescribeJoystickAxis( char const *axis, joy_axis_t *mapping )
{
	if ( !mapping->AxisMap )
	{
		Msg( "%s:  unmapped\n", axis );
	}
	else
	{
		Msg( "%s:  mapped to %s (%s)\n",
			axis, 
			DescribeAxis( mapping->AxisMap ),
			mapping->ControlMap != 0 ? "relative" : "absolute" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Allow joystick to issue key events
//-----------------------------------------------------------------------------
void CInput::ControllerCommands( void )
{
	// No valid joysticks
	if ( !m_fJoystickAvailable )
	{
		return;
	}

	int		i, key_index;
	DWORD	buttonstate;

	// loop through the joystick buttons
	// key a joystick event or auxillary event for higher number buttons for each state change
	buttonstate = ji.dwButtons;

	for (i=0 ; i < m_nJoystickButtons ; i++)
	{
		if ( (buttonstate & (1<<i)) && !(m_nJoystickOldButtons & (1<<i)) )
		{
			key_index = (i < 4) ? K_JOY1 : K_AUX1;
			engine->Key_Event (key_index + i, 1);
		}

		if ( !(buttonstate & (1<<i)) && (m_nJoystickOldButtons & (1<<i)) )
		{
			key_index = (i < 4) ? K_JOY1 : K_AUX1;
			engine->Key_Event (key_index + i, 0);
		}
	}

	m_nJoystickOldButtons = (unsigned int)buttonstate;

	// Joystick has POV hat control
	if ( m_fJoystickHasPOVControl )
	{
		// convert POV information into 4 bits of state information
		// this avoids any potential problems related to moving from one
		// direction to another without going through the center position
		DWORD povstate = 0;

		if ( ji.dwPOV != JOY_POVCENTERED )
		{
			if (ji.dwPOV == JOY_POVFORWARD)  // 0
			{
				povstate |= 0x01;
			}
			if (ji.dwPOV == JOY_POVRIGHT)  // 9000
			{
				povstate |= 0x02;
			}
			if (ji.dwPOV == JOY_POVBACKWARD) // 18000
			{
				povstate |= 0x04;
			}
			if (ji.dwPOV == JOY_POVLEFT)  // 27000
			{
				povstate |= 0x08;
			}

			// Deal with diagonals if user wants them
			if ( joy_diagonalpov.GetBool() )
			{
				if (ji.dwPOV == JOY_POVFWDRIGHT)  // 4500
				{
					povstate |= ( 0x01 | 0x02 );
				}
				if (ji.dwPOV == JOY_POVRIGHTBACK)  // 13500
				{
					povstate |= ( 0x02 | 0x04 );
				}
				if (ji.dwPOV == JOY_POVFBACKLEFT) // 22500
				{
					povstate |= ( 0x04 | 0x08 );
				}
				if (ji.dwPOV == JOY_POVLEFTFWD)  // 31500
				{
					povstate |= ( 0x08 | 0x01 );
				}
			}
		}

		// determine which bits have changed and key an auxillary event for each change
		for ( i=0 ; i < 4 ; i++ )
		{
			// Keydown on POV buttons
			if ( (povstate & (1<<i)) && !(m_nJoystickOldPOVState & (1<<i)) )
			{
				engine->Key_Event (K_AUX29 + i, 1);
			}

			// KeyUp on POV buttons
			if ( !(povstate & (1<<i)) && (m_nJoystickOldPOVState & (1<<i)) )
			{
				engine->Key_Event (K_AUX29 + i, 0);
			}
		}

		// Latch old values
		m_nJoystickOldPOVState = (unsigned int)povstate;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sample the joystick
//-----------------------------------------------------------------------------
bool CInput::ReadJoystick (void)
{
	Q_memset( &ji, 0, sizeof( ji ) );
	ji.dwSize = sizeof( ji );
	ji.dwFlags = (DWORD)m_nJoystickFlags;

	if ( joyGetPosEx( m_nJoystickID, &ji ) == JOYERR_NOERROR )
	{
		// This hack fixes a bug in the Logitech WingMan Warrior DirectInput Driver
		// rather than having 32768 be the zero point, they have the zero point at 32668
		// go figure -- anyway, now we get the full resolution out of the device
		if ( joy_wwhack1.GetBool() )
		{
			ji.dwUpos += 100;
		}
		return true;
	}

	// A read error occurred, just ignore...
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Apply joystick to CUserCmd creation
// Input  : frametime - 
//			*cmd - 
//-----------------------------------------------------------------------------
void CInput::JoyStickMove( float frametime, CUserCmd *cmd )
{
	// complete initialization if first time in ( needed as cvars are not available at initialization time )
	if ( !m_fJoystickAdvancedInit )
	{
		Joystick_Advanced();
		m_fJoystickAdvancedInit = true;
	}

	// verify joystick is available and that the user wants to use it
	if (!m_fJoystickAvailable || !in_joystick.GetInt() )
	{
		return; 
	}
 
	// collect the joystick data, if possible
	if ( !ReadJoystick() )
	{
		return;
	}

	QAngle viewangles;

	// Get starting angles
	engine->GetViewAngles( viewangles );

	int		i;
	float	speed = 1.0f;
	float   aspeed = speed * frametime;

	// Loop through each axis
	for ( i = 0; i < JOY_MAX_AXES; i++ )
	{
		// get the floating point zero-centered, potentially-inverted data for the current axis
		float fAxisValue = (float)( *m_rgAxes[i].pRawValue );
		// move centerpoint to zero
		fAxisValue -= 32768.0;

		if (joy_wwhack2.GetInt() != 0 )
		{
			if (m_rgAxes[i].AxisMap == AxisTurn)
			{
				// this is a special formula for the Logitech WingMan Warrior
				// y=ax^b; where a = 300 and b = 1.3
				// also x values are in increments of 800 (so this is factored out)
				// then bounds check result to level out excessively high spin rates
				float fTemp = 300.0 * pow(abs(fAxisValue) / 800.0, 1.3);
				if (fTemp > 14000.0)
					fTemp = 14000.0;
				// restore direction information
				fAxisValue = (fAxisValue > 0.0) ? fTemp : -fTemp;
			}
		}

		// convert range from -32768..32767 to -1..1 
		fAxisValue /= 32768.0;

		fAxisValue = clamp( fAxisValue, -1.0f, 1.0f );

		switch ( m_rgAxes[i].AxisMap )
		{
		case AxisForward:
			if ((joy_advanced.GetInt() == 0) && (in_jlook.state & 1))
			{
				// user wants forward control to become look control
				if (fabs(fAxisValue) > joy_pitchthreshold.GetFloat())
				{		
					// if mouse invert is on, invert the joystick pitch value
					// only absolute control support here (joy_advanced is 0)
					if (m_pitch.GetFloat() < 0.0)
					{
						viewangles[PITCH] -= (fAxisValue * joy_pitchsensitivity.GetFloat()) * aspeed * cl_pitchspeed.GetFloat();
					}
					else
					{
						viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity.GetFloat()) * aspeed * cl_pitchspeed.GetFloat();
					}
					view->StopPitchDrift();
				}
				else
				{
					// no pitch movement
					// disable pitch return-to-center unless requested by user
					// *** this code can be removed when the lookspring bug is fixed
					// *** the bug always has the lookspring feature on
					if(lookspring.GetFloat() == 0.0)
					{
						view->StopPitchDrift();
					}
				}
			}
			else
			{
				// user wants forward control to be forward control
				if (fabs(fAxisValue) > joy_forwardthreshold.GetFloat())
				{
					cmd->forwardmove += (fAxisValue * joy_forwardsensitivity.GetFloat()) * speed * cl_forwardspeed.GetFloat();
				}
			}
			break;

		case AxisSide:
			if (fabs(fAxisValue) > joy_sidethreshold.GetFloat())
			{
				cmd->sidemove += (fAxisValue * joy_sidesensitivity.GetFloat()) * speed * cl_sidespeed.GetFloat();
			}
			break;

		case AxisTurn:
			if ((in_strafe.state & 1) || (lookstrafe.GetFloat() && (in_jlook.state & 1)))
			{
				// user wants turn control to become side control
				if (fabs(fAxisValue) > joy_sidethreshold.GetFloat())
				{
					cmd->sidemove -= (fAxisValue * joy_sidesensitivity.GetFloat()) * speed * cl_sidespeed.GetFloat();
				}
			}
			else
			{
				// user wants turn control to be turn control
				if (fabs(fAxisValue) > joy_yawthreshold.GetFloat())
				{
					if ( m_rgAxes[i].ControlMap == JOY_ABSOLUTE_AXIS )
					{
						viewangles[YAW] += (fAxisValue * joy_yawsensitivity.GetFloat()) * aspeed * cl_yawspeed.GetFloat();
					}
					else
					{
						viewangles[YAW] += (fAxisValue * joy_yawsensitivity.GetFloat()) * speed * 180.0;
					}

				}
			}
			break;

		case AxisLook:
			if (in_jlook.state & 1)
			{
				if (fabs(fAxisValue) > joy_pitchthreshold.GetFloat())
				{
					// pitch movement detected and pitch movement desired by user
					if ( m_rgAxes[i].ControlMap == JOY_ABSOLUTE_AXIS )
					{
						viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity.GetFloat()) * aspeed * cl_pitchspeed.GetFloat();
					}
					else
					{
						viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity.GetFloat()) * speed * 180.0;
					}
					view->StopPitchDrift();
				}
				else
				{
					// no pitch movement
					// disable pitch return-to-center unless requested by user
					// *** this code can be removed when the lookspring bug is fixed
					// *** the bug always has the lookspring feature on
					if( lookspring.GetFloat() == 0.0 )
					{
						view->StopPitchDrift();
					}
				}
			}
			break;

		default:
			break;
		}
	}

	// Bound pitch
	viewangles[PITCH] = clamp( viewangles[ PITCH ], -cl_pitchup.GetFloat(), cl_pitchdown.GetFloat() );

	engine->SetViewAngles( viewangles );

}

