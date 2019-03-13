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
  inline constexpr const db_statement_st insert_config_create_statement{
      R"(insert into config (config_text, config_key, readonly_config_key) VALUES (?, ?, ?);)"};
  inline constexpr const db_statement_st select_keys_config_create_statement{
      R"(select config_key,readonly_config_key from config where id = ? ;)"};
  inline constexpr const db_statement_st select_config_from_key_statement{
      R"(select config_text,id from config where config_key = ?;)"};
  inline constexpr const db_statement_st select_config_from_readonly_key_statement{
      R"(select config_text,id from config where readonly_config_key = ?;)"};
  inline constexpr const db_statement_st select_count_key_statement{
      R"(select count(*) FROM config where config_key = ?;)"};
  inline constexpr const db_statement_st select_count_readonly_key_statement{
      R"(select count(*) FROM config where readonly_config_key = ?;)"};
  inline constexpr const db_statement_st select_count_config_from_id_statement{
      R"(select count(*) FROM config where id = ?;)"};
  inline constexpr const db_statement_st select_config_from_id_statement{
      R"(select config_text from config where id = ?;)"};

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
                data_to_bind[config_name_keyword] = config_create_data[config_name_keyword];
                data_to_bind["SETTINGS"] = json::json::object();
                data_to_bind["OTHER_CONFIG"] = json::json::array();
                std::cout << data_to_bind.dump() << std::endl;
                auto data_to_bind_str = data_to_bind.dump();
                database_ << insert_config_create_statement.value()
                          << data_to_bind_str
                          << (random_string() +
                              std::to_string(std::hash<std::string>()(config_create_data[config_name_keyword])))
                          << (random_string() +
                              std::to_string(std::hash<std::string>()(config_create_data[config_name_keyword])));

                database_ << select_keys_config_create_statement.value()
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

    config_load_answer config_load(const config_load &config_load_data) noexcept
    {
        config_load_answer db_answer{"", config_id_st{}, convert_request_state.at(request_state::success)};
        try {
            auto functor_receive_data = [&db_answer](const std::string json_text, int id) {
                auto json_data = json::json::parse(json_text);
                db_answer.config_id = config_id_st{id};
                db_answer.config_name = json_data.at(config_name_keyword).get<std::string>();
            };
            int nb_count = 0;
            database_ << "SELECT count(*) FROM config;" >> nb_count;
            if (!nb_count)
                throw sqlite::errors::empty(0, "SELECT count(*) FROM config;");
            if (config_load_data.config_key) {
                throw_misuse_if_count_return_zero_for_this_statement(select_count_key_statement,
                                                                     config_load_data.config_key.value().value());
                database_ << select_config_from_key_statement.value() << config_load_data.config_key.value().value()
                          >> functor_receive_data;
            } else if (config_load_data.config_read_only_key) {
                throw_misuse_if_count_return_zero_for_this_statement(select_count_readonly_key_statement,
                                                                     config_load_data.config_read_only_key.value().value());
                database_ << select_config_from_readonly_key_statement.value()
                          << config_load_data.config_read_only_key.value().value() >> functor_receive_data;
            }
        }
        catch (const sqlite::errors::misuse &error) {
            std::cerr << "misuse of api or wrong key:" << error.what() << std::endl;
            db_answer.request_state = convert_request_state.at(request_state::unknown_key);
        }
        catch (const std::exception &error) {
            std::cerr << error.what() << std::endl;
            db_answer.request_state = convert_request_state.at(request_state::db_error);
        }
        return db_answer;
    }

    request_state settings_update(const setting_update &setting_update_data)
    {
        try {
            auto functor_receive_data = [&setting_update_data](const std::string json_text) {
                auto json_data = json::json::parse(json_text);
                for (auto&[key, value] : setting_update_data.settings_to_update.items()) {
                    json_data["SETTINGS"][key] = value;
                }
                std::cout << json_data.dump() << std::endl;
            };
            throw_misuse_if_count_return_zero_for_this_statement(select_count_config_from_id_statement,
                                                                 setting_update_data.id.value());
            database_ << select_config_from_id_statement.value() << setting_update_data.id.value()
                      >> functor_receive_data;
        }
        catch (const sqlite::errors::misuse &error) {
            std::cerr << error.what() << std::endl;
            return request_state::unknown_id;
        }
        catch (const std::exception &error) {
            std::cerr << error.what() << std::endl;
            return request_state::db_error;
        }
        return request_state::success;
    }

  private:
    template <typename Value>
    void throw_misuse_if_count_return_zero_for_this_statement(const db_statement_st &statement, Value value_statement)
    {
        int nb_count = 0;
        database_ << statement.value() << value_statement >> nb_count;
        if (!nb_count)
            throw sqlite::errors::misuse(0, statement.value());
    }

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
                WARN_NE(db.config_create(config_create).config_id.value(),
                    static_cast<int>(request_state::db_error));
                if (db.config_create(config_create).config_id.value() == static_cast<int>(request_state::db_error)) {
                    break;
                }
            }
        }
        std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
    }

    TEST_CASE_CLASS ("config load db")
    {
        SUBCASE("normal case key not readonly") {
            config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
            json::json config_create = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config"})"_json;
            auto config_create_answer = db.config_create(config_create);
            struct config_load data{config_create_answer.config_key};
            auto res = db.config_load(data);
            CHECK_EQ(res.config_name, "ma_config");
            CHECK_EQ(res.config_id.value(), config_id_st{1}.value());
            std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
        }
        SUBCASE("normal case key readonly") {
            config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
            json::json config_create = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config_readonly"})"_json;
            auto config_create_answer = db.config_create(config_create);
            struct config_load data;
            data.config_read_only_key = config_create_answer.readonly_config_key;
            auto res = db.config_load(data);
            CHECK_EQ(res.config_name, "ma_config_readonly");
            CHECK_EQ(res.config_id.value(), config_id_st{1}.value());
            std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
        }
    }

    TEST_CASE_CLASS ("setting update db")
    {
        SUBCASE("setting update unknown id") {
            config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
            setting_update setting_update_data{config_id_st{42}, {{"foo", "bar"}}};
            CHECK_EQ(static_cast<short>(db.settings_update(setting_update_data)),
                static_cast<short>(request_state::unknown_id));
            std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
        }

        SUBCASE("normal update setting") {
            config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
            json::json config_create = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config"})"_json;
            auto config_create_answer = db.config_create(config_create);
            setting_update setting_update_data{config_create_answer.config_id, {{"foo", "bar"}}};
            CHECK_EQ(static_cast<short>(db.settings_update(setting_update_data)),
                static_cast<short>(request_state::success));
            std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
        }

        SUBCASE("normal update multiple settings") {
            config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
            json::json config_create = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config"})"_json;
            auto config_create_answer = db.config_create(config_create);
            setting_update setting_update_data{config_create_answer.config_id, {{"foo", "bar"}, {"titi", 1}}};
            CHECK_EQ(static_cast<short>(db.settings_update(setting_update_data)),
                static_cast<short>(request_state::success));
            std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
        }
    }

#endif

  private:
    sqlite::database database_;
    unsigned int nb_tries_{0};
    unsigned int maximum_retries_{4};
  };
}
