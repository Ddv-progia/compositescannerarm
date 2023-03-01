#include "RTContext.h"
#include "RTAudioCollector.h"
#include "RTHead.h"



realtime::RTContext::
RTContext(const Ice::CommunicatorPtr& comm)
: m_communicator(comm){
	Ice::ObjectPrx obj = m_communicator->stringToProxy("IceStorm/TopicManager@Ice-Storm.TopicManager");
	m_topicManager = IceStorm::TopicManagerPrx::checkedCast(obj);
	m_adapter = m_communicator->createObjectAdapterWithEndpoints("Real-Time-Adapter", "tcp");
    m_adapter->activate();
}

std::shared_ptr<realtime::RTReceiverI> realtime::RTContext::
getRTDevice(std::string name) {
	auto device = m_rtDevices.find(name);
	if (device != m_rtDevices.end())
		return device->second;

    std::shared_ptr<realtime::RTReceiverI> reseiver;
    if (name == "AudioDataCollector")
        reseiver = std::make_shared<RTAudioCollector>();
    else if (name == "APLHead")
        reseiver = std::make_shared<RTHead>();
    else
        return nullptr;
    
    Ice::ObjectPrx proxy = m_adapter->addWithUUID(reseiver.get())->ice_oneway();
    IceStorm::TopicPrx topic;
    while (!topic) {
        try {
            topic = m_topicManager->retrieve(name);
        }
        catch (const IceStorm::NoSuchTopic&) {
            try {
                topic = m_topicManager->create(name);
            }
            catch (const IceStorm::TopicExists&) {
                // Another client created the topic.
            }
        }
    }
    IceStorm::QoS qos;
    topic->subscribeAndGetPublisher(qos, proxy);

    m_rtDevices.insert({ name , reseiver });
    return reseiver;
}

realtime::
RTContext::~RTContext() {

}