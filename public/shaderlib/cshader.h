//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CSHADER_H
#define CSHADER_H

#ifdef _WIN32		   
#pragma once
#endif

// uncomment this if you want to build for nv3x
//#define NV3X 1

// This is what all shaders include.
// CBaseShader will become CShader in this file.
#include "materialsystem/ishaderapi.h"
#include "utlvector.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterial.h"
#include "BaseShader.h"

#include "materialsystem/itexture.h"

// Included for convenience because they are used in a bunch of shaders
#include "materialsystem/imesh.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/materialsystem_config.h"
#include "shaderlib/ShaderDLL.h"


//-----------------------------------------------------------------------------
// Global interfaces
//-----------------------------------------------------------------------------
extern IMaterialSystemHardwareConfig* g_pHardwareConfig;
extern const MaterialSystem_Config_t *g_pConfig;


// Helper method
bool IsUsingGraphics();

#define SOFTWARE_VERTEX_SHADER(name)			\
	static void SoftwareVertexShader_ ## name( CMeshBuilder &meshBuilder, IMaterialVar **params, IShaderDynamicAPI* pShaderAPI )

#define FORWARD_DECLARE_SOFTWARE_VERTEX_SHADER(name)\
	static void SoftwareVertexShader_ ## name( CMeshBuilder &meshBuilder, IMaterialVar **params, IShaderDynamicAPI* pShaderAPI );

#define USE_SOFTWARE_VERTEX_SHADER(name) \
	m_SoftwareVertexShader = SoftwareVertexShader_ ## name

#define SHADER_INIT_PARAMS()			\
	virtual void OnInitShaderParams( IMaterialVar **params, const char *pMaterialName )

#define SHADER_FALLBACK			\
	virtual char const* GetFallbackShader( IMaterialVar** params ) const

// Typesafe flag setting
inline void CShader_SetFlags( IMaterialVar **params, MaterialVarFlags_t _flag )
{
	params[FLAGS]->SetIntValue( params[FLAGS]->GetIntValue() | (_flag) );
}

inline bool CShader_IsFlagSet( IMaterialVar **params, MaterialVarFlags_t _flag )
{
	return ((params[FLAGS]->GetIntValue() & (_flag) ) != 0);
}

#define SET_FLAGS( _flag )		CShader_SetFlags( params, _flag )
#define CLEAR_FLAGS( _flag )	params[FLAGS]->SetIntValue( params[FLAGS]->GetIntValue() & ~(_flag) )
#define IS_FLAG_SET( _flag )	CShader_IsFlagSet( params, _flag )
#define IS_FLAG_DEFINED( _flag ) ((params[FLAGS_DEFINED]->GetIntValue() & (_flag) ) != 0)

// Typesafe flag setting
inline void CShader_SetFlags2( IMaterialVar **params, MaterialVarFlags2_t _flag )
{
	params[FLAGS2]->SetIntValue( params[FLAGS2]->GetIntValue() | (_flag) );
}

inline bool CShader_IsFlag2Set( IMaterialVar **params, MaterialVarFlags2_t _flag )
{
	return ((params[FLAGS2]->GetIntValue() & (_flag) ) != 0);
}

#define SET_FLAGS2( _flag )		CShader_SetFlags2( params, _flag )
#define CLEAR_FLAGS2( _flag )	params[FLAGS2]->SetIntValue( params[FLAGS2]->GetIntValue() & ~(_flag) )
#define IS_FLAG2_SET( _flag )	CShader_IsFlag2Set( params, _flag )
#define IS_FLAG2_DEFINED( _flag ) ((params[FLAGS_DEFINED2]->GetIntValue() & (_flag) ) != 0)

#define __BEGIN_SHADER_INTERNAL(_baseclass, name, help) \
	namespace name \
	{\
		typedef _baseclass CBaseClass;\
		static const char *s_HelpString = help; \
		static const char *s_Name = #name; \
		class CShaderParam;\
		static CUtlVector<CShaderParam *> s_ShaderParams;\
		class CShaderParam\
		{\
		public:\
			CShaderParam( const char *pName, ShaderParamType_t type, const char *pDefaultParam, const char *pHelp )\
			{\
				m_Info.m_pName = pName;\
				m_Info.m_Type = type;\
				m_Info.m_pDefaultValue = pDefaultParam;\
				m_Info.m_pHelp = pHelp;\
				m_Index = NUM_SHADER_MATERIAL_VARS + s_ShaderParams.Count();\
				s_ShaderParams.AddToTail( this );\
			}\
			operator int()	\
			{\
				return m_Index;\
			}\
			const char *GetName()\
			{\
				return m_Info.m_pName;\
			}\
			ShaderParamType_t GetType()\
			{\
				return m_Info.m_Type;\
			}\
			const char *GetDefault()\
			{\
				return m_Info.m_pDefaultValue;\
			}\
			const char *GetHelp()\
			{\
				return m_Info.m_pHelp;\
			}\
		private:\
			ShaderParamInfo_t m_Info; \
			int m_Index;\
		};\

#define BEGIN_SHADER(name,help)		__BEGIN_SHADER_INTERNAL( CBaseShader, name, help )


#define BEGIN_SHADER_PARAMS

#define SHADER_PARAM(param,paramtype,paramdefault,paramhelp) \
	static CShaderParam param( "$" #param, paramtype, paramdefault, paramhelp ); \

#define END_SHADER_PARAMS \
	class CShader : public CBaseClass\
	{\
	public:
			
#define END_SHADER }; \
	static CShader s_ShaderInstance;\
} // namespace


#define SHADER_INIT						\
	char const* GetName() const			\
	{\
		return s_Name;					\
	}\
	int GetNumParams() const			\
	{\
		return CBaseClass::GetNumParams() + s_ShaderParams.Count();\
	}\
	char const* GetParamName( int param ) const				\
	{\
		int nBaseClassParamCount = CBaseClass::GetNumParams();	\
		if (param < nBaseClassParamCount)					\
			return CBaseClass::GetParamName(param);			\
		else												\
			return s_ShaderParams[param - nBaseClassParamCount]->GetName();	\
	}\
	char const* GetParamHelp( int param ) const \
	{\
		int nBaseClassParamCount = CBaseClass::GetNumParams();	\
		if (param < nBaseClassParamCount)				\
			return CBaseClass::GetParamHelp( param ); \
		else \
			return s_ShaderParams[param - nBaseClassParamCount]->GetHelp();		\
	}\
	ShaderParamType_t GetParamType( int param ) const \
	{\
		int nBaseClassParamCount = CBaseClass::GetNumParams();	\
		if (param < nBaseClassParamCount)				\
			return CBaseClass::GetParamType( param ); \
		else \
			return s_ShaderParams[param - nBaseClassParamCount]->GetType();		\
	}\
	char const* GetParamDefault( int param ) const \
	{\
		int nBaseClassParamCount = CBaseClass::GetNumParams();	\
		if (param < nBaseClassParamCount)				\
			return CBaseClass::GetParamDefault( param ); \
		else \
			return s_ShaderParams[param - nBaseClassParamCount]->GetDefault();		\
	}\
	void OnInitShaderInstance( IMaterialVar **params, IShaderInit *pShaderInit, const char *pMaterialName )

#define SHADER_DRAW \
	void OnDrawElements( IMaterialVar **params, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI )

#define SHADOW_STATE if (pShaderShadow)
#define DYNAMIC_STATE if (pShaderAPI)

#define ShaderWarning	if (pShaderShadow) Warning

//-----------------------------------------------------------------------------
// Used to easily define a shader which *always* falls back
//-----------------------------------------------------------------------------
#define DEFINE_FALLBACK_SHADER( _shadername, _fallbackshadername )	\
	BEGIN_SHADER( _shadername, "" )	\
	BEGIN_SHADER_PARAMS	\
	END_SHADER_PARAMS \
	SHADER_FALLBACK { return #_fallbackshadername; }	\
	SHADER_INIT {} \
	SHADER_DRAW {} \
	END_SHADER


//-----------------------------------------------------------------------------
// Used to easily define a shader which inherits from another shader
//-----------------------------------------------------------------------------

// FIXME: There's a compiler bug preventing this from working. 
// Maybe it'll work under VC7!

//#define BEGIN_INHERITED_SHADER( name, _baseclass, help ) \
//	namespace _baseclass \
//	{\
//	__BEGIN_SHADER_INTERNAL( _baseclass::CShader, name, help )

//#define END_INHERITED_SHADER END_SHADER }

//#define CHAIN_SHADER_INIT_PARAMS()	CBaseClass::OnInitShaderParams( params, pMaterialName )
//#define CHAIN_SHADER_FALLBACK()	 CBaseClass::GetFallbackShader( params )
//#define CHAIN_SHADER_INIT()	CBaseClass::OnInitShaderInstance( params, pShaderInit, pMaterialName )
//#define CHAIN_SHADER_DRAW()	CBaseClass::OnDrawElements( params, pShaderShadow, pShaderAPI )

// A dumbed-down version which does what I need now which works
// This version doesn't allow you to do chain *anything* down to the base class
#define BEGIN_INHERITED_SHADER( name, base, help ) \
	namespace base\
	{\
		namespace name\
		{\
			static const char *s_Name = #name; \
			static const char *s_HelpString = help;\
			class CShader : public base::CShader\
			{\
			public:\
				char const* GetName() const			\
				{\
					return s_Name;					\
				}\

#define END_INHERITED_SHADER END_SHADER }

#endif // CSHADER_H
