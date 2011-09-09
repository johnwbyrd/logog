/**
 * \file macro.hpp Macros for instantiation of a message.
 */

#ifndef __LOGOG_MACRO_HPP__
#define __LOGOG_MACRO_HPP__

namespace logog {

#ifdef LOGOG_USE_PREFIX
#define LOGOG_PREFIX LOGOG_
#endif // LOGOG_USE_PREFIX

#ifndef LOGOG_GROUP
#define LOGOG_GROUP NULL
#endif

#ifndef LOGOG_CATEGORY
#define LOGOG_CATEGORY NULL
#endif

/*
 * When you have a macro replacement, the preprocessor will only expand the macros recursively
 * if neither the stringizing operator # nor the token-pasting operator ## are applied to it.
 * So, you have to use some extra layers of indirection, you can use the token-pasting operator
 * with a recursively expanded argument.
 */
#define TOKENPASTE2(x, y) x ## y
#define TOKENPASTE(x, y) TOKENPASTE2(x, y)

#define LOGOG_LEVEL_GROUP_CATEGORY_MESSAGE_NO_VA( level, group, cat, msg ) \
{ \
	Mutex *___pMCM = &GetMessageCreationMutex(); \
	___pMCM->Lock(); \
	static logog::Message *TOKENPASTE(_logog_,__LINE__) = new logog::Message( level, \
		__FILE__, __LINE__, group, cat, msg ); \
	___pMCM->Unlock(); \
	TOKENPASTE(_logog_,__LINE__)->m_Transmitting.Lock(); \
	TOKENPASTE(_logog_,__LINE__)->Transmit(); \
	TOKENPASTE(_logog_,__LINE__)->m_Transmitting.Unlock(); \
}

#define LOGOG_LEVEL_GROUP_CATEGORY_MESSAGE( level, group, cat, formatstring, ... ) \
{ \
	Mutex *___pMCM = &GetMessageCreationMutex(); \
	___pMCM->Lock(); \
	static logog::Message * TOKENPASTE(_logog_,__LINE__) = new logog::Message( level, \
	__FILE__, __LINE__, group, cat ); \
	___pMCM->Unlock(); \
	TOKENPASTE(_logog_,__LINE__)->m_Transmitting.Lock(); \
	TOKENPASTE(_logog_,__LINE__)->Format( formatstring, ##__VA_ARGS__ ); \
	TOKENPASTE(_logog_,__LINE__)->Transmit(); \
	TOKENPASTE(_logog_,__LINE__)->m_Transmitting.Unlock(); \
}

#define LOGOG_LEVEL_MESSAGE( level, formatstring, ... ) \
	LOGOG_LEVEL_GROUP_CATEGORY_MESSAGE( level, LOGOG_GROUP, LOGOG_CATEGORY, formatstring, ##__VA_ARGS__ )

#define LOGOG_MESSAGE( formatstring, ... ) \
	LOGOG_LEVEL_MESSAGE( LOGOG_LEVEL, formatstring, ##__VA_ARGS__ )


#if LOGOG_LEVEL >= LOGOG_LEVEL_DEBUG
#define LOGOG_DEBUG( formatstring, ... ) \
	LOGOG_LEVEL_MESSAGE( LOGOG_LEVEL_DEBUG, formatstring, ##__VA_ARGS__ )
#else 
#define LOGOG_DEBUG( formatstring, ... ) {};
#endif

#if LOGOG_LEVEL >= LOGOG_LEVEL_INFO
#define LOGOG_INFO( formatstring, ... ) \
	LOGOG_LEVEL_MESSAGE( LOGOG_LEVEL_INFO, formatstring, ##__VA_ARGS__ )
#else
#define LOGOG_INFO( formatstring, ... ) {};
#endif

#if LOGOG_LEVEL	>= LOGOG_LEVEL_WARN3
#define LOGOG_WARN3( formatstring, ... ) \
	LOGOG_LEVEL_MESSAGE( LOGOG_LEVEL_WARN3, formatstring, ##__VA_ARGS__ )
#else
#define LOGOG_WARN3( formatstring, ... ) {};
#endif

#if LOGOG_LEVEL	>= LOGOG_LEVEL_WARN2
#define LOGOG_WARN2( formatstring, ... ) \
	LOGOG_LEVEL_MESSAGE( LOGOG_LEVEL_WARN2, formatstring, ##__VA_ARGS__ )
#else
#define LOGOG_WARN2( formatstring, ... ) {};
#endif

#if LOGOG_LEVEL	>= LOGOG_LEVEL_WARN1
#define LOGOG_WARN1( formatstring, ... ) \
	LOGOG_LEVEL_MESSAGE( LOGOG_LEVEL_WARN1, formatstring, ##__VA_ARGS__ )
#else
#define LOGOG_WARN1( formatstring, ... ) {};
#endif

#if LOGOG_LEVEL	>= LOGOG_LEVEL_WARN
#define LOGOG_WARN( formatstring, ... ) \
	LOGOG_LEVEL_MESSAGE( LOGOG_LEVEL_WARN, formatstring, ##__VA_ARGS__ )
#else
#define LOGOG_WARN( formatstring, ... ) {};
#endif

#if LOGOG_LEVEL	>= LOGOG_LEVEL_ERROR
#define LOGOG_ERROR( formatstring, ... ) \
	LOGOG_LEVEL_MESSAGE( LOGOG_LEVEL_ERROR, formatstring, ##__VA_ARGS__ )
#else
#define LOGOG_ERROR( formatstring, ... ) {};
#endif

#if LOGOG_LEVEL	>= LOGOG_LEVEL_CRITICAL
#define LOGOG_CRITICAL( formatstring, ... ) \
	LOGOG_LEVEL_MESSAGE( LOGOG_LEVEL_CRITICAL, formatstring, ##__VA_ARGS__ )
#else
#define LOGOG_CRITICAL( formatstring, ... ) {};
#endif

#if LOGOG_LEVEL	>= LOGOG_LEVEL_ALERT
#define LOGOG_ALERT( formatstring, ... ) \
	LOGOG_LEVEL_MESSAGE( LOGOG_LEVEL_ALERT, formatstring, ##__VA_ARGS__ )
#else
#define LOGOG_CRITICAL( formatstring, ... ) {};
#endif

#if LOGOG_LEVEL	>= LOGOG_LEVEL_EMERGENCY
#define LOGOG_EMERGENCY( formatstring, ... ) \
	LOGOG_LEVEL_MESSAGE( LOGOG_LEVEL_EMERGENCY, formatstring, ##__VA_ARGS__ )
#else
#define LOGOG_EMERGENCY( formatstring, ... ) {};
#endif

#define LOGOG_SET_LEVEL( level ) \
	SetDefaultLevel( level );


#ifndef LOGOG_USE_PREFIX
/* If you get compilation errors in this section, then define the flag LOGOG_USE_PREFIX during compilation, and these 
 * shorthand logging macros won't exist -- you'll need to use the LOGOG_* equivalents above.
 */
/* We can't use DEBUG in Win32 unfortunately, so we use DBUG for shorthand here. */
#define DBUG(...) LOGOG_DEBUG( __VA_ARGS__ )
#define INFO(...) LOGOG_INFO( __VA_ARGS__ )
#define WARN3(...) LOGOG_WARN3( __VA_ARGS__ )
#define WARN2(...) LOGOG_WARN2( __VA_ARGS__ )
#define WARN1(...) LOGOG_WARN1( __VA_ARGS__ )
#define WARN(...) LOGOG_WARN( __VA_ARGS__ )
/** Same thing for error unfortunately */
#define ERR(...) LOGOG_ERROR( __VA_ARGS__ )
#define ALERT(...) LOGOG_ALERT( __VA_ARGS__ )
#define CRITICAL(...) LOGOG_CRITICAL( __VA_ARGS__ )
#define EMERGENCY(...) LOGOG_EMERGENCY( __VA_ARGS__ )
#endif

/** Call this function to initialize logog and prepare for logging. */
#define LOGOG_INITIALIZE(...)  logog::Initialize( __VA_ARGS__ );

/** Call this function to shut down logog and release all memory allocated. */
#define LOGOG_SHUTDOWN()   logog::Shutdown();

} // namespace logog

#endif // __LOGOG_MACRO_HPP_
