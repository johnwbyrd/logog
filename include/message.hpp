/**
 * \file message.hpp Messages; items transmitted to a log.
 */

#ifndef __LOGOG_MESSAGE_HPP__
#define __LOGOG_MESSAGE_HPP__

namespace logog
{
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

        /* NOTE!  The message is typically assigned to a checkpoint AFTER it's been published.
         * Ergo a message needs to be unpublished from, and published to, all filters
         * iff one of those filters is searching for a substring of that message.
         */
        PublishToMultiple( AllFilters() );
    }

    /** Causes this checkpoint to republish itself to all existing filters after
      * unpublishing itself.  This can be necessary if the message within this
      * message has changed in such a way that the downstream Filter objects
      * might react differently to it.
      */
    virtual bool Republish()
    {
        UnpublishToMultiple( AllFilters() );
        return PublishToMultiple( AllFilters() );
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
