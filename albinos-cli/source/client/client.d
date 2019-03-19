module client.client;
import std.socket;

class Client
{
    ///
    this(string socket_path)
    {
        this.address_ = new UnixAddress(socket_path);
        this.socket_ = new Socket(AddressFamily.UNIX, SocketType.STREAM);
        socket_.connect(this.address_);
    }

    @property Socket socket() pure nothrow @nogc
    {
        return socket_;
    }

    private UnixAddress address_;
    private Socket socket_;
}
