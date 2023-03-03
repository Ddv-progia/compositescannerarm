#pragma once

#include <DevTalk/Core/RealTime.hh>
#include <IceStorm/IceStorm.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Communicator.h>
#include <IceUtil/Handle.h>
#include <QtCore/QObject>

namespace realtime {
	class RTReceiverI : public virtual uts::devtalk::RTReceiver,
		public QObject {
		std::string m_name;
	public:
		RTReceiverI(std::string name);
		virtual void setSubscriptionName(const std::string& name, const ::Ice::Current & = ::Ice::Current()) override;
		virtual void dataReady(const uts::devtalk::ByteSeq& data, const ::Ice::Current & = ::Ice::Current()) override;
		virtual ~RTReceiverI();
	};
}


