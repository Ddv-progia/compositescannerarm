#pragma once
#include <Ice/Communicator.h>
#include <IceStorm/IceStorm.h>
#include "RTReceiverI.h"
#include <map>

namespace realtime {
	class RTContext {
		const Ice::CommunicatorPtr m_communicator;
		Ice::ObjectAdapterPtr m_adapter;
		IceStorm::TopicManagerPrx m_topicManager;

		//Ice::ObjectPrx m_proxy;
		std::map<std::string, std::shared_ptr<RTReceiverI>> m_rtDevices;
	public:
		RTContext(const Ice::CommunicatorPtr& comm);
		std::shared_ptr<RTReceiverI> getRTDevice(std::string name);

		~RTContext();
	};
}