# Communication lib-service

All requests must contain **REQUEST_NAME**, containing the type of action they want to do.
Each response contain at least **REQUEST_STATE** (see below).

### Requests
| Request Name | Request Description | Additional Argument(s) | Additional Returned Value(s) |
| -------- | -------- | -------- | -------- |
|*CONFIG_CREATE*|Create a new config.|**CONFIG_NAME**|**CONFIG_KEY**<br>**READONLY_CONFIG_KEY**|
|*CONFIG_LOAD*| Load an existing config. |**CONFIG_KEY** *or* **READONLY_CONFIG_KEY**| **CONFIG_NAME**<br>**CONFIG_ID**|
|*CONFIG_UNLOAD*| Unload config. |**CONFIG_ID**|*none*|
|*CONFIG_INCLUDE*| Include a config in another |**CONFIG_ID**<br>**SRC**<br>**DEST**|*none*|
|*SETTING_UPDATE*| Update or create setting |**CONFIG_ID**<br>**SETTING_NAME**<br>**SETTING_VALUE**|*none*|
|*SETTING_REMOVE*| Remove setting |**CONFIG_ID**<br>**SETTING_NAME**|*none*|
|*SETTING_GET*| Get setting |**CONFIG_ID**<br>**SETTING_NAME**|**SETTING_VALUE**|
|*ALIAS_SET*| Update or create alias |**CONFIG_ID**<br>**SETTING_NAME**<br>**ALIAS_NAME**|*none*|
|*ALIAS_UNSET*| Unset alias |**CONFIG_ID**<br>**ALIAS_NAME**|*none*|

### REQUEST_STATE


| Value | Meaning |
| -------- | -------- |
| SUCCESS | Everything ok |
| BAD_ORDER | Argument(s) missing or bad |
| UNKNOWN_REQUEST | Bad request name |
| INTERNAL_ERROR | The service got an error |
| UNKNOWN_ID | Given config id don't exist|
| UNKNOWN_KEY | Given config key don't exist|
