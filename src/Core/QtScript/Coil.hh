/*
 * Core/QtScript/Coil.hh
 */

#pragma once

#include <DevTalk/Device/Coil.hh>
#include <QtCore/QObject>
#include <QtScript/QScriptEngine>

namespace script {

  class Coil : public QObject
  {
    Q_OBJECT

    Q_PROPERTY(double halfPeriod READ getHalfPeriod WRITE setHalfPeriod)
    Q_PROPERTY(double initialHalfPeriod READ getInitialHalfPeriod WRITE setInitialHalfPeriod)
    Q_PROPERTY(double workingHalfPeriod READ getWorkingHalfPeriod WRITE setWorkingHalfPeriod)
    Q_PROPERTY(double minimumLevel READ getMinimumLevel WRITE setMinimumLevel)
  public:
    Coil(const uts::devtalk::device::utscp::APLCoilPrx& prx, QScriptEngine* scriptEngine);

    Q_INVOKABLE void start();
    Q_INVOKABLE void startSingle();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void switchOnGenerator();
    Q_INVOKABLE void searchWorkingRange();

  private:
    uts::devtalk::device::utscp::APLCoilPrx prx;
    QScriptEngine* scriptEngine;

    double getHalfPeriod() const;
    void setHalfPeriod(double x);
    double getInitialHalfPeriod() const;
    void setInitialHalfPeriod(double x);
    double getWorkingHalfPeriod() const;
    void setWorkingHalfPeriod(double x);
    double getMinimumLevel() const;
    void setMinimumLevel(double x);

    template<typename F>
    void wrapExceptions(F f);

    template<typename F>
    double wrapExceptionsDouble(F f) const;

    template<typename F>
    void setMode(F f, Ice::Long timeout);
  };

}
