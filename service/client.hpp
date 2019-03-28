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
        auto db_id = config_ids_.at(id.value());
        config_ids_.erase(id.value());
        reverse_config_ids_.erase(db_id);
        return *this;
    }

    config_id_st get_last_id() const noexcept
    {
        return last_id;
    }

  private:
    client_ptr sock_;
    raven::config_id_st last_id{0};
    std::unordered_map<raven::config_id_st::value_type, raven::config_id_st::value_type> config_ids_;
    std::unordered_map<raven::config_id_st::value_type, raven::config_id_st::value_type> reverse_config_ids_; // temporary workaround for a basic id lookup, will need in the future to be able to do that outside of the client class
  };
};
