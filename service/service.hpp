//
// Created by milerius on 19/10/18.
//

#pragma once

#include <string_view>
#include <iostream>
#include <filesystem>
#include <memory>
#include <uvw.hpp>

namespace raven
{
    class service
    {
    public:
        service()
        {
            server_->on<uvw::ErrorEvent>([](auto const &, auto &) { /* TODO: Fill it */ });
            server_->on<uvw::ListenEvent>([](uvw::ListenEvent const &, uvw::PipeHandle &handle) {
                std::shared_ptr<uvw::PipeHandle> socket = handle.loop().resource<uvw::PipeHandle>();
                socket->on<uvw::CloseEvent>(
                    [&handle](uvw::CloseEvent const &, uvw::PipeHandle &) {
                        std::cout << "close event receive" << std::endl;
                        handle.close();
                    });
                socket->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::PipeHandle &sock) {
                    std::cout << "end event receive" << std::endl;
                    sock.close();
                });

                socket->on<uvw::DataEvent>([](const uvw::DataEvent &data, uvw::PipeHandle &sock) {
                    std::string_view data_str(data.data.get(), data.length);
                    std::cout << "data received: " << data_str;

                    //TODO: Treat the data here

                });

                handle.accept(*socket);
                std::cout << "socket connected" << std::endl;
                socket->read();
            });
        }

        void run() noexcept
        {
            clean_socket();
            std::string socket = (std::filesystem::temp_directory_path() / "raven-os_service_libconfig.sock").string();
            std::cout << "binding to socket: " << socket << std::endl;
            server_->bind(socket);
            server_->listen();
            uv_loop_->run();
        }

        //! In case that the service have been stopped, we want to remove the old socket and create a new one.
        void clean_socket() noexcept
        {
            auto socket_path = std::filesystem::temp_directory_path() / "raven-os_service_libconfig.sock";
            if (std::filesystem::exists(socket_path)) {
                std::cout << "socket: " << socket_path.string() << " already exist, removing" << std::endl;
                std::filesystem::remove(socket_path);
            }
        }

    private:
        std::shared_ptr<uvw::Loop> uv_loop_{uvw::Loop::getDefault()};
        std::shared_ptr<uvw::PipeHandle> server_{uv_loop_->resource<uvw::PipeHandle>()};
    };
}