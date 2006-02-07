//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef MPIVRAD_H
#define MPIVRAD_H
#ifdef _WIN32
#pragma once
#endif


// Called first thing in the exe.
void		VRAD_SetupMPI( int argc, char **argv );

void		RunMPIBuildFacelights(void);
void		RunMPIBuildVisLeafs(void);


// This handles disconnections. They're usually not fatal for the master.
void		HandleMPIDisconnect( int procID );


#endif // MPIVRAD_H
