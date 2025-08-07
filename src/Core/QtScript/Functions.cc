/*
 * Core/QtScript/Functions.cc
 */

#include <boost/chrono/chrono.hpp>
#include <boost/thread/thread.hpp>

#include "Core/QtScript/Functions.hh"
#include <QtWidgets/QMessageBox>

//QScriptValue script::sleep(QScriptContext* ctx, QScriptEngine* engine)
//{
  //if (ctx->argumentCount() != 1) {
  //  return ctx->throwError("sleep() требует один числовой аргумент");
  //}
  //auto seconds = ctx->argument(0).toInt32();
  //myEngine.globalObject().setProperty("myNumber", 123);
  //...
  // QJSValue myNumberPlusOne = myEngine.evaluate("myNumber + 1");
//}

//QScriptValue script::pause(QScriptContext* ctx, QScriptEngine* engine)
//{
  //if (ctx->argumentCount() != 1) {
  //  return ctx->throwError("pause() требует один числовой аргумент");
  //}

  //auto msseconds = ctx->argument(0).toInt32();
  //boost::this_thread::sleep_for(boost::chrono::milliseconds(msseconds));
  //return QScriptValue();
//}

Q_INVOKABLE QJSValue script::FunctionalObject::sleep(int val)
{
	boost::this_thread::sleep_for(boost::chrono::seconds(val));
	return Q_INVOKABLE QJSValue();
}

Q_INVOKABLE QJSValue script::FunctionalObject::pause(int val)
{
	boost::this_thread::sleep_for(boost::chrono::seconds(val));
	return Q_INVOKABLE QJSValue();
}

Q_INVOKABLE QJSValue script::FunctionalObject::alert(QString str)
{
	pause(1);
	//QMessageBox::information(nullptr, QString::fromStdString(""), str);
	QMessageBox mb;
	mb.setWindowTitle("");
	mb.setText(str);
	//mb.setDetailedText(QString::fromStdString(boost::diagnostic_information(e)));
	mb.setDetailedText(str);
	mb.exec();
	pause(1);
	//TODO  зависает после исполнения.
	return Q_INVOKABLE QJSValue();
}
