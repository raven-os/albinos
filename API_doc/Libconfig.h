//
// Must be included to use the LibConfig
//
//

#pragma once

# include <stddef.h>

#ifdef _cplusplus
namespace LibConfig
{
#endif

  ///
  /// \brief typedef the funcptr for setting change subscription
  ///
  typedef void (*FCPTR_ON_CHANGE_NOTIFIER)(void *data, char const *newValue);

  ///
  /// \brief contain a unique id for each configuration
  ///
  /// \param 'size' is the size of 'data' in bytes
  ///
  struct Id {
    void *data;
    size_t size;
  };

  ///
  ///
  /// SUBSCRIPTION
  ///
  ///

  ///
  /// \brief manage your subscriptions
  ///
  struct Subscription;

  ///
  /// \brief unsubscribe
  /// \param the 'struct Subscription' you want to unsubscribe
  /// \return error code
  ///
  int unsubscribe(struct Subscription*);

  ///
  ///
  /// CONFIG
  ///
  ///

  ///
  /// \brief Config class is used to manipulate configuration
  ///
  struct Config;

  ///
  /// \brief Create a config with the given name
  /// \param 'configName' the name you want to give to your config
  /// \param 'returnedConfig' if the func success, contain a pointer to a new 'struct Config'
  /// \return error code
  ///
  int createConfig(char const *configName, struct Config **returnedConfig);

  ///
  /// \brief get config from id
  /// \param 'id' the id of the config that you want to retrieve
  /// \param 'returnedConfig' if the func success, contain a pointer to a new 'struct Config'
  /// \return error code
  ///
  int getConfig(struct Id const* id, struct Config **returnedConfig);

  ///
  /// \brief get const config from id
  /// \param 'id' the id of the config that you want to retrieve
  /// \param 'returnedConfig' if the func success, contain a const pointer to a new 'struct Config'
  /// \return error code
  ///
  int getReadOnlyConfig(struct Id const* id, struct Config const **returnedConfig);

  ///
  /// \brief release the config. Must be call when config is not needed anymore
  /// \param the config
  /// \return error code
  ///
  int releaseConfig(struct Config const *);

  ///
  /// \brief get the id of the given config
  /// \param the config
  /// \param 'configId' if the func success, contain the config id
  /// \return error code
  ///
  int getConfigId(struct Config const *, struct Id *configId);

  ///
  /// \brief basic operation to add a new setting or modify an existing one
  /// \param the config you want to modify
  /// \param 'name' setting name
  /// \param 'value' new setting value
  /// \return error code
  ///
  int setSetting(struct Config*, char const *name, char const *value);

  ///
  /// \brief set or modify an alias
  /// \param the config you want to modify
  /// \param 'name' setting name
  /// \param 'aliasName' alias name
  /// \return error code
  ///
  int setSettingAlias(struct Config*, char const *name, char const *aliasName);

  ///
  /// \brief unset the given alias
  /// \param the config you want to modify
  /// \param 'aliasName' alias name
  /// \return error code
  ///
  int unsetAlias(struct Config*, char const *aliasName);

  ///
  /// \brief remove the given setting
  /// \param the config you want to modify
  /// \param 'name' setting name
  /// \return error code
  ///
  int removeSetting(struct Config*, char const *name);

  ///
  /// \brief get the setting's value
  /// \param the config
  /// \param 'settingName' setting name
  /// \param 'value' if NULL, the value's size will be stored in 'valueSize', else setting value will be wrote here without exceeding 'valueSize' bytes
  /// \param 'valueSize' if 'value' is NULL, the value's size will be stored here, else, must contain the size of the buffer pointed by 'value'
  /// \return error code
  ///
  int getSettingValue(struct Config const*, char const *settingName, char *value, size_t *valueSize);

  ///
  /// \brief get the size of the setting's value
  /// \param the config
  /// \param 'settingName' setting name
  /// \param 'size' size of the setting's value
  /// \return error code
  ///
  int getSettingSize(struct Config const*, char const *settingName, size_t *value);

  ///
  /// \brief inherit from another config
  /// \param the config you want to modify
  /// \param 'inheritFrom' the other config which will be included
  /// \return error code
  ///
  int include(struct Config *config, struct Config const *inheritFrom);

  ///
  /// \brief be notified when a setting change
  /// \param the config you want to modify
  /// \param 'name' setting you want to watch
  /// \param 'data' point to what-you-want, allowing you to get your own data on callback
  /// \param 'onChange' func ptr on callback
  /// \param 'subscription' in case of success, point on a new 'struct Subscription'
  /// \return error code
  ///
  int subscribeToSetting(struct Config*, char const *name, void *data, FCPTR_ON_CHANGE_NOTIFIER onChange, struct Subscription **subscription);

#ifdef _cplusplus
}
#endif
