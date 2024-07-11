/*
 * Core/ConfigurationLocator.hh
 */

#pragma once

#include <QtCore/QString>

class Configuration {
	static QString m_configPath;
public:
	static void init(int argc, char* argv[]);
	static QString getConfigurationPathname(const QString& basename);
};

QString getConfigurationPathname(const QString& basename);

