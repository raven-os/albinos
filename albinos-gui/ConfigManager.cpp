//
// Created by thedoktor on 12/6/19.
//

#include "ConfigManager.hpp"
# include <iostream>

ConfigManager::ConfigManager(GtkWidget *list, GtkWidget *configDisplay)
: p_list(list)
, p_configDisplay(configDisplay)
, p_focusedConfig("")
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
    if (name == p_focusedConfig)
        p_focusedConfig = "";
}

void ConfigManager::removeConfig(std::string const &name)
{
    p_focusedConfig = "";
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

static void upButton(GtkWidget *widget, gpointer data)
{
    ConfigManager *configManager = static_cast<ConfigManager *>(data);

    GtkGrid *settingGrid = GTK_GRID(gtk_widget_get_parent(widget));
    configManager->updateSetting(gtk_label_get_label(GTK_LABEL(gtk_grid_get_child_at(settingGrid, 2, 0))),
            gtk_entry_get_text(GTK_ENTRY(gtk_grid_get_child_at(settingGrid, 3, 0))));
}

static void delButton(GtkWidget *widget, gpointer data)
{
    ConfigManager *configManager = static_cast<ConfigManager *>(data);

    GtkGrid *settingGrid = GTK_GRID(gtk_widget_get_parent(widget));
    configManager->deleteSetting(gtk_label_get_label(GTK_LABEL(gtk_grid_get_child_at(settingGrid, 2, 0))));
    gtk_widget_destroy(GTK_WIDGET(settingGrid));
}

void ConfigManager::fetchConfig(std::string const &name)
{
    gtk_container_foreach(GTK_CONTAINER(p_configDisplay), emptyGrid, nullptr);
    if (p_configs.find(name) == p_configs.end()) {
        gtk_widget_show_all(p_list);
        return;
    }
    p_focusedConfig = name;
    size_t settingsSize;
    Albinos::Setting *settings;
//    size_t aliasesSize;
//    Albinos::Alias *aliases;
    Albinos::Key key;

    gint y = 0;
    if (Albinos::getConfigKey(p_configs[name], &key) == Albinos::SUCCESS) {
        gtk_grid_insert_row(GTK_GRID(p_configDisplay), y);
        GtkWidget *titleKeyLabel = gtk_label_new(std::string("RW key :").c_str());
        GtkWidget *keyLabel = gtk_label_new(std::string(key.data, key.size).c_str());
        gtk_label_set_selectable(GTK_LABEL(keyLabel), TRUE);
        gtk_grid_attach(GTK_GRID(p_configDisplay), titleKeyLabel, 0, y, 1, 1);
        ++y;
        gtk_grid_insert_row(GTK_GRID(p_configDisplay), y);
        gtk_grid_attach(GTK_GRID(p_configDisplay), keyLabel, 0, y, 1, 1);
        delete key.data;
        ++y;
    }

    if (Albinos::getReadOnlyConfigKey(p_configs[name], &key) == Albinos::SUCCESS) {
        gtk_grid_insert_row(GTK_GRID(p_configDisplay), y);
        GtkWidget *titleKeyLabel = gtk_label_new(std::string("RO key :").c_str());
        GtkWidget *keyLabel = gtk_label_new(std::string(key.data, key.size).c_str());
        gtk_label_set_selectable(GTK_LABEL(keyLabel), TRUE);
        gtk_grid_attach(GTK_GRID(p_configDisplay), titleKeyLabel, 0, y, 1, 1);
        ++y;
        gtk_grid_insert_row(GTK_GRID(p_configDisplay), y);
        gtk_grid_attach(GTK_GRID(p_configDisplay), keyLabel, 0, y, 1, 1);
        delete key.data;
        ++y;
    }

    if (Albinos::getLocalSettings(p_configs[name], &settings, &settingsSize) == Albinos::SUCCESS) {
        for (unsigned i = 0 ; i < settingsSize ; ++i, ++y) {
            p_fetchedSettings[settings[i].name] = settings[i].value;
            gtk_grid_insert_row(GTK_GRID(p_configDisplay), y);
            GtkWidget *settingBox = gtk_grid_new();
            GtkWidget *upBut = gtk_button_new_with_label("up");
            gtk_grid_insert_column(GTK_GRID(settingBox), 0);
            gtk_grid_insert_column(GTK_GRID(settingBox), 1);
            gtk_grid_insert_column(GTK_GRID(settingBox), 2);
            gtk_grid_insert_column(GTK_GRID(settingBox), 3);
            gtk_grid_attach(GTK_GRID(settingBox), upBut, 0, 0, 1, 1);
            g_signal_connect(G_OBJECT(upBut), "clicked", G_CALLBACK(upButton), this);
            GtkWidget *delBut = gtk_button_new_with_label("del");
            gtk_grid_attach(GTK_GRID(settingBox), delBut, 1, 0, 1, 1);
            g_signal_connect(G_OBJECT(delBut), "clicked", G_CALLBACK(delButton), this);
            GtkWidget *name = gtk_label_new(settings[i].name);
            gtk_label_set_selectable(GTK_LABEL(name), TRUE);
            gtk_grid_attach(GTK_GRID(settingBox), name, 2, 0, 1, 1);
            GtkWidget *value = gtk_entry_new_with_buffer(gtk_entry_buffer_new(settings[i].value, p_fetchedSettings[settings[i].name].size()));
            gtk_grid_attach(GTK_GRID(settingBox), value, 3, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(p_configDisplay), settingBox, 0, y, 4, 1);
        }
        Albinos::destroySettingsArray(settings, settingsSize);
    }

//    if (Albinos::getLocalAliases(p_configs[name], &aliases, &aliasesSize) == Albinos::SUCCESS) {
//        for (unsigned i = 0 ; i < aliasesSize ; ++i, ++y) {
//            p_fetchedAliases[aliases[i].alias] = aliases[i].setting;
//            gtk_grid_insert_row(GTK_GRID(p_configDisplay), y);
//            GtkWidget *alias = gtk_entry_new_with_buffer(gtk_entry_buffer_new(aliases[i].alias, strlen(aliases[i].alias)));
//            gtk_grid_attach(GTK_GRID(p_configDisplay), GTK_WIDGET(alias), 0, y, 1, 1);
//            GtkWidget *name = gtk_label_new(aliases[i].setting);
//            gtk_grid_attach(GTK_GRID(p_configDisplay), GTK_WIDGET(name), 1, y, 1, 1);
//        }
//        Albinos::destroyAliasesArray(aliases, aliasesSize);
//    }
    gtk_widget_show_all(gtk_widget_get_parent(p_configDisplay));
}

GtkWidget *ConfigManager::getList()
{
    return p_list;
}

GtkWidget const *ConfigManager::getList() const
{
    return p_list;
}

void ConfigManager::deleteSetting(std::string const &name)
{
    Albinos::removeSetting(p_configs[p_focusedConfig], name.c_str());
}

void ConfigManager::updateSetting(std::string const &name, std::string const &value)
{
    Albinos::setSetting(p_configs[p_focusedConfig], name.c_str(), value.c_str());
}

std::string const &ConfigManager::getFocusedConfig() const
{
    return p_focusedConfig;
}
