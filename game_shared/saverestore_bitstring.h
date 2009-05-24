//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef SAVERESTORE_BITSTRING_H
#define SAVERESTORE_BITSTRING_H

#include "isaverestore.h"

#if defined( _WIN32 )
#pragma once
#endif

//-------------------------------------

template <class BITSTRING>
class CBitStringSaveRestoreOps : public CDefSaveRestoreOps
{
public:
	CBitStringSaveRestoreOps()
	{
	}

	// save data type interface
	virtual void Save( const SaveRestoreFieldInfo_t &fieldInfo, ISave *pSave )
	{
		BITSTRING *pBitString = (BITSTRING *)fieldInfo.pField;
		int numBits = pBitString->Size();
		pSave->WriteInt( &numBits );
		pSave->WriteInt( pBitString->GetInts(), pBitString->GetNumInts() );
	}
	
	virtual void Restore( const SaveRestoreFieldInfo_t &fieldInfo, IRestore *pRestore )
	{
		BITSTRING *pBitString = (BITSTRING *)fieldInfo.pField;
		int numBits = pRestore->ReadInt();
		if ( !pBitString->IsFixedSize() )
			pBitString->Resize( numBits );
		else
		{
			Assert( pBitString->Size() >= numBits );
			pBitString->ClearAllBits();
		}
		int numIntsInStream = CalcNumIntsForBits( numBits );
		int readSize = min( pBitString->GetNumInts(), numIntsInStream );
		pRestore->ReadInt( pBitString->GetInts(), numIntsInStream );

		numIntsInStream -= readSize;
		while ( numIntsInStream-- > 0 )
		{
			int ignored;
			pRestore->ReadInt( &ignored, 1 );
		}
	}
	
	virtual void MakeEmpty( const SaveRestoreFieldInfo_t &fieldInfo )
	{
		BITSTRING *pBitString = (BITSTRING *)fieldInfo.pField;
		pBitString->ClearAllBits();
	}

	virtual bool IsEmpty( const SaveRestoreFieldInfo_t &fieldInfo )
	{
		BITSTRING *pBitString = (BITSTRING *)fieldInfo.pField;
		return pBitString->IsAllClear();
	}
};

//-------------------------------------

template <class BITSTRING>
ISaveRestoreOps *GetBitstringDataOps(BITSTRING *)
{
	static CBitStringSaveRestoreOps<BITSTRING> ops;
	return &ops;
}

//-------------------------------------

#define SaveBitString( pSave, pBitString, fieldtype) \
	CDataopsInstantiator<fieldtype>::GetDataOps( pBitString )->Save( pBitString, pSave );

#define RestoreBitString( pRestore, pBitString, fieldtype) \
	CDataopsInstantiator<fieldtype>::GetDataOps( pBitString )->Restore( pBitString, pRestore );

//-------------------------------------

#define DEFINE_BITSTRING(name) \
	{ FIELD_CUSTOM, #name, { offsetof(classNameTypedef,name), 0 }, 1, FTYPEDESC_SAVE, NULL, GetBitstringDataOps(&(((classNameTypedef *)0)->name)), NULL }

#endif // SAVERESTORE_BITSTRING_H

