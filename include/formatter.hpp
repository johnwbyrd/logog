/**
 * \file formatter.hpp Formats a topic into human-readable, compiler lookalike format.
 */

#ifndef __LOGOG_FORMATTER_HPP__
#define __LOGOG_FORMATTER_HPP__

namespace logog
{

#define LOGOG_TIME_STRING_MAX 80

/** A helper object for generating a current timestamp as a string. */
class TimeStamp : public Object
{
public:
	/** Returns a pointer to a string representing the current time.  Note!  This pointer is only
	 ** valid while this object is valid -- if you destroy this object, this pointer is no longer
	 ** valid.
	 */
	const char *Get();

protected:
	char cTimeString[ LOGOG_TIME_STRING_MAX ]; 
};

/** Converts a topic into a human-readable string for printing or otherwise rendering to a target. */
class Formatter : public Object
{
public:
	Formatter();

    /** Causes this formatter to format a topic into its own m_sMessageBuffer field, and thence to
     ** return a reference to that string.  This function must be written to be efficient; it will be called
     ** for every logging operation.  It is strongly recommended not to allocate or free memory in this function.
     **/
    virtual LOGOG_STRING &Format( const Topic &topic, const Target &target ) = 0;

	/** Causes the time of day to be rendered, if it needs to be rendered.  This function is only supported on
	 ** ANSI builds, not Unicode, as the underlying functions are ANSI only.
	 */
	virtual void RenderTimeOfDay();

	/** Should this formatter render the current time of day? */
	bool GetShowTimeOfDay() const;

	/** Sets whether this formatter renders the current time of day. */
	void SetShowTimeOfDay(bool val);

protected:
    const LOGOG_CHAR *ErrorDescription( const LOGOG_LEVEL_TYPE level );

    LOGOG_STRING m_sMessageBuffer;
    LOGOG_STRING m_sIntBuffer;

	bool m_bShowTimeOfDay;
};

class FormatterGCC : public Formatter
{
public:
    virtual LOGOG_STRING &Format( const Topic &topic, const Target &target );

};

class FormatterMSVC : public Formatter
{
public:
    virtual LOGOG_STRING &Format( const Topic &topic, const Target &target );
};

extern Formatter &GetDefaultFormatter();
extern void DestroyDefaultFormatter();

}

#endif // __LOGOG_FORMATTER_HPP_
