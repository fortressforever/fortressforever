
// ff_mathackman.h

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef UTLMAP_H
	#include "utlmap.h"
#endif

/////////////////////////////////////////////////////////////////////////////
class CFFMathackModel
{
public:
	// 'structors
	CFFMathackModel(int iModelIndex, studiohdr_t *pStudioPtr);

	CFFMathackModel() {}
	~CFFMathackModel() {}

	void PrintToConsole();
	void CheckForMathack();
	void ReportMathack();
	
	int m_iModelIndex;
	float m_flLastChecked;
	bool m_bIsMathacked;

private:
	studiohdr_t *m_pStudioPtr;
};

/////////////////////////////////////////////////////////////////////////////
class CFFMathackManager
{
public:
	// 'structors
	CFFMathackManager();
	~CFFMathackManager();

	void Init();
	void Shutdown();
	void Update();

	// adds a model
	void AddMathackModel(int iModelIndex, studiohdr_t *pStudioPtr);

	// removes a model
	void RemoveMathackModel(int iModelIndex);
	
	void PrintToConsole();

private:
	// list of schedules. key is the checksum of an identifying name; it
	// isnt necessarily the name of the lua function to call
	CUtlMap<int, CFFMathackModel>	m_models;
};

/////////////////////////////////////////////////////////////////////////////
extern CFFMathackManager _mathackman;

/////////////////////////////////////////////////////////////////////////////
