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
	RecvPropInt(RECVINFO(m_iSequence)) 
#else
	SendPropInt(SENDINFO(m_iSequence)), 
#endif
END_NETWORK_TABLE() 

BEGIN_DATADESC(CFFMapGuide) 

	DEFINE_KEYFIELD(m_iSequence, FIELD_INTEGER, "order"), 
	DEFINE_KEYFIELD(m_angDirection, FIELD_VECTOR, "direction"), 
	DEFINE_KEYFIELD(m_iNarrationFile, FIELD_STRING, "narration"), 

END_DATADESC();

LINK_ENTITY_TO_CLASS(ff_mapguide, CFFMapGuide);
PRECACHE_REGISTER(ff_mapguide);

CFFMapGuide::CFFMapGuide() 
{
	m_iSequence = 0;
	m_angDirection = QAngle(0, 0, 0);
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
	
	SetEffects(EF_NODRAW);

	SetAbsAngles(m_angDirection);

#ifdef GAME_DLL
	DevMsg("[SERVER] Spawned an ff_mapguide(%d) \n", m_iSequence);
#else
	DevMsg("[CLIENT] Spawned an ff_mapguide(%d) \n", m_iSequence);
#endif
}

void CFFMapGuide::SetSpawnFlags(int flags) 
{
	//m_spawnflags = flags;
}
