//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
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
extern IMaterialSystemHardwareConfig *g_pHardwareConfig;
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

#define __BEGIN_SHADER_INTERNAL(_baseclass, name, help, flags) \
	namespace name \
	{\
		typedef _baseclass CBaseClass;\
		static const char *s_HelpString = help; \
		static const char *s_Name = #name; \
		static int s_nFlags = flags; \
		class CShaderParam;\
		static CUtlVector<CShaderParam *> s_ShaderParams;\
		static CShaderParam *s_pShaderParamOverrides[NUM_SHADER_MATERIAL_VARS];\
		class CShaderParam\
		{\
		public:\
			CShaderParam( ShaderMaterialVars_t var, ShaderParamType_t type, const char *pDefaultParam, const char *pHelp, int nFlags )\
			{\
				m_Info.m_pName = "override";\
				m_Info.m_Type = type;\
				m_Info.m_pDefaultValue = pDefaultParam;\
				m_Info.m_pHelp = pHelp;\
				m_Info.m_nFlags = nFlags;\
				AssertMsg( !s_pShaderParamOverrides[var], ( "Shader parameter override duplicately defined!" ) );\
				s_pShaderParamOverrides[var] = this;\
				m_Index = var;\
			}\
			CShaderParam( const char *pName, ShaderParamType_t type, const char *pDefaultParam, const char *pHelp, int nFlags )\
			{\
				m_Info.m_pName = pName;\
				m_Info.m_Type = type;\
				m_Info.m_pDefaultValue = pDefaultParam;\
				m_Info.m_pHelp = pHelp;\
				m_Info.m_nFlags = nFlags;\
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
			int GetFlags() const\
			{\
				return m_Info.m_nFlags;\
			}\
			const char *GetHelp()\
			{\
				return m_Info.m_pHelp;\
			}\
		private:\
			ShaderParamInfo_t m_Info; \
			int m_Index;\
		};\

#define BEGIN_SHADER(name,help)	__BEGIN_SHADER_INTERNAL( CBaseShader, name, help, 0 )
#define BEGIN_SHADER_FLAGS(name,help,flags)	__BEGIN_SHADER_INTERNAL( CBaseShader, name, help, flags )

#define BEGIN_SHADER_PARAMS

#define SHADER_PARAM( param, paramtype, paramdefault, paramhelp ) \
	static CShaderParam param( "$" #param, paramtype, paramdefault, paramhelp, 0 );

#define SHADER_PARAM_FLAGS( param, paramtype, paramdefault, paramhelp, flags ) \
	static CShaderParam param( "$" #param, paramtype, paramdefault, paramhelp, flags );

#define SHADER_PARAM_OVERRIDE( param, paramtype, paramdefault, paramhelp, flags ) \
	static CShaderParam param( (ShaderMaterialVars_t)param, paramtype, paramdefault, paramhelp, flags );

#define END_SHADER_PARAMS \
	class CShader : public CBaseClass\
	{\
	public:
			
#define END_SHADER }; \
	static CShader s_ShaderInstance;\
} // namespace


#define SHADER_INIT						\
	char const* GetName() const			\
	{									\
		return s_Name;					\
	}									\
	int GetFlags() const				\
	{									\
		return s_nFlags;				\
	}									\
	int GetNumParams() const			\
	{\
		return CBaseClass::GetNumParams() + s_ShaderParams.Count();\
	}\
	char const* GetParamName( int param ) const \
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
		if (param < nBaseClassParamCount)						\
		{														\
			if ( !s_pShaderParamOverrides[param] )				\
				return CBaseClass::GetParamHelp( param );		\
			else												\
				return s_pShaderParamOverrides[param]->GetHelp(); \
		}														\
		else													\
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
		if (param < nBaseClassParamCount)						\
		{														\
			if ( !s_pShaderParamOverrides[param] )				\
				return CBaseClass::GetParamDefault( param );	\
			else												\
				return s_pShaderParamOverrides[param]->GetDefault(); \
		}														\
		else													\
			return s_ShaderParams[param - nBaseClassParamCount]->GetDefault();	\
	}\
	int GetParamFlags( int param ) const \
	{\
		int nBaseClassParamCount = CBaseClass::GetNumParams();	\
		if (param < nBaseClassParamCount)						\
		{														\
			if ( !s_pShaderParamOverrides[param] )				\
				return CBaseClass::GetParamFlags( param );		\
			else												\
				return s_pShaderParamOverrides[param]->GetFlags(); \
		}														\
		else													\
			return s_ShaderParams[param - nBaseClassParamCount]->GetFlags(); \
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
#define BEGIN_INHERITED_SHADER_FLAGS( _name, _base, _help, _flags ) \
	namespace _base\
	{\
		namespace _name\
		{\
			static const char *s_Name = #_name; \
			static const char *s_HelpString = _help;\
			static int s_nFlags = _flags;\
			class CShader : public _base::CShader\
			{\
			public:\
				char const* GetName() const			\
				{									\
					return s_Name;					\
				}									\
				int GetFlags() const				\
				{									\
					return s_nFlags;				\
				}

#define BEGIN_INHERITED_SHADER( _name, _base, _help ) BEGIN_INHERITED_SHADER_FLAGS( _name, _base, _help, 0 )
#define END_INHERITED_SHADER END_SHADER }

#ifdef _DEBUG

// psh ## shader is used here to generate a warning if you don't ever call SET_DYNAMIC_PIXEL_SHADER
#define DECLARE_DYNAMIC_PIXEL_SHADER( shader ) \
	int declaredynpixshader_ ## shader ## _missingcurlybraces = 0; \
	declaredynpixshader_ ## shader ## _missingcurlybraces = declaredynpixshader_ ## shader ## _missingcurlybraces; \
	shader ## _Dynamic_Index _pshIndex; \
	int psh ## shader = 0

// vsh ## shader is used here to generate a warning if you don't ever call SET_DYNAMIC_VERTEX_SHADER
#define DECLARE_DYNAMIC_VERTEX_SHADER( shader ) \
	int declaredynvertshader_ ## shader ## _missingcurlybraces = 0; \
	declaredynvertshader_ ## shader ## _missingcurlybraces = declaredynvertshader_ ## shader ## _missingcurlybraces; \
	shader ## _Dynamic_Index _vshIndex; \
	int vsh ## shader = 0


// psh ## shader is used here to generate a warning if you don't ever call SET_STATIC_PIXEL_SHADER
#define DECLARE_STATIC_PIXEL_SHADER( shader ) \
	int declarestaticpixshader_ ## shader ## _missingcurlybraces = 0; \
	declarestaticpixshader_ ## shader ## _missingcurlybraces = declarestaticpixshader_ ## shader ## _missingcurlybraces; \
	shader ## _Static_Index _pshIndex; \
	int psh ## shader = 0

// vsh ## shader is used here to generate a warning if you don't ever call SET_STATIC_VERTEX_SHADER
#define DECLARE_STATIC_VERTEX_SHADER( shader ) \
	int declarestaticvertshader_ ## shader ## _missingcurlybraces = 0; \
	declarestaticvertshader_ ## shader ## _missingcurlybraces = declarestaticvertshader_ ## shader ## _missingcurlybraces; \
	shader ## _Static_Index _vshIndex; \
	int vsh ## shader = 0


// psh_forgot_to_set_dynamic_ ## var is used to make sure that you set all
// all combos.  If you don't, you will get an undefined variable used error 
// in the SET_DYNAMIC_PIXEL_SHADER block.
#define SET_DYNAMIC_PIXEL_SHADER_COMBO( var, val ) \
	int dynpixshadercombo_ ## var ## _missingcurlybraces = 0; \
	dynpixshadercombo_ ## var ## _missingcurlybraces = dynpixshadercombo_ ## var ## _missingcurlybraces; \
	_pshIndex.Set ## var( ( val ) ); \
	int psh_forgot_to_set_dynamic_ ## var = 0

// vsh_forgot_to_set_dynamic_ ## var is used to make sure that you set all
// all combos.  If you don't, you will get an undefined variable used error 
// in the SET_DYNAMIC_VERTEX_SHADER block.
#define SET_DYNAMIC_VERTEX_SHADER_COMBO( var, val ) \
	int dynvertshadercombo_ ## var ## _missingcurlybraces = 0; \
	dynvertshadercombo_ ## var ## _missingcurlybraces = dynvertshadercombo_ ## var ## _missingcurlybraces; \
	_vshIndex.Set ## var( ( val ) ); \
	int vsh_forgot_to_set_dynamic_ ## var = 0


// psh_forgot_to_set_static_ ## var is used to make sure that you set all
// all combos.  If you don't, you will get an undefined variable used error 
// in the SET_STATIC_PIXEL_SHADER block.
#define SET_STATIC_PIXEL_SHADER_COMBO( var, val ) \
	int staticpixshadercombo_ ## var ## _missingcurlybraces = 0; \
	staticpixshadercombo_ ## var ## _missingcurlybraces = staticpixshadercombo_ ## var ## _missingcurlybraces; \
	_pshIndex.Set ## var( ( val ) ); \
	int psh_forgot_to_set_static_ ## var = 0

// vsh_forgot_to_set_static_ ## var is used to make sure that you set all
// all combos.  If you don't, you will get an undefined variable used error 
// in the SET_STATIC_VERTEX_SHADER block.
#define SET_STATIC_VERTEX_SHADER_COMBO( var, val ) \
	int staticvertshadercombo_ ## var ## _missingcurlybraces = 0; \
	staticvertshadercombo_ ## var ## _missingcurlybraces = staticvertshadercombo_ ## var ## _missingcurlybraces; \
	_vshIndex.Set ## var( ( val ) ); \
	int vsh_forgot_to_set_static_ ## var = 0


// psh_testAllCombos adds up all of the psh_forgot_to_set_dynamic_ ## var's from 
// SET_DYNAMIC_PIXEL_SHADER_COMBO so that an error is generated if they aren't set.
// psh_testAllCombos is set to itself to avoid an unused variable warning.
// psh ## shader being set to itself ensures that DECLARE_DYNAMIC_PIXEL_SHADER 
// was called for this particular shader.
#define SET_DYNAMIC_PIXEL_SHADER( shader ) \
	int dynamicpixshader_ ## shader ## _missingcurlybraces = 0; \
	dynamicpixshader_ ## shader ## _missingcurlybraces = dynamicpixshader_ ## shader ## _missingcurlybraces; \
	int psh_testAllCombos = shaderDynamicTest_ ## shader; \
	psh_testAllCombos = psh_testAllCombos; \
	psh ## shader = psh ## shader; \
	pShaderAPI->SetPixelShaderIndex( _pshIndex.GetIndex() )

// vsh_testAllCombos adds up all of the vsh_forgot_to_set_dynamic_ ## var's from 
// SET_DYNAMIC_VERTEX_SHADER_COMBO so that an error is generated if they aren't set.
// vsh_testAllCombos is set to itself to avoid an unused variable warning.
// vsh ## shader being set to itself ensures that DECLARE_DYNAMIC_VERTEX_SHADER 
// was called for this particular shader.
#define SET_DYNAMIC_VERTEX_SHADER( shader ) \
	int dynamicvertshader_ ## shader ## _missingcurlybraces = 0; \
	dynamicvertshader_ ## shader ## _missingcurlybraces = dynamicvertshader_ ## shader ## _missingcurlybraces; \
	int vsh_testAllCombos = shaderDynamicTest_ ## shader; \
	vsh_testAllCombos = vsh_testAllCombos; \
	vsh ## shader = vsh ## shader; \
	pShaderAPI->SetVertexShaderIndex( _vshIndex.GetIndex() )



// psh_testAllCombos adds up all of the psh_forgot_to_set_static_ ## var's from 
// SET_STATIC_PIXEL_SHADER_COMBO so that an error is generated if they aren't set.
// psh_testAllCombos is set to itself to avoid an unused variable warning.
// psh ## shader being set to itself ensures that DECLARE_STATIC_PIXEL_SHADER 
// was called for this particular shader.
#define SET_STATIC_PIXEL_SHADER( shader ) \
	int staticpixshader_ ## shader ## _missingcurlybraces = 0; \
	staticpixshader_ ## shader ## _missingcurlybraces = staticpixshader_ ## shader ## _missingcurlybraces; \
	int psh_testAllCombos = shaderStaticTest_ ## shader; \
	psh_testAllCombos = psh_testAllCombos; \
	psh ## shader = psh ## shader; \
	pShaderShadow->SetPixelShader( #shader, _pshIndex.GetIndex() )

// vsh_testAllCombos adds up all of the vsh_forgot_to_set_static_ ## var's from 
// SET_STATIC_VERTEX_SHADER_COMBO so that an error is generated if they aren't set.
// vsh_testAllCombos is set to itself to avoid an unused variable warning.
// vsh ## shader being set to itself ensures that DECLARE_STATIC_VERTEX_SHADER 
// was called for this particular shader.
#define SET_STATIC_VERTEX_SHADER( shader ) \
	int staticvertshader_ ## shader ## _missingcurlybraces = 0; \
	staticvertshader_ ## shader ## _missingcurlybraces = staticvertshader_ ## shader ## _missingcurlybraces; \
	int vsh_testAllCombos = shaderStaticTest_ ## shader; \
	vsh_testAllCombos = vsh_testAllCombos; \
	vsh ## shader = vsh ## shader; \
	pShaderShadow->SetVertexShader( #shader, _vshIndex.GetIndex() )

#else // #ifdef _DEBUG

#define DECLARE_DYNAMIC_PIXEL_SHADER( shader ) \
	shader ## _Dynamic_Index _pshIndex

#define DECLARE_DYNAMIC_VERTEX_SHADER( shader ) \
	shader ## _Dynamic_Index _vshIndex


#define DECLARE_STATIC_PIXEL_SHADER( shader ) \
	shader ## _Static_Index _pshIndex

#define DECLARE_STATIC_VERTEX_SHADER( shader ) \
	shader ## _Static_Index _vshIndex


#define SET_DYNAMIC_PIXEL_SHADER_COMBO( var, val ) \
	_pshIndex.Set ## var( ( val ) )

#define SET_DYNAMIC_VERTEX_SHADER_COMBO( var, val ) \
	_vshIndex.Set ## var( ( val ) )


#define SET_STATIC_PIXEL_SHADER_COMBO( var, val ) \
	_pshIndex.Set ## var( ( val ) )

#define SET_STATIC_VERTEX_SHADER_COMBO( var, val ) \
	_vshIndex.Set ## var( ( val ) )


#define SET_DYNAMIC_PIXEL_SHADER( shader ) \
	pShaderAPI->SetPixelShaderIndex( _pshIndex.GetIndex() )

#define SET_DYNAMIC_VERTEX_SHADER( shader ) \
	pShaderAPI->SetVertexShaderIndex( _vshIndex.GetIndex() )



#define SET_STATIC_PIXEL_SHADER( shader ) \
	pShaderShadow->SetPixelShader( #shader, _pshIndex.GetIndex() )

#define SET_STATIC_VERTEX_SHADER( shader ) \
	pShaderShadow->SetVertexShader( #shader, _vshIndex.GetIndex() )

#endif

#endif // CSHADER_H
