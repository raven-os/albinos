//
// Created by milerius on 19/10/18.
//

#pragma once

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
            server_->on<uvw::ErrorEvent>([](const auto &, auto &) { /* TODO: Fill it */ });
            server_->on<uvw::ListenEvent>([](const uvw::ListenEvent &, uvw::PipeHandle &handle) {
                std::shared_ptr<uvw::PipeHandle> socket = handle.loop().resource<uvw::PipeHandle>();
                socket->once<uvw::CloseEvent>(
                    [&handle](const uvw::CloseEvent &, uvw::PipeHandle &) { handle.close(); });

                //! TODO: do more
            });
        }

        void run() noexcept
        {
            server_->bind((std::filesystem::temp_directory_path() / "raven-os_service_libconfig.sock").string());
            server_->listen();
            uv_loop_->run();
        }

    private:
        std::shared_ptr<uvw::Loop> uv_loop_{uvw::Loop::getDefault()};
        std::shared_ptr<uvw::PipeHandle> server_{uv_loop_->resource<uvw::PipeHandle>()};
    };
}