 
/* 
 * \file formatter.cpp
 */

#include "logog.hpp"

namespace logog {

	Formatter::Formatter()
	{
		m_sMessageBuffer.reserve( LOGOG_FORMATTER_MAX_LENGTH );
		m_sIntBuffer.reserve_for_int();
	}

	const LOGOG_CHAR * Formatter::ErrorDescription( const LOGOG_LEVEL_TYPE level )
	{
		if ( level <= LOGOG_LEVEL_NONE )
			return "none";

		if ( level <= LOGOG_LEVEL_EMERGENCY )
			return "emergency";

		if ( level <= LOGOG_LEVEL_ALERT )
			return "alert";

		if ( level <= LOGOG_LEVEL_CRITICAL )
			return "critical";

		if ( level <= LOGOG_LEVEL_ERROR )
			return "error";

		if ( level <= LOGOG_LEVEL_WARN )
			return "warning";

		if ( level <= LOGOG_LEVEL_INFO )
			return "info";

		if ( level <= LOGOG_LEVEL_DEBUG )
			return "debug";

		return "unknown";
	}

	LOGOG_STRING &FormatterGCC::Format( const Topic &topic, const Target &target )
	{
		TOPIC_FLAGS flags;
		flags = topic.GetTopicFlags();

		m_sMessageBuffer.clear();

		if ( flags & TOPIC_FILE_NAME_FLAG )
		{
			m_sMessageBuffer.append( topic.FileName() );
			m_sMessageBuffer.append( ':' );
		}

		if ( flags & TOPIC_LINE_NUMBER_FLAG )
		{
			m_sIntBuffer.assign( topic.LineNumber() );
			m_sMessageBuffer.append( m_sIntBuffer );

			m_sMessageBuffer.append( ": ");
		}

		if ( flags & TOPIC_LEVEL_FLAG )
		{
			m_sMessageBuffer.append( ErrorDescription( topic.Level()));
			m_sMessageBuffer.append( ": ");
		}

		if ( flags & TOPIC_GROUP_FLAG )
		{
			m_sMessageBuffer.append( "{");
			m_sMessageBuffer.append( topic.Group() );
			m_sMessageBuffer.append( "} ");
		}

		if ( flags & TOPIC_CATEGORY_FLAG )
		{
			m_sMessageBuffer.append( "[");
			m_sMessageBuffer.append( topic.Category() );
			m_sMessageBuffer.append( "] ");
		}

		if ( flags & TOPIC_MESSAGE_FLAG )
		{
			m_sMessageBuffer.append( topic.Message() );
			m_sMessageBuffer.append( '\n' );
		}

		if ( target.GetNullTerminatesStrings() )
			m_sMessageBuffer.append( (char)NULL );

		return m_sMessageBuffer;
	}

	LOGOG_STRING &FormatterMSVC::Format( const Topic &topic, const Target &target )
    {
        m_sMessageBuffer.clear();

        TOPIC_FLAGS flags;
        flags = topic.GetTopicFlags();

        if ( flags & TOPIC_FILE_NAME_FLAG )
        {
            m_sMessageBuffer.append( topic.FileName() );
            m_sMessageBuffer.append( '(' );
        }

        if ( flags & TOPIC_LINE_NUMBER_FLAG  )
        {
            m_sIntBuffer.assign( topic.LineNumber() );
            m_sMessageBuffer.append( m_sIntBuffer );

            m_sMessageBuffer.append( ") : " );
        }

        if ( flags & TOPIC_LEVEL_FLAG )
        {
            m_sMessageBuffer.append( ErrorDescription( topic.Level() ) );
            m_sMessageBuffer.append(": ");
        }

        if ( flags & TOPIC_GROUP_FLAG )
        {
            m_sMessageBuffer.append( "{");
            m_sMessageBuffer.append( topic.Group() );
            m_sMessageBuffer.append( "} ");
        }

        if ( flags & TOPIC_CATEGORY_FLAG )
        {
            m_sMessageBuffer.append( "[");
            m_sMessageBuffer.append( topic.Category() );
            m_sMessageBuffer.append( "] ");
        }

        if ( flags & TOPIC_MESSAGE_FLAG )
        {
            m_sMessageBuffer.append( topic.Message() );
            m_sMessageBuffer.append( '\n' );
        }

		if ( target.GetNullTerminatesStrings() )
			m_sMessageBuffer.append( (char)NULL );

        return m_sMessageBuffer;
    }

	Formatter &GetDefaultFormatter()
	{
		Statics *pStatic = &Static();

		if ( pStatic->s_pDefaultFormatter == NULL )
		{
#ifdef LOGOG_FLAVOR_WINDOWS
			pStatic->s_pDefaultFormatter = new FormatterMSVC();
#else
			pStatic->s_pDefaultFormatter = new FormatterGCC();
#endif
		}

		return *( pStatic->s_pDefaultFormatter );
	}

	void DestroyDefaultFormatter()
	{
		Statics *pStatic = &Static();
		Formatter *pDefaultFormatter = pStatic->s_pDefaultFormatter;

		if ( pDefaultFormatter != NULL )
			delete pDefaultFormatter;

		pStatic->s_pDefaultFormatter = NULL;
}


}

