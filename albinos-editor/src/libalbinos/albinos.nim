##
##  Must be included to use the Albinos
##
##

{.deadCodeElim: on.}
when defined(linux):
  const
    albinosdll* = "libalbinos.so"
when defined(MacOSX):
  const
    albinosdll* = "libalbinos.dylib"
## /
## / \brief contain all possible returned values for the API
## /

type
  ReturnedValue* {.size: sizeof(cint).} = enum ## / returned if error is not managed
    UNKNOWN,                  ## / returned in case of success
    SUCCESS,                  ## / returned if one of given parameters isn't good
    BAD_PARAMETERS,           ## / returned if socket file is not found
    SOCKET_NOT_FOUND,         ## / returned if an error occur when connecting
    CONNECTION_ERROR,         ## / returned by getReadOnlyConfigKey or getConfigKey if the requested key wasn't set up
    KEY_NOT_INITIALIZED


## /
## / \brief type of function pointer for setting change subscription
## /

type
  FCPTR_ON_CHANGE_NOTIFIER* = proc (data: pointer; newValue: cstring) {.cdecl.}

## /
## / \brief indicate the key type
## /

type
  KeyType* {.size: sizeof(cint).} = enum
    READ_ONLY, READ_WRITE


## /
## / \brief contains a unique key for each configuration
## /
## / \param 'size' is the size of 'data' in bytes
## /

type
  Key* {.bycopy.} = object
    data*: pointer
    size*: csize
    `type`*: KeyType


## /
## / \brief define a struct representing a setting
## /

type
  Setting* {.bycopy.} = object
    value*: cstring
    name*: cstring


## /
## / \brief define a struct representing an alias
## /

type
  Alias* {.bycopy.} = object
    alias*: cstring
    setting*: cstring


## /
## /
## / CONFIG
## /
## /
## /
## / \brief Represents a configuration
## /

type
  Config* {.bycopy.} = object

## /
## /
## / SUBSCRIPTION
## /
## /
## /
## / \brief represents a subscription
## /

type
  Subscription* {.bycopy.} = object


## /
## / \brief unsubscribe
## / \param the subscription to unsubscribe from. Must be successfully initialised with "subscribeToSetting"
## /

proc unsubscribe*(a1: ptr Subscription) {.cdecl, importc: "unsubscribe",
                                      dynlib: albinosdll.}
## /
## / \brief be notified when a setting change. Cancel the subscription using "unsubscribe"
## / \param the config
## / \param 'name' setting you want to watch
## / \param 'data' point to userdata, which will be forwarded to the callback
## / \param 'onChange' function pointer callback which will be called once for each setting change
## / \param 'subscription' in case of success, a new 'struct Subscription' will be written
## / \return error code
## /
## / To stop the subscription, `unsubsribe` must be called.
## /

proc subscribeToSetting*(a1: ptr Config; name: cstring; data: pointer;
                        onChange: FCPTR_ON_CHANGE_NOTIFIER;
                        subscription: ptr ptr Subscription): ReturnedValue {.cdecl,
    importc: "subscribeToSetting", dynlib: albinosdll.}

## /
## / \brief Create a config with the given name. The created struct must be released using "releaseConfig"
## / \param 'configName' the new config's name
## / \param 'returnedConfig' if the function succeeds, a pointer is written to a new 'struct Config'
## / \return error code
## /

proc createConfig*(configName: cstring; returnedConfig: ptr ptr Config): ReturnedValue {.
    cdecl, importc: "createConfig", dynlib: albinosdll.}
## /
## / \brief delete the given config
## / \param the config to delete
## / \return error code
## /

proc destroyConfig*(a1: ptr Config): ReturnedValue {.cdecl, importc: "destroyConfig",
    dynlib: albinosdll.}
## /
## / \brief get config from a read/write Key. The created struct must be released using "releaseConfig"
## / \param 'key' the Key of the requested config
## / \param 'returnedConfig' if the function succeeds, a pointer is written to a new 'struct Config'
## / \return error code
## /

proc getConfig*(key: Key; returnedConfig: ptr ptr Config): ReturnedValue {.cdecl,
    importc: "getConfig", dynlib: albinosdll.}
## /
## / \brief get const config from a read only key. The created struct must be released using "releaseConfig"
## / \param 'key' the key of the requested config
## / \param 'returnedConfig' if the function succeeds, a pointer is written to a new 'struct Config'
## / \return error code
## /

proc getReadOnlyConfig*(key: Key; returnedConfig: ptr ptr Config): ReturnedValue {.
    cdecl, importc: "getReadOnlyConfig", dynlib: albinosdll.}
## /
## / \brief release the config, freeing the underlying memory. Must be called when config is no longer used.
## / \param the config to release
## /

proc releaseConfig*(a1: ptr Config) {.cdecl, importc: "releaseConfig",
                                  dynlib: albinosdll.}
## /
## / \brief get the Read/Write key of the given config
## / \param the config
## / \param 'configKey' if the function succeeds, contain the Read/Write config key
## / \return error code
## /

proc getConfigKey*(a1: ptr Config; configKey: ptr Key): ReturnedValue {.cdecl,
    importc: "getConfigKey", dynlib: albinosdll.}
## /
## / \brief get the ReadOnly key of the given config
## / \param the config
## / \param 'configKey' if the function succeeds, contain the ReadOnly config key
## / \return error code
## /

proc getReadOnlyConfigKey*(a1: ptr Config; configKey: ptr Key): ReturnedValue {.cdecl,
    importc: "getReadOnlyConfigKey", dynlib: albinosdll.}
## /
## / \brief basic operation to add a new setting or modify an existing one
## / \param the config to add a setting to
## / \param 'name' setting name
## / \param 'value' new setting value
## / \return error code
## /

proc setSetting*(a1: ptr Config; name: cstring; value: cstring): ReturnedValue {.cdecl,
    importc: "setSetting", dynlib: albinosdll.}
## /
## / \brief set or modify an alias
## / \param the config to add a setting alias to
## / \param 'name' setting name
## / \param 'aliasName' alias name
## / \return error code
## /

proc setSettingAlias*(a1: ptr Config; name: cstring; aliasName: cstring): ReturnedValue {.
    cdecl, importc: "setSettingAlias", dynlib: albinosdll.}
## /
## / \brief unset the given alias
## / \param the config to remove a setting alias from
## / \param 'aliasName' alias name
## / \return error code
## /

proc unsetAlias*(a1: ptr Config; aliasName: cstring): ReturnedValue {.cdecl,
    importc: "unsetAlias", dynlib: albinosdll.}
## /
## / \brief remove the given setting
## / \param the config to remove a setting from
## / \param 'name' setting name
## / \return error code
## /

proc removeSetting*(a1: ptr Config; name: cstring): ReturnedValue {.cdecl,
    importc: "removeSetting", dynlib: albinosdll.}
## /
## / \brief get the setting's value
## / \param the config
## / \param 'settingName' setting name
## / \param 'value' setting value will be written here without exceeding 'valueSize' bytes
## / \param 'valueSize' must contain the size of the buffer pointed by 'value'
## / \return error code
## /

proc getSettingValue*(a1: ptr Config; settingName: cstring; value: cstring;
                     valueSize: csize): ReturnedValue {.cdecl,
    importc: "getSettingValue", dynlib: albinosdll.}
## /
## / \brief get the size of the setting's value
## / \param the config
## / \param 'settingName' setting name
## / \param 'size' size of the setting's value
## / \return error code
## /

proc getSettingSize*(a1: ptr Config; settingName: cstring; size: ptr csize): ReturnedValue {.
    cdecl, importc: "getSettingSize", dynlib: albinosdll.}
## /
## / \brief get the ordered list of all dependencies
## / \param the config
## / \param 'deps' contain all dependencies.  Must be released using "destroyDependenciesArray".
## / \param 'size' size of the 'deps' array
## / \return error code
## /

proc getDependencies*(a1: ptr Config; deps: ptr ptr Config; size: ptr csize): ReturnedValue {.
    cdecl, importc: "getDependencies", dynlib: albinosdll.}
## /
## / \brief destroy a dependencies array obtained with 'getDependencies'
## / \param 'deps' array to destroy
## / \param 'size' size of the 'deps' array
## /

proc destroyDependenciesArray*(deps: ptr Config; size: csize) {.cdecl,
    importc: "destroyDependenciesArray", dynlib: albinosdll.}
## /
## / \brief get the ordered list of all local settings
## / \param the config
## / \param 'settings' ordered list of all local settings. Must be released using "destroySettingsArray".
## / \param 'size' size of the 'settings' array
## / \return error code
## /

proc getLocalSettings*(a1: ptr Config; settings: ptr ptr Setting; size: ptr csize): ReturnedValue {.
    cdecl, importc: "getLocalSettings", dynlib: albinosdll.}
## /
## / \brief destroy a settings array obtained with 'getLocalSettings'
## / \param 'settings' array to destroy
## / \param 'size' size of the 'settings' array
## /

proc destroySettingsArray*(settings: ptr Setting; size: csize) {.cdecl,
    importc: "destroySettingsArray", dynlib: albinosdll.}
## /
## / \brief get the ordered list of all local settings names.
## / \param the config
## / \param 'names' ordered list of all local settings names. NULL terminated. Must be released using "destroySettingsNamesArray".
## / \return error code
## /

proc getLocalSettingsNames*(a1: ptr Config; names: ptr cstringArray): ReturnedValue {.
    cdecl, importc: "getLocalSettingsNames", dynlib: albinosdll.}
## /
## / \brief destroy a settings names array obtained with 'getLocalSettingsNames'
## / \param 'names' array to destroy
## /

proc destroySettingsNamesArray*(names: cstringArray) {.cdecl,
    importc: "destroySettingsNamesArray", dynlib: albinosdll.}
## /
## / \brief get the ordered list of all local aliases
## / \param the config
## / \param 'aliases' ordered list of all local aliases.
## / \param 'size' size of the 'aliases' array
## / \return error code
## /

proc getLocalAliases*(a1: ptr Config; aliases: ptr ptr Alias; size: ptr csize): ReturnedValue {.
    cdecl, importc: "getLocalAliases", dynlib: albinosdll.}
## /
## / \brief destroy an aliases array obtained with 'getLocalAliases'
## / \param 'aliases' array to destroy
## / \param 'size' size of the 'aliases' array
## /

proc destroyAliasesArray*(aliases: ptr Alias; size: csize) {.cdecl,
    importc: "destroyAliasesArray", dynlib: albinosdll.}
## /
## / \brief inherit from another config. To remove an included config, use "uninclude"
## / \param the config to modify
## / \param 'inheritFrom' the READ_ONLY key of the config to include
## / \param 'position' the position in the list of dependencies, where 0 is the first to be included. Negative values can be used, and then position will be *size* decreased by value.
## / \return error code
## /

proc `include`*(config: ptr Config; inheritFrom: ptr Key; position: cint): ReturnedValue {.
    cdecl, importc: "include", dynlib: albinosdll.}
## /
## / \brief uninclude a config
## / \param the config to modify
## / \param 'otherConfig' the READ_ONLY key of the config to include. Can be null if 'position' must be used
## / \param 'position' the position in the list of dependencies. Not used if 'otherConfig' isn't null.
## / \return error code
## /

proc uninclude*(config: ptr Config; otherConfig: ptr Key; position: cint): ReturnedValue {.
    cdecl, importc: "uninclude", dynlib: albinosdll.}