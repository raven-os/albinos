//
// Must be included to use the LibConfig
//
//

#pragma once

#ifdef _cplusplus
namespace LibConfig
{
#endif

  typedef void (*onChangeNotifier)(void *data, char const *newValue);

  /*
   * contain a unique id for each configuration
   */
  struct Id;
#ifndef _cplusplus
  typedef struct Id Id;
#endif

  /*
   * manage your subscriptions
   */
  struct Subscription;
#ifndef _cplusplus
  typedef struct Subscription Subscription;
#endif

  void unsubscribe(Subscription*);

  /*
   * Config class is used to manipulate configuration
   */
  struct Config;
#ifndef _cplusplus
  typedef struct Config Config;
#endif

  Id getConfigId(Config const *);

  void addSetting(Config*, char const *name, char const *value); // basic operation to add a new setting
  void addSettingAlias(Config*, char const *name, char const *aliasName); // set alias for given setting
  void unsetAlias(Config*, char const *aliasName);
  void removeSetting(Config*, char const *name);

  void include(Config *config, Config const *inheriftFrom); // inherit from another config

  Subscription subscribeToSetting(Config*, char const *name, void *data, onChangeNotifier onChange); // be notifier when a setting change

  Config createConfig(char const *configName); // create a config with the given name
  Config getConfig(Id id); // get config from id

#ifdef _cplusplus
}
#endif
