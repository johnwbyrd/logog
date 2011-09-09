/**
 * \file timer.hpp Time management.
 */

#ifndef __LOGOG_TIMER_HPP__
#define __LOGOG_TIMER_HPP__

#ifdef LOGOG_FLAVOR_POSIX
#include <sys/time.h>
#endif

namespace logog
{
	/** A value for a high resolution timer on this platform.  Time representations are in seconds. */
	typedef double LOGOG_TIME;

	/** A high-resolution timer.  Reports in seconds. */
	class Timer : public Object {
	public:
		Timer()
		{
			m_fStartTime = 0.0f;

#ifdef LOGOG_FLAVOR_WINDOWS
			LARGE_INTEGER TicksPerSecond;
			QueryPerformanceFrequency( &TicksPerSecond );
			m_fTicksPerMicrosecond = (DOUBLE)TicksPerSecond.QuadPart * 0.000001;
#endif
			Set( 0.0f );
		}

		/** Returns the offset from the time since the creation of the timer, or the time set by the most 
		 ** recent Set() call.  Time is assumed to be a value in LOGOG_TIME seconds.
		 ** \sa LOGOG_TIME
		 **/
		LOGOG_TIME Get()
		{
#ifdef LOGOG_FLAVOR_WINDOWS
			LARGE_INTEGER liTime;
			QueryPerformanceCounter( &liTime );

			double dusec;
			dusec =( liTime.QuadPart / m_fTicksPerMicrosecond );

			return ( dusec / 1000000.0f ) - m_fStartTime;
#endif

#ifdef LOGOG_FLAVOR_POSIX
#ifdef LOGOG_TARGET_PS3
	LOGOG_PS3_GET_TIME;
#else // LOGOG_TARGET_PS3
			// General Posix implementation
			timeval tv;
			gettimeofday( &tv, 0 );
			return ((double) (tv.tv_sec) + ((double)(tv.tv_usec ) / 1000000.0 ) - m_fStartTime);
#endif // LOGOG_TARGET_PS3
#endif
		}

		/** Sets the current time for this timer. */
		void Set( LOGOG_TIME time )
		{
			m_fStartTime = time + Get();
		}

	protected:
#ifdef LOGOG_FLAVOR_WINDOWS
		/** Windows only.  Stores the number of high resolution timer ticks per second. */
		double m_fTicksPerMicrosecond;
#endif
		/** Zero, if no calls to Set() have been made; else the value of the previous call to Set(). */
		LOGOG_TIME m_fStartTime;
	};

	static Timer &GetGlobalTimer()
	{
		Statics *pStatic = &Static();

#ifdef LOGOG_INTERNAL_DEBUGGING
		if ( pStatic == NULL )
			LOGOG_INTERNAL_FAILURE;
#endif

		if ( pStatic->s_pTimer == NULL )
			pStatic->s_pTimer = new Timer();

		return *(pStatic->s_pTimer );
	}

	static void DestroyGlobalTimer()
	{
		Statics *pStatic = &Static();
		Timer *pGlobalTimer = pStatic->s_pTimer;

		if ( pGlobalTimer != NULL )
			delete pGlobalTimer;

		pStatic->s_pTimer = NULL;
	}
}

#endif // __LOGOG_TIMER_HPP_
