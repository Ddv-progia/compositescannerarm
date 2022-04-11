/*
 * Core/ObjectKeeper.hh
 */

#pragma once

#include <memory>
#include <DevTalk/Core/GarbageCollection.hh>

class ObjectKeeper
{
  struct Impl;
  std::unique_ptr<Impl> impl;
public:
  ObjectKeeper();
  ~ObjectKeeper();

  void registerObject(const uts::devtalk::AutomaticallyReclaimedPrx& prx);
  void unregisterObject(const uts::devtalk::AutomaticallyReclaimedPrx& prx);

  void start();
  void stop();
};
