# Communication lib-service

All orders must contain at least **ORDER**, containing the name of the order, and **CONFIG_ID**, containing the id of the config you want to work on (except for *CONFIG_CREATE*).
Each response must contain at least **ORDER_STATE** (see below).

### Orders
| Order Name | Order Description | Additional Argument(s) | Additional Returned Value(s) |
| -------- | -------- | -------- | -------- |
|*CONFIG_CREATE*|Create a new config.|**CONFIG_NAME**|**CONFIG_ID**|
|*CONFIG_LOAD*| Load an existing config. |*none*| **CONFIG_NAME**|
|*CONFIG_UNLOAD*| Unload config. |*none*|*none*|
|*CONFIG_INCLUDE*| Include a config in another |**SRC**<br>**DEST**|*none*|
|*SETTING_UPDATE*| Update or create setting |**SETTING_NAME**<br>**SETTING_VALUE**|*none*|
|*SETTING_REMOVE*| Remove setting |**SETTING_NAME**|*none*|
|*SETTING_GET*| Get setting |**SETTING_NAME**|**SETTING_VALUE**|
|*ALIAS_SET*| Update or create alias |**SETTING_NAME**<br>**ALIAS_NAME**|*none*|
|*ALIAS_UNSET*| Unset alias |**ALIAS_NAME**|*none*|

### ORDER_STATE


| Value | Meaning |
| -------- | -------- |
| SUCCESS | Everything ok |
| BAD_ORDER | Argument(s) missing or bad |
| UNKNOWN_ORDER | Bad order name |
| INTERNAL_ERROR | The service got an error |
| UNKNOWN_CONFIG | Given config id don't exist|