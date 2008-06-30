-------------------------------------------------------------------------------
--
--	tek.ui.class.slider
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
--		[[#tek.ui.class.numeric : Numeric]] /
--		Slider
--
--	OVERVIEW::
--		This class implements a slider for adjusting a numerical value.
--
--	ATTRIBUTES::
--		- {{Child [IG]}} ([[#tek.ui.class.gadget : Gadget]])
--			A gadget for being used as the slider's knob. By default,
--			an internal knob gadget is used.
--		- {{ForceInteger [IG]}} (boolean)
--			If '''true''', integer steps are enforced. By default, the
--			slider moves continuously.
--		- {{Orientation [IG]}} (string)
--			Orientation of the slider, which can be "horizontal" or
--			"vertical". [Default: "horizontal"]
--		- {{Range [ISG]}} (number)
--			The range of the slider, i.e. the size it represents. Setting
--			this value invokes the Slider:onSetRange() method.
--		- {{Style [IG]}} (string)
--			Style of the slider:
--				- "scrollbar" - for scrollbars
--				- "number" - for adjusting numbers
--				- "normal" - unspecified
--			Default: "normal"
--
--	IMPLEMENTS::
--		- Slider:onSetRange() - Handler for the {{Range}} attribute
--
--	OVERRIDES:
--		- Element:cleanup()
--		- Area:draw()
--		- Area:hide()
--		- Object.init()
--		- Area:layout()
--		- Area:passMsg()
--		- Gadget:onFocus()
--		- Gadget:onHold()
--		- Area:relayout()
--		- Area:setState()
--		- Element:setup()
--		- Area:show()
--		- Numeric:onSetMax()
--		- Numeric:onSetValue()
--
-------------------------------------------------------------------------------

local ui = require "tek.ui"
local Region = require "tek.lib.region"

local Gadget = ui.Gadget
local Numeric = ui.Numeric

local floor = math.floor
local max = math.max
local min = math.min
local unpack = unpack

module("tek.ui.class.slider", tek.ui.class.numeric)
_VERSION = "Slider 6.4"

-------------------------------------------------------------------------------
--	SliderKnob:
-------------------------------------------------------------------------------

local SliderKnob = Gadget:newClass { Name = "_sliderknob" }

local RATIO = 0x23 -- of 0x100
local OFFS = 4

function SliderKnob:draw()
	Gadget.draw(self)
	if self.Style == "scrollbar" then
		local d = self.Drawable
		local r = self.Rect
		local p1 = d.Pens[ui.PEN_HALFSHADOW]
		local p2 = d.Pens[ui.PEN_HALFSHINE]
		if self.Parent.Orientation == "horizontal" then
			local w = r[3] - r[1] + 1
			local n = floor(w * RATIO / (OFFS * 0x100))
			n = min(6, max(n, 2))
			local x = r[1] + floor((w - (n - 1) * OFFS) / 2)
			local y = r[2] + floor((r[4] - r[2] + 1) / 2)
			for i = 1, n do
				d:drawLine(x - 1, y - 3, x, y - 3, p1)
				d:drawPlot(x - 1, y - 2, p1)
				d:drawPlot(x, y - 2, p2)
				d:drawLine(x - 1, y + 1, x, y + 1, p1)
				d:drawPlot(x - 1, y + 2, p1)
				d:drawPlot(x, y + 2, p2)
				x = x + OFFS
			end
		else
			local h = r[4] - r[2] + 1
			local n = floor(h * RATIO / (OFFS * 0x100))
			n = min(6, max(n, 2))
			local x = r[1] + floor((r[3] - r[1] + 1) / 2)
			local y = r[2] + floor((h - (n - 1) * OFFS) / 2)
			for i = 1, n do
				d:drawLine(x - 3, y, x - 2, y, p1)
				d:drawPlot(x - 3, y + 1, p1)
				d:drawPlot(x - 2, y + 1, p2)
				d:drawLine(x + 1, y, x + 2, y, p1)
				d:drawPlot(x + 1, y + 1, p1)
				d:drawPlot(x + 2, y + 1, p2)
				y = y + OFFS
			end
		end
	end
end

-------------------------------------------------------------------------------
--	Constants & Class data:
-------------------------------------------------------------------------------

local DEF_KNOBMARGIN = { 1, 1, 1, 1 }
local DEF_KNOBPADDING = { 4, 4, 4, 4 }

local NOTIFY_RANGE = { ui.NOTIFY_SELF, "onSetRange", ui.NOTIFY_VALUE }

-------------------------------------------------------------------------------
-- Class implementation:
-------------------------------------------------------------------------------

local Slider = _M

function Slider.init(self)
	self.Mode = "button"
	self.Orientation = self.Orientation or "horizontal"
	self.ForceInteger = self.ForceInteger or false
	self.Range = self.Range or false
	self.Child = self.Child or SliderKnob:new
	{
		IBorderStyle = "button",
		Margin = DEF_KNOBMARGIN,
		Padding = DEF_KNOBPADDING,
		Style = self.Style,
		Parent = self
	}
	self.Width = self.Width or "fill"
	self.Height = self.Height or "fill"
	self.Move0 = false
	self.Pos0 = 0
	self.HoldXY = { }
	self = Numeric.init(self)
	self.Range = max(self.Max, self.Range or self.Max)
	return self
end

-------------------------------------------------------------------------------
--	setup: overrides
-------------------------------------------------------------------------------

function Slider:setup(app, window)
	-- connect knob to our common parent:
	self.Child.Parent = self.Parent
	Numeric.setup(self, app, window)
	self:addNotify("Range", ui.NOTIFY_CHANGE, NOTIFY_RANGE, 1)
	self.Child:setup(app, window)
end

-------------------------------------------------------------------------------
--	cleanup: overrides
-------------------------------------------------------------------------------

function Slider:cleanup()
	self.Child:cleanup()
	self:remNotify("Range", ui.NOTIFY_CHANGE, NOTIFY_RANGE)
	Numeric.cleanup(self)
end

-------------------------------------------------------------------------------
--	show: overrides
-------------------------------------------------------------------------------

function Slider:show(display, drawable)
	local theme = display.Theme
	self.Margin = self.Margin or theme.SliderMargin or false
	self.Border = self.Border or theme.SliderBorder or false
	self.IBorder = self.IBorder or theme.SliderIBorder or false
	self.Padding = self.Padding or theme.SliderPadding or false
	self.BorderStyle = self.BorderStyle or theme.SliderBorderStyle or "recess"
	self.IBorderStyle = self.IBorderStyle or theme.SliderIBorderStyle or ""
	self.Child.Margin = self.Child.Margin or theme.SliderKnobMargin or false
	self.Child.Border = self.Child.Border or theme.SliderKnobBorder or false
	self.Child.IBorder = self.Child.IBorder or theme.SliderKnobIBorder or false
	self.Child.BorderStyle = self.Child.BorderStyle or
		theme.SliderKnobBorderStyle or false
	self.Child.IBorderStyle = self.Child.IBorderStyle or
		theme.SliderKnobIBorderStyle or false
	Numeric.show(self, display, drawable)
	self.Child:show(display, drawable)
	return true
end

-------------------------------------------------------------------------------
--	hide: overrides
-------------------------------------------------------------------------------

function Slider:hide()
	self.Child:hide()
	Numeric.hide(self)
	return true
end

-------------------------------------------------------------------------------
--	askMinMax: overrides
-------------------------------------------------------------------------------

function Slider:askMinMax(m1, m2, m3, m4)
	local w, h = self.Child:askMinMax(0, 0, 0, 0)
	if self.Orientation == "horizontal" then
		w = w + w
	else
		h = h + h
	end
	m1 = m1 + w
	m2 = m2 + h
	m3 = m3 + w
	m4 = m4 + h
	return Numeric.askMinMax(self, m1, m2, m3, m4)
end

-------------------------------------------------------------------------------
--	getKnobRect: internal
-------------------------------------------------------------------------------

function Slider:getKnobRect()

	local r = self.Rect
	if r[1] >= 0 then
		local p = self.PaddingAndBorder
		local m = self.Child.MarginAndBorder
		local km = self.Child.MinMax
		local x0 = r[1] + p[1] + m[1]
		local y0 = r[2] + p[2] + m[2]
		local x1 = r[3] - p[3] - m[3]
		local y1 = r[4] - p[4] - m[4]
		local r = self.Range - self.Min
		local v = self.Value
		v = self.ForceInteger and floor(v) or v
		if self.Orientation == "horizontal" then
			local w = x1 - x0 - km[1] + 1
			x0 = max(x0, x0 + floor((v - self.Min) * w / r))
			x1 = min(x1, x0 + floor((self.Range - self.Max) * w / r) +
				km[1] - 1)
		else
			local h = y1 - y0 - km[2] + 1
			y0 = max(y0, y0 + floor((v - self.Min) * h / r))
			y1 = min(y1, y0 + floor((self.Range - self.Max) * h / r) +
				km[2] - 1)
		end
		return x0 - m[1], y0 - m[2], x1 + m[3], y1 + m[4]
	end
end

-------------------------------------------------------------------------------
--	layout:
-------------------------------------------------------------------------------

function Slider:layout(r1, r2, r3, r4, markdamage)
	local res = Numeric.layout(self, r1, r2, r3, r4, markdamage)
	local x0, y0, x1, y1 = self:getKnobRect()
	return self.Child:layout(x0, y0, x1, y1, markdamage) or res
end

-------------------------------------------------------------------------------
--	relayout: overrides
-------------------------------------------------------------------------------

function Slider:relayout(e, r1, r2, r3, r4)
	local res, changed = Numeric.relayout(self, e, r1, r2, r3, r4)
	if res then
		return res, changed
	end
	return self.Child:relayout(e, r1, r2, r3, r4)
end

-------------------------------------------------------------------------------
--	draw: overrides
-------------------------------------------------------------------------------

function Slider:draw()
	local d = self.Drawable
	local b1, b2, b3, b4 = self:getIBorder()
	local r = self.Rect
	local bg = Region.new(r[1] + b1, r[2] + b2, r[3] - b3, r[4] - b4)
	local c = self.Child.Rect
	local x0, y0, x1, y1 = c[1], c[2], c[3], c[4]
	local m = self.Child.MarginAndBorder
	local kb1, kb2, kb3, kb4 = self.Child:getBorder()
	x0 = x0 + m[1] - kb1
	y0 = y0 + m[2] - kb2
	x1 = x1 - m[3] + kb3
	y1 = y1 - m[4] + kb4
	bg:subRect(x0, y0, x1, y1)
	local bgpen = d.Pens[self.Background]
	for _, r in bg:getRects() do
		local r1, r2, r3, r4 = bg:getRect(r)
		d:fillRect(r1, r2, r3, r4, bgpen)
	end
	self.Child:drawBorder()
	self.Child:draw()
end

function Slider:clickContainer(xy)
	local b1, b2, b3, b4 = self.Child:getBorder()
	if self.Orientation == "horizontal" then
		local cx = self.Step
		if xy[1] < self.Child.Rect[1] - b1 then
			self:decrease(self.Step)
		elseif xy[1] > self.Child.Rect[3] + b3 then
			self:increase(self.Step)
		end
	else
		local cy = self.Step
		if xy[2] < self.Child.Rect[2] - b2 then
			self:decrease(self.Step)
		elseif xy[2] > self.Child.Rect[4] + b4 then
			self:increase(self.Step)
		end
	end
end

-------------------------------------------------------------------------------
--	onHold: overrides
-------------------------------------------------------------------------------

function Slider:onHold(hold)
	if hold and not self.Move0 then
		if self.HoldXY[1] then
			self:clickContainer(self.HoldXY)
		end
	end
	Numeric.onHold(self, hold)
end

function Slider:startMove(x, y)
	local b1, b2, b3, b4 = self.Child:getBorder()
	if x >= self.Child.Rect[1] - b1 and x <= self.Child.Rect[3] + b3 and
		y >= self.Child.Rect[2] - b2 and y <= self.Child.Rect[4] + b4 then
	 	self.Move0 = { x, y }
	 	self.Pos0 = self.Value
		return self
	end
	return false
end

function Slider:doMove(x, y)
	local r = self.Rect
	local m = self.Child.MarginAndBorder
	local newv
	local km = self.Child.MinMax
	if self.Orientation == "horizontal" then
		local w = r[3] - r[1] - m[3] - m[1] - km[1] + 1
		newv = self.Pos0 +
			(x - self.Move0[1]) * (self.Range - self.Min) / max(w, 1)
	else
		local h = r[4] - r[2] - m[4] - m[2] - km[2] + 1
		newv = self.Pos0 +
			(y - self.Move0[2]) * (self.Range - self.Min) / max(h, 1)
	end
	if self.ForceInteger then
		newv = floor(newv)
	end
	self:setValue("Value", newv)
end

-------------------------------------------------------------------------------
--	updateslider: internal
-------------------------------------------------------------------------------

local function updateslider(self)
	local x0, y0, x1, y1 = self:getKnobRect()
	if x0 then
		self.Window:relayout(self.Child, x0, y0, x1, y1)
		self.Redraw = true
	end
end

-------------------------------------------------------------------------------
--	onSetValue: overrides
-------------------------------------------------------------------------------

function Slider:onSetValue(v)
	Numeric.onSetValue(self, v)
	updateslider(self)
end

-------------------------------------------------------------------------------
--	onSetMax: overrides
-------------------------------------------------------------------------------

function Slider:onSetMax(m)
	Numeric.onSetMax(self, m)
	updateslider(self)
end

-------------------------------------------------------------------------------
--	Slider:onSetRange(range): This handler is invoked when the Slider's
--	{{Range}} attribute has changed.
-------------------------------------------------------------------------------

function Slider:onSetRange(r)
	updateslider(self)
end

-------------------------------------------------------------------------------
--	setState: overrides
-------------------------------------------------------------------------------

function Slider:setState(bg)
	if not bg then
		if self.Disabled then
			bg = ui.PEN_BUTTONDISABLED
		elseif self.Selected then
			bg = ui.PEN_SLIDERACTIVE
		elseif self.Hilite then
			bg = ui.PEN_SLIDEROVER
		else
			bg = ui.PEN_SLIDERBACK
		end
	end
	return Numeric.setState(self, bg)
end

-------------------------------------------------------------------------------
--	passMsg: overrides
-------------------------------------------------------------------------------

function Slider:passMsg(msg)
	local win = self.Window
	if win then
		if msg[2] == ui.MSG_MOUSEBUTTON then
			if msg[3] == 1 then -- leftdown:
				if win.HoverElement == self and not self.Disabled then
					if self:startMove(msg[4], msg[5]) then
						win:setMovingElement(self)
					else
						-- otherwise the container was clicked:
						self.HoldXY[1] = msg[4]
						self.HoldXY[2] = msg[5]
						self:clickContainer(self.HoldXY)
					end
				end
			elseif msg[3] == 2 then -- leftup:
				if win.MovingElement == self then
					win:setMovingElement()
				end
				self.Move0 = false
			end
		elseif msg[2] == ui.MSG_MOUSEMOVE then
			if win.MovingElement == self then
				self:doMove(msg[4], msg[5])
				-- do not pass message to other elements while dragging:
				return false
			end
		end
	end
	return Numeric.passMsg(self, msg)
end

-------------------------------------------------------------------------------
--	handleInput:
-------------------------------------------------------------------------------

function Slider:handleInput(msg)
	if msg[2] == ui.MSG_KEYDOWN then
		if self.Orientation == "horizontal" then
			if msg[3] == 0xf010 then -- left
				self:decrease()
				return false
			elseif msg[3] == 0xf011 then -- right
				self:increase()
				return false
			end
		else
			if msg[3] == 0xf012 then -- up
				self:decrease()
				return false
			elseif msg[3] == 0xf013 then -- down
				self:increase()
				return false
			end
		end
	end
	return msg
end

-------------------------------------------------------------------------------
--	onFocus: overrides
-------------------------------------------------------------------------------

function Slider:onFocus(focused)
	if focused then
		self.Window:addInputHandler(self, Slider.handleInput)
	else
		self.Window:remInputHandler(self, Slider.handleInput)
	end
	Numeric.onFocus(self, focused)
end
