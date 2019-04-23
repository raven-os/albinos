//
// Created by milerius on 15/03/19.
//

#pragma once

#include <unordered_map>
#include "service_strong_types.hpp"

namespace raven
{
  class client
  {
  private:
    using client_ptr = std::shared_ptr<uvw::PipeHandle>;
  public:
    client(client_ptr sock) noexcept : sock_(sock)
    {
    };

    client() = delete;

    client &operator+=(raven::config_id_st db_id)
    {
        last_id++;
        config_ids_.insert({{last_id.value(), db_id.value()}});
        reverse_config_ids_.insert({{db_id.value(), last_id.value()}});
        return *this;
    }

    client &operator-=(raven::config_id_st id)
    {
        DLOG_F(INFO, "erasing id: %lu from config: %d", id.value(), static_cast<int>(this->sock_->fileno()));
        if (config_ids_.find(id.value()) != config_ids_.end()) {
            auto db_id = config_ids_.at(id.value());
            reverse_config_ids_.erase(db_id);
        }
        config_ids_.erase(id.value());
        return *this;
    }

    config_id_st get_last_id() const noexcept
    {
        return last_id;
    }

    void subscribe(raven::config_id_st id, const std::string &setting_name)
    {
        // TODO check if not subscribed
        sub_settings_.insert({config_ids_.at(id.value()), setting_name});
    }

    void unsubscribe(raven::config_id_st id, const std::string &setting_name)
    {
        DLOG_F(INFO, "unsubscribing setting: %s within config id: %d from client: %d", setting_name.c_str(), id.value(),
               static_cast<int>(this->sock_->fileno()));
        auto range = sub_settings_.equal_range(config_ids_.at(id.value()));
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == setting_name) {
                sub_settings_.erase(it);
                break;
            }
        }
    }

    bool has_subscribed(raven::config_id_st db_id, const std::string &setting_name)
    {
        if (sub_settings_.find(db_id.value()) == sub_settings_.end())
            return false;
        auto range = sub_settings_.equal_range(db_id.value());
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == setting_name)
                return true;
        }
        return false;
    }

    client_ptr &get_socket()
    {
        return sock_;
    }

    raven::config_id_st get_db_id_from(raven::config_id_st id)
    {
        return raven::config_id_st{config_ids_.at(id.value())};
    }

    raven::config_id_st get_id_from_db(raven::config_id_st db_id)
    {
        return raven::config_id_st{reverse_config_ids_.at(db_id.value())};
    }

    bool has_loaded(raven::config_id_st id)
    {
        return config_ids_.find(id.value()) != config_ids_.end();
    }

    bool has_loaded_db_id(raven::config_id_st db_id)
    {
        return reverse_config_ids_.find(db_id.value()) != reverse_config_ids_.end();
    }

  private:
    client_ptr sock_;
    raven::config_id_st last_id{0};
    std::unordered_map<raven::config_id_st::value_type, raven::config_id_st::value_type> config_ids_;
    std::unordered_map<raven::config_id_st::value_type, raven::config_id_st::value_type> reverse_config_ids_; // temporary workaround for a basic id lookup, will need in the future to be able to do that outside of the client class
    std::unordered_multimap<raven::config_id_st::value_type, std::string> sub_settings_;
  };
};
