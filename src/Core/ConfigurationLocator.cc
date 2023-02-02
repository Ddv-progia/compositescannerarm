/*
 * Core/ConfigurationLocator.cc
 */

#include <QtGlobal>
#include <QtCore/QDir>
#include <qmessagebox.h>
#include "Core/ConfigurationLocator.hh"
#include <boost/program_options.hpp>

QString Configuration::m_configPath = "";


void Configuration::init(int argc, char* argv[]) {
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("config-root", boost::program_options::value<std::string>(), "set configuration file to use")
        ;
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm);
    boost::program_options::notify(vm);

    std::string configRoot;
    if (vm.count("config-root")) {
        configRoot = vm["config-root"].as<std::string>();
    }
    else {
        QMessageBox::critical(nullptr, "Ошибка", "Не задан конфигурационный каталог");
        exit(1);
    }
    m_configPath = QString::fromStdString(configRoot);
}

QString Configuration::getConfigurationPathname(const QString& basename)
{
  return m_configPath + "/" + basename;
}
