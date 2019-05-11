import terminal
import strutils
import rdstdin
import linenoise

let doc = """
Usage:
    help (clear the screen)
    exit (quit the cli)
    clear (clear the terminal)
    config create <name> (create a config with the given name)
    config load <key> (load a config with the given key)

Example:
    > config create my_cfg
"""

proc handleConfigCmd(argsSet: openArray[string]) =
    echo argsSet

proc launchCLI*() =
    discard historySetMaxLen(500)
    while true:
        var line: TaintedString
        let res = readLineFromStdin("> ", line)
        if res == false:
            styledEcho(fgMagenta, "Quitting CLI mode...")
            break
        let args = line.string.unindent.splitWhitespace
        case args[0]:
            of "exit": break
            of "help":
                echo doc
                continue
            of "clear":
                clearScreen()
                continue
            of "config":
                handleConfigCmd(toOpenArray(args, 0, len(args) - 1))
            else:
                echo doc

