//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef MINMAX_H
#define MINMAX_H

#if defined( _WIN32 )
#pragma once
#endif

#ifndef valve_min
#define valve_min(a,b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef valve_max
#define valve_max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

#endif // MINMAX_H
