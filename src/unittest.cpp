
/*
 * \file unittest.cpp
 */

#define LOGOG_UNIT_TESTING 1

#include "logog.hpp"

namespace logog
{

TestRegistryType &LogogTestRegistry()
{
    static TestRegistryType *pRegistry = new TestRegistryType();
    return *pRegistry;
}

TestSignup::TestSignup( UnitTest *pTest )
{
    m_pTest = pTest;
    LogogTestRegistry().push_back( pTest );
}

UnitTest::UnitTest( const TestNameType &sTestName )
{
    m_sTestName = sTestName;
    m_pTestSignup = new TestSignup( this );
}

UnitTest::~UnitTest()
{
	delete m_pTestSignup;
}
/** Returns the name of this UnitTest provided at construction time. */
TestNameType &UnitTest::GetName()
{
	return m_sTestName;
}

/** Executes all currently registered tests and prints a report of success or failure. */
int RunAllTests()
{
    using namespace std;

    int nTests = 0, nTestsSucceeded = 0;
    int nTestResult;
    int nFailures = 0;

    ostream *pOut;
    pOut = &(std::cout);

    nTests = LogogTestRegistry().size();

    if ( nTests == 0 )
    {
        *pOut << "No tests currently defined." << endl;
        return 1;
    }

    for ( TestRegistryType::iterator it = LogogTestRegistry().begin();
            it != LogogTestRegistry().end();
            it++ )
    {
        (*pOut) << "Test " << (*it)->GetName() << " running... " << endl;
        nTestResult = (*it)->RunTest();

        (*pOut) << "Test " << (*it)->GetName();

        if ( nTestResult == 0 )
        {
            *pOut << " successful." << endl;
            nTestsSucceeded++;
        }
        else
        {
            *pOut << " failed!" << endl;
            nFailures++;
        }

        /* Validate that no allocations are currently outstanding.  Make sure to handle the case
         * where leak detection is disabled */
        int nMemoryTestResult = ReportMemoryAllocations();

        if ( nMemoryTestResult != -1 )
        {
            (*pOut) << "Test " << (*it)->GetName() << " has " << nMemoryTestResult << " memory allocations outstanding at end of test." << endl;
            nFailures += nMemoryTestResult;
        }
    }

    *pOut << "Testing complete, "
          << nTests << " total tests, "
          << nTestsSucceeded << " tests succeeded, "
          << ( nTests - nTestsSucceeded ) << " failed"
          << endl;

    return nFailures;
}

/** Should remove all memory allocated during unit testing. */
void ShutdownTests()
{
    delete &LogogTestRegistry();
}

int CompareAndDeleteFile( const LOGOG_CHAR *pValidOutput,
                          const LOGOG_CHAR *pFileName )
{
    FILE *fp;

#ifdef LOGOG_FLAVOR_WINDOWS
    // Microsoft prefers its variant
    int nError = fopen_s( &fp, pFileName, "r" );
    if ( nError != 0 )
        return nError;
#else // LOGOG_FLAVOR_WINDOWS
    fp = fopen( pFileName, "r" );
#endif // LOGOG_FLAVOR_WINDOWS

    while ( !feof( fp ) && *pValidOutput )
    {
        if ( fgetc( fp ) != *pValidOutput++ )
        {
            fclose( fp );
            remove( pFileName );
            return -1; // found a mismatch
        }
    }

    // it matched!
    fclose( fp );
    return remove( pFileName );
}

}

