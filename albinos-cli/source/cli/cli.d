module cli.cli;

import core.exception : AssertError;
import std.stdio;
import std.string;
import std.socket : SocketOSException;
import std.getopt;
import std.uni : isWhite;
import std.process : executeShell;
import std.file;
import asdf;
import protocol.protocol_type;
import client.client;
import cli.deserializer;

class CLI
{
    this(string socket_path)
    {
        try
        {
            client_ = new Client(socket_path);
        }
        catch (SocketOSException e)
        {
            writeln(stderr, e.msg);
            is_running_ = false;
        }
    }

    private void load_config(string[] args)
    in(args !is null, "args cannot be null")
    in(args.length == 2, "need 1 arguments")
    {
        string config_key;
        getopt(args, "key", &config_key);
        auto cfg = deserialize_to!config_load("../API_doc/json_recipes/config_load.json");
        cfg.config_key = config_key;
        writeln(cfg);
    }

    private config_create_answer create_config(string[] args)
    in(args !is null, "args cannot be null")
    in(args.length == 2, "need 1 arguments")
    out(r; r.state == "SUCCESS", "create_config should success")
    out(r; !r.config_key.empty, "config_key should not be empty")
    out(r; !r.readonly_config_key.empty, "readonly_config_key should not be empty")
    {
        string config_name;
        getopt(args, "name", &config_name);
        auto cfg = deserialize_to!config_create("../API_doc/json_recipes/config_create.json");
        cfg.config_name = config_name.strip("\"");
        client_.socket.send(cfg.serializeToJson);
        auto answer = new ubyte[256];
        client_.socket.receive(answer);
        (cast(string) answer).writeln;
        return (cast(string) answer).deserialize!config_create_answer;
    }

    unittest
    {
        import std.exception : collectException;

        auto cli = new CLI("/tmp/raven-os_service_albinos.sock");
        assert(cli.create_config(["create_config", "--name=toto"])
                .state == "SUCCESS", "should be success");
        assert(cli.create_config(["create_config", "--name=\"titi\""])
                .state == "SUCCESS", "should be success");
        assert(collectException(cli.create_config(["create_config", "--name="])));
        assert(collectException!AssertError(cli.create_config(["create_config"])));
        assert(collectException!AssertError(cli.create_config(["create_config", "--name", "lol"])));
        assert(collectException!AssertError(cli.create_config(null)));
    }

    ///
    public void run_program()
    {
        if (!is_running_)
            return;
        write("> ");
        string line;
        while (((line = stdin.readln().strip("\n \t")) !is null))
        {
            if (line == "exit")
            {
                break;
            }
            auto splited_line = line.split!isWhite;
            const auto cmd = splited_line[0];
            try
            {
                switch (cmd)
                {
                case "create_config":
                    this.create_config(splited_line);
                    break;
                case "load_config":
                    this.load_config(splited_line);
                    break;
                default:
                    line.dup.executeShell.output.write;
                    break;
                }
            }
            catch (AssertError e)
            {
                writeln(stderr, e.msg);
            }
            write("> ");
        }
    }

    private Client client_;
    private bool is_running_ = true;
}
