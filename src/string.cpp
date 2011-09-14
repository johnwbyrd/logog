 
/* 
 * \file string.cpp
 */

#include "logog.hpp"

namespace logog {

	String::String()
	{
		Initialize();
	}

	String::~String()
	{
		Free();
	}

	void String::Free()
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

	unsigned int String::Length( const LOGOG_CHAR *chars )
	{
		unsigned int len = 0;

		while ( *chars++ )
			len++;

		return len;
	}

	String & String::operator=( const String & other )
	{
		Initialize();
		assign( other );
		return *this;
	}

	String & String::operator=( const LOGOG_CHAR *pstr )
	{
		Initialize();
		assign( pstr );
		return *this;
	}

	String::String( const String &other )
	{
		Initialize();
		assign( other );
	}

	String::String( const LOGOG_CHAR *pstr )
	{
		Initialize();
		assign( pstr );
	}

	int String::size() const
	{
		return ( m_pOffset - m_pBuffer );
	}

	void String::clear()
	{
		m_pOffset = m_pBuffer;
	}

	int String::reserve( unsigned int nSize )
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

	int String::reserve_for_int()
	{
		return reserve( 32 );
	}

	String::operator const LOGOG_CHAR *() const
	{
		return m_pBuffer;
	}

	int String::assign( const String &other )
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

	int String::assign( const int value )
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

	int String::assign( const LOGOG_CHAR *other, const LOGOG_CHAR *pEnd /*= NULL */ )
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
	int String::append( const String &other )
	{
#ifdef LOGOG_INTERNAL_DEBUGGING
		if ( m_bIsConst )
			cout << "Can't reassign const string!" << endl;
#endif

		return append( other.m_pBuffer );
	}

	int String::append( const LOGOG_CHAR *other )
	{
#ifdef LOGOG_INTERNAL_DEBUGGING
		if ( m_bIsConst )
			cout << "Can't reassign const string!" << endl;
#endif

		while (( m_pOffset < m_pEndOfBuffer ) && ( *other != (LOGOG_CHAR)NULL ))
			*m_pOffset++ = *other++;

		return ( m_pOffset - m_pBuffer );
	}

	int String::append( const LOGOG_CHAR c )
	{
		if ( m_pOffset < m_pEndOfBuffer )
			*m_pOffset++ = c;

		return ( m_pOffset - m_pBuffer );
	}
	void String::reverse( LOGOG_CHAR* pStart, LOGOG_CHAR* pEnd )
	{
		char temp;

		while( pEnd > pStart)
		{
			temp=*pEnd, *pEnd-- =*pStart, *pStart++=temp;
		}
	}

	bool String::is_valid() const
	{
		return ( m_pBuffer != NULL );
	}

	int String::find( String &other ) const
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

	void String::format( const LOGOG_CHAR *cFormatString, ... )
	{
		va_list args;

		va_start(args, cFormatString);
		format_va( cFormatString, args );
		va_end( args );
	}

	void String::format_va( const LOGOG_CHAR *cFormatString, va_list args )
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

	void String::Initialize()
	{
		m_pBuffer = NULL;
		m_pOffset = NULL;
		m_pEndOfBuffer = NULL;
		m_pKMP = NULL;
		m_bIsConst = false;
	}

	void String::preKmp( const int m )
	{
		ScopedLock sl( GetStringSearchMutex() );

		int i, j;

		if ( m_pBuffer == NULL )
			return;

		if ( m_pKMP == NULL )
		{
			m_pKMP = (int *)Allocate( sizeof( int ) * ( m + 1) );
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

	unsigned int String::KMP( const LOGOG_CHAR *y, int n )
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

	void String::preBmBc( char *x, int m, int bmBc[] )
	{
		int i;

		for (i = 0; i < ASIZE; ++i)
			bmBc[i] = m;
		for (i = 0; i < m - 1; ++i)
			bmBc[x[i]] = m - i - 1;
	}

	void String::suffixes( char *x, int m, int *suff )
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

	void String::preBmGs( char *x, int m, int bmGs[] )
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

	int String::BM( char *y, int n )
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
}

