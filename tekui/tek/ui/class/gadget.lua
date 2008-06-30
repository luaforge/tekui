-------------------------------------------------------------------------------
--
--	tek.ui.class.gadget
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
--		Gadget
--
--	OVERVIEW::
--		This class implements interactivity.
--
--	ATTRIBUTES::
--		- {{Active [SG]}} (boolean)
--			Signifies a change of the Gadget's activation state. While active,
--			the position of the mouse pointer is being verified (which is also
--			reflected by the {{Hover}} attribute). When the {{Active}} state
--			changes, the Gadget's behavior depends on its {{Mode}} attribute:
--				* in "button" mode, the {{Selected}} attribute is set to
--				the value of the {{Hover}} attribute. The {{Pressed}} attribute
--				is set to the value of the {{Active}} attribute, if it caused a
--				change of the {{Selected}} state.
--				* in "toggle" mode, the {{Selected}} attribute of the
--				Gadget is logically toggled, and the {{Pressed}} attribute is
--				set to '''true'''.
--				* in "touch" mode, the {{Selected}} and {{Pressed}}
--				attributes are set to '''true''', if the Gadget wasn't selected
--				already.
--			Changing this attribute invokes the Gadget:onActivate() method.
--		- {{DblClick [SG]}} (boolean)
--			Signifies that the element was doubleclicked; is is set to
--			'''true''' when the element was doubleclicked and is still being
--			held, and '''false''' when it was doubleclicked and then released.
--			This attribute usually needs to get a notification handler attached
--			to it before it is useful.
--		- {{Disabled [ISG]}} (boolean)
--			Signifies a change of the Gadget's ability to interact with the
--			user. Invokes the Gadget:onDisable() method. When an element is
--			getting disabled, it loses its focus, too.
--		- {{Hilite [SG]}} (boolean)
--			Signifies a change of the Gadget's highligting state. Invokes
--			Gadget:onHilite().
--		- {{Hold [SG]}} (boolean)
--			Signifies that the element is being held. Invokes
--			Gadget:onHold().
--		- {{Hover [SG]}} (boolean)
--			Signifies a change of the Gadget being hovered by the pointing
--			device. Invokes Gadget:onHover().
--		- {{KeyCode [IG]}} (string)
--			If set, the keyboard equivalent that can be used to activate the
--			element while it is shown [default: '''false''']. See also
--			[[#tek.ui.class.popitem : PopItem]] for a discussion of
--			denoting qualifiers, which applies to the {{KeyCode}} attribute
--			as well.
--		- {{Mode [IG]}} (string)
--			Interaction mode of the Gadget; can be
--			- "inert": The element does not react to input,
--			- "toggle": The element does not rebound at all and keeps its
--			{{Selected}} state; it can't be unselected by the user.
--			- "touch": The element rebounds immediately and acts as a strobe,
--			submitting always '''true''' for {{Pressed}} and {{Selected}}.
--			- "button": The element sets the {{Pressed}} attribute only if
--			the mouse pointer is released when hovering it.
--			See also {{Active}}.
--		- {{Pressed [SG]}} (boolean)
--			Signifies that a button was pressed or released. Invokes
--			Gadget:onPress().
--		- {{Selected [ISG]}} (boolean)
--			Signifies a change of the gadget's selection state. Invokes
--			Gadget:onSelect().
--		- {{Style [IG]}} (string)
--			Visual style of the element; can be
--			- "normal": The element is a normal gadget
--			- "knob": The element is being used as a knob for a
--			[[#tek.ui.class.slider : Slider]]
--
--	IMPLEMENTS::
--		- Gadget:onActivate() - Handler for {{Active}}
--		- Gadget:onDisable() - Handler for {{Disabled}}
--		- Gadget:onHilite() - Handler for {{Hilite}}
--		- Gadget:onHold() - Handler for {{Hold}}
--		- Gadget:onHover() - Handler for {{Hover}}
--		- Gadget:onPress() - Handler for {{Pressed}}
--		- Gadget:onSelect() - Handler for {{Selected}}
--
--	OVERRIDES::
--		- Area:checkFocus()
--		- Element:cleanup()
--		- Area:hide()
--		- Object.init()
--		- Area:passMsg()
--		- Element:setup()
--		- Area:setState()
--		- Area:show()
--
-------------------------------------------------------------------------------

local db = require "tek.lib.debug"
local ui = require "tek.ui"
local Frame = ui.Frame

module("tek.ui.class.gadget", tek.ui.class.frame)
_VERSION = "Gadget 9.3"

local Gadget = _M

-------------------------------------------------------------------------------
--	Constants & Class data:
-------------------------------------------------------------------------------

local DEF_IBORDERSTYLE = "none"
local DEF_NULL = { 0, 0, 0, 0 }

local NOTIFY_HOVER = { ui.NOTIFY_SELF, "onHover", ui.NOTIFY_VALUE }
local NOTIFY_ACTIVE = { ui.NOTIFY_SELF, "onActivate", ui.NOTIFY_VALUE }
local NOTIFY_HILITE = { ui.NOTIFY_SELF, "onHilite", ui.NOTIFY_VALUE }
local NOTIFY_DISABLED = { ui.NOTIFY_SELF, "onDisable", ui.NOTIFY_VALUE }
local NOTIFY_SELECTED = { ui.NOTIFY_SELF, "onSelect", ui.NOTIFY_VALUE }
local NOTIFY_PRESSED = { ui.NOTIFY_SELF, "onPress", ui.NOTIFY_VALUE }
local NOTIFY_HOLD = { ui.NOTIFY_SELF, "onHold", ui.NOTIFY_VALUE }

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

function Gadget.init(self)
	-- Element has been activated:
	self.Active = false
	-- Element has been doubleclicked (or double-activated):
	self.DblClick = false
	-- Element is being held:
	self.Hold = false
	-- The pointer is hovering over the element:
	self.Hover = false
	-- keycode shortcut:
	self.KeyCode = self.KeyCode or false
	-- Mode of behavior ("inert", "toggle", "touch", "button"):
	self.Mode = self.Mode or "inert"
	-- Element is being "pressed":
	self.Pressed = false
	-- Style of the Gadget ("normal", "knob"):
	self.Style = self.Style or "normal"
	return Frame.init(self)
end

-------------------------------------------------------------------------------
--	setup: overrides
-------------------------------------------------------------------------------

function Gadget:setup(app, window)
	Frame.setup(self, app, window)
	self:addNotify("Disabled", ui.NOTIFY_CHANGE, NOTIFY_DISABLED)
	self:addNotify("Hilite", ui.NOTIFY_CHANGE, NOTIFY_HILITE)
	self:addNotify("Selected", ui.NOTIFY_CHANGE, NOTIFY_SELECTED)
	self:addNotify("Hover", ui.NOTIFY_CHANGE, NOTIFY_HOVER)
	self:addNotify("Active", ui.NOTIFY_CHANGE, NOTIFY_ACTIVE)
	self:addNotify("Pressed", ui.NOTIFY_CHANGE, NOTIFY_PRESSED)
	self:addNotify("Hold", ui.NOTIFY_ALWAYS, NOTIFY_HOLD)
end

-------------------------------------------------------------------------------
--	cleanup: overrides
-------------------------------------------------------------------------------

function Gadget:cleanup()
	self:remNotify("Hold", ui.NOTIFY_ALWAYS, NOTIFY_HOLD)
	self:remNotify("Pressed", ui.NOTIFY_CHANGE, NOTIFY_PRESSED)
	self:remNotify("Active", ui.NOTIFY_CHANGE, NOTIFY_ACTIVE)
	self:remNotify("Hover", ui.NOTIFY_CHANGE, NOTIFY_HOVER)
	self:remNotify("Selected", ui.NOTIFY_CHANGE, NOTIFY_SELECTED)
	self:remNotify("Hilite", ui.NOTIFY_CHANGE, NOTIFY_HILITE)
	self:remNotify("Disabled", ui.NOTIFY_CHANGE, NOTIFY_DISABLED)
	Frame.cleanup(self)
end

-------------------------------------------------------------------------------
--	show: overrides
-------------------------------------------------------------------------------

function Gadget:show(display, drawable)
	if self.KeyCode and self.Mode ~= "inert" then
		self.Window:addKeyShortcut(self.KeyCode, self)
	end
	local theme = display.Theme

	if self.Style == "knob" then
		self.Margin = self.Margin or DEF_NULL
		self.Padding = self.Padding or DEF_NULL
		self.BorderStyle = self.BorderStyle or "socket"
		self.IBorderStyle = self.IBorderStyle or "button"
	else
		-- outer spacing:
		self.Margin = self.Margin or theme.GadgetMargin or false
		-- outer border:
		self.Border = self.Border or theme.GadgetBorder or false
		-- inner border:
		self.IBorder = self.IBorder or theme.GadgetIBorder or false
		-- inner spacing:
		self.Padding = self.Padding or theme.GadgetPadding or false
		-- outer borderstyle:
		self.BorderStyle = self.BorderStyle or theme.GadgetBorderStyle or false
		-- inner borderstyle:
		self.IBorderStyle = self.IBorderStyle or theme.GadgetIBorderStyle or
			DEF_IBORDERSTYLE
	end
	return Frame.show(self, display, drawable)
end

-------------------------------------------------------------------------------
--	hide: overrides
-------------------------------------------------------------------------------

function Gadget:hide()
	Frame.hide(self)
	if self.KeyCode then
		self.Window:remKeyShortcut(self.KeyCode, self)
	end
end

-------------------------------------------------------------------------------
--	onHover(hovered): This method is invoked when the gadget's {{Hover}}
--	attribute has changed.
-------------------------------------------------------------------------------

function Gadget:onHover(hover)
	if self.Mode == "button" then
		self:setValue("Selected", self.Active and hover)
	end
	if self.Mode ~= "inert" then
		self:setValue("Hilite", hover)
	end
	self:setState()
end

-------------------------------------------------------------------------------
--	onActivate(active): This method is invoked when the gadget's {{Active}}
--	attribute has changed.
-------------------------------------------------------------------------------

function Gadget:onActivate(active)
	local mode, selected, dblclick = self.Mode, self.Selected
	if mode == "toggle" then
		if active then
			self:setValue("Selected", not selected)
			self:setValue("Pressed", true)
			dblclick = self
		end
	elseif mode == "touch" then
		if active and not selected then
			self:setValue("Selected", true)
			self:setValue("Pressed", true)
			dblclick = self
		end
	elseif mode == "button" then
		self:setValue("Selected", active and self.Hover)
		if not selected ~= not active then
			self:setValue("Pressed", active)
			dblclick = active and self
		end
	end
	if dblclick ~= nil then
		local win = self.Window
		if win then
			win:setDblClickElement(dblclick)
		end
	end
	self:setState()
end

-------------------------------------------------------------------------------
--	onDisable(disabled): This method is invoked when the gadget's {{Disabled}}
--	attribute has changed.
-------------------------------------------------------------------------------

function Gadget:onDisable(disabled)
	local win = self.Window
	if disabled and self.Focus and win then
		win:setFocusElement()
	end
	self:setState()
end

-------------------------------------------------------------------------------
--	onSelect(selected): This method is invoked when the gadget's {{Selected}}
--	attribute has changed.
-------------------------------------------------------------------------------

function Gadget:onSelect(selected)
	self:setState()
end

-------------------------------------------------------------------------------
--	onHilite(selected): This method is invoked when the gadget's {{Selected}}
--	attribute has changed.
-------------------------------------------------------------------------------

function Gadget:onHilite(hilite)
	self:setState()
end

-------------------------------------------------------------------------------
--	onPress(pressed): This method is invoked when the gadget's {{Pressed}}
--	attribute has changed.
-------------------------------------------------------------------------------

function Gadget:onPress(pressed)
end

-------------------------------------------------------------------------------
--	onHold(held): This method is invoked when the gadget's {{Hold}}
--	attribute is set. While the gadget is being held, repeated
--	{{Hold}} = '''true''' events are submitted in intervals of n/50 seconds
--	determined by the [[#tek.ui.class.window : Window]].HoldTickInitRepeat
--	attribute.
-------------------------------------------------------------------------------

function Gadget:onHold(hold)
end

-------------------------------------------------------------------------------
--	setState: overrides
-------------------------------------------------------------------------------

function Gadget:setState(bg)
	if not bg then
		if self.Disabled then
			bg = ui.PEN_BUTTONDISABLED
		elseif self.Selected then
			bg = ui.PEN_BUTTONACTIVE
		elseif self.Hilite then
			bg = ui.PEN_BUTTONOVER
		end
	end
	Frame.setState(self, bg)
end

-------------------------------------------------------------------------------
--	passMsg: overrides
-------------------------------------------------------------------------------

function Gadget:passMsg(msg)
	local win = self.Window
	if win then -- might be gone if in a PopupWindow
		local he = win.HoverElement
		he = he == self and not he.Disabled and he
		if msg[2] == ui.MSG_MOUSEBUTTON then
			if msg[3] == 1 then -- leftdown:
				if he then
					win:setHiliteElement(self)
					if self:checkFocus() then
						win:setFocusElement(self)
					end
					win:setActiveElement(self)
				end
			elseif msg[3] == 2 then -- leftup:
				if he then
					win:setHiliteElement()
					win:setHiliteElement(self)
				end
			end
		elseif msg[2] == ui.MSG_MOUSEMOVE then
			if win.HiliteElement == self or he and not win.MovingElement then
				win:setHiliteElement(he)
				return false
			end
		end
	end
	return msg
end

-------------------------------------------------------------------------------
--	checkFocus: overrides
-------------------------------------------------------------------------------

function Gadget:checkFocus()
	return not self.Disabled and self.Mode ~= "inert"
end
