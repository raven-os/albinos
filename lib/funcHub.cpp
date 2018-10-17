# include "funcHub.hpp"

void LibConfig::unsubscribe(Subscription *sub)
{
  delete sub;
}

///
/// \todo implementation
///
LibConfig::ReturnedValue LibConfig::createConfig(char const *configName, Config **returnedConfig)
{
  (void)configName;
  (void)returnedConfig;
  return SUCCESS;
}

///
/// \todo implementation
///
LibConfig::ReturnedValue LibConfig::getConfig(Id const* id, Config **returnedConfig)
{
  (void)id;
  (void)returnedConfig;
  return SUCCESS;
}

///
/// \todo implementation
///
LibConfig::ReturnedValue LibConfig::getReadOnlyConfig(Id const* id, Config const **returnedConfig)
{
  (void)id;
  (void)returnedConfig;
  return SUCCESS;
}

void LibConfig::releaseConfig(Config const *config)
{
  delete config;
}

LibConfig::ReturnedValue LibConfig::getConfigId(Config const *config, Id *configId)
{
  if (!config || !configId)
    return BAD_PARAMETERS;
  return config->getId(configId);
}

LibConfig::ReturnedValue LibConfig::setSetting(Config *config, char const *name, char const *value)
{
  if (!config || !name || !value)
    return BAD_PARAMETERS;
  return config->setSetting(name, value);
}

LibConfig::ReturnedValue LibConfig::setSettingAlias(Config *config, char const *name, char const *aliasName)
{
  if (!config || !name || !aliasName)
    return BAD_PARAMETERS;
  return config->setSettingAlias(name, aliasName);
}

LibConfig::ReturnedValue LibConfig::unsetAlias(Config *config, char const *aliasName)
{
  if (!config || !aliasName)
    return BAD_PARAMETERS;
  return config->unsetAlias(aliasName);
}

LibConfig::ReturnedValue LibConfig::removeSetting(Config *config, char const *name)
{
  if (!config || !name)
    return BAD_PARAMETERS;
  return config->removeSetting(name);
}

LibConfig::ReturnedValue LibConfig::getSettingValue(Config const *config, char const *settingName, char *value, size_t valueSize)
{
  if (!config || !settingName || !value)
    return BAD_PARAMETERS;
  return config->getSettingValue(settingName, value, valueSize);
}

LibConfig::ReturnedValue LibConfig::getSettingSize(Config const *config, char const *settingName, size_t *size)
{
  if (!config || !settingName || !size)
    return BAD_PARAMETERS;
  return config->getSettingSize(settingName, size);
}

LibConfig::ReturnedValue LibConfig::include(Config *config, Config const *inheritFrom)
{
  if (!config || !inheritFrom)
    return BAD_PARAMETERS;
  return config->include(inheritFrom);
}

LibConfig::ReturnedValue LibConfig::subscribeToSetting(Config *config, char const *name, void *data, FCPTR_ON_CHANGE_NOTIFIER onChange, Subscription **subscription)
{
  if (!config || !name || !onChange)
    return BAD_PARAMETERS;
  return config->subscribeToSetting(name, data, onChange, subscription);
}
