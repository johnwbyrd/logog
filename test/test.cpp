/** Only define this macro in source files that create unit tests.  This setting brings in the std namespace, so its
 ** use is not recommended outside of unit tests. */
#define LOGOG_UNIT_TESTING 1

/** Change this to higher constants to exponentially increase test difficulty. */
const int TEST_STRESS_LEVEL = 1;

#include "logog.hpp"

#include <cstdio>

#include <fcntl.h>

#ifdef LOGOG_FLAVOR_WINDOWS
#include <io.h>
#endif

using namespace logog;
using namespace std;

UNITTEST( SimpleLocking )
{
//! [SimpleLocking]
    LOGOG_INITIALIZE();

    {
        logog::Mutex m;

        {
            logog::ScopedLock s1( m );
            LOGOG_COUT << _LG("Lock is on") << endl;
        }

        logog::Mutex *pM = new logog::Mutex();

        {
            logog::ScopedLock s1( *pM );
            LOGOG_COUT << _LG("Lock is on") << endl;
        }

        delete pM;

        LOGOG_COUT << _LG("Lock unlocked") << endl;

        logog::Message m1;

    }

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
        LOGOG_COUT << _LG("LockingThread received a NULL argument!") << endl;
        return;
    }

    for ( int t = 0; t < NUM_TRIALS; t++ )
    {
        {
            ScopedLock sl( *pMutex );
            INFO( _LG("Thread acquired lock on trial %d"), t);
        }

        INFO( _LG("Thread released lock %d"), t );
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
        LOGOG_COUT << _LG("Incorrect number of nodes!") << endl;
        return 1;
    }

    // Try having that master node send a message to all other nodes!
    vTopics.at( MAX_NODES - 1)->Transmit();

    // Let's try leaving all the nodes allocated and let logog clean them all up.
    LOGOG_SHUTDOWN();

    if ( Static().s_pAllNodes != NULL )
    {
        LOGOG_COUT << _LG("Could not delete all nodes!") << endl;
        return 1;
    }

    return 0;
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
            LOGOG_COUT << _LG("Timer error!  Non monotonic timer behavior") << endl;
            return 1;
        }
        fCurrent = time.Get();
        LOGOG_COUT << _LG("Current reported time: ") << fCurrent << endl;
    }

    return 0;
}

UNITTEST( TopicTest1 )
{
    LOGOG_INITIALIZE();

    {
        Topic t1( LOGOG_LEVEL_WARN,
                  _LG( "file1.cpp" ), 50 );
        Topic t2( LOGOG_LEVEL_WARN );
        Topic t3( LOGOG_LEVEL_ERROR,
                  _LG( "file2.cpp" ), 100 );
        Topic t4( LOGOG_LEVEL_WARN, NULL, 0,
                  _LG( "Group" ),
                  _LG( "Category" ),
                  _LG( "Message" ),
                  30.0f);
        Topic t5( LOGOG_LEVEL_CRITICAL, NULL, 0,
                  _LG( "GroupGROUP" ),
                  _LG( "Important Category" ),
                  _LG( "Your Message Here"),
                  150.0f);

        if (t1.CanSubscribeTo( t2 ) == true)
        {
            LOGOG_COUT << _LG("Subscription test failed; t1 can subscribe to t2") << endl;
            return 1;
        }

        if (t2.CanSubscribeTo( t1 ) == false )
        {
            LOGOG_COUT << _LG("Subscription test failed; t2 can't subscribe to t1") << endl;
            return 1;
        }

        if ( t1.CanSubscribeTo( t3 ) == true )
        {
            LOGOG_COUT << _LG("Subscription test failed; t1 can subscribe to t3") << endl;
            return 1;
        }

        if (t2.CanSubscribeTo( t3 ) == false )
        {
            LOGOG_COUT << _LG("Subscription test failed; t2 can't subscribe to t3") << endl;
            return 1;
        }

        if (t4.CanSubscribeTo( t5 ) == false )
        {
            LOGOG_COUT << _LG("Subscription test failed; t4 can't subscribe to t5") << endl;
            return 1;
        }

        if ( t5.CanSubscribeTo( t4 ) == true )
        {
            LOGOG_COUT << _LG("Subscription test failed; t5 can subscribe to t4") << endl;
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
        Checkpoint check( LOGOG_LEVEL_ALL, _LG( __FILE__ ), __LINE__,
                          _LG("Group"), _LG("Category"), _LG("Message"), 1.0f );

        Topic t;

        Cerr cerrobj;
        Cerr cerrgnu;
        OutputDebug outdebug;

        FormatterGCC f;
        cerrgnu.SetFormatter( f );

        check.PublishTo( t );
        t.PublishToMultiple( AllTargets() );

        LOGOG_COUT << _LG("Setup complete; ready to transmit ") << endl;
        check.Transmit();
    }

    LOGOG_SHUTDOWN();

    return 0;
}

UNITTEST( FormatString1 )
{
    LOGOG_INITIALIZE();
    {
        String s;

        s.format( _LG("This is a test message.\n"));
        LOGOG_COUT << s.c_str();

        s.format( _LG("This is a test message: %d %x %f\n"), 1234, 0xf00d, 1.234f );
        LOGOG_COUT << s.c_str();

        const LOGOG_CHAR *p = _LG("Here is a string");

#ifdef LOGOG_UNICODE
        s.format( _LG("Here are six strings: %ls %ls %ls %ls %ls %ls \n"), p,p,p,p,p,p );
#else // LOGOG_UNICODE
        s.format( _LG("Here are six strings: %s %s %s %s %s %s \n"), p,p,p,p,p,p );
#endif

        LOGOG_COUT << s.c_str();
    }

    LOGOG_SHUTDOWN();

    return 0;
}

UNITTEST( FormatTopic1 )
{
    LOGOG_INITIALIZE();
    {
        Cout out;
        LogFile f( "log.txt");
        Message m;

        m.PublishToMultiple( AllTargets() );

        m.Format( _LG("This is a test message: %d %x %f"), 1234, 0xf00d, 1.234f );
        m.Transmit();

        const char *p = "Here is a string";

        m.Format( _LG("Here are six strings: %s %s %s %s %s %s"), p,p,p,p,p,p );
        m.Transmit();
    }

    LOGOG_SHUTDOWN();

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

		for ( int t = 0; t < NUM_THREADS; t++ )
			delete vpThreads[t];

    }

    LOGOG_SHUTDOWN();

    return _s_ThreadLockingTest;
}

//! [FormatterCustom1]
class FormatterCustom : public FormatterMSVC
{
    virtual TOPIC_FLAGS GetTopicFlags( const Topic &topic )
    {
        return ( Formatter::GetTopicFlags( topic ) &
                 ~( TOPIC_FILE_NAME_FLAG | TOPIC_LINE_NUMBER_FLAG ));
    }
};

UNITTEST( CustomFormatter )
{
    LOGOG_INITIALIZE();
    {
        Cout out;
        FormatterCustom customFormat;

        out.SetFormatter( customFormat );

        INFO( _LG( "No file and line number info is provided with this output.") );

        /* The following output is produced:
         * info: No file and line number info is provided with this output.
         */
    }
    LOGOG_SHUTDOWN();

    return 0;
}
//! [FormatterCustom1]

//! [FormatterModified]

class FormatterModified : public FormatterMSVC
{
	/* In this example custom Formatter, we've basically rewritten the parent
	 * class to include !!!'s before any topic of LOGOG_LEVEL_WARN
	 * intensity or more important, and we append ... to the message
	 * in all cases.  Large quantities of code are simply copied 
	 * and modified from the parent class.
	 */
    virtual LOGOG_STRING &Format( const Topic &topic, const Target &target )
    {
		TOPIC_FLAGS flags = GetTopicFlags( topic );

		m_sMessageBuffer.clear();

		if ( topic.Level() <= LOGOG_LEVEL_WARN )
			m_sMessageBuffer.append( _LG("!!! ") );

		if ( flags & TOPIC_FILE_NAME_FLAG )
		{
			m_sMessageBuffer.append( topic.FileName() );
			m_sMessageBuffer.append( ':' );
		}

		if ( flags & TOPIC_LINE_NUMBER_FLAG )
		{
			m_sIntBuffer.assign( topic.LineNumber() );
			m_sMessageBuffer.append( m_sIntBuffer );
			m_sMessageBuffer.append( LOGOG_CONST_STRING(": "));
		}

		RenderTimeOfDay();

		if ( flags & TOPIC_LEVEL_FLAG )
		{
			m_sMessageBuffer.append( ErrorDescription( topic.Level()));
			m_sMessageBuffer.append( LOGOG_CONST_STRING(": "));
		}

		if ( flags & TOPIC_GROUP_FLAG )
		{
			m_sMessageBuffer.append( LOGOG_CONST_STRING("{") );
			m_sMessageBuffer.append( topic.Group() );
			m_sMessageBuffer.append( LOGOG_CONST_STRING("} ") );
		}

		if ( flags & TOPIC_CATEGORY_FLAG )
		{
			m_sMessageBuffer.append( LOGOG_CONST_STRING("["));
			m_sMessageBuffer.append( topic.Category() );
			m_sMessageBuffer.append( LOGOG_CONST_STRING("] "));
		}	

		if ( flags & TOPIC_MESSAGE_FLAG )
		{
			m_sMessageBuffer.append( topic.Message() );
		}

		/* At this point we use the String's format() function to reformat the
		* old string into the new format. 
		*/
		if ( topic.Level() <= LOGOG_LEVEL_WARN )
		{
			/* If this message is more important than LOGOG_LEVEL_WARN,
			 * add exclamation marks
			 */
			m_sMessageBuffer.append( _LG(" !!!") );
		}
		else
		{
			/* Otherwise add periods */
			m_sMessageBuffer.append( _LG(" ...") );
		}

		if ( flags & TOPIC_MESSAGE_FLAG )
		{
			m_sMessageBuffer.append( (LOGOG_CHAR)'\n' );
		}

		if ( target.GetNullTerminatesStrings() )
			m_sMessageBuffer.append( (LOGOG_CHAR)NULL );

		/* Return m_sMessageBuffer like the parent class does */
		return m_sMessageBuffer;
    }
};

UNITTEST( CustomFormatterModified )
{
    LOGOG_INITIALIZE();
    {
		/* This test produces output resembling: 
		 
		 C:\logog\test\test.cpp:484: info: This info message output has trailing periods ...
		 !!! C:\logog\test\test.cpp:485: warning: This warning message is surrounded by exclamation marks !!!

		 */

        Cout out;
        FormatterModified customFormat;

        out.SetFormatter( customFormat );

        INFO( _LG( "This info message output has trailing periods") );
        WARN( _LG( "This warning message is surrounded by exclamation marks") );
    }

    LOGOG_SHUTDOWN();

    return 0;
}
//! [FormatterModified]

UNITTEST( HelloLogog )
{
    //! [HelloLogog]

    /* The LOGOG_INITIALIZE() function must be called before we call
     * any other logog functions.
     */
    LOGOG_INITIALIZE();

    {
        /* In order to see any output, we have to instance a Target object,
         * such as a Cerr or a Cout.  Additionally, we have to destroy
         * this object before calling LOGOG_SHUTDOWN().  This
         * is why we have this object within these enclosing brackets.
         */
        Cout out;

        /* Send some debugging information to any targets that have
         * been instanced.
         */
        /* If you're just getting started, and you haven't defined
         * LOGOG_UNICODE, then ASCII logging is easy and works
         * out of the box.
         */
#ifndef LOGOG_UNICODE
        INFO("Hello, logog!");
        WARN("This is a warning");
#endif // !LOGOG_UNICODE

        /* The _LG() macro around static text permits the given text to
         * display correctly in both Unicode and ASCII builds of the
         * logog library.
         */
        ERR( _LG( "This is an error") );
        DBUG( _LG( "This is debugging info") );

        /* The Cout object is destroyed here because it falls out of
         * scope. */
    }

    /* Call LOGOG_SHUTDOWN() at the termination of your program to free
     * all memory allocated by logog.  Make sure no logog objects exist
     * when you call LOGOG_SHUTDOWN().
     */
    LOGOG_SHUTDOWN();

    /* Depending on your compiler, the output of the preceding code is
     * something like:
     *
     * test.cpp(373) : info: Hello, logog!
     * test.cpp(374) : warning: This is a warning
     * test.cpp(375) : error: This is an error
     * test.cpp(376) : debug: This is debugging info
     */
    //! [HelloLogog]
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

        EMERGENCY(_LG("The graphics card has been destroyed"));

#undef LOGOG_CATEGORY
#define LOGOG_CATEGORY	"Recoverable"

        WARN(_LG("The graphics card has been replaced"));

#undef LOGOG_GROUP
#define LOGOG_GROUP "Audio"

        WARN(_LG("The headphones are unplugged"));

#undef LOGOG_CATEGORY
#undef LOGOG_GROUP
#define LOGOG_CATEGORY NULL
#define LOGOG_GROUP NULL

        INFO(_LG("Everything's back to... %s!"), _LG("normal"));
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
        GetDefaultFilter().Category(_LG("Unrecoverable"));
        Cerr err;

        WARN(_LG("Logging messages in the Unrecoverable category..."));


#undef LOGOG_GROUP
#undef LOGOG_CATEGORY
#define LOGOG_GROUP  "Graphics"
#define LOGOG_CATEGORY "Unrecoverable"

        EMERGENCY(_LG("The graphics card has been destroyed"));

#undef LOGOG_CATEGORY
#define LOGOG_CATEGORY	"Recoverable"

        WARN(_LG("The graphics card has been replaced"));

#undef LOGOG_GROUP
#define LOGOG_GROUP "Audio"

        WARN(_LG("The headphones are unplugged"));

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
		/* Assigning a group twice does not leak memory. */
		GetDefaultFilter().Group( _LG( "Controller" ));
        GetDefaultFilter().Group( _LG( "Graphics" ));

        Cerr err;
        WARN(_LG("This message won't happen because it's not in the Graphics group"));

#undef LOGOG_GROUP
#undef LOGOG_CATEGORY
#define LOGOG_GROUP  "Graphics"
#define LOGOG_CATEGORY "Unrecoverable"

        EMERGENCY(_LG("The graphics card has been destroyed"));

#undef LOGOG_CATEGORY
#define LOGOG_CATEGORY	"Recoverable"

        WARN(_LG("The graphics card has been replaced"));

#undef LOGOG_GROUP
#define LOGOG_GROUP "Audio"

        WARN(_LG("The headphones are unplugged"));

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
        GetDefaultFilter().Group(_LG("Graphics"));
        Filter filter;
        filter.Group(_LG("Audio"));
        Cerr err;

        WARN(_LG("This message won't happen because it's not in the Graphics group"));

#undef LOGOG_GROUP
#undef LOGOG_CATEGORY
#define LOGOG_GROUP  "Graphics"
#define LOGOG_CATEGORY "Unrecoverable"

        EMERGENCY(_LG("The graphics card has been destroyed"));

#undef LOGOG_CATEGORY
#define LOGOG_CATEGORY	"Recoverable"

        WARN(_LG("The graphics card has been replaced"));

#undef LOGOG_GROUP
#define LOGOG_GROUP "Audio"

        WARN(_LG("The headphones are unplugged"));

#undef LOGOG_GROUP
#define LOGOG_GROUP "Inputs"

        WARN(_LG("The inputs have been yanked off but this fact won't be reported!"));

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
                INFO( _LG("t is now: %d"), t );
                if ( t > 8 )
                    WARN(_LG("t is pretty high now!"));
            }

            DBUG(_LG("This warning isn't very interesting"));
            EMERGENCY(_LG("THIS IS SUPER IMPORTANT!"));
        }

        LOGOG_SET_LEVEL( LOGOG_LEVEL_DEBUG );
        LOGOG_DEBUG(_LG("Messages instantiated for the first time will be called now.  However, debug messages"));
        LOGOG_DEBUG(_LG("instantiated before the LOGOG_SET_LEVEL won't be transmitted."));

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

        ERR( _LG("We must inform you of this pseudo-random error code: %d"), ps );

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

#ifdef LOGOG_UNICODE
UNITTEST( UnicodeLogFile )
{
    LOGOG_INITIALIZE();

    {
        LogFile logFile( "unicode.txt" );

        // see http://blogs.msdn.com/b/michkap/archive/2008/03/18/8306597.aspx
        INFO(L"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd");
        WARN(L"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd");
        ERR(L"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd");
    }

    LOGOG_SHUTDOWN();

    return 0;
}
#endif // LOGOG_UNICODE

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
            ERR(_LG("This is error %d of 10"), i);

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
        LogFile logFile( "log.txt" );
        LogBuffer logBuffer( &logFile );

        /* Because the LogBuffer is not writing to a line device a la stdout or stderr, it does not need to
         * send null terminated strings to its destination (the LogFile).  This is a particular peculiarity
         * of having a buffering target to a serial-type target, such as a socket or a file.
         */
        logBuffer.SetNullTerminatesStrings( false );

        // Make sure that the log file does not receive messages via the general filter mechanism.
        logFile.UnsubscribeToMultiple( AllFilters() );

        for ( int i = 1; i <= 20; i++ )
        {
            WARN(_LG("This is warning %d of 20"), i);

            int q = 27832;
            for ( int j = 0; j < TEST_STRESS_LEVEL * 10000000; j++ )
                q *= q;
        }
    }

    LOGOG_SHUTDOWN();
//! [DeferredFileLogging]
    return 0;
}

UNITTEST( DateAndTime )
{
//! [DateAndTimeLogging]
    LOGOG_INITIALIZE();

    {
        Cerr err;

        Formatter *pFormatter = &GetDefaultFormatter();

        pFormatter->SetShowTimeOfDay( true );

        for ( int i = 1; i <= 20; i++ )
        {
            WARN(_LG("This is warning %d of 20... but with time!"), i);

            int q = 27832;
            for ( int j = 0; j < TEST_STRESS_LEVEL * 10000000; j++ )
                q *= q;
        }
    }

    LOGOG_SHUTDOWN();
//! [DateAndTimeLogging]

    return 0;
}

UNITTEST ( InitializingStatements )
{
    for ( int i = 1; i <= 2; i++ )
        if ( i == 1 )
            LOGOG_INITIALIZE();
        else if (i == 2)
            LOGOG_SHUTDOWN();
    return 0;
}

UNITTEST ( LoggingInsideIfThenStatement )
{
    LOGOG_INITIALIZE();
    {
        Cerr err;
        for ( int i = 1; i <= 11; i++ )
            if ( i == 1 )
                EMERGENCY(_LG("This message is EMERGENCY (%d)"), i);
            else if ( i == 2 )
                ALERT(_LG("This message is ALERT (%d)"), i);
            else if ( i == 3 )
                CRITICAL(_LG("This message is CRITICAL (%d)"), i);
            else if ( i == 4 )
                ERR(_LG("This message is ERROR (%d)"), i);
            else if ( i == 5 )
                WARN(_LG("This message is WARN (%d)"), i);
            else if ( i == 6 )
                WARN1(_LG("This message is WARN1 (%d)"), i);
            else if ( i == 7 )
                WARN2(_LG("This message is WARN2 (%d)"), i);
            else if ( i == 8 )
                WARN3(_LG("This message is WARN3 (%d)"), i);
            else if ( i == 9 )
                INFO(_LG("This message is INFO (%d)"), i);
            else if ( i == 10 )
                DBUG(_LG("This message is DEBUG (%d)"), i);
    }
    LOGOG_SHUTDOWN();
    return 0;
}


UNITTEST ( ChangeLogLevelAtRuntime )
{
	LOGOG_INITIALIZE();
	{
		logog::Cout out;

        logog::SetDefaultLevel(LOGOG_LEVEL_NONE);

        EMERGENCY(_LG("NOT SHOWN at level LOGOG_LEVEL_NONE"));
        ALERT(_LG("NOT SHOWN at level LOGOG_LEVEL_NONE"));
        CRITICAL(_LG("NOT SHOWN at level LOGOG_LEVEL_NONE"));
        ERR(_LG("NOT SHOWN at level LOGOG_LEVEL_NONE"));
        WARN(_LG("NOT SHOWN at level LOGOG_LEVEL_NONE"));
        WARN1(_LG("NOT SHOWN at level LOGOG_LEVEL_NONE"));
        WARN2(_LG("NOT SHOWN at level LOGOG_LEVEL_NONE"));
        WARN3(_LG("NOT SHOWN at level LOGOG_LEVEL_NONE"));
        INFO(_LG("NOT SHOWN at level LOGOG_LEVEL_NONE"));
        DBUG(_LG("NOT SHOWN at level LOGOG_LEVEL_NONE"));


        logog::SetDefaultLevel(LOGOG_LEVEL_EMERGENCY);

        EMERGENCY(_LG("Shown at level LOGOG_LEVEL_EMERGENCY"));
        ALERT(_LG("NOT SHOWN at level LOGOG_LEVEL_EMERGENCY"));
        CRITICAL(_LG("NOT SHOWN at level LOGOG_LEVEL_EMERGENCY"));
        ERR(_LG("NOT SHOWN at level LOGOG_LEVEL_EMERGENCY"));
        WARN(_LG("NOT SHOWN at level LOGOG_LEVEL_EMERGENCY"));
        WARN1(_LG("NOT SHOWN at level LOGOG_LEVEL_EMERGENCY"));
        WARN2(_LG("NOT SHOWN at level LOGOG_LEVEL_EMERGENCY"));
        WARN3(_LG("NOT SHOWN at level LOGOG_LEVEL_EMERGENCY"));
        INFO(_LG("NOT SHOWN at level LOGOG_LEVEL_EMERGENCY"));
        DBUG(_LG("NOT SHOWN at level LOGOG_LEVEL_EMERGENCY"));


        logog::SetDefaultLevel(LOGOG_LEVEL_ALERT);

        EMERGENCY(_LG("Shown at level LOGOG_LEVEL_ALERT"));
        ALERT(_LG("Shown at level LOGOG_LEVEL_ALERT"));
        CRITICAL(_LG("NOT SHOWN at level LOGOG_LEVEL_ALERT"));
        ERR(_LG("NOT SHOWN at level LOGOG_LEVEL_ALERT"));
        WARN(_LG("NOT SHOWN at level LOGOG_LEVEL_ALERT"));
        WARN1(_LG("NOT SHOWN at level LOGOG_LEVEL_ALERT"));
        WARN2(_LG("NOT SHOWN at level LOGOG_LEVEL_ALERT"));
        WARN3(_LG("NOT SHOWN at level LOGOG_LEVEL_ALERT"));
        INFO(_LG("NOT SHOWN at level LOGOG_LEVEL_ALERT"));
        DBUG(_LG("NOT SHOWN at level LOGOG_LEVEL_ALERT"));


        logog::SetDefaultLevel(LOGOG_LEVEL_CRITICAL);

        EMERGENCY(_LG("Shown at level LOGOG_LEVEL_CRITICAL"));
        ALERT(_LG("Shown at level LOGOG_LEVEL_CRITICAL"));
        CRITICAL(_LG("Shown at level LOGOG_LEVEL_CRITICAL"));
        ERR(_LG("NOT SHOWN at level LOGOG_LEVEL_CRITICAL"));
        WARN(_LG("NOT SHOWN at level LOGOG_LEVEL_CRITICAL"));
        WARN1(_LG("NOT SHOWN at level LOGOG_LEVEL_CRITICAL"));
        WARN2(_LG("NOT SHOWN at level LOGOG_LEVEL_CRITICAL"));
        WARN3(_LG("NOT SHOWN at level LOGOG_LEVEL_CRITICAL"));
        INFO(_LG("NOT SHOWN at level LOGOG_LEVEL_CRITICAL"));
        DBUG(_LG("NOT SHOWN at level LOGOG_LEVEL_CRITICAL"));


        logog::SetDefaultLevel(LOGOG_LEVEL_ERROR);

        EMERGENCY(_LG("Shown at level LOGOG_LEVEL_ERROR"));
        ALERT(_LG("Shown at level LOGOG_LEVEL_ERROR"));
        CRITICAL(_LG("Shown at level LOGOG_LEVEL_ERROR"));
        ERR(_LG("Shown at level LOGOG_LEVEL_ERROR"));
        WARN(_LG("NOT SHOWN at level LOGOG_LEVEL_ERROR"));
        WARN1(_LG("NOT SHOWN at level LOGOG_LEVEL_ERROR"));
        WARN2(_LG("NOT SHOWN at level LOGOG_LEVEL_ERROR"));
        WARN3(_LG("NOT SHOWN at level LOGOG_LEVEL_ERROR"));
        INFO(_LG("NOT SHOWN at level LOGOG_LEVEL_ERROR"));
        DBUG(_LG("NOT SHOWN at level LOGOG_LEVEL_ERROR"));


        logog::SetDefaultLevel(LOGOG_LEVEL_WARN);

        EMERGENCY(_LG("Shown at level LOGOG_LEVEL_WARN"));
        ALERT(_LG("Shown at level LOGOG_LEVEL_WARN"));
        CRITICAL(_LG("Shown at level LOGOG_LEVEL_WARN"));
        ERR(_LG("Shown at level LOGOG_LEVEL_WARN"));
        WARN(_LG("Shown at level LOGOG_LEVEL_WARN"));
        WARN1(_LG("NOT SHOWN at level LOGOG_LEVEL_WARN"));
        WARN2(_LG("NOT SHOWN at level LOGOG_LEVEL_WARN"));
        WARN3(_LG("NOT SHOWN at level LOGOG_LEVEL_WARN"));
        INFO(_LG("NOT SHOWN at level LOGOG_LEVEL_WARN"));
        DBUG(_LG("NOT SHOWN at level LOGOG_LEVEL_WARN"));


        logog::SetDefaultLevel(LOGOG_LEVEL_WARN1);

        EMERGENCY(_LG("Shown at level LOGOG_LEVEL_WARN1"));
        ALERT(_LG("Shown at level LOGOG_LEVEL_WARN1"));
        CRITICAL(_LG("Shown at level LOGOG_LEVEL_WARN1"));
        ERR(_LG("Shown at level LOGOG_LEVEL_WARN1"));
        WARN(_LG("Shown at level LOGOG_LEVEL_WARN1"));
        WARN1(_LG("Shown at level LOGOG_LEVEL_WARN1"));
        WARN2(_LG("NOT SHOWN at level LOGOG_LEVEL_WARN1"));
        WARN3(_LG("NOT SHOWN at level LOGOG_LEVEL_WARN1"));
        INFO(_LG("NOT SHOWN at level LOGOG_LEVEL_WARN1"));
        DBUG(_LG("NOT SHOWN at level LOGOG_LEVEL_WARN1"));


        logog::SetDefaultLevel(LOGOG_LEVEL_WARN2);

        EMERGENCY(_LG("Shown at level LOGOG_LEVEL_WARN2"));
        ALERT(_LG("Shown at level LOGOG_LEVEL_WARN2"));
        CRITICAL(_LG("Shown at level LOGOG_LEVEL_WARN2"));
        ERR(_LG("Shown at level LOGOG_LEVEL_WARN2"));
        WARN(_LG("Shown at level LOGOG_LEVEL_WARN2"));
        WARN1(_LG("Shown at level LOGOG_LEVEL_WARN2"));
        WARN2(_LG("Shown at level LOGOG_LEVEL_WARN2"));
        WARN3(_LG("NOT SHOWN at level LOGOG_LEVEL_WARN2"));
        INFO(_LG("NOT SHOWN at level LOGOG_LEVEL_WARN2"));
        DBUG(_LG("NOT SHOWN at level LOGOG_LEVEL_WARN2"));


        logog::SetDefaultLevel(LOGOG_LEVEL_WARN3);

        EMERGENCY(_LG("Shown at level LOGOG_LEVEL_WARN3"));
        ALERT(_LG("Shown at level LOGOG_LEVEL_WARN3"));
        CRITICAL(_LG("Shown at level LOGOG_LEVEL_WARN3"));
        ERR(_LG("Shown at level LOGOG_LEVEL_WARN3"));
        WARN(_LG("Shown at level LOGOG_LEVEL_WARN3"));
        WARN1(_LG("Shown at level LOGOG_LEVEL_WARN3"));
        WARN2(_LG("Shown at level LOGOG_LEVEL_WARN3"));
        WARN3(_LG("Shown at level LOGOG_LEVEL_WARN3"));
        INFO(_LG("NOT SHOWN at level LOGOG_LEVEL_WARN3"));
        DBUG(_LG("NOT SHOWN at level LOGOG_LEVEL_WARN3"));


        logog::SetDefaultLevel(LOGOG_LEVEL_INFO);

        EMERGENCY(_LG("Shown at level LOGOG_LEVEL_INFO"));
        ALERT(_LG("Shown at level LOGOG_LEVEL_INFO"));
        CRITICAL(_LG("Shown at level LOGOG_LEVEL_INFO"));
        ERR(_LG("Shown at level LOGOG_LEVEL_INFO"));
        WARN(_LG("Shown at level LOGOG_LEVEL_INFO"));
        WARN1(_LG("Shown at level LOGOG_LEVEL_INFO"));
        WARN2(_LG("Shown at level LOGOG_LEVEL_INFO"));
        WARN3(_LG("Shown at level LOGOG_LEVEL_INFO"));
        INFO(_LG("Shown at level LOGOG_LEVEL_INFO"));
        DBUG(_LG("NOT SHOWN at level LOGOG_LEVEL_INFO"));


        logog::SetDefaultLevel(LOGOG_LEVEL_DEBUG);

        EMERGENCY(_LG("Shown at level LOGOG_LEVEL_DEBUG"));
        ALERT(_LG("Shown at level LOGOG_LEVEL_DEBUG"));
        CRITICAL(_LG("Shown at level LOGOG_LEVEL_DEBUG"));
        ERR(_LG("Shown at level LOGOG_LEVEL_DEBUG"));
        WARN(_LG("Shown at level LOGOG_LEVEL_DEBUG"));
        WARN1(_LG("Shown at level LOGOG_LEVEL_DEBUG"));
        WARN2(_LG("Shown at level LOGOG_LEVEL_DEBUG"));
        WARN3(_LG("Shown at level LOGOG_LEVEL_DEBUG"));
        INFO(_LG("Shown at level LOGOG_LEVEL_DEBUG"));
        DBUG(_LG("Shown at level LOGOG_LEVEL_DEBUG"));


        logog::SetDefaultLevel(LOGOG_LEVEL_ALL);

        EMERGENCY(_LG("Shown at level LOGOG_LEVEL_ALL"));
        ALERT(_LG("Shown at level LOGOG_LEVEL_ALL"));
        CRITICAL(_LG("Shown at level LOGOG_LEVEL_ALL"));
        ERR(_LG("Shown at level LOGOG_LEVEL_ALL"));
        WARN(_LG("Shown at level LOGOG_LEVEL_ALL"));
        WARN1(_LG("Shown at level LOGOG_LEVEL_ALL"));
        WARN2(_LG("Shown at level LOGOG_LEVEL_ALL"));
        WARN3(_LG("Shown at level LOGOG_LEVEL_ALL"));
        INFO(_LG("Shown at level LOGOG_LEVEL_ALL"));
        DBUG(_LG("Shown at level LOGOG_LEVEL_ALL"));

    }
    LOGOG_SHUTDOWN();
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
#ifdef LOGOG_LEAK_DETECTION_WINDOWS
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF );
#endif // LOGOG_LEAK_DETECTION_WINDOWS

//! [WindowsUnicodeSetup]
#ifdef LOGOG_UNICODE
#ifdef LOGOG_FLAVOR_WINDOWS
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
#endif // LOGOG_FLAVOR_WINDOWS
#endif // LOGOG_UNICODE
//! [WindowsUnicodeSetup]

    int nPlatformSpecific;
    nPlatformSpecific = DoPlatformSpecificTestInitialization();
    if ( nPlatformSpecific != 0)
        return nPlatformSpecific;

    int nResult;
    nResult = RunAllTests();
    ShutdownTests();

#ifdef LOGOG_LEAK_DETECTION_WINDOWS
	_CrtMemState crtMemState;
	_CrtMemCheckpoint( &crtMemState );
	_CrtMemDumpStatistics( &crtMemState );
	_CrtDumpMemoryLeaks();
#endif // LOGOG_LEAK_DETECTION_WINDOWS

    return nResult;
}

