import std.stdio;
import cli.cli;

void main(string[] args)
{
	writeln(args);
	auto cli = new CLI("/tmp/raven-os_service_albinos.sock");
	cli.run_program;
}
