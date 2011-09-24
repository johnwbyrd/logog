/**
 * \file object.hpp Base class for all allocated logog objects.
 */

#ifndef __LOGOG_OBJECT_HPP__
#define __LOGOG_OBJECT_HPP__

#ifdef LOGOG_LEAK_DETECTION
#include <cstdio>
#include <iostream>
using namespace std;
#endif // LOGOG_LEAK_DETECTION

namespace logog
{

#ifdef LOGOG_LEAK_DETECTION
/** Iteration over an unordered map is slow, but we only do it at shutdown time.  Access to a lookup or insert
 ** is very fast.  Additionally, we do not allocate leak detection records from our own heap; it saves us from
 ** doing a recursive allocation.
 **/
typedef void *PointerType;

/** A type describing the currently outstanding memory allocations.  Note that this type does not inherit from
 ** Object, because to do so would create an infinite recursion when memory is allocated.
 **/
typedef LOGOG_UNORDERED_MAP< PointerType, size_t > AllocationsType;

/** All currently outstanding memory allocations, including their size.  */
static AllocationsType s_Allocations;

/** A global function to lock the global allocations mutex.  We must do this as a static
 ** void because Mutexes depend on Objects.  This should probably belong in a class, but then
 ** we get recursive class definitions.
 ** */
static void LockAllocationsMutex();

/** A global function to unlock the global allocations mutex.  We must do this as a static
 ** void because Mutexes depend on Objects. */
static void UnlockAllocationsMutex();
#endif // LOGOG_LEAK_DETECTION

/** Base class for all objects allocated with logog. */
class Object
{
public:
    Object();
	/* Some builds complain about ~Object() being virtual... sorry lint :( */
    virtual ~Object();
    /** Initializes an object of size new. */
    static void *operator new( size_t nSize );
    /** Initializes an array of size new. */
    static void *operator new[](size_t nSize);
    /** Deletes an object pointed to by ptr. */
    static void operator delete( void *ptr );
    /** Deletes an object array pointed to by ptr. */
    static void operator delete[]( void *ptr );

    /** Allocates nSize bytes of memory.  You must call logog::Initialize() before calling this function.
     * \sa Initialize()
     */
    static void *Allocate( size_t nSize );

    /** Deallocate a pointer previously acquired by Allocate(). */
    static void Deallocate( void *ptr );
 };

/** An STL-compatible allocator which redirects all memory requests to the logog allocator.  Used for all STL-like classes within logog. */
template <class T>
class Allocator
{
public:
    /** Memory allocation size type. */
    typedef size_t    size_type;
    /** Memory allocation comparison type. */
    typedef ptrdiff_t difference_type;
    /** A pointer to T type. */
    typedef T        *pointer;
    /** A const pointer to T type. */
    typedef const T  *const_pointer;
    /** A reference to T type. */
    typedef T        &reference;
    /** A const reference to T type. */
    typedef const T  &const_reference;
    /** A value type (T itself). */
    typedef T         value_type;

    Allocator() {}
    /** Not implemented here -- required by the STL standard though. */
    Allocator( const Allocator & ) {}

    /** Allocate and return n value_types of memory through this allocator.  Requires that logog::Initialize() has been called. */
    pointer   allocate( size_type n, const void * = 0 )
    {
        T *t = ( T * ) Object::Allocate( n * sizeof( value_type ) );
        return t;
    }

    /** Frees memory previously allocated by allocate(). */
    void      deallocate( void *p, size_type )
    {
        if ( p )
        {
            Object::Deallocate( p );
        }
    }

    /** Returns the address of a reference to T. */
    pointer           address( reference x ) const
    {
        return &x;
    }
    /** Returns the address of a const reference to T. */
    const_pointer     address( const_reference x ) const
    {
        return &x;
    }
    /** STL required override for = operator. */
    Allocator<T>&  operator=( const Allocator & )
    {
        return *this;
    }
    /** Constructs a new T at location p with value val. */
    void              construct( pointer p, const T &val )
    {
        new(( T * ) p ) T( val );
    }
    /** Destroys a T at location p. */
    void              destroy( pointer p )
    {
#ifdef LOGOG_FLAVOR_WINDOWS
        // MSVC tends to complain unless we reference this pointer here.
        p;
#endif // LOGOG_FLAVOR_WINDOWS
        p->~T();
    }

    /** The largest size of an object that can be allocated with this allocator. */
    size_type         max_size() const
    {
        return size_t( -1 );
    }

    /** Rebinding to permit allocations of unknown types.  Part of std::allocator definition.
     * \param other The other "unknown" type to be permitted access to this allocator */
    template <class U>
    struct rebind
    {
        /** The "other" class that will use this allocator for its allocation. */
        typedef Allocator<U> other;
    };

    /** Required by STL -- unused here. */
    template <class U>
    Allocator( const Allocator<U>& ) {}

    /** Permit this allocator to be used for assignment in other classes */
    template <class U>
    Allocator &operator=( const Allocator<U>& )
    {
        return *this;
    }
};

/* All specializations of this allocator are interchangeable. */
template <class T1, class T2>
bool operator== ( const Allocator<T1>&,
                  const Allocator<T2>& )
{
    return true;
}
template <class T1, class T2>
bool operator!= ( const Allocator <T1>&,
                  const Allocator<T2>& )
{
    return false;
}

//

/** Returns the current number of outstanding memory allocations in logog.  Returns -1 iff LOGOG_LEAK_DETECTION
 ** has not been defined at compile time.
 **/
extern int MemoryAllocations();

/** Sends a report to cout describing the current memory allocations that exist.  Returns the outstanding number of
 ** memory allocations, or -1 iff LOGOG_LEAK_DETECTION is defined.
 **/
extern int ReportMemoryAllocations();

}
#endif // __LOGOG_OBJECT_HPP
