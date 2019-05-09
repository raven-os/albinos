#import gintro/[gtk, glib, gobject, gio, gtksource]
import config/config
import libalbinos/albinos

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


proc main =
    config.loadConfig()
    var cfg : Config
    var cfgp = addr cfg
    var cstr: cstring = "lol"
    discard createConfig(cstr, cfgp.addr)

main()
