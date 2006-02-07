//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef SMOOTH_AVERAGE_H
#define SMOOTH_AVERAGE_H
#ifdef _WIN32
#pragma once
#endif


#include "utldict.h"



// Use this macro around any value, and it'll queue up the results given to it nTimes and 
// provide a running average.
#define SMOOTH_AVERAGE( value, nTimes ) CalcSmoothAverage( value, nTimes, __FILE__, __LINE__ )



template< class T >
class CAveragesInfo
{
public:
	CUtlVector<T> m_Values;
	int m_iCurValue;
};


extern CUtlDict<void*, int> g_SmoothAverages;


template< class T >
inline T CalcSmoothAverage( const T &value, int nTimes, const char *pFilename, int iLine )
{
	char fullStr[1024];
	Q_snprintf( fullStr, sizeof( fullStr ), "%s_%i", pFilename, iLine );
	
	int index = g_SmoothAverages.Find( fullStr );
	CAveragesInfo<T> *pInfo;
	if ( index == g_SmoothAverages.InvalidIndex() )
	{
		pInfo = new CAveragesInfo<T>;
		index = g_SmoothAverages.Insert( fullStr, pInfo );
	}
	else
	{
		pInfo = (CAveragesInfo<T>*)g_SmoothAverages[index];
	}
	
	if ( pInfo->m_Values.Count() < nTimes )
	{
		pInfo->m_Values.AddToTail( value );
		pInfo->m_iCurValue = 0;
	}
	else
	{
		pInfo->m_Values[pInfo->m_iCurValue] = value;
		pInfo->m_iCurValue = (pInfo->m_iCurValue+1) % pInfo->m_Values.Count();
	}

	T totalValue = pInfo->m_Values[0];
	for ( int i=1; i < pInfo->m_Values.Count(); i++ )
		totalValue += pInfo->m_Values[i];

	totalValue /= pInfo->m_Values.Count();
	return totalValue;
}


#endif // SMOOTH_AVERAGE_H
