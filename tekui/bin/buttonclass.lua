#!/usr/bin/env lua

ui = require "tek.ui"


Button = ui.Text:newClass { _NAME = "_button" }

function Button.init(self)
	self.Style = "button"
	self.Mode = self.Mode or "button"
	return ui.Text.init(self)
end


app = ui.Application:new()

win = ui.Window:new { Title = "Hello" }

button = Button:new { Text = "_Hello, World!" }

button:addNotify("Pressed", false, {
	ui.NOTIFY_SELF,
	ui.NOTIFY_FUNCTION,
	function(self)
		print "Hello, World!"
	end
})

app:addMember(win)

win:addMember(button)

app:run()
