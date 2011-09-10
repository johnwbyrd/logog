/**
 * \file mutex.hpp Defines a mutual exclusion object for the current platform.
 */

#ifndef __LOGOG_MUTEX_HPP__
#define __LOGOG_MUTEX_HPP__

#ifdef LOGOG_FLAVOR_POSIX
#include <pthread.h>
#endif

namespace logog
{

//! [Mutex]
#ifndef LOGOG_MUTEX

#ifdef LOGOG_FLAVOR_WINDOWS
#define LOGOG_MUTEX(x)           CRITICAL_SECTION (x);
#define LOGOG_MUTEX_INIT(x)      InitializeCriticalSection (x)
#define LOGOG_MUTEX_DELETE(x)    DeleteCriticalSection (x)
#define LOGOG_MUTEX_LOCK(x)      EnterCriticalSection (x)
#define LOGOG_MUTEX_UNLOCK(x)    LeaveCriticalSection (x)
#define LOGOG_MUTEX_CTOR(x)
#endif // LOGOG_FLAVOR_WINDOWS

#ifdef LOGOG_FLAVOR_POSIX
#define LOGOG_MUTEX(x)           pthread_mutex_t (x);
#define LOGOG_MUTEX_INIT(x)      pthread_mutex_init(x, 0)
#define LOGOG_MUTEX_DELETE(x)    pthread_mutex_destroy (x)
#define LOGOG_MUTEX_LOCK(x)      pthread_mutex_lock (x)
#define LOGOG_MUTEX_UNLOCK(x)    pthread_mutex_unlock (x)
#define LOGOG_MUTEX_CTOR(x)
#endif // LOGOG_FLAVOR_POSIX
#endif // LOGOG_MUTEX

#ifndef LOGOG_MUTEX
#error You need to define mutex macros for your platform; please see mutex.hpp
#endif

//! [Mutex]

/** An object that can only be locked by one thread at a time.  Implement the LOGOG_MUTEX_* functions for your platform
 * to support the Mutex object.
 * A mutex is intended to be used with the ScopedLock object to implement critical sections within logog.
 * \sa ScopedLock
 */
class Mutex : public Object
{
public:
    Mutex() LOGOG_MUTEX_CTOR( m_Mutex )
    {
        LOGOG_MUTEX_INIT(&m_Mutex);
    }
    ~Mutex()
    {
        LOGOG_MUTEX_DELETE(&m_Mutex);
    }
    /** Acquires a lock on the mutex.  Only one thread is permitted to lock the mutex at one time. */
    void Lock()
    {
        LOGOG_MUTEX_LOCK(&m_Mutex);
    }
    /** Releases the lock on the mutex. */
    void Unlock()
    {
        LOGOG_MUTEX_UNLOCK(&m_Mutex);
    }

protected:
    Mutex(const Mutex &)
    LOGOG_MUTEX_CTOR( m_Mutex )
    {
        LOGOG_MUTEX_INIT(&m_Mutex);
    };
    Mutex & operator = (const Mutex &)
    LOGOG_MUTEX_CTOR( m_Mutex )
    {
        LOGOG_MUTEX_INIT(&m_Mutex);
        return *this;
    };
    LOGOG_MUTEX( m_Mutex );
};

/** Asserts a lock while this object exists and is in scope.  A ScopedLock should be
 * declared in "auto" format, typically on the stack.
 */
class ScopedLock : public Object
{
public :
    /** Instances and locks a ScopedLock.
     * \param mutex The mutex to attempt to lock.  Program execution halts at this point until the lock can be obtained.
     */
    ScopedLock( Mutex &mutex )
    {
        m_pMutex = &mutex;
        m_pMutex->Lock();
    }
    ~ScopedLock()
    {
        m_pMutex->Unlock();
    }
protected:
    /** A pointer to the lockable mutex. */
    Mutex *m_pMutex;

private:
    /* no default constructor */
    ScopedLock();
    /* no copy constructor */
    ScopedLock( const ScopedLock &other );
};

#ifdef LOGOG_LEAK_DETECTION
Mutex s_AllocationsMutex;
static void LockAllocationsMutex()
{
    s_AllocationsMutex.Lock();
}
static void UnlockAllocationsMutex()
{
    s_AllocationsMutex.Unlock();
}
#endif // LOGOG_LEAK_DETECTION

static Mutex &GetStringSearchMutex()
{
    Statics *pStatic = &Static();
    Mutex **ppStringSearchMutex = (Mutex **)&( pStatic->s_pStringSearchMutex );

#ifdef LOGOG_INTERNAL_DEBUGGING
    if ( pStatic == NULL )
        LOGOG_INTERNAL_FAILURE;
#endif
    if ( *ppStringSearchMutex == NULL )
        *ppStringSearchMutex = new Mutex();

    return *(( Mutex *)( *ppStringSearchMutex ));
}

static void DestroyStringSearchMutex()
{
    Statics *pStatic = &Static();
    Mutex **ppStringSearchMutex = (Mutex **)&( pStatic->s_pStringSearchMutex );

    if ( *ppStringSearchMutex != NULL )
    {
        delete *ppStringSearchMutex;
        *ppStringSearchMutex = NULL;
    }
}
}

#endif // __LOGOG_MUTEX_HPP_
