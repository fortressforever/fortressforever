//========== Copyright � 2005, Valve Corporation, All rights reserved. ========
//
// Purpose: A collection of utility classes to simplify thread handling, and
//			as much as possible contain portability problems. Here avoiding 
//			including windows.h.
//
//=============================================================================

#ifndef THREADTOOLS_H
#define THREADTOOLS_H

#include "tier0/platform.h"
#include "tier0/dbg.h"
#include "tier0/vcrmode.h"

#ifdef _LINUX
#include <pthread.h>
#endif

#if defined( _WIN32 )
#pragma once
#endif

#ifndef STATIC_TIER0

#ifdef TIER0_DLL_EXPORT
#define TT_INTERFACE	DLL_EXPORT
#define TT_OVERLOAD	DLL_GLOBAL_EXPORT
#define TT_CLASS		DLL_CLASS_EXPORT
#else
#define TT_INTERFACE	DLL_IMPORT
#define TT_OVERLOAD	DLL_GLOBAL_IMPORT
#define TT_CLASS		DLL_CLASS_IMPORT
#endif

#else // BUILD_AS_DLL

#define TT_INTERFACE	extern
#define TT_OVERLOAD	
#define TT_CLASS		
#endif // BUILD_AS_DL

#ifndef _RETAIL
#define THREAD_MUTEX_TRACING_SUPPORTED
#if defined(_WIN32) && defined(_DEBUG)
#define THREAD_MUTEX_TRACING_ENABLED
#endif
#endif

#ifdef _WIN32
typedef void *HANDLE;
#endif

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

const unsigned TT_INFINITE = 0xffffffff;

#ifndef NO_THREAD_LOCAL

#ifndef THREAD_LOCAL
#ifdef _WIN32
#define THREAD_LOCAL __declspec(thread)
#elif _LINUX
#define THREAD_LOCAL __thread
#endif
#endif

#endif // NO_THREAD_LOCAL

#ifndef _XBOX
typedef unsigned long ThreadId_t;
#else
typedef unsigned ThreadId_t;
#endif

//-----------------------------------------------------------------------------
//
// Simple thread creation. Differs from VCR mode/CreateThread/_beginthreadex
// in that it accepts a standard C function rather than compiler specific one.
//
//-----------------------------------------------------------------------------
FORWARD_DECLARE_HANDLE( ThreadHandle_t );
typedef unsigned (*ThreadFunc_t)( void *pParam );

TT_INTERFACE ThreadHandle_t CreateSimpleThread( ThreadFunc_t, void *pParam, unsigned stackSize = 0 );

//-----------------------------------------------------------------------------

TT_INTERFACE void ThreadSleep(unsigned duration = 0);
TT_INTERFACE uint ThreadGetCurrentId();
TT_INTERFACE int ThreadGetPriority( ThreadHandle_t hThread = NULL );
TT_INTERFACE bool ThreadInMainThread();

inline void ThreadPause()
{
#ifdef _WIN32
	__asm pause;
#elif _LINUX
	__asm __volatile("pause");
#else
#error "implement me"
#endif
}

//-----------------------------------------------------------------------------
//
// Interlock methods. These perform very fast atomic thread
// safe operations. These are especially relevant in a multi-core setting.
//
//-----------------------------------------------------------------------------

#ifdef _WIN32
#define NOINLINE
#elif _LINUX
#define NOINLINE __attribute__ ((noinline))
#endif

TT_INTERFACE long ThreadInterlockedIncrement( long volatile * ) NOINLINE;
TT_INTERFACE long ThreadInterlockedDecrement( long volatile * ) NOINLINE;
TT_INTERFACE long ThreadInterlockedExchange( long volatile *, long value ) NOINLINE;
TT_INTERFACE long ThreadInterlockedExchangeAdd( long volatile *, long value ) NOINLINE;
TT_INTERFACE long ThreadInterlockedCompareExchange( long volatile *, long value, long comperand ) NOINLINE;

TT_INTERFACE void *ThreadInterlockedExchangePointer( void * volatile *, void *value ) NOINLINE;
TT_INTERFACE void *ThreadInterlockedCompareExchangePointer( void * volatile *, void *value, void *comperand ) NOINLINE;

TT_INTERFACE int64 ThreadInterlockedIncrement64( int64 volatile * ) NOINLINE;
TT_INTERFACE int64 ThreadInterlockedDecrement64( int64 volatile * ) NOINLINE;
TT_INTERFACE int64 ThreadInterlockedCompareExchange64( int64 volatile *, int64 value, int64 comperand ) NOINLINE;
TT_INTERFACE int64 ThreadInterlockedExchange64( int64 volatile *, int64 value ) NOINLINE;
TT_INTERFACE int64 ThreadInterlockedExchangeAdd64( int64 volatile *, int64 value ) NOINLINE;
TT_INTERFACE bool ThreadInterlockedAssignIf64(volatile int64 *pDest, int64 value, int64 comperand ) NOINLINE;

//-----------------------------------------------------------------------------
// Encapsulation of a thread local datum (needed because THREAD_LOCAL doesn't
// work in a DLL loaded with LoadLibrary()
//-----------------------------------------------------------------------------

#ifndef __AFXTLS_H__ // not compatible with some Windows headers
#ifndef NO_THREAD_LOCAL

class TT_CLASS CThreadLocalBase
{
public:
	CThreadLocalBase();
	~CThreadLocalBase();

	void * Get() const;
	void   Set(void *);

private:
#ifdef _WIN32
	uint32 m_index;
#elif _LINUX
	pthread_key_t m_index;
#endif
};

//---------------------------------------------------------

#ifndef __AFXTLS_H__

template <class T>
class CThreadLocal : public CThreadLocalBase
{
public:
	CThreadLocal()
	{
		COMPILE_TIME_ASSERT( sizeof(T) == sizeof(void *) );
	}

	T Get() const
	{
		return reinterpret_cast<T>(CThreadLocalBase::Get());
	}

	void Set(T val)
	{
		CThreadLocalBase::Set(reinterpret_cast<void *>(val));
	}
};

#endif

//---------------------------------------------------------

template <class T = int>
class CThreadLocalInt : public CThreadLocal<T>
{
public:
	operator const T() const { return Get(); }
	int	operator=( T i ) { Set( i ); return i; }

	T operator++()					{ T i = Get(); Set( ++i ); return i; }
	T operator++(int)				{ T i = Get(); Set( i + 1 ); return i; }

	T operator--()					{ T i = Get(); Set( --i ); return i; }
	T operator--(int)				{ T i = Get(); Set( i - 1 ); return i; }
};

//---------------------------------------------------------

template <class T>
class CThreadLocalPtr : private CThreadLocalBase
{
public:
	CThreadLocalPtr() {}

	operator const void *() const          					{ return (T *)Get(); }
	operator void *()                      					{ return (T *)Get(); }

	operator const T *() const							    { return (T *)Get(); }
	operator const T *()          							{ return (T *)Get(); }
	operator T *()											{ return (T *)Get(); }

	int			operator=( int i )							{ AssertMsg( i == 0, "Only NULL allowed on integer assign" ); Set( NULL ); return 0; }
	T *			operator=( T *p )							{ Set( p ); return p; }

	bool        operator !() const							{ return (!Get()); }
	bool        operator!=( int i ) const					{ AssertMsg( i == 0, "Only NULL allowed on integer compare" ); return (Get() != NULL); }
	bool        operator==( int i ) const					{ AssertMsg( i == 0, "Only NULL allowed on integer compare" ); return (Get() == NULL); }
	bool		operator==( const void *p ) const			{ return (Get() == p); }
	bool		operator!=( const void *p ) const			{ return (Get() != p); }
	bool		operator==( const T *p ) const				{ return operator==((void*)p); }
	bool		operator!=( const T *p ) const				{ return operator!=((void*)p); }

	T *  		operator->()								{ return (T *)Get(); }
	T &  		operator *()								{ return *((T *)Get()); }

	const T *   operator->() const							{ return (T *)Get(); }
	const T &   operator *() const							{ return *((T *)Get()); }

	const T &	operator[]( int i ) const					{ return *((T *)Get() + i); }
	T &			operator[]( int i )							{ return *((T *)Get() + i); }

private:
	// Disallowed operations
	CThreadLocalPtr( T *pFrom );
	CThreadLocalPtr( const CThreadLocalPtr<T> &from );
	T **operator &();
	T * const *operator &() const;
	void operator=( const CThreadLocalPtr<T> &from );
	bool operator==( const CThreadLocalPtr<T> &p ) const;
	bool operator!=( const CThreadLocalPtr<T> &p ) const;
};

#endif // NO_THREAD_LOCAL
#endif // !__AFXTLS_H__

//-----------------------------------------------------------------------------
//
// A super-fast thread-safe integer A simple class encapsulating the notion of an 
// atomic integer used across threads that uses the built in and faster 
// "interlocked" functionality rather than a full-blown mutex. Useful for simple 
// things like reference counts, etc.
//
//-----------------------------------------------------------------------------

template <typename T>
class CInterlockedIntT
{
public:
	CInterlockedIntT() : m_value( 0 ) 				{ COMPILE_TIME_ASSERT( sizeof(T) == sizeof(long) ); }
	CInterlockedIntT( T value ) : m_value( value ) 	{}

	operator T() const				{ return m_value; }

	bool operator!() const			{ return ( m_value == 0 ); }
	bool operator==( T rhs ) const	{ return ( m_value == rhs ); }
	bool operator!=( T rhs ) const	{ return ( m_value != rhs ); }

	T operator++()					{ return (T)ThreadInterlockedIncrement( (long *)&m_value ); }
	T operator++(int)				{ return operator++() - 1; }

	T operator--()					{ return (T)ThreadInterlockedDecrement( (long *)&m_value ); }
	T operator--(int)				{ return operator--() + 1; }

	bool AssignIf( T conditionValue, T newValue )	{ return ( ThreadInterlockedCompareExchange( (long *)&m_value, (long)newValue, (long)conditionValue ) == (long)conditionValue ); }

	T operator=( T newValue )		{ ThreadInterlockedExchange((long *)&m_value, newValue); return m_value; }

	void operator+=( T add )		{ ThreadInterlockedExchangeAdd( (long *)&m_value, (long)add ); }
	void operator-=( T subtract )	{ operator+=( -subtract ); }
	void operator*=( T multiplier )	{ 
		T original, result; 
		do 
		{ 
			original = m_value; 
			result = original * multiplier; 
		} while ( !AssignIf( original, result ) );
	}
	void operator/=( T divisor )	{ 
		T original, result; 
		do 
		{ 
			original = m_value; 
			result = original / divisor;
		} while ( !AssignIf( original, result ) );
	}

	T operator+( T rhs ) const		{ return m_value + rhs; }
	T operator-( T rhs ) const		{ return m_value - rhs; }

private:
	volatile T m_value;
};

typedef CInterlockedIntT<int> CInterlockedInt;
typedef CInterlockedIntT<unsigned> CInterlockedUInt;

//-----------------------------------------------------------------------------

template <typename T>
class CInterlockedPtr
{
public:
	CInterlockedPtr() : m_value( 0 ) 				{ COMPILE_TIME_ASSERT( sizeof(T *) == sizeof(long) ); /* Will need to rework operator+= for 64 bit */ }
	CInterlockedPtr( T *value ) : m_value( value ) 	{}

	operator T *() const			{ return m_value; }

	bool operator!() const			{ return ( m_value == 0 ); }
	bool operator==( T *rhs ) const	{ return ( m_value == rhs ); }
	bool operator!=( T *rhs ) const	{ return ( m_value != rhs ); }

	T *operator++()					{ return ((T *)ThreadInterlockedExchangeAdd( (long *)&m_value, sizeof(T) )) + 1; }
	T *operator++(int)				{ return (T *)ThreadInterlockedExchangeAdd( (long *)&m_value, sizeof(T) ); }

	T *operator--()					{ return ((T *)ThreadInterlockedExchangeAdd( (long *)&m_value, -sizeof(T) )) - 1; }
	T *operator--(int)				{ return (T *)ThreadInterlockedExchangeAdd( (long *)&m_value, -sizeof(T) ); }

	bool AssignIf( T *conditionValue, T *newValue )	{ return ( ThreadInterlockedCompareExchangePointer( (void **)&m_value, (void *)newValue, (void *)conditionValue ) == (void *)conditionValue ); }

	T *operator=( T *newValue )		{ ThreadInterlockedExchangePointer((void **)&m_value, newValue); return newValue; }

	void operator+=( int add )		{ ThreadInterlockedExchangeAdd( (long *)&m_value, add * sizeof(T) ); }
	void operator-=( int subtract )	{ operator+=( -subtract ); }

	T *operator+( int rhs ) const	{ return m_value + rhs; }
	T *operator-( int rhs ) const	{ return m_value - rhs; }

private:
	T * volatile m_value;
};


//-----------------------------------------------------------------------------
//
// Platform independent for critical sections management
//
//-----------------------------------------------------------------------------

class TT_CLASS CThreadMutex
{
public:
	CThreadMutex();
	~CThreadMutex();

	//------------------------------------------------------
	// Mutex acquisition/release. Const intentionally defeated.
	//------------------------------------------------------
	void Lock();
	void Lock() const		{ (const_cast<CThreadMutex *>(this))->Lock(); }
	void Unlock();
	void Unlock() const		{ (const_cast<CThreadMutex *>(this))->Unlock(); }

	bool TryLock();
	bool TryLock() const	{ return (const_cast<CThreadMutex *>(this))->TryLock(); }

	//------------------------------------------------------
	// Use this to make deadlocks easier to track by asserting
	// when it is expected that the current thread owns the mutex
	//------------------------------------------------------
	bool AssertOwnedByCurrentThread();

	//------------------------------------------------------
	// Enable tracing to track deadlock problems
	//------------------------------------------------------
	void SetTrace( bool );

private:
	// Disallow copying
	CThreadMutex( const CThreadMutex & );
	CThreadMutex &operator=( const CThreadMutex & );

#if defined( _WIN32 )
	// Efficient solution to breaking the windows.h dependency, invariant is tested.
#ifndef _XBOX
	#define TT_SIZEOF_CRITICALSECTION 24
#else
	#define TT_SIZEOF_CRITICALSECTION 28
#endif
	byte m_CriticalSection[TT_SIZEOF_CRITICALSECTION];
#elif _LINUX
	pthread_mutex_t m_Mutex;
	pthread_mutexattr_t m_Attr;
#else
#error
#endif

#ifdef THREAD_MUTEX_TRACING_SUPPORTED
	// Debugging (always here to allow mixed debug/release builds w/o changing size)
	uint	m_currentOwnerID;
	uint16	m_lockCount;
	bool	m_bTrace;
#endif
};

//-----------------------------------------------------------------------------
//
// An alternative mutex that is useful for cases when thread contention is 
// rare, but a mutex is required. Instances should be declared volatile.
// Sleep of 0 may not be sufficient to keep high priority threads from starving 
// lesser threads. This class is not a suitable replacement for a critical
// section if the resource contention is high.
//
//-----------------------------------------------------------------------------

class TT_CLASS CThreadFastMutex
{
public:
	CThreadFastMutex()
	  :	m_ownerID( 0 ),
	  	m_depth( 0 )
	{
	}

private:
	bool TryLock( const uint32 threadId ) volatile
	{
		if ( threadId != m_ownerID && ThreadInterlockedCompareExchange( (volatile long *)&m_ownerID, (long)threadId, 0 ) != 0 )
			return false;

		m_depth++;
		return true;
	}

public:
	bool TryLock() volatile
	{
		return TryLock( ThreadGetCurrentId() );
	}

	void Lock( unsigned nSpinSleepTime = 1 ) volatile 
	{
		const uint32 threadId = ThreadGetCurrentId();


		if ( TryLock( threadId ) )
		{
			return;
		}
        
		ThreadPause();

		for ( int i = 16; i > 0; --i )
		{
			if ( TryLock( threadId ) )
			{
				return;
			}

			ThreadPause();
			ThreadSleep( 0 );
		}

		while ( !TryLock( threadId ) )
		{ 
			ThreadPause();
			ThreadSleep( nSpinSleepTime );		
		};
		
#ifdef _DEBUG
		if ( m_ownerID != ThreadGetCurrentId() )
			DebuggerBreak();
#endif
	}

	void Unlock() volatile
	{
#ifdef _DEBUG
		if ( m_ownerID != ThreadGetCurrentId() )
			DebuggerBreak();
#endif
		m_depth--;
		if ( !m_depth )
			m_ownerID = 0;
	}

	bool TryLock() const volatile							{ return (const_cast<CThreadFastMutex *>(this))->TryLock(); }
	void Lock(unsigned nSpinSleepTime = 1 ) const volatile	{ (const_cast<CThreadFastMutex *>(this))->Lock( nSpinSleepTime ); }
	void Unlock() const	volatile							{ (const_cast<CThreadFastMutex *>(this))->Unlock(); }

	// To match regular CThreadMutex:
	bool AssertOwnedByCurrentThread()	{ return true; }
	void SetTrace( bool )				{}
private:
	volatile uint32	m_ownerID;
	volatile int	m_depth;
};

//-----------------------------------------------------------------------------
//
// Class to Lock a critical section, and unlock it automatically
// when the lock goes out of scope
//
//-----------------------------------------------------------------------------

template <class MUTEX_TYPE = CThreadMutex>
class CAutoLockT
{
public:
	CAutoLockT(MUTEX_TYPE &lock)
		: m_lock(lock)
	{
		m_lock.Lock();
	}

	CAutoLockT(const MUTEX_TYPE &lock)
		: m_lock(const_cast<CThreadMutex &>(lock))
	{
		m_lock.Lock();
	}

	~CAutoLockT()
	{
		m_lock.Unlock();
	}


private:
	MUTEX_TYPE &m_lock;

	// Disallow copying
	CAutoLockT<MUTEX_TYPE>( const CAutoLockT<MUTEX_TYPE> & );
	CAutoLockT<MUTEX_TYPE> &operator=( const CAutoLockT<MUTEX_TYPE> & );
};

typedef CAutoLockT<CThreadMutex> CAutoLock;

//---------------------------------------------------------

template <int size>	struct CAutoLockTypeDeducer {};
template <> struct CAutoLockTypeDeducer<sizeof(CThreadMutex)> {	typedef CThreadMutex Type_t; };
template <> struct CAutoLockTypeDeducer<sizeof(CThreadFastMutex)> {	typedef CThreadFastMutex Type_t; };

#define AUTO_LOCK( mutex ) \
	CAutoLockT<CAutoLockTypeDeducer<sizeof(mutex)>::Type_t> autoLock_##mutex ( const_cast<CAutoLockTypeDeducer<sizeof(mutex)>::Type_t &>( mutex ) )

#define AUTO_LOCK_FM( mutex ) \
	AUTO_LOCK( mutex )

#define LOCAL_THREAD_LOCK_( tag ) \
	; \
	static CThreadMutex autoMutex_##tag; \
	CAutoLock autoLock_##tag ( autoMutex_##tag )

#define LOCAL_THREAD_LOCK() \
	LOCAL_THREAD_LOCK_(_)

//-----------------------------------------------------------------------------
//
// Base class for event, semaphore and mutex objects.
//
//-----------------------------------------------------------------------------

class TT_CLASS CThreadSyncObject
{
public:
	~CThreadSyncObject();

	//-----------------------------------------------------
	// Query if object is useful
	//-----------------------------------------------------
	bool operator!() const;

	//-----------------------------------------------------
	// Access handle
	//-----------------------------------------------------
#ifdef _WIN32
	operator HANDLE() { return m_hSyncObject; }
#endif
	//-----------------------------------------------------
	// Wait for a signal from the object
	//-----------------------------------------------------
	bool Wait( uint32 dwTimeout = TT_INFINITE );

protected:
	CThreadSyncObject();
	void AssertUseable();

#ifdef _WIN32
	HANDLE m_hSyncObject;
#elif _LINUX
	pthread_mutex_t	m_Mutex;
	pthread_cond_t	m_Condition;
	bool m_bInitalized;
#else
#error "Implement me"
#endif

private:
	CThreadSyncObject( const CThreadSyncObject & );
	CThreadSyncObject &operator=( const CThreadSyncObject & );
};


//-----------------------------------------------------------------------------
//
// Wrapper for unnamed event objects
//
//-----------------------------------------------------------------------------

#if defined( _WIN32 )

//-----------------------------------------------------------------------------
//
// CThreadSemaphore
//
//-----------------------------------------------------------------------------

class TT_CLASS CThreadSemaphore : public CThreadSyncObject
{
public:
	CThreadSemaphore(long initialValue, long maxValue);

	//-----------------------------------------------------
	// Increases the count of the semaphore object by a specified
	// amount.  Wait() decreases the count by one on return.
	//-----------------------------------------------------
	bool Release(long releaseCount = 1, long * pPreviousCount = NULL );

private:
	CThreadSemaphore(const CThreadSemaphore &);
	CThreadSemaphore &operator=(const CThreadSemaphore &);
};


//-----------------------------------------------------------------------------
//
// A mutex suitable for out-of-process, multi-processor usage
//
//-----------------------------------------------------------------------------

class TT_CLASS CThreadFullMutex : public CThreadSyncObject
{
public:
	CThreadFullMutex( bool bEstablishInitialOwnership = false, const char * pszName = NULL );

	//-----------------------------------------------------
	// Release ownership of the mutex
	//-----------------------------------------------------
	bool Release();

	// To match regular CThreadMutex:
	void Lock()							{ Wait(); }
	void Lock( unsigned timeout )		{ Wait( timeout ); }
	void Unlock()						{ Release(); }
	bool AssertOwnedByCurrentThread()	{ return true; }
	void SetTrace( bool )				{}

private:
	CThreadFullMutex( const CThreadFullMutex & );
	CThreadFullMutex &operator=( const CThreadFullMutex & );
};
#endif


class TT_CLASS CThreadEvent : public CThreadSyncObject
{
public:
	CThreadEvent( bool fManualReset = false );

	//-----------------------------------------------------
	// Set the state to signaled
	//-----------------------------------------------------
	bool Set();

	//-----------------------------------------------------
	// Set the state to nonsignaled
	//-----------------------------------------------------
	bool Reset();

	//-----------------------------------------------------
	// Check if the event is signaled
	//-----------------------------------------------------
	bool Check();

	bool Wait( uint32 dwTimeout = TT_INFINITE );

private:
	CThreadEvent( const CThreadEvent & );
	CThreadEvent &operator=( const CThreadEvent & );
#ifdef _LINUX
	CInterlockedInt m_cSet;
#endif
};

// Hard-wired manual event for use in array declarations
class CThreadManualEvent : public CThreadEvent
{
public:
	CThreadManualEvent()
	 :	CThreadEvent( true )
	{
	}
};

//-----------------------------------------------------------------------------
//
// A thread wrapper similar to a Java thread.
//
//-----------------------------------------------------------------------------

class TT_CLASS CThread
{
public:
	CThread();
	virtual ~CThread();

	//-----------------------------------------------------

	const char *GetName();
	void SetName( const char * );

	size_t CalcStackDepth( void *pStackVariable )		{ return ((byte *)m_pStackBase - (byte *)pStackVariable); }

	//-----------------------------------------------------
	// Functions for the other threads
	//-----------------------------------------------------

	// Start thread running  - error if already running
	virtual bool Start( unsigned nBytesStack = 0 );

	// Returns true if thread has been created and hasn't yet exited
	bool IsAlive();

	// This method causes the current thread to wait until this thread
	// is no longer alive.
	bool Join( unsigned timeout = TT_INFINITE );

#ifdef _WIN32
	// Access the thread handle directly
	HANDLE GetThreadHandle();
	uint GetThreadId();
#endif

	//-----------------------------------------------------

	int GetResult();

	//-----------------------------------------------------
	// Functions for both this, and maybe, and other threads
	//-----------------------------------------------------

	// Forcibly, abnormally, but relatively cleanly stop the thread
	void Stop( int exitCode = 0 );

	// Get the priority
	int GetPriority() const;

	// Set the priority
	bool SetPriority( int );

	// Suspend a thread
	unsigned Suspend();

	// Resume a suspended thread
	unsigned Resume();

	// Force hard-termination of thread.  Used for critical failures.
	bool Terminate( int exitCode = 0 );

	//-----------------------------------------------------
	// Global methods
	//-----------------------------------------------------

	// Get the Thread object that represents the current thread, if any.
	// Can return NULL if the current thread was not created using
	// CThread
	static CThread *GetCurrentCThread();

	// Offer a context switch. Under Win32, equivalent to Sleep(0)
#ifdef Yield
#undef Yield
#endif
	static void Yield();

	// This method causes the current thread to yield and not to be
	// scheduled for further execution until a certain amount of real
	// time has elapsed, more or less.
	static void Sleep( unsigned duration );

protected:

	// Optional pre-run call, with ability to fail-create. Note Init()
	// is forced synchronous with Start()
	virtual bool Init();

	// Thread will run this function on startup, must be supplied by
	// derived class, performs the intended action of the thread.
	virtual int Run() = 0;

	// Called when the thread exits
	virtual void OnExit();

#ifdef _WIN32
	// Allow for custom start waiting
	virtual bool WaitForCreateComplete( CThreadEvent *pEvent );
#endif

	// "Virtual static" facility
	typedef unsigned (__stdcall *ThreadProc_t)( void * );
	virtual ThreadProc_t GetThreadProc();

	CThreadMutex m_Lock;

private:

	// Thread initially runs this. param is actually 'this'. function
	// just gets this and calls ThreadProc
	struct ThreadInit_t
	{
		CThread *     pThread;
#ifdef _WIN32
		CThreadEvent *pInitCompleteEvent;
#endif
		bool *        pfInitSuccess;
	};

	static unsigned __stdcall ThreadProc( void * pv );

	// make copy constructor and assignment operator inaccessible
	CThread( const CThread & );
	CThread &operator=( const CThread & );

#ifdef _WIN32
	HANDLE 	m_hThread;
	ThreadId_t m_threadId;
#elif _LINUX
	pthread_t m_threadId;
#endif
	int		m_result;
	char	m_szName[32];
	void *	m_pStackBase;
};

//-----------------------------------------------------------------------------
// Simple thread class encompasses the notion of a worker thread, handing
// synchronized communication.
//-----------------------------------------------------------------------------

#ifdef _WIN32

// These are internal reserved error results from a call attempt
enum WTCallResult_t
{
	WTCR_FAIL			= -1,
	WTCR_TIMEOUT		= -2,
	WTCR_THREAD_GONE	= -3,
};

class TT_CLASS CWorkerThread : public CThread
{
public:
	CWorkerThread();

	//-----------------------------------------------------
	//
	// Inter-thread communication
	//
	// Calls in either direction take place on the same "channel."
	// Seperate functions are specified to make identities obvious
	//
	//-----------------------------------------------------

	// Master: Signal the thread, and block for a response
	int CallWorker( unsigned, unsigned timeout = TT_INFINITE, bool fBoostWorkerPriorityToMaster = true );

	// Worker: Signal the thread, and block for a response
	int CallMaster( unsigned, unsigned timeout = TT_INFINITE );

	// Wait for the next request
	bool WaitForCall( unsigned dwTimeout, unsigned *pResult = NULL );
	bool WaitForCall( unsigned *pResult = NULL );

	// Is there a request?
	bool PeekCall( unsigned *pParam = NULL );

	// Reply to the request
	void Reply( unsigned );

	// If you want to do WaitForMultipleObjects you'll need to include
	// this handle in your wait list or you won't be responsive
	HANDLE GetCallHandle();

	// Find out what the request was
	unsigned GetCallParam() const;

	// Boost the worker thread to the master thread, if worker thread is lesser, return old priority
	int BoostPriority();

protected:
	typedef uint32 (__stdcall *WaitFunc_t)( uint32 nHandles, const HANDLE*pHandles, int bWaitAll, uint32 timeout );
	int Call( unsigned, unsigned timeout, bool fBoost, WaitFunc_t = NULL );

private:
	CWorkerThread( const CWorkerThread & );
	CWorkerThread &operator=( const CWorkerThread & );

#ifdef _WIN32
	CThreadEvent	m_EventSend;
	CThreadEvent	m_EventComplete;
#endif

	unsigned        m_Param;
	int				m_ReturnVal;
};

#else

typedef CThread CWorkerThread;

#endif

//-----------------------------------------------------------------------------
//
// CThreadMutex. Inlining to reduce overhead and to allow client code
// to decide debug status (tracing)
//
//-----------------------------------------------------------------------------

#ifdef _WIN32
typedef struct _RTL_CRITICAL_SECTION RTL_CRITICAL_SECTION;
typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;

extern "C"
{
#ifndef _XBOX
void __declspec(dllimport) __stdcall InitializeCriticalSection(CRITICAL_SECTION *);
void __declspec(dllimport) __stdcall EnterCriticalSection(CRITICAL_SECTION *);
void __declspec(dllimport) __stdcall LeaveCriticalSection(CRITICAL_SECTION *);
void __declspec(dllimport) __stdcall DeleteCriticalSection(CRITICAL_SECTION *);
#else
void __declspec(dllimport) __stdcall RtlInitializeCriticalSection(RTL_CRITICAL_SECTION *);
void __declspec(dllimport) __stdcall RtlEnterCriticalSection(RTL_CRITICAL_SECTION *);
void __declspec(dllimport) __stdcall RtlLeaveCriticalSection(RTL_CRITICAL_SECTION *);
#define RtlDeleteCriticalSection(CriticalSection) ((void)0)
#define InitializeCriticalSection RtlInitializeCriticalSection
#define DeleteCriticalSection     RtlDeleteCriticalSection
#define EnterCriticalSection      RtlEnterCriticalSection
#define LeaveCriticalSection      RtlLeaveCriticalSection
#define TryEnterCriticalSection   RtlTryEnterCriticalSection
#endif
};

//---------------------------------------------------------

inline CThreadMutex::CThreadMutex()
{
#ifdef THREAD_MUTEX_TRACING_ENABLED
	memset( &m_CriticalSection, 0, sizeof(m_CriticalSection) );
#endif
	InitializeCriticalSection((CRITICAL_SECTION *)&m_CriticalSection);
#ifdef THREAD_MUTEX_TRACING_SUPPORTED
	// These need to be initialized unconditionally in case mixing release & debug object modules
	// Lock and unlock may be emitted as COMDATs, in which case may get spurious output
	m_currentOwnerID = m_lockCount = 0;
	m_bTrace = false;
#endif
}

//---------------------------------------------------------

inline CThreadMutex::~CThreadMutex()
{
	DeleteCriticalSection((CRITICAL_SECTION *)&m_CriticalSection);
}

//---------------------------------------------------------

inline void CThreadMutex::Lock()
{
#ifdef THREAD_MUTEX_TRACING_ENABLED
	uint thisThreadID = ThreadGetCurrentId();
	if ( m_bTrace && m_currentOwnerID && ( m_currentOwnerID != thisThreadID ) )
		Msg( "Thread %u about to wait for lock %x owned by %u\n", ThreadGetCurrentId(), (CRITICAL_SECTION *)&m_CriticalSection, m_currentOwnerID );
#endif

	VCRHook_EnterCriticalSection((CRITICAL_SECTION *)&m_CriticalSection);

#ifdef THREAD_MUTEX_TRACING_ENABLED
	if (m_lockCount == 0)
	{
		// we now own it for the first time.  Set owner information
		m_currentOwnerID = thisThreadID;
		if ( m_bTrace )
			Msg( "Thread %u now owns lock 0x%x\n", m_currentOwnerID, (CRITICAL_SECTION *)&m_CriticalSection );
	}
	m_lockCount++;
#endif
}

//---------------------------------------------------------

inline void CThreadMutex::Unlock()
{
#ifdef THREAD_MUTEX_TRACING_ENABLED
	AssertMsg( m_lockCount >= 1, "Invalid unlock of thread lock" );
	m_lockCount--;
	if (m_lockCount == 0)
	{
		if ( m_bTrace )
			Msg( "Thread %u releasing lock 0x%x\n", m_currentOwnerID, (CRITICAL_SECTION *)&m_CriticalSection );
		m_currentOwnerID = 0;
	}
#endif
	LeaveCriticalSection((CRITICAL_SECTION *)&m_CriticalSection);
}

//---------------------------------------------------------

inline bool CThreadMutex::AssertOwnedByCurrentThread()
{
#ifdef THREAD_MUTEX_TRACING_ENABLED
	if (ThreadGetCurrentId() == m_currentOwnerID)
		return true;
	AssertMsg3( 0, "Expected thread %u as owner of lock 0x%x, but %u owns", ThreadGetCurrentId(), (CRITICAL_SECTION *)&m_CriticalSection, m_currentOwnerID );
	return false;
#else
	return true;
#endif
}

//---------------------------------------------------------

inline void CThreadMutex::SetTrace( bool bTrace )
{
#ifdef THREAD_MUTEX_TRACING_ENABLED
	m_bTrace = bTrace;
#endif
}

//---------------------------------------------------------

#elif _LINUX

inline CThreadMutex::CThreadMutex()
{
	// enable recursive locks as we need them
	pthread_mutexattr_init( &m_Attr );
	pthread_mutexattr_settype( &m_Attr, PTHREAD_MUTEX_RECURSIVE_NP );
	pthread_mutex_init( &m_Mutex, &m_Attr );
}

//---------------------------------------------------------

inline CThreadMutex::~CThreadMutex()
{
	pthread_mutex_destroy( &m_Mutex );
}

//---------------------------------------------------------

inline void CThreadMutex::Lock()
{
	pthread_mutex_lock( &m_Mutex );
}

//---------------------------------------------------------

inline void CThreadMutex::Unlock()
{
	pthread_mutex_unlock( &m_Mutex );
}

//---------------------------------------------------------

inline bool CThreadMutex::AssertOwnedByCurrentThread()
{
	return true;
}

//---------------------------------------------------------

inline void CThreadMutex::SetTrace(bool fTrace)
{
}

#endif // _LINUX

//-----------------------------------------------------------------------------

#endif // THREADTOOLS_H
