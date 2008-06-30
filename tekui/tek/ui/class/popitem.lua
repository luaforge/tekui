-------------------------------------------------------------------------------
--
--	tek.ui.class.popitem
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	LINEAGE::
--		[[#ClassOverview]] :
--		[[#tek.class : Class]] /
--		[[#tek.class.object : Object]] /
--		[[#tek.ui.class.element : Element]] /
--		[[#tek.ui.class.area : Area]] /
--		[[#tek.ui.class.frame : Frame]] /
--		[[#tek.ui.class.gadget : Gadget]] /
--		[[#tek.ui.class.text : Text]] /
--		PopItem
--
--	OVERVIEW::
--		This class provides an anchorage for popups. This also works
--		recursively, i.e. elements of the PopItem class may contain other
--		PopItems as their children. The most notable child class of the
--		PopItem is the [[#tek.ui.class.menuitem : MenuItem]].
--
--	ATTRIBUTES::
--		- {{Children [I]}} (table)
--			Array of child objects - will be connected to the application
--			while the popup is open.
--		- {{Shortcut [IG]}} (string)
--			Keyboard shortcut for the object; unlike
--			[[#tek.ui.class.gadget : Gadget]].KeyCode, this shortcut is
--			also enabled while the object is invisible. By convention, only
--			combinations with a qualifier should be used here, e.g.
--			"Alt+C", "Shift+Ctrl+Q". Qualifiers are separated by "+" and
--			must precede the key. Valid qualifiers are:
--				- "Alt", "LAlt", "RAlt"
--				- "Shift", "LShift", "RShift"
--				- "Ctrl", "LCtrl", "RCtrl"
--				- "IgnoreCase"
--			Alias names for keys are
--				- "F1" ... "F12" (function keys),
--				- "Left", "Right", "Up", "Down" (cursor keys)
--				- "BckSpc", "Tab", "Esc", "Insert", "Overwrite",
--				"PageUp", "PageDown", "Pos1", "End", "Print", "Scroll",
--				and "Pause".
--
--	OVERRIDES::
--		- Element:cleanup()
--		- Object.init()
--		- Gadget:onPress()
--		- Area:passMsg()
--		- Element:setup()
--
-------------------------------------------------------------------------------

local db = require "tek.lib.debug"
local ui = require "tek.ui"
local PopupWindow = ui.PopupWindow
local Text = ui.Text

local ipairs = ipairs
local max = math.max

module("tek.ui.class.popitem", tek.ui.class.text)
_VERSION = "PopItem 4.0"

-------------------------------------------------------------------------------
--	Constants and class data:
-------------------------------------------------------------------------------

local DEF_POPUPFADEINDELAY = 6
local DEF_POPUPFADEOUTDELAY = 10

local NOTIFY_SUBMENU = { ui.NOTIFY_SELF, "submenu", ui.NOTIFY_VALUE }
local NOTIFY_ONSELECT = { ui.NOTIFY_SELF, "selectPopup" }
local NOTIFY_ONUNSELECT = { ui.NOTIFY_SELF, "unselectPopup" }
local NOTIFY_ONRELEASE = { ui.NOTIFY_SELF, "onPress", false }
local NOTIFY_ONRELEASEITEM = { ui.NOTIFY_SELF, "setValue", "Pressed", false }

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

local PopItem = _M

function PopItem.init(self)
	self.PopupBase = false
	self.PopupWindow = false
	self.DelayedBeginPopup = false
	self.DelayedEndPopup = false
	if self.Children then
		self.Mode = "toggle"
		self.FocusNotification = { self, "unselectPopup" }
	else
		self.Mode = "button"
	end
	self.Shortcut = self.Shortcut or false
	self.Width = "fill"
	return Text.init(self)
end

-------------------------------------------------------------------------------
--	setup: overrides
-------------------------------------------------------------------------------

function PopItem:setup(app, window)
	Text.setup(self, app, window)
	if window:getClass() ~= PopupWindow then
		self:connectPopItems(app, window)
	end
end

-------------------------------------------------------------------------------
--	cleanup: overrides
-------------------------------------------------------------------------------

function PopItem:cleanup()
	local app, window = self.Application, self.Window
	if self.Window:getClass() ~= PopupWindow then
		self:disconnectPopItems(self.Window)
	end
	Text.cleanup(self)
	-- restore application and window, as they are needed in
	-- popitems' notification handlers even when they are not visible:
	self.Application, self.Window = app, window
end

-------------------------------------------------------------------------------
--	show: overrides
-------------------------------------------------------------------------------

function PopItem:show(display, drawable)
	local theme = display.Theme
	self.FontSpec = self.FontSpec or theme.PopItemFontSpec or false
	self.Margin = self.Margin or theme.PopItemMargin or false
	self.Border = self.Border or theme.PopItemBorder or false
	self.IBorder = self.IBorder or theme.PopItemIBorder or false
	self.Padding = self.Padding or theme.PopItemPadding or false
	self.BorderStyle = self.BorderStyle or
		theme.PopItemChildrenBorderStyle or "socket"
	self.IBorderStyle = self.IBorderStyle or
		theme.PopItemChildrenIBorderStyle or "button"
	return Text.show(self, display, drawable)
end

-------------------------------------------------------------------------------
--	calcPopup:
-------------------------------------------------------------------------------

function PopItem:calcPopup()
	local _, _, x, y = self.Drawable:getAttrs()
	local w
	local r = self.Rect
	if self.PopupBase then
		x =	x + r[3]
		y = y + r[2]
	else
		x =	x + r[1]
		y = y + r[4]
		w = r[3] - r[1] + 1
	end
	return x, y, w
end

-------------------------------------------------------------------------------
--	beginPopup:
-------------------------------------------------------------------------------

function PopItem:beginPopup()

	local theme = self.Display.Theme

	local winx, winy, winw, winh = self:calcPopup()

	if self.Window.ActivePopup then
		db.info("Killed active popup")
		self.Window.ActivePopup:endPopup()
	end

	-- prepare children for being used in a popup window:
	for _, c in ipairs(self.Children) do
		c:init()
		c.Selected = false
		c.Focus = false
		if c:checkDescend(PopItem) then
			c.PopupBase = self.PopupBase or self
		end
		c:setState()
	end

	self.PopupWindow = PopupWindow:new
	{
		-- window in which the popup cascade is rooted:
		PopupRootWindow = self.Window.PopupRootWindow or self.Window,
		-- item in which this popup window is rooted:
		PopupBase = self.PopupBase or self,
		Children = self.Children,
		Orientation = "vertical",
		Left = winx,
		Top = winy,
		Width = winw,
		Height = winh,
	}

	-- connect children recursively:
	ui.Application.connect(self.PopupWindow)

	self.Window.ActivePopup = self

	self.Application:addMember(self.PopupWindow)

	self.PopupWindow:setValue("Status", "show")

	self.Window:addNotify("Status", "hide", self.FocusNotification)
	self.Window:addNotify("WindowFocus", ui.NOTIFY_CHANGE,
		self.FocusNotification)

end

-------------------------------------------------------------------------------
--	endPopup:
-------------------------------------------------------------------------------

function PopItem:endPopup()
	self:setValue("Selected", false, false) -- must not invoke notification!
	self:setValue("Focus", false)
	self:setState()
	self.Window:remNotify("WindowFocus", ui.NOTIFY_CHANGE,
		self.FocusNotification)
	self.Window:remNotify("Status", "hide", self.FocusNotification)
	self.PopupWindow:setValue("Status", "hide")
	self.Application:remMember(self.PopupWindow)
	self.Window.ActivePopup = false
	self.PopupWindow = false
end

-------------------------------------------------------------------------------
--	unselectPopup:
-------------------------------------------------------------------------------

function PopItem:unselectPopup()
	db.trace("unselectpopup: %s", self:getClassName())
	if self.PopupWindow then
		self:endPopup()
		self.Window:setActiveElement()
	end
end

function PopItem:passMsg(msg)
	if msg[2] == ui.MSG_MOUSEBUTTON then
		if msg[3] == 1 then -- leftdown:
			if self.PopupWindow and self.Window.ActiveElement ~= self and
				not self.PopupBase and self.Window.HoverElement == self then
				self:endPopup()
				-- swallow event, don't let ourselves get reactivated:
				return false
			end
		elseif msg[3] == 2 then -- leftup:
			if self.PopupWindow and self.Window.HoverElement ~= self and
				not self.Disabled then
				self:endPopup()
			end
		end
	end
	return Text.passMsg(self, msg)
end


function PopItem:submenu(val)
	-- check if not the baseitem:
	if self.PopupBase then
		self.Window.DelayedBeginPopup = false
		if val == true then
			if not self.PopupWindow then
				db.trace("Begin beginPopup delay")
				self.Window.BeginPopupTicks = DEF_POPUPFADEINDELAY
				self.Window.DelayedBeginPopup = self
			elseif self.Window.DelayedEndPopup == self then
				self.Window.DelayedEndPopup = false
			end
		elseif val == false and self.PopupWindow then
			db.trace("Begin endPopup delay")
			self.Window.BeginPopupTicks = DEF_POPUPFADEOUTDELAY
			self.Window.DelayedEndPopup = self
		end
	end
end

-------------------------------------------------------------------------------
--	selectPopup:
-------------------------------------------------------------------------------

function PopItem:selectPopup()
	if self.Children then
		if not self.PopupWindow then
			self:beginPopup()
		end
		if self.PopupBase then
			self.Selected = false
			self.Redraw = true
		end
	end
end

-------------------------------------------------------------------------------
--	onPress:
-------------------------------------------------------------------------------

function PopItem:onPress(pressed)
	if not pressed and self.PopupBase then
		-- unselect base item, causing the tree to collapse:
		self.PopupBase:setValue("Selected", false)
	end
end

-------------------------------------------------------------------------------
--	connectPopItems:
-------------------------------------------------------------------------------

function PopItem:connectPopItems(app, window)
	if self:checkDescend(PopItem) then
		db.info("adding popitem %s", self:getClassName())
		if self.Children then
			self:addNotify("Hilite", ui.NOTIFY_CHANGE, NOTIFY_SUBMENU)
			self:addNotify("Selected", true, NOTIFY_ONSELECT)
			self:addNotify("Selected", false, NOTIFY_ONUNSELECT)
			for _, child in ipairs(self.Children) do
				connectPopItems(child, app, window)
			end
		else
			if self.Shortcut then
				window:addKeyShortcut("IgnoreCase+" .. self.Shortcut, self)
			end
			self.Application = app
			self.Window = window
			self:addNotify("Active", false, NOTIFY_ONRELEASEITEM)
			self:addNotify("Pressed", false, NOTIFY_ONRELEASE)
		end
	end
end

-------------------------------------------------------------------------------
--	disconnectPopItems:
-------------------------------------------------------------------------------

function PopItem:disconnectPopItems(window)
	if self:checkDescend(PopItem) then
		db.info("removing popitem %s", self:getClassName())
		if self.Children then
			for _, child in ipairs(self.Children) do
				disconnectPopItems(child, window)
			end
			self:remNotify("Selected", false, NOTIFY_ONUNSELECT)
			self:remNotify("Selected", true, NOTIFY_ONSELECT)
			self:remNotify("Hilite", ui.NOTIFY_CHANGE, NOTIFY_SUBMENU)
		else
			if self.Shortcut then
				window:remKeyShortcut(self.Shortcut, self)
			end
			self:remNotify("Pressed", false, NOTIFY_ONRELEASE)
			self:remNotify("Active", false, NOTIFY_ONRELEASEITEM)
		end
	end
end
