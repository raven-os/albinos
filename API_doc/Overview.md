# Creating and retrieving a configuration

*For C++ developers, all functions and structs are in the "$LIB_NAME" namespace*

The first call to $LIB_NAME config is to create a new config giving it a name
```c
struct Config awesomeConfig = createConfig("My awesome config");
```
or to retrieve it giving the previously stored id.

```c
// retrieve your config id by your storage
...

// get config back with retrieved id
struct Config awesomeConfig = getConfig(myConfigId);
```
If your config is new, you can store its id allowing you to retrieve it later :
```c
struct Id myConfigId = getConfigId(&awesomeConfig);

// you have to keep the id by your side
myStorageFunc(myConfigId);
```

Using the above functions it's possible for an application to persistently use one configuration once it has been created.

# Modifying a configuration

A configuration is represented by a series of instructions.
The most basic instruction is a setting, which can alias another setting.
You can also unset an alias or remove an existing setting.
If a setting is already set, it is overridden.
```c
struct Config awesomeConfig = createConfig("My awesome config");

// add a setting
addSetting(&awesomeConfig, "my setting name", "my setting value");

// make an alias for the previously defined setting
addSettingAlias(&awesomeConfig, "my setting name", "new alias");

// unset a defined alias
unsetAlias(&awesomeConfig, "new alias");

// remove a setting
removeSetting(&awesomeConfig, "my setting name");
```

It is also possible to apply all settings from another `Config`

```c
struct Config awesomeConfig = createConfig("My awesome config");

include(&awesomeConfig, anotherConfig);
```

A freshly created config only imports the global configuration of the OS.

# Subscribing to a setting

```c
void beNotified(void *data, char const *newValue)
{
  //do some stuff
}

...

// I want to be notified when "my setting name" from 'awesomeConfig' is modified
struct Subscription manageSub = subscribeToSetting(&awesomeConfig, "my setting name", &data, &beNotified);

...
// I don't need to be notified anymore
unsubscribe(&manageSub);
```

# Convenience functions (C++)

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
    $LIB_NAME::addSetting(&config, "Music directory", "."); // default to current directory
    $LIB_NAME::subscribeToSetting(&config, "Music specific config", nullptr,
                              [&musicConfigKeyPath](void*, char const * newValue)
                              {
                                musicConfigKeyPath = newValue;
                              });
    $LIB_NAME::include(&config, $LIB_NAME::loadConfig(readConfigIdFromFile(musicConfigKeyPath)));
    store(id); // store the id to be able to retrieve this config in the future;
  }
  subscribeToSetting(&config, "Music volume", volumeHandler);
  subscribeToSetting(&config, "Music directory", directoryHandler);
}
```
In this example, `volumeHandler` will recieve all config updates from the included config, unless the included config has no "Music volume" in which case it will recieve modifications from the global config. If this last one does not include such a setting either, then `volumeHandler` will never be called.

`directoryHandler` will recieve all config updates from the included config, unless the included config has no "Music directory" in which case it will recieve '.', which is the setting set before it's inclusion.
