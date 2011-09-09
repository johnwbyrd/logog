/**
 * \file target.hpp Abstractions representing logging outputs.
 */

#ifndef __LOGOG_TARGET_HPP_
#define __LOGOG_TARGET_HPP_

namespace logog
{
	/** A target is abstraction representing an output stream from logog.  cerr, cout, and syslog, and other logging formats are supported.
	 ** Targets do not validate their received data.  Their only job is to render it on a call to Receive() to their supported target 
	 ** type.  Targets should generally make sure to handle calls on multiple threads -- they should make sure to avoid overlapping
	 ** outputs from multiple threads correctly.
	 **/
	class Target : public TopicSink 
	{
		friend class LogBuffer;
	public :
		Target()
		{
			SetFormatter( GetDefaultFormatter() );
			LockableNodesType *pAllTargets = &AllTargets();

			{
				ScopedLock sl( *pAllTargets );
				pAllTargets->insert( this );
			}

			SubscribeToMultiple( AllFilters() );
		}

		void SetFormatter( Formatter &formatter )
		{
			m_pFormatter = &formatter;
		}

		Formatter &GetFormatter() const
		{
			return *m_pFormatter;
		}

		virtual int Output( const LOGOG_STRING &data ) = 0;

		virtual int Receive( const Topic &topic )
		{
			ScopedLock sl( m_MutexReceive );
			return Output( m_pFormatter->Format( topic ) );
		}

	protected: 
		/** A pointer to the formatter used for this output. */
		Formatter *m_pFormatter;
		Mutex m_MutexReceive;
	};

	class Cerr : public Target 
	{
		virtual int Output( const LOGOG_STRING &data )
		{
			cerr << (const LOGOG_CHAR *)data;
			return 0;
		}
	};

	class Cout : public Target 
	{
		virtual int Output( const LOGOG_STRING &data )
		{
			cout << (const LOGOG_CHAR *)data;
			return 0;
		}
	};

	class OutputDebug : public Target 
	{
		virtual int Output( const LOGOG_STRING &data )
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
	};

	/** A LogFile renders received messages to a file.  Provide the name of the file to be rendered to as
	 * a parameter to the construction of the LogFile() object.  Destroying a LogFile object will cause the
	 * output file to be closed.  LogFile objects always append to the output file; they do not delete the previous
	 * log file.
	 */
	class LogFile : public Target
	{
	public:
		/** Creates a LogFile object.
		 * \param sFileName The name of the file to be created. */
		LogFile(const LOGOG_CHAR *sFileName) :
			m_bFirstTime( true ),
			m_pFile( NULL )
		{
			m_FileName.assign( sFileName );
		}

		/** Closes the log file. */
		~LogFile()
		{
			if ( m_pFile )
				fclose( m_pFile );
		}

		/** Opens the log file on first write. */
		virtual int Open()
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

		/** Writes the message to the log file. */
		virtual int Output( const LOGOG_STRING &data )
		{
			int result = 0;
			if ( m_bFirstTime )
			{
				result = Open();
				if ( result != 0 )
					return result;

				m_bFirstTime = false;
			}

			result = fwrite( &(*data), data.size(), 1, m_pFile );

			if ( result != 1 )
				return -1;

			return 0;
		}

	protected:
		FILE *m_pFile;
		LOGOG_STRING m_FileName;
		bool m_bFirstTime;

	private:
		LogFile();
	};

	/** A buffering target.  Stores up to a fixed buffer size of output and then renders that output to another
	  * target.  Can be used for buffering log output in memory and then storing it to a log file upon program completion.
	  * To use, create another target (such as a LogFile) and then create a LogBuffer, providing the other target
	  * as a parameter to the creation function.
	  */
	class LogBuffer : public Target
	{
	public:
		LogBuffer( Target *pTarget = NULL, 
			size_t s = LOGOG_DEFAULT_LOG_BUFFER_SIZE ) :
			m_pStart( NULL ),
			m_nSize( 0 )
		{
			m_pOutputTarget = pTarget;
			Allocate( s );
		}

		~LogBuffer()
		{
			Dump();
			Deallocate();
		}

		/** Changes the current rendering target.  NOTE: This function does no locking on either the target or 
		 * this object.  Program accordingly.
		 */
		virtual void SetTarget( Target &t )
		{
			m_pOutputTarget = &t;
		}

		/** Inserts a range of LOGOG_CHAR objects into this buffer.  The characters should consist of a null-terminated 
		 * string of length size.  Providing anything else as input creates undefined behavior.
		 */
		virtual int Insert( const LOGOG_CHAR *pChars, size_t size )
		{
			// account for null termination now
			size++;

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

		/** Dumps the current contents of the buffer to the output target. */
		virtual int Dump()
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

				sOut.assign( pCurrent, pCurrent + *pSize );

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

		virtual int Output( const LOGOG_STRING &data )
		{
			return Insert( &(*data), data.size() );
		}

	protected:
		virtual void Allocate( size_t size )
		{
			m_nSize = size;
			m_pCurrent = m_pStart = (LOGOG_CHAR *)Object::Allocate( size * sizeof( LOGOG_CHAR ));
			m_pEnd = m_pStart + size;
		}

		virtual void Deallocate()
		{
			if ( m_pStart )
				Object::Deallocate( m_pStart );

			m_nSize = 0;
		}

		/** The non-changing pointer to the basic buffer. */
		LOGOG_CHAR *m_pStart;
		/** The current write offset into the buffer. */
		LOGOG_CHAR *m_pCurrent;
		/** The position in the buffer after which no data may be written. */
		LOGOG_CHAR *m_pEnd;
		/** The size of the buffer in LOGOG_CHAR primitives. */
		size_t m_nSize;
		/** A pointer to the target to which the buffer will be rendered upon calling Dump(). */
		Target *m_pOutputTarget;
	};

}

#endif // __LOGOG_TARGET_HPP_
