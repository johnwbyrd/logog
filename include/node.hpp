/**
 * \file node.hpp Base class for higher-level logog objects.
 */

#ifndef __LOGOG_NODE_HPP__
#define __LOGOG_NODE_HPP__

namespace logog
{
/** The base class for most high-level logog objects.  Represents the publisher-subscriber
 ** model within logog. */
class Node;

/** An aggregation of nodes.  Internally, we choose a set representation because we want to be able to traverse
 ** the aggregation quickly while still looking up entries quickly.
 **/
typedef LOGOG_SET< Node *, std::less< Node * >, Allocator< Node * > > NodesType;

/** A type that double inherits from NodesType and Mutex.  A lockable NodesType.  Handles the copy
 ** case correctly.
 **/
class LockableNodesType : public NodesType, public Mutex
{
public:
    /** A LockableNodesType shouldn't copy the internal Mutex when it is copied, but it
     ** should copy all the internal nodes.
     **/
    LockableNodesType & operator = (const LockableNodesType &other)
    {
        /* This function is only used at shutdown. */
        LockableNodesType::const_iterator it;

        it = other.begin();
        while ( it != other.end())
        {
            this->insert( *it );
            it++;
        }

        return *this;
    }
};

static LockableNodesType &GetStaticNodes( void ** pvLocation )
{
    Statics *pStatic = &Static();

#ifdef LOGOG_INTERNAL_DEBUGGING
    if ( pStatic == NULL )
        LOGOG_INTERNAL_FAILURE;
#endif

    if ( *pvLocation == NULL )
        *pvLocation = new LockableNodesType();

    return *(( LockableNodesType *)( *pvLocation ));

}

/** Returns a reference to the global nodes group.  Allocates a new global node group if one does not already
 ** exist.
 */
static LockableNodesType &AllNodes()
{
    return GetStaticNodes( &(Static().s_pAllNodes) );
}

/** Returns a reference to the group of nodes that are capable of subscribing.  Allocates a new global subscriber
 ** node group if one does not already exist.
 */
static LockableNodesType &AllSubscriberNodes()
{
    return GetStaticNodes( &(Static().s_pAllSubscriberNodes ) );
}

/** Returns a reference to the group of nodes that are capable of both subscribing as well as publishing.  Allocates a new global subscriber
 ** node group if one does not already exist.
 */
static LockableNodesType &AllFilters()
{
    return GetStaticNodes( &(Static().s_pAllFilterNodes ) );
}

/** Returns a reference to the group of nodes that represent terminals in the graph, i.e. nodes that can't publish. */
static LockableNodesType &AllTargets()
{
    return GetStaticNodes( &(Static().s_pAllTargets ) );
}

class Node : public Object
{
public:

    /** All nodes self-register as part of the all-nodes database. */
    Node()
    {
        AllNodes().insert( this );
    }

    ~Node()
    {
        Clear();
        AllNodes().erase( this );
    }

    /** Call this function immediately after creating a node (or any of the children of the node class.)  This function currently
     ** registers the node as part of the list of subscriber nodes, if this node may in fact subscribe.
     ** If this node is capable of subscribing at all, then this function registers this node as a possible subscriber.
     ** Doing this helps to keep down the number of nodes we search, when we are determining which nodes a new node
     ** might subscribe to.  We have to do this registration as a second step, after the node is completely
     ** initialized, as subscriberness is determined late in initialization.
     **/
    virtual void Initialize()
    {
        if ( CanSubscribe() )
        {
            LockableNodesType *pSubscriberNodes = &AllSubscriberNodes();

            {
                ScopedLock sl( *pSubscriberNodes );
                pSubscriberNodes->insert( this );
            }

            /* This branch is taken iff this node can both subscribe and publish */
            if ( CanPublish() )
            {
                LockableNodesType *pFilterNodes = &AllFilters();
                {
                    ScopedLock sl( *pFilterNodes );
                    pFilterNodes->insert( this );
                }
            }
        }
    }

    /** Can a node send notifications?  By default they can; later subclasses may not be able to. */
    virtual bool CanPublish() const
    {
        return true;
    }
    /** Can a node receive notifications?  By default they can; later subclasses may not be able to. */
    virtual bool CanSubscribe() const
    {
        return true;
    }
    /** Is this node interested in receiving notifications from another topic? */
    virtual bool CanSubscribeTo( const Node & )
    {
        return CanSubscribe();
    }

    /** In order to avoid bringing in a bunch of RTTI stuff, we permit nodes to be asked whether they're topics or not */
    virtual bool IsTopic() const
    {
        return false;
    }

    /** Causes this node to begin publishing events to the given subscriber.
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

        {
            ScopedLock sl( m_Subscribers );
            bWasInserted = ( m_Subscribers.insert( &subscriber ) ).second;
        }

        if ( bWasInserted )
            subscriber.SubscribeTo( *this );

        return bWasInserted;
    }

    /** Causes this node to attempt to publish to some other nodes. */
    virtual bool PublishToMultiple( LockableNodesType &nodes )
    {
        LockableNodesType::iterator it;

        bool bWasPublished = false;

        nodes.Lock();
        it = nodes.begin();

        while ( it != nodes.end() )
        {
            nodes.Unlock();

            if ( PublishTo( **it ) == true )
                bWasPublished = true;

            nodes.Lock();
            it++;
        }

        nodes.Unlock();

        return bWasPublished;
    }

    /** Causes this node to stop publishing events to this subscriber.
     ** \param subscriber The node to stop receiving events
     ** \return true if successful, false if the subscriber was not being published to in the first place
     **/
    virtual bool UnpublishTo( Node &subscriber )
    {
#ifdef LOGOG_INTERNAL_DEBUGGING
        if ( &subscriber == this )
            LOGOG_INTERNAL_FAILURE;
#endif
        bool bWasRemoved = false;

        {
            ScopedLock sl( m_Subscribers );
            NodesType::iterator it;

            if ( ( it = m_Subscribers.find( &subscriber) ) != m_Subscribers.end() )
            {
                bWasRemoved = true;
                m_Subscribers.erase( it );
            }
        }

        if ( bWasRemoved )
            subscriber.UnsubscribeTo( *this );

        return bWasRemoved;
    }

    /** Causes this node to attempt to unpublish to some other nodes. */
    virtual bool UnpublishToMultiple( LockableNodesType &nodes )
    {
        LockableNodesType::iterator it;

        bool bWasUnpublished = false;

        nodes.Lock();
        it = nodes.begin();

        while ( it != nodes.end() )
        {
            nodes.Unlock();

            if ( UnpublishTo( **it ) == true )
                bWasUnpublished = true;

            nodes.Lock();
            it++;
        }

        nodes.Unlock();

        return bWasUnpublished;
    }

    /** Causes this node to start receiving events from the given publisher.
     ** \param publisher The node to start receiving events from
     ** \return true if successful, false if the publisher was already subscribed
     **/
    virtual bool SubscribeTo( Node &publisher )
    {
#ifdef LOGOG_INTERNAL_DEBUGGING
        if ( &publisher == this )
            LOGOG_INTERNAL_FAILURE;
#endif
        bool bWasInserted;

        {
            ScopedLock sl( m_Publishers );
            bWasInserted = ( m_Publishers.insert( &publisher ) ).second;
        }

        if ( bWasInserted )
            publisher.PublishTo( *this );

        return bWasInserted;
    }

    /** Causes this node to attempt to subscribe to some other nodes. */
    virtual bool SubscribeToMultiple( LockableNodesType &nodes )
    {
        LockableNodesType::iterator it;

        bool bWasSubscribed = false;

        nodes.Lock();
        it = nodes.begin();

        while ( it != nodes.end() )
        {
            nodes.Unlock();
            if ( SubscribeTo( **it ) == true )
                bWasSubscribed = true;

            nodes.Lock();
            it++;
        }

        nodes.Unlock();

        return bWasSubscribed;
    }


    /** Causes this node to unsubscribe from the given publisher's events.
    ** \param publisher The publisher to unsubscribe from
    ** \return true if successful, false if the node was already unsubscribed
    **/
    virtual bool UnsubscribeTo( Node &publisher )
    {
#ifdef LOGOG_INTERNAL_DEBUGGING
        if ( &publisher == this )
            LOGOG_INTERNAL_FAILURE;
#endif
        bool bWasRemoved = false;

        {
            ScopedLock sl( m_Publishers );
            NodesType::iterator it;

            if ( ( it = m_Publishers.find( &publisher ) ) != m_Publishers.end() )
            {
                bWasRemoved = true;
                m_Publishers.erase( it );
            }
        }

        if ( bWasRemoved )
            publisher.UnpublishTo( *this );

        return bWasRemoved;
    }

    /** Causes this node to attempt to unsubscribe to some other nodes. */
    virtual bool UnsubscribeToMultiple( LockableNodesType &nodes )
    {
        LockableNodesType::iterator it;

        bool bWasUnsubscribed = false;

        nodes.Lock();
        it = nodes.begin();

        while ( it != nodes.end() )
        {
            nodes.Unlock();
            if ( UnsubscribeTo( **it ) == true )
                bWasUnsubscribed = true;

            nodes.Lock();
            it++;
        }

        nodes.Unlock();

        return bWasUnsubscribed;
    }

    void Clear()
    {
        {
            ScopedLock sl( m_Publishers );
            m_Publishers.clear();
        }
        {
            ScopedLock sl( m_Subscribers );
            m_Publishers.clear();
        }
    }


    /** A pointer to any custom data you need to store for a node. */
    void *m_pUserData1;

    /** A pointer to any custom data you need to store for a node. */
    void *m_pUserData2;

protected:
    /** A bunch of nodes that are interested in what this node has to report. */
    LockableNodesType	m_Subscribers;

    /** A bunch of nodes that this node interested in hearing from. */
    LockableNodesType	m_Publishers;
};

static void DestroyNodesList( void **pvList )
{
    LockableNodesType **ppNodesList = (LockableNodesType **)pvList;

    if ( *ppNodesList == NULL )
        return;

    (*ppNodesList)->clear();
    delete *ppNodesList;
    *ppNodesList = NULL;
}


/** Destroys all nodes currently recorded.  This happens at shutdown time.  NOTE!  If you have allocated
 ** your own logog items and free them yourself AFTER this call, exciting crashes will occur.
 **/
static void DestroyAllNodes()
{
    Statics *pStatics = &Static();

    LockableNodesType *pAllNodes = ( LockableNodesType *)pStatics->s_pAllNodes;

    if ( pAllNodes == NULL )
        return;

    /** Destroy all the node groups, but don't destroy their contents -- we'll do that as the next step. */
    DestroyNodesList( &(pStatics->s_pAllSubscriberNodes ));
    DestroyNodesList( &(pStatics->s_pAllFilterNodes ));
    DestroyNodesList( &(pStatics->s_pAllTargets ));

    /* We have to copy the AllNodes because destroying each node will remove it from AllNodes.  Fortunately
     * this only happens at shutdown, so we don't have to worry about efficiency.
     */
    LockableNodesType nodes = *pAllNodes;

    LockableNodesType::iterator it;

    it = nodes.begin();

    while ( it != nodes.end() )
    {
        delete *it;
        it++;
    }

    nodes.clear();

#ifdef LOGOG_INTERNAL_DEBUGGING
    if ( pAllNodes->size() != 0 )
        cout << "Not all nodes were deleted at shutdown -- memory leak may have occurred" << endl;
#endif

    pAllNodes->clear(); // just in case
    delete pAllNodes;
    pStatics->s_pAllNodes = NULL;
}
}

#endif // __LOGOG_NODE_HPP_
