//
// Created by milerius on 19/10/18.
//

#pragma once

#include <utility>
#include <unordered_map>
#include <iomanip>
#include <filesystem>
#include <uvw.hpp>
#include <uv.h>
#include <loguru.hpp>
#include "client.hpp"
#include "protocol.hpp"
#include "db.hpp"

namespace raven
{
  class service
  {
  public:
    service(std::filesystem::path db_path = std::filesystem::current_path() / "albinos_service.db") noexcept : db_{
        db_path}
    {
        VLOG_SCOPE_F(loguru::Verbosity_INFO, "service constructor");
        DVLOG_F(loguru::Verbosity_INFO, "register error_event libuv listener: %s",
                "<uvw::ErrorEvent>([this](auto const &error_event, auto &)");
        server_->on<uvw::ErrorEvent>([this](auto const &error_event, auto &) {
            LOG_SCOPE_F(ERROR, __PRETTY_FUNCTION__);
            DVLOG_F(loguru::Verbosity_ERROR, "%s", error_event.what());
            this->error_occurred = true;
        });
        DVLOG_F(loguru::Verbosity_INFO, "register listen_event libuv listener: %s",
                "<uvw::ListenEvent>([this](uvw::ListenEvent const &, uvw::PipeHandle &handle)");
        server_->on<uvw::ListenEvent>([this](uvw::ListenEvent const &, uvw::PipeHandle &handle) {
            LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
            std::shared_ptr<uvw::PipeHandle> socket = handle.loop().resource<uvw::PipeHandle>();
            DVLOG_F(loguru::Verbosity_INFO, "register close_event libuv listener: %s",
                    "<uvw::CloseEvent>([this](uvw::CloseEvent const &, uvw::PipeHandle &handle)");
            socket->on<uvw::CloseEvent>([this](uvw::CloseEvent const &, uvw::PipeHandle &handle) {
                LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
                DVLOG_F(loguru::Verbosity_INFO, "socket closed.");
                handle.close();
#ifdef DOCTEST_LIBRARY_INCLUDED
                this->uv_loop_->stop();
#endif
            });

            DVLOG_F(loguru::Verbosity_INFO, "register end_event libuv listener: %s",
                    "<uvw::EndEvent>([](const uvw::EndEvent &, uvw::PipeHandle &sock)");
            socket->on<uvw::EndEvent>([this](const uvw::EndEvent &, uvw::PipeHandle &sock) {
                LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
                DVLOG_F(loguru::Verbosity_INFO, "closing socket: %d", static_cast<int>(sock.fileno()));
                //! Since the client will disconnect, we unload every config related to him
                DVLOG_F(loguru::Verbosity_INFO, "unload every config for the client -> %d",
                        static_cast<int>(sock.fileno()));
                this->config_clients_registry_.erase(sock.fileno());
                sock.close();
            });

            DVLOG_F(loguru::Verbosity_INFO, "register data_event libuv listener: %s",
                    "<uvw::DataEvent>([this](const uvw::DataEvent &data, uvw::PipeHandle &sock)");
            socket->on<uvw::DataEvent>([this](const uvw::DataEvent &data, uvw::PipeHandle &sock) {
                LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
                std::string_view data_str(data.data.get(), data.length);
                try {
                    auto json_data = json::json::parse(data_str);
                    std::string command_order = json_data.at(raven::request_keyword).get<std::string>();
                    order_registry.at(command_order)(json_data, sock);
                }
                catch (const std::exception &error) {
                    DVLOG_F(loguru::Verbosity_ERROR, "error in received data: %s", error.what());
                    json::json unknown_request_data = R"(
                                                          {"REQUEST_STATE": "UNKNOWN_REQUEST"}
                                                        )"_json;
                    sock.write(unknown_request_data.dump().data(),
                               static_cast<unsigned int>(unknown_request_data.dump().size()));
                }
            });

            handle.accept(*socket);
            config_clients_registry_.emplace(std::make_pair(socket->fileno(), raven::client(socket)));
            socket->read();
        });
    }

    ~service() noexcept
    {
        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        DVLOG_F(loguru::Verbosity_INFO, "destroy service");
    }

    void run() noexcept
    {
        clean_socket();
        create_socket();
        run_loop();
    }

  private:
    void run_loop()
    {
        uv_loop_->run();
    }

    bool clean_socket() noexcept
    {
        if (std::filesystem::exists(socket_path_)) {
            LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
            DVLOG_F(loguru::Verbosity_WARNING, "socket: %s already exist, removing", socket_path_.string().c_str());
            std::filesystem::remove(socket_path_);
            return true;
        }
        return false;
    }

    bool create_socket() noexcept
    {
        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        std::string socket = socket_path_.string();
        DVLOG_F(loguru::Verbosity_INFO, "binding to socket: %s", socket.c_str());
        server_->bind(socket);
        DLOG_IF_F(ERROR, this->error_occurred, "an error occured during the bind");
        if (this->error_occurred) return this->error_occurred;
        server_->listen();
        DLOG_IF_F(ERROR, this->error_occurred, "an error occured during the listen");
        return this->error_occurred;
    }

    //! Helpers
    void send_answer(json::json &response_json_data, uvw::PipeHandle &sock) noexcept
    {
        auto response_str = response_json_data.dump();
        sock.write(response_str.data(), static_cast<unsigned int>(response_str.size()));
    }

    template <typename ProtocolType>
    void prepare_answer(uvw::PipeHandle &sock, const ProtocolType &answer) noexcept
    {
        json::json response_json_data;
        to_json(response_json_data, answer);
        send_answer(response_json_data, sock);
    }

    void prepare_answer(uvw::PipeHandle &sock, request_state state = request_state::success) noexcept
    {
        json::json response_json_data;
        response_json_data[request_state_keyword] = convert_request_state.at(state);
        send_answer(response_json_data, sock);
    }

    template <typename Request>
    Request fill_request(json::json &json_data)
    {
        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        Request request;
        from_json(json_data, request);
        DLOG_F(INFO, "json receive:%s", json_data.dump().c_str());
        return request;
    }

    void create_config(json::json &json_data, uvw::PipeHandle &sock)
    {
        auto cfg = fill_request<config_create>(json_data);
        auto config_create_db_result = db_.config_create(json_data);
        if (!config_create_db_result.state_error.has_value()) {
            using namespace std::string_literals;
            auto unique_id = std::hash<std::string>()(cfg.config_name + "_"s + std::to_string(sock.fileno()) + "_"s +
                std::to_string(config_create_db_result.config_id.value()));
            config_clients_registry_.at(sock.fileno()) += std::make_pair(config_id_st{unique_id},
                config_create_db_result.config_id);
            const config_create_answer answer{config_create_db_result.config_key,
                                              config_create_db_result.readonly_config_key,
                                              convert_request_state.at(request_state::success)};
            prepare_answer(sock, answer);
        } else {
            const config_create_answer answer{config_key_st{""}, config_key_st{""},
                                              convert_request_state.at(config_create_db_result.state_error.value())};
            prepare_answer(sock, answer);
        }
    }

    void load_config(json::json &json_data, uvw::PipeHandle &sock)
    {
        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        auto cfg = fill_request<config_load>(json_data);
        DLOG_IF_F(INFO, cfg.config_key.has_value(), "cfg.config_key: %s", cfg.config_key.value().value().c_str());
        DLOG_IF_F(INFO, cfg.config_read_only_key.has_value(), "cfg.config_read_only_key: %s",
                  cfg.config_read_only_key.value().value().c_str());

        auto answer = db_.config_load(cfg);
        using namespace std::string_literals;
        //TODO:  Use bimap here would be better
        answer.config_id = config_id_st{(std::hash<std::string>()(answer.config_name + "_"s +
                                        std::to_string(sock.fileno()) + "_"s + std::to_string(answer.config_id.value())))};
        prepare_answer(sock, answer);
    }

    void unload_config(json::json &json_data, uvw::PipeHandle &sock)
    {
        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        auto cfg = fill_request<config_unload>(json_data);
        auto &config_ids = config_clients_registry_.at(sock.fileno());
        config_ids -= cfg.id;
        prepare_answer(sock);
    }

    void include_config(json::json &json_data, uvw::PipeHandle &sock)
    {
        //TODO: Communication.md has changed, need to be modified
        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        auto cfg = fill_request<config_include>(json_data);
        DLOG_F(INFO, "cfg.id: %lu", cfg.id.value());
        DLOG_F(INFO, "cfg.src_id: %lu", cfg.src_id.value());
        auto answer = db_.config_include(cfg);
        prepare_answer(sock, answer.state);
    }

    void update_setting(json::json &json_data, uvw::PipeHandle &sock)
    {
        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        auto cfg = fill_request<setting_update>(json_data);
        DLOG_F(INFO, "cfg.id: %lu", cfg.id.value());
        DLOG_F(INFO, "settings_to_update: %s", cfg.settings_to_update.dump().c_str());
        auto state = db_.settings_update(cfg);
        prepare_answer(sock, state);
    }

    void remove_setting(json::json &json_data, uvw::PipeHandle &sock)
    {
        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        auto cfg = fill_request<setting_remove>(json_data);
        DLOG_F(INFO, "cfg.id: %d", cfg.id);
        DLOG_F(INFO, "cfg.setting_name: %s", cfg.setting_name.c_str());
        prepare_answer(sock);
    }

    void get_setting(json::json &json_data, uvw::PipeHandle &sock)
    {
        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        auto cfg = fill_request<setting_get>(json_data);
        DLOG_F(INFO, "cfg.id: %d", cfg.id);
        DLOG_F(INFO, "cfg.setting_name: %s", cfg.setting_name.c_str());
        //! TODO: load from sql

        //! TODO: change after arthur work
        const setting_get_answer answer{"FooBar", convert_request_state.at(request_state::success)};
        prepare_answer(sock, answer);
    }

    void set_alias(json::json &json_data, uvw::PipeHandle &sock)
    {
        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        auto cfg = fill_request<alias_set>(json_data);
        DLOG_F(INFO, "cfg.id: %d", cfg.id);
        DLOG_F(INFO, "cfg.alias_name: %s", cfg.alias_name.c_str());
        DLOG_F(INFO, "cfg.setting_name: %s", cfg.setting_name.c_str());
        prepare_answer(sock);
    }

    void unset_alias(json::json &json_data, uvw::PipeHandle &sock)
    {
        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        auto cfg = fill_request<alias_unset>(json_data);
        DLOG_F(INFO, "cfg.id: %d", cfg.id);
        DLOG_F(INFO, "cfg.alias_name: %s", cfg.alias_name.c_str());
        prepare_answer(sock);
    }

    void subscribe_setting(json::json &json_data, uvw::PipeHandle &sock)
    {
        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        auto cfg = fill_request<setting_subscribe>(json_data);
        DLOG_F(INFO, "cfg.id: %d", cfg.id);
        DLOG_IF_F(INFO, cfg.setting_name.has_value(), "cfg.setting_name: %s", cfg.setting_name.value().c_str());
        DLOG_IF_F(INFO, cfg.alias_name.has_value(), "cfg.alias_name: %s", cfg.alias_name.value().c_str());
        prepare_answer(sock);
    }

    void unsubscribe_setting(json::json &json_data, uvw::PipeHandle &sock)
    {
        LOG_SCOPE_F(INFO, __PRETTY_FUNCTION__);
        auto cfg = fill_request<setting_subscribe>(json_data);
        DLOG_F(INFO, "cfg.id: %d", cfg.id);
        DLOG_IF_F(INFO, cfg.setting_name.has_value(), "cfg.setting_name: %s", cfg.setting_name.value().c_str());
        DLOG_IF_F(INFO, cfg.alias_name.has_value(), "cfg.alias_name: %s", cfg.alias_name.value().c_str());
        prepare_answer(sock);
    }

    std::shared_ptr<uvw::Loop> uv_loop_{uvw::Loop::getDefault()};
    std::shared_ptr<uvw::PipeHandle> server_{uv_loop_->resource<uvw::PipeHandle>()};
    std::filesystem::path socket_path_{(std::filesystem::temp_directory_path() / "raven-os_service_albinos.sock")};
    std::unordered_map<uvw::OSFileDescriptor::Type, raven::client> config_clients_registry_;
    config_db db_;
    bool error_occurred{false};
    const std::unordered_map<std::string, std::function<void(json::json &, uvw::PipeHandle &)>>
        order_registry
        {
            {
                "CONFIG_CREATE",       [this](json::json &json_data, uvw::PipeHandle &sock) {
                this->create_config(json_data, sock);
            }},
            {
                "CONFIG_LOAD",         [this](json::json &json_data, uvw::PipeHandle &sock) {
                this->load_config(json_data, sock);
            }},
            {
                "CONFIG_UNLOAD",       [this](json::json &json_data, uvw::PipeHandle &sock) {
                this->unload_config(json_data, sock);
            }},
            {
                "CONFIG_INCLUDE",      [this](json::json &json_data, uvw::PipeHandle &sock) {
                this->include_config(json_data, sock);
            }},
            {
                "SETTING_UPDATE",      [this](json::json &json_data, uvw::PipeHandle &sock) {
                this->update_setting(json_data, sock);
            }},
            {
                "SETTING_REMOVE",      [this](json::json &json_data, uvw::PipeHandle &sock) {
                this->remove_setting(json_data, sock);
            }},
            {
                "SETTING_GET",         [this](json::json &json_data, uvw::PipeHandle &sock) {
                this->get_setting(json_data, sock);
            }},
            {
                "ALIAS_SET",           [this](json::json &json_data, uvw::PipeHandle &sock) {
                this->set_alias(json_data, sock);
            },
            },
            {
                "ALIAS_UNSET",         [this](json::json &json_data, uvw::PipeHandle &sock) {
                this->unset_alias(json_data, sock);
            },
            },
            {
                "SUBSCRIBE_SETTING",   [this](json::json &json_data, uvw::PipeHandle &sock) {
                this->subscribe_setting(json_data, sock);
            },
            },
            {
                "UNSUBSCRIBE_SETTING", [this](json::json &json_data, uvw::PipeHandle &sock) {
                this->unsubscribe_setting(json_data, sock);
            },
            }
        };

#ifdef DOCTEST_LIBRARY_INCLUDED
    TEST_CASE_CLASS ("test create socket")
    {
        service service_{std::filesystem::current_path() / "albinos_service_test_internal.db"};
        if (std::filesystem::exists(service_.socket_path_)) {
            std::filesystem::remove(service_.socket_path_);
        }
        CHECK_FALSE(service_.create_socket());
        CHECK(service_.create_socket());
        CHECK(service_.clean_socket());
        std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test_internal.db");
    }

    TEST_CASE_CLASS ("test clean socket")
    {
        service service_{std::filesystem::current_path() / "albinos_service_test_internal.db"};
        CHECK_FALSE(service_.clean_socket());
        CHECK_FALSE(service_.create_socket());
        CHECK(service_.clean_socket());
        std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test_internal.db");
    }

    static void
    test_setup_client(const nlohmann::json &request, const nlohmann::json &expected_answer, bool consider_only_state,
                      const service &service_, std::shared_ptr<uvw::PipeHandle> &client) noexcept
    {
        client->once<uvw::ConnectEvent>([&request](const uvw::ConnectEvent &, uvw::PipeHandle &handle) {
            CHECK(handle.writable());
            CHECK(handle.readable());
            auto request_str = request.dump();
            handle.write(request_str.data(), static_cast<unsigned int>(request_str.size()));
            handle.read();
        });

        client->once<uvw::DataEvent>(
            [&expected_answer, &consider_only_state](const uvw::DataEvent &data, uvw::PipeHandle &sock) {
                std::string_view data_str(data.data.get(), data.length);
                auto json_data = json::json::parse(data_str);
                auto json_data_str = json_data.dump();
                if (!consider_only_state) {
                    CHECK(json_data == expected_answer);
                } else {
                    CHECK(json_data.at("REQUEST_STATE").get<std::string>() ==
                    expected_answer.at("REQUEST_STATE").get<std::string>());
                }
                sock.close();
            });

        client->connect(service_.socket_path_.string());
    }

    static void test_run_and_clean_client(service &service_, std::shared_ptr<uvw::Loop> &loop) noexcept
    {
        loop->run();
        service_.clean_socket();
        std::filesystem::remove(std::filesystem::current_path() / "albinos_service_test_internal.db");
    }

    static void test_client_server_communication(json::json &&request, json::json &&expected_answer,
                                                 bool consider_only_state = false) noexcept
    {
        service service_{std::filesystem::current_path() / "albinos_service_test_internal.db"};
        CHECK_FALSE(service_.create_socket());
        auto loop = uvw::Loop::getDefault();
        auto client = loop->resource<uvw::PipeHandle>();
        test_setup_client(request, expected_answer, consider_only_state, service_, client);
        test_run_and_clean_client(service_, loop);
    }

    TEST_CASE_CLASS ("unknown request")
    {
        auto data = R"({"REQUEST_NAME": "HELLOBRUH"})"_json;
        auto answer = R"({"REQUEST_STATE":"UNKNOWN_REQUEST"})"_json;
        test_client_server_communication(std::move(data), std::move(answer));
    }

    TEST_CASE_CLASS ("create_config request")
    {
        auto data = R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config"})"_json;
        auto answer = R"({"CONFIG_KEY":"","READONLY_CONFIG_KEY":"","REQUEST_STATE":"SUCCESS"})"_json;
        test_client_server_communication(std::move(data), std::move(answer), true);
    }

    TEST_CASE_CLASS ("load_config request")
    {
        SUBCASE("read only key unknown in empty config table") {
            auto data = R"({"REQUEST_NAME": "CONFIG_LOAD","READONLY_CONFIG_KEY": "422Key"})"_json;
            auto answer = R"({"CONFIG_NAME":"Foo","CONFIG_ID":42,"REQUEST_STATE":"DB_ERROR"})"_json;
            test_client_server_communication(std::move(data), std::move(answer), true);
        }

        SUBCASE("non read only key unknown key") {
            auto data = R"({"REQUEST_NAME": "CONFIG_LOAD","CONFIG_KEY": "42Key"})"_json;
            auto answer = R"({"CONFIG_NAME":"Foo","CONFIG_ID":42,"REQUEST_STATE":"DB_ERROR"})"_json;
            test_client_server_communication(std::move(data), std::move(answer), true);
        }

        SUBCASE ("read only key unknown") {
            using namespace std::string_literals;
            service service_{std::filesystem::current_path() / "albinos_service_test_internal.db"};
            auto answer_create = service_.db_.config_create(
                R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config_to_load_lol"})"_json);
            auto request = R"({"REQUEST_NAME": "CONFIG_LOAD","READONLY_CONFIG_KEY": "unknown_readonly_config_key"})"_json;
            auto expected_answer = R"({"CONFIG_NAME":"","CONFIG_ID":0,"REQUEST_STATE":"UNKNOWN_KEY"})"_json;
            CHECK_FALSE(service_.create_socket());
            auto loop = uvw::Loop::getDefault();
            auto client = loop->resource<uvw::PipeHandle>();
            test_setup_client(request, expected_answer, true, service_, client);
            test_run_and_clean_client(service_, loop);
        };

        SUBCASE ("load_config request with known readonly_config_key") {
            using namespace std::string_literals;
            service service_{std::filesystem::current_path() / "albinos_service_test_internal.db"};
            auto answer_create = service_.db_.config_create(
                R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config_to_load"})"_json);
            auto request = R"({"REQUEST_NAME": "CONFIG_LOAD","READONLY_CONFIG_KEY": "42Key"})"_json;
            request["READONLY_CONFIG_KEY"] = answer_create.readonly_config_key.value();
            auto expected_answer = R"({"CONFIG_NAME":"ma_config_to_load","CONFIG_ID":42,"REQUEST_STATE":"SUCCESS"})"_json;
            expected_answer["CONFIG_ID"] = answer_create.config_id.value();
            CHECK_FALSE(service_.create_socket());
            auto loop = uvw::Loop::getDefault();
            auto client = loop->resource<uvw::PipeHandle>();
            test_setup_client(request, expected_answer, true, service_, client);
            test_run_and_clean_client(service_, loop);
        };

        SUBCASE ("load_config request with known config_key") {
            using namespace std::string_literals;
            service service_{std::filesystem::current_path() / "albinos_service_test_internal.db"};
            auto answer_create = service_.db_.config_create(
                R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config_to_load_regular"})"_json);
            auto request = R"({"REQUEST_NAME": "CONFIG_LOAD","CONFIG_KEY": "42Key"})"_json;
            request["CONFIG_KEY"] = answer_create.config_key.value();
            auto expected_answer = R"({"CONFIG_NAME":"ma_config_to_load_regular","CONFIG_ID":42,"REQUEST_STATE":"SUCCESS"})"_json;
            expected_answer["CONFIG_ID"] = answer_create.config_id.value();
            CHECK_FALSE(service_.create_socket());
            auto loop = uvw::Loop::getDefault();
            auto client = loop->resource<uvw::PipeHandle>();
            test_setup_client(request, expected_answer, true, service_, client);
            test_run_and_clean_client(service_, loop);
        };
    }

    TEST_CASE_CLASS ("unload_config request")
    {
        auto data = R"({"REQUEST_NAME": "CONFIG_UNLOAD","CONFIG_ID": 42})"_json;
        auto answer = R"({"REQUEST_STATE":"SUCCESS"})"_json;
        test_client_server_communication(std::move(data), std::move(answer));
    }

    TEST_CASE_CLASS ("include_config request")
    {
        SUBCASE("include nonexistent config src && dst") {
            auto data = R"({"REQUEST_NAME": "CONFIG_INCLUDE","CONFIG_ID": 42, "SRC": 31})"_json;
            auto answer = R"({"REQUEST_STATE":"UNKNOWN_ID"})"_json;
            test_client_server_communication(std::move(data), std::move(answer));
        }
        SUBCASE("include into nonexistent config") {
            using namespace std::string_literals;
            service service_{std::filesystem::current_path() / "albinos_service_test_internal.db"};
            auto answer_create = service_.db_.config_create(
                R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config"})"_json);
            auto answer_second = service_.db_.config_create(
                R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config_second"})"_json);
            auto request = R"({"REQUEST_NAME": "CONFIG_INCLUDE","CONFIG_ID": 42, "SRC": 31})"_json;
            request["SRC"] = answer_second.config_id.value();
            auto expected_answer = R"({"REQUEST_STATE":"UNKNOWN_ID"})"_json;
            CHECK_FALSE(service_.create_socket());
            auto loop = uvw::Loop::getDefault();
            auto client = loop->resource<uvw::PipeHandle>();
            test_setup_client(request, expected_answer, false, service_, client);
            test_run_and_clean_client(service_, loop);
        }

        SUBCASE("include from nonexistent config") {
            using namespace std::string_literals;
            service service_{std::filesystem::current_path() / "albinos_service_test_internal.db"};
            auto answer_create = service_.db_.config_create(
                R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config"})"_json);
            auto answer_second = service_.db_.config_create(
                R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config_second"})"_json);
            auto request = R"({"REQUEST_NAME": "CONFIG_INCLUDE","CONFIG_ID": 42, "SRC": 31})"_json;
            request["CONFIG_ID"] = answer_create.config_id.value();
            auto expected_answer = R"({"REQUEST_STATE":"UNKNOWN_ID"})"_json;
            CHECK_FALSE(service_.create_socket());
            auto loop = uvw::Loop::getDefault();
            auto client = loop->resource<uvw::PipeHandle>();
            test_setup_client(request, expected_answer, false, service_, client);
            test_run_and_clean_client(service_, loop);
        }

        SUBCASE("include from nonexistent config") {
            using namespace std::string_literals;
            service service_{std::filesystem::current_path() / "albinos_service_test_internal.db"};
            auto answer_create = service_.db_.config_create(
                R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config"})"_json);
            auto answer_second = service_.db_.config_create(
                R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config_second"})"_json);
            auto request = R"({"REQUEST_NAME": "CONFIG_INCLUDE","CONFIG_ID": 42, "SRC": 31})"_json;
            request["CONFIG_ID"] = answer_create.config_id.value();
            request["SRC"] = answer_second.config_id.value();
            auto expected_answer = R"({"REQUEST_STATE":"SUCCESS"})"_json;
            CHECK_FALSE(service_.create_socket());
            auto loop = uvw::Loop::getDefault();
            auto client = loop->resource<uvw::PipeHandle>();
            test_setup_client(request, expected_answer, false, service_, client);
            test_run_and_clean_client(service_, loop);
        }
    }

    TEST_CASE_CLASS ("update_setting request")
    {
        SUBCASE("update_setting with unknown id") {
            auto data = R"({"REQUEST_NAME": "SETTING_UPDATE","CONFIG_ID": 42,"SETTINGS_TO_UPDATE": {"foo": "bar","titi": 1}})"_json;
            auto answer = R"({"REQUEST_STATE":"UNKNOWN_ID"})"_json;
            test_client_server_communication(std::move(data), std::move(answer), true);
        }

        SUBCASE ("update_setting with valid id") {
            using namespace std::string_literals;
            service service_{std::filesystem::current_path() / "albinos_service_test_internal.db"};
            auto answer_create = service_.db_.config_create(
                R"({"REQUEST_NAME": "CONFIG_CREATE","CONFIG_NAME": "ma_config"})"_json);
            auto request = R"({"REQUEST_NAME": "SETTING_UPDATE","CONFIG_ID": 42,"SETTINGS_TO_UPDATE": {"foo": "bar","titi": 1}})"_json;
            request["CONFIG_ID"] = answer_create.config_id.value();
            auto expected_answer = R"({"REQUEST_STATE":"SUCCESS"})"_json;
            CHECK_FALSE(service_.create_socket());
            auto loop = uvw::Loop::getDefault();
            auto client = loop->resource<uvw::PipeHandle>();
            test_setup_client(request, expected_answer, true, service_, client);
            test_run_and_clean_client(service_, loop);
        };
    }

    TEST_CASE_CLASS ("remove_setting request")
    {
        auto data = R"({"REQUEST_NAME": "SETTING_REMOVE","CONFIG_ID": 43,"SETTING_NAME": "foobar"})"_json;
        auto answer = R"({"REQUEST_STATE":"SUCCESS"})"_json;
        test_client_server_communication(std::move(data), std::move(answer));
    }

    TEST_CASE_CLASS ("get_setting request")
    {
        auto data = R"({"REQUEST_NAME": "SETTING_GET","CONFIG_ID": 43,"SETTING_NAME": "foobar"})"_json;
        auto answer = R"({"SETTING_VALUE": "FooBar", "REQUEST_STATE":"SUCCESS"})"_json;
        test_client_server_communication(std::move(data), std::move(answer));
    }

    TEST_CASE_CLASS ("alias_set request")
    {
        auto data = R"({"REQUEST_NAME": "ALIAS_SET","CONFIG_ID": 43,"SETTING_NAME": "foobar","ALIAS_NAME": "barfoo"})"_json;
        auto answer = R"({"REQUEST_STATE":"SUCCESS"})"_json;
        test_client_server_communication(std::move(data), std::move(answer));
    }

    TEST_CASE_CLASS ("alias_unset request")
    {
        auto data = R"({"REQUEST_NAME": "ALIAS_UNSET","CONFIG_ID": 43,"ALIAS_NAME": "barfoo"})"_json;
        auto answer = R"({"REQUEST_STATE":"SUCCESS"})"_json;
        test_client_server_communication(std::move(data), std::move(answer));
    }

    TEST_CASE_CLASS ("setting_subscribe request")
    {
        SUBCASE("without alias") {
            auto data = R"({"REQUEST_NAME": "SUBSCRIBE_SETTING","CONFIG_ID": 43,"SETTING_NAME": "foobar"})"_json;
            auto answer = R"({"REQUEST_STATE":"SUCCESS"})"_json;
            test_client_server_communication(std::move(data), std::move(answer));
        }

        SUBCASE("with alias") {
            auto data = R"({"REQUEST_NAME": "SUBSCRIBE_SETTING","CONFIG_ID": 43,"ALIAS_NAME": "barfoo"})"_json;
            auto answer = R"({"REQUEST_STATE":"SUCCESS"})"_json;
            test_client_server_communication(std::move(data), std::move(answer));
        }
    }

    TEST_CASE_CLASS ("setting_unsubscribe request")
    {
        SUBCASE("without alias") {
            auto data = R"({"REQUEST_NAME": "UNSUBSCRIBE_SETTING","CONFIG_ID": 43,"SETTING_NAME": "foobar"})"_json;
            auto answer = R"({"REQUEST_STATE":"SUCCESS"})"_json;
            test_client_server_communication(std::move(data), std::move(answer));
        }

        SUBCASE("with alias") {
            auto data = R"({"REQUEST_NAME": "UNSUBSCRIBE_SETTING","CONFIG_ID": 43,"ALIAS_NAME": "barfoo"})"_json;
            auto answer = R"({"REQUEST_STATE":"SUCCESS"})"_json;
            test_client_server_communication(std::move(data), std::move(answer));
        }
    }

#endif
  };
}
