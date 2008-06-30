-------------------------------------------------------------------------------
--
--	tek.ui.class.gauge
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
--		Gauge
--
--	OVERVIEW::
--		This class implements a gauge for the visualization of
--		numerical values.
--
--	OVERRIDES::
--		- Area:askMinMax()
--		- Element:cleanup()
--		- Area:draw()
--		- Area:hide()
--		- Object.init()
--		- Area:layout()
--		- Numeric:onSetValue()
--		- Area:relayout()
--		- Area:setState()
--		- Element:setup()
--		- Area:show()
--
-------------------------------------------------------------------------------

local ui = require "tek.ui"
local Region = require "tek.lib.region"

local Frame = ui.Frame
local Numeric = ui.Numeric

local floor = math.floor
local max = math.max
local min = math.min
local unpack = unpack

module("tek.ui.class.gauge", tek.ui.class.numeric)
_VERSION = "Gauge 3.0"

-------------------------------------------------------------------------------
-- GaugeFill:
-------------------------------------------------------------------------------

local DEF_KNOBMARGIN = { 0, 0, 0, 0 }

local GaugeFill = Frame:newClass { _NAME = "_gaugefill" }

function GaugeFill.init(self)
	self.BorderStyle = "socket"
	self.IBorderStyle = ""
	self.Margin = DEF_KNOBMARGIN
	if self.Orientation == "vertical" then
		self.Padding = { 6, 0, 6, 0 }
	else
		self.Padding = { 0, 6, 0, 6 }
	end
	return Frame.init(self)
end

function GaugeFill:setState(bg)
	Frame.setState(self, bg or ui.PEN_FILL)
end

-------------------------------------------------------------------------------
-- Gauge:
-------------------------------------------------------------------------------

local Gauge = _M

function Gauge.init(self)
	self.Mode = "inert"
	self.Orientation = self.Orientation or "horizontal"
	self.Knob = self.Knob or GaugeFill:new { Orientation = self.Orientation }
	self.Width = self.Width or "fill"
	self.Height = self.Height or "fill"
	return Numeric.init(self)
end

-------------------------------------------------------------------------------
--	setup: overrides
-------------------------------------------------------------------------------

function Gauge:setup(app, window)
	Numeric.setup(self, app, window)
	self.Knob:setup(app, window)
end

-------------------------------------------------------------------------------
--	cleanup: overrides
-------------------------------------------------------------------------------

function Gauge:cleanup()
	self.Knob:cleanup()
	Numeric.cleanup(self)
end

-------------------------------------------------------------------------------
--	show: overrides
-------------------------------------------------------------------------------

function Gauge:show(display, drawable)
	local theme = display.Theme
	self.Margin = self.Margin or theme.GaugeMargin or false
	self.Border = self.Border or theme.GaugeBorder or false
	self.IBorder = self.IBorder or theme.GaugeIBorder or false
	self.Padding = self.Padding or theme.GaugePadding or false
	self.BorderStyle = self.BorderStyle or theme.GaugeBorderStyle or "recess"
	self.IBorderStyle = self.IBorderStyle or theme.GaugeIBorderStyle or ""
	self.Knob.Margin = self.Knob.Margin or theme.GaugeKnobMargin or false
	self.Knob.Border = self.Knob.Border or theme.GaugeKnobBorder or false
	self.Knob.IBorder = self.Knob.IBorder or theme.GaugeKnobIBorder or false
	self.Knob.BorderStyle = self.Knob.BorderStyle or
		theme.GaugeKnobBorderStyle or false
	self.Knob.IBorderStyle = self.Knob.IBorderStyle or
		theme.GaugeKnobIBorderStyle or false
	Numeric.show(self, display, drawable)
	self.Knob:show(display, drawable)
	return true
end

-------------------------------------------------------------------------------
--	hide: overrides
-------------------------------------------------------------------------------

function Gauge:hide()
	self.Knob:hide()
	Numeric.hide(self)
	return true
end

-------------------------------------------------------------------------------
--	askMinMax: overrides
-------------------------------------------------------------------------------

function Gauge:askMinMax(m1, m2, m3, m4)
	local w, h = self.Knob:askMinMax(0, 0, 0, 0)
	m1 = m1 + w
	m2 = m2 + h
	m3 = m3 + w
	m4 = m4 + h
	return Numeric.askMinMax(self, m1, m2, m3, m4)
end

-------------------------------------------------------------------------------
--	getKnobRect:
-------------------------------------------------------------------------------

function Gauge:getKnobRect()
	local r = self.Rect
	if r[1] >= 0 then
		local p = self.PaddingAndBorder
		local m = self.Knob.MarginAndBorder
		local km = self.Knob.MinMax
		local x0 = r[1] + p[1] + m[1]
		local y0 = r[2] + p[2] + m[2]
		local x1 = r[3] - p[3] - m[3]
		local y1 = r[4] - p[4] - m[4]
		local r = self.Max - self.Min
		if self.Orientation == "horizontal" then
			local w = x1 - x0 - km[1] + 1
			x1 = min(x1, x0 + floor((self.Value - self.Min) * w / r) + km[1])
		else
			local h = y1 - y0 - km[2] + 1
			y1 = min(y1, y0 + floor((self.Value - self.Min) * h / r) + km[2])
		end
		return x0 - m[1], y0 - m[2], x1 + m[3], y1 + m[4]
	end
end

-------------------------------------------------------------------------------
--	layout: overrides
-------------------------------------------------------------------------------

function Gauge:layout(r1, r2, r3, r4, markdamage)
	if Numeric.layout(self, r1, r2, r3, r4, markdamage) then
		local x0, y0, x1, y1 = self:getKnobRect()
		self.Knob:layout(x0, y0, x1, y1, markdamage)
		return true
	end
end

-------------------------------------------------------------------------------
--	relayout: overrides
-------------------------------------------------------------------------------

function Gauge:relayout(e, r1, r2, r3, r4)
	local res, changed = Numeric.relayout(self, e, r1, r2, r3, r4)
	if res then
		return res, changed
	end
	return self.Knob:relayout(e, r1, r2, r3, r4)
end

-------------------------------------------------------------------------------
--	draw: overrides
-------------------------------------------------------------------------------

function Gauge:draw()

	local d = self.Drawable
	local b1, b2, b3, b4 = self:getIBorder()
	local r = self.Rect

	local bg = Region.new(r[1] + b1, r[2] + b2, r[3] - b3, r[4] - b4)

	local x0, y0, x1, y1 = self:getKnobRect()
	local m = self.Knob.MarginAndBorder
	local kb1, kb2, kb3, kb4 = self.Knob:getBorder()
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

	self.Knob:draw()
	self.Knob:drawBorder()
end

-------------------------------------------------------------------------------
--	onSetValue: overrides
-------------------------------------------------------------------------------

function Gauge:onSetValue(v)
	Numeric.onSetValue(self, v)
	local x0, y0, x1, y1 = self:getKnobRect()
	if x0 then
		self.Window:relayout(self.Knob, x0, y0, x1, y1)
		self.Redraw = true
	end
end

-------------------------------------------------------------------------------
--	setState: overrides
-------------------------------------------------------------------------------

function Gauge:setState(bg)
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
