#pragma once

#include "RTReceiverI.h"
#include <thread>
#include <array>

#define PI 3.1415926535897932384626433832795

namespace realtime {
	class RTHead : public RTReceiverI
	{
		Q_OBJECT

		struct Hand {
			static const int QUANTITY_ENC = 3;
			static const int QUANTITY_EDGES = 4;
			std::array<const uint16_t, QUANTITY_ENC> encShifts = { 4449, 64620, 22489 };
			std::array<const double, QUANTITY_EDGES> edgeLengths = { 57, 191.121, 801.987, 815.529 };
			const uint32_t ENC_MAX = UINT16_MAX;
			const double ENC_TO_RAD = ENC_MAX / (PI * 2.0);
		}m_hand;

		std::thread m_thread;
		std::atomic_bool m_isStarted;
	public:
		RTHead();
		void start(int sampleRate);
		void stop();
		void dataReady(const uts::devtalk::ByteSeq& data, const ::Ice::Current & = ::Ice::Current()) override;
		~RTHead();
		Q_SIGNAL void newData(float x, float y, float z, std::time_t timeStamp);
	};
}