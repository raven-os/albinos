# Communication lib-service

All requests must contain **REQUEST_NAME**, containing the type of action they want to do.
Each response contain at least **REQUEST_STATE** (see below).

### Requests
| Request Name | Request Description | Additional Argument(s) | Additional Returned Value(s) |
| -------- | -------- | -------- | -------- |
|*CONFIG_CREATE*|Create a new config.|**CONFIG_NAME**|**CONFIG_ID**<br>**READONLY_CONFIG_ID**|
|*CONFIG_LOAD*| Load an existing config. |**CONFIG_ID** *or* **READONLY_CONFIG_ID**| **CONFIG_NAME**<br>**REQUEST_ID**|
|*CONFIG_UNLOAD*| Unload config. |**REQUEST_ID**|*none*|
|*CONFIG_INCLUDE*| Include a config in another |**REQUEST_ID**<br>**SRC**<br>**DEST**|*none*|
|*SETTING_UPDATE*| Update or create setting |**REQUEST_ID**<br>**SETTING_NAME**<br>**SETTING_VALUE**|*none*|
|*SETTING_REMOVE*| Remove setting |**REQUEST_ID**<br>**SETTING_NAME**|*none*|
|*SETTING_GET*| Get setting |**REQUEST_ID**<br>**SETTING_NAME**|**SETTING_VALUE**|
|*ALIAS_SET*| Update or create alias |**REQUEST_ID**<br>**SETTING_NAME**<br>**ALIAS_NAME**|*none*|
|*ALIAS_UNSET*| Unset alias |**REQUEST_ID**<br>**ALIAS_NAME**|*none*|

### REQUEST_STATE


| Value | Meaning |
| -------- | -------- |
| SUCCESS | Everything ok |
| BAD_ORDER | Argument(s) missing or bad |
| UNKNOWN_REQUEST | Bad request name |
| INTERNAL_ERROR | The service got an error |
| UNKNOWN_CONFIG | Given config id don't exist|
