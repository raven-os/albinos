//
// Created by milerius on 19/10/18.
//

#pragma once

#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <uvw.hpp>
#include "protocol.hpp"

namespace raven
{
  class service
  {
  public:
    service() noexcept
    {
        server_->on<uvw::ErrorEvent>([this](auto const &error_event, auto &) {
            std::cerr << error_event.what() << std::endl;
            this->is_error_occured = true;
        });

        server_->on<uvw::ListenEvent>([this](uvw::ListenEvent const &, uvw::PipeHandle &handle) {
            std::shared_ptr<uvw::PipeHandle> socket = handle.loop().resource<uvw::PipeHandle>();
            socket->on<uvw::CloseEvent>(
                [this](uvw::CloseEvent const &, uvw::PipeHandle &handle) {
                    std::cout << "socket closed." << std::endl;
                    handle.close();
#ifdef DOCTEST_LIBRARY_INCLUDED
                    this->uv_loop_->stop();
#endif
                });

            socket->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::PipeHandle &sock) {
                std::cout << "end event received" << std::endl;
                sock.close();
            });

            socket->on<uvw::DataEvent>([this](const uvw::DataEvent &data, uvw::PipeHandle &sock) {
                static const std::unordered_map<std::string, std::function<void(json::json &, uvw::PipeHandle &)>>
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

                std::string_view data_str(data.data.get(), data.length);
                try {
                    auto json_data = json::json::parse(data_str);
                    std::string command_order = json_data.at(raven::request_keyword).get<std::string>();
                    order_registry.at(command_order)(json_data, sock);
                }
                catch (const json::json::exception &error) {
                    std::cerr << "error in received data: " << error.what() << std::endl;
                    json::json unknown_request_data = R"(
                                                          {"REQUEST_STATE": "UNKNOWN_REQUEST"}
                                                        )"_json;
                    sock.write(unknown_request_data.dump().data(),
                               static_cast<unsigned int>(unknown_request_data.dump().size()));
                }
            });

            handle.accept(*socket);
            std::cout << "socket connected" << std::endl;
            socket->read();
        });
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
            std::cout << "socket: " << socket_path_.string() << " already exist, removing" << std::endl;
            std::filesystem::remove(socket_path_);
            return true;
        }
        return false;
    }

    bool create_socket() noexcept
    {
        std::string socket = socket_path_.string();
        std::cout << "binding to socket: " << socket << std::endl;
        server_->bind(socket);
        if (this->is_error_occured) return this->is_error_occured;
        server_->listen();
        return this->is_error_occured;
    }

    //! Helpers
    template <typename Request>
    Request fill_request(json::json &json_data)
    {
        Request request;
        from_json(json_data, request);
        std::cout << "json receive:\n" << std::setw(4) << json_data << std::endl;
        return request;
    }

    void create_config(json::json &json_data, uvw::PipeHandle &sock)
    {
        json::json response_json_data = create_config(json_data);
        sock.write(response_json_data.dump().data(), static_cast<unsigned int>(response_json_data.dump().size()));
    }

    json::json create_config(json::json &json_data)
    {
        auto cfg = fill_request<config_create>(json_data);
        std::cout << "cfg.config_name: " << cfg.config_name << std::endl;
        // TODO: Insert in SQL

        // TODO: Send config key create by SQL (arthur work)
        const config_create_answer answer{"Foo", "Foo"};
        json::json response_json_data;
        to_json(response_json_data, answer);
        return response_json_data;
    }

    void load_config(json::json &json_data, uvw::PipeHandle &sock)
    {
        auto cfg = fill_request<config_load>(json_data);
        if (cfg.config_key) {
            std::cout << "cfg.config_key: " << cfg.config_key.value() << std::endl;
        }
        if (cfg.config_read_only_key) {
            std::cout << "cfg.config_read_only_key: " << cfg.config_read_only_key.value() << std::endl;
        }

        //! TODO: load from sql

        //! TODO: change after arthur work
        const config_load_answer answer{"Foo", 42};
        json::json response_json_data;
        to_json(response_json_data, answer);
        sock.write(response_json_data.dump().data(), static_cast<unsigned int>(response_json_data.dump().size()));
    }

    void unload_config(json::json &json_data, uvw::PipeHandle &sock)
    {
        auto cfg = fill_request<config_unload>(json_data);
        std::cout << "cfg.id: " << cfg.id << std::endl;
    }

    void include_config(json::json &json_data, uvw::PipeHandle &sock)
    {
        auto cfg = fill_request<config_include>(json_data);
        std::cout << "cfg.id: " << cfg.id << std::endl;
        std::cout << "cfg.src: " << cfg.src << std::endl;
        std::cout << "cfg.dst: " << cfg.dst << std::endl;
    }

    void update_setting(json::json &json_data, uvw::PipeHandle &sock)
    {
        auto cfg = fill_request<setting_update>(json_data);
        std::cout << "cfg.id: " << cfg.id << std::endl;
        std::cout << "cfg.setting_name: " << cfg.setting_name << std::endl;
        std::cout << "cfg.setting_value: " << cfg.setting_value << std::endl;
    }

    void remove_setting(json::json &json_data, uvw::PipeHandle &sock)
    {
        auto cfg = fill_request<setting_remove>(json_data);
        std::cout << "cfg.id: " << cfg.id << std::endl;
        std::cout << "cfg.setting_name: " << cfg.setting_name << std::endl;
    }

    void get_setting(json::json &json_data, uvw::PipeHandle &sock)
    {
        auto cfg = fill_request<setting_get>(json_data);
        std::cout << "cfg.id: " << cfg.id << std::endl;
        std::cout << "cfg.setting_name: " << cfg.setting_name << std::endl;

        //! TODO: load from sql

        //! TODO: change after arthur work
        const setting_get_answer answer{"FooBar"};
        json::json response_json_data;
        to_json(response_json_data, answer);
        sock.write(response_json_data.dump().data(), static_cast<unsigned int>(response_json_data.dump().size()));
    }

    void set_alias(json::json &json_data, uvw::PipeHandle &sock)
    {
        auto cfg = fill_request<alias_set>(json_data);
        std::cout << "cfg.id: " << cfg.id << std::endl;
        std::cout << "cfg.setting_name: " << cfg.setting_name << std::endl;
        std::cout << "cfg.alias_name: " << cfg.alias_name << std::endl;
    }

    void unset_alias(json::json &json_data, uvw::PipeHandle &sock)
    {
        auto cfg = fill_request<alias_unset>(json_data);
        std::cout << "cfg.id: " << cfg.id << std::endl;
        std::cout << "cfg.alias_name: " << cfg.alias_name << std::endl;
    }

    void subscribe_setting(json::json &json_data, uvw::PipeHandle &sock)
    {
        auto cfg = fill_request<setting_subscribe>(json_data);
        std::cout << "cfg.id: " << cfg.id << std::endl;
        if (cfg.setting_name) {
            std::cout << "cfg.setting_name: " << cfg.setting_name.value() << std::endl;
        }
        if (cfg.alias_name) {
            std::cout << "cfg.alias_name: " << cfg.alias_name.value() << std::endl;
        }
    }

    void unsubscribe_setting(json::json &json_data, uvw::PipeHandle &sock)
    {
        auto cfg = fill_request<setting_subscribe>(json_data);
        std::cout << "cfg.id: " << cfg.id << std::endl;
        if (cfg.setting_name) {
            std::cout << "cfg.setting_name: " << cfg.setting_name.value() << std::endl;
        }
        if (cfg.alias_name) {
            std::cout << "cfg.alias_name: " << cfg.alias_name.value() << std::endl;
        }
    }

    std::shared_ptr<uvw::Loop> uv_loop_{uvw::Loop::getDefault()};
    std::shared_ptr<uvw::PipeHandle> server_{uv_loop_->resource<uvw::PipeHandle>()};
    std::filesystem::path socket_path_{(std::filesystem::temp_directory_path() / "raven-os_service_albinos.sock")};
    bool is_error_occured{false};

#ifdef DOCTEST_LIBRARY_INCLUDED
    TEST_CASE_CLASS ("test create socket")
    {
        service service_;
        if (std::filesystem::exists(service_.socket_path_)) {
            std::filesystem::remove(service_.socket_path_);
        }
        CHECK_FALSE(service_.create_socket());
        CHECK(service_.create_socket());
        CHECK(service_.clean_socket());
    }

    TEST_CASE_CLASS ("test clean socket")
    {
        service service_;
        CHECK_FALSE(service_.clean_socket());
        CHECK_FALSE(service_.create_socket());
        CHECK(service_.clean_socket());
    }

    TEST_CASE_CLASS ("test create_config")
    {
        service service_;
        json::json data = R"(
                            {
                                "REQUEST_NAME": "CONFIG_CREATE",
                                "CONFIG_NAME": "ma_config"
                            }
                            )"_json;
        json::json data_expected = R"(
                                        {"CONFIG_KEY":"Foo","READONLY_CONFIG_KEY":"Foo","REQUEST_STATE":"SUCCESS"}
                                     )"_json;
        CHECK(service_.create_config(data) == data_expected);
    }

    TEST_CASE_CLASS ("check fake client")
    {
        service service_;
        CHECK_FALSE(service_.create_socket());
        auto loop = uvw::Loop::getDefault();
        auto client = loop->resource<uvw::PipeHandle>();
        client->once<uvw::ConnectEvent>([](const uvw::ConnectEvent &, uvw::PipeHandle &handle) {
            CHECK(handle.writable());
            CHECK(handle.readable());

            auto dataTryWrite = std::unique_ptr<char[]>(new char[1]{'a'});
            handle.tryWrite(std::move(dataTryWrite), 1);
            auto dataWrite = std::unique_ptr<char[]>(new char[2]{'b', 'c'});
            handle.write(std::move(dataWrite), 2);
        });

        client->once<uvw::WriteEvent>([](const uvw::WriteEvent &, uvw::PipeHandle &handle) {
            handle.close();
        });

        client->connect(service_.socket_path_.string());

        CHECK(service_.clean_socket());
        loop->run();
    }

#endif
  };
}
