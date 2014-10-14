
// ff_mathackman.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "ff_mathackman.h"
#include "tier0/vprof.h"
#include "c_ff_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MATHACK_CHECKINTERVAL 60.0f

/////////////////////////////////////////////////////////////////////////////
CFFMathackManager _mathackman;

bool MathackModelLessFunc( const int &lhs, const int &rhs )	
{ 
	return lhs < rhs; 
}

/////////////////////////////////////////////////////////////////////////////
// CFFMathackModel
/////////////////////////////////////////////////////////////////////////////
CFFMathackModel::CFFMathackModel(int iModelIndex)
{
	m_iModelIndex = iModelIndex;
	m_flLastChecked = 0;
	m_bIsMathacked = false;
}

/////////////////////////////////////////////////////////////////////////////
void CFFMathackModel::CheckForMathack()
{
	VPROF_BUDGET( "CFFMathackModel::CheckForMathack", VPROF_BUDGETGROUP_FF_MATHACKDETECT );
	
	if (m_bIsMathacked)
		return;

	const model_t *pModel = modelinfo->GetModel( m_iModelIndex );
	if (pModel)
	{
		studiohdr_t *pStudioPtr = modelinfo->GetStudiomodel( pModel );

		if (pStudioPtr)
		{
			for (int i=0; i<pStudioPtr->numtextures; i++)
			{
				mstudiotexture_t *pTex = pStudioPtr->pTexture(i);
				if (pTex)
				{
					// there are way less texture paths (cdtexture) than textures, and there's no way to determine exactly
					// which cdtexture is associated with which texture
					// so just loop through all cdtextures for each texture and see if they exist
					for (int j=0; j<pStudioPtr->numcdtextures; j++)
					{
						char searchPath[MAX_PATH] = "";
						Q_strcat(searchPath, pStudioPtr->pCdtexture(j), MAX_PATH);
						Q_strcat(searchPath, pTex->pszName(), MAX_PATH);
						
						//Msg("[mathack] %s findmaterial: %s\n", pStudioPtr->pszName(), searchPath);

						IMaterial *pMat = materials->FindMaterial(searchPath, TEXTURE_GROUP_MODEL);
						if (pMat)
						{
							//Msg("[mathack] -> material found\n");
							int ignorezval = (int)(pMat->GetMaterialVarFlag( MATERIAL_VAR_IGNOREZ ));
							if (ignorezval != 0)
							{
								//DevMsg("[mathack] %s -> %s ignorez is true\n", pStudioPtr->pszName(), searchPath);

								m_bIsMathacked = true;
								ReportMathack( pStudioPtr->pszName() );
							}
						}
					}
				}
			}
		}
	}
	m_flLastChecked = gpGlobals->curtime;
}

void CFFMathackModel::ReportMathack( const char *pszModelName )
{
	VPROF_BUDGET( "CFFMathackModel::ReportMathack", VPROF_BUDGETGROUP_FF_MATHACKDETECT );

	C_FFPlayer *pLocalPlayer = C_FFPlayer::GetLocalFFPlayer();

	if (!pLocalPlayer)
		return;

	if (pLocalPlayer->m_bMathackDetected)
		return;
	
	pLocalPlayer->m_bMathackDetected = true;
}

/////////////////////////////////////////////////////////////////////////////
void CFFMathackModel::PrintToConsole()
{
	if (m_bIsMathacked)
		Msg("-> ");

	Msg("%i | %f | %d\n", m_iModelIndex, m_flLastChecked, m_bIsMathacked);
}

/////////////////////////////////////////////////////////////////////////////
// CFFMathackManager
/////////////////////////////////////////////////////////////////////////////
CFFMathackManager::CFFMathackManager()
{
	m_models.SetLessFunc( MathackModelLessFunc );
	Init();
}

/////////////////////////////////////////////////////////////////////////////
CFFMathackManager::~CFFMathackManager()
{
	Shutdown();
}

/////////////////////////////////////////////////////////////////////////////
void CFFMathackManager::AddMathackModel(int iModelIndex)
{
	VPROF_BUDGET( "CFFMathackManager::AddMathackModel", VPROF_BUDGETGROUP_FF_MATHACKDETECT );

	// check if the model of the specified id already exists
	if(m_models.IsValidIndex(m_models.Find(iModelIndex)))
		return;

	m_models.Insert( iModelIndex, CFFMathackModel( iModelIndex ) );
}

/////////////////////////////////////////////////////////////////////////////
void CFFMathackManager::Init()
{
	Shutdown();
}

/////////////////////////////////////////////////////////////////////////////
void CFFMathackManager::RemoveMathackModel(int iModelIndex)
{
	VPROF_BUDGET( "CFFMathackManager::RemoveMathackModel", VPROF_BUDGETGROUP_FF_MATHACKDETECT );

	// remove the model from the list
	unsigned short it = m_models.Find(iModelIndex);
	if(m_models.IsValidIndex(it))
		m_models.RemoveAt(it);
}

/////////////////////////////////////////////////////////////////////////////
void CFFMathackManager::Shutdown()
{
	m_models.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
void CFFMathackManager::Update()
{
	VPROF_BUDGET( "CFFMathackManager::Update", VPROF_BUDGETGROUP_FF_MATHACKDETECT );
	
	if (m_models.Count() <= 0)
		return;

	// check each item in the model list
	unsigned short it = m_models.FirstInorder();
	while(m_models.IsValidIndex(it))
	{
		if (!((m_models.Element(it)).m_bIsMathacked) && 
			((m_models.Element(it)).m_flLastChecked <= 0.0f || gpGlobals->curtime >= (m_models.Element(it)).m_flLastChecked + MATHACK_CHECKINTERVAL) )
		{
			(m_models.Element(it)).CheckForMathack();
		}
		it = m_models.NextInorder(it);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CFFMathackManager::PrintToConsole()
{
	VPROF_BUDGET( "CFFMathackManager::PrintToConsole", VPROF_BUDGETGROUP_FF_MATHACKDETECT );

	Msg("\n==MATHACK MANAGER==\n");
	// update each item in the schedule list
	unsigned short it = m_models.FirstInorder();
	while(m_models.IsValidIndex(it))
	{
		(m_models.Element(it)).PrintToConsole();

		it = m_models.NextInorder(it);
	}
}

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
CON_COMMAND(dumpmathack, "Print what the mathack manager has in it")
{
	_mathackman.PrintToConsole();
}
#endif
