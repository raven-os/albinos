//
// Created by milerius on 03/03/19.
//

#pragma once

#include <string>
#include <sqlite_modern_cpp.h>
#include "st/st.hpp"

namespace raven
{
  class config_db
  {
  public:
    config_db(std::filesystem::path path_to_db) noexcept : database_{path_to_db.string()}
    {
        try {
            database_ << R"(create table if not exists config
                      (
                        config_text text,
                        id          integer not null
                        constraint  table_name_pk
                        primary key autoincrement
                      );
                    )";

            database_ << R"(
                            create unique index if not exists table_name_id_uindex on config (id);
                        )";
        }
        catch (const std::exception &error) {
            std::cerr << "error db occured: " << error.what() << std::endl;
        }
    }

    using config_id = st::type<int, struct config_id_tag, st::arithmetic>;

    config_id config_create(const json::json &config_create_data) noexcept
    {
        using namespace std::string_literals;
        try {
            json::json data_to_bind;
            data_to_bind["CONFIG_NAME"] = config_create_data["CONFIG_NAME"];
            data_to_bind["SETTINGS"] = {};
            std::cout << data_to_bind.dump() << std::endl;
            auto data_to_bind_str = data_to_bind.dump();
            database_ << "insert into config (config_text) VALUES (?);" << data_to_bind_str;
        }
        catch (const std::exception &error) {
            std::cerr << "error: " << error.what() << std::endl;
            return config_id{static_cast<int>(request_state::db_error)};
        }
        return config_id{static_cast<int>(database_.last_insert_rowid())};;
    }

  private:

#ifdef DOCTEST_LIBRARY_INCLUDED
    TEST_CASE_CLASS ("config_create db")
    {
        config_db db{std::filesystem::current_path() / "albinos_service_test.db"};
        SUBCASE("normal case") {
            json::json config_create = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config"})"_json;
            CHECK_EQ(db.config_create(config_create).value(), 1u);

            config_create = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config_de_ouf"})"_json;
            CHECK_EQ(db.config_create(config_create).value(), 2u);
        }
        std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test.db");
    }

#endif

  private:
    sqlite::database database_;
  };
}