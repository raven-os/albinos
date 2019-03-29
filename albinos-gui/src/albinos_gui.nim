import nigui

##! Require for gtk3
app.init()

##! Window configuration
var window = newWindow("Albinos GUI")
window.width = 420
window.height = 420

##! Layout
var container = newLayoutContainer(Layout_Vertical)
window.add(container)

##! Contents of the layout
var button = newButton("Button 1")
container.add(button)

var textArea = newTextArea()
container.add(textArea)

##! Callback
button.onClick = proc(event: ClickEvent) =

  textArea.addLine("Button 1 clicked, message box opened.")
  window.alert("This is a simple message box.")
  textArea.addLine("Message box closed.")

##! Enable Window
window.show()

##! Run application
app.run()
