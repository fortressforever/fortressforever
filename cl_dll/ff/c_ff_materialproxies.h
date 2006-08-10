//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file c_ff_materialproxies.h
//	@author Patrick O'Leary (Mulchman)
//	@date 09/23/2005
//	@brief Fortress Forever Material Proxies
//
//	REVISIONS
//	---------
//	09/23/2005,	Mulchman: 
//		First created

#ifndef FF_MATERIALPROXIES_H
#define FF_MATERIALPROXIES_H

#include "materialsystem/IMaterialProxy.h"

// Forward declarations
class IMaterialVar;
enum MaterialVarType_t;
class C_BaseEntity;

//=============================================================================
//
//	class C_TeamColorMaterialProxy
//
//=============================================================================

// Generic TeamColor material proxy. Provides basic functionality
// for easily adding additional team colored proxies. Just supply
// an appropriate $value that will get modified and appropriate
// names of the team color values (in the .vmt).
class C_TeamColorMaterialProxy : public IMaterialProxy
{
public:
	C_TeamColorMaterialProxy( void );
	virtual ~C_TeamColorMaterialProxy( void );
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pC_BaseEntity );
	virtual void Release( void ) { delete this; }

private:	
	// Actual $ value we will be modifying in the .vmt to adjust
	// the team coloring
	IMaterialVar	*m_pValue;

	// Array of vectors to hold the team coloring values
	// that are read in from the .vmt
	// 0 = blue
	// 1 = red
	// 2 = yellow
	// 3 = green
	Vector			m_vecTeamColorVals[ 4 ];

protected:
	// This gets overridden by the super class
	const char**	m_ppszStrings;

};

//=============================================================================
//
//	class C_Color_TeamColorMaterialProxy
//
//=============================================================================
class C_Color_TeamColorMaterialProxy : public C_TeamColorMaterialProxy
{
public:
	C_Color_TeamColorMaterialProxy( void );

};

//=============================================================================
//
//	class C_Refract_TeamColorMaterialProxy
//
//=============================================================================
class C_Refract_TeamColorMaterialProxy : public C_TeamColorMaterialProxy
{
public:
	C_Refract_TeamColorMaterialProxy( void );

};

//=============================================================================
//
//	class C_GlowMaterialProxy
//
//=============================================================================
class C_GlowMaterialProxy : public IMaterialProxy
{
public:
	C_GlowMaterialProxy( void );
	virtual ~C_GlowMaterialProxy( void );
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pC_BaseEntity );
	virtual void Release( void ) { delete this; }

private:	
	// Actual $ value we will be modifying in the .vmt to adjust
	// the team coloring
	IMaterialVar	*m_pValue;
};

#endif // FF_MATERIALPROXIES_H
