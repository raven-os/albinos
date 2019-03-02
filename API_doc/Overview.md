# Creating and retrieving a configuration

*For C++ developers, all functions and structs are in the "Albinos" namespace*

The first call to Albinos config is to create a new config giving it a name
```c

struct Config *awesomeConfig;

createConfig("My awesome config", &awesomeConfig);
```
or to retrieve it giving the previously stored id.

```c
// get config from an id
getConfig(myConfigId, &awesomeConfig);
```
If the config is new, one can store it's id allowing it to be retrieve it later
```c
struct Id *myConfigId;
getConfigId(awesomeConfig, &myConfigId);

// id's have to be stored by the application
myStorageFunc(myConfigId);
```

To free the underlyong memory, call

```c
releaseConfig(awesomeConfig);
```

Using the above functions it's possible for an application to persistently use one configuration once it has been created.

# Modifying a configuration

A configuration is represented by a series of instructions.
The most basic instruction is a setting, which can alias another setting.
It's also possible to unset an alias or remove an existing setting.
If a setting is already set, it is overridden.
```c
struct Config *awesomeConfig;
createConfig("My awesome config", &awesomeConfig);

// add a setting
setSetting(awesomeConfig, "my setting name", "my setting value");

// make an alias for the previously defined setting
setSettingAlias(awesomeConfig, "my setting name", "new alias");

// unset a defined alias
unsetAlias(awesomeConfig, "new alias");

// remove a setting
removeSetting(awesomeConfig, "my setting name");
```

It is also possible to apply all settings from another `Config`

```c
struct Config *awesomeConfig;

createConfig("My awesome config", &awesomeConfig);
include(awesomeConfig, anotherConfig);
```

A freshly created config only imports the global configuration of the OS.

# Read and look after a setting

To get a setting's value, one must subscribe to it.

```c
void beNotified(void *data, char const *newValue)
{
  //do some stuff
}

...

// be notified when "my setting name" from 'awesomeConfig' is modified
struct Subscription *manageSub;
subscribeToSetting(awesomeConfig, "my setting name", &data, &beNotified, &manageSub);

...
// stop being notified
unsubscribe(manageSub);
```

Some settings are rarely used (such as an application's default template), in that case it is possible to read a setting without subscribing to it:
```c
char *mySettingValue;
size_t size;

// get the size
getSettingSize(awesomeConfig, "my setting name", &size);

if ((mySettingValue = malloc(size * sizeof(char))))
  getSettingValue(awesomeConfig, "my setting name", mySettingValue, size);
```

# Convenience functions (C++)

A convenient more C++ friendly templated function is provided to subscribe to a setting
```cpp
template<class Func>
Albinos::Subscription Albinos::Config::subscribeToSetting(char const *name, Func &&func);
```
# TODO:
- config removal / modification (currently one can only add, which is enough but may not be very expressive)
- config backup / undo
