//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef ANTLION_DUST_H
#define ANTLION_DUST_H

#include "te_particlesystem.h"

class CTEAntlionDust : public CTEParticleSystem
{
public:
	
	DECLARE_CLASS( CTEAntlionDust, CTEParticleSystem );
	DECLARE_SERVERCLASS();

					CTEAntlionDust( const char *name );
	virtual			~CTEAntlionDust( void );

	virtual void	Test( const Vector& current_origin, const QAngle& current_angles ) { };

	CNetworkVector( m_vecOrigin );
	CNetworkVar( QAngle, m_vecAngles );
};

void UTIL_CreateAntlionDust( const Vector &origin, const QAngle &angles );

#endif	//ANTLION_DUST_H