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
  inline constexpr const db_statement_st select_config_name_statement{
      R"(select config_text from config where id = ?;)"};
  inline constexpr const db_statement_st select_keys_config_create_statement{
      R"(select config_key,readonly_config_key from config where id = ? ;)"};
  inline constexpr const db_statement_st select_config_from_key_statement{
      R"(select id from config where config_key = ? or readonly_config_key = ?;)"};
  inline constexpr const db_statement_st select_config_from_readwrite_key_statement{
      R"(select config_text,id from config where config_key = ?;)"};
  inline constexpr const db_statement_st select_config_from_readonly_key_statement{
      R"(select config_text,id from config where readonly_config_key = ?;)"};
  inline constexpr const db_statement_st select_count_key_statement{
      R"(select count(*) FROM config where config_key = ? or readonly_config_key = ?;)"};
  inline constexpr const db_statement_st select_count_readwrite_key_statement{
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

  inline constexpr const char config_settings_field_keyword[] = "SETTINGS";
  inline constexpr const char config_includes_field_keyword[] = "INCLUDES";


  // maybe do fieldbits in the future, but seems sufficient for now
  enum class db_state : short
  {
    ok = 0,
    unknow_config_key,
    unknow_config_id,
    sql_error,
    fatal_error,
  };

  struct config_create_result
  {
    config_key_st config_key;
    config_key_st readonly_config_key;
    config_id_st config_id;
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

    explicit config_db(const std::filesystem::path &path_to_db) noexcept : database_{path_to_db.string()}
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

    config_create_result config_create(const std::string &name) noexcept
    {
        /*
         * Create a config with the name passed as parameter
         *
         * Return the resulting config ID as well as the config key and read-only config key
         *
         * In case an error occur, the state will be set accordingly (`sql_error`, `fatal_error`)
         *
         */

        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        using namespace std::string_literals;
        config_key_st config_key;
        config_key_st readonly_config_key;
        config_id_st config_id;
        for (unsigned int nb_tries_ = 0; nb_tries_ < maximum_retries_; ++nb_tries_) {
            try {
                json::json data_to_bind;
                data_to_bind[config_name_keyword] = name;
                data_to_bind[config_settings_field_keyword] = json::json::object();
                data_to_bind[config_includes_field_keyword] = json::json::array();
                DLOG_F(INFO, "attempt nb: %u", nb_tries_);
                DLOG_F(INFO, "json to insert in db: %s", data_to_bind.dump().c_str());
                auto data_to_bind_str = data_to_bind.dump();
                execute_statement(insert_config_create_statement,
                                  data_to_bind_str, (random_string() +
                                                     std::to_string(std::hash<std::string>()(name))),
                                  (random_string() + std::to_string(std::hash<std::string>()(name))));
                execute_statement(select_keys_config_create_statement, database_.last_insert_rowid())
                    >> [&](const std::string &config_key_, const std::string &readonly_config_key_) {
                        config_key = config_key_st{config_key_};
                        readonly_config_key = config_key_st{readonly_config_key_};
                    };
                //success;
                config_id = config_id_st{static_cast<std::size_t>(database_.last_insert_rowid())};
                state = db_state::ok;
                break;
            }
            catch (const sqlite::errors::constraint_unique &error) {
                DLOG_F(ERROR, "%s, from sql -> %s", error.what(), error.get_sql().c_str());
                state = db_state::sql_error;
                /* error constraint violated, we retry with a new random_string() */
            }
            catch (const sqlite::sqlite_exception &error) {
                DLOG_F(ERROR, "error: %s, from sql: %s", error.what(), error.get_sql().c_str());
                state = db_state::sql_error;
                /* this is another error from database, we retry with a new random_string() */
            }
            catch (const std::exception &error) {
                DLOG_F(ERROR, "error: %s", error.what());
                state = db_state::fatal_error;
                /* this error seem's fatal, we break */
                break;
            }
        }
        return {config_key, readonly_config_key, config_id};
    }

    std::string get_config_name(config_id_st config_id)
    {
        /*
         * Get the corresponding name to the config id passed as parameter
         *
         * In case an error occur, the state will be set accordingly (`unknow_config_id`, `sql_error`, `fatal_error`)
         *
         */

        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        std::string config_name;
        auto functor_receive_data = [&config_name](const std::string json_text) {
            auto json_data = json::json::parse(json_text);
            config_name = json_data.at(config_name_keyword).get<std::string>();
        };
        try {
            int nb_count = 0;
            execute_statement(select_count_config_element_statement) >> nb_count;
            if (!nb_count)
                throw sqlite::errors::empty(0, select_count_config_element_statement.value());
            throw_misuse_if_count_return_zero_for_this_statement(select_count_config_from_id_statement, config_id.value());
            execute_statement(select_config_name_statement, config_id.value()) >> functor_receive_data;
            state = db_state::ok;
        }
        catch (const sqlite::errors::empty &error) {
            DLOG_F(ERROR, "misuse of api or wrong key: %s, from sql: %s", error.what(), error.get_sql().c_str());
            state = db_state::sql_error;
        }
        catch (const sqlite::errors::misuse &error) {
            DLOG_F(ERROR, "misuse of api or wrong key: %s, from sql: %s", error.what(), error.get_sql().c_str());
            state = db_state::unknow_config_id;
        }
        catch (const sqlite::sqlite_exception &error) {
            DLOG_F(ERROR, "error: %s, from sql: %s", error.what(), error.get_sql().c_str());
            state = db_state::sql_error;
        }
        catch (const std::exception &error) {
            DLOG_F(ERROR, "%s", error.what());
            state = db_state::fatal_error;
        }
        return std::move(config_name);
    }

    config_id_st get_config_id(const config_key_st &config_key) noexcept
    {
        /*
         * Get the corresponding config_id to the key passed as parameter
         *
         * In case an error occur, the state will be set accordingly (`unknow_config_key`, `sql_error`, `fatal_error`)
         *
         */

        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        config_id_st config_id;
        auto functor_receive_data = [&config_id](int id) { config_id = config_id_st{static_cast<std::size_t>(id)}; };
        try {
            int nb_count = 0;
            execute_statement(select_count_config_element_statement) >> nb_count;
            if (!nb_count)
                throw sqlite::errors::empty(0, select_count_config_element_statement.value());
            throw_misuse_if_count_return_zero_for_this_statement(select_count_key_statement, config_key.value(), config_key.value());
            execute_statement(select_config_from_key_statement, config_key.value(), config_key.value()) >> functor_receive_data;
            state = db_state::ok;
        }
        catch (const sqlite::errors::empty &error) {
            DLOG_F(ERROR, "misuse of api or wrong key: %s, from sql: %s", error.what(), error.get_sql().c_str());
            state = db_state::sql_error;
        }
        catch (const sqlite::errors::misuse &error) {
            DLOG_F(ERROR, "misuse of api or wrong key: %s, from sql: %s", error.what(), error.get_sql().c_str());
            state = db_state::unknow_config_key;
        }
        catch (const sqlite::sqlite_exception &error) {
            DLOG_F(ERROR, "error: %s, from sql: %s", error.what(), error.get_sql().c_str());
            state = db_state::sql_error;
        }
        catch (const std::exception &error) {
            DLOG_F(ERROR, "%s", error.what());
            state = db_state::fatal_error;
        }
        return config_id;
    }

    // TODO get permission from id

    json::json get_config(config_id_st id) noexcept
    {
        /*
         * Get the corresponding config in json format to the id passed as parameter
         *
         * In case an error occur, the state will be set accordingly (`unknow_config_id`, `sql_error`, `fatal_error`)
         *
         */

        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        json::json data;
        auto functor_receive_data = [&data](const std::string &json_text) { data = json::json::parse(json_text); };
        try {
            throw_misuse_if_count_return_zero_for_this_statement(select_count_config_from_id_statement, id.value());
            execute_statement(select_config_from_id_statement, id.value()) >> functor_receive_data;
            state = db_state::ok;
        }
        catch (const sqlite::errors::misuse &error) {
            DLOG_F(ERROR, "misuse of api or wrong id: %s, from sql: %s -> [with id = %lu]", error.what(),
                   error.get_sql().c_str(), id.value());
            state = db_state::unknow_config_id;
        }
        catch (const sqlite::sqlite_exception &error) {
            DLOG_F(ERROR, "error: %s, from sql: %s", error.what(), error.get_sql().c_str());
            state = db_state::sql_error;
        }
        catch (const std::exception &error) {
            DLOG_F(ERROR, "%s", error.what());
            state = db_state::fatal_error;
        }
        return data;
    }

    void update_config(const json::json &updated_data, config_id_st id) noexcept
    {
        /*
         * Update the config corresponding to the id with the json data passed as parameter
         *
         * In case an error occur, the state will be set accordingly (`unknow_config_id`, `sql_error`, `fatal_error`)
         *
         */

        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        try {
            throw_misuse_if_count_return_zero_for_this_statement(select_count_config_from_id_statement, id.value());
            execute_statement(update_config_text_from_id_statement, updated_data.dump(), id.value());
            state = db_state::ok;
        }
        catch (const sqlite::errors::misuse &error) {
            DLOG_F(ERROR, "misuse of api or wrong id: %s, from sql: %s -> [with id = %lu]", error.what(),
                   error.get_sql().c_str(), id.value());
            state = db_state::unknow_config_id;
        }
        catch (const sqlite::sqlite_exception &error) {
            DLOG_F(ERROR, "error: %s, from sql: %s", error.what(), error.get_sql().c_str());
            state = db_state::sql_error;
        }
        catch (const std::exception &error) {
            DLOG_F(ERROR, "%s", error.what());
            state = db_state::fatal_error;
        }
    }

    bool good() const noexcept { return state == db_state::ok; }
    /*
     *  Return True if the last operation succeeded
     */

    bool fail() const noexcept { return !good(); }
     /*
     *  Return False if the last operation succeeded
     */

    db_state get_state() const noexcept { return state; }
     /*
     *  Return the state after the last operation
     */

  private:
    template <typename ... Args>
    void throw_misuse_if_count_return_zero_for_this_statement(const db_statement_st &statement, Args &&...args)
    {
        int nb_count = 0;
        execute_statement(statement, std::forward<Args>(args)...) >> nb_count;
        if (!nb_count)
            throw sqlite::errors::misuse(0, statement.value());
    }


    sqlite::database database_;
    static constexpr const unsigned int maximum_retries_{4};
    db_state state{db_state::ok};

#ifdef DOCTEST_LIBRARY_INCLUDED
    TEST_CASE_CLASS ("config_create db")
    {
        config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
        SUBCASE("normal case") {
            CHECK_EQ(db.config_create("ma_config").config_id.value(), 1u);
            CHECK_FALSE(db.fail());
            CHECK_EQ(db.config_create("ma_config_de_ouf").config_id.value(), 2u);
            CHECK_FALSE(db.fail());
        }
        SUBCASE("unique constraints violation") {
            for (int i = 0; i < 100; ++i) {
                WARN_NE(db.config_create("ma_config_qui_marche_pas").config_id.value(), -1);
                /*if (db.config_create(config_create).config_id.value() == -1) {
                    break;
                }*/
            }
        }
        std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
    }

    TEST_CASE_CLASS ("get config id")
    {
        SUBCASE("normal case key not readonly") {
            config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
            auto config_create_answer = db.config_create("ma_config");
            auto id = db.get_config_id(config_create_answer.config_key);
            CHECK_EQ(id.value(), config_create_answer.config_id.value());
            std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
        }
        SUBCASE("normal case key readonly") {
            config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
            auto config_create_answer = db.config_create("ma_config_readonly");
            auto id = db.get_config_id(config_create_answer.readonly_config_key);
            CHECK_EQ(id.value(), config_create_answer.config_id.value());
            std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
        }
        SUBCASE("unknow key") {
            config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
            auto config_create_answer = db.config_create("ma_config_readonly");
            db.get_config_id(config_key_st{"lalakey"});
            CHECK_FALSE(db.good());
            CHECK_EQ(db.get_state(), db_state::unknow_config_key);
            std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
        }
    }

    TEST_CASE_CLASS ("get config name")
    {
        SUBCASE("normal case get config name") {
            config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
            auto config_create_answer = db.config_create("ma_config");
            auto name = db.get_config_name(config_create_answer.config_id);
            CHECK_EQ(name, "ma_config");
            std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
        }
        SUBCASE("unknow id") {
            config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
            auto config_create_answer = db.config_create("ma_config_readonly");
            db.get_config_name(config_id_st{43});
            CHECK_FALSE(db.good());
            CHECK_EQ(db.get_state(), db_state::unknow_config_id);
            std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
        }
    }

#endif
  };
}
