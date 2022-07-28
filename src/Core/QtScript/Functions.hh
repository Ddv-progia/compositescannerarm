/*
 * Core/QtScript/Functions.hh
 */

#pragma once

#include <QtScript/QScriptContext>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>

namespace script {

  QScriptValue sleep(QScriptContext* ctx, QScriptEngine* engine);
  QScriptValue pause(QScriptContext* ctx, QScriptEngine* engine);

}