/**
 * \file string.hpp Defines the logog string class.
 */
#ifndef __LOGOG_STRING_HPP__
#define __LOGOG_STRING_HPP__

/** \def LOGOG_UNICODE Define this typedef in order to support wide characters in all logog strings. */
#ifdef LOGOG_UNICODE
/** logog has detected Unicode; therefore a LOGOG_CHAR is a wide character. */
typedef wchar_t LOGOG_CHAR;
#else // LOGOG_UNICODE
/** logog has not detected Unicode; therefore a LOGOG_CHAR is simply a char. */
typedef char LOGOG_CHAR;
#endif // LOGOG_UNICODE

#ifndef LOGOG_STRING_MAX_LENGTH
#define LOGOG_STRING_MAX_LENGTH 4096
#endif

namespace logog
{
class String : public Object
{
public:
    enum
    {
        npos = -1
    };

	String();
    virtual ~String();
    virtual void Free();
	static size_t Length( const LOGOG_CHAR *chars );

    String( const String &other );
    String( const LOGOG_CHAR *pstr );
    String & operator =( const String & other);
    String & operator =( const LOGOG_CHAR *pstr );
    size_t size() const;
    virtual void clear();
    virtual size_t reserve( size_t nSize );
    virtual size_t reserve_for_int();
    virtual operator const LOGOG_CHAR *() const;
    virtual size_t assign( const String &other );
    virtual size_t append( const String &other );
    virtual size_t append( const LOGOG_CHAR *other );
    virtual void reverse( LOGOG_CHAR* pStart, LOGOG_CHAR* pEnd);
    virtual size_t assign( const int value );
    virtual size_t append( const LOGOG_CHAR c );
    virtual bool is_valid() const;
    virtual size_t assign( const LOGOG_CHAR *other, const LOGOG_CHAR *pEnd = NULL );

    virtual size_t find( String &other ) const;
    virtual void format( const LOGOG_CHAR *cFormatString, ... );
    virtual void format_va( const LOGOG_CHAR *cFormatString, va_list args );

protected:
    virtual void Initialize();

    /* Code modified from http://www-igm.univ-mlv.fr/~lecroq/string/node8.html#SECTION0080 */
    void preKmp(size_t m);

    size_t KMP( const LOGOG_CHAR *y, size_t n );
    static const int ASIZE = 1 << ( sizeof( LOGOG_CHAR ) * 8 );

    // from http://www-igm.univ-mlv.fr/~lecroq/string/node14.html#SECTION00140
    void preBmBc(char *x, size_t m, size_t bmBc[]);
    void suffixes(char *x, size_t m, size_t *suff);
    void preBmGs(char *x, size_t m, size_t bmGs[]);

#define LOGOG_MAX( a, b ) ( ( a > b ) ? a : b )

	size_t BM(char *y, size_t n);

    LOGOG_CHAR *m_pBuffer;
    LOGOG_CHAR *m_pOffset;
    LOGOG_CHAR *m_pEndOfBuffer;
    size_t *m_pKMP;
    size_t *bmBc;
    size_t *bmGs;
    bool m_bIsConst;
};

}

#define LOGOG_STRING ::logog::String

#endif // __LOGOG_STRING_HPP_
