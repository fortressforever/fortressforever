/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file ff_location.h
/// @author Patrick O'Leary [Mulchman]
/// @date 02/20/2006
/// @brief Location entity
/// 
/// Revisions
/// ---------
/// 02/20/2006	Initial version

#ifndef FF_LOCATION
#define FF_LOCATION

#include "triggers.h"

class CFFLocation : public CTriggerMultiple
{
public:
	DECLARE_CLASS( CFFLocation, CTriggerMultiple );
	DECLARE_DATADESC();

	CFFLocation( void );
	void Spawn( void );
	void MultiTouch( CBaseEntity *pOther );

private:
	int m_iTeam;
	string_t m_szAreaName;	

};

#endif //FF_LOCATION
