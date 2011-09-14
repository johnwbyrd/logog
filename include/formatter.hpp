/**
 * \file formatter.hpp Formats a topic into human-readable, compiler lookalike format.
 */

#ifndef __LOGOG_FORMATTER_HPP__
#define __LOGOG_FORMATTER_HPP__

namespace logog
{
class Formatter : public Object
{
public:
    Formatter();

    /** Causes this formatter to format a topic into its own m_sMessageBuffer field, and thence to
     ** return a reference to that string.  This function must be written to be efficient; it will be called
     ** for every logging operation.  It is strongly recommended not to allocate or free memory in this function.
     **/
    virtual LOGOG_STRING &Format( const Topic &topic ) = 0;

protected:
    const LOGOG_CHAR *ErrorDescription( const LOGOG_LEVEL_TYPE level );

    LOGOG_STRING m_sMessageBuffer;
    LOGOG_STRING m_sIntBuffer;
};

class FormatterGCC : public Formatter
{
public:
    virtual LOGOG_STRING &Format( const Topic &topic );
};

class FormatterMSVC : public Formatter
{
public:
    virtual LOGOG_STRING &Format( const Topic &topic );
};

extern Formatter &GetDefaultFormatter();
extern void DestroyDefaultFormatter();

};

#endif // __LOGOG_FORMATTER_HPP_
