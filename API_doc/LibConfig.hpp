#pragma once

# include "APILibConfig.h"

namespace LibConfig
{

  /*
   * manage your subscriptions
   */
  struct Subscription
  {
  private:
    onChangeNotifier subscribedFunc;
  public:

    Subscription(onChangeNotifier);

    void unsubscribe(); // stop your subscription
  };

  /*
   * Config class is used to manipulate configuration
   */
  struct Config
  {
  private:
    Id configId;

  public:

    Id getId() const;

    bool addSetting(char const *name, char const *value); // basic operation to add a new setting
    bool addSettingAlias(char const *name, char const *aliasName); // set alias for given setting
    void unsetAlias(char const *aliasName);
    void removeSetting(char const *name);

    void include(Config const &config); // inherit from another config

    Subscription subscribeToSetting(char const *name, void *data, onChangeNotifier onChange); // be notifier when a setting change
  };
}
