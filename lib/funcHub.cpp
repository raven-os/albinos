#include "funcHub.hpp"
#include "Subscription.hpp"

void Albinos::unsubscribe(Subscription *sub)
{
  delete sub;
}

Albinos::ReturnedValue Albinos::createConfig(char const *configName, Config **returnedConfig)
{
  try {
    *returnedConfig = new Config(configName);
  } catch (LibError const &e) {
    return e.getCode();
  }
  return SUCCESS;
}

Albinos::ReturnedValue Albinos::getConfig(Key key, Config **returnedConfig)
{
  try {
    *returnedConfig = new Config(key);
  } catch (LibError const &e) {
    return e.getCode();
  }
  return SUCCESS;
}

Albinos::ReturnedValue Albinos::getReadOnlyConfig(Key key, Config const **returnedConfig)
{
  try {
    *returnedConfig = new Config(key);
  } catch (LibError const &e) {
    return e.getCode();
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
  try {
    return config->getKey(configKey);
  } catch (LibError const &e) {
    return e.getCode();
  }
}

Albinos::ReturnedValue Albinos::getReadOnlyConfigKey(Config const *config, Key *configKey)
{
  configKey->data = nullptr;
  if (!config || !configKey)
    return BAD_PARAMETERS;
  try {
    return config->getReadOnlyKey(configKey);
  } catch (LibError const &e) {
    return e.getCode();
  }
}

Albinos::ReturnedValue Albinos::setSetting(Config *config, char const *name, char const *value)
{
  if (!config || !name || !value)
    return BAD_PARAMETERS;
  try {
    return config->setSetting(name, value);
  } catch (LibError const &e) {
    return e.getCode();
  }
}

Albinos::ReturnedValue Albinos::setSettingAlias(Config *config, char const *name, char const *aliasName)
{
  if (!config || !name || !aliasName)
    return BAD_PARAMETERS;
  try {
    return config->setSettingAlias(name, aliasName);
  } catch (LibError const &e) {
    return e.getCode();
  }
}

Albinos::ReturnedValue Albinos::unsetAlias(Config *config, char const *aliasName)
{
  if (!config || !aliasName)
    return BAD_PARAMETERS;
  try {
    return config->unsetAlias(aliasName);
  } catch (LibError const &e) {
    return e.getCode();
  }
}

Albinos::ReturnedValue Albinos::removeSetting(Config *config, char const *name)
{
  if (!config || !name)
    return BAD_PARAMETERS;
  try {
    return config->removeSetting(name);
  } catch (LibError const &e) {
    return e.getCode();
  }
}

Albinos::ReturnedValue Albinos::getSettingValue(Config const *config, char const *settingName, char *value, size_t valueSize)
{
  if (!config || !settingName || !value)
    return BAD_PARAMETERS;
  try {
    return config->getSettingValue(settingName, value, valueSize);
  } catch (LibError const &e) {
    return e.getCode();
  }
}

Albinos::ReturnedValue Albinos::getSettingSize(Config const *config, char const *settingName, size_t *size)
{
  if (!config || !settingName || !size)
    return BAD_PARAMETERS;
  try {
    return config->getSettingSize(settingName, size);
  } catch (LibError const &e) {
    return e.getCode();
  }
}

Albinos::ReturnedValue Albinos::include(Config *config, Key *inheritFrom, int position)
{
  if (!config || !inheritFrom)
    return BAD_PARAMETERS;
  try {
    return config->include(inheritFrom, position);
  } catch (LibError const &e) {
    return e.getCode();
  }
}

Albinos::ReturnedValue Albinos::uninclude(Config *config, Key *otherConfig, int position)
{
  if (!config || !otherConfig)
    return BAD_PARAMETERS;
  try {
    return config->uninclude(otherConfig, position);
  } catch (LibError const &e) {
    return e.getCode();
  }
}

Albinos::ReturnedValue Albinos::subscribeToSetting(Config *config, char const *name, void *data, FCPTR_ON_CHANGE_NOTIFIER onChange, Subscription **subscription)
{
  if (!config || !name || !onChange)
    return BAD_PARAMETERS;
  try {
    return config->subscribeToSetting(name, data, onChange, subscription);
  } catch (LibError const &e) {
    return e.getCode();
  }
}

Albinos::ReturnedValue Albinos::getDependencies(Config const *config, Config **deps, size_t *size)
{
  if (!config || !deps || !size)
    return BAD_PARAMETERS;
  try {
    return config->getDependencies(deps, size);
  } catch (LibError const &e) {
    return e.getCode();
  }
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
  try {
    return config->getLocalSettings(settings, size);
  } catch (LibError const &e) {
    return e.getCode();
  }
}

Albinos::ReturnedValue Albinos::getLocalSettingsNames(Config const *config, char const * const **names)
{
  if (!config || !names)
    return BAD_PARAMETERS;
  try {
    return config->getLocalSettingsNames(names);
  } catch (LibError const &e) {
    return e.getCode();
  }
}

void Albinos::destroySettingsNamesArray(char const * const *names)
{
  if (!names)
    return;
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
  try {
    return config->getLocalAliases(aliases, size);
  } catch (LibError const &e) {
    return e.getCode();
  }
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
  try {
    return config->deleteConfig();
  } catch (LibError const &e) {
    return e.getCode();
  }
}

Albinos::ReturnedValue Albinos::pollSubscriptions(Config *config)
{
  if (!config)
    return BAD_PARAMETERS;
  try {
    return config->pollSubscriptions();
  } catch (LibError const &e) {
    return e.getCode();
  }
}

void *Albinos::getSupscriptionUserData(struct Albinos::Subscription const *subsription)
{
  return subsription->getAssociatedUserData();
}

char const *Albinos::getSupscriptionSettingName(struct Albinos::Subscription const *subsription)
{
  return subsription->getAssociatedSetting().c_str();
}
