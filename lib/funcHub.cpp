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

Albinos::ReturnedValue Albinos::include(Config *config, Config const *inheritFrom)
{
  if (!config || !inheritFrom)
    return BAD_PARAMETERS;
  return config->include(inheritFrom);
}

Albinos::ReturnedValue Albinos::subscribeToSetting(Config *config, char const *name, void *data, FCPTR_ON_CHANGE_NOTIFIER onChange, Subscription **subscription)
{
  if (!config || !name || !onChange)
    return BAD_PARAMETERS;
  return config->subscribeToSetting(name, data, onChange, subscription);
}
