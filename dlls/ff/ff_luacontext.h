
// ff_luacontext.h

//---------------------------------------------------------------------------
#ifndef FF_LUACONTEXT_H
#define FF_LUACONTEXT_H

//---------------------------------------------------------------------------
// includes
#ifndef UTLVECTOR_H
	#include "utlvector.h"
#endif
#ifndef VECTOR_H
	#include "vector.h"
#endif

//---------------------------------------------------------------------------
// foward declarations
class CBaseEntity;
class CFFBuildableObject;
class CFFDispenser;
class CFFSentryGun;
class CFFDetpack;
class CTeam;
class CFFTeam;
class CFFGrenadeBase;
class CBasePlayer;
class CFFPlayer;
class CFFInfoScript;
class CBeam;
class CTakeDamageInfo;

namespace luabind
{
	namespace adl
	{
		class object;
	}
}

//---------------------------------------------------------------------------
class CFFLuaSC
{
public:
	// 'structors
	CFFLuaSC() {}
	CFFLuaSC( int iArgs, ... );
	~CFFLuaSC();

public:
	// pushes a parameter in preperation for a function call
	void Push(float value);
	void Push(int value);
	void Push(bool value);
	void Push(const char *value);
	void Push(luabind::adl::object& luabindObject);
	void Push(CBaseEntity* pEntity);
	void Push(CFFBuildableObject* pEntity);
	void Push(CFFDispenser* pEntity);
	void Push(CFFSentryGun* pEntity);
	void Push(CFFDetpack* pEntity);
	void Push(CTeam* pEntity);
	void Push(CFFTeam* pEntity);
	void Push(CFFGrenadeBase* pEntity);
	void Push(CBasePlayer* pEntity);
	void Push(CFFPlayer* pEntity);
	void Push(CFFInfoScript* pEntity);
	void Push(CBeam* pEntity);
	void Push(Vector vector);
	void Push(QAngle angle);
	void Push(const CTakeDamageInfo* pInfo);

	// pushes a parameter by reference in preparation for a function call
	void PushRef(CTakeDamageInfo& info);
	void PushRef(luabind::adl::object& luabindObject);
	void PushRef(Vector &vector);
	void PushRef(QAngle &angle);

	// returns the number of parameters
	int GetNumParams() const { return m_params.Count(); }

	// clears the parameter list
	void ClearParams();

	// calls out to a script function. if pEntity is NULL, the szFunctionName
	// will be interpreted as a global function. returns true if the
	// script call was successful; otherwise, it returns false
	bool CallFunction(CBaseEntity* pEntity, const char* szFunctionName, const char *szTargetEntName = 0);

	// calls a global function
	bool CallFunction(const char* szFunctionName);

	// returns the number of return values
	int GetNumReturns() const { return m_returnVals.Count(); }

	// gets the return value
	bool	GetBool();
	float	GetFloat();
	int		GetInt();
	QAngle	GetQAngle();
	Vector	GetVector();
	luabind::adl::object* GetObject();

public:
	static void QuickCallFunction(CBaseEntity* pEntity, const char* szFunctionName);
	static void QuickCallFunction(const char* szFunctionName);

protected:
	void	SetParams( int iArgs, ... );

private:
	// private data
	CUtlVector<luabind::adl::object*>	m_params;			// parameters
	CUtlVector<luabind::adl::object*>	m_returnVals;		// return values
	char								m_szFunction[256];	// function called
};

//---------------------------------------------------------------------------
#endif
