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
}