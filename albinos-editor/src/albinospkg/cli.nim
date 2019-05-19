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
   let (currentConfig, line) = createCfg(args[1])
   case line:
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
         echo line
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


const prompt = "\x1b[1;32malbinos-editor\x1b[0m$ "
const history_file = "albinos-editor_history.txt"
let cmd_registry = @["help", "exit", "clear", "config create "]

proc cliLoop(repl: ptr Replxx) =
   while true:
      var cline = replxx_input(repl, prompt);
      if cline == nil:
         styledEcho(fgMagenta, "Quitting CLI mode...")
         break
      var line = $cline
      if line.len == 0:
         continue
      let args = line.unindent.splitWhitespace
      replxx_history_add(repl, cline)
      case args[0]:
         of "exit": break
         of "help":
            globalHelpMsg()
            continue
         of "clear": replxx_clear_screen(repl)
         of "config": handleConfigCmd(toOpenArray(args, 0, len(args) - 1))
         else: globalHelpMsg()

func `+`*(p: cstring, offset: int): type(p) {.inline.} =
   ## Pointer increment
   {.emit: "`result` = `p` + `offset`;".}


proc strncmp(s1: cstring, s2: cstring, n: csize): cint
   {.header: "<string.h>", importc: "strncmp".}


proc completionHook(input: cstring, completions: ptr replxx_completions,
      contextLen: ptr cint, userData: pointer) {.cdecl.} =
   var utf8ContextLen = context_len(input)
   var prefixLen = input.len - utf8ContextLen;
   contextLen[] = utf8str_codepoint_len(input + prefixLen, utf8ContextLen);
   for idx, item in cmd_registry:
      if strncmp(input + prefixLen, item, utf8ContextLen) == 0:
         replxx_add_completion(completions, item)

proc hintHook(input: cstring, hints: ptr replxx_hints, contextLen: ptr cint,
      color: ptr ReplxxColor, userData: pointer) {.cdecl.} =
   var utf8ContextLen = context_len(input)
   var prefixLen = input.len - utf8ContextLen;
   contextLen[] = utf8str_codepoint_len(input + prefixLen, utf8ContextLen);
   var sub: string = substr($input, prefixLen, input.len)
   if not sub.len == 0 or sub.len >= 2:
      for idx, item in cmd_registry:
         if strncmp(input + prefixLen, item, utf8ContextLen) == 0:
            replxx_add_hint(hints, item)

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
   globalHelpMsg()
   cliLoop(repl)
   discard replxx_history_save(repl, history_file)
   replxx_end(repl);
