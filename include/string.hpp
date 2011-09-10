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

    virtual ~String()
    {
        Free();
    }

    virtual void Free()
    {
        if ( m_pBuffer && ( m_bIsConst == false ))
        {
            Deallocate( (void *)m_pBuffer );
            m_pBuffer = NULL;
        }

        if ( m_pKMP )
        {
            Deallocate( (void *)m_pKMP );
            m_pKMP = NULL;
        }
    }

    static unsigned int Length( const LOGOG_CHAR *chars )
    {
        unsigned int len = 0;

        while ( *chars++ )
            len++;

        return len;
    }

    String()
    {
        Initialize();
    }

    String( const String &other )
    {
        Initialize();
        assign( other );
    }

    String( const LOGOG_CHAR *pstr )
    {
        Initialize();
        assign( pstr );
    }

    String & operator =( const String & other)
    {
        Initialize();
        assign( other );
        return *this;
    }

    String & operator =( const LOGOG_CHAR *pstr )
    {
        Initialize();
        assign( pstr );
        return *this;
    }

    virtual int size() const
    {
        return ( m_pOffset - m_pBuffer );
    }

    virtual void clear()
    {
        m_pOffset = m_pBuffer;
    }

    virtual int reserve( unsigned int nSize )
    {
        if ( nSize == (unsigned int)( m_pOffset - m_pBuffer ))
            return nSize;

        if ( nSize == 0 )
        {
            if ( *m_pBuffer != (LOGOG_CHAR)NULL )
                Deallocate( (void *)m_pBuffer );

            Initialize();
            return 0;
        }

        LOGOG_CHAR *pNewBuffer = (LOGOG_CHAR *)Allocate( sizeof( LOGOG_CHAR ) * nSize );
        LOGOG_CHAR *pNewOffset = pNewBuffer;
        LOGOG_CHAR *pNewEnd = pNewBuffer + nSize;

        LOGOG_CHAR *pOldOffset = m_pOffset;

        if ( pOldOffset != NULL )
        {
            while (( pNewOffset < pNewEnd ) && ( *pOldOffset != (LOGOG_CHAR)NULL ))
                *pNewOffset++ = *pOldOffset++;
        }

        if (( m_pBuffer != NULL ) && ( m_bIsConst == false ))
            Deallocate( m_pBuffer );

        m_pBuffer = pNewBuffer;
        m_pOffset = pNewBuffer;
        m_pEndOfBuffer = pNewEnd;

        return ( m_pOffset - m_pBuffer );
    }

    virtual int reserve_for_int()
    {
        return reserve( 32 );
    }

    virtual operator const LOGOG_CHAR *() const
    {
        return m_pBuffer;
    }

    virtual int assign( const String &other )
    {
#ifdef LOGOG_INTERNAL_DEBUGGING
        if ( m_bIsConst )
            cout << "Can't reassign const string!" << endl;
#endif

        m_pOffset = m_pBuffer;

        LOGOG_CHAR *pOther = other.m_pBuffer;

        if ( pOther == NULL )
            return 0;

        int othersize = other.size();

        reserve( othersize + 1 );

        for ( int t = 0; t <= othersize ; t++ )
            *m_pOffset++ = *pOther++;

        return this->size();
    }

    virtual int append( const String &other )
    {
#ifdef LOGOG_INTERNAL_DEBUGGING
        if ( m_bIsConst )
            cout << "Can't reassign const string!" << endl;
#endif

        return append( other.m_pBuffer );
    }

    virtual int append( const LOGOG_CHAR *other )
    {
#ifdef LOGOG_INTERNAL_DEBUGGING
        if ( m_bIsConst )
            cout << "Can't reassign const string!" << endl;
#endif

        while (( m_pOffset < m_pEndOfBuffer ) && ( *other != (LOGOG_CHAR)NULL ))
            *m_pOffset++ = *other++;

        return ( m_pOffset - m_pBuffer );
    }

    virtual void reverse( LOGOG_CHAR* pStart, LOGOG_CHAR* pEnd)
    {

        char temp;

        while( pEnd > pStart)
        {
            temp=*pEnd, *pEnd-- =*pStart, *pStart++=temp;
        }
    }

    virtual int assign( const int value )
    {

#ifdef LOGOG_INTERNAL_DEBUGGING
        if ( m_bIsConst )
            cout << "Can't reassign const string!" << endl;
#endif

        int number = value;
        m_pOffset = m_pBuffer;

        static LOGOG_CHAR s_cLookups[] = "0123456789";

        int bSign;

        if (( bSign = number) < 0)
            number = -number;

        do
        {
            *m_pOffset++ = s_cLookups[ number % 10 ];
        }
        while( number /= 10 );

        if (bSign < 0)
            *m_pOffset++ = '-';

        *m_pOffset = NULL;

        reverse( m_pBuffer, m_pOffset - 1 );

        return ( m_pOffset - m_pBuffer );
    }

    virtual int append( const LOGOG_CHAR c )
    {
        if ( m_pOffset < m_pEndOfBuffer )
            *m_pOffset++ = c;

        return ( m_pOffset - m_pBuffer );
    }

    virtual bool is_valid() const
    {
        return ( m_pBuffer != NULL );
    }

    virtual int assign( const LOGOG_CHAR *other, const LOGOG_CHAR *pEnd = NULL )
    {
        unsigned int len;

        if ( pEnd == NULL )
            len = Length( other );
        else
            len = ( pEnd - other );
        /** This constant decides whether assigning a LOGOG_CHAR * to a String will cause the String to use the previous buffer
          * in place, or create a new buffer and copy the results.
          */
#ifdef LOGOG_COPY_CONST_CHAR_ARRAY_ON_ASSIGNMENT
        reserve( len + 1 );

        for (unsigned int t = 0; t <= len; t++ )
            *m_pOffset++ = *other++;
#else  // LOGOG_COPY_CONST_CHAR_ARRAY_ON_ASSIGNMENT

#ifdef LOGOG_INTERNAL_DEBUGGING
        if ( m_bIsConst )
            cout << "Can't reassign const string!" << endl;
#endif
        /* In this case we don't copy the buffer, just reuse it */
        m_pBuffer = const_cast< LOGOG_CHAR *>( other );
        m_pOffset = m_pBuffer + len + 1;
        m_pEndOfBuffer = m_pOffset;
        m_bIsConst = true;

#endif // LOGOG_COPY_CONST_CHAR_ARRAY_ON_ASSIGNMENT

        return (int) len;
    }

    virtual int find( String &other ) const
    {
        if ( is_valid() && other.is_valid())
        {
            // BM solution
            // return other.BM( m_pBuffer, m_pOffset - m_pBuffer );
            // KMP solution
            return other.KMP( m_pBuffer, m_pOffset - m_pBuffer );
        }

        return npos;
    }

    virtual void format( const LOGOG_CHAR *cFormatString, ... )
    {
        va_list args;

        va_start(args, cFormatString);
        format_va( cFormatString, args );
        va_end( args );
    }

    virtual void format_va( const LOGOG_CHAR *cFormatString, va_list args )
    {
        int nActualSize = -1, nAttemptedSize;
        LOGOG_CHAR *pszFormatted = NULL;

        Free();

        /* Estimate length of output; don't pull in strlen() if we can help it */
        int nEstLength = 0;
        const LOGOG_CHAR *pCurChar = cFormatString;
        while ( *pCurChar++ )
            nEstLength++;

        nAttemptedSize = nEstLength * 2 * sizeof( LOGOG_CHAR );

        /* Some *printf implementations, such as msvc's, return -1 on failure.  Others, such as gcc, return the number
         * of characters actually formatted on failure.  Deal with either case here.
         */
        while ( nActualSize < nAttemptedSize )
        {
            pszFormatted = (LOGOG_CHAR *)Allocate( nAttemptedSize );
            if ( !pszFormatted )
            {
                LOGOG_INTERNAL_FAILURE;
            }

            *pszFormatted = NULL;

            va_list argsCopy;

#if defined( va_copy )
            va_copy( argsCopy, args );
#elif defined( __va_copy )
            __va_copy( argsCopy, args );
#else
            memcpy( &argsCopy, &args, sizeof(va_list) );
#endif

#ifdef LOGOG_FLAVOR_WINDOWS
            nActualSize = vsnprintf_s( pszFormatted, nAttemptedSize - 1, _TRUNCATE, cFormatString, argsCopy );
#else
            nActualSize = vsnprintf( pszFormatted, nAttemptedSize - 1, cFormatString, argsCopy );
#endif

            va_end( argsCopy );

            if (( nAttemptedSize > nActualSize ) && ( nActualSize != -1))
                break;

            // try again
            Deallocate( pszFormatted );
            nAttemptedSize *= 2;
        }

        assign( pszFormatted );

        /* We just allocated this string, which means it needs to be deallocated at shutdown time. */
        m_bIsConst = false;
    }

protected:
    virtual void Initialize()
    {
        m_pBuffer = NULL;
        m_pOffset = NULL;
        m_pEndOfBuffer = NULL;
        m_pKMP = NULL;
        m_bIsConst = false;
    }


    /* Code modified from http://www-igm.univ-mlv.fr/~lecroq/string/node8.html#SECTION0080 */
    void preKmp(const int m)
    {

        ScopedLock sl( GetStringSearchMutex() );

        int i, j;

        if ( m_pBuffer == NULL )
            return;

        if ( m_pKMP == NULL )
        {
            m_pKMP = (int *)Allocate( sizeof( int ) * ( m + 1) );
            cout << "Assigning KMP to address " << m_pKMP << endl;
        }

        i = 0;
        j = *m_pKMP = -1;

        while (i < m)
        {
            while (j > -1 && m_pBuffer[i] != m_pBuffer[j])
                j = m_pKMP[j];
            i++;
            j++;
            if (m_pBuffer[i] == m_pBuffer[j])
                m_pKMP[i] = m_pKMP[j];
            else
                m_pKMP[i] = j;
        }
    }


    unsigned int KMP( const LOGOG_CHAR *y, int n )
    {
        int i, j;

        int m = size() - 1; // ignore NULL char

        /* Preprocessing */
        if ( m_pKMP == NULL )
            preKmp( m );

        /* Searching */
        i = j = 0;
        while (j < n)
        {
            while (i > -1 && m_pBuffer[i] != y[j])
                i = m_pKMP[i];
            i++;
            j++;
            if (i >= m)
            {
                return (j - i);
                // We would do this if we cared about multiple substrings
                // i = m_pKMP[i];
            }
        }

        return (unsigned int)npos;
    }

    static const int ASIZE = 1 << ( sizeof( LOGOG_CHAR ) * 8 );

    // from http://www-igm.univ-mlv.fr/~lecroq/string/node14.html#SECTION00140
    void preBmBc(char *x, int m, int bmBc[])
    {
        int i;

        for (i = 0; i < ASIZE; ++i)
            bmBc[i] = m;
        for (i = 0; i < m - 1; ++i)
            bmBc[x[i]] = m - i - 1;
    }


    void suffixes(char *x, int m, int *suff)
    {
        int f, g, i;

        suff[m - 1] = m;
        g = m - 1;
        for (i = m - 2; i >= 0; --i)
        {
            if (i > g && suff[i + m - 1 - f] < i - g)
                suff[i] = suff[i + m - 1 - f];
            else
            {
                if (i < g)
                    g = i;
                f = i;
                while (g >= 0 && x[g] == x[g + m - 1 - f])
                    --g;
                suff[i] = f - g;
            }
        }
    }

    void preBmGs(char *x, int m, int bmGs[])
    {
        int i, j;
        int *suff;

        suff = (int *)Allocate( sizeof( int ) * size());

        suffixes(x, m, suff);

        for (i = 0; i < m; ++i)
            bmGs[i] = m;
        j = 0;
        for (i = m - 1; i >= 0; --i)
            if (suff[i] == i + 1)
                for (; j < m - 1 - i; ++j)
                    if (bmGs[j] == m)
                        bmGs[j] = m - 1 - i;
        for (i = 0; i <= m - 2; ++i)
            bmGs[m - 1 - suff[i]] = m - 1 - i;

        Deallocate( suff );
    }

#define LOGOG_MAX( a, b ) ( ( a > b ) ? a : b )

    int BM(char *y, int n)
    {
        int i, j;

        char *x = m_pBuffer;
        int m = size();

        if ( bmGs == NULL )
        {
            bmGs = (int *)Allocate( sizeof( int ) * size() );
            bmBc = (int *)Allocate( sizeof( int ) * ASIZE );
        }

        /* Preprocessing */
        preBmGs(x, m, bmGs);
        preBmBc(x, m, bmBc);

        /* Searching */
        j = 0;
        while (j <= n - m)
        {
            for (i = m - 1; i >= 0 && x[i] == y[i + j]; --i);
            if (i < 0)
            {
                return j;
                // we would do this if we cared about multiple matches
                // j += bmGs[0];
            }
            else
            {
                int q = bmGs[i];
                int r = bmBc[y[i + j]] - m + 1 + i;
                j += LOGOG_MAX( q, r );
            }
        }

        return npos;
    }

    LOGOG_CHAR *m_pBuffer;
    LOGOG_CHAR *m_pOffset;
    LOGOG_CHAR *m_pEndOfBuffer;
    int *m_pKMP;
    int *bmBc;
    int *bmGs;
    bool m_bIsConst;
};

}

#define LOGOG_STRING ::logog::String

#endif // __LOGOG_STRING_HPP_
