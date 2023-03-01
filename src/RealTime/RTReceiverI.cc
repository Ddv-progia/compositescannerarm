#include "RTReceiverI.h"
#include <iostream>

realtime::RTReceiverI::
RTReceiverI( std::string name )
: m_name(name)
{
    /*Ice::ObjectPrx obj = m_communicator->stringToProxy("IceStorm/TopicManager@Ice-Storm.TopicManager");
    m_topicManager = IceStorm::TopicManagerPrx::checkedCast(obj);
    m_adapter = m_communicator->createObjectAdapterWithEndpoints("Real-Time-Adapter", "tcp");
    m_proxy = m_adapter->addWithUUID(this)->ice_oneway();
    m_adapter->activate();

    while (!m_topic) {
        try {
            m_topic = m_topicManager->retrieve(m_name);
        }
        catch (const IceStorm::NoSuchTopic&) {
            try {
                m_topic = m_topicManager->create(m_name);
            }
            catch (const IceStorm::TopicExists&) {
                // Another client created the topic.
            }
        }
    }
    IceStorm::QoS qos;
    m_topic->subscribeAndGetPublisher(qos, m_proxy);*/
}

void realtime::RTReceiverI::
setSubscriptionName(const ::std::string& name, const ::Ice::Current &) {
    m_name = name;
}

void realtime::RTReceiverI::
dataReady(const ::uts::devtalk::ByteSeq& data, const ::Ice::Current &) {
    std::cout << "Data : " << data.size() << data.at(0) << " " << data.at(1) << data.at(2) << std::endl;
}

realtime::RTReceiverI::
~RTReceiverI() {
    //m_topic->unsubscribe(m_proxy);
}