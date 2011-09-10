/**
 * \file formatter.hpp Formats a topic into human-readable, compiler lookalike format.
 */

#ifndef __LOGOG_FORMATTER_HPP__
#define __LOGOG_FORMATTER_HPP__

#ifndef LOGOG_FORMATTER_MAX_LENGTH
#define LOGOG_FORMATTER_MAX_LENGTH ( 1024 * 16 )
#endif

namespace logog
{
class Formatter : public Object
{
public:
    Formatter()
    {
        m_sMessageBuffer.reserve( LOGOG_FORMATTER_MAX_LENGTH );
        m_sIntBuffer.reserve_for_int();
    }

    /** Causes this formatter to format a topic into its own m_sMessageBuffer field, and thence to
     ** return a reference to that string.  This function must be written to be efficient; it will be called
     ** for every logging operation.  It is strongly recommended not to allocate or free memory in this function.
     **/
    virtual LOGOG_STRING &Format( const Topic &topic ) = 0;

protected:
    const LOGOG_CHAR *ErrorDescription( const LOGOG_LEVEL_TYPE level )
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

    LOGOG_STRING m_sMessageBuffer;
    LOGOG_STRING m_sIntBuffer;
};

class FormatterGCC : public Formatter
{
public:
    virtual LOGOG_STRING &Format( const Topic &topic )
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

        m_sMessageBuffer.append( (char)NULL );

        return m_sMessageBuffer;
    }
};

class FormatterMSVC : public Formatter
{
public:
    virtual LOGOG_STRING &Format( const Topic &topic )
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

        m_sMessageBuffer.append( (char)NULL );

        return m_sMessageBuffer;
    }
};

static Formatter &GetDefaultFormatter()
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

static void DestroyDefaultFormatter()
{
    Statics *pStatic = &Static();
    Formatter *pDefaultFormatter = pStatic->s_pDefaultFormatter;

    if ( pDefaultFormatter != NULL )
        delete pDefaultFormatter;

    pStatic->s_pDefaultFormatter = NULL;
}
}

#endif // __LOGOG_FORMATTER_HPP_
