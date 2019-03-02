//
// Created by milerius on 20/10/18.
//

#pragma once

#include <string>
#include "json.hpp"

namespace raven
{
  enum class request_state : unsigned short
  {
    success,
    bad_order,
    unknown_request,
    internal_error,
    unauthorized,
    unknown_id,
    unknown_key,
    unknown_setting,
    unknown_alias
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
      };

  namespace json = nlohmann;

  //! Keywords
  static inline constexpr const char *request_keyword = "REQUEST_NAME";

  //! Protocol Constants
  static inline constexpr const char *config_name_keyword = "CONFIG_NAME";
  static inline constexpr const char *config_key_keyword = "CONFIG_KEY";
  static inline constexpr const char *config_read_only_key_keyword = "READONLY_CONFIG_KEY";
  static inline constexpr const char *config_id = "CONFIG_ID";
  static inline constexpr const char *config_include_src = "SRC";
  static inline constexpr const char *config_include_dst = "DST";
  static inline constexpr const char *setting_name = "SETTING_NAME";
  static inline constexpr const char *setting_value = "SETTING_VALUE";
  static inline constexpr const char *alias_name = "ALIAS_NAME";

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
    std::string config_key;
    std::string readonly_config_key;
    std::string request_state;
  };

  void to_json(raven::json::json &json_data, const config_create_answer &cfg)
  {
      json_data = {{"CONFIG_KEY", cfg.config_key},
                   {"READONLY_CONFIG_KEY", cfg.readonly_config_key},
                   {"REQUEST_STATE", convert_request_state.at(request_state::success)}};
  }

  //! CONFIG_LOAD
  struct config_load
  {
    std::optional<std::string> config_key{std::nullopt};
    std::optional<std::string> config_read_only_key{std::nullopt};
  };

  inline void from_json(const raven::json::json &json_data, config_load &cfg)
  {
      //! We are checking if the config key keyword is present, otherwise it's probably a read only value
      if (json_data.count(config_key_keyword) > 0) {
          cfg.config_key = json_data.at(config_key_keyword).get<std::string>();
      } else if (json_data.count(config_read_only_key_keyword) > 0) {
          cfg.config_read_only_key = json_data.at(config_read_only_key_keyword).get<std::string>();
      }
  }

  //! CONFIG_LOAD ANSWER
  struct config_load_answer
  {
    std::string config_name;
    std::uint32_t config_id;
  };

  void to_json(raven::json::json &json_data, const config_load_answer &cfg)
  {
      json_data = {{"CONFIG_NAME", cfg.config_name},
                   {"CONFIG_ID",   cfg.config_id}};
  }

  //! CONFIG_UNLOAD
  struct config_unload
  {
    std::uint32_t id;
  };

  inline void from_json(const raven::json::json &json_data, config_unload &cfg)
  {
      cfg.id = json_data.at(config_id).get<std::uint32_t>();
  }

  //! CONFIG_INCLUDE
  struct config_include
  {
    std::uint32_t id;
    std::string src;
    std::string dst;
  };

  inline void from_json(const raven::json::json &json_data, config_include &cfg)
  {
      cfg.id = json_data.at(config_id).get<std::uint32_t>();
      cfg.src = json_data.at(config_include_src).get<std::string>();
      cfg.dst = json_data.at(config_include_dst).get<std::string>();
  }

  //! SETTING_UPDATE
  struct setting_update
  {
    std::uint32_t id;
    std::string setting_name;
    std::string setting_value;
  };

  inline void from_json(const raven::json::json &json_data, setting_update &cfg)
  {
      cfg.id = json_data.at(config_id).get<std::uint32_t>();
      cfg.setting_name = json_data.at(setting_name).get<std::string>();
      cfg.setting_value = json_data.at(setting_value).get<std::string>();
  }

  //! SETTING_REMOVE
  struct setting_remove
  {
    std::uint32_t id;
    std::string setting_name;
  };

  inline void from_json(const raven::json::json &json_data, setting_remove &cfg)
  {
      cfg.id = json_data.at(config_id).get<std::uint32_t>();
      cfg.setting_name = json_data.at(setting_name).get<std::string>();
  }

  //! SETTING_GET
  struct setting_get
  {
    std::uint32_t id;
    std::string setting_name;
  };

  inline void from_json(const raven::json::json &json_data, setting_get &cfg)
  {
      cfg.id = json_data.at(config_id).get<std::uint32_t>();
      cfg.setting_name = json_data.at(setting_name).get<std::string>();
  }

  //! SETTING_GET ANSWER
  struct setting_get_answer
  {
    std::string setting_value;
  };

  void to_json(raven::json::json &json_data, const setting_get_answer &cfg)
  {
      json_data = {{"SETTING_VALUE", cfg.setting_value}};
  }

  //! ALIAS_SET
  struct alias_set
  {
    std::uint32_t id;
    std::string setting_name;
    std::string alias_name;
  };

  inline void from_json(const raven::json::json &json_data, alias_set &cfg)
  {
      cfg.id = json_data.at(config_id).get<std::uint32_t>();
      cfg.setting_name = json_data.at(setting_name).get<std::string>();
      cfg.alias_name = json_data.at(alias_name).get<std::string>();
  }

  //! ALIAS_UNSET
  struct alias_unset
  {
    std::uint32_t id;
    std::string alias_name;
  };

  inline void from_json(const raven::json::json &json_data, alias_unset &cfg)
  {
      cfg.id = json_data.at(config_id).get<std::uint32_t>();
      cfg.alias_name = json_data.at(alias_name).get<std::string>();
  }

  template <typename TSetting>
  void fill_subscription_struct(const raven::json::json &json_data, TSetting &&cfg)
  {
      cfg.id = json_data.at(config_id).get<std::uint32_t>();
      if (json_data.count(alias_name) > 0) {
          cfg.alias_name = json_data.at(alias_name).get<std::string>();
      } else if (json_data.count(setting_name) > 0) {
          cfg.setting_name = json_data.at(setting_name).get<std::string>();
      }
  }

  //! SUBSCRIBE_SETTING
  struct setting_subscribe
  {
    std::uint32_t id;
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
    std::uint32_t id;
    std::optional<std::string> setting_name{std::nullopt};
    std::optional<std::string> alias_name{std::nullopt};
  };

  inline void from_json(const raven::json::json &json_data, setting_unsubscribe &cfg)
  {
      fill_subscription_struct<setting_unsubscribe>(json_data, std::forward<setting_unsubscribe>(cfg));
  }
}