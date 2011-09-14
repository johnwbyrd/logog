 
/* 
 * \file object.cpp
 */

#include "logog.hpp"

namespace logog {

#ifdef LOGOG_LEAK_DETECTION
AllocationsType s_Allocations;
#endif

	Object::Object() {}

	Object::~Object()
	{

	}

	void *Object::operator new( size_t nSize )
	{
		return Allocate( nSize );
	}

	void *Object::operator new[]( size_t nSize )
	{
		return Allocate( nSize );
	}

	    /** Deletes an object pointed to by ptr. */
	void Object::operator delete( void *ptr )
    {
        Deallocate( ptr );
    }
    /** Deletes an object array pointed to by ptr. */
	void Object::operator delete[]( void *ptr )
    {
        Deallocate( ptr );
    }

    /** Allocates nSize bytes of memory.  You must call logog::Initialize() before calling this function.
     * \sa Initialize()
     */
	void *Object::Allocate( size_t nSize )
    {
        void *ptr = Static().s_pfMalloc( nSize );
#ifdef LOGOG_REPORT_ALLOCATIONS
        cout << "Allocated " << nSize << " bytes of memory at " << ptr << endl;
#endif // LOGOG_REPORT_ALLOCATIONS
#ifdef LOGOG_LEAK_DETECTION
        AllocationsType::iterator it;

        LockAllocationsMutex();
        it = s_Allocations.find( ptr );

        if ( it != s_Allocations.end() )
        {
            cout << "Reallocation detected in memory manager!  We seem to have allocated the same address twice "
                 << "without freeing it!  Address = " << ptr << endl;
            UnlockAllocationsMutex();
            LOGOG_INTERNAL_FAILURE;
        }

        s_Allocations.insert( LOGOG_PAIR< const PointerType, size_t >( ptr, nSize ) );
        UnlockAllocationsMutex();
#endif // LOGOG_LEAK_DETECTION
        return ptr;
    }

    /** Deallocate a pointer previously acquired by Allocate(). */
	void Object::Deallocate( void *ptr )
    {
#ifdef LOGOG_LEAK_DETECTION
        LockAllocationsMutex();
        AllocationsType::iterator it;

        it = s_Allocations.find( ptr );

        if ( it == s_Allocations.end() )
        {
            cout << "Freeing memory not previously allocated!  Address = " << ptr << endl;
            UnlockAllocationsMutex();
            LOGOG_INTERNAL_FAILURE;
        }

#ifdef LOGOG_REPORT_ALLOCATIONS
        cout << "Freeing " << it->second << " bytes of memory at " << it->first << endl;
#endif // LOGOG_REPORT_ALLOCATIONS
        s_Allocations.erase( ptr );
        UnlockAllocationsMutex();
#endif // LOGOG_LEAK_DETECTION
        Static().s_pfFree( ptr );
    }

	int MemoryAllocations()
	{
#ifdef LOGOG_LEAK_DETECTION
		LockAllocationsMutex();
		size_t nSize = s_Allocations.size();

		if ( nSize != 0 )
			cout << "Total active allocations: " << nSize << endl;

		UnlockAllocationsMutex();
		return nSize;
#else // LOGOG_LEAK_DETECTION
		return -1;
#endif // LOGOG_LEAK_DETECTION
	}

	int ReportMemoryAllocations()
	{
#ifdef LOGOG_LEAK_DETECTION
		LockAllocationsMutex();

		for ( AllocationsType::iterator it = s_Allocations.begin();
			it != s_Allocations.end();
			it++ )
		{
			cout << "Memory allocated at " << it->first << " with size " << it->second << " bytes " << endl;
		}

		UnlockAllocationsMutex();
#endif
		return MemoryAllocations();
	}

}

