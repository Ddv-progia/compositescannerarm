/*
 * Core/Devices.hh
 */

#pragma once

#include <DevTalk/Device/AudioDataCollector.hh>
#include <DevTalk/Device/Coil.hh>
#include <DevTalk/Device/IODevice.hh>
#include <DevTalk/Factory/UnitestAPLSystem.hh>
#include <DevTalk/Factory/UnitestAPLCoil.hh>
#include <DevTalk/Factory/UnitestAPLHead.hh>
#include <DevTalk/LowLevelBoard/APLSystem.hh>
#include <DevTalk/Device/APLMultiDevice.hh>

#include <Ice/CommunicatorF.h>
#include "Core/DevicesConfiguration.hh"
#include "Core/ObjectKeeper.hh"

namespace devices {

  extern uts::devtalk::AudioDataCollectorPrx audioDataCollector;
  extern uts::devtalk::device::utscp::APLHeadPrx head;
  extern uts::devtalk::device::utscp::APLCoilPrx coile;
  extern uts::devtalk::utscp::APLSystemPrx aplSystemPrx;
  extern uts::devtalk::drivers::utscp::APLMultiDevicePrx aplMultiDevicePrx;

  void setup(const DevicesConfiguration& conf, const Ice::CommunicatorPtr& comm, ObjectKeeper& objectKeeper);
}
