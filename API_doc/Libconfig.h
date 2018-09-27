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

  typedef void (*onChangeNotifier)(void *data, char const *newValue);

  /*
   * contain a unique id for each configuration
   */
  struct Id {
    void *data;
    size_t size;
  };

  /*
   * manage your subscriptions
   */
  struct Subscription;

  void unsubscribe(struct Subscription*);

  /*
   * Config class is used to manipulate configuration
   */
  struct Config;

  struct Id getConfigId(struct Config const *);

  void addSetting(struct Config*, char const *name, char const *value); // basic operation to add a new setting
  void addSettingAlias(struct Config*, char const *name, char const *aliasName); // set alias for given setting
  void unsetAlias(struct Config*, char const *aliasName);
  void removeSetting(struct Config*, char const *name);

  void include(struct Config *config, struct Config const *inheriftFrom); // inherit from another config

  struct Subscription subscribeToSetting(struct Config*, char const *name, void *data, onChangeNotifier onChange); // be notifier when a setting change

  struct Config createConfig(char const *configName); // create a config with the given name
  struct Config getConfig(struct Id id); // get config from id

#ifdef _cplusplus
}
#endif
