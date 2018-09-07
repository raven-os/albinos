# Creating and retrieving a configuration

The first call $LIB_NAME should be to the following function:
```cpp
$LIB_NAME::Config $LIB_NAME::createConfig(char const *configName);
```
`configName` the name given to the configuration.

```cpp
struct $LIB_NAME::Id
{
  void *data;
  size_t size; // size of data
};

$LIB_NAME::Id $LIB_NAME::Config::getId();
$LIB_NAME::Config $LIB_NAME::getConfig($LIB_NAME::Id id);
```

Using the above functions it's possible for an application to persistently use one configuration once it has been created.

# Modifying a configuration

A configuration is represented by a series of instructions.
The most basic instruction is a setting, which can alias another setting.
You can also unset an alias or remove an existing setting.
If a setting is already set, it is overridden.
```cpp
void $LIB_NAME::Config::addSetting(char const *name, char const *value);
void $LIB_NAME::Config::addSettingAlias(char const *name, char const *aliasName);
void $LIB_NAME::Config::unsetAlias(char const *aliasName);
void $LIB_NAME::Config::removeSetting(char const *name);
```

It is also possible to apply all settings from another `$LIB_NAME::Config`

```cpp
void $LIB_NAME::Config::include($LIB_NAME::Config config);
```

A freshly created config only imports the global configuration of the OS.

# Subscribing to a setting

Retrieving a value is simply done calling this function:
```cpp
$LIB_NAME::Subscription $LIB_NAME::Config::subscribeToSetting(char const *name, void *data, void (*onChange)(void *data, char const *newValue));
void $LIB_NAME::Subscription::unsubscribe();
```
`onChange` will imediatly be called with the current setting's value, and will called each time the value of the setting changes.
`data` can be used to pass user data.

# Convenience functions

A convenient more C++ friendly templated function is provided to subscribe to a setting
```cpp
template<class Func>
$LIB_NAME::Subscription $LIB_NAME::Config::subscribeToSetting(char const *name, Func &&func);
```
# TODO:
- config removal / modification (currently one can only add, which is enough but may not be very expressive)
- config backup / undo

# Example
```cpp
void loadConfig(XXX &&volumeHandler)
{
  $LIB_NAME::Id id{nullptr, 0};
  $LIB_NAME::Config config;
  if (!loadKey(&id))
  {
    config = $LIB_NAME::createConfig("My super music app");
    // since config automaticly inherits from the global config, we can go ahead and retreive another config
    char const *musicConfigKeyPath = nullptr;
    config.subscribeToSetting("Music specific config",
                              [&musicConfigKeyPath](char const * newValue)
                              {
                                musicConfigKeyPath = newValue;
                              }).unsubscribe();
    config.includeConfig($LIB_NAME::loadConfig(readConfigIdFromFile(musicConfigKeyPath)));
    store(id); // store the id to be able to retrieve this config in the future;
  }
  config.subscribeToSetting("Music volume", volumeHandler);
}
```
