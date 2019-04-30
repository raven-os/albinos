module protocol.protocol_type;

import asdf;

struct config_create
{
    @serializationKeys("REQUEST_NAME") string request_name;
    @serializationKeys("CONFIG_NAME") string config_name;
}

struct config_create_answer
{
    @serializationKeys("REQUEST_STATE") string state;
    @serializationKeys("CONFIG_KEY") string config_key;
    @serializationKeys("READONLY_CONFIG_KEY") string readonly_config_key;
}

struct config_load
{
    @serializationKeys("REQUEST_NAME") string request_name;
    @serializationKeys("CONFIG_KEY") string config_key;
    @serializationKeys("READONLY_CONFIG_KEY") string readonly_config_key;
}

struct config_load_answer
{
    @serializationKeys("REQUEST_STATE") string state;
    @serializationKeys("CONFIG_NAME") string config_name;
    @serializationKeys("CONFIG_ID") size_t config_id;
}

struct setting_get
{
    @serializationKeys("REQUEST_NAME") string request_name;
    @serializationKeys("CONFIG_ID") size_t config_id;
    @serializationKeys("SETTING_NAME") string setting_name;
}

struct setting_get_answer
{
    @serializationKeys("REQUEST_STATE") string state;
    @serializationKeys("SETTING_VALUE") string setting_value;
}

struct setting_update
{
    @serializationKeys("REQUEST_NAME") string request_name;
    @serializationKeys("CONFIG_ID") size_t config_id;
    @serializationKeys("SETTINGS_TO_UPDATE") string settings_to_update;
}

struct setting_update_answer
{
    @serializationKeys("REQUEST_STATE") string state;
}

struct setting_subscribe
{
    @serializationKeys("REQUEST_NAME") string request_name;
    @serializationKeys("CONFIG_ID") size_t config_id;
    @serializationKeys("SETTING_NAME") string setting_name;
}

struct setting_subscribe_answer
{
    @serializationKeys("REQUEST_STATE") string state;
}

struct setting_unsubscribe
{
    @serializationKeys("REQUEST_NAME") string request_name;
    @serializationKeys("CONFIG_ID") size_t config_id;
    @serializationKeys("SETTING_NAME") string setting_name;
}

struct setting_unsubscribe_answer
{
    @serializationKeys("REQUEST_STATE") string state;
}

struct subscribe_event
{
    @serializationKeys("CONFIG_ID") size_t config_id;
    @serializationKeys("SETTING_NAME") string setting_name;
    @serializationKeys("SUBSCRIBE_EVENT_TYPE") string event_type;
}
