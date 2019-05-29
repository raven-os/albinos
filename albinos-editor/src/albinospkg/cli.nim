import tables
import nimcx
import terminal
import strutils
import rdstdin
import linenoise
import cmd
import ../libalbinos/albinos
import ../ext/replxx
import util
import regex

var currentConfig: ptr Config

const
   prompt = "\x1b[1;32malbinos-editor\x1b[0m> "
   history_file = "albinos-editor_history.txt"
   regexColorRegistry = {
      "\\(": REPLXX_COLOR_BRIGHTRED,
      "\\)": REPLXX_COLOR_BRIGHTRED,
      "\\\"": REPLXX_COLOR_BLUE,
      "\\\"": REPLXX_COLOR_BLUE,
      "\\exit": REPLXX_COLOR_GREEN,
      "\\clear": REPLXX_COLOR_GREEN,
      "\\help": REPLXX_COLOR_GREEN,
      "\\config": REPLXX_COLOR_YELLOW,
      "\\create": REPLXX_COLOR_BRIGHTBLUE,
      "[\\-|+]{0,1}[0-9]+": REPLXX_COLOR_YELLOW,
      "\".*?\"": REPLXX_COLOR_BRIGHTGREEN
   }.toTable
   englishTable = {
    "usage": "Usage:",
    "help_cmd_msg": "help (show this message)",
    "exit_cmd_msg": "exit (quitting the app)",
    "clear_cmd_msg": "clear (clear the screen)",
    "config_create_cmd_msg": "config create <name> (create a config with the given name)",
    "config_load_cmd_msg": "config load <file> (load config from the key in the given file)",
    "setting_update_cmd_msg": "setting update <name> <value> (update setting with the given name to the given value)"
   }.toTable
let cmd_registry = @["help", "exit", "clear", "config create ",
      "config load ", "setting update "]

var msgTable = englishTable

proc globalHelpMsg() =
   var nocolor: bool = false
   println(msgTable["usage"], bgr = bgDefault)
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
   printLn(" (create a config with the given name)", bgr = bgDefault)
   printBiCol("config load", colLeft = if nocolor ==
         true: termwhite else: yellow, colRight = if nocolor ==
         true: termwhite else: dodgerblue,
         sep = " ", xpos = 8)
   printHL("<file>", substr = "file", col = if nocolor ==
         true: termwhite else: magenta)
   printLn(" (load config from the key in the given file)", bgr = bgDefault)
   printBiCol("setting update", colLeft = if nocolor ==
         true: termwhite else: yellow, colRight = if nocolor ==
         true: termwhite else: dodgerblue,
         sep = " ", xpos = 8)
   printHL("<name> ", substr = "name", col = if nocolor ==
         true: termwhite else: magenta)
   printHL("<value>", substr = "value", col = if nocolor ==
         true: termwhite else: magenta)
   printLn(" (update setting with the given name to the given value)", bgr = bgDefault)

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

proc handleLoadConfig(key: albinos.Key) =
   echo "Loading configuration"
   let err = albinos.getConfig(key, addr currentConfig)
   if err != albinos.SUCCESS:
      echo "Failed to load configuration"

proc handleLoadConfigFromFileCmd(args: openArray[string]) =
   let data = readFile(args[1])
   var key: albinos.Key
   key.data = data.string.cstring
   key.size = data.string.len
   handleLoadConfig(key)

proc handleCreateConfigCmd(args: openArray[string]) =
   var configName = args[1]
   if args[1].contains("\""):
      configName = configName.unescape
   styledEcho "Creating configuration ", fgMagenta, configName, fgWhite,
      "..."
   let (currentConfig, line) = createCfg(configName)
   case line:
      of ReturnedValue.SUCCESS:
         var
            regularKey: albinos.Key
            readOnlyKey: albinos.Key
         discard getConfigKey(currentConfig, addr regularKey)
         discard getReadOnlyConfigKey(currentConfig, addr readOnlyKey)
         styledEcho "Successfuly created configuration ", fgMagenta,
            configName, fgWhite
         styledEcho "Regular key: ", fgGreen,
                     $ cast[cstring](regularKey.data)
         styledEcho "Read only key: ", fgGreen, $ cast[cstring](
                     readOnlyKey.data)
         if yes("Do you want to load the configuration: " & "\e[35m" &
               configName & "\e[39m" & " ?"):
            handleLoadConfig(regularKey)
         else:
            styledEcho "Releasing configuration: ", fgMagenta, configName
            releaseConfig(currentConfig)
      else:
         echo line
         return


proc handleConfigCmd(configCmdArgs: openArray[string]) =
   case len(configCmdArgs):
      of 3:
         case configCmdArgs[1]:
            of "create":
               handleCreateConfigCmd(configCmdArgs[1..2])
            of "load":
               handleLoadConfigFromFileCmd(configCmdArgs[1..2])
            else:
               globalHelpMsg()
      else:
         globalHelpMsg()

proc handleUpdateSettingCmd(args: openArray[string]) =
   echo "Updating setting " & args[1] & " to " & args[2]
   var name = args[1];
   if name.contains("\""):
      name = name.unescape
   var value = args[2];
   if value.contains("\""):
      value = value.unescape
   discard albinos.setSetting(currentConfig, name, value)

proc handleSettingCmd(settingCmdArgs: openArray[string]) =
   echo settingCmdArgs
   case len(settingCmdArgs):
      of 4:
         case settingCmdArgs[1]:
            of "update":
               handleUpdateSettingCmd(settingCmdArgs[1..3])
            else:
               globalHelpMsg()
      else:
         globalHelpMsg()

proc cliLoop(repl: ptr Replxx) =
   while true:
      var cline = replxx_input(repl, prompt);
      if cline == nil:
         styledEcho(fgMagenta, "Quitting CLI mode...")
         break
      let line = $cline
      if line.len == 0 or isSpaceAscii(line):
         continue
      let args = line.unindent.parseCmdLine
      replxx_history_add(repl, cline)
      case args[0]:
         of "exit": break
         of "help":
            globalHelpMsg()
            continue
         of "clear": replxx_clear_screen(repl)
         of "config": handleConfigCmd(toOpenArray(args, 0, len(args) - 1))
         of "setting": handleSettingCmd(toOpenArray(args, 0, len(args) - 1))
         else: globalHelpMsg()

proc completionHook(input: cstring, completions: ptr replxx_completions,
      contextLen: ptr cint, userData: pointer) {.cdecl.} =
   var
      utf8ContextLen = context_len(input)
      prefixLen = input.len - utf8ContextLen;
   contextLen[] = utf8str_codepoint_len(input + prefixLen, utf8ContextLen);
   for idx, item in cmd_registry:
      if strncmp(input + prefixLen, item, utf8ContextLen) == 0:
         replxx_add_completion(completions, item)

proc hintHook(input: cstring, hints: ptr replxx_hints, contextLen: ptr cint,
      color: ptr ReplxxColor, userData: pointer) {.cdecl.} =
   var
      nbHints = 0
      utf8ContextLen = context_len(input)
      prefixLen = input.len - utf8ContextLen
   contextLen[] = utf8str_codepoint_len(input + prefixLen, utf8ContextLen);
   var sub = substr($input, prefixLen, input.len)
   if not sub.len == 0 or sub.len >= 2:
      for idx, item in cmd_registry:
         if strncmp(input + prefixLen, item, utf8ContextLen) == 0:
            replxx_add_hint(hints, item)
            nbHints += 1
   if nbHints == 1:
      color[] = REPLXX_COLOR_GREEN

proc highlighterHook(input: cstring, colors: ptr ReplxxColor,
                                      size: cint, userData: pointer) {.cdecl.} =
   for key, currentColor in regexColorRegistry:
      for m in findAll($input, re(key)):
         for i in m.boundaries.a .. m.boundaries.b:
            (colors + i)[] = currentColor

proc launchCLI*() =
   var repl = replxx_init()
   discard replxx_install_window_change_handler(repl)
   discard replxx_history_load(repl, history_file)
   replxx_set_word_break_characters(repl, " \t.,-%!;:=*~^'\"/?<>|[](){}")
   replxx_set_complete_on_empty(repl, 1)
   replxx_set_completion_callback(repl, cast[ptr replxx_completion_callback_t](
         completionHook), cast[pointer](cmd_registry.unsafeAddr))
   replxx_set_hint_callback(repl, cast[ptr replxx_hint_callback_t](hintHook), cast[
         pointer](cmd_registry.unsafeAddr))
   replxx_set_highlighter_callback(repl,
         cast[ptr replxx_highlighter_callback_t](
         highlighterHook), nil)
   globalHelpMsg()
   cliLoop(repl)
   discard replxx_history_save(repl, history_file)
   replxx_end(repl);
