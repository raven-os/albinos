#include <filesystem>
#include "service.hpp"

int main()
{
    raven::service service;
    service.run();
    /*const std::string sockname = "lol.sock";

    auto loop = uvw::Loop::getDefault();
    auto server = loop->resource<uvw::PipeHandle>();
    auto client = loop->resource<uvw::PipeHandle>();

    server->on<uvw::ErrorEvent>([](const auto &, auto &) {  });

    server->once<uvw::ListenEvent>([](const uvw::ListenEvent &, uvw::PipeHandle &handle) {
        std::shared_ptr<uvw::PipeHandle> socket = handle.loop().resource<uvw::PipeHandle>();

        socket->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &, uvw::PipeHandle &) {  });
        socket->on<uvw::CloseEvent>([&handle](const uvw::CloseEvent &, uvw::PipeHandle &) { handle.close(); });
        socket->on<uvw::EndEvent>([](const uvw::EndEvent &, uvw::PipeHandle &sock) { sock.close(); });

        handle.accept(*socket);
        socket->read();*/
    //});

    //server->bind(sockname);
    //server->listen();
    //loop->run();
    //return 0;
}
