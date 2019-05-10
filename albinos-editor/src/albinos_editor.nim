import libalbinos/albinos
include albinospkg/options


when isMainModule:
    let args = parseOpt()
    if args["config"]:
        echo "config options invoked"
    elif args["--cli"]:
        echo "CLI mode invoked"
    elif args["--gui"]:
        echo "GUI mode invoked"
        echo args

#import gintro/[gtk, glib, gobject, gio, gtksource]

#proc appActivate(app: Application) =
#  var builder = newBuilder()
#  discard addFromFile(builder, "example.glade")
#  let window = getApplicationWindow(builder, "window")
#  window.setApplication(app)
#  showAll(window)

#proc main =
#  loadConfig()
#  let app = newApplication("org.gtk.example")
#  connect(app, "activate", appActivate)
#  discard run(app)

    #var cfg : Config
    #var cfgp = addr cfg
    #var cstr: cstring = "mycfg"
    #discard createConfig(cstr, cfgp.addr)
