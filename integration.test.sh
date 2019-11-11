#!/usr/bin/env bash

config_key=""
config_id=""
function test_config_creation() {
    echo "testing config creation"
    echo `cat API_doc/json_recipes/config_create.json` | nc -U /tmp/raven-os_service_albinos.sock -q1 > res.json
    echo "api output: " `cat res.json`
    request_state=`jq -r '.REQUEST_STATE' res.json`
    config_key=`jq -r '.CONFIG_KEY' res.json`
    [[ "${request_state}" == "SUCCESS" ]] && echo "Success" || echo "Failure"
    rm res.json
}

function test_config_load() {
    echo "testing config loading"
    echo "{\"REQUEST_NAME\": \"CONFIG_LOAD\", \"CONFIG_KEY\": \"${config_key}\"}" | nc -U /tmp/raven-os_service_albinos.sock -q1 > res.json
    request_state=`jq -r '.REQUEST_STATE' res.json`
    config_id=`jq -r '.CONFIG_ID' res.json`
    echo ${config_id}
    echo "api output: " `cat res.json`
    [[ "${request_state}" == "SUCCESS" ]] && echo "Success" || echo "Failure"
    rm res.json
}

function test_config_unload() {
    echo "testing config unloading"
    echo "{\"REQUEST_NAME\": \"CONFIG_UNLOAD\", \"CONFIG_ID\": ${config_id}}" | nc -U /tmp/raven-os_service_albinos.sock -q1 > res.json
    request_state=`jq -r '.REQUEST_STATE' res.json`
    echo "api output: " `cat res.json`
    [[ "${request_state}" == "SUCCESS" ]] && echo "Success" || echo "Failure"
    rm res.json
}

test_config_creation
test_config_load
test_config_unload

