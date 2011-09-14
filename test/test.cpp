/** Only define this macro in source files that create unit tests.  This setting brings in the std namespace, so its 
 ** use is not recommended outside of unit tests. */
#define LOGOG_UNIT_TESTING 1

/** Define this macro to debug logog itself.  This setting enables sanity checks on many logog functions.  This setting
 ** is not very useful for end users of logog. */
#define LOGOG_INTERNAL_DEBUGGING 1

/** Define this macro to check all memory allocations and frees.  This setting will check against double allocations
 ** and double deletes as well.  Do not enable this on production code -- it'll slow down the performance considerably.
 **/
#define LOGOG_LEAK_DETECTION 1

/** Define this macro to report on ALL allocations and frees that happen through logog.
 ** Enable this if you get paid by lines of program output. */
// #define LOGOG_REPORT_ALLOCATIONS 1

/** Change this to higher constants to exponentially increase test difficulty. */
const int TEST_STRESS_LEVEL = 1;

#include "logog.hpp"

#include <cstdio>

using namespace logog;
using namespace std;

UNITTEST( SimpleLocking )
{
//! [SimpleLocking]
	LOGOG_INITIALIZE();

	logog::Mutex m;

	{
		logog::ScopedLock s1( m );
		cout << "Lock is on" << endl;
	}

	logog::Mutex *pM = new logog::Mutex();

	{
		logog::ScopedLock s1( *pM );
		cout << "Lock is on" << endl;
	}

	delete pM;

	cout << "Lock unlocked" << endl;

	LOGOG_SHUTDOWN();

	return 0;
//! [SimpleLocking]
}

int _s_ThreadLockingTest = 0;

void LockingThread( void *pvMutex )
{
	const int NUM_TRIALS = 10 * TEST_STRESS_LEVEL;
	Mutex *pMutex = (Mutex *)pvMutex;

	if ( pMutex == NULL )
	{
		_s_ThreadLockingTest = 1;
		cout << "LockingThread received a NULL argument!" << endl;
		return;
	}

	for ( int t = 0; t < NUM_TRIALS; t++ )
	{
		{
			ScopedLock sl( *pMutex );
			INFO( "Thread acquired lock on trial %d", t);
		}

		INFO( "Thread released lock" );
	}
}


UNITTEST( Subscription )
{
	int nResult = 0;

//! [Subscription]
	LOGOG_INITIALIZE();

	{
		Topic n1, n2;

		// should succeed
		if ( n1.SubscribeTo( n2 ) != true )
			nResult++;

		// should indicate no insertion took place
		if ( n2.PublishTo( n1 ) != false )
			nResult++;

		n2.Transmit();

		if ( n2.UnpublishTo( n1 ) != true )
			nResult++;

		if ( n1.UnsubscribeTo( n2 ) != false )
			nResult++;
	}

	LOGOG_SHUTDOWN();

//! [Subscription]

	return nResult;
}

UNITTEST( GlobalNodelist )
{
	LOGOG_INITIALIZE();

	const int MAX_NODES = 10 * TEST_STRESS_LEVEL; 

	LOGOG_VECTOR< Topic *> vTopics;

	for (int t = 0; t < MAX_NODES; t++ )
		vTopics.push_back( new Topic );

	for ( int i = 0; i < MAX_NODES; i++ )
		for ( int j = i + 1; j < MAX_NODES; j++ )
			vTopics.at( i )->SubscribeTo( *vTopics.at( j ) );

	LockableNodesType *pNodes = ( LockableNodesType * )Static().s_pAllNodes;

	// For this test we assume that the default Filter has always been created.
	if ( pNodes->size() != ( MAX_NODES + 1))
	{
		cout << "Incorrect number of nodes!" << endl;
		return 1;
	}

	// Try having that master node send a message to all other nodes!
	vTopics.at( MAX_NODES - 1)->Transmit();

	// Let's try leaving all the nodes allocated and let logog clean them all up.
	LOGOG_SHUTDOWN();

	if ( Static().s_pAllNodes != NULL )
	{
		cout << "Could not delete all nodes!" << endl;
		return 1;
	}

	return 0;
}


UNITTEST( ThreadLocking )
{
	LOGOG_INITIALIZE();
	{
		Cout out; // only one of these please

		const int NUM_THREADS = 10 * TEST_STRESS_LEVEL;

		LOGOG_VECTOR< Thread *> vpThreads;
		Mutex mSharedMutex;

		for ( int t = 0; t < NUM_THREADS; t++ )
			vpThreads.push_back( new Thread( (Thread::ThreadStartLocationType) LockingThread,&mSharedMutex ));

		for ( int t = 0; t < NUM_THREADS; t++ )
			vpThreads[ t ]->Start();

		for ( int t = 0; t < NUM_THREADS; t++ )
			Thread::WaitFor( *vpThreads[ t ]);
	}

	LOGOG_SHUTDOWN();

	return _s_ThreadLockingTest;
}

UNITTEST( TimerTest )
{
	const int NUM_TRIALS = 10 * TEST_STRESS_LEVEL;
	const int WAIT_TIME = 1000000 * TEST_STRESS_LEVEL;

	Timer time;
	LOGOG_TIME fCurrent = time.Get();

	for ( int t = 0; t < NUM_TRIALS; t++  )
	{
		/* do busy work */
		int k = 0;
		for ( int i = 0; i < WAIT_TIME; i++ )
			k = k + 1;  // fixes a lint warning

		if ( time.Get() < fCurrent )
		{
			cout << "Timer error!  Non monotonic timer behavior" << endl;
			return 1;
		}
		fCurrent = time.Get();
		cout << "Current reported time: " << fCurrent << endl;
	}

	return 0;
}

UNITTEST( TopicTest1 )
{
	LOGOG_INITIALIZE();

	{
		Topic t1( LOGOG_LEVEL_WARN, "file1.cpp", 50 );
		Topic t2( LOGOG_LEVEL_WARN );
		Topic t3( LOGOG_LEVEL_ERROR, "file2.cpp", 100 );
		Topic t4( LOGOG_LEVEL_WARN, NULL, NULL, "Group", "Category", "Message", 30.0f);
		Topic t5( LOGOG_LEVEL_CRITICAL, NULL, NULL, "GroupGROUP", "Important Category", "Your Message Here", 150.0f);

		if (t1.CanSubscribeTo( t2 ) == true)
		{
			cout << "Subscription test failed; t1 can subscribe to t2" << endl;
			return 1;
		}

		if (t2.CanSubscribeTo( t1 ) == false )
		{
			cout << "Subscription test failed; t2 can't subscribe to t1" << endl;
			return 1;
		}

		if ( t1.CanSubscribeTo( t3 ) == true )
		{
			cout << "Subscription test failed; t1 can subscribe to t3" << endl;
			return 1;
		}

		if (t2.CanSubscribeTo( t3 ) == false )
		{
			cout << "Subscription test failed; t2 can't subscribe to t3" << endl;
			return 1;
		}

		if (t4.CanSubscribeTo( t5 ) == false )
		{
			cout << "Subscription test failed; t4 can't subscribe to t5" << endl;
			return 1;
		}

		if ( t5.CanSubscribeTo( t4 ) == true )
		{
			cout << "Subscription test failed; t5 can subscribe to t4" << endl;
			return 1;
		}
	}

	LOGOG_SHUTDOWN();

	return 0;
}

UNITTEST( Checkpoint1 )
{
	LOGOG_INITIALIZE();

	{
		Checkpoint check( LOGOG_LEVEL_ALL, __FILE__, __LINE__, "Group", "Category", "Message", 1.0f );

		Topic t;

		Cerr cerrobj;
		Cerr cerrgnu;
		OutputDebug outdebug;

		FormatterGCC f;
		cerrgnu.SetFormatter( f );

		check.PublishTo( t );
		t.PublishToMultiple( AllTargets() );

		cout << "Setup complete; ready to transmit " << endl;
		check.Transmit();
	}

	LOGOG_SHUTDOWN();

	return 0;
}

UNITTEST( LowLevelMessage1 )
{
	LOGOG_INITIALIZE();
	{
		int foo = 9001;
		int maxfoo = 9000;

		Cerr cerrobj;
		Filter filter;

		for ( int t = 0; t < 5; t++ )
		{
			if ( foo > maxfoo )
				LOGOG_LEVEL_GROUP_CATEGORY_MESSAGE_NO_VA( LOGOG_LEVEL_WARN, "Group", "Category", "Foo is over 9000!  Current value is 9001." );
		}
	}

	LOGOG_SHUTDOWN();

	return 0;
}

UNITTEST( FormatString1 )
{
	LOGOG_INITIALIZE();
	{
		String s;

		s.format( "This is a test message: %d %x %f\n", 1234, 0xf00d, 1.234f );
		cout << s;

		const char *p = "Here is a string";

		s.format( "Here are six strings: %s %s %s %s %s %s \n", p,p,p,p,p,p );
		cout << s;
	}

	LOGOG_SHUTDOWN();

	return 0;
}

UNITTEST( FormatTopic1 )
{
	LOGOG_INITIALIZE();
	{
		Cout out;
		LogFile f("log.txt");
		Message m;

		m.PublishToMultiple( AllTargets() );

		m.Format( "This is a test message: %d %x %f", 1234, 0xf00d, 1.234f );
		m.Transmit();

		const char *p = "Here is a string";

		m.Format( "Here are six strings: %s %s %s %s %s %s", p,p,p,p,p,p );
		m.Transmit();
	}

	LOGOG_SHUTDOWN();

	return 0;
}

UNITTEST( GroupCategory1 )
{
//! [GroupCategory1]
	/*
	The following example produces something like:
	.\test.cpp(364) : emergency: {Graphics} [Unrecoverable] The graphics card has been destroyed
	.\test.cpp(368) : warning: {Graphics} [Recoverable] The graphics card has been replaced
	.\test.cpp(372) : warning: {Audio} [Recoverable] The headphones are unplugged
	.\test.cpp(377) : info: Everything's back to normal
	*/

	LOGOG_INITIALIZE();

	{
		Cerr err;

#undef LOGOG_GROUP
#undef LOGOG_CATEGORY
#define LOGOG_GROUP  "Graphics"
#define LOGOG_CATEGORY "Unrecoverable"

		EMERGENCY("The graphics card has been destroyed");

#undef LOGOG_CATEGORY
#define LOGOG_CATEGORY	"Recoverable"

		WARN("The graphics card has been replaced");

#undef LOGOG_GROUP
#define LOGOG_GROUP "Audio"

		WARN("The headphones are unplugged");

#undef LOGOG_CATEGORY
#undef LOGOG_GROUP
#define LOGOG_CATEGORY NULL
#define LOGOG_GROUP NULL

		INFO("Everything's back to normal");
	}

	LOGOG_SHUTDOWN();
	//! [GroupCategory1]
	return 0;
}

UNITTEST( GroupCategory2 )
{
//! [GroupCategory2]
	LOGOG_INITIALIZE();

	{
		GetDefaultFilter().Category("Unrecoverable");
		Cerr err;

		WARN("Logging messages in the Unrecoverable category...");


#undef LOGOG_GROUP
#undef LOGOG_CATEGORY
#define LOGOG_GROUP  "Graphics"
#define LOGOG_CATEGORY "Unrecoverable"

		EMERGENCY("The graphics card has been destroyed");

#undef LOGOG_CATEGORY
#define LOGOG_CATEGORY	"Recoverable"

		WARN("The graphics card has been replaced");

#undef LOGOG_GROUP
#define LOGOG_GROUP "Audio"

		WARN("The headphones are unplugged");

#undef LOGOG_CATEGORY
#undef LOGOG_GROUP
#define LOGOG_CATEGORY NULL
#define LOGOG_GROUP NULL

	}

	LOGOG_SHUTDOWN();
//! [GroupCategory2]
	return 0;
}

UNITTEST( GroupCategory3 )
{
	LOGOG_INITIALIZE();

	{
		GetDefaultFilter().Group("Graphics");
		Cerr err;
		WARN("This message won't happen because it's not in the Graphics group");

#undef LOGOG_GROUP
#undef LOGOG_CATEGORY
#define LOGOG_GROUP  "Graphics"
#define LOGOG_CATEGORY "Unrecoverable"

		EMERGENCY("The graphics card has been destroyed");

#undef LOGOG_CATEGORY
#define LOGOG_CATEGORY	"Recoverable"

		WARN("The graphics card has been replaced");

#undef LOGOG_GROUP
#define LOGOG_GROUP "Audio"

		WARN("The headphones are unplugged");

#undef LOGOG_CATEGORY
#undef LOGOG_GROUP
#define LOGOG_CATEGORY NULL
#define LOGOG_GROUP NULL
	}

	LOGOG_SHUTDOWN();
	return 0;
}

UNITTEST( GroupCategory4 )
{
//! [GroupCategory4]
	LOGOG_INITIALIZE();

	{
		GetDefaultFilter().Group("Graphics");
		Filter filter;
		filter.Group("Audio");
		Cerr err;

		WARN("This message won't happen because it's not in the Graphics group");

#undef LOGOG_GROUP
#undef LOGOG_CATEGORY
#define LOGOG_GROUP  "Graphics"
#define LOGOG_CATEGORY "Unrecoverable"

		EMERGENCY("The graphics card has been destroyed");

#undef LOGOG_CATEGORY
#define LOGOG_CATEGORY	"Recoverable"

		WARN("The graphics card has been replaced");

#undef LOGOG_GROUP
#define LOGOG_GROUP "Audio"

		WARN("The headphones are unplugged");

#undef LOGOG_GROUP
#define LOGOG_GROUP "Inputs"

		WARN("The inputs have been yanked off but this fact won't be reported!");

#undef LOGOG_CATEGORY
#undef LOGOG_GROUP
#define LOGOG_CATEGORY NULL
#define LOGOG_GROUP NULL
	}

	LOGOG_SHUTDOWN();
//! [GroupCategory4]
	return 0;
}


UNITTEST( Info1 )
{
	LOGOG_INITIALIZE();
	{
		Cout out;

		for ( int i = 0; i < 2; i++ )
		{
			for ( int t = 0; t < 10; t++ )
			{
				INFO( "t is now: %d", t );
				if ( t > 8 )
					WARN("t is pretty high now!");
			}

			DBUG("This warning isn't very interesting");
			EMERGENCY("THIS IS SUPER IMPORTANT!");
		}

		LOGOG_SET_LEVEL( LOGOG_LEVEL_DEBUG );
		LOGOG_DEBUG("Messages instantiated for the first time will be called now.  However, debug messages");
		LOGOG_DEBUG("instantiated before the LOGOG_SET_LEVEL won't be transmitted.");

	}

	LOGOG_SHUTDOWN();

	return 0;
}

void GeneratePseudoRandomErrorMessages()
{
	int pr = 0xf8d92347;
	int ps;

	for ( int t = 0; t < TEST_STRESS_LEVEL * 10; t++ )
	{
		pr = pr * 0xd9381381 + 0x13d7b;
		ps = pr % ( (1 << 19) - 1 );

		ERR( "We must inform you of this pseudo-random error code: %d", ps );

		// Do a non specific amount of busy work.
		int swap1 = 0, swap2 = 1;
		for ( int i = 0; i < ps; i++ )
		{
			int swap3 = swap1;
			swap1 = swap2;
			swap2 = swap3;
		}
	}

}

UNITTEST( ImmediateLogging )
{
	LOGOG_INITIALIZE();

	{
		LogFile logFile( "log.txt" );

		GeneratePseudoRandomErrorMessages();
	}

	LOGOG_SHUTDOWN();

	return 0;
}

UNITTEST( DeferredCoutLogging )
{
	LOGOG_INITIALIZE();

	{
		Cout out;
		LogBuffer logBuffer( &out );

		// Make sure that out does not receive messages via the general filter mechanism.
		out.UnsubscribeToMultiple( AllFilters() );

		for ( int i = 1; i <= 10; i++ )
		{
			ERR("This is error %d of 10", i);

			int q = 27832;

			for ( int j = 0; j < TEST_STRESS_LEVEL * 10000000; j++ )
				q *= q;
		}
	}

	LOGOG_SHUTDOWN();

	return 0;
}

UNITTEST( DeferredFileLogging )
{
//! [DeferredFileLogging]
	LOGOG_INITIALIZE();

	{
		LogFile logFile("log.txt");
		LogBuffer logBuffer( &logFile );

		// Make sure that the log file does not receive messages via the general filter mechanism.
		logFile.UnsubscribeToMultiple( AllFilters() );

		for ( int i = 1; i <= 20; i++ )
		{
			WARN("This is warning %d of 20", i);

			int q = 27832;
			for ( int j = 0; j < TEST_STRESS_LEVEL * 10000000; j++ )
				q *= q;
		}
	}

	LOGOG_SHUTDOWN();
//! [DeferredFileLogging]
	return 0;
}

#ifndef LOGOG_TARGET_PS3
int DoPlatformSpecificTestInitialization()
{
	return 0;
}
#endif

int main( int , char ** )
{
	int nPlatformSpecific;
	nPlatformSpecific = DoPlatformSpecificTestInitialization();
	if ( nPlatformSpecific != 0)
		return nPlatformSpecific;

	int nResult;
	nResult = RunAllTests();
	ShutdownTests();
	return nResult;
}

