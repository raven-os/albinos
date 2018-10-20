//
// Created by milerius on 20/10/18.
//

#pragma once

#include <string>
#include "json.hpp"

namespace raven
{
    namespace json = nlohmann;

    struct config_create
    {
        const std::string order{"CONFIG_CREATE"};
        std::string config_name;
        std::string provider;
    };

    inline void from_json(const raven::json::json &json_data, config_create &cfg)
    {
      cfg.config_name = json_data.at("config").get<std::string>();
      cfg.provider = json_data.at("provider").get<std::string>();
    }
}




