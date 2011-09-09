/**
 * \file unittest.hpp Unit testing interface; may be used for other programs as well.
 */
#ifndef __LOGOG_UNITTEST_HPP
#define __LOGOG_UNITTEST_HPP

#include <iostream>
#include <string>

namespace logog {

	/**
	 * \page unittesting Unit test framework
	 * 
	 * A unit test framework is included with logog.  This framework was intended specifically to exercise logog functionality, but it may also
	 * be used as a general purpose test framework.
	 * 
	 * A typical test program will look like this:
	 * \code
	 * int main( int argc, char *argv[] )
	 * {
	 *     int nResult;
	 *     nResult = RunAllTests();
	 *     ShutdownTests();
	 *     return nResult;
	 * }
	 * \endcode
	 * 
	 * To define a unit test, create a function of the following form:
	 * 
	 * \code
	 * UNITTEST( AdditionTest )
	 * {
	 *     logog::Initialize();
	 *     
	 *     int nResult = 0;
	 *
	 *     if ( 2 + 2 == 4 )
	 *     {
	 *        cout << "Sane." << endl;
	 *     }
	 *     else 
	 *     {
	 *        cout << "Insane!" << endl;
	 *        nResult = 1;
	 *     }
	 *
	 *     logog::Shutdown();
	 *
	 *     return nResult;
	 *	   
	 * };
	 * \endcode
	 * 
	 * The UNITTEST() macro defines a unique UnitTest object that encompasses your function.
	 * Your function should take no parameters and return an integer value.  It should return
	 * zero if the test was successful, and non-zero if the test was unsuccesful.  If any
	 * of your tests fail, the stub main() program will propagate that error to the operating
	 * system, which in turn can be used to halt an automated build system.
	 * 
	 * The unit testing framework is known to leak memory.  However, the underlying logog macros are not known to leak memory (let us know
	 * if you find any leaks).
	 */

	/** A standard string type, used for labelling a test.  We don't use LOGOG_STRING here because that class is mutable
	 ** and it allocates memory. 
	 **/
	typedef const char * TestNameType;
	class UnitTest;

	/** A registry for all tests.  All tests are instanced using the UNITTEST() macro and stored in the LogogTestRegistry.
	 ** \ref UNITTEST
	 **/
	typedef LOGOG_LIST< UnitTest * > TestRegistryType;

	/** All unit tests are registered in here at program initialization time. */
	TestRegistryType &LogogTestRegistry()
	{
		static TestRegistryType *pRegistry = new TestRegistryType();
		return *pRegistry;
	}

	/** A TestSignup is responsible for recording each instanced UnitTest in the test registry. */
	class TestSignup
	{
	public:
		/** Creates a new TestSignup.  Only called from constructor for UnitTest. */
		TestSignup( UnitTest *pTest )
		{
			m_pTest = pTest;
			LogogTestRegistry().push_back( pTest );
		}
	protected:
		/** A pointer back to the UnitTest that created this TestSignup */
		UnitTest *m_pTest;
	private:
		TestSignup();
	};

	/** The base class for unit testing.  Children of UnitTest are instanced by the UNITTEST() macro. */
	class UnitTest
	{
	public:
		/** Instances a test.  An instanced test is automatically executed when the RunAllTests() function is called.
		 * \param sTestName A string representing the name of this test.
		 */
		UnitTest( const TestNameType &sTestName ) 
		{ 
			m_sTestName = sTestName;
			m_pTestSignup = new TestSignup( this );
		}
		virtual ~UnitTest()
		{
			delete m_pTestSignup;
		}
		/** Returns the name of this UnitTest provided at construction time. */
		virtual TestNameType &GetName() { return m_sTestName; }
		/** Child classes of UnitTest() must provide a RunTest() function.  A RunTest() function must initialize logog, conduct its
		 ** tests, and return 0 if the test was successful, a non-0 value otherwise.  If any RunTest() function returns any value other than
		 ** zero, then the main RunAllTests() function will return non zero as well.
		 */
		virtual int RunTest() = 0;

	protected:
		/** The name of this particular test. */
		TestNameType m_sTestName;
		/** A pointer to the TestSignup constructed by this UnitTest. */
		TestSignup *m_pTestSignup;
	private:
		UnitTest();
	};

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

/** This should be the function prefix for a unit test.  It defines a new class for the test inherited from UnitTest.  It instances
 ** a member of the class at run-time (before main() starts).  Lastly it provides the function definition for the actual test class.
 */
#define UNITTEST( x ) class x : public UnitTest { \
public: \
	x( TestNameType name ) : \
	  UnitTest( name ) \
	  {} \
	  virtual ~x() {}; \
	  virtual int RunTest(); \
	}; \
	x __LogogTestInstance_ ## x ( #x ); \
	int x::RunTest()

}

#endif // __LOGOG_UNITTEST_HPP
