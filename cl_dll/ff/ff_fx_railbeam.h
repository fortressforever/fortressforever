/********************************************************************
	created:	2006/07/04
	created:	4:7:2006   13:48
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\ff_fx_railbeam.h
	file path:	f:\ff-svn\code\trunk\cl_dll\ff
	file base:	ff_fx_railbeam
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Rail beam
*********************************************************************/

#ifndef FF_FX_RAILBEAM_H
#define FF_FX_RAILBEAM_H

#include "vector.h"
#include "clientsideeffects.h"

#include "fx.h"
#include "fx_sparks.h"
#include "fx_line.h"

void FX_RailBeam(int entindex, const Vector &target, float scale);

class IMaterial;

class CFFFXRailBeam : public CClientSideEffect
{
public:

	CFFFXRailBeam(const char *name, const Vector &start, const Vector &end, float scale);
	~CFFFXRailBeam();

	virtual void	Draw(double frametime);
	virtual bool	IsActive();
	virtual void	Destroy();
	virtual	void	Update(double frametime);

protected:

	IMaterial		*m_pMaterial;
	Vector			m_vecOrigin, m_vecTarget;
	float			m_flStartTime;
	float			m_flScale;
};

#endif