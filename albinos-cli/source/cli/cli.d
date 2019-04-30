module cli.cli;

import core.exception : AssertError;
import std.stdio;
import std.string;
import std.socket;
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

    private void check(string[] args)
    {
        client_.socket.setOption(SocketOptionLevel.SOCKET,
                                 SocketOption.RCVTIMEO, dur!"seconds"(1));   
        auto answer = new ubyte[256];
        client_.socket.receive(answer);
        client_.socket.setOption(SocketOptionLevel.SOCKET,
                                 SocketOption.RCVTIMEO, dur!"seconds"(0));
        (cast(string) answer).writeln;
    }

    private setting_unsubscribe_answer unsubscribe_setting(string[] args)
    in(args !is null, "args cannot be null")
    in(args.length == 3, "need 2 arguments")
    {
        size_t config_id;
        string setting_name;
        getopt(args, "id", &config_id, "setting_name", &setting_name);
        auto cfg = deserialize_to!setting_unsubscribe("../API_doc/json_recipes/setting_unsubscribe.json");
        cfg.config_id = config_id;
        cfg.setting_name = setting_name;
        client_.socket.send(cfg.serializeToJson);
        auto answer = new ubyte[256];
        client_.socket.receive(answer);
        (cast(string) answer).writeln;
        return (cast(string) answer).deserialize!setting_unsubscribe_answer;
    }

    private setting_subscribe_answer subscribe_setting(string[] args)
    in(args !is null, "args cannot be null")
    in(args.length == 3, "need 2 arguments")
    {
        size_t config_id;
        string setting_name;
        getopt(args, "id", &config_id, "setting_name", &setting_name);
        auto cfg = deserialize_to!setting_subscribe("../API_doc/json_recipes/setting_subscribe.json");
        cfg.config_id = config_id;
        cfg.setting_name = setting_name;
        client_.socket.send(cfg.serializeToJson);
        auto answer = new ubyte[256];
        client_.socket.receive(answer);
        (cast(string) answer).writeln;
        return (cast(string) answer).deserialize!setting_subscribe_answer;
    }

    private setting_update_answer update_setting(string[] args)
    in(args !is null, "args cannot be null")
    in(args.length == 4, "need 3 arguments")
    {
        size_t config_id;
        string setting_name;
        string setting_value;
        getopt(args, "id", &config_id, 
                     "setting_name", &setting_name, 
                     "setting_value", &setting_value);
        setting_update cfg;
        cfg.request_name = "SETTING_UPDATE";
        //auto cfg = deserialize_to!setting_update("../API_doc/json_recipes/setting_update.json");
        cfg.config_id = config_id;
        cfg.settings_to_update = format("{\"%s\":\"%s\"}", setting_name.strip("\""), setting_value.strip("\""));
        string cfg_str = format("{\"REQUEST_NAME\":\"SETTING_UPDATE\",\"CONFIG_ID\":%d,\"SETTINGS_TO_UPDATE\":%s}",
                                cfg.config_id, cfg.settings_to_update);
        //client_.socket.send(cfg.serializeToJson);
        client_.socket.send(cfg_str);
        auto answer = new ubyte[256];
        client_.socket.receive(answer);
        (cast(string) answer).writeln;
        return (cast(string) answer).deserialize!setting_update_answer;
    }

    private setting_get_answer get_setting(string[] args)
    in(args !is null, "args cannot be null")
    in(args.length == 3, "need 2 arguments")
    {
        size_t config_id;
        string setting_name;
        getopt(args, "id", &config_id, 
                     "setting_name", &setting_name);
        auto cfg = deserialize_to!setting_get("../API_doc/json_recipes/setting_get.json");
        cfg.config_id = config_id;
        cfg.setting_name = setting_name;
        client_.socket.send(cfg.serializeToJson);
        auto answer = new ubyte[256];
        client_.socket.receive(answer);
        (cast(string) answer).writeln;
        return (cast(string) answer).deserialize!setting_get_answer;
    }

    private config_load_answer load_config(string[] args)
    in(args !is null, "args cannot be null")
    in(args.length == 2, "need 1 arguments")
    out(r; r.state == "SUCCESS", "load_config should succeed")
    out(r; !r.config_name.empty, "config_name should not be empty")
    out(r; r.config_id > 0, "config_id should be different than 0")
    {
        string config_key;
        string readonly_config_key;
        getopt(args, "key", &config_key, "readonly_key", &readonly_config_key);
        auto cfg = deserialize_to!config_load("../API_doc/json_recipes/config_load.json");
        cfg.config_key = config_key;
        cfg.readonly_config_key = readonly_config_key;
        writeln(cfg);
        writeln(cfg.serializeToJsonPretty);
        client_.socket.send(cfg.serializeToJson);
        auto answer = new ubyte[256];
        client_.socket.receive(answer);
        (cast(string) answer).writeln;
        return (cast(string) answer).deserialize!config_load_answer;
    }


    unittest
    {
        auto cli = new CLI("/tmp/raven-os_service_albinos.sock");
        auto create_answer = cli.create_config(["create_config", "--name=toto"]);
        assert(create_answer.state == "SUCCESS", "should be SUCCESS");
        auto load_answer = cli.load_config(["load_config", "--key=" ~ create_answer.config_key]);
        assert(load_answer.state == "SUCCESS", "should be SUCCESS");
        //TODO: add more tests
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
                case "setting_get":
                    this.get_setting(splited_line);
                    break;
                case "setting_update":
                    this.update_setting(splited_line);
                    break;
                case "setting_subscribe":
                    this.subscribe_setting(splited_line);
                    break;
                case "setting_unsubscribe":
                    this.unsubscribe_setting(splited_line);
                    break;
                case "check":
                    this.check(splited_line);
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
