#pragma once

#include "RTReceiverI.h"


namespace realtime {
	class RTAudioCollector : public RTReceiverI
	{
	public:
		RTAudioCollector();
		void start(int sampleRate);
		void stop();
		void dataReady(const uts::devtalk::ByteSeq& data, const ::Ice::Current & = ::Ice::Current()) override;
		~RTAudioCollector();
	};
}