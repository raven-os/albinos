import tables
import nimcx
import terminal
import strutils
import rdstdin
import linenoise
import cmd
import ../libalbinos/albinos

var currentConfig: ptr Config

const englishTable =
   {
    "usage": "Usage:",
    "help_cmd_msg": "help (show this message)",
    "exit_cmd_msg": "exit (quitting the app)",
    "clear_cmd_msg": "clear (clear the screen)",
    "config_create_cmd_msg": "config create <name> (create a config with the given name)"
   }.toTable

var msgTable = englishTable

proc globalHelpMsg() =
   var nocolor: bool = false
   println(msgTable["usage"])
   printLnBiCol(msgTable["help_cmd_msg"],
         colLeft = if nocolor == true: termwhite else: yellow,
      colRight = termwhite, sep = " ", xpos = 8)
   printLnBiCol(msgTable["exit_cmd_msg"],
         colLeft = if nocolor == true: termwhite else: yellow,
         colRight = termwhite, sep = " ", xpos = 8)
   printLnBiCol(msgTable["clear_cmd_msg"],
         colLeft = if nocolor == true: termwhite else: yellow,
         colRight = termwhite, sep = " ", xpos = 8)
   printBiCol("config create", colLeft = if nocolor ==
         true: termwhite else: yellow, colRight = if nocolor ==
         true: termwhite else: dodgerblue,
         sep = " ", xpos = 8)
   printHL("<name>", substr = "name", col = if nocolor ==
         true: termwhite else: magenta)
   printLn(" (create a config with the given name)")

proc completion(word: cstring, lc: ptr Completions) {.cdecl.} =
   if word[0] == 'h':
      addCompletion(lc, "help")
   if word[0] == 'e':
      addCompletion(lc, "exit")
   if word[0] == 'c':
      addCompletion(lc, "config create")
      addCompletion(lc, "clear")

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
         else: echo "Please be clear: yes or no, you wrote ", line, "."

proc handleLoadConfigCmd() =
   echo "Loading configuration"

proc handleCreateConfigCmd(args: openArray[string]) =
   styledEcho "Creating configuration ", fgMagenta, args[1], fgWhite, "..."
   let (currentConfig, result) = createCfg(args[1])
   case result:
      of ReturnedValue.SUCCESS:
         var regularKey: albinos.Key
         var readOnlyKey: albinos.Key
         discard getConfigKey(currentConfig, addr regularKey)
         discard getReadOnlyConfigKey(currentConfig, addr readOnlyKey)
         styledEcho "Successfuly created configuration ", fgMagenta, args[
                    1], fgWhite
         styledEcho "Regular key: ", fgGreen,
                    $ cast[cstring](regularKey.data)
         styledEcho "Read only key: ", fgGreen, $ cast[cstring](
                    readOnlyKey.data)
      else:
         echo result
         return
   if yes("Do you want to load the configuration: " & "\e[35m" & args[
            1] & "\e[39m" & " ?"):
      handleLoadConfigCmd()

proc handleConfigCmd(configCmdArgs: openArray[string]) =
   case len(configCmdArgs):
      of 3:
         case configCmdArgs[1]:
            of "create":
               handleCreateConfigCmd(configCmdArgs[1..2])
            else:
               globalHelpMsg()
      else:
         globalHelpMsg()

proc cliLoop() =
   while true:
      var line: TaintedString
      let res = readLineFromStdin("> ", line)
      if res == false:
         styledEcho(fgMagenta, "Quitting CLI mode...")
         break
      if line.string.len == 0:
         continue
      let args = line.string.unindent.splitWhitespace
      case args[0]:
         of "exit": break
         of "help":
            globalHelpMsg()
            continue
         of "clear":
            clearScreen()
            continue
         of "config":
            handleConfigCmd(toOpenArray(args, 0, len(args) - 1))
         else:
            globalHelpMsg()


proc launchCLI*() =
   globalHelpMsg()
   discard historySetMaxLen(500)
   setCompletionCallback(cast[ptr CompletionCallback](completion))
   cliLoop()
