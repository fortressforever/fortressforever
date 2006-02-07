//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef HLTVCAMERA_H
#define HLTVCAMERA_H
#ifdef _WIN32
#pragma once
#endif

#include <igameevents.h>

class C_HLTVCamera : IGameEventListener2
{
public:
	C_HLTVCamera();
	virtual ~C_HLTVCamera();

	void Init();
	void Reset();
	void Shutdown();

	void CalcView(Vector& origin, QAngle& angles, float& fov);
	void FireGameEvent( IGameEvent *event );

	void SetMode(int iMode);
	void SetChaseCamParams( float flOffset, float flDistance, float flTheta, float flPhi  );
	void SpecNextPlayer( bool bInverse );
	void SpecNamedPlayer( const char *szPlayerName );

	int  GetObserverMode();	// returns current camera mode
	C_BaseEntity *GetObserverTarget();  // return primary target
	C_BaseEntity *GetCameraMan();  // return camera entity if any

	void CreateMove(CUserCmd *cmd);
			
protected:

	void CalcChaseCamView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );
	void CalcFixedView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );
	void CalcInEyeCamView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );
	void CalcRoamingView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov);

	int			m_nCameraMode; // current camera mode
	int			m_iCameraMan; // camera man entindex or 0
	Vector		m_vCamOrigin;  //current camera origin
	QAngle		m_aCamAngle;   //current camera angle
	int			m_iTraget1;	// first tracked target or 0
	int			m_iTraget2; // second tracked target or 0
	float		m_flFOV; // current FOV
	float		m_flOffset;  // z-offset from target origin
	float		m_flDistance; // distance to traget origin+offset
	float		m_flLastDistance; // too smooth distance
	float		m_flTheta; // view angle horizontal 
	float		m_flPhi; // view angle vertical
	float		m_flInertia; // camera inertia 0..100
};

extern C_HLTVCamera *HLTVCamera();	// get Singelton





#endif // HLTVCAMERA_H
