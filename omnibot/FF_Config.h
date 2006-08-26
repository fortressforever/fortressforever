////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-08-25 22:08:05 -0700 (Fri, 25 Aug 2006) $
// $LastChangedRevision: 1256 $
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
	FF_VERSION_0_4,
	FF_VERSION_0_5,
	FF_VERSION_0_6,
	FF_VERSION_LAST,
	FF_VERSION_LATEST = FF_VERSION_LAST - 1
} FF_Version;

#endif
