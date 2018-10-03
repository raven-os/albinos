# Creating and retrieving a configuration

*For C++ developers, all functions and structs are in the "$LIB_NAME" namespace*

The first call to $LIB_NAME config is to create a new config giving it a name
```c

struct Config *awesomeConfig;

createConfig("My awesome config", &awesomeConfig);
```
or to retrieve it giving the previously stored id.

```c
// retrieve your config id by your storage
...

// get config back with retrieved id
getConfig(myConfigId, &awesomeConfig);
```
If your config is new, you can store its id allowing you to retrieve it later :
```c
struct Id *myConfigId;
getConfigId(awesomeConfig, &myConfigId);

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

If you have to retrieve a setting, you can simply get it.
```c
char *mySettingValue;

getSettingValue(awesomeConfig, "my setting name", &mySettingValue);
```

You can also subscribe to a setting to be notified on change.

```c
void beNotified(void *data, char const *newValue)
{
  //do some stuff
}

...

// I want to be notified when "my setting name" from 'awesomeConfig' is modified
struct Subscription *manageSub;
subscribeToSetting(awesomeConfig, "my setting name", &data, &beNotified, &manageSub);

...
// I don't need to be notified anymore
unsubscribe(manageSub);
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
