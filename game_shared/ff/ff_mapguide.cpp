/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file ff_mapguide.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date Sep. 01, 2005
/// @brief A map guide entity
/// 
/// Revisions
/// ---------
/// Sep. 01, 2005	Mirv: Initial Creation

#include "cbase.h"
#include "ff_mapguide.h"
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(FFMapGuide, DT_FFMapGuide) 

BEGIN_NETWORK_TABLE(CFFMapGuide, DT_FFMapGuide) 
#ifdef CLIENT_DLL
	RecvPropInt(RECVINFO(m_iSequence)),
	RecvPropFloat(RECVINFO(m_flTime)),
	RecvPropVector(RECVINFO(m_vecCurvePoint)),
#else
	SendPropInt(SENDINFO(m_iSequence)), 
	SendPropFloat(SENDINFO(m_flTime)),
	SendPropVector(SENDINFO(m_vecCurvePoint)),
#endif
END_NETWORK_TABLE() 

BEGIN_DATADESC(CFFMapGuide) 
	DEFINE_KEYFIELD(m_iSequence, FIELD_INTEGER, "order"),
	DEFINE_KEYFIELD(m_flTime, FIELD_FLOAT, "time"),
	DEFINE_KEYFIELD(m_vecCurvePoint, FIELD_VECTOR, "curvetowards"),
	DEFINE_KEYFIELD(m_iNarrationFile, FIELD_STRING, "narration"),
END_DATADESC();

LINK_ENTITY_TO_CLASS(info_ff_mapguide, CFFMapGuide);
PRECACHE_REGISTER(info_ff_mapguide);

CFFMapGuide::CFFMapGuide() 
{
	m_iSequence = 0;
	m_iNarrationFile = NULL_STRING;
}

void CFFMapGuide::Precache() 
{
	if (m_iNarrationFile != NULL_STRING) 
		PrecacheSound(STRING(m_iNarrationFile));
}

void CFFMapGuide::Spawn() 
{
	Precache();

	BaseClass::Spawn();

	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_NONE);
	AddSolidFlags(FSOLID_NOT_SOLID);
	
#ifdef GAME_DLL
	DevMsg("[SERVER] Spawned an ff_mapguide(%d) (%.1f %.1f %.1f)\n", m_iSequence); //, GetAbsAngles().x, GetAbsAngles().y, GetAbsAngles().z);
#else
	DevMsg("[CLIENT] Spawned an ff_mapguide(%d) (%.1f %.1f %.1f)\n", m_iSequence); //, GetAbsAngles().x, GetAbsAngles().y, GetAbsAngles().z);
#endif
}

void CFFMapGuide::SetSpawnFlags(int flags) 
{
}

// Only transmit to spectators
int CFFMapGuide::ShouldTransmit(const CCheckTransmitInfo *pInfo)
{
	return FL_EDICT_ALWAYS;
}