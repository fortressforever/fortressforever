#include "cbase.h"
#include "baseparticleentity.h"
#include "ff_env_jetpackflame.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


// tables
IMPLEMENT_SERVERCLASS_ST(CFFJetpackFlame, DT_FFJetpackFlame) 
	SendPropInt(SENDINFO(m_bEmit), 1, SPROP_UNSIGNED), 	// Declare our boolean state variable
END_SEND_TABLE() 

LINK_ENTITY_TO_CLASS(env_jetpackflame, CFFJetpackFlame);

BEGIN_DATADESC(CFFJetpackFlame) 
	DEFINE_FIELD(m_bEmit, FIELD_BOOLEAN), 
END_DATADESC() 

// everything else done