#pragma once

#include <DevTalk/Core/RealTime.hh>
#include <IceStorm/IceStorm.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Communicator.h>
#include <IceUtil/Handle.h>

namespace realtime {
	class RTReceiverI : public virtual uts::devtalk::RTReceiver {
		std::string m_name;
		/*const Ice::CommunicatorPtr m_communicator;
		Ice::ObjectAdapterPtr m_adapter;
		IceStorm::TopicManagerPrx m_topicManager;
		IceStorm::TopicPrx m_topic;
		Ice::ObjectPrx m_proxy;*/
	public:
		RTReceiverI(std::string name);
		virtual void setSubscriptionName(const std::string& name, const ::Ice::Current & = ::Ice::Current()) override;
		virtual void dataReady(const uts::devtalk::ByteSeq& data, const ::Ice::Current & = ::Ice::Current()) override;
		virtual ~RTReceiverI();
	};
}


