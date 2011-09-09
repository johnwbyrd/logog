/**
 * \file message.hpp Messages; items transmitted to a log.
 */

#ifndef __LOGOG_MESSAGE_HPP__
#define __LOGOG_MESSAGE_HPP__

namespace logog {
	class Message : public Checkpoint
	{
	public:
		Message( const LOGOG_LEVEL_TYPE level = LOGOG_LEVEL_ALL,
			const LOGOG_CHAR *sFileName = NULL,
			const int nLineNumber = 0,
			const LOGOG_CHAR *sGroup = NULL,
			const LOGOG_CHAR *sCategory = NULL,
			const LOGOG_CHAR *sMessage = NULL,
			const double dTimestamp = 0.0f ) :
		Checkpoint( level, sFileName, nLineNumber, sGroup, sCategory, sMessage, dTimestamp )
		{
			/* Messages are always sources, so there's no need to call Initialize() here */
			// Initialize();
			
			PublishToMultiple( AllFilters() );
		}
		Mutex m_Transmitting;
	};

	static Mutex &GetMessageCreationMutex()
	{
		Statics *pStatic = &Static();

		if ( pStatic->s_pMessageCreationMutex == NULL )
			pStatic->s_pMessageCreationMutex = new Mutex();

		return *( pStatic->s_pMessageCreationMutex );
	}

	void DestroyMessageCreationMutex()
	{
		Statics *pStatic = &Static();

		if ( pStatic->s_pMessageCreationMutex != NULL )
		{
			delete pStatic->s_pMessageCreationMutex;
			pStatic->s_pMessageCreationMutex = NULL;
		}
	}
};


#endif // __LOGOG_MESSAGE_HPP_
