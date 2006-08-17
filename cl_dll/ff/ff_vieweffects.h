/********************************************************************
	created:	2006/08/17
	created:	17:8:2006   18:45
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\ff_vieweffects.h
	file path:	f:\ff-svn\code\trunk\cl_dll\ff
	file base:	ff_vieweffects
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#ifndef FF_VIEWEFFECTS_H
#define FF_VIEWEFFECTS_H

#ifdef WIN32
#pragma once
#endif

//=============================================================================
// Purpose: Our own FF view effects
//=============================================================================
abstract_class IFFViewEffects
{
public:
	// Initialize subsystem
	virtual void Init() = 0;

	// Initialize after each level change
	virtual void LevelInit() = 0;

	// Initialize after each level change
	virtual void Reset() = 0;

	// Render our view effects
	virtual void Render(int type, int width, int height) = 0;

	// A message for our view effect
	virtual void Message(bf_read &msg) = 0;
};

extern IFFViewEffects *ffvieweffects;

#define DECLARE_VIEWEFFECT(x)	static x g_##x(#x)

enum
{
	VIEWEFFECT_BEFOREHUD,
	VIEWEFFECT_AFTERHUD
};

#endif