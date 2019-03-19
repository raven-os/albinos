module cli.deserializer;
static import std.file;
import asdf;

auto deserialize_to(TProtocol)(string recipe_path)
{
    return (cast(const char[])(std.file.read(recipe_path))).deserialize!TProtocol;
}
