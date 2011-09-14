 
/* 
 * \file mutex.cpp
 */

#include "logog.hpp"

namespace logog {

	Mutex::Mutex() LOGOG_MUTEX_CTOR( m_Mutex )
	{
		LOGOG_MUTEX_INIT(&m_Mutex);
	}

	Mutex::Mutex( const Mutex & )
	{
		LOGOG_MUTEX_INIT(&m_Mutex);
	}

	Mutex & Mutex::operator = (const Mutex &)
	{
		LOGOG_MUTEX_INIT(&m_Mutex);
		return *this;
	};

	Mutex::~Mutex()
	{
		LOGOG_MUTEX_DELETE(&m_Mutex);
	}

	void Mutex::Lock()
	{
		LOGOG_MUTEX_LOCK(&m_Mutex);
	}

	void Mutex::Unlock()
	{
		LOGOG_MUTEX_UNLOCK(&m_Mutex);
	}

	ScopedLock::ScopedLock( Mutex &mutex )
	{
		m_pMutex = &mutex;
		m_pMutex->Lock();
	}

	ScopedLock::~ScopedLock()
	{
		m_pMutex->Unlock();
	}

#ifdef LOGOG_LEAK_DETECTION
	Mutex s_AllocationsMutex;
	void LockAllocationsMutex()
	{
		s_AllocationsMutex.Lock();
	}
	void UnlockAllocationsMutex()
	{
		s_AllocationsMutex.Unlock();
	}
#endif // LOGOG_LEAK_DETECTION

	Mutex &GetStringSearchMutex()
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

	void DestroyStringSearchMutex()
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

