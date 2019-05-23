# include "funcHub.hpp"

void Albinos::unsubscribe(Subscription *sub)
{
  delete sub;
}

Albinos::ReturnedValue Albinos::createConfig(char const *configName, Config **returnedConfig)
{
  try {
    *returnedConfig = new Config(configName);
  } catch (std::exception const &e) {
    /// \todo catch error nicely and return relevant error
    return UNKNOWN;
  }
  return SUCCESS;
}

Albinos::ReturnedValue Albinos::getConfig(Key key, Config **returnedConfig)
{
  try {
    *returnedConfig = new Config(key);
  } catch (std::exception const &e) {
    /// \todo catch error nicely and return relevant error
    return UNKNOWN;
  }
  return SUCCESS;
}

Albinos::ReturnedValue Albinos::getReadOnlyConfig(Key key, Config const **returnedConfig)
{
  try {
    *returnedConfig = new Config(key);
  } catch (std::exception const &e) {
    /// \todo catch error nicely and return relevant error
    return UNKNOWN;
  }
  return SUCCESS;
}

void Albinos::releaseConfig(Config const *config)
{
  delete config;
}

Albinos::ReturnedValue Albinos::getConfigKey(Config const *config, Key *configKey)
{
  configKey->data = nullptr;
  if (!config || !configKey)
    return BAD_PARAMETERS;
  return config->getKey(configKey);
}

Albinos::ReturnedValue Albinos::getReadOnlyConfigKey(Config const *config, Key *configKey)
{
  configKey->data = nullptr;
  if (!config || !configKey)
    return BAD_PARAMETERS;
  return config->getReadOnlyKey(configKey);
}

Albinos::ReturnedValue Albinos::setSetting(Config *config, char const *name, char const *value)
{
  if (!config || !name || !value)
    return BAD_PARAMETERS;
  return config->setSetting(name, value);
}

Albinos::ReturnedValue Albinos::setSettingAlias(Config *config, char const *name, char const *aliasName)
{
  if (!config || !name || !aliasName)
    return BAD_PARAMETERS;
  return config->setSettingAlias(name, aliasName);
}

Albinos::ReturnedValue Albinos::unsetAlias(Config *config, char const *aliasName)
{
  if (!config || !aliasName)
    return BAD_PARAMETERS;
  return config->unsetAlias(aliasName);
}

Albinos::ReturnedValue Albinos::removeSetting(Config *config, char const *name)
{
  if (!config || !name)
    return BAD_PARAMETERS;
  return config->removeSetting(name);
}

Albinos::ReturnedValue Albinos::getSettingValue(Config const *config, char const *settingName, char *value, size_t valueSize)
{
  if (!config || !settingName || !value)
    return BAD_PARAMETERS;
  return config->getSettingValue(settingName, value, valueSize);
}

Albinos::ReturnedValue Albinos::getSettingSize(Config const *config, char const *settingName, size_t *size)
{
  if (!config || !settingName || !size)
    return BAD_PARAMETERS;
  return config->getSettingSize(settingName, size);
}

Albinos::ReturnedValue Albinos::include(Config *config, Key *inheritFrom, int position)
{
  if (!config || !inheritFrom)
    return BAD_PARAMETERS;
  return config->include(inheritFrom, position);
}

Albinos::ReturnedValue Albinos::uninclude(Config *config, Key *otherConfig, int position)
{
  if (!config || !otherConfig)
    return BAD_PARAMETERS;
  return config->include(otherConfig, position);
}

Albinos::ReturnedValue Albinos::subscribeToSetting(Config *config, char const *name, void *data, FCPTR_ON_CHANGE_NOTIFIER onChange, Subscription **subscription)
{
  if (!config || !name || !onChange)
    return BAD_PARAMETERS;
  return config->subscribeToSetting(name, data, onChange, subscription);
}

Albinos::ReturnedValue Albinos::getDependencies(Config const *config, Config **deps, size_t *size)
{
  if (!config || !deps || !size)
    return BAD_PARAMETERS;
  return config->getDependencies(deps, size);
}

void Albinos::destroyDependenciesArray(Config *deps, size_t size)
{
  if (!deps)
    return;
  delete[] deps;
}

Albinos::ReturnedValue Albinos::getLocalSettings(Config const *config, Setting **settings, size_t *size)
{
  if (!config || !settings || !size)
    return BAD_PARAMETERS;
  return config->getLocalSettings(settings, size);
}

Albinos::ReturnedValue Albinos::getLocalSettingsNames(Config const *config, char ***names)
{
  if (!config || !names)
    return BAD_PARAMETERS;
  return config->getLocalSettingsNames(names);
}

void Albinos::destroySettingsNamesArray(char **names)
{
  if (!names)
    return;
  for (size_t i = 0 ; names[i] ; ++i)
    delete names[i];
  delete[] names;
}

void Albinos::destroySettingsArray(Setting *settings, size_t size)
{
  if (!settings)
    return;
  for (size_t i = 0 ; i < size ; ++i) {
    delete settings[i].name;
    delete settings[i].value;
  }
  delete[] settings;
}

Albinos::ReturnedValue Albinos::getLocalAliases(Config const *config, Alias **aliases, size_t *size)
{
  if (!config || !aliases || !size)
    return BAD_PARAMETERS;
  return config->getLocalAliases(aliases, size);
}

void Albinos::destroyAliasesArray(Alias *aliases, size_t size)
{
  if (!aliases)
    return;
  for (size_t i = 0 ; i < size ; ++i) {
    delete aliases[i].alias;
    delete aliases[i].setting;
  }
  delete[] aliases;
}

Albinos::ReturnedValue Albinos::destroyConfig(Config const *config)
{
  if (!config)
    return BAD_PARAMETERS;
  return config->deleteConfig();
}

Albinos::ReturnedValue Albinos::pollSubscriptions(Config *config)
{
  if (!config)
    return BAD_PARAMETERS;
  return config->pollSubscriptions();
}
