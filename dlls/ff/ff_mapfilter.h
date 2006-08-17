// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_mapfilter.h
// @author Patrick O'Leary (Mulchman) 
// @date 8/17/2006
// @brief Map filter
//
// REVISIONS
// ---------
//	8/17/2006, Mulchman: 
//		First created - my birthday!

#ifndef FF_MAPFILTER_H
#define FF_MAPFILTER_H

#ifdef _WIN32
#pragma once
#endif

#include "mapentities.h"
#include "UtlSortVector.h"

//=============================================================================
//
// Class CFFMapFilter
//
//=============================================================================

class CFFMapFilter : public IMapEntityFilter
{
public:
	CFFMapFilter( void );
	~CFFMapFilter( void );

public:
	typedef const char* szPtr;
	class CFFMapFilterLess
	{
	public:
		bool Less( const szPtr &src1, const szPtr &src2, void *pCtx )
		{
			if( Q_strcmp( src1, src2 ) >= 0 )
				return false;
			else
				return true;
		}
	};

public:
	virtual bool ShouldCreateEntity( const char *pszClassname );
	virtual CBaseEntity *CreateNextEntity( const char *pszClassname );
	void AddKeep( const char *pszClassname );

private:
	void Initialize( void );

private:
	CUtlSortVector< const char *, CFFMapFilterLess > m_vKeepList;

};

#endif // FF_MAPFILTER_H
