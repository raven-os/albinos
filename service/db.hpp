//
// Created by milerius on 03/03/19.
//

#pragma once

#include <string>
#include <sqlite_modern_cpp.h>
#include <random>
#include "service_strong_types.hpp"
#include "utils.hpp"

namespace raven
{
  inline constexpr const db_statement_st create_table_statement{R"(create table if not exists config
                      ( config_text text,
                        id          integer not null
                        constraint  config_pk
                        primary key autoincrement,
                        config_key  text,
	                readonly_config_key text
                      );
                    )"};
  inline constexpr const db_statement_st create_unique_index_config_id_statement{
      R"(create unique index if not exists config_id_uindex on config (id);)"};
  inline constexpr const db_statement_st create_unique_index_config_key_statement{
      R"(create unique index if not exists config_config_key_uindex on config (config_key);)"};
  inline constexpr const db_statement_st create_unique_index_readonly_config_key_statement{
      R"(create unique index if not exists config_readonly_config_key_uindex on config (readonly_config_key);)"};

  struct config_create_answer_db
  {
    config_key_st config_key;
    config_key_st readonly_config_key;
    config_id_st config_id;
  };

  class config_db
  {
  public:
    config_db(std::filesystem::path path_to_db) noexcept : database_{path_to_db.string()}
    {
        try {
            database_ << create_table_statement.value();
            database_ << create_unique_index_config_id_statement.value();
            database_ << create_unique_index_config_key_statement.value();
            database_ << create_unique_index_readonly_config_key_statement.value();
        }
        catch (const std::exception &error) {
            std::cerr << "error db occured: " << error.what() << std::endl;
        }
    }

    config_create_answer_db config_create(const json::json &config_create_data) noexcept
    {
        using namespace std::string_literals;
        config_key_st config_key;
        config_key_st readonly_config_key;
        config_id_st request_state_value{static_cast<int>(request_state::db_error)};
        for (nb_tries_ = 0; nb_tries_ < maximum_retries_; ++nb_tries_) {
            try {
                json::json data_to_bind;
                data_to_bind["CONFIG_NAME"] = config_create_data["CONFIG_NAME"];
                data_to_bind["SETTINGS"] = {};
                std::cout << data_to_bind.dump() << std::endl;
                auto data_to_bind_str = data_to_bind.dump();
                database_ << "insert into config (config_text, config_key, readonly_config_key) VALUES (?, ?, ?);"
                          << data_to_bind_str
                          << (random_string() +
                              std::to_string(std::hash<std::string>()(config_create_data["CONFIG_NAME"])))
                          << (random_string() +
                              std::to_string(std::hash<std::string>()(config_create_data["CONFIG_NAME"])));

                database_ << "select config_key,readonly_config_key from config where id = ? ;"
                          << database_.last_insert_rowid()
                          >> [&](std::string config_key_, std::string readonly_config_key_) {
                              config_key = config_key_st{config_key_};
                              readonly_config_key = config_key_st{readonly_config_key_};
                          };
                //success;
                request_state_value = config_id_st{static_cast<int>(database_.last_insert_rowid())};
                break;
            }
            catch (const sqlite::errors::constraint_unique &error) {
                std::cerr << error.what() << std::endl;
                /* error constraint violated, we retry with a new random_string() */
            }
            catch (const std::exception &error) {
                std::cerr << "error: " << error.what() << std::endl;
                /* this error seem's fatal, we break */
                break;
            }
        }
        return {config_key, readonly_config_key, request_state_value};
    }

  private:

#ifdef DOCTEST_LIBRARY_INCLUDED
    TEST_CASE_CLASS ("config_create db")
    {
        config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
        SUBCASE("normal case") {
            json::json config_create = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config"})"_json;
            CHECK_EQ(db.config_create(config_create).config_id.value(), 1u);

            config_create = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config_de_ouf"})"_json;
            CHECK_EQ(db.config_create(config_create).config_id.value(), 2u);
        }
        SUBCASE("unique constraints violation") {
            json::json config_create = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config_foo"})"_json;
            for (int i = 0; i < 100; ++i) {
                WARN_NE(db.config_create(config_create).config_id.value(), static_cast<int>(request_state::db_error));
                if (db.config_create(config_create).config_id.value() == static_cast<int>(request_state::db_error)) {
                    break;
                }
            }
        }
        std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
    }

#endif

  private:
    sqlite::database database_;
    unsigned int nb_tries_{0};
    unsigned int maximum_retries_{4};
  };
}