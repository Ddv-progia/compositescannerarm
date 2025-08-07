/*
 * Core/QtScript/Functions.hh
 */

#pragma once

//#include <QtScript/QScriptContext>
//#include <QtScript/QScriptEngine>
//#include <QtScript/QScriptValue>
#include <QJSEngine>
#include <QJSValue>

namespace script {
	class FunctionalObject : public QObject {
		Q_OBJECT
	public:
		FunctionalObject(QObject* parent = nullptr) : QObject(parent) {}

		Q_INVOKABLE QJSValue sleep(int val);
		Q_INVOKABLE QJSValue pause(int val);
		Q_INVOKABLE QJSValue alert(QString str);
	};

 // //QScriptValue sleep(QScriptContext* ctx, QScriptEngine* engine);
 // //QScriptValue pause(QScriptContext* ctx, QScriptEngine* engine);
}
