
// ff_mathackman.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "ff_mathackman.h"
#include "tier0/vprof.h"
#include "irc/ff_socks.h"
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
CFFMathackModel::CFFMathackModel(int iModelIndex, 
									studiohdr_t *pStudioPtr)
{
	m_iModelIndex = iModelIndex;
	m_pStudioPtr = pStudioPtr;
	m_flLastChecked = 0;
	m_bIsMathacked = false;
}

/////////////////////////////////////////////////////////////////////////////
void CFFMathackModel::CheckForMathack()
{
	VPROF_BUDGET( "CFFMathackModel::CheckForMathack", VPROF_BUDGETGROUP_FF_MATHACKDETECT );
	
	if (m_bIsMathacked)
		return;

	if (m_pStudioPtr)
	{
		for (int i=0; i<m_pStudioPtr->numtextures; i++)
		{
			mstudiotexture_t *pTex = m_pStudioPtr->pTexture(i);
			if (pTex)
			{
				// there are way less texture paths (cdtexture) than textures, and there's no way to determine exactly
				// which cdtexture is associated with which texture
				// so just loop through all cdtextures for each texture and see if they exist
				for (int j=0; j<m_pStudioPtr->numcdtextures; j++)
				{
					char searchPath[MAX_PATH] = "";
					Q_strcat(searchPath, m_pStudioPtr->pCdtexture(j), MAX_PATH);
					Q_strcat(searchPath, pTex->pszName(), MAX_PATH);
					
					//Msg("[mathack] %s findmaterial: %s\n", m_pStudioPtr->pszName(), searchPath);

					IMaterial *pMat = materials->FindMaterial(searchPath, TEXTURE_GROUP_MODEL);
					if (pMat)
					{
						//Msg("[mathack] -> material found\n");
						int ignorezval = (int)(pMat->GetMaterialVarFlag( MATERIAL_VAR_IGNOREZ ));
						if (ignorezval != 0)
						{
							//DevMsg("[mathack] %s -> %s ignorez is true\n", m_pStudioPtr->pszName(), searchPath);

							m_bIsMathacked = true;
							ReportMathack();
						}
					}
				}
			}
		}
	}
	m_flLastChecked = gpGlobals->curtime;
}

void CFFMathackModel::ReportMathack()
{
	VPROF_BUDGET( "CFFMathackModel::ReportMathack", VPROF_BUDGETGROUP_FF_MATHACKDETECT );

	C_FFPlayer *pLocalPlayer = C_FFPlayer::GetLocalFFPlayer();

	if (!pLocalPlayer)
		return;

	if (pLocalPlayer->m_bMathackDetected)
		return;
	
	pLocalPlayer->m_bMathackDetected = true;

	Socks sock;
	char buf[1024];
	
	// Open up a socket
	if (!sock.Open( 1, 0)) 
	{
		//Warning("[mathack] Could not open socket\n");
		return;
	}
	
	// Connect to remote host
	if (!sock.Connect("www.fortress-forever.com", 80)) 
	{
		//Warning("[mathack] Could not connect to remote host\n");
		return;
	}

	player_info_t sPlayerInfo;

	engine->GetPlayerInfo( pLocalPlayer->entindex(), &sPlayerInfo );

	char getData[255];

	Q_snprintf(getData, sizeof(getData),
		"/notifier/mathack.php?steamid=%s&mdl=%s&name=%s&key=donthack",

		sPlayerInfo.guid,
		m_pStudioPtr->pszName(),
		pLocalPlayer->GetPlayerName());

	Q_snprintf(buf, sizeof(buf),
		"GET %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Connection: close\r\n"
		"Accept-Charset: ISO-8859-1,UTF-8;q=0.7,*;q=0.7\r\n"
		"Cache-Control: no-cache\r\n"
		"\r\n",
		
		getData,
		"www.fortress-forever.com");

	// Send data
	if (!sock.Send(buf)) 
	{
		//Warning("[mathack] Could not send data to remote host\n");
		sock.Close();
		return;
	}
	
	int a;

	// Send data
	if ((a = sock.Recv(buf, sizeof(buf)-1)) == 0) 
	{
		//Warning("[mathack] Did not get response from server\n");
		sock.Close();
		return;
	}

	buf[a] = '\0';
	
	//DevMsg("[mathack] Successfully logged mathacker. Response:\n---\n%s\n---\n", buf);

	// Close socket
	sock.Close();
}

/////////////////////////////////////////////////////////////////////////////
void CFFMathackModel::PrintToConsole()
{
	if (m_bIsMathacked)
		Msg("-> ");

	Msg("%i | %p | %f | %d\n", m_iModelIndex, m_pStudioPtr, m_flLastChecked, m_bIsMathacked);
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
void CFFMathackManager::AddMathackModel(int iModelIndex, 
										studiohdr_t *pStudioPtr)
{
	VPROF_BUDGET( "CFFMathackManager::AddMathackModel", VPROF_BUDGETGROUP_FF_MATHACKDETECT );

	// check if the model of the specified id already exists
	if(m_models.IsValidIndex(m_models.Find(iModelIndex)))
		return;

	m_models.Insert( iModelIndex, CFFMathackModel( iModelIndex, pStudioPtr ) );
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
