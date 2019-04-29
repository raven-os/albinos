//
// Created by milerius on 03/03/19.
//

#pragma once

#include <optional>
#include <string>
#include <sqlite_modern_cpp.h>
#include <random>
#include <loguru.hpp>
#include "service_strong_types.hpp"
#include "utils.hpp"

namespace raven
{
  inline constexpr const db_statement_st create_table_statement{
      R"(create table if not exists config(config_text text,id integer not null constraint config_pk primary key autoincrement, config_key  text, readonly_config_key text);)"};
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
  inline constexpr const db_statement_st select_count_config_element_statement{R"(select count(*) from config;)"};
  inline constexpr const db_statement_st update_config_text_from_id_statement{
      R"(UPDATE config set config_text = ? where id = ?;)"};

  struct config_create_answer_db
  {
    config_key_st config_key;
    config_key_st readonly_config_key;
    config_id_st config_id;
    std::optional<request_state> state_error;
  };

  struct config_include_answer_db
  {
    request_state state;
    std::size_t nb_configs;
  };

  class config_db
  {
  public:
    template <typename ... Args>
    inline auto execute_statement(const db_statement_st &statement, Args &&...args)
    {
        DLOG_F(INFO, "%s", statement.value());
        return (database_ << statement.value() << ... << args);
    }

    config_db(std::filesystem::path path_to_db) noexcept : database_{path_to_db.string()}
    {
        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        try {
            execute_statement(create_table_statement);
            execute_statement(create_unique_index_config_id_statement);
            execute_statement(create_unique_index_config_key_statement);
            execute_statement(create_unique_index_readonly_config_key_statement);
        }
        catch (const std::exception &error) {
            DLOG_F(ERROR, "error db occured: %s", error.what());
        }
    }

    config_create_answer_db config_create(const json::json &config_create_data) noexcept
    {
        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        using namespace std::string_literals;
        config_key_st config_key;
        config_key_st readonly_config_key;
        config_id_st request_state_value;
        std::optional<request_state> state_error = std::nullopt;
        for (nb_tries_ = 0; nb_tries_ < maximum_retries_; ++nb_tries_) {
            try {
                json::json data_to_bind;
                data_to_bind[config_name_keyword] = config_create_data[config_name_keyword];
                data_to_bind["SETTINGS"] = json::json::object();
                data_to_bind["OTHER_CONFIG"] = json::json::array();
                DLOG_F(INFO, "json to insert in db: %s", data_to_bind.dump().c_str());
                auto data_to_bind_str = data_to_bind.dump();
                execute_statement(insert_config_create_statement,
                                  data_to_bind_str, (random_string() +
                                                     std::to_string(std::hash<std::string>()(
                                                         config_create_data[config_name_keyword]))),
                                  (random_string() + std::to_string(std::hash<std::string>()
                                                                        (config_create_data[config_name_keyword]))));

                execute_statement(select_keys_config_create_statement, database_.last_insert_rowid())
                    >> [&](std::string config_key_, std::string readonly_config_key_) {
                        config_key = config_key_st{config_key_};
                        readonly_config_key = config_key_st{readonly_config_key_};
                    };
                //success;
                request_state_value = config_id_st{static_cast<std::size_t>(database_.last_insert_rowid())};
                state_error = std::nullopt;
                break;
            }
            catch (const sqlite::errors::constraint_unique &error) {
                DLOG_F(ERROR, "%s, from sql -> %s", error.what(), error.get_sql().c_str());
                state_error = request_state::db_error;
                /* error constraint violated, we retry with a new random_string() */
            }
            catch (const sqlite::sqlite_exception &error) {
                DLOG_F(ERROR, "error: %s, from sql: %s", error.what(), error.get_sql().c_str());
                state_error = request_state::db_error;
                /* this is another error from database, we retry with a new random_string() */
            }
            catch (const std::exception &error) {
                DLOG_F(ERROR, "error: %s", error.what());
                state_error = request_state::db_error;
                /* this error seem's fatal, we break */
                break;
            }
        }
        return {config_key, readonly_config_key, request_state_value, state_error};
    }

    config_load_answer config_load(const config_load &config_load_data) noexcept
    {
        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        config_load_answer db_answer{"", config_id_st{}, convert_request_state.at(request_state::success)};
        try {
            auto functor_receive_data = [&db_answer](const std::string json_text, int id) {
                auto json_data = json::json::parse(json_text);
                db_answer.config_id = config_id_st{static_cast<std::size_t>(id)};
                db_answer.config_name = json_data.at(config_name_keyword).get<std::string>();
            };
            int nb_count = 0;
            execute_statement(select_count_config_element_statement) >> nb_count;
            if (!nb_count)
                throw sqlite::errors::empty(0, select_count_config_element_statement.value());
            if (config_load_data.config_key) {
                throw_misuse_if_count_return_zero_for_this_statement(select_count_key_statement,
                                                                     config_load_data.config_key.value().value());
                execute_statement(select_config_from_key_statement,
                                  config_load_data.config_key.value().value()) >> functor_receive_data;
            } else if (config_load_data.config_read_only_key) {
                throw_misuse_if_count_return_zero_for_this_statement(select_count_readonly_key_statement,
                                                                     config_load_data.config_read_only_key.value().value());
                execute_statement(select_config_from_readonly_key_statement,
                                  config_load_data.config_read_only_key.value().value()) >> functor_receive_data;
            }
        }
        catch (const sqlite::errors::misuse &error) {
            DLOG_F(ERROR, "misuse of api or wrong key: %s, from sql: %s", error.what(), error.get_sql().c_str());
            db_answer.request_state = convert_request_state.at(request_state::unknown_key);
        }
        catch (const sqlite::sqlite_exception &error) {
            DLOG_F(ERROR, "error: %s, from sql: %s", error.what(), error.get_sql().c_str());
            db_answer.request_state = convert_request_state.at(request_state::db_error);
        }
        catch (const std::exception &error) {
            DLOG_F(ERROR, "%s", error.what());
            db_answer.request_state = convert_request_state.at(request_state::db_error);
        }
        return db_answer;
    }

    request_state settings_update(const setting_update &setting_update_data, raven::config_id_st db_id)
    {
        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        try {
            auto functor_receive_data = [&setting_update_data, db_id, this](const std::string json_text) {
                auto json_data = json::json::parse(json_text);
                for (auto&[key, value] : setting_update_data.settings_to_update.items()) {
                    json_data["SETTINGS"][key] = value;
                }
                DLOG_F(INFO, "json after update: %s", json_data.dump().c_str());
                this->execute_statement(update_config_text_from_id_statement,
                    json_data.dump().c_str(), db_id.value());
            };
            throw_misuse_if_count_return_zero_for_this_statement(select_count_config_from_id_statement,
                                                                 db_id.value());
            execute_statement(select_config_from_id_statement, db_id.value()) >> functor_receive_data;
        }
        catch (const sqlite::errors::misuse &error) {
            DLOG_F(ERROR, "misuse of api or wrong id: %s, from sql: %s -> [with id = %lu]", error.what(),
                error.get_sql().c_str(),
                db_id.value());
            return request_state::unknown_id;
        }
        catch (const sqlite::sqlite_exception &error) {
            DLOG_F(ERROR, "error: %s, from sql: %s", error.what(), error.get_sql().c_str());
            return request_state::db_error;
        }
        catch (const std::exception &error) {
            DLOG_F(ERROR, "%s", error.what());
            return request_state::db_error;
        }
        return request_state::success;
    }

    setting_get_answer setting_get(const setting_get& data, raven::config_id_st db_id) noexcept
    {
        setting_get_answer answer;
        try {
            auto functor_receive_data = [&data, &answer, this](const std::string json_text) {
                auto json_data = json::json::parse(json_text);
                answer.setting_value = json_data["SETTINGS"].at(data.setting_name);
            };
            execute_statement(select_config_from_id_statement, db_id.value()) >> functor_receive_data;
            DLOG_F(INFO, "get setting %s with value %s from db", data.setting_name.c_str(), answer.setting_value.c_str());
            answer.request_state = convert_request_state.at(request_state::success);
        }
        catch (const nlohmann::json::out_of_range &error) {
            DLOG_F(ERROR, "error: %s, trying to find setting: %s", error.what(), data.setting_name.c_str());
            answer.request_state = convert_request_state.at(request_state::unknown_setting);
        }
        catch (const sqlite::sqlite_exception &error) {
            DLOG_F(ERROR, "error: %s, from sql: %s", error.what(), error.get_sql().c_str());
            answer.request_state = convert_request_state.at(request_state::db_error);
        }
        catch (const std::exception &error) {
            DLOG_F(ERROR, "%s", error.what());
            answer.request_state = convert_request_state.at(request_state::db_error);
        }
        return answer;
    }

    config_include_answer_db config_include(const config_include& config_include_data) noexcept
    {
        //TODO: Communication.md has changed, need to be modified
        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        config_include_answer_db answer{request_state::success, 0u};
        try {
            auto functor_receive_data = [this, &config_include_data, &answer](const std::string json_text) {
                auto json_data = json::json::parse(json_text);
                DLOG_F(INFO, "json before update: %s", json_data.dump().c_str());
                auto& array_other_config = json_data["OTHER_CONFIG"];
                array_other_config.push_back(config_include_data.src_id.value());
                std::sort(begin(array_other_config), end(array_other_config));
                auto last = std::unique(begin(array_other_config), end(array_other_config));
                array_other_config.erase(last, end(array_other_config));
                DLOG_F(INFO, "json after update: %s", json_data.dump().c_str());
                answer.nb_configs = array_other_config.size();
                this->execute_statement(update_config_text_from_id_statement,
                                        json_data.dump().c_str(),
                                        config_include_data.id.value());
            };
            throw_misuse_if_count_return_zero_for_this_statement(select_count_config_from_id_statement,
                config_include_data.id.value());
            throw_misuse_if_count_return_zero_for_this_statement(select_count_config_from_id_statement,
                                                                 config_include_data.src_id.value());
            execute_statement(select_config_from_id_statement, config_include_data.id.value()) >> functor_receive_data;
        }
        catch(const sqlite::errors::misuse& error) {
            DLOG_F(ERROR, "misuse of api or wrong id: %s, from sql: %s -> [with dst_id = %lu, src_id = %lu]", error.what(),
                   error.get_sql().c_str(),
                   config_include_data.id.value(),
                   config_include_data.src_id.value());
            answer.state = request_state::unknown_id;
            return answer;
        }
        catch(const std::exception& error) {
            DLOG_F(ERROR, "%s", error.what());
            answer.state = request_state::db_error;
        }
        return answer;
    }

  private:
    template <typename Value>
    void throw_misuse_if_count_return_zero_for_this_statement(const db_statement_st &statement, Value value_statement)
    {
        int nb_count = 0;
        execute_statement(statement, value_statement) >> nb_count;
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
                /*if (db.config_create(config_create).config_id.value() == static_cast<int>(request_state::db_error)) {
                    break;
                }*/
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
            CHECK_EQ(static_cast<short>(db.settings_update(setting_update_data, raven::config_id_st{42})),
                static_cast<short>(request_state::unknown_id));
            std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
        }

        SUBCASE("normal update setting") {
            config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
            json::json config_create = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config"})"_json;
            auto config_create_answer = db.config_create(config_create);
            setting_update setting_update_data{config_create_answer.config_id, {{"foo", "bar"}}};
            CHECK_EQ(static_cast<short>(db.settings_update(setting_update_data, config_create_answer.config_id)),
                static_cast<short>(request_state::success));
            std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
        }

        SUBCASE("normal update multiple settings") {
            config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
            json::json config_create = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config"})"_json;
            auto config_create_answer = db.config_create(config_create);
            setting_update setting_update_data{config_create_answer.config_id, {{"foo", "bar"}, {"titi", 1}}};
            CHECK_EQ(static_cast<short>(db.settings_update(setting_update_data, config_create_answer.config_id)),
                static_cast<short>(request_state::success));
            std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
        }
    }

    TEST_CASE_CLASS("config include db") {
        SUBCASE("include into nonexistent  destination") {
            config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
            json::json config_create = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config"})"_json;
            auto config_create_answer = db.config_create(config_create);
            struct config_include config_include_data{config_id_st{42}, config_create_answer.config_id};
            CHECK_EQ(static_cast<short>(db.config_include(config_include_data).state),
                static_cast<short>(request_state::unknown_id));
            std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
        }

        SUBCASE("include from nonexistent src") {
            config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
            json::json config_create = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config"})"_json;
            auto config_create_answer = db.config_create(config_create);
            struct config_include config_include_data{config_create_answer.config_id, config_id_st{42}};
            CHECK_EQ(static_cast<short>(db.config_include(config_include_data).state),
                static_cast<short>(request_state::unknown_id));
            std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
        }

        SUBCASE("include unique config into existent src") {
            config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
            json::json config_create = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config"})"_json;
            json::json config_create_second = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config_second"})"_json;
            auto config_create_answer = db.config_create(config_create);
            auto config_create_answer_second = db.config_create(config_create_second);
            struct config_include config_include_data{config_create_answer.config_id,
                config_create_answer_second.config_id};
            auto answer = db.config_include(config_include_data);
            CHECK_EQ(static_cast<short>(answer.state), static_cast<short>(request_state::success));
            CHECK_EQ(answer.nb_configs, 1u);
            std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
        }

        SUBCASE("include multiple config into existent src") {
            config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
            json::json config_create = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config"})"_json;
            json::json config_create_second = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config_second"})"_json;
            json::json config_create_third = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config_third"})"_json;
            auto config_create_answer = db.config_create(config_create);
            auto config_create_answer_second = db.config_create(config_create_second);
            auto config_create_answer_third = db.config_create(config_create_third);
            struct config_include config_include_data{config_create_answer.config_id,
                                                      config_create_answer_second.config_id};
            db.config_include(config_include_data);
            config_include_data.src_id = config_create_answer_third.config_id;
            auto answer = db.config_include(config_include_data);
            CHECK_EQ(static_cast<short>(answer.state), static_cast<short>(request_state::success));
            CHECK_EQ(answer.nb_configs, 2u);
            std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
        }

        SUBCASE("include multiple config with same id into existent src") {
            config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
            json::json config_create = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config"})"_json;
            json::json config_create_second = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config_second"})"_json;
            json::json config_create_third = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config_third"})"_json;
            auto config_create_answer = db.config_create(config_create);
            auto config_create_answer_second = db.config_create(config_create_second);
            auto config_create_answer_third = db.config_create(config_create_third);
            struct config_include config_include_data{config_create_answer.config_id,
                                                      config_create_answer_second.config_id};
            db.config_include(config_include_data);
            config_include_data.src_id = config_create_answer_third.config_id;
            db.config_include(config_include_data);
            auto answer = db.config_include(config_include_data);
            CHECK_EQ(static_cast<short>(answer.state), static_cast<short>(request_state::success));
            CHECK_EQ(answer.nb_configs, 2u);
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
