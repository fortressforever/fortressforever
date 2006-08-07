//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// $Header: $
// $NoKeywords: $
//
// Main header file for the serializers DLL
//
//=============================================================================

#ifndef IDMSERIALIZERS_H
#define IDMSERIALIZERS_H

#ifdef _WIN32
#pragma once
#endif

#include "appframework/iappsystem.h"


//-----------------------------------------------------------------------------
// Interface
//-----------------------------------------------------------------------------
class IDmSerializers : public IAppSystem
{
public:
};

#define DMSERIALIZERS_INTERFACE_VERSION		"VDmSerializers001"


#endif // DMSERIALIZERS_H


