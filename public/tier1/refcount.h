//========== Copyright � 2005, Valve Corporation, All rights reserved. ========
//
// Purpose: Tools for correctly implementing & handling reference counted
//			objects
//
//=============================================================================

#ifndef REFCOUNT_H
#define REFCOUNT_H

#include "tier0/threadtools.h"

#if defined( _WIN32 )
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose:	Implement a standard reference counted interface. Use of this
//			is optional insofar as all the concrete tools only require
//			at compile time that the function signatures match.
//-----------------------------------------------------------------------------

class IRefCounted
{
public:
	virtual int AddRef() = 0;
	virtual int Release() = 0;
};


//-----------------------------------------------------------------------------
// Purpose:	Release a pointer and mark it NULL
//-----------------------------------------------------------------------------

template <class REFCOUNTED_ITEM_PTR>
inline int SafeRelease( REFCOUNTED_ITEM_PTR &pRef )
{
	// Use funny syntax so that this works on "auto pointers"
	REFCOUNTED_ITEM_PTR *ppRef = &pRef;
	if ( *ppRef )
	{
		int result = (*ppRef)->Release();
		*ppRef = NULL;
		return result;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose:	Maintain a reference across a scope
//-----------------------------------------------------------------------------

template <class T = IRefCounted>
class CAutoRef
{
public:
	CAutoRef( T *pRef )
	  : m_pRef( pRef )
	{
		if ( m_pRef )
			m_pRef->AddRef();
	}

   ~CAutoRef()
   {
      if (m_pRef)
         m_pRef->Release();
   }

private:
   T *m_pRef;
};

//-----------------------------------------------------------------------------
// Purpose:	Do a an inline AddRef then return the pointer, usefull when
//			returning an object from a function
//-----------------------------------------------------------------------------

#define RetAddRef( p ) ( (p)->AddRef(), (p) )


//-----------------------------------------------------------------------------
// Purpose:	A class to both hold a pointer to an object and its reference.
//			Base exists to support other cleanup models
//-----------------------------------------------------------------------------

template <class T>
class CBaseAutoPtr
{
public:
	CBaseAutoPtr()                                         	: m_pObject(0) {}
	CBaseAutoPtr(T *pFrom)                        			: m_pObject(pFrom) {}

	operator const void *() const          					{ return m_pObject; }
	operator void *()                      					{ return m_pObject; }

	operator const T *() const							    { return m_pObject; }
	operator const T *()          							{ return m_pObject; }
	operator T *()											{ return m_pObject; }

	int			operator=( int i )							{ AssertMsg( i == 0, "Only NULL allowed on integer assign" ); m_pObject = 0; return 0; }
	T *			operator=( T *p )							{ m_pObject = p; return p; }

    bool        operator !() const							{ return (!m_pObject); }
    bool        operator!=( int i ) const					{ AssertMsg( i == 0, "Only NULL allowed on integer compare" ); return (m_pObject != NULL); }
	bool		operator==( const void *p ) const			{ return (m_pObject == p); }
	bool		operator!=( const void *p ) const			{ return (m_pObject != p); }
	bool		operator==( T *p ) const					{ return operator==((void*)p); }
	bool		operator!=( T *p ) const					{ return operator!=((void*)p); }
	bool		operator==( const CBaseAutoPtr<T> &p ) const { return operator==((const void*)p); }
	bool		operator!=( const CBaseAutoPtr<T> &p ) const { return operator!=((const void*)p); }

	T *  		operator->()								{ return m_pObject; }
	T &  		operator *()								{ return *m_pObject; }
	T ** 		operator &()								{ return &m_pObject; }

	const T *   operator->() const							{ return m_pObject; }
	const T &   operator *() const							{ return *m_pObject; }
	T * const * operator &() const							{ return &m_pObject; }

protected:
	CBaseAutoPtr( const CBaseAutoPtr<T> &from )				: m_pObject(from.m_pObject) {}
	void operator=( const CBaseAutoPtr<T> &from ) 			{ m_pObject = from.m_pObject; }

	T *m_pObject;
};

//---------------------------------------------------------

template <class T>
class CRefPtr : public CBaseAutoPtr<T>
{
	typedef CBaseAutoPtr<T> CBaseClass;
public:
	CRefPtr()												{}
	CRefPtr( T *pInit )										: CBaseClass( pInit ) {}
	CRefPtr( const CRefPtr<T> &from )						: CBaseClass( from ) {}
	~CRefPtr()												{ if ( CBaseClass::m_pObject ) CBaseClass::m_pObject->Release(); }

	void operator=( const CRefPtr<T> &from )				{ CBaseClass::operator=(from); }

	int operator=( int i )									{ return CBaseClass::operator=(i); }
	T * operator=( T *p )									{ return CBaseClass::operator=(p); }

	operator bool() const									{ return !CBaseClass::operator!(); }
	operator bool()											{ return !CBaseClass::operator!(); }

	void SafeRelease()										{ if (CBaseClass::m_pObject) CBaseClass::m_pObject->Release(); CBaseClass::m_pObject = 0; }
	void AssignAddRef( T * pFrom )							{ SafeRelease(); if (pFrom) pFrom->AddRef(); CBaseClass::m_pObject = pFrom; }
};


//-----------------------------------------------------------------------------
// Purpose:	Traits classes defining reference count threading model
//-----------------------------------------------------------------------------

class CRefMT
{
public:
	static int Increment( int *p) { return ThreadInterlockedIncrement( (long *)p ); }
	static int Decrement( int *p) { return ThreadInterlockedDecrement( (long *)p ); }
};

class CRefST
{
public:
	static int Increment( int *p) { return ++(*p); }
	static int Decrement( int *p) { return --(*p); }
};

//-----------------------------------------------------------------------------
// Purpose:	Actual reference counting implementation. Pulled out to reduce
//			code bloat.
//-----------------------------------------------------------------------------

template <const bool bSelfDelete, typename CRefThreading = CRefMT>
class NO_VTABLE CRefCountServiceBase
{
protected:
	CRefCountServiceBase()
	  : m_iRefs( 1 )
	{
	}

	virtual ~CRefCountServiceBase()
	{
	}

	virtual void OnFinalRelease()
	{
	}

	int GetRefCount() const
	{
		return m_iRefs;
	}

	int DoAddRef()
	{
		return CRefThreading::Increment( &m_iRefs );
	}

	int DoRelease()
	{
		int result = CRefThreading::Decrement( &m_iRefs );
		if ( result )
			return result;
		OnFinalRelease();
		if ( bSelfDelete )
			delete this;
		return 0;
	}

private:
	int m_iRefs;
};

typedef CRefCountServiceBase<true, CRefST>	CRefCountServiceST;
typedef CRefCountServiceBase<false, CRefST>	CRefCountServiceNoDeleteST;

typedef CRefCountServiceBase<true, CRefMT>	CRefCountServiceMT;
typedef CRefCountServiceBase<false, CRefMT> CRefCountServiceNoDeleteMT;

// Default to threadsafe
typedef CRefCountServiceNoDeleteMT			CRefCountService;
typedef CRefCountServiceMT					CRefCountServiceNoDelete;

//-----------------------------------------------------------------------------
// Purpose:	Base classes to implement reference counting
//-----------------------------------------------------------------------------

template < class REFCOUNT_SERVICE = CRefCountService > 
class NO_VTABLE CRefCounted : public REFCOUNT_SERVICE
{
public:
	virtual ~CRefCounted()	{}
	int AddRef() 			{ return REFCOUNT_SERVICE::DoAddRef(); }
	int Release()			{ return REFCOUNT_SERVICE::DoRelease(); }
};

//-------------------------------------

template < class BASE1, class REFCOUNT_SERVICE = CRefCountService > 
class NO_VTABLE CRefCounted1 : public BASE1,
							   public REFCOUNT_SERVICE
{
public:
	virtual ~CRefCounted1()	{}
	int AddRef() 			{ return REFCOUNT_SERVICE::DoAddRef(); }
	int Release()			{ return REFCOUNT_SERVICE::DoRelease(); }
};

//-------------------------------------

template < class BASE1, class BASE2, class REFCOUNT_SERVICE = CRefCountService > 
class NO_VTABLE CRefCounted2 : public BASE1, public BASE2,
							   public REFCOUNT_SERVICE
{
public:
	virtual ~CRefCounted2()	{}
	int AddRef() 			{ return REFCOUNT_SERVICE::DoAddRef(); }
	int Release()			{ return REFCOUNT_SERVICE::DoRelease(); }
};

//-------------------------------------

template < class BASE1, class BASE2, class BASE3, class REFCOUNT_SERVICE = CRefCountService > 
class NO_VTABLE CRefCounted3 : public BASE1, public BASE2, public BASE3,
							   public REFCOUNT_SERVICE
{
	virtual ~CRefCounted3()	{}
	int AddRef() 			{ return REFCOUNT_SERVICE::DoAddRef(); }
	int Release()			{ return REFCOUNT_SERVICE::DoRelease(); }
};

//-------------------------------------

template < class BASE1, class BASE2, class BASE3, class BASE4, class REFCOUNT_SERVICE = CRefCountService > 
class NO_VTABLE CRefCounted4 : public BASE1, public BASE2, public BASE3, public BASE4,
							   public REFCOUNT_SERVICE
{
public:
	virtual ~CRefCounted4()	{}
	int AddRef() 			{ return REFCOUNT_SERVICE::DoAddRef(); }
	int Release()			{ return REFCOUNT_SERVICE::DoRelease(); }
};

//-------------------------------------

template < class BASE1, class BASE2, class BASE3, class BASE4, class BASE5, class REFCOUNT_SERVICE = CRefCountService > 
class NO_VTABLE CRefCounted5 : public BASE1, public BASE2, public BASE3, public BASE4, public BASE5,
							   public REFCOUNT_SERVICE
{
public:
	virtual ~CRefCounted5()	{}
	int AddRef() 			{ return REFCOUNT_SERVICE::DoAddRef(); }
	int Release()			{ return REFCOUNT_SERVICE::DoRelease(); }
};

//-----------------------------------------------------------------------------
// Purpose:	Class to throw around a reference counted item to debug
//			referencing problems
//-----------------------------------------------------------------------------

template <class BASE_REFCOUNTED, int FINAL_REFS = 0, const char *pszName = NULL>
class CRefDebug : public BASE_REFCOUNTED
{
public:
#ifdef _DEBUG
	CRefDebug()
	{
		AssertMsg( GetRefCount() == 1, "Expected initial ref count of 1" );
		DevMsg( "%s:create 0x%x\n", ( pszName ) ? pszName : "", this );
	}

	virtual ~CRefDebug()
	{
		AssertDevMsg( GetRefCount() == FINAL_REFS, "Object still referenced on destroy?" );
		DevMsg( "%s:destroy 0x%x\n", ( pszName ) ? pszName : "", this );
	}

	int AddRef()
	{
		DevMsg( "%s:(0x%x)->AddRef() --> %d\n", ( pszName ) ? pszName : "", this, GetRefCount() + 1 );
		return BASE_REFCOUNTED::AddRef();
	}

	int Release()
	{
		DevMsg( "%s:(0x%x)->Release() --> %d\n", ( pszName ) ? pszName : "", this, GetRefCount() - 1 );
		Assert( GetRefCount() > 0 );
		return BASE_REFCOUNTED::Release();
	}
#endif
};

//-----------------------------------------------------------------------------

#endif // REFCOUNT_H
