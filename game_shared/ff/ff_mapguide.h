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

#ifndef FF_MAPGUIDE
#define FF_MAPGUIDE

#ifdef CLIENT_DLL 
	#define CFFMapGuide C_FFMapGuide
	#include "c_baseentity.h"
#else	
	#include "baseentity.h"
#endif

class CFFMapGuide : public CBaseEntity
{
public:
	DECLARE_CLASS(CFFMapGuide, CBaseEntity);
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

	CFFMapGuide();

	void Spawn();
	void Precache();

	void SetSpawnFlags(int flags);

	CNetworkVar(float, m_flWait);
	CNetworkVar(float, m_flTime);

	string_t m_iNextMapguide;
	string_t m_iCurveEntity;

	virtual int ShouldTransmit(const CCheckTransmitInfo *pInfo);

	string_t	m_iNarrationFile;
};

#endif //FF_MAPGUIDE
