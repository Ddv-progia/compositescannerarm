/*
 * Core/ObjectKeeper.cc
 */

#include <algorithm>
#include <vector>
#include <boost/chrono/chrono.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include "Core/ObjectKeeper.hh"

struct ObjectKeeper::Impl
{
  std::vector<uts::devtalk::AutomaticallyReclaimedPrx> objects;
  boost::mutex objectsMutex;
  boost::thread keeperThread;

  void operator()();
};


void ObjectKeeper::Impl::operator()()
{
  try {
    while (true) {
      {
        boost::lock_guard<boost::mutex> lock(objectsMutex);
        for (auto i = begin(objects); i != end(objects);) {
          try {
            (*i)->keepObject();
            ++i;
          } catch (...) {
            i = objects.erase(i);
          }
        }
      }

      boost::this_thread::sleep_for(boost::chrono::minutes(1));
    }
  } catch (boost::thread_interrupted&) {
  }
}

ObjectKeeper::ObjectKeeper()
  : impl(new Impl)
{ }

ObjectKeeper::~ObjectKeeper()
{ }

void ObjectKeeper::registerObject(const uts::devtalk::AutomaticallyReclaimedPrx& prx)
{
  boost::lock_guard<boost::mutex> lock(impl->objectsMutex);
  impl->objects.push_back(prx);
}

void ObjectKeeper::unregisterObject(const uts::devtalk::AutomaticallyReclaimedPrx& prx)
{
  boost::lock_guard<boost::mutex> lock(impl->objectsMutex);
  auto i = std::find(begin(impl->objects), end(impl->objects), prx);
  if (i != impl->objects.end()) impl->objects.erase(i);
}

void ObjectKeeper::start()
{
  impl->keeperThread = boost::thread(boost::ref(*impl));
}

void ObjectKeeper::stop()
{
  if (impl->keeperThread.get_id() != boost::thread().get_id()) {
    impl->keeperThread.interrupt();
    impl->keeperThread.join();
  }
}
