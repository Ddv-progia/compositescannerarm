/*
 * Core/BackgroundTaskExecutor.cc
 */

#include <deque>
#include <boost/exception/all.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <UCL/Exception.hh>

#include "Core/BackgroundTaskExecutor.hh"

struct BackgroundTaskExecutor::Impl
{
  std::deque<ProgressReportingTask*> tasks;
  boost::condition_variable tasksNotEmpty;
  boost::mutex tasksMutex;

  ProgressReportingTask* getTask();
};

ProgressReportingTask* BackgroundTaskExecutor::Impl::getTask()
{
  boost::unique_lock<boost::mutex> lock(tasksMutex);
  while (tasks.empty()) tasksNotEmpty.wait(lock);
  auto t = tasks.front();
  tasks.pop_front();
  return t;
}

BackgroundTaskExecutor::BackgroundTaskExecutor()
  : impl(new Impl)
{ }

BackgroundTaskExecutor::~BackgroundTaskExecutor()
{ }

void BackgroundTaskExecutor::enqueue(ProgressReportingTask* task)
{
  boost::unique_lock<boost::mutex> lock(impl->tasksMutex);
  impl->tasks.push_back(task);
  impl->tasksNotEmpty.notify_one();
}

void BackgroundTaskExecutor::operator()()
{
  try {
    while (true) {
      std::unique_ptr<ProgressReportingTask> t(impl->getTask());
      connect(t.get(), SIGNAL(started(const QString&, int)), this, SIGNAL(started(const QString&, int)), Qt::DirectConnection);
      connect(t.get(), SIGNAL(stageStarted(const QString&, int)), this, SIGNAL(stageStarted(const QString&, int)), Qt::DirectConnection);
      connect(t.get(), SIGNAL(stageProgressed()), this, SIGNAL(stageProgressed()), Qt::DirectConnection);
      connect(t.get(), SIGNAL(finished()), this, SIGNAL(finished()), Qt::DirectConnection);
      connect(t.get(), SIGNAL(terminated(const QString&)), this, SIGNAL(terminated(const QString&)), Qt::DirectConnection);
      connect(t.get(), SIGNAL(createdTask(ProgressReportingTask*)), this, SLOT(enqueue(ProgressReportingTask*)), Qt::DirectConnection);
      try {
        (*t)();
      } catch (boost::exception& exc) {
        auto msg = boost::get_error_info<uts::ErrInfo_Description>(exc);
        if (msg) {
          emit terminated(QString::fromUtf8(msg->c_str()));
        } else {
          emit terminated(QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
        }
      } catch (...) {
        emit terminated(QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
      }
    }
  } catch (boost::thread_interrupted&) {
  }
}
