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
    return name;
}

static void rowDestroyer(GtkWidget *widget, gpointer data)
{
    std::string searchedName = (char const *)data;
    std::string currentName = gtk_label_get_label(GTK_LABEL(gtk_bin_get_child(GTK_BIN(widget))));
    if (searchedName == currentName)
        gtk_widget_destroy(widget);
}

void ConfigManager::deleteConfig(std::string const &name)
{
    if (Albinos::destroyConfig(p_configs[name]) != Albinos::SUCCESS) {
        std::cerr << "Error when destroying config" << std::endl;
    }
    Albinos::releaseConfig(p_configs[name]);
    gtk_container_foreach(GTK_CONTAINER(p_list), rowDestroyer, (gpointer)name.c_str());
    p_configs.erase(name);
}

void ConfigManager::removeConfig(std::string const &name)
{
    Albinos::releaseConfig(p_configs[name]);
    gtk_container_foreach(GTK_CONTAINER(p_list), rowDestroyer, (gpointer)name.c_str());
    p_configs.erase(name);
}

ConfigManager::~ConfigManager()
{
    for (const auto& conf : p_configs)
        Albinos::releaseConfig(conf.second);
    gtk_widget_destroy(p_list);
}

static void emptyGrid(GtkWidget *widget, gpointer data)
{
    gtk_widget_destroy(widget);
}

void ConfigManager::fetchConfig(std::string const &name)
{
    gtk_container_foreach(GTK_CONTAINER(p_configDisplay), emptyGrid, nullptr);
    if (p_configs.find(name) == p_configs.end()) {
        gtk_widget_show_all(p_list);
        return;
    }
    size_t settingsSize;
    Albinos::Setting *settings;
    size_t aliasesSize;
    Albinos::Alias *aliases;

    gint y = 0;

    if (Albinos::getLocalSettings(p_configs[name], &settings, &settingsSize) == Albinos::SUCCESS) {
        for (unsigned i = 0 ; i < settingsSize ; ++i, ++y) {
            p_fetchedSettings[settings[i].name] = settings[i].value;
            gtk_grid_insert_row(GTK_GRID(p_configDisplay), y);
            GtkWidget *name = gtk_label_new(settings[i].name);
            gtk_grid_attach(GTK_GRID(p_configDisplay), GTK_WIDGET(name), 0, y, 1, 1);
            GtkWidget *value = gtk_entry_new_with_buffer(gtk_entry_buffer_new(settings[i].value, p_fetchedSettings[settings[i].name].size()));
            gtk_grid_attach(GTK_GRID(p_configDisplay), GTK_WIDGET(value), 1, y, 1, 1);
        }
        Albinos::destroySettingsArray(settings, settingsSize);
    }

    if (Albinos::getLocalAliases(p_configs[name], &aliases, &aliasesSize) == Albinos::SUCCESS) {
        for (unsigned i = 0 ; i < aliasesSize ; ++i, ++y) {
            p_fetchedAliases[aliases[i].alias] = aliases[i].setting;
            gtk_grid_insert_row(GTK_GRID(p_configDisplay), y);
            GtkWidget *alias = gtk_entry_new_with_buffer(gtk_entry_buffer_new(aliases[i].alias, strlen(aliases[i].alias)));
            gtk_grid_attach(GTK_GRID(p_configDisplay), GTK_WIDGET(alias), 0, y, 1, 1);
            GtkWidget *name = gtk_label_new(aliases[i].setting);
            gtk_grid_attach(GTK_GRID(p_configDisplay), GTK_WIDGET(name), 1, y, 1, 1);
        }
        Albinos::destroyAliasesArray(aliases, aliasesSize);
    }
    gtk_widget_show_all(p_configDisplay);
}

GtkWidget *ConfigManager::getList()
{
    return p_list;
}

GtkWidget const *ConfigManager::getList() const
{
    return p_list;
}
