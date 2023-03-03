#pragma once

#include "RTReceiverI.h"
#include "Core/ScanDataMetatypes.hh"


namespace realtime {
	class RTAudioCollector : public RTReceiverI
	{
		Q_OBJECT
			Sound *m_sound = nullptr;
	public:
		RTAudioCollector();
		void start(int sampleRate, Sound *sound);
		void stop();
		void dataReady(const uts::devtalk::ByteSeq& data, const ::Ice::Current & = ::Ice::Current()) override;
		~RTAudioCollector();
		Q_SIGNAL void newData();
	};
}