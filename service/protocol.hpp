//
// Created by milerius on 20/10/18.
//

#pragma once

#include <string>
#include <json.hpp>
#include "service_strong_types.hpp"

namespace raven
{
  enum class request_state : short
  {
    success,
    bad_order,
    unknown_request,
    internal_error,
    unauthorized,
    unknown_id,
    unknown_key,
    unknown_setting,
    unknown_alias,
    db_error = -1
  };

  inline const std::unordered_map<request_state, std::string> convert_request_state
      {
          {request_state::success,         "SUCCESS"},
          {request_state::bad_order,       "BAD_ORDER"},
          {request_state::unknown_request, "UNKNOWN_REQUEST"},
          {request_state::internal_error,  "INTERNAL_ERROR"},
          {request_state::unauthorized,    "UNAUTHORIZED"},
          {request_state::unknown_id,      "UNKNOWN_ID"},
          {request_state::unknown_key,     "UNKNOWN_KEY"},
          {request_state::unknown_setting, "UNKNOWN_SETTING"},
          {request_state::unknown_alias,   "UNKNOWN_ALIAS"},
          {request_state::db_error,        "DB_ERROR"},
      };

  namespace json = nlohmann;

  //! Keywords
  inline constexpr const char request_keyword[] = "REQUEST_NAME";
  inline constexpr const char request_state_keyword[] = "REQUEST_STATE";

  //! Protocol Constants
  inline constexpr const char config_name_keyword[] = "CONFIG_NAME";
  inline constexpr const char config_key_keyword[] = "CONFIG_KEY";
  inline constexpr const char config_read_only_key_keyword[] = "READONLY_CONFIG_KEY";
  inline constexpr const char config_id_keyword[] = "CONFIG_ID";
  inline constexpr const char config_include_src[] = "SRC";
  inline constexpr const char setting_name[] = "SETTING_NAME";
  inline constexpr const char settings_to_update_keyword[] = "SETTINGS_TO_UPDATE";
  //inline constexpr const char setting_value[] = "SETTING_VALUE";
  inline constexpr const char alias_name[] = "ALIAS_NAME";
  inline constexpr const char sub_event_type[] = "SUBSCRIBE_EVENT_TYPE";

  //! CONFIG_CREATE
  struct config_create
  {
    std::string config_name;
  };

  inline void from_json(const raven::json::json &json_data, config_create &cfg)
  {
      cfg.config_name = json_data.at(config_name_keyword).get<std::string>();
  }

  //! CONFIG_CREATE ANSWER
  struct config_create_answer
  {
    config_key_st config_key;
    config_key_st readonly_config_key;
    std::string request_state;
  };

  void to_json(raven::json::json &json_data, const config_create_answer &cfg)
  {
      json_data = {{"CONFIG_KEY",          cfg.config_key.value()},
                   {"READONLY_CONFIG_KEY", cfg.readonly_config_key.value()},
                   {"REQUEST_STATE",       cfg.request_state}};
  }

  //! CONFIG_LOAD
  struct config_load
  {
    std::optional<config_key_st> config_key{std::nullopt};
    std::optional<config_key_st> config_read_only_key{std::nullopt};
  };

  inline void from_json(const raven::json::json &json_data, config_load &cfg)
  {
      //! We are checking if the config key keyword is present, otherwise it's probably a read only value
      if (json_data.count(config_key_keyword) > 0 && json_data.at(config_key_keyword).is_string()) {
          cfg.config_key = config_key_st{json_data.at(config_key_keyword).get<std::string>()};
      } else if (json_data.count(config_read_only_key_keyword) > 0 && json_data.at(config_read_only_key_keyword).is_string()) {
          cfg.config_read_only_key = config_key_st{json_data.at(config_read_only_key_keyword).get<std::string>()};
      }
  }

  //! CONFIG_LOAD ANSWER
  struct config_load_answer
  {
    std::string config_name;
    config_id_st config_id;
    std::string request_state;
  };

  void to_json(raven::json::json &json_data, const config_load_answer &cfg)
  {
      json_data = {{"CONFIG_NAME",   cfg.config_name},
                   {"CONFIG_ID",     cfg.config_id.value()},
                   {"REQUEST_STATE", cfg.request_state}};
  }

  //! CONFIG_UNLOAD
  struct config_unload
  {
    config_id_st id;
  };

  inline void from_json(const raven::json::json &json_data, config_unload &cfg)
  {
      cfg.id = config_id_st{json_data.at(config_id_keyword).get<std::size_t>()};
  }

  //! CONFIG_INCLUDE
  struct config_include
  {
    config_id_st id;
    config_id_st src_id;
  };

  inline void from_json(const raven::json::json &json_data, config_include &cfg)
  {
      cfg.id = config_id_st{json_data.at(config_id_keyword).get<std::size_t>()};
      cfg.src_id = config_id_st{json_data.at(config_include_src).get<std::size_t>()};
  }

  //! SETTING_UPDATE
  struct setting_update
  {
    config_id_st id;
    json::json settings_to_update{json::json::object()};
  };

  inline void from_json(const raven::json::json &json_data, setting_update &cfg)
  {
      cfg.id = config_id_st{json_data.at(config_id_keyword).get<std::size_t>()};
      cfg.settings_to_update = json_data.at(settings_to_update_keyword);
  }

  //! SETTING_REMOVE
  struct setting_remove
  {
    config_id_st id;
    std::string setting_name;
  };

  inline void from_json(const raven::json::json &json_data, setting_remove &cfg)
  {
      cfg.id = config_id_st{json_data.at(config_id_keyword).get<std::size_t>()};
      cfg.setting_name = json_data.at(setting_name).get<std::string>();
  }

  //! SETTING_GET
  struct setting_get
  {
    config_id_st id;
    std::string setting_name;
  };

  inline void from_json(const raven::json::json &json_data, setting_get &cfg)
  {
      cfg.id = config_id_st{json_data.at(config_id_keyword).get<std::size_t>()};
      cfg.setting_name = json_data.at(setting_name).get<std::string>();
  }

  //! SETTING_GET ANSWER
  struct setting_get_answer
  {
    std::string setting_value;
    std::string request_state;
  };

  void to_json(raven::json::json &json_data, const setting_get_answer &cfg)
  {
      json_data = {{"SETTING_VALUE", cfg.setting_value},
                   {"REQUEST_STATE", cfg.request_state}};
  }

  //! CONFIG_GET_SETTINGS_NAMES
  struct config_get_settings_names
  {
    config_id_st id;
  };

  inline void from_json(const raven::json::json &json_data, config_get_settings_names &cfg)
  {
      cfg.id = config_id_st{json_data.at(config_id_keyword).get<std::size_t>()};
  }

  //! CONFIG_GET_SETTINGS_NAMES_ANSwER
  struct config_get_settings_names_answer
  {
    json::json settings_name{json::json::array()};
    std::string request_state;
  };

  void to_json(raven::json::json &json_data, const config_get_settings_names_answer &cfg)
  {
      json_data = {{"SETTINGS_NAMES", cfg.settings_name},
                   {"REQUEST_STATE", cfg.request_state}};
  }

  //! CONFIG_GET_SETTINGS
  struct config_get_settings
  {
    config_id_st id;
  };

  inline void from_json(const raven::json::json &json_data, config_get_settings &cfg)
  {
      cfg.id = config_id_st{json_data.at(config_id_keyword).get<std::size_t>()};
  }

  //! CONFIG_GET_SETTINGS
  struct config_get_settings_answer
  {
    json::json settings{json::json::object()};
    std::string request_state;
  };

  void to_json(raven::json::json &json_data, const config_get_settings_answer &cfg)
  {
      json_data = {{"SETTINGS", cfg.settings},
                   {"REQUEST_STATE", cfg.request_state}};
  }


  //! ALIAS_SET
  struct alias_set
  {
    config_id_st id;
    std::string setting_name;
    std::string alias_name;
  };

  inline void from_json(const raven::json::json &json_data, alias_set &cfg)
  {
      cfg.id = config_id_st{json_data.at(config_id_keyword).get<std::size_t>()};
      cfg.setting_name = json_data.at(setting_name).get<std::string>();
      cfg.alias_name = json_data.at(alias_name).get<std::string>();
  }

  //! ALIAS_UNSET
  struct alias_unset
  {
    config_id_st id;
    std::string alias_name;
  };

  inline void from_json(const raven::json::json &json_data, alias_unset &cfg)
  {
      cfg.id = config_id_st{json_data.at(config_id_keyword).get<std::size_t>()};
      cfg.alias_name = json_data.at(alias_name).get<std::string>();
  }

  template <typename TSetting>
  void fill_subscription_struct(const raven::json::json &json_data, TSetting &&cfg)
  {
      cfg.id = config_id_st{json_data.at(config_id_keyword).get<std::size_t>()};
      if (json_data.count(alias_name) > 0) {
          cfg.alias_name = json_data.at(alias_name).get<std::string>();
      } else if (json_data.count(setting_name) > 0) {
          cfg.setting_name = json_data.at(setting_name).get<std::string>();
      }
  }

  //! SUBSCRIBE_SETTING
  struct setting_subscribe
  {
    config_id_st id;
    std::optional<std::string> setting_name{std::nullopt};
    std::optional<std::string> alias_name{std::nullopt};
  };

  inline void from_json(const raven::json::json &json_data, setting_subscribe &cfg)
  {
      fill_subscription_struct<setting_subscribe>(json_data, std::forward<setting_subscribe>(cfg));
  }

  //! UNSUBSCRIBE_SETTING
  struct setting_unsubscribe
  {
    config_id_st id;
    std::optional<std::string> setting_name{std::nullopt};
    std::optional<std::string> alias_name{std::nullopt};
  };

  inline void from_json(const raven::json::json &json_data, setting_unsubscribe &cfg)
  {
      fill_subscription_struct<setting_unsubscribe>(json_data, std::forward<setting_unsubscribe>(cfg));
  }

  enum class subscribe_event_type : short
  {
    update_setting,
    delete_setting
  };

  //! SUBSCRIBE_EVENT
  struct subscribe_event
  {
    config_id_st id;
    std::string setting_name;
    subscribe_event_type type;
  };

  inline void from_json(const raven::json::json &json_data, subscribe_event &cfg)
  {
      cfg.id = config_id_st{json_data.at(config_id_keyword).get<std::uint32_t>()};
      cfg.setting_name = json_data.at(setting_name).get<std::string>();
      if (json_data.at(sub_event_type) == "UPDATE")
          cfg.type = subscribe_event_type::update_setting;
      else
          cfg.type = subscribe_event_type::delete_setting;
  }

  void to_json(raven::json::json &json_data, const subscribe_event &cfg)
  {
      std::string type = "DELETE";
      if (cfg.type == subscribe_event_type::update_setting)
          type = "UPDATE";
      json_data = {{"CONFIG_ID", cfg.id.value()},
                   {"SETTING_NAME", cfg.setting_name},
                   {"SUBSCRIPTION_EVENT_TYPE", type}};
  }
}
