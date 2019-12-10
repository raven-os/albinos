//
// Created by thedoktor on 12/6/19.
//

#include "ConfigManager.hpp"
# include <iostream>

ConfigManager::ConfigManager(GtkWidget *list, GtkWidget *configDisplay)
: p_list(list)
, p_configDisplay(configDisplay)
{}



void ConfigManager::addNewConfig(std::string const &name)
{
    if (!name.size() || p_configs.find(name) != p_configs.end())
        return;
    Albinos::Config *newConf = nullptr;

    if (Albinos::createConfig(name.c_str(), &newConf) != Albinos::SUCCESS) {
        std::cerr << "Error when creating config" << std::endl;
        return ;
    }
    p_configs[name] = newConf;
    GtkWidget *item_list = gtk_label_new(name.c_str());
    gtk_list_box_prepend(GTK_LIST_BOX(p_list), item_list);
    p_listElems[name] = item_list;
}

std::string ConfigManager::addExistingConfig(std::string const &key)
{
    Albinos::Config *newConf = nullptr;
    Albinos::Key keyConfig{key.c_str(), key.size(), Albinos::KeyType::READ_WRITE};

    if (Albinos::getConfig(keyConfig, &newConf) != Albinos::SUCCESS) {
        std::cerr << "Error when retrieving config" << std::endl;
        return "";
    }

    std::string name = Albinos::getConfigName(newConf);
    p_configs[name] = newConf;
    GtkWidget *item_list = gtk_label_new(name.c_str());
    gtk_list_box_prepend(GTK_LIST_BOX(p_list), item_list);
    p_listElems[name] = item_list;
    return name;
}

void ConfigManager::removeConfig(std::string const &name)
{
    Albinos::releaseConfig(p_configs[name]);
    gtk_widget_destroy(p_listElems[name]);
    p_configs.erase(name);
    p_listElems.erase(name);
}

void ConfigManager::deleteConfig(std::string const &name)
{
    if (Albinos::destroyConfig(p_configs[name]) != Albinos::SUCCESS) {
        std::cerr << "Error when destroying config" << std::endl;
    }
    Albinos::releaseConfig(p_configs[name]);
    gtk_widget_destroy(p_listElems[name]);
    p_configs.erase(name);
    p_listElems.erase(name);
}

ConfigManager::~ConfigManager()
{
    for (const auto& conf : p_configs)
        Albinos::releaseConfig(conf.second);
    for (const auto& widget : p_listElems)
        gtk_widget_destroy(widget.second);
}

std::map<std::string, GtkWidget *> const &ConfigManager::getListElems() const
{
    return p_listElems;
}
