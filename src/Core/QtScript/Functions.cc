/*
 * Core/QtScript/Functions.cc
 */

#include <boost/chrono/chrono.hpp>
#include <boost/thread/thread.hpp>

#include "Core/QtScript/Functions.hh"

QScriptValue script::sleep(QScriptContext* ctx, QScriptEngine* engine)
{
  if (ctx->argumentCount() != 1) {
    return ctx->throwError("sleep() требует один числовой аргумент");
  }

  auto seconds = ctx->argument(0).toInt32();
  boost::this_thread::sleep_for(boost::chrono::seconds(seconds));
  return QScriptValue();
}

QScriptValue script::pause(QScriptContext* ctx, QScriptEngine* engine)
{
  if (ctx->argumentCount() != 1) {
    return ctx->throwError("pause() требует один числовой аргумент");
  }

  auto msseconds = ctx->argument(0).toInt32();
  boost::this_thread::sleep_for(boost::chrono::milliseconds(msseconds));
  return QScriptValue();
}
