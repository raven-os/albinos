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

  typedef void (*FCPTR_ON_CHANGE_NOTIFIER)(void *data, char const *newValue);

  /*
   * contain a unique id for each configuration
   * 'size' is the size of 'data' in bytes
   */
  struct Id {
    void *data;
    size_t size;
  };

  /*
   * manage your subscriptions
   */
  struct Subscription;

  int unsubscribe(struct Subscription*);

  /*
   * Config class is used to manipulate configuration
   */
  struct Config;

  int createConfig(char const *configName, struct Config **returnedConfig); // create a config with the given name
  int getConfig(struct Id const* id, struct Config **returnedConfig); // get config from id
  int getReadOnlyConfig(struct Id const* id, struct Config const **returnedConfig); // get const config from id

  int getConfigId(struct Config const *, struct Id ** configId);

  int addSetting(struct Config*, char const *name, char const *value); // basic operation to add a new setting
  int addSettingAlias(struct Config*, char const *name, char const *aliasName); // set alias for given setting
  int unsetAlias(struct Config*, char const *aliasName);
  int removeSetting(struct Config*, char const *name);

  int getSettingValue(struct Config const*, char const *settingName, char **); //get the setting's value

  int include(struct Config *config, struct Config const *inheritFrom); // inherit from another config

  int subscribeToSetting(struct Config*, char const *name, void *data, FCPTR_ON_CHANGE_NOTIFIER onChange, struct Subscription **); // be notifier when a setting change

#ifdef _cplusplus
}
#endif
