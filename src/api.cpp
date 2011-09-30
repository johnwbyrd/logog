 /* 
 * \file api.cpp
 */

#include "logog.hpp"

namespace logog {

int Initialize( INIT_PARAMS *params )
{
    if ( params == NULL )
    {
        Static().s_pfMalloc = malloc;
        Static().s_pfFree = free;
    }
    else
    {
        if ( params->m_pfMalloc != NULL )
        {
            Static().s_pfMalloc = params->m_pfMalloc;
            Static().s_pfFree = params->m_pfFree;
        }
        else
        {
            Static().s_pfMalloc = malloc;
            Static().s_pfFree = free;
        }
    }

    // Let's allocate a default filter here.
    GetDefaultFilter();

    // Socket::Initialize();

    return 0;
}

int Shutdown( )
{
    // Socket::Shutdown();

#ifdef LOGOG_DESTROY_STATIC_AREA
    delete &( Static() );
#else
    Static().Reset();
#endif

#ifdef LOGOG_LEAK_DETECTION
	ReportMemoryAllocations();
#endif

    return 0;
}


}

