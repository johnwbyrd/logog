 /* 
 * \file target.cpp
 */

#include "logog.hpp"

#include <iostream>

namespace logog {

	Target::Target() :
		m_bNullTerminatesStrings( true )
	{
		SetFormatter( GetDefaultFormatter() );
		LockableNodesType *pAllTargets = &AllTargets();

		{
			ScopedLock sl( *pAllTargets );
			pAllTargets->insert( this );
		}

		SubscribeToMultiple( AllFilters() );
	}

	void Target::SetFormatter( Formatter &formatter )
	{
		m_pFormatter = &formatter;
	}

	Formatter & Target::GetFormatter() const
	{
		return *m_pFormatter;
	}

	int Target::Receive( const Topic &topic )
	{
		ScopedLock sl( m_MutexReceive );
		return Output( m_pFormatter->Format( topic, *this ) );
	}


	int Cerr::Output( const LOGOG_STRING &data )
	{
		std::cerr << (const LOGOG_CHAR *)data;
		return 0;
	}
//! [Cout]
	int Cout::Output( const LOGOG_STRING &data )
	{
		std::cout << (const LOGOG_CHAR *)data;
		return 0;
	}
//! [Cout]

	int OutputDebug::Output( const LOGOG_STRING &data )
	{
#ifdef LOGOG_FLAVOR_WINDOWS
#ifdef LOGOG_UNICODE
		OutputDebugStringW( (const LOGOG_CHAR *)data );
#else
		OutputDebugStringA( (const LOGOG_CHAR *)data );
#endif // LOGOG_UNICODE
#endif // LOGOG_FLAVOR_WINDOWS
		return 0;
	}

	LogFile::LogFile(const LOGOG_CHAR *sFileName) :
	m_bFirstTime( true ),
		m_pFile( NULL )
	{
		m_bNullTerminatesStrings = false;
		// We do this in two steps to make sure the file name is copied into the string
		// structure and not just reused.
		String sStringFileName( sFileName );
		m_FileName.assign( sStringFileName );
	}

	LogFile::~LogFile()
	{
		if ( m_pFile )
			fclose( m_pFile );
	}

	int LogFile::Open()
	{
#ifdef LOGOG_FLAVOR_WINDOWS
		// Microsoft prefers its variant
		int nError = fopen_s( &m_pFile, (const LOGOG_CHAR *)m_FileName, "a+" );
		if ( nError != 0 )
			return nError;

#else // LOGOG_FLAVOR_WINDOWS
		m_pFile = fopen( (const LOGOG_CHAR *)m_FileName, "a+" );

#endif // LOGOG_FLAVOR_WINDOWS

		return ( m_pFile ? 0 : -1 );
	}

	int LogFile::Output( const LOGOG_STRING &data )
	{
		int result = 0;
		if ( m_bFirstTime )
		{
			result = Open();
			if ( result != 0 )
				return result;

			m_bFirstTime = false;
		}

		result = fwrite( &(*data), 1, data.size(), m_pFile );

		if ( (size_t)result != data.size() )
			return -1;

		return 0;
	}

	LogBuffer::LogBuffer( Target *pTarget ,
		size_t s  ) :
	m_pStart( NULL ),
		m_nSize( 0 )
	{
		m_pOutputTarget = pTarget;
		Allocate( s );
	}

	LogBuffer::~LogBuffer()
	{
		Dump();
		Deallocate();
	}

	void LogBuffer::SetTarget( Target &t )
	{
		m_pOutputTarget = &t;
	}

	int LogBuffer::Insert( const LOGOG_CHAR *pChars, size_t size )
	{
		if (( m_pCurrent + size ) >= m_pEnd )
			Dump();

		if ( size > (size_t)( m_pEnd - m_pStart ))
		{
#ifdef LOGOG_INTERNAL_DEBUGGING
			cerr << "Cannot insert string into buffer; string is larger than buffer.  Allocate a larger size for the LogBuffer." << endl;
#endif
			return -1; // can't fit this into buffer; punt it
		}

		// Store the size of this string in the buffer
		size_t *pSize;
		pSize = ( size_t *)m_pCurrent;
		*pSize = size;
		m_pCurrent = (LOGOG_CHAR *)++pSize;

		while ( size-- )
			*m_pCurrent++ = *pChars++;

		return 0;
	}

	int LogBuffer::Dump()
	{
		LOGOG_CHAR *pCurrent = m_pStart;
		size_t *pSize;
		int nError;

		if ( m_pOutputTarget == NULL )
			return -1;

		// We have to lock the output target here, as we do an end run around its Receive() function */
		ScopedLock sl( m_pOutputTarget->m_MutexReceive );

		while ( pCurrent < m_pCurrent )
		{
			String sOut;
			// Get the size of this entry
			pSize = ( size_t * )pCurrent;
			// Move past that entry into the data area
			pCurrent = ( LOGOG_CHAR * )( pSize + 1 );

			sOut.assign( pCurrent, pCurrent + *pSize - 1 );

			if ( m_pOutputTarget )
			{
				nError = m_pOutputTarget->Output( sOut );
				if ( nError != 0 )
					return nError;
			}

			pCurrent += *pSize;
		}

		// reset buffer
		m_pCurrent = m_pStart;

		return 0;
	}

	int LogBuffer::Output( const LOGOG_STRING &data )
	{
		return Insert( &(*data), data.size() );
	}

	void LogBuffer::Allocate( size_t size )
	{
		m_nSize = size;
		m_pCurrent = m_pStart = (LOGOG_CHAR *)Object::Allocate( size * sizeof( LOGOG_CHAR ));
		m_pEnd = m_pStart + size;
	}

	void LogBuffer::Deallocate()
	{
		if ( m_pStart )
			Object::Deallocate( m_pStart );

		m_nSize = 0;
	}
}

