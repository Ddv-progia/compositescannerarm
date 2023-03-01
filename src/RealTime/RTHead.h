#pragma once

#include "RTReceiverI.h"
#include <thread>

namespace realtime {
	class RTHead : public RTReceiverI
	{
		std::thread m_thread;
		std::atomic_bool m_isStarted;
	public:
		RTHead();
		void start(int periodMs);
		void stop();
		void dataReady(const uts::devtalk::ByteSeq& data, const ::Ice::Current & = ::Ice::Current()) override;
		~RTHead();
	};
}