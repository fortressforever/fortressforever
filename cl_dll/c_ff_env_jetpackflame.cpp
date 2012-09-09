#include "cbase.h"
#include "particle_prototype.h"
#include "particle_util.h"
#include "baseparticleentity.h"
#include "ClientEffectPrecacheSystem.h"
#include "c_ff_player.h"
#include "c_ff_env_jetpackflame.h"
#include "iinput.h"
#include "iefx.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// tables
// Expose to the particle app
EXPOSE_PROTOTYPE_EFFECT(JetpackFlame, C_FFJetpackFlame);

IMPLEMENT_CLIENTCLASS_DT(C_FFJetpackFlame, DT_FFJetpackFlame, CFFJetpackFlame) 
	RecvPropInt(RECVINFO(m_bEmit), 0), 
END_RECV_TABLE() 

LINK_ENTITY_TO_CLASS(env_jetpackflame, C_FFJetpackFlame);

CLIENTEFFECT_REGISTER_BEGIN(PrecacheJetpackFlame) 
CLIENTEFFECT_MATERIAL("effects/flame") 
CLIENTEFFECT_REGISTER_END() 

C_FFJetpackFlame::C_FFJetpackFlame()
{
	
}

C_FFJetpackFlame::~C_FFJetpackFlame()
{
	BaseClass::Cleanup();
}


void C_FFJetpackFlame::Update(float fTimeDelta)
{
	DevMsg("C_FFJetpackFlame::Update\n");
	//BaseClass::Update(fTimeDelta);
	// return;
}