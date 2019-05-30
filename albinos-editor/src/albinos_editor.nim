import terminal
import docopt
import albinospkg/gui
import albinospkg/cli
import albinospkg/options

when isMainModule:
    let args = parseOpt()
    if args["config"]:
        echo "config options invoked"
    elif args["--cli"]:
        if args["--load-config"]:
            launchCLI($args["--load-config"])
        else:
            launchCLI()
    elif args["--gui"]:
        styledEcho(fgCyan, "Launching GUI")
        launchGUI()

    #var cfg : Config
    #var cfgp = addr cfg
    #var cstr: cstring = "mycfg"
    #discard createConfig(cstr, cfgp.addr)
