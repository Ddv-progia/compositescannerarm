/*
 * Core/QtScript/Coil.cc
 */

#include <limits>
#include "Core/QtScript/Coil.hh"

script::Coil::Coil(const uts::devtalk::CoilPrx& prx, QJSEngine* scriptEngine)
  : prx(prx), scriptEngine(scriptEngine)
{ }

void script::Coil::start()
{
  setMode([=] (const uts::devtalk::CompletionWaitTimingPtr& timing) { return prx->switchWorkingMode(timing); }, 100);
}

void script::Coil::startSingle()
{
  setMode([=] (const uts::devtalk::CompletionWaitTimingPtr& timing) { return prx->switchSingleWorkingMode(timing); }, 100);
}

void script::Coil::searchWorkingRange()
{
  setMode([=] (const uts::devtalk::CompletionWaitTimingPtr& timing) { return prx->searchWorkingRange(timing); }, 100);
}

void script::Coil::stop()
{
  wrapExceptions([=] () { prx->stop(); });
}

void script::Coil::switchOnGenerator()
{
  wrapExceptions([=] () { return prx->switchOnGenerator(); });
}

double script::Coil::getHalfPeriod() const
{
  return wrapExceptionsDouble([=] () { return prx->getHalfPeriod(); });
}

void script::Coil::setHalfPeriod(double x)
{
  wrapExceptions([=] () { prx->setHalfPeriod(x); });
}

double script::Coil::getInitialHalfPeriod() const
{
  return wrapExceptionsDouble([=] () { return prx->getInitialHalfPeriod(); });
}

void script::Coil::setInitialHalfPeriod(double x)
{
  wrapExceptions([=] () { prx->setInitialHalfPeriod(x); });
}

double script::Coil::getWorkingHalfPeriod() const
{
  return wrapExceptionsDouble([=] () { return prx->getWorkingHalfPeriod(); });
}

void script::Coil::setWorkingHalfPeriod(double x)
{
  wrapExceptions([=] () { prx->setWorkingHalfPeriod(x); });
}

double script::Coil::getMinimumLevel() const
{
  return wrapExceptionsDouble([=] () { return prx->getMinimumLevel(); });
}

void script::Coil::setMinimumLevel(double x)
{
  wrapExceptions([=] () { prx->setMinimumLevel(x); });
}

template<typename F>
void script::Coil::wrapExceptions(F f)
{
  try {
    f();
  } catch (uts::devtalk::CommunicationException& exc) {
      scriptEngine->throwError(QString::fromUtf8(exc.reason.c_str()));
  } catch (Ice::Exception& exc) {
      scriptEngine->throwError(QString::fromUtf8(exc.what()));
  }
}

template<typename F>
double script::Coil::wrapExceptionsDouble(F f) const
{
  try {
    return f();
  } catch (uts::devtalk::CommunicationException& exc) {
      scriptEngine->throwError(QString::fromUtf8(exc.reason.c_str()));
  } catch (Ice::Exception& exc) {
      scriptEngine->throwError(QString::fromUtf8(exc.what()));
  }
  return std::numeric_limits<double>::quiet_NaN();
}

template<typename F>
void script::Coil::setMode(F f, Ice::Long timeout)
{
  wrapExceptions([=] () { 
    IceUtil::Handle<uts::devtalk::CompletionWaitTiming> timing = new uts::devtalk::CompletionWaitTiming;
    timing->firstTestDelay = 0.5;
    timing->testPause = 0.1;
    timing->timeout = timeout;
    switch (f(timing)) {
    default:
      break;
    case uts::devtalk::ProcessStoppedEmergently:
      {
	auto st = prx->getState();
	switch (st.failure) {
	default:
	  break;
	case uts::devtalk::CoilFailureNoResponse:
        scriptEngine->throwError(QString("Сбой включения рабочего режима датчика АСК: не найден отклик на всём диапазоне"));
        break;
	case uts::devtalk::CoilFailureNoUpperLimit:
        scriptEngine->throwError(QString("Сбой включения рабочего режима датчика АСК: не найдена верхняя граница частоты"));
        break;
	case uts::devtalk::CoilFailureNoWorkingReponse:
        scriptEngine->throwError(QString("Сбой включения рабочего режима датчика АСК: не найден отклик в рабочем диапазоне"));
	  break;
	case uts::devtalk::CoilFailureLostSignal:
        scriptEngine->throwError(QString("Сбой включения рабочего режима датчика АСК: потеря сигнала при работе"));
	  break;
	}
      }
      break;
    case uts::devtalk::ProcessTimedOut:
        scriptEngine->throwError(QString("Таймаут включения рабочего режима датчика АСК"));
      break;
    }
  });
}
