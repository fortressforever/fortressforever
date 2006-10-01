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
	RecvPropFloat(RECVINFO(m_flWait)),
	RecvPropFloat(RECVINFO(m_flTime)),
#else
	SendPropFloat(SENDINFO(m_flWait)),
	SendPropFloat(SENDINFO(m_flTime)),
#endif
END_NETWORK_TABLE() 

BEGIN_DATADESC(CFFMapGuide)
	DEFINE_KEYFIELD(m_iNextMapguide, FIELD_STRING, "target"),
	DEFINE_KEYFIELD(m_flWait, FIELD_FLOAT, "wait"),
	DEFINE_KEYFIELD(m_flTime, FIELD_FLOAT, "time"),
	DEFINE_KEYFIELD(m_iCurveEntity, FIELD_VECTOR, "curvetowards"),
	DEFINE_KEYFIELD(m_iNarrationFile, FIELD_STRING, "narration"),
	//DEFINE_KEYFIELD(m_iKeyName, FIELD_STRING, "keyname"),
END_DATADESC();

LINK_ENTITY_TO_CLASS(path_mapguide, CFFMapGuide);
PRECACHE_REGISTER(path_mapguide);

CFFMapGuide::CFFMapGuide() 
{
	m_iNarrationFile = NULL_STRING;
	m_iCurveEntity = NULL_STRING;
	m_iNextMapguide = NULL_STRING;
	m_iKeyName = NULL_STRING;

	m_flWait = 10.0f;
	m_flTime = 10.0f;
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
	
#ifdef _DEBUG
#ifdef GAME_DLL
	DevMsg("[SERVER] Spawned an ff_mapguide (%s)\n", STRING(GetEntityName()));
#else
	DevMsg("[CLIENT] Spawned an ff_mapguide (%s)\n", "NULL");
#endif
#endif
}

void CFFMapGuide::SetSpawnFlags(int flags) 
{
}

// TODO: Only transmit to spectators
int CFFMapGuide::ShouldTransmit(const CCheckTransmitInfo *pInfo)
{
	return FL_EDICT_ALWAYS;
}