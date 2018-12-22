//
// Created by milerius on 20/10/18.
//

#pragma once

#include <string>
#include "json.hpp"

namespace raven
{
  namespace json = nlohmann;

  //! Keywords
  static inline constexpr const char *request_keyword = "REQUEST_NAME";

  //! Protocol Constants
  static inline constexpr const char *config_name_keyword = "CONFIG_NAME";
  static inline constexpr const char *config_key_keyword = "CONFIG_KEY";
  static inline constexpr const char *config_read_only_key_keyword = "READONLY_CONFIG_KEY";
  static inline constexpr const char *config_id = "CONFIG_ID";

  //! CONFIG_CREATE
  struct config_create
  {
    std::string config_name;
  };

  inline void from_json(const raven::json::json &json_data, config_create &cfg)
  {
      cfg.config_name = json_data.at(config_name_keyword).get<std::string>();
  }

  //! CONFIG_LOAD
  struct config_load
  {
    std::optional<std::string> config_key{std::nullopt};
    std::optional<std::string> config_read_only_key{std::nullopt};
  };

  inline void from_json(const raven::json::json &json_data, config_load &cfg)
  {
      if (json_data.count(config_key_keyword) > 0) {
          cfg.config_key = json_data.at(config_key_keyword).get<std::string>();
      } else if (json_data.count(config_read_only_key_keyword) > 0) {
          cfg.config_read_only_key = json_data.at(config_read_only_key_keyword).get<std::string>();
      }
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
}
