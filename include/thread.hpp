/**
 * \file thread.hpp Defines a thread object for the current platform.
 */

#ifndef __LOGOG_THREAD_HPP__
#define __LOGOG_THREAD_HPP__

//! [Thread]
#ifndef LOGOG_THREAD
#if defined(LOGOG_FLAVOR_WINDOWS)

#include <process.h>

typedef unsigned int (WINAPI * __logog_pThreadFn)(void *);

/* 64-bit versions of Windows use 32-bit handles for interoperability. When
* sharing a handle between 32-bit and 64-bit applications, only the lower
* 32 bits are significant, so it is safe to truncate the handle (when passing
* it from 64-bit to 32-bit) or sign-extend the handle (when passing it from
* 32-bit to 64-bit)."
* source: https://msdn.microsoft.com/en-us/library/windows/desktop/aa384203%28v=vs.85%29.aspx
*/

#define LOGOG_THREAD HANDLE

#define LOGOG_THREAD_CREATE(handle, attr, pStartFn, arg) \
	(int)((*handle=(HANDLE) _beginthreadex (NULL,  /* security */ \
			0, /* stack size */ \
			(__logog_pThreadFn)pStartFn, /* start address */ \
			arg, /* pv argument */ \
			0, /* init flag */ \
			NULL /*thread addr */ ))==NULL)

#define LOGOG_THREAD_JOIN( thread ) \
	( (WaitForSingleObject(( thread ),INFINITE)!=WAIT_OBJECT_0) \
				|| !CloseHandle(thread) \
				)
#define LOGOG_THREAD_SELF (LOGOG_THREAD)((size_t)GetCurrentThreadId())

#endif // defined(LOGOG_FLAVOR_WINDOWS)

#if defined(LOGOG_FLAVOR_POSIX)

#define LOGOG_THREAD \
	pthread_t

#define LOGOG_THREAD_CREATE(handle, attr, pStartFn, arg) \
	pthread_create(handle, attr, pStartFn, arg)

#define LOGOG_THREAD_JOIN(thread) \
	pthread_join(thread, NULL)

#define LOGOG_THREAD_SELF \
	pthread_self()

#endif

#endif // LOGOG_THREAD

#ifndef LOGOG_THREAD
#error You need to define mutex macros for your platform; please see mutex.hpp
#endif

//! [Thread]

namespace logog
{

/** A thread abstraction.  Requires definition of macros to describe how to create, start, and wait for threads to terminate. */
class Thread
{
public:

    /** A type describing the entry point of a function. */
    typedef void* (*ThreadStartLocationType)(void *);

    /** Creating a new thread requires the starting location as well as a single void pointer to the argument to a function. */
    Thread(ThreadStartLocationType fnThreadStart, void* pvParams)
    {
        m_pFnThreadStart = fnThreadStart;
        m_pvThreadParams = pvParams;
        m_Thread = nullptr;
    }

    /** Cause the created thread to commence execution asynchronously. */
    int Start()
    {
        return LOGOG_THREAD_CREATE(&m_Thread, NULL, m_pFnThreadStart, m_pvThreadParams);
    }

    /** Causes the current thread to wait for completion of the provided thread.
     ** \param thread The thread object to wait for
     */
    static int WaitFor(const Thread& thread)
    {
        return LOGOG_THREAD_JOIN(thread.m_Thread);
    }

#ifdef LOGOG_THREAD_JOIN_SUPPORT
    static void Join(const std::vector<Thread*>& threads)
    {
        for(size_t i=0; i<threads.size(); i++)
            WaitFor(*threads.at(i));
    }

    static void Delete(std::vector<Thread*>& threads)
    {
        for(size_t i=0; i<threads.size(); i++)
            delete threads.at(i);
        threads.clear();
    }
#endif // LOGOG_THREAD_JOIN_SUPPORT

    /** Returns a LOGOG_THREAD representing the calling process. */
    static LOGOG_THREAD GetCurrent()
    {
        return (LOGOG_THREAD) LOGOG_THREAD_SELF ;
    }

private:
    /** A platform-specific identifier for the calling process. */
    LOGOG_THREAD m_Thread;
    /** The entry point for this thread. */
    ThreadStartLocationType m_pFnThreadStart;
    /** An arbitrary argument to the thread entry point. */
    void* m_pvThreadParams;
};
}

#endif // __LOGOG_THREAD_HPP_
