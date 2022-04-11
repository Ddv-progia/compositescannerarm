/*
 * Core/ConfigurationLocator.cc
 */

#include <QtGlobal>
#include <QtCore/QDir>

#include "Core/ConfigurationLocator.hh"

QString getConfigurationPathname(const QString& basename)
{
  auto envdir = QString::fromLocal8Bit(qgetenv("COMPOSITE_SCANNER_CONFIG_DIR"));
  return (envdir.isEmpty() ? QDir::currentPath() : envdir) + "/" + basename;
}
