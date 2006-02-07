//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Base combat character with no AI
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ai_squadslot.h"
#include "ai_basenpc.h"
#include "stringregistry.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// Init static variables
//=============================================================================
CAI_GlobalNamespace CAI_BaseNPC::gm_SquadSlotNamespace;

//-----------------------------------------------------------------------------
// Purpose: Given and SquadSlot name, return the SquadSlot ID
//-----------------------------------------------------------------------------
int CAI_BaseNPC::GetSquadSlotID(const char* slotName)
{
	return gm_SquadSlotNamespace.SymbolToId(slotName);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CAI_BaseNPC::InitDefaultSquadSlotSR(void)
{
	gm_SquadSlotNamespace.AddSymbol( "SQUAD_SLOT_ATTACK1", AI_RemapToGlobal(SQUAD_SLOT_ATTACK1) );
	gm_SquadSlotNamespace.AddSymbol( "SQUAD_SLOT_ATTACK2", AI_RemapToGlobal(SQUAD_SLOT_ATTACK2) );
	gm_SquadSlotNamespace.AddSymbol( "SQUAD_SLOT_INVESTIGATE_SOUND", AI_RemapToGlobal(SQUAD_SLOT_INVESTIGATE_SOUND) );
}
