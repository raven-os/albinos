//
// Must be included to use the LibConfig
//
//

#pragma once

# include <stddef.h>
# include "ReturnedValue.h"

#ifdef __cplusplus
namespace LibConfig
{
#endif

  ///
  /// \brief type of function pointer for setting change subscription
  ///
  typedef void (*FCPTR_ON_CHANGE_NOTIFIER)(void *data, char const *newValue);

  ///
  /// \brief contains a unique id for each configuration
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
  /// \brief represents a subscription
  ///
  struct Subscription;

  ///
  /// \brief unsubscribe
  /// \param the subscription to unsubscribe from
  ///
  void unsubscribe(struct Subscription*);

  ///
  ///
  /// CONFIG
  ///
  ///

  ///
  /// \brief Represents a configuration
  ///
  struct Config;

  ///
  /// \brief Create a config with the given name
  /// \param 'configName' the new config's name
  /// \param 'returnedConfig' if the function succeeds, a pointer is written to a new 'struct Config'
  /// \return error code
  ///
  enum ReturnedValue createConfig(char const *configName, struct Config **returnedConfig);

  ///
  /// \brief get config from id
  /// \param 'id' the id of the requested config
  /// \param 'returnedConfig' if the function succeeds, a pointer is written to a new 'struct Config'
  /// \return error code
  ///
  enum ReturnedValue getConfig(struct Id const* id, struct Config **returnedConfig);

  ///
  /// \brief get const config from id
  /// \param 'id' the id of the requested config
  /// \param 'returnedConfig' if the function succeeds, a pointer is written to a new 'struct Config'
  /// \return error code
  ///
  enum ReturnedValue getReadOnlyConfig(struct Id const* id, struct Config const **returnedConfig);

  ///
  /// \brief release the config, freeing the underlying memory
  /// \param the config to release
  ///
  /// Must be called when config is no longer used.
  ///
  void releaseConfig(struct Config const *);

  ///
  /// \brief get the id of the given config
  /// \param the config
  /// \param 'configId' if the function succeeds, contain the config id
  /// \return error code
  ///
  enum ReturnedValue getConfigId(struct Config const *, struct Id *configId);

  ///
  /// \brief basic operation to add a new setting or modify an existing one
  /// \param the config to add a setting to
  /// \param 'name' setting name
  /// \param 'value' new setting value
  /// \return error code
  ///
  enum ReturnedValue setSetting(struct Config*, char const *name, char const *value);

  ///
  /// \brief set or modify an alias
  /// \param the config to add a setting alias to
  /// \param 'name' setting name
  /// \param 'aliasName' alias name
  /// \return error code
  ///
  enum ReturnedValue setSettingAlias(struct Config*, char const *name, char const *aliasName);

  ///
  /// \brief unset the given alias
  /// \param the config to remove a setting alias from
  /// \param 'aliasName' alias name
  /// \return error code
  ///
  enum ReturnedValue unsetAlias(struct Config*, char const *aliasName);

  ///
  /// \brief remove the given setting
  /// \param the config to remove a setting from
  /// \param 'name' setting name
  /// \return error code
  ///
  enum ReturnedValue removeSetting(struct Config*, char const *name);

  ///
  /// \brief get the setting's value
  /// \param the config
  /// \param 'settingName' setting name
  /// \param 'value' setting value will be written here without exceeding 'valueSize' bytes
  /// \param 'valueSize' must contain the size of the buffer pointed by 'value'
  /// \return error code
  ///
  enum ReturnedValue getSettingValue(struct Config const*, char const *settingName, char *value, size_t valueSize);

  ///
  /// \brief get the size of the setting's value
  /// \param the config
  /// \param 'settingName' setting name
  /// \param 'size' size of the setting's value
  /// \return error code
  ///
  enum ReturnedValue getSettingSize(struct Config const*, char const *settingName, size_t *size);

  ///
  /// \brief inherit from another config
  /// \param the config to modify
  /// \param 'inheritFrom' the other config which will be included
  /// \return error code
  ///
  enum ReturnedValue include(struct Config *config, struct Config const *inheritFrom);

  ///
  /// \brief be notified when a setting change
  /// \param the config
  /// \param 'name' setting you want to watch
  /// \param 'data' point to userdata, which will be forwarded to the callback
  /// \param 'onChange' function pointer callback which will be called once for each setting change
  /// \param 'subscription' in case of success, a new 'struct Subscription' will be written
  /// \return error code
  ///
  /// To stop the subscription, `unsubsribe` must be called.
  ///
  enum ReturnedValue subscribeToSetting(struct Config*, char const *name, void *data, FCPTR_ON_CHANGE_NOTIFIER onChange, struct Subscription **subscription);

#ifdef __cplusplus
}
#endif
