#pragma once

# include <vector>
# include <gtk/gtk.h>
# include <map>
# include "../lib/Albinos.h"
# include <memory>

class ConfigManager {

    std::map<std::string, Albinos::Config*> p_configs;
    std::map<std::string, std::string> p_fetchedSettings;
    std::map<std::string, std::string> p_fetchedAliases;
    GtkWidget *p_list;
    GtkWidget *p_configDisplay;

public:

    ConfigManager(GtkWidget *list, GtkWidget *configDisplay);
    ~ConfigManager();

    void addNewConfig(std::string const &name);
    std::string addExistingConfig(std::string const &key);
    void removeConfig(std::string const &name);
    void deleteConfig(std::string const &name);
    void fetchConfig(std::string const &name);

    GtkWidget const *getList() const;
    GtkWidget *getList();
};
