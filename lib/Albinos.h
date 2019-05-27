//
// Must be included to use the Albinos
//
//

#pragma once

#ifdef C2NIM
#  dynlib albinosdll
#  cdecl
#  if defined(linux)
#    define albinosdll "libalbinos.so"
#  endif
#endif

# include <stddef.h>

#ifdef __cplusplus
namespace Albinos
{
  extern "C" {
#endif

    ///
    /// \brief contain all possible returned values for the API
    ///
    enum ReturnedValue
      {

       /// returned if error is not managed
       UNKNOWN,

       /// returned in case of success
       SUCCESS,

       /// returned if one of given parameters isn't good
       BAD_PARAMETERS,

       /// returned if socket file is not found
       SOCKET_NOT_FOUND,

       /// returned if an error occur when connecting
       CONNECTION_ERROR,

       /// returned by getReadOnlyConfigKey or getConfigKey if the requested key wasn't set up
       KEY_NOT_INITIALIZED,
      };

    ///
    /// \brief indicate the type of modification which happen to subscribed setting
    ///
    enum ModifType
      {
       UPDATE,
       DELETE,
      };

    ///
    /// \brief type of function pointer for setting change subscription
    ///
    typedef void (*FCPTR_ON_CHANGE_NOTIFIER)(struct Subscription const *, enum ModifType);

    ///
    /// \brief indicate the key type
    ///
    enum KeyType
      {
       READ_ONLY,
       READ_WRITE,
      };

    ///
    /// \brief contains a unique key for each configuration
    ///
    /// \param 'size' is the size of 'data' in bytes
    ///
    struct Key {
      char const *data;
      size_t size;
      enum KeyType type;
    };

    ///
    /// \brief define a struct representing a setting
    ///
    struct Setting {
      char *value;
      char *name;
    };

    ///
    /// \brief define a struct representing an alias
    ///
    struct Alias {
      char *alias;
      char *setting;
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
    /// \param the subscription to unsubscribe from. Must be successfully initialised with "subscribeToSetting"
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
    /// \brief Create a config with the given name. The created struct must be released using "releaseConfig"
    /// \param 'configName' the new config's name
    /// \param 'returnedConfig' if the function succeeds, a pointer is written to a new 'struct Config'
    /// \return error code
    ///
    enum ReturnedValue createConfig(char const *configName, struct Config **returnedConfig);

    ///
    /// \brief delete the given config
    /// \param the config to delete
    /// \return error code
    ///
    enum ReturnedValue destroyConfig(struct Config const*);

    ///
    /// \brief get config from a read/write Key. The created struct must be released using "releaseConfig"
    /// \param 'key' the Key of the requested config
    /// \param 'returnedConfig' if the function succeeds, a pointer is written to a new 'struct Config'
    /// \return error code
    ///
    enum ReturnedValue getConfig(struct Key key, struct Config **returnedConfig);

    ///
    /// \brief get const config from a read only key. The created struct must be released using "releaseConfig"
    /// \param 'key' the key of the requested config
    /// \param 'returnedConfig' if the function succeeds, a pointer is written to a new 'struct Config'
    /// \return error code
    ///
    enum ReturnedValue getReadOnlyConfig(struct Key key, struct Config const **returnedConfig);

    ///
    /// \brief release the config, freeing the underlying memory. Must be called when config is no longer used.
    /// \param the config to release
    ///
    void releaseConfig(struct Config const *);

    ///
    /// \brief get the Read/Write key of the given config
    /// \param the config
    /// \param 'configKey' if the function succeeds, contain the Read/Write config key
    /// \return error code
    ///
    enum ReturnedValue getConfigKey(struct Config const *, struct Key *configKey);

    ///
    /// \brief get the ReadOnly key of the given config
    /// \param the config
    /// \param 'configKey' if the function succeeds, contain the ReadOnly config key
    /// \return error code
    ///
    enum ReturnedValue getReadOnlyConfigKey(struct Config const *, struct Key *configKey);

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
    /// \brief get the ordered list of all dependencies
    /// \param the config
    /// \param 'deps' contain all dependencies.  Must be released using "destroyDependenciesArray".
    /// \param 'size' size of the 'deps' array
    /// \return error code
    ///
    enum ReturnedValue getDependencies(struct Config const*, struct Config **deps, size_t *size);

    ///
    /// \brief destroy a dependencies array obtained with 'getDependencies'
    /// \param 'deps' array to destroy
    /// \param 'size' size of the 'deps' array
    ///
    void destroyDependenciesArray(struct Config *deps, size_t size);

    ///
    /// \brief get the ordered list of all local settings
    /// \param the config
    /// \param 'settings' ordered list of all local settings. Must be released using "destroySettingsArray".
    /// \param 'size' size of the 'settings' array
    /// \return error code
    ///
    enum ReturnedValue getLocalSettings(struct Config const*, struct Setting **settings, size_t *size);

    ///
    /// \brief destroy a settings array obtained with 'getLocalSettings'
    /// \param 'settings' array to destroy
    /// \param 'size' size of the 'settings' array
    ///
    void destroySettingsArray(struct Setting *settings, size_t size);

    ///
    /// \brief get the ordered list of all local settings names.
    /// \param the config
    /// \param 'names' ordered list of all local settings names. NULL terminated. Must be released using "destroySettingsNamesArray".
    /// \return error code
    ///
    enum ReturnedValue getLocalSettingsNames(struct Config const*, char ***names);

    ///
    /// \brief destroy a settings names array obtained with 'getLocalSettingsNames'
    /// \param 'names' array to destroy
    ///
    void destroySettingsNamesArray(char **names);

    ///
    /// \brief get the ordered list of all local aliases
    /// \param the config
    /// \param 'aliases' ordered list of all local aliases.
    /// \param 'size' size of the 'aliases' array
    /// \return error code
    ///
    enum ReturnedValue getLocalAliases(struct Config const*, struct Alias **aliases, size_t *size);

    ///
    /// \brief destroy an aliases array obtained with 'getLocalAliases'
    /// \param 'aliases' array to destroy
    /// \param 'size' size of the 'aliases' array
    ///
    void destroyAliasesArray(struct Alias *aliases, size_t size);

    ///
    /// \brief inherit from another config. To remove an included config, use "uninclude"
    /// \param the config to modify
    /// \param 'inheritFrom' the READ_ONLY key of the config to include
    /// \param 'position' the position in the list of dependencies, where 0 is the first to be included. Negative values can be used, and then position will be *size* decreased by value.
    /// \return error code
    ///
    enum ReturnedValue include(struct Config *config, struct Key *inheritFrom, int position);

    ///
    /// \brief uninclude a config
    /// \param the config to modify
    /// \param 'otherConfig' the READ_ONLY key of the config to include. Can be null if 'position' must be used
    /// \param 'position' the position in the list of dependencies. Not used if 'otherConfig' isn't null.
    /// \return error code
    ///
    enum ReturnedValue uninclude(struct Config *config, struct Key *otherConfig, int position);

    ///
    /// \brief be notified when a setting change
    /// \param the config
    /// \param 'name' setting you want to watch
    /// \param 'data' point to userdata, which will be available in from the subscription in the callback
    /// \param 'onChange' function pointer callback which will be called once for each setting change
    /// \param 'subscription' in case of success, a new 'struct Subscription' will be written
    /// \return error code
    ///
    /// To stop the subscription, `unsubsribe` must be called.
    /// To get the subsrption's user data one can use `getSubscriptionUserData`
    /// To get the subsrption's setting name one can use `getSubscriptionSettingName`
    ///
    enum ReturnedValue subscribeToSetting(struct Config*, char const *name, void *data, FCPTR_ON_CHANGE_NOTIFIER onChange, struct Subscription **subscription);

    ///
    /// \brief call all callbacks for subscribed settings with updates
    /// \param the config
    /// \return error code
    ///
    enum ReturnedValue pollSubscriptions(struct Config*);

    /// Query functions:

    ///
    /// \brief get a subscription's user data
    /// \param 'subscription' the subscription to query the user data from
    /// \return the user data
    ///
    void *getSupscriptionUserData(struct Subscription const *);

    ///
    /// \brief get a subscription's setting name
    /// \param 'subscription' the subscription to query the setting name from
    /// \return the setting name
    ///
    char const *getSupscriptionSettingName(struct Subscription const *);


#ifdef __cplusplus
  }
}
#endif
