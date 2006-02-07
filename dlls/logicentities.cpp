//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements many of the entities that control logic flow within a map.
//
//=============================================================================//

#include "cbase.h"
#include "entityinput.h"
#include "entityoutput.h"
#include "eventqueue.h"
#include "mathlib.h"
#include "globalstate.h"
#include "ndebugoverlay.h"
#include "vstdlib/random.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Compares a set of integer inputs to the one main input
//			Outputs true if they are all equivalant, false otherwise
//-----------------------------------------------------------------------------
class CLogicCompareInteger : public CLogicalEntity
{
public:
	DECLARE_CLASS( CLogicCompareInteger, CLogicalEntity );

	// outputs
	COutputEvent m_OnEqual;
	COutputEvent m_OnNotEqual;

	// data
	int m_iIntegerValue;
	int m_iShouldCompareToValue;

	DECLARE_DATADESC();

	CMultiInputVar m_AllIntCompares;

	// Input handlers
	void InputValue( inputdata_t &inputdata );
	void InputCompareValues( inputdata_t &inputdata );
};


LINK_ENTITY_TO_CLASS( logic_multicompare, CLogicCompareInteger );


BEGIN_DATADESC( CLogicCompareInteger )

	DEFINE_OUTPUT( m_OnEqual, "OnEqual" ),
	DEFINE_OUTPUT( m_OnNotEqual, "OnNotEqual" ),

	DEFINE_KEYFIELD( m_iIntegerValue, FIELD_INTEGER, "IntegerValue" ),
	DEFINE_KEYFIELD( m_iShouldCompareToValue, FIELD_INTEGER, "ShouldComparetoValue" ),

	DEFINE_FIELD( m_AllIntCompares, FIELD_INPUT ),

	DEFINE_INPUTFUNC( FIELD_INPUT, "InputValue", InputValue ),
	DEFINE_INPUTFUNC( FIELD_INPUT, "CompareValues", InputCompareValues ),

END_DATADESC()




//-----------------------------------------------------------------------------
// Purpose: Adds to the list of compared values
//-----------------------------------------------------------------------------
void CLogicCompareInteger::InputValue( inputdata_t &inputdata )
{
	// make sure it's an int, if it can't be converted just throw it away
	if ( !inputdata.value.Convert(FIELD_INTEGER) )
		return;

	// update the value list with the new value
	m_AllIntCompares.AddValue( inputdata.value, inputdata.nOutputID );

	// if we haven't already this frame, send a message to ourself to update and fire
	if ( !m_AllIntCompares.m_bUpdatedThisFrame )
	{
		// TODO: need to add this event with a lower priority, so it gets called after all inputs have arrived
		g_EventQueue.AddEvent( this, "ForceRecompare", 0, inputdata.pActivator, this, inputdata.nOutputID );
		m_AllIntCompares.m_bUpdatedThisFrame = TRUE;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Forces a recompare
//-----------------------------------------------------------------------------
void CLogicCompareInteger::InputCompareValues( inputdata_t &inputdata )
{
	m_AllIntCompares.m_bUpdatedThisFrame = FALSE;

	// loop through all the values comparing them
	int value = m_iIntegerValue;
	CMultiInputVar::inputitem_t *input = m_AllIntCompares.m_InputList;

	if ( !m_iShouldCompareToValue && input )
	{
		value = input->value.Int();
	}

	while ( input )
	{
		if ( input->value.Int() != value )
		{
			// false
			m_OnNotEqual.FireOutput( inputdata.pActivator, this );
			return;
		}

		input = input->next;
	}

	// true! all values equal
	m_OnEqual.FireOutput( inputdata.pActivator, this );
}


//-----------------------------------------------------------------------------
// Purpose: Timer entity. Fires an output at regular or random intervals.
//-----------------------------------------------------------------------------
//
// Spawnflags and others constants.
//
const int SF_TIMER_UPDOWN = 1;
const float LOGIC_TIMER_MIN_INTERVAL = 0.01;


class CTimerEntity : public CLogicalEntity
{
public:
	DECLARE_CLASS( CTimerEntity, CLogicalEntity );

	void Spawn( void );
	void Think( void );

	void Toggle( void );
	void Enable( void );
	void Disable( void );
	void FireTimer( void );

	// outputs
	COutputEvent m_OnTimer;
	COutputEvent m_OnTimerHigh;
	COutputEvent m_OnTimerLow;

	// inputs
	void InputToggle( inputdata_t &inputdata );
	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );
	void InputFireTimer( inputdata_t &inputdata );
	void InputRefireTime( inputdata_t &inputdata );

	int m_iDisabled;
	float m_flRefireTime;
	bool m_bUpDownState;
	int m_iUseRandomTime;
	float m_flLowerRandomBound;
	float m_flUpperRandomBound;

	// methods
	void ResetTimer( void );
	
	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( logic_timer, CTimerEntity );


BEGIN_DATADESC( CTimerEntity )

	// Keys
	DEFINE_KEYFIELD( m_iDisabled, FIELD_INTEGER, "StartDisabled" ),
	DEFINE_KEYFIELD( m_flRefireTime, FIELD_FLOAT, "RefireTime" ),

	DEFINE_FIELD( m_bUpDownState, FIELD_BOOLEAN ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_FLOAT, "RefireTime", InputRefireTime ),
	DEFINE_INPUTFUNC( FIELD_VOID, "FireTimer", InputFireTimer ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),

	DEFINE_INPUT( m_iUseRandomTime, FIELD_INTEGER, "UseRandomTime" ),
	DEFINE_INPUT( m_flLowerRandomBound, FIELD_FLOAT, "LowerRandomBound" ),
	DEFINE_INPUT( m_flUpperRandomBound, FIELD_FLOAT, "UpperRandomBound" ),

	// Outputs
	DEFINE_OUTPUT( m_OnTimer, "OnTimer" ),
	DEFINE_OUTPUT( m_OnTimerHigh, "OnTimerHigh" ),
	DEFINE_OUTPUT( m_OnTimerLow, "OnTimerLow" ),

END_DATADESC()



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTimerEntity::Spawn( void )
{
	if (!m_iUseRandomTime && (m_flRefireTime < LOGIC_TIMER_MIN_INTERVAL))
	{
		m_flRefireTime = LOGIC_TIMER_MIN_INTERVAL;
	}

	if ( !m_iDisabled && (m_flRefireTime > 0 || m_iUseRandomTime) )
	{
		Enable();
	}
	else
	{
		Disable();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTimerEntity::Think( void )
{
	FireTimer();
}


//-----------------------------------------------------------------------------
// Purpose: Sets the time the timerentity will next fire
//-----------------------------------------------------------------------------
void CTimerEntity::ResetTimer( void )
{
	if ( m_iDisabled )
		return;

	if ( m_iUseRandomTime )
	{
		m_flRefireTime = random->RandomFloat( m_flLowerRandomBound, m_flUpperRandomBound );
	}

	SetNextThink( gpGlobals->curtime + m_flRefireTime );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTimerEntity::Enable( void )
{
	m_iDisabled = FALSE;
	ResetTimer();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTimerEntity::Disable( void )
{
	m_iDisabled = TRUE;
	SetNextThink( TICK_NEVER_THINK );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTimerEntity::Toggle( void )
{
	if ( m_iDisabled )
	{
		Enable();
	}
	else
	{
		Disable();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTimerEntity::FireTimer( void )
{
	if ( !m_iDisabled )
	{
		//
		// Up/down timers alternate between two outputs.
		//
		if (m_spawnflags & SF_TIMER_UPDOWN)
		{
			if (m_bUpDownState)
			{
				m_OnTimerHigh.FireOutput( this, this );
			}
			else
			{
				m_OnTimerLow.FireOutput( this, this );
			}

			m_bUpDownState = !m_bUpDownState;
		}
		//
		// Regular timers only fire a single output.
		//
		else
		{
			m_OnTimer.FireOutput( this, this );
		}

		ResetTimer();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTimerEntity::InputEnable( inputdata_t &inputdata )
{
	Enable();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTimerEntity::InputDisable( inputdata_t &inputdata )
{
	Disable();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTimerEntity::InputToggle( inputdata_t &inputdata )
{
	Toggle();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTimerEntity::InputFireTimer( inputdata_t &inputdata )
{
	FireTimer();
}


//-----------------------------------------------------------------------------
// Purpose: Changes the time interval between timer fires
//			Resets the next firing to be time + newRefireTime
// Input  : Float refire frequency in seconds.
//-----------------------------------------------------------------------------
void CTimerEntity::InputRefireTime( inputdata_t &inputdata )
{
	float flRefireInterval = inputdata.value.Float();

	if ( flRefireInterval < LOGIC_TIMER_MIN_INTERVAL)
	{
		flRefireInterval = LOGIC_TIMER_MIN_INTERVAL;
	}

	if (m_flRefireTime != flRefireInterval )
	{
		m_flRefireTime = flRefireInterval;
		ResetTimer();
	}
}


//-----------------------------------------------------------------------------
// Purpose: Computes a line between two entities
//-----------------------------------------------------------------------------
class CLogicLineToEntity : public CLogicalEntity
{
public:
	DECLARE_CLASS( CLogicLineToEntity, CLogicalEntity );

	void Activate(void);
	void Spawn( void );
	void Think( void );

	// outputs
	COutputVector m_Line;

	DECLARE_DATADESC();

private:
	string_t m_SourceName;
	EHANDLE	m_StartEntity;
	EHANDLE m_EndEntity;
};

LINK_ENTITY_TO_CLASS( logic_lineto, CLogicLineToEntity );


BEGIN_DATADESC( CLogicLineToEntity )

	// Keys
	// target is handled in the base class, stored in field m_target
	DEFINE_KEYFIELD( m_SourceName, FIELD_STRING, "source" ),
 	DEFINE_FIELD( m_StartEntity, FIELD_EHANDLE ),
	DEFINE_FIELD( m_EndEntity, FIELD_EHANDLE ),

	// Outputs
	DEFINE_OUTPUT( m_Line, "Line" ),

END_DATADESC()



//-----------------------------------------------------------------------------
// Find the entities
//-----------------------------------------------------------------------------
void CLogicLineToEntity::Activate(void)
{
	BaseClass::Activate();

	if (m_target != NULL_STRING)
	{
		m_EndEntity = gEntList.FindEntityByName(NULL, m_target, NULL);

		//
		// If we were given a bad measure target, just measure sound where we are.
		//
		if ((m_EndEntity == NULL) || (m_EndEntity->edict() == NULL))
		{
			Warning( "logic_lineto - Target not found or target with no origin!\n");
			m_EndEntity = this;
		}
	}
	else
	{
		m_EndEntity = this;
	}

	if (m_SourceName != NULL_STRING)
	{
		m_StartEntity = gEntList.FindEntityByName(NULL, m_SourceName, NULL);

		//
		// If we were given a bad measure target, just measure sound where we are.
		//
		if ((m_StartEntity == NULL) || (m_StartEntity->edict() == NULL))
		{
			Warning( "logic_lineto - Source not found or source with no origin!\n");
			m_StartEntity = this;
		}
	}
	else
	{
		m_StartEntity = this;
	}
}


//-----------------------------------------------------------------------------
// Find the entities
//-----------------------------------------------------------------------------
void CLogicLineToEntity::Spawn(void)
{
	SetNextThink( gpGlobals->curtime + 0.01f );
}


//-----------------------------------------------------------------------------
// Find the entities
//-----------------------------------------------------------------------------
void CLogicLineToEntity::Think(void)
{
	CBaseEntity* pDest = m_EndEntity.Get();
	CBaseEntity* pSrc = m_StartEntity.Get();
	if (!pDest || !pSrc || !pDest->edict() || !pSrc->edict())
	{
		// Can sleep for a long time, no more lines.
		m_Line.Set( vec3_origin, this, this );
		SetNextThink( gpGlobals->curtime + 10 );
		return;
	}

	Vector delta;
	VectorSubtract( pDest->GetAbsOrigin(), pSrc->GetAbsOrigin(), delta ); 
	m_Line.Set(delta, this, this);

	SetNextThink( gpGlobals->curtime + 0.01f );
}


//-----------------------------------------------------------------------------
// Purpose: Remaps a given input range to an output range.
//-----------------------------------------------------------------------------
const int SF_MATH_REMAP_IGNORE_OUT_OF_RANGE = 1;

class CMathRemap : public CLogicalEntity
{
public:

	DECLARE_CLASS( CMathRemap, CLogicalEntity );

	void Spawn(void);

	// Keys
	float m_flInMin;
	float m_flInMax;
	float m_flOut1;		// Output value when input is m_fInMin
	float m_flOut2;		// Output value when input is m_fInMax

	// Inputs
	void InputValue( inputdata_t &inputdata );

	// Outputs
	COutputFloat m_OutValue;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(math_remap, CMathRemap);


BEGIN_DATADESC( CMathRemap )

	DEFINE_INPUTFUNC(FIELD_FLOAT, "InValue", InputValue ),

	DEFINE_OUTPUT(m_OutValue, "OutValue"),

	DEFINE_KEYFIELD(m_flInMin, FIELD_FLOAT, "in1"),
	DEFINE_KEYFIELD(m_flInMax, FIELD_FLOAT, "in2"),
	DEFINE_KEYFIELD(m_flOut1, FIELD_FLOAT, "out1"),
	DEFINE_KEYFIELD(m_flOut2, FIELD_FLOAT, "out2"),

END_DATADESC()




//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMathRemap::Spawn(void)
{
	//
	// Avoid a divide by zero in ValueChanged.
	//
	if (m_flInMin == m_flInMax)
	{
		m_flInMin = 0;
		m_flInMax = 1;
	}

	//
	// Make sure min and max are set properly relative to one another.
	//
	if (m_flInMin > m_flInMax)
	{
		float flTemp = m_flInMin;
		m_flInMin = m_flInMax;
		m_flInMax = flTemp;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Input handler that is called when the input value changes.
//-----------------------------------------------------------------------------
void CMathRemap::InputValue( inputdata_t &inputdata )
{
	float flValue = inputdata.value.Float();

	//
	// Disallow out-of-range input values to avoid out-of-range output values.
	//
	float flClampValue = clamp(flValue, m_flInMin, m_flInMax);

	if ((flClampValue == flValue) || !FBitSet(m_spawnflags, SF_MATH_REMAP_IGNORE_OUT_OF_RANGE))
	{
		//
		// Remap the input value to the desired output range and update the output.
		//
		float flRemappedValue = m_flOut1 + (((flValue - m_flInMin) * (m_flOut2 - m_flOut1)) / (m_flInMax - m_flInMin));
		m_OutValue.Set(flRemappedValue, inputdata.pActivator, this);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Remaps a given input range to an output range.
//-----------------------------------------------------------------------------
const int SF_COLOR_BLEND_IGNORE_OUT_OF_RANGE = 1;

class CMathColorBlend : public CLogicalEntity
{
public:

	DECLARE_CLASS( CMathColorBlend, CLogicalEntity );

	void Spawn(void);

	// Keys
	float m_flInMin;
	float m_flInMax;
	color32 m_OutColor1;		// Output color when input is m_fInMin
	color32 m_OutColor2;		// Output color when input is m_fInMax

	// Inputs
	void InputValue( inputdata_t &inputdata );

	// Outputs
	COutputColor32 m_OutValue;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(math_colorblend, CMathColorBlend);


BEGIN_DATADESC( CMathColorBlend )

	DEFINE_INPUTFUNC(FIELD_FLOAT, "InValue", InputValue ),

	DEFINE_OUTPUT(m_OutValue, "OutColor"),

	DEFINE_KEYFIELD(m_flInMin, FIELD_FLOAT, "inmin"),
	DEFINE_KEYFIELD(m_flInMax, FIELD_FLOAT, "inmax"),
	DEFINE_KEYFIELD(m_OutColor1, FIELD_COLOR32, "colormin"),
	DEFINE_KEYFIELD(m_OutColor2, FIELD_COLOR32, "colormax"),

END_DATADESC()




//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMathColorBlend::Spawn(void)
{
	//
	// Avoid a divide by zero in ValueChanged.
	//
	if (m_flInMin == m_flInMax)
	{
		m_flInMin = 0;
		m_flInMax = 1;
	}

	//
	// Make sure min and max are set properly relative to one another.
	//
	if (m_flInMin > m_flInMax)
	{
		float flTemp = m_flInMin;
		m_flInMin = m_flInMax;
		m_flInMax = flTemp;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Input handler that is called when the input value changes.
//-----------------------------------------------------------------------------
void CMathColorBlend::InputValue( inputdata_t &inputdata )
{
	float flValue = inputdata.value.Float();

	//
	// Disallow out-of-range input values to avoid out-of-range output values.
	//
	float flClampValue = clamp(flValue, m_flInMin, m_flInMax);
	if ((flClampValue == flValue) || !FBitSet(m_spawnflags, SF_COLOR_BLEND_IGNORE_OUT_OF_RANGE))
	{
		//
		// Remap the input value to the desired output color and update the output.
		//
		color32 Color;
		Color.r = m_OutColor1.r + (((flClampValue - m_flInMin) * (m_OutColor2.r - m_OutColor1.r)) / (m_flInMax - m_flInMin));
		Color.g = m_OutColor1.g + (((flClampValue - m_flInMin) * (m_OutColor2.g - m_OutColor1.g)) / (m_flInMax - m_flInMin));
		Color.b = m_OutColor1.b + (((flClampValue - m_flInMin) * (m_OutColor2.b - m_OutColor1.b)) / (m_flInMax - m_flInMin));
		Color.a = m_OutColor1.a + (((flClampValue - m_flInMin) * (m_OutColor2.a - m_OutColor1.a)) / (m_flInMax - m_flInMin));

		m_OutValue.Set(Color, inputdata.pActivator, this);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Holds a global state that can be queried by other entities to change
//			their behavior, such as "predistaster".
//-----------------------------------------------------------------------------
const int SF_GLOBAL_SET = 1;	// Set global state to initial state on spawn

class CEnvGlobal : public CLogicalEntity
{
public:
	DECLARE_CLASS( CEnvGlobal, CLogicalEntity );

	void Spawn( void );

	// Input handlers
	void InputTurnOn( inputdata_t &inputdata );
	void InputTurnOff( inputdata_t &inputdata );
	void InputRemove( inputdata_t &inputdata );
	void InputToggle( inputdata_t &inputdata );

	int DrawDebugTextOverlays(void);

	DECLARE_DATADESC();
	
	string_t	m_globalstate;
	int			m_triggermode;
	int			m_initialstate;
};


BEGIN_DATADESC( CEnvGlobal )

	DEFINE_KEYFIELD( m_globalstate, FIELD_STRING, "globalstate" ),
	DEFINE_FIELD( m_triggermode, FIELD_INTEGER ),
	DEFINE_KEYFIELD( m_initialstate, FIELD_INTEGER, "initialstate" ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "TurnOn",	InputTurnOn ),
	DEFINE_INPUTFUNC( FIELD_VOID, "TurnOff", InputTurnOff ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Remove",	InputRemove ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle",	InputToggle ),

END_DATADESC()


LINK_ENTITY_TO_CLASS( env_global, CEnvGlobal );


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvGlobal::Spawn( void )
{
	if ( !m_globalstate )
	{
		UTIL_Remove( this );
		return;
	}
	if ( FBitSet( m_spawnflags, SF_GLOBAL_SET ) )
	{
		if ( !GlobalEntity_IsInTable( m_globalstate ) )
			GlobalEntity_Add( m_globalstate, gpGlobals->mapname, (GLOBALESTATE)m_initialstate );
	}
}


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CEnvGlobal::InputTurnOn( inputdata_t &inputdata )
{
	if ( GlobalEntity_IsInTable( m_globalstate ) )
	{
		GlobalEntity_SetState( m_globalstate, GLOBAL_ON );
	}
	else
	{
		GlobalEntity_Add( m_globalstate, gpGlobals->mapname, GLOBAL_ON );
	}
}


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CEnvGlobal::InputTurnOff( inputdata_t &inputdata )
{
	if ( GlobalEntity_IsInTable( m_globalstate ) )
	{
		GlobalEntity_SetState( m_globalstate, GLOBAL_OFF );
	}
	else
	{
		GlobalEntity_Add( m_globalstate, gpGlobals->mapname, GLOBAL_OFF );
	}
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CEnvGlobal::InputRemove( inputdata_t &inputdata )
{
	if ( GlobalEntity_IsInTable( m_globalstate ) )
	{
		GlobalEntity_SetState( m_globalstate, GLOBAL_DEAD );
	}
	else
	{
		GlobalEntity_Add( m_globalstate, gpGlobals->mapname, GLOBAL_DEAD );
	}
}


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CEnvGlobal::InputToggle( inputdata_t &inputdata )
{
	GLOBALESTATE oldState = GlobalEntity_GetState( m_globalstate );
	GLOBALESTATE newState;

	if ( oldState == GLOBAL_ON )
	{
		newState = GLOBAL_OFF;
	}
	else if ( oldState == GLOBAL_OFF )
	{
		newState = GLOBAL_ON;
	}
	else
	{
		return;
	}

	if ( GlobalEntity_IsInTable( m_globalstate ) )
	{
		GlobalEntity_SetState( m_globalstate, newState );
	}
	else
	{
		GlobalEntity_Add( m_globalstate, gpGlobals->mapname, newState );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Draw any debug text overlays
// Input  :
// Output : Current text offset from the top
//-----------------------------------------------------------------------------
int CEnvGlobal::DrawDebugTextOverlays(void) 
{
	// Skip AIClass debug overlays
	int text_offset = CBaseEntity::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT) 
	{
		char tempstr[512];
		Q_snprintf(tempstr,sizeof(tempstr),"State: %s",STRING(m_globalstate));
		NDebugOverlay::EntityText(entindex(),text_offset,tempstr,0);
		text_offset++;

		GLOBALESTATE nState = GlobalEntity_GetState( m_globalstate );

		switch( nState )
		{
		case GLOBAL_OFF:
			Q_strncpy(tempstr,"Value: OFF",sizeof(tempstr));
			break;

		case GLOBAL_ON:
			Q_strncpy(tempstr,"Value: ON",sizeof(tempstr));
			break;

		case GLOBAL_DEAD:
			Q_strncpy(tempstr,"Value: DEAD",sizeof(tempstr));
			break;
		}
		NDebugOverlay::EntityText(entindex(),text_offset,tempstr,0);
		text_offset++;
	}
	return text_offset;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
#define MS_MAX_TARGETS 32

const int SF_MULTI_INIT	= 1;

class CMultiSource : public CLogicalEntity
{
public:
	DECLARE_CLASS( CMultiSource, CLogicalEntity );

	void Spawn( );
	bool KeyValue( const char *szKeyName, const char *szValue );
	void Use( ::CBaseEntity *pActivator, ::CBaseEntity *pCaller, USE_TYPE useType, float value );
	int	ObjectCaps( void ) { return(BaseClass::ObjectCaps() | FCAP_MASTER); }
	bool IsTriggered( ::CBaseEntity *pActivator );
	void Register( void );

	DECLARE_DATADESC();

	EHANDLE		m_rgEntities[MS_MAX_TARGETS];
	int			m_rgTriggered[MS_MAX_TARGETS];

	COutputEvent m_OnTrigger;		// Fired when all connections are triggered.

	int			m_iTotal;
	string_t	m_globalstate;
};

BEGIN_DATADESC( CMultiSource )

	//!!!BUGBUG FIX
	DEFINE_ARRAY( m_rgEntities, FIELD_EHANDLE, MS_MAX_TARGETS ),
	DEFINE_ARRAY( m_rgTriggered, FIELD_INTEGER, MS_MAX_TARGETS ),
	DEFINE_FIELD( m_iTotal, FIELD_INTEGER ),

	DEFINE_KEYFIELD( m_globalstate, FIELD_STRING, "globalstate" ),

	// Function pointers
	DEFINE_FUNCTION( Register ),

	// Outputs
	DEFINE_OUTPUT(m_OnTrigger, "OnTrigger"),

END_DATADESC()


LINK_ENTITY_TO_CLASS( multisource, CMultiSource );


//-----------------------------------------------------------------------------
// Purpose: Cache user entity field values until spawn is called.
// Input  : szKeyName - Key to handle.
//			szValue - Value for key.
// Output : Returns true if the key was handled, false if not.
//-----------------------------------------------------------------------------
bool CMultiSource::KeyValue( const char *szKeyName, const char *szValue )
{
	if (	FStrEq(szKeyName, "style") ||
				FStrEq(szKeyName, "height") ||
				FStrEq(szKeyName, "killtarget") ||
				FStrEq(szKeyName, "value1") ||
				FStrEq(szKeyName, "value2") ||
				FStrEq(szKeyName, "value3"))
	{
	}
	else
	{
		return BaseClass::KeyValue( szKeyName, szValue );
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMultiSource::Spawn()
{ 
	SetNextThink( gpGlobals->curtime + 0.1f );
	m_spawnflags |= SF_MULTI_INIT;	// Until it's initialized
	SetThink(&CMultiSource::Register);
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pActivator - 
//			pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
void CMultiSource::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{ 
	int i = 0;

	// Find the entity in our list
	while (i < m_iTotal)
		if ( m_rgEntities[i++] == pCaller )
			break;

	// if we didn't find it, report error and leave
	if (i > m_iTotal)
	{
		Warning("MultiSrc: Used by non member %s.\n", pCaller->edict() ? pCaller->GetClassname() : "<logical entity>");
		return;	
	}

	// CONSIDER: a Use input to the multisource always toggles.  Could check useType for ON/OFF/TOGGLE

	m_rgTriggered[i-1] ^= 1;

	// 
	if ( IsTriggered( pActivator ) )
	{
		DevMsg( 2, "Multisource %s enabled (%d inputs)\n", GetDebugName(), m_iTotal );
		USE_TYPE useType = USE_TOGGLE;
		if ( m_globalstate != NULL_STRING )
			useType = USE_ON;

		m_OnTrigger.FireOutput(pActivator, this);
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CMultiSource::IsTriggered( CBaseEntity * )
{
	// Is everything triggered?
	int i = 0;

	// Still initializing?
	if ( m_spawnflags & SF_MULTI_INIT )
		return 0;

	while (i < m_iTotal)
	{
		if (m_rgTriggered[i] == 0)
			break;
		i++;
	}

	if (i == m_iTotal)
	{
		if ( !m_globalstate || GlobalEntity_GetState( m_globalstate ) == GLOBAL_ON )
			return 1;
	}
	
	return 0;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMultiSource::Register(void)
{ 
	CBaseEntity *pTarget = NULL;

	m_iTotal = 0;
	memset( m_rgEntities, 0, MS_MAX_TARGETS * sizeof(EHANDLE) );

	SetThink(&CMultiSource::SUB_DoNothing);

	// search for all entities which target this multisource (m_iName)
	// dvsents2: port multisource to entity I/O!

	pTarget = gEntList.FindEntityByTarget( NULL, STRING(GetEntityName()) );

	while ( pTarget && (m_iTotal < MS_MAX_TARGETS) )
	{
		if ( pTarget )
			m_rgEntities[m_iTotal++] = pTarget;

		pTarget = gEntList.FindEntityByTarget( pTarget, STRING(GetEntityName()) );
	}

	pTarget = gEntList.FindEntityByClassname( NULL, "multi_manager" );
	while (pTarget && (m_iTotal < MS_MAX_TARGETS))
	{
		if ( pTarget && pTarget->HasTarget(GetEntityName()) )
			m_rgEntities[m_iTotal++] = pTarget;

		pTarget = gEntList.FindEntityByClassname( pTarget, "multi_manager" );
	}

	m_spawnflags &= ~SF_MULTI_INIT;
}


//-----------------------------------------------------------------------------
// Purpose: Holds a value that can be added to and subtracted from.
//-----------------------------------------------------------------------------
class CMathCounter : public CLogicalEntity
{
	DECLARE_CLASS( CMathCounter, CLogicalEntity );
private:
	float m_flMin;		// Minimum clamp value. If min and max are BOTH zero, no clamping is done.
	float m_flMax;		// Maximum clamp value.
	bool m_bHitMin;		// Set when we reach or go below our minimum value, cleared if we go above it again.
	bool m_bHitMax;		// Set when we reach or exceed our maximum value, cleared if we fall below it again.

	bool KeyValue(const char *szKeyName, const char *szValue);
	void Spawn(void);

	void UpdateOutValue(CBaseEntity *pActivator, float fNewValue);

	// Inputs
	void InputAdd( inputdata_t &inputdata );
	void InputDivide( inputdata_t &inputdata );
	void InputMultiply( inputdata_t &inputdata );
	void InputSetValue( inputdata_t &inputdata );
	void InputSetValueNoFire( inputdata_t &inputdata );
	void InputSubtract( inputdata_t &inputdata );
	void InputSetHitMax( inputdata_t &inputdata );
	void InputSetHitMin( inputdata_t &inputdata );

	// Outputs
	COutputFloat m_OutValue;
	COutputEvent m_OnHitMin;
	COutputEvent m_OnHitMax;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(math_counter, CMathCounter);


BEGIN_DATADESC( CMathCounter )

	DEFINE_FIELD(m_bHitMax, FIELD_BOOLEAN),
	DEFINE_FIELD(m_bHitMin, FIELD_BOOLEAN),

	// Keys
	DEFINE_KEYFIELD(m_flMin, FIELD_FLOAT, "min"),
	DEFINE_KEYFIELD(m_flMax, FIELD_FLOAT, "max"),

	// Inputs
	DEFINE_INPUTFUNC(FIELD_FLOAT, "Add", InputAdd),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "Divide", InputDivide),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "Multiply", InputMultiply),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetValue", InputSetValue),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetValueNoFire", InputSetValueNoFire),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "Subtract", InputSubtract),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetHitMax", InputSetHitMax),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetHitMin", InputSetHitMin),

	// Outputs
	DEFINE_OUTPUT(m_OutValue, "OutValue"),
	DEFINE_OUTPUT(m_OnHitMin, "OnHitMin"),
	DEFINE_OUTPUT(m_OnHitMax, "OnHitMax"),

END_DATADESC()




//-----------------------------------------------------------------------------
// Purpose: Handles key values from the BSP before spawn is called.
//-----------------------------------------------------------------------------
bool CMathCounter::KeyValue(const char *szKeyName, const char *szValue)
{
	//
	// Set the initial value of the counter.
	//
	if (!stricmp(szKeyName, "startvalue"))
	{
		m_OutValue.Init(atoi(szValue));
		return(true);
	}

	return(BaseClass::KeyValue(szKeyName, szValue));
}


//-----------------------------------------------------------------------------
// Purpose: Called before spawning, after key values have been set.
//-----------------------------------------------------------------------------
void CMathCounter::Spawn( void )
{
	//
	// Make sure max and min are ordered properly or clamp won't work.
	//
	if (m_flMin > m_flMax)
	{
		float flTemp = m_flMax;
		m_flMax = m_flMin;
		m_flMin = flTemp;
	}

	//
	// Clamp initial value to within the valid range.
	//
	if ((m_flMin != 0) || (m_flMax != 0))
	{
		float flStartValue = clamp(m_OutValue.Get(), m_flMin, m_flMax);
		m_OutValue.Init(flStartValue);
	}
}


//-----------------------------------------------------------------------------
// Change min/max
//-----------------------------------------------------------------------------
void CMathCounter::InputSetHitMax( inputdata_t &inputdata )
{
	m_flMax = inputdata.value.Float();
	if ( m_flMax < m_flMin )
	{
		m_flMin = m_flMax;
	}
	UpdateOutValue( inputdata.pActivator, m_OutValue.Get() );
}

void CMathCounter::InputSetHitMin( inputdata_t &inputdata )
{
	m_flMin = inputdata.value.Float();
	if ( m_flMax < m_flMin )
	{
		m_flMax = m_flMin;
	}
	UpdateOutValue( inputdata.pActivator, m_OutValue.Get() );
}

	
//-----------------------------------------------------------------------------
// Purpose: Input handler for adding to the accumulator value.
// Input  : Float value to add.
//-----------------------------------------------------------------------------
void CMathCounter::InputAdd( inputdata_t &inputdata )
{
	float fNewValue = m_OutValue.Get() + inputdata.value.Float();
	UpdateOutValue( inputdata.pActivator, fNewValue );
}


//-----------------------------------------------------------------------------
// Purpose: Input handler for multiplying the current value.
// Input  : Float value to multiply the value by.
//-----------------------------------------------------------------------------
void CMathCounter::InputDivide( inputdata_t &inputdata )
{
	if (inputdata.value.Float() != 0)
	{
		float fNewValue = m_OutValue.Get() / inputdata.value.Float();
		UpdateOutValue( inputdata.pActivator, fNewValue );
	}
	else
	{
		DevMsg( 1, "LEVEL DESIGN ERROR: Divide by zero in math_value\n" );
		UpdateOutValue( inputdata.pActivator, m_OutValue.Get() );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Input handler for multiplying the current value.
// Input  : Float value to multiply the value by.
//-----------------------------------------------------------------------------
void CMathCounter::InputMultiply( inputdata_t &inputdata )
{
	float fNewValue = m_OutValue.Get() * inputdata.value.Float();
	UpdateOutValue( inputdata.pActivator, fNewValue );
}


//-----------------------------------------------------------------------------
// Purpose: Input handler for updating the value.
// Input  : Float value to set.
//-----------------------------------------------------------------------------
void CMathCounter::InputSetValue( inputdata_t &inputdata )
{
	UpdateOutValue( inputdata.pActivator, inputdata.value.Float() );
}


//-----------------------------------------------------------------------------
// Purpose: Input handler for updating the value.
// Input  : Float value to set.
//-----------------------------------------------------------------------------
void CMathCounter::InputSetValueNoFire( inputdata_t &inputdata )
{
	float flNewValue = inputdata.value.Float();
	if (( m_flMin != 0 ) || (m_flMax != 0 ))
	{
		flNewValue = clamp(flNewValue, m_flMin, m_flMax);
	}

	m_OutValue.Init( flNewValue );
}


//-----------------------------------------------------------------------------
// Purpose: Input handler for subtracting from the current value.
// Input  : Float value to subtract.
//-----------------------------------------------------------------------------
void CMathCounter::InputSubtract( inputdata_t &inputdata )
{
	float fNewValue = m_OutValue.Get() - inputdata.value.Float();
	UpdateOutValue( inputdata.pActivator, fNewValue );
}


//-----------------------------------------------------------------------------
// Purpose: Sets the value to the new value, clamping and firing the output value.
// Input  : fNewValue - Value to set.
//-----------------------------------------------------------------------------
void CMathCounter::UpdateOutValue(CBaseEntity *pActivator, float fNewValue)
{
	if ((m_flMin != 0) || (m_flMax != 0))
	{
		//
		// Fire an output any time we reach or exceed our maximum value.
		//
		if ( fNewValue >= m_flMax )
		{
			if ( !m_bHitMax )
			{
				m_bHitMax = true;
				m_OnHitMax.FireOutput( pActivator, this );
			}
		}
		else
		{
			m_bHitMax = false;
		}

		//
		// Fire an output any time we reach or go below our minimum value.
		//
		if ( fNewValue <= m_flMin )
		{
			if ( !m_bHitMin )
			{
				m_bHitMin = true;
				m_OnHitMin.FireOutput( pActivator, this );
			}
		}
		else
		{
			m_bHitMin = false;
		}

		fNewValue = clamp(fNewValue, m_flMin, m_flMax);
	}

	m_OutValue.Set(fNewValue, pActivator, this);
}



//-----------------------------------------------------------------------------
// Purpose: Compares a single string input to up to 16 case values, firing an
//			output corresponding to the case value that matched, or a default
//			output if the input value didn't match any of the case values.
//
//			This can also be used to fire a random output from a set of outputs.
//-----------------------------------------------------------------------------
#define MAX_LOGIC_CASES 16

class CLogicCase : public CLogicalEntity
{
	DECLARE_CLASS( CLogicCase, CLogicalEntity );
private:
	string_t m_nCase[MAX_LOGIC_CASES];

	void Spawn(void);

	// Inputs
	void InputValue( inputdata_t &inputdata );
	void InputPickRandom( inputdata_t &inputdata );

	// Outputs
	COutputEvent m_OnCase[MAX_LOGIC_CASES];		// Fired when the input value matches one of the case values.
	COutputVariant m_OnDefault;					// Fired when no match was found.

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(logic_case, CLogicCase);


BEGIN_DATADESC( CLogicCase )

// Silence, Classcheck!
//	DEFINE_ARRAY( m_nCase, FIELD_STRING, MAX_LOGIC_CASES ),

	// Keys
	DEFINE_KEYFIELD(m_nCase[0], FIELD_STRING, "Case01"),
	DEFINE_KEYFIELD(m_nCase[1], FIELD_STRING, "Case02"),
	DEFINE_KEYFIELD(m_nCase[2], FIELD_STRING, "Case03"),
	DEFINE_KEYFIELD(m_nCase[3], FIELD_STRING, "Case04"),
	DEFINE_KEYFIELD(m_nCase[4], FIELD_STRING, "Case05"),
	DEFINE_KEYFIELD(m_nCase[5], FIELD_STRING, "Case06"),
	DEFINE_KEYFIELD(m_nCase[6], FIELD_STRING, "Case07"),
	DEFINE_KEYFIELD(m_nCase[7], FIELD_STRING, "Case08"),
	DEFINE_KEYFIELD(m_nCase[8], FIELD_STRING, "Case09"),
	DEFINE_KEYFIELD(m_nCase[9], FIELD_STRING, "Case10"),
	DEFINE_KEYFIELD(m_nCase[10], FIELD_STRING, "Case11"),
	DEFINE_KEYFIELD(m_nCase[11], FIELD_STRING, "Case12"),
	DEFINE_KEYFIELD(m_nCase[12], FIELD_STRING, "Case13"),
	DEFINE_KEYFIELD(m_nCase[13], FIELD_STRING, "Case14"),
	DEFINE_KEYFIELD(m_nCase[14], FIELD_STRING, "Case15"),
	DEFINE_KEYFIELD(m_nCase[15], FIELD_STRING, "Case16"),

	// Inputs
	DEFINE_INPUTFUNC(FIELD_INPUT, "InValue", InputValue),
	DEFINE_INPUTFUNC(FIELD_VOID, "PickRandom", InputPickRandom),

	// Outputs
	DEFINE_OUTPUT(m_OnCase[0], "OnCase01"),
	DEFINE_OUTPUT(m_OnCase[1], "OnCase02"),
	DEFINE_OUTPUT(m_OnCase[2], "OnCase03"),
	DEFINE_OUTPUT(m_OnCase[3], "OnCase04"),
	DEFINE_OUTPUT(m_OnCase[4], "OnCase05"),
	DEFINE_OUTPUT(m_OnCase[5], "OnCase06"),
	DEFINE_OUTPUT(m_OnCase[6], "OnCase07"),
	DEFINE_OUTPUT(m_OnCase[7], "OnCase08"),
	DEFINE_OUTPUT(m_OnCase[8], "OnCase09"),
	DEFINE_OUTPUT(m_OnCase[9], "OnCase10"),
	DEFINE_OUTPUT(m_OnCase[10], "OnCase11"),
	DEFINE_OUTPUT(m_OnCase[11], "OnCase12"),
	DEFINE_OUTPUT(m_OnCase[12], "OnCase13"),
	DEFINE_OUTPUT(m_OnCase[13], "OnCase14"),
	DEFINE_OUTPUT(m_OnCase[14], "OnCase15"),
	DEFINE_OUTPUT(m_OnCase[15], "OnCase16"),

	DEFINE_OUTPUT(m_OnDefault, "OnDefault"),

END_DATADESC()




//-----------------------------------------------------------------------------
// Purpose: Called before spawning, after key values have been set.
//-----------------------------------------------------------------------------
void CLogicCase::Spawn( void )
{
}


//-----------------------------------------------------------------------------
// Purpose: Evaluates the new input value, firing the appropriate OnCaseX output
//			if the input value matches one of the "CaseX" keys.
// Input  : Value - Variant value to compare against the values of the case fields.
//				We use a variant so that we can convert any input type to a string.
//-----------------------------------------------------------------------------
void CLogicCase::InputValue( inputdata_t &inputdata )
{
	const char *pszValue = inputdata.value.String();
	for (int i = 0; i < MAX_LOGIC_CASES; i++)
	{
		if ((m_nCase[i] != NULL_STRING) && !stricmp(STRING(m_nCase[i]), pszValue))
		{
			m_OnCase[i].FireOutput( inputdata.pActivator, this );
			return;
		}
	}
	
	m_OnDefault.Set( inputdata.value, inputdata.pActivator, this );
}


//-----------------------------------------------------------------------------
// Purpose: Makes the case statement choose a case at random.
//-----------------------------------------------------------------------------
void CLogicCase::InputPickRandom( inputdata_t &inputdata )
{
	//
	// Count the number of valid cases, building a packed array
	// that maps 0..NumCases to the actual CaseX values.
	//
	// This allows our zany mappers to set up cases sparsely if they desire.
	//
	int nCaseMap[MAX_LOGIC_CASES];
	memset(nCaseMap, 0, sizeof(nCaseMap));

	int nNumCases = 0;
	for (int i = 0; i < MAX_LOGIC_CASES; i++)
	{
		if (m_OnCase[i].NumberOfElements() > 0)
		{
			nCaseMap[nNumCases] = i;
			nNumCases++;
		}
	}

	//
	// Choose a random case from the ones that were set up by the level designer.
	//
	if ( nNumCases > 0 )
	{
		int nRandom = random->RandomInt(0, nNumCases - 1);
		int nCase = nCaseMap[nRandom];

		Assert(nCase < MAX_LOGIC_CASES);

		if (nCase < MAX_LOGIC_CASES)
		{
			m_OnCase[nCase].FireOutput( inputdata.pActivator, this );
		}
	}
	else
	{
		DevMsg( 1, "Firing PickRandom input on logic_case %s with no cases set up\n", GetDebugName() );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Compares a floating point input to a predefined value, firing an
//			output to indicate the result of the comparison.
//-----------------------------------------------------------------------------
class CLogicCompare : public CLogicalEntity
{
	DECLARE_CLASS( CLogicCompare, CLogicalEntity );
private:
	// Inputs
	void InputSetValue( inputdata_t &inputdata );
	void InputSetValueCompare( inputdata_t &inputdata );
	void InputSetCompareValue( inputdata_t &inputdata );
	void InputCompare( inputdata_t &inputdata );

	void DoCompare(CBaseEntity *pActivator, float flInValue);

	float m_flInValue;					// Place to hold the last input value for a recomparison.
	float m_flCompareValue;				// The value to compare the input value against.

	// Outputs
	COutputFloat m_OnLessThan;			// Fired when the input value is less than the compare value.
	COutputFloat m_OnEqualTo;			// Fired when the input value is equal to the compare value.
	COutputFloat m_OnNotEqualTo;		// Fired when the input value is not equal to the compare value.
	COutputFloat m_OnGreaterThan;		// Fired when the input value is greater than the compare value.

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(logic_compare, CLogicCompare);


BEGIN_DATADESC( CLogicCompare )

	// Keys
	DEFINE_KEYFIELD(m_flCompareValue, FIELD_FLOAT, "CompareValue"),
	DEFINE_KEYFIELD(m_flInValue, FIELD_FLOAT, "InitialValue"),

	// Inputs
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetValue", InputSetValue),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetValueCompare", InputSetValueCompare),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetCompareValue", InputSetCompareValue),
	DEFINE_INPUTFUNC(FIELD_VOID, "Compare", InputCompare),

	// Outputs
	DEFINE_OUTPUT(m_OnEqualTo, "OnEqualTo"),
	DEFINE_OUTPUT(m_OnNotEqualTo, "OnNotEqualTo"),
	DEFINE_OUTPUT(m_OnGreaterThan, "OnGreaterThan"),
	DEFINE_OUTPUT(m_OnLessThan, "OnLessThan"),

END_DATADESC()




//-----------------------------------------------------------------------------
// Purpose: Input handler for a new input value without performing a comparison.
//-----------------------------------------------------------------------------
void CLogicCompare::InputSetValue( inputdata_t &inputdata )
{
	m_flInValue = inputdata.value.Float();
}


//-----------------------------------------------------------------------------
// Purpose: Input handler for a setting a new value and doing the comparison.
//-----------------------------------------------------------------------------
void CLogicCompare::InputSetValueCompare( inputdata_t &inputdata )
{
	m_flInValue = inputdata.value.Float();
	DoCompare( inputdata.pActivator, m_flInValue );
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for a new input value without performing a comparison.
//-----------------------------------------------------------------------------
void CLogicCompare::InputSetCompareValue( inputdata_t &inputdata )
{
	m_flCompareValue = inputdata.value.Float();
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for forcing a recompare of the last input value.
//-----------------------------------------------------------------------------
void CLogicCompare::InputCompare( inputdata_t &inputdata )
{
	DoCompare( inputdata.pActivator, m_flInValue );
}


//-----------------------------------------------------------------------------
// Purpose: Compares the input value to the compare value, firing the appropriate
//			output(s) based on the comparison result.
// Input  : flInValue - Value to compare against the comparison value.
//-----------------------------------------------------------------------------
void CLogicCompare::DoCompare(CBaseEntity *pActivator, float flInValue)
{
	if (flInValue == m_flCompareValue)
	{
		m_OnEqualTo.Set(flInValue, pActivator, this);
	}
	else
	{
		m_OnNotEqualTo.Set(flInValue, pActivator, this);

		if (flInValue > m_flCompareValue)
		{
			m_OnGreaterThan.Set(flInValue, pActivator, this);
		}
		else
		{
			m_OnLessThan.Set(flInValue, pActivator, this);
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Tests a boolean value, firing an output to indicate whether the
//			value was true or false.
//-----------------------------------------------------------------------------
class CLogicBranch : public CLogicalEntity
{
	DECLARE_CLASS( CLogicBranch, CLogicalEntity );

private:
	// Inputs
	void InputSetValue( inputdata_t &inputdata );
	void InputSetValueTest( inputdata_t &inputdata );
	void InputToggle( inputdata_t &inputdata );
	void InputToggleTest( inputdata_t &inputdata );
	void InputTest( inputdata_t &inputdata );

	void DoTest(CBaseEntity *pActivator, bool bInValue);

	bool m_bInValue;					// Place to hold the last input value for a future test.

	// Outputs
	COutputEvent m_OnTrue;				// Fired when the value is true.
	COutputEvent m_OnFalse;				// Fired when the value is false.

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(logic_branch, CLogicBranch);


BEGIN_DATADESC( CLogicBranch )

	// Keys
	DEFINE_KEYFIELD(m_bInValue, FIELD_BOOLEAN, "InitialValue"),

	// Inputs
	DEFINE_INPUT(m_bInValue, FIELD_BOOLEAN, "SetValue"),
	DEFINE_INPUTFUNC(FIELD_BOOLEAN, "SetValueTest", InputSetValueTest),
	DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),
	DEFINE_INPUTFUNC(FIELD_VOID, "ToggleTest", InputToggleTest),
	DEFINE_INPUTFUNC(FIELD_VOID, "Test", InputTest),

	// Outputs
	DEFINE_OUTPUT(m_OnTrue, "OnTrue"),
	DEFINE_OUTPUT(m_OnFalse, "OnFalse"),

END_DATADESC()




//-----------------------------------------------------------------------------
// Purpose: Input handler to set a new input value and perform the test.
// Input  : Boolean value to set.
//-----------------------------------------------------------------------------
void CLogicBranch::InputSetValueTest( inputdata_t &inputdata )
{
	m_bInValue = inputdata.value.Bool();
	DoTest( inputdata.pActivator, m_bInValue );
}


//-----------------------------------------------------------------------------
// Purpose: Input handler for toggling the boolean value.
//-----------------------------------------------------------------------------
void CLogicBranch::InputToggle( inputdata_t &inputdata )
{
	m_bInValue = !m_bInValue;
}


//-----------------------------------------------------------------------------
// Purpose: Input handler for toggling the boolean value and then firing the
//			appropriate output based on the new value.
//-----------------------------------------------------------------------------
void CLogicBranch::InputToggleTest( inputdata_t &inputdata )
{
	m_bInValue = !m_bInValue;
	DoTest( inputdata.pActivator, m_bInValue );
}


//-----------------------------------------------------------------------------
// Purpose: Input handler for forcing a test of the last input value.
//-----------------------------------------------------------------------------
void CLogicBranch::InputTest( inputdata_t &inputdata )
{
	DoTest( inputdata.pActivator, m_bInValue );
}


//-----------------------------------------------------------------------------
// Purpose: Tests the last input value, firing the appropriate output based on
//			the test result.
// Input  : bInValue - 
//-----------------------------------------------------------------------------
void CLogicBranch::DoTest(CBaseEntity *pActivator, bool bInValue)
{
	if (bInValue)
	{
		m_OnTrue.FireOutput(pActivator, this);
	}
	else
	{
		m_OnFalse.FireOutput(pActivator, this);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Autosaves when triggered
//-----------------------------------------------------------------------------
class CLogicAutosave : public CLogicalEntity
{
	DECLARE_CLASS( CLogicAutosave, CLogicalEntity );

private:
	// Inputs
	void InputSave( inputdata_t &inputdata );

	DECLARE_DATADESC();
	bool m_bForceNewLevelUnit;
};

LINK_ENTITY_TO_CLASS(logic_autosave, CLogicAutosave);

BEGIN_DATADESC( CLogicAutosave )
	DEFINE_KEYFIELD( m_bForceNewLevelUnit, FIELD_BOOLEAN, "NewLevelUnit" ),
	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "Save", InputSave ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Save!
//-----------------------------------------------------------------------------
void CLogicAutosave::InputSave( inputdata_t &inputdata )
{
	if ( m_bForceNewLevelUnit )
	{
		engine->ClearSaveDir();
	}
	engine->ServerCommand( "autosave\n" );
}

class CLogicCollisionPair : public CLogicalEntity
{
	DECLARE_CLASS( CLogicCollisionPair, CLogicalEntity );
public:

	void EnableCollisions( bool bEnable )
	{
		IPhysicsObject *pPhysics0 = FindPhysicsObjectByName( STRING(m_nameAttach1) );
		IPhysicsObject *pPhysics1 = FindPhysicsObjectByName( STRING(m_nameAttach2) );

		if ( !pPhysics0 )
		{
			pPhysics0 = g_PhysWorldObject;
		}
		if ( !pPhysics1 )
		{
			pPhysics1 = g_PhysWorldObject;
		}

		if ( pPhysics0 != pPhysics1 )
		{
			m_disabled = !bEnable;
			if ( bEnable )
			{
				PhysEnableEntityCollisions( pPhysics0, pPhysics1 );
			}
			else
			{
				PhysDisableEntityCollisions( pPhysics0, pPhysics1 );
			}
		}
	}

	void Activate( void )
	{
		if ( m_disabled )
		{
			EnableCollisions( false );
		}
		BaseClass::Activate();
	}

	void InputDisableCollisions( inputdata_t &inputdata )
	{
		if ( m_disabled )
			return;
		EnableCollisions( false );
	}

	void InputEnableCollisions( inputdata_t &inputdata )
	{
		if ( !m_disabled )
			return;
		EnableCollisions( true );
	}
	// If Activate() becomes PostSpawn()
	//void OnRestore() { Activate(); }

	DECLARE_DATADESC();

private:
	string_t		m_nameAttach1;
	string_t		m_nameAttach2;
	bool			m_disabled;
};

BEGIN_DATADESC( CLogicCollisionPair )
	DEFINE_KEYFIELD( m_nameAttach1, FIELD_STRING, "attach1" ),
	DEFINE_KEYFIELD( m_nameAttach2, FIELD_STRING, "attach2" ),
	DEFINE_KEYFIELD( m_disabled, FIELD_BOOLEAN, "startdisabled" ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "DisableCollisions", InputDisableCollisions ),
	DEFINE_INPUTFUNC( FIELD_VOID, "EnableCollisions", InputEnableCollisions ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( logic_collision_pair, CLogicCollisionPair );
