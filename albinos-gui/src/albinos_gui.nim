import gintro/[gtk, glib, gobject, gio, gtksource]
import config/config

proc appActivate(app: Application) =
  var builder = newBuilder()
  discard addFromFile(builder, "example.glade")
  let window = getApplicationWindow(builder, "window")
  window.setApplication(app)
  showAll(window)

proc main =
  loadConfig()
  let app = newApplication("org.gtk.example")
  connect(app, "activate", appActivate)
  discard run(app)

main()
