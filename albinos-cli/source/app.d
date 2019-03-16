import std.stdio;

void main()
{
	write("> ");
	string line;
    while ((line = stdin.readln()) !is null) {
		if (line == "exit\n") break;
        write(line);
		write("> ");
	}
}