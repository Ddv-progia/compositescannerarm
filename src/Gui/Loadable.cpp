
#pragma once

#include "Loadable.hh"
#include <QSettings>

void Loadable::loadLastOpenDir()
{
    QString  settingsFile = "SettingsForAutoScanWindow.ini";
    QSettings* settings = new QSettings(settingsFile, QSettings::IniFormat);

    lastOpenDir = settings->value(QString::fromUtf8("lastOpenDir"), "").toString();
    //if (lastOpenDir == "") {
    //    lastOpenDir = settings->value(QString::fromUtf8("lastOpenDir"), "").toString();
    //}
}

void Loadable::saveLastOpenDir()
{
    QString  settingsFile = "SettingsForAutoScanWindow.ini";
    QSettings* settings = new QSettings(settingsFile, QSettings::IniFormat);
    settings->setValue(QString::fromUtf8("lastOpenDir"), lastOpenDir);
    settings->sync();
}
