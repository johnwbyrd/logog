/**
 * \file topic.hpp Topics -- subjects of interests for publishers and
 * subscribers to communicate about
 */

#ifndef __LOGOG_TOPIC_HPP__
#define __LOGOG_TOPIC_HPP__

namespace logog
{

/** Bit flags representing whether a topic cares about a specific field or not.  1 = care, 0 = don't care. */
typedef enum
{
	TOPIC_LEVEL_FLAG =			0x01,
	TOPIC_LINE_NUMBER_FLAG =	0x02,
	TOPIC_FILE_NAME_FLAG =		0x04,
	TOPIC_GROUP_FLAG =			0x08,
	TOPIC_CATEGORY_FLAG =		0x10,
	TOPIC_MESSAGE_FLAG =		0x20,
	TOPIC_TIMESTAMP_FLAG =		0x40,
	/** Bits 0 through TOPIC_COUNT turned on */
	TOPIC_ALL = 0x7f
} TopicBitsType;

typedef int TOPIC_FLAGS;

/** Offsets within the m_vIntProps and m_vStringProps arrays for this topic. */
typedef enum
{
	TOPIC_LEVEL = 0,
	TOPIC_LINE_NUMBER = 1,
	/** This must be the number of integer fields. */
	TOPIC_INT_COUNT = 2,

	TOPIC_FILE_NAME = 0,
	TOPIC_GROUP = 1,
	TOPIC_CATEGORY = 2,
	TOPIC_MESSAGE = 3,
	/** This must be the number of string fields for this topic. */
	TOPIC_STRING_COUNT = 4
} TopicOffsetType;


/** A subject that nodes can choose to discuss with one another.
 ** Subscribers generally have very general topics, while publishers generally have very specific topics.
 **/
class Topic : public Node
{
	friend class TopicLevel;
	friend class TopicGroup;

public:
	/** Creates a topic.  Note the defaults for creating a topic -- these defaults are equivalent to "no setting"
	 ** for those fields.
	 **/
	Topic( const LOGOG_LEVEL_TYPE level = LOGOG_LEVEL_ALL,
	       const LOGOG_CHAR *sFileName = NULL,
	       const int nLineNumber = 0,
	       const LOGOG_CHAR *sGroup = NULL,
	       const LOGOG_CHAR *sCategory = NULL,
	       const LOGOG_CHAR *sMessage = NULL,
	       const double dTimestamp = 0.0f )
	{
		m_TopicFlags = 0;

		if ( sFileName != NULL )
		{
			m_vStringProps[ TOPIC_FILE_NAME ] = sFileName;
			m_TopicFlags |= TOPIC_FILE_NAME_FLAG;
		}

		if ( sGroup != NULL )
		{
			m_vStringProps[ TOPIC_GROUP ] = sGroup;
			m_TopicFlags |= TOPIC_GROUP_FLAG;
		}

		if ( sCategory != NULL )
		{
			m_vStringProps[ TOPIC_CATEGORY ] = sCategory;
			m_TopicFlags |= TOPIC_CATEGORY_FLAG;
		}

		if ( sMessage != NULL )
		{
			m_vStringProps[ TOPIC_MESSAGE ] = sMessage;
			m_TopicFlags |= TOPIC_MESSAGE_FLAG;
		}

		m_vIntProps[ TOPIC_LEVEL ] = level;

		if ( level != LOGOG_LEVEL_ALL )
		{
			m_TopicFlags |= TOPIC_LEVEL_FLAG;
		}

		m_vIntProps[ TOPIC_LINE_NUMBER ] = nLineNumber;

		if ( nLineNumber != 0 )
		{
			m_TopicFlags |= TOPIC_LINE_NUMBER_FLAG;
		}

		m_tTime = dTimestamp;

		if ( dTimestamp != 0.0f )
			m_TopicFlags |= TOPIC_TIMESTAMP_FLAG;
	}

	/** Topics are always topics.  We use this to avoid any RTTI dependence. */
	virtual bool IsTopic() const
	{
		return true;
	}

	/** Causes this topic to publish another topic to all its subscribers.
	 ** \return 0 if successful, non-zero if this topic failed to send the publication to all subscribers */
	virtual int Send( const Topic &node )
	{
		LockableNodesType::iterator it;

		{
			ScopedLock sl( m_Subscribers );
			it = m_Subscribers.begin();
		}

		/* Iterate over the subscribers, but only addressing the subscribers group while locking it */
		Topic *pCurrentTopic;
		Node *pCurrentNode;
		m_Subscribers.Lock();
		int nError = 0;

		while ( it != m_Subscribers.end() )
		{
			pCurrentNode = *it;

			if ( pCurrentNode->IsTopic() == false )
				continue;

			pCurrentTopic = ( Topic * )pCurrentNode;

			m_Subscribers.Unlock();

			if ( pCurrentTopic )
				nError += pCurrentTopic->Receive( node );

			m_Subscribers.Lock();
			it++;
		}

		m_Subscribers.Unlock();

		return nError;
	}

	/** Causes this topic to publish itself to all its subscribers. */
	virtual int Transmit()
	{
		return Send( *this );
	}

	/** Permits this node to receive a publication from another node, and act upon it.
	 ** \param node The node constituting the publication
	 ** \return 0 if successful, non-zero if this node failed to process the publication
	 **/
	virtual int Receive( const Topic &node )
	{
		/* Default implementation -- send it on to all children */
		return Send( node );
	}

	/** Is this topic interested in receiving notifications from another topic?  This function implements
	 ** a generic (slow) test that should work for all topic types.  This function only checks fields
	 ** that have previously been set on this topic -- fields that have not been set will not limit this
	 ** topic's ability to subscribe.  If any of the previously set fields does not "match" the other topic,
	 ** this function will return false.  The matching function behaves slightly differently from field to
	 ** field.
	 ** - In the topic level case, this function rejects a publisher with a lower topic level than our
	 ** own.
	 ** - In the group, category, file name and message case, this function rejects a publisher if our
	 ** own group/category/file name or message cannot be found as a substring in the possible publisher.
	 ** This functionality permits very simple pattern matching functionality (i.e. show me all the message
	 ** lines that have the word "upload" in them, regardless of their log level.)
	 ** - In the line number case, this function rejects a publisher unless the line number matches exactly.
	 ** - In the timestamp case, this function rejects a publisher if their timestamp is before our own.
	 ** \param otherNode The topic which we are considering subscribing to
	 **/
	virtual bool CanSubscribeTo( const Node &otherNode )
	{
		if ( CanSubscribe() == false )
			return false;

		if ( otherNode.IsTopic() == false )
			return false;

		Topic *pTopic = ( Topic * )&otherNode;

		/* This function will change from topic class to topic class. */
		return CanSubscribeCheckTopic( *pTopic );
	}

	virtual bool CanSubscribeCheckTopic( const Topic &other )
	{
		/* This is the generic comparison case.  We'll want to optimize this function for other types
		 * of topics.
		 */

		/* Check topics in likely order of disinterest */
		if ( m_TopicFlags & TOPIC_LEVEL_FLAG )
		{
			/* Topic levels are less interesting the larger the numbers are. */
			if ( other.m_vIntProps[ TOPIC_LEVEL ] > m_vIntProps[ TOPIC_LEVEL ] )
				return false;
		}

		if ( m_TopicFlags & TOPIC_GROUP_FLAG )
		{
			/* If our topic is not a substring of the publisher's topic, ignore this */
			if (( other.m_vStringProps[ TOPIC_GROUP ] ).find( m_vStringProps[ TOPIC_GROUP ] ) == LOGOG_STRING::npos )
				return false;
		}

		if ( m_TopicFlags & TOPIC_CATEGORY_FLAG )
		{
			/* If our topic is not a substring of the publisher's topic, ignore this */
			if (( other.m_vStringProps[ TOPIC_CATEGORY ] ).find( m_vStringProps[ TOPIC_CATEGORY ] ) == LOGOG_STRING::npos )
				return false;
		}

		if ( m_TopicFlags & TOPIC_FILE_NAME_FLAG )
		{
			/* If our topic is not a substring of the publisher's file name, ignore this. */
			if (( other.m_vStringProps[ TOPIC_FILE_NAME ] ).find( m_vStringProps[ TOPIC_FILE_NAME ] ) == LOGOG_STRING::npos )
				return false;
		}

		if ( m_TopicFlags & TOPIC_LINE_NUMBER_FLAG )
		{
			/* If our line number doesn't equal theirs, ignore this */
			if ( other.m_vIntProps[ TOPIC_LINE_NUMBER ] != m_vIntProps[ TOPIC_LINE_NUMBER ] )
				return false;
		}

		if ( m_TopicFlags & TOPIC_MESSAGE_FLAG )
		{
			/* If our topic is not a substring of the publisher's file name, ignore this. */
			if (( other.m_vStringProps[ TOPIC_MESSAGE ] ).find( m_vStringProps[ TOPIC_MESSAGE ] ) == LOGOG_STRING::npos )
				return false;
		}

		if ( m_TopicFlags & TOPIC_TIMESTAMP_FLAG )
		{
			/* Timestamps are only interesting if they're greater than or equal to ours. */
			if ( other.m_tTime < m_tTime )
				return false;
		}

		/* all tests passed */
		return true;
	}

		/** Causes this topic to begin publishing events to the given subscriber.
		 ** \param subscriber The node to receive published events
		 ** \return true if the request was successful, false if the subscriber was already subscribed
		 **/
		virtual bool PublishTo( Node &subscriber )
		{
	#ifdef LOGOG_INTERNAL_DEBUGGING
			if ( &subscriber == this )
				LOGOG_INTERNAL_FAILURE;
	#endif
			bool bWasInserted;

			/** Additional checking may be required first -- can the subscriber handle this publishing? */
			if ( subscriber.IsTopic() )
			{
				Topic *pSubscriber = (Topic *)&subscriber;

				if ( pSubscriber->CanSubscribeTo( *this ) == false )
					return false;
			}

			{
				ScopedLock sl( m_Subscribers );
				bWasInserted = ( m_Subscribers.insert( &subscriber ) ).second;
			}

			if ( bWasInserted )
				subscriber.SubscribeTo( *this );

			return bWasInserted;
		}



	/** Formats the message in this topic given a sprintf-style set of arguments.
	 ** This function can be used to set the current message in this topic to a string with a variable number of parameters.
	 **/
	void Format( const LOGOG_CHAR *cFormatMessage, ... )
	{
		va_list args;

		va_start( args, cFormatMessage );
		m_vStringProps[ TOPIC_MESSAGE ].format_va( cFormatMessage, args );
		va_end( args );

		m_TopicFlags |= TOPIC_MESSAGE_FLAG;
	}

	const LOGOG_STRING &FileName() const
	{
		return m_vStringProps[ TOPIC_FILE_NAME ];
	}

	void FileName( const LOGOG_STRING &s )
	{
		m_vStringProps[ TOPIC_FILE_NAME ] = s;
		m_TopicFlags |= TOPIC_FILE_NAME_FLAG;
	}

	const LOGOG_STRING &Message() const
	{
		return m_vStringProps[ TOPIC_MESSAGE ];
	}

	void Message( const LOGOG_STRING &s )
	{
		m_vStringProps[ TOPIC_MESSAGE ] = s;
		m_TopicFlags |= TOPIC_MESSAGE_FLAG;
	}

	const LOGOG_STRING &Category() const
	{
		return m_vStringProps[ TOPIC_CATEGORY ];
	}

	void Category( const LOGOG_STRING &s )
	{
		m_vStringProps[ TOPIC_CATEGORY ] = s;
		m_TopicFlags |= TOPIC_CATEGORY_FLAG;
	}

	const LOGOG_STRING &Group() const
	{
		return m_vStringProps[ TOPIC_GROUP ];
	}

	void Group( const LOGOG_STRING &s )
	{
		m_vStringProps[ TOPIC_GROUP ] = s;
		m_TopicFlags |= TOPIC_GROUP_FLAG;
	}

	int LineNumber() const
	{
		return m_vIntProps[ TOPIC_LINE_NUMBER ];
	}

	void LineNumber( const int num )
	{
		m_vIntProps[ TOPIC_LINE_NUMBER ] = num;
		m_TopicFlags |= TOPIC_LINE_NUMBER_FLAG;
	}

	LOGOG_LEVEL_TYPE Level() const
	{
		return ( LOGOG_LEVEL_TYPE )m_vIntProps[ TOPIC_LEVEL ];
	}

	void Level( LOGOG_LEVEL_TYPE level )
	{
		m_vIntProps[ TOPIC_LEVEL ] = level;
		m_TopicFlags |= TOPIC_LEVEL_FLAG;
	}

	LOGOG_TIME Timestamp() const
	{
		return m_tTime;
	}

	void Timestamp( const LOGOG_TIME t )
	{
		m_tTime = t;
		m_TopicFlags |= TOPIC_TIMESTAMP_FLAG;
	}

	TOPIC_FLAGS GetTopicFlags() const
	{
		return m_TopicFlags;
	}

protected:
	/** An array (not an STL vector) of string properties for this topic. */
	LOGOG_STRING m_vStringProps[ TOPIC_STRING_COUNT ];
	/** An array (not an STL vector) of integer properties for this topic. */
	int m_vIntProps[ TOPIC_INT_COUNT ];
	/** The time associated with this topic.  Usually this field is updated when a topic is triggered.  Times need not be associated
	 ** with a particular topic, in which case this value is zero.
	 ** */
	LOGOG_TIME m_tTime;
	/** A bitfield representing the "important" fields in this topic.  Not all fields are considered to contain important information
	 ** all the time.  A logical OR of the TOPIC_*_FLAG fields.
	 ** \sa TopicBitsType
	 **/
	TOPIC_FLAGS m_TopicFlags;
};

/** A topic that permits both publishing as well as subscribing.  This class is functionally same as a Topic; we've added it
 ** as a class for clarity when referring to different topic types.  Filters should be instantiated only after outputs are 
 ** instantiated, as they automatically search for and publish to targets.  If you instantiate a filter before you
 ** instantiate a target, you will need to call PublishTo( theTarget ) yourself before using the target.
 **/
class Filter : public Topic
{
public:
	Filter( const LOGOG_LEVEL_TYPE level = LOGOG_LEVEL_ALL,
		const LOGOG_CHAR *sFileName = NULL,
		const int nLineNumber = 0,
		const LOGOG_CHAR *sGroup = NULL,
		const LOGOG_CHAR *sCategory = NULL,
		const LOGOG_CHAR *sMessage = NULL,
		const double dTimestamp = 0.0f ) :
	Topic( level, sFileName, nLineNumber, sGroup, sCategory, sMessage, dTimestamp )
	{
		Statics *pStatic = &Static();

#ifdef LOGOG_INTERNAL_DEBUGGING
		if ( pStatic == NULL )
			LOGOG_INTERNAL_FAILURE;
#endif

		if ( pStatic->s_pDefaultFilter == NULL )
			pStatic->s_pDefaultFilter = this;

		PublishToMultiple( AllTargets() );

		LockableNodesType *pFilterNodes = &AllFilters();

		{
			ScopedLock sl( *pFilterNodes );
			pFilterNodes->insert( this );
		}
	}
};

/** Returns a reference to the group of nodes that represent terminals in the graph, i.e. nodes that can't publish. */
static Filter &GetDefaultFilter()
{
	Statics *pStatic = &Static();

#ifdef LOGOG_INTERNAL_DEBUGGING
	if ( pStatic == NULL )
		LOGOG_INTERNAL_FAILURE;
#endif

	if ( pStatic->s_pDefaultFilter == NULL )
	{
		pStatic->s_pDefaultFilter = new Filter( LOGOG_LEVEL );
	}

	return *((Filter *)(pStatic->s_pDefaultFilter));
}


void SetDefaultLevel( LOGOG_LEVEL_TYPE level )
{
	Filter *pDefaultFilter = &GetDefaultFilter();

	pDefaultFilter->Level( level );
}

/** A topic with the group name being the only field of significance. */
class TopicGroup : public Topic
{
public:
	TopicGroup( const LOGOG_CHAR *sGroup = NULL ) :
		Topic( LOGOG_LEVEL_ALL, NULL, 0, sGroup )
	{
	}

	virtual bool CanSubscribeCheckTopic( const Topic &other )
	{
		if ( m_TopicFlags & TOPIC_LEVEL_FLAG )
		{
			/* Topic levels are less interesting the larger the numbers are. */
			if ( other.m_vIntProps[ TOPIC_LEVEL ] > m_vIntProps[ TOPIC_LEVEL ] )
				return false;
		}

		return true;
	}
};

/** A topic with the level being the only field of significance. */
class TopicLevel : public Topic
{
public:
	TopicLevel( const LOGOG_LEVEL_TYPE level ) :
		Topic( level )
	{
	}

	virtual bool CanSubscribeCheckTopic( const Topic &other )
	{
		/* Check topics in likely order of disinterest */
		if ( m_TopicFlags & TOPIC_LEVEL_FLAG )
		{
			/* Topic levels are less interesting the larger the numbers are. */
			if ( other.m_vIntProps[ TOPIC_LEVEL ] > m_vIntProps[ TOPIC_LEVEL ] )
				return false;
		}

		/* all tests passed */
		return true;
	}
};

/** A topic that is also a source. */
class TopicSource : public Topic
{
public:
	TopicSource( const LOGOG_LEVEL_TYPE level = LOGOG_LEVEL_ALL,
	             const LOGOG_CHAR *sFileName = NULL,
	             const int nLineNumber = 0,
	             const LOGOG_CHAR *sGroup = NULL,
	             const LOGOG_CHAR *sCategory = NULL,
	             const LOGOG_CHAR *sMessage = NULL,
	             const double dTimestamp = 0.0f ) :
		Topic( level, sFileName, nLineNumber, sGroup, sCategory, sMessage, dTimestamp )
	{
	}

	/** Returns false.  Sources do not subscribe. */
	virtual bool SubscribeTo( Node & )
	{
		return false;
	}

	/** Returns false.  Sources do not unsubscribe. */
	virtual bool UnsubscribeTo( Node & )
	{
		return false;
	}

	virtual bool CanSubscribe() const
	{
		return false;
	}
};

/** A topic that is also a sink. */
class TopicSink : public Topic
{
public:
	virtual bool IsTopic() const
	{
		return true;
	}

	/** Sinks do not add themselves to the list of interested subscribers.  That's up to intermediate topics to decide. */
	virtual void Initialize() {}

	/** Returns false.  Sinks do not publish. */
	virtual bool PublishTo( Node & )
	{
		return false;
	}

	/** Returns false.  Sinks do not unpublish. */
	virtual bool UnpublishTo( Node & )
	{
		return false;
	}

	/** Returns false.  Sinks do not publish. */
	virtual bool CanPublish() const
	{
		return false;
	}
};
}

#endif // __LOGOG_TOPIC_HPP_
