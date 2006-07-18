////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-01-28 12:33:58 -0500 (Sat, 28 Jan 2006) $
// $LastChangedRevision: 1137 $
//
// File: FF Config
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __FF_EVENTS_H__
#define __FF_EVENTS_H__

#include "TF_Config.h"

#include "Omni-Bot_Types.h"
#include "Omni-Bot_Events.h"

typedef enum eFF_Version
{
	FF_VERSION_0_1 = 1,
	FF_VERSION_0_2,
	FF_VERSION_0_3,
	FF_VERSION_LAST,
	FF_VERSION_LATEST = FF_VERSION_LAST - 1
} FF_Version;

#endif
