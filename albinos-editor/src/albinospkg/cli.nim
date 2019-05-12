import terminal
import strutils
import rdstdin
import linenoise
import cmd
import ../libalbinos/albinos

const help_cmd_msg = "\t\e[93mhelp\e[39m (show this message)\n\t"
const exit_cmd_msg = "\e[93mexit\e[39m (quitting the app)\n\t"
const clear_cmd_msg = "\e[93mclear\e[39m (clear the screen)\n\t"
const config_create_cmd_msg = "\e[93mconfig\e[39m \e[94mcreate\e[39m <\e[95mname\e[39m> (create a config with the given name)"
const global_help = "Usage:\n" & help_cmd_msg & exit_cmd_msg & clear_cmd_msg &
    config_create_cmd_msg



proc yes(question: string): bool =
    echo question, " (\e[93my\e[39m/\e[93mN\e[39m)"
    while true:
        var line: TaintedString
        let res = readLineFromStdin("Answer: ", line)
        if res == false:
            styledEcho(fgMagenta, "Question Aborted, (assuming no).")
            return false
        case line:
            of "y", "Y", "yes", "Yes": return true
            of "n", "N", "no", "No": return false
            else: echo "Please be clear: yes or no, you write ", line, "."

proc handleCreateConfigCmd(args: openArray[string]) =
    styledEcho "Creating configuration ", fgMagenta, args[1], fgWhite, "..."
    let (cfg, result) = createCfg(args[1])
    case result:
        of ReturnedValue.SUCCESS:
            var regularKey: Key
            var readOnlyKey: Key
            discard getConfigKey(cfg, addr regularKey)
            discard getReadOnlyConfigKey(cfg, addr readOnlyKey)
            styledEcho "Successfuly created configuration ", fgMagenta, args[
                    1], fgWhite
            styledEcho "Regular key: ", fgGreen, $cast[cstring](regularKey.data)
            styledEcho "Read only key: ", fgGreen, $cast[cstring](readOnlyKey.data)
        else:
            echo result
            return
    if yes("Do you want to load the configuration: " & "\e[35m" & args[
            1] & "\e[39m" & " ?"):
        echo "Loading configuration"

proc handleConfigCmd(configCmdArgs: openArray[string]) =
    case len(configCmdArgs):
        of 3:
            case configCmdArgs[1]:
                of "create":
                    handleCreateConfigCmd(configCmdArgs[1..2])
                else:
                    echo global_help
        else:
            echo global_help

proc cliLoop() =
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
                echo global_help
                continue
            of "clear":
                clearScreen()
                continue
            of "config":
                handleConfigCmd(toOpenArray(args, 0, len(args) - 1))
            else:
                echo global_help


proc launchCLI*() =
    discard historySetMaxLen(500)
    cliLoop()
