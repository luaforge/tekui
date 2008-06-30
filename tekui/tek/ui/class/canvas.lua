-------------------------------------------------------------------------------
--
--	tek.ui.class.canvas
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	LINEAGE::
--		[[#ClassOverview]] :
--		[[#tek.class : Class]] /
--		[[#tek.class.object : Object]] /
--		[[#tek.ui.class.element : Element]] /
--		[[#tek.ui.class.area : Area]] /
--		Canvas
--
--	OVERVIEW::
--		This class implements a scrollable area acting as a container
--		for a child element. Currently, this class is used exclusively
--		by the [[#tek.ui.class.scrollgroup : ScrollGroup]] class.
--
--	ATTRIBUTES::
--		- {{AutoHeight [IG]}} (boolean)
--			The height of the canvas is automatically adapted to the height
--			the canvas is layouted into. [Default: '''false''']
--		- {{AutoWidth [IG]}} (boolean)
--			The width of the canvas is automatically adapted to the width
--			the canvas is layouted into. [Default: '''false''']
--		- {{CanvasHeight [ISG]}} (number)
--			The height of the canvas (in pixels)
--		- {{CanvasLeft [ISG]}} (number)
--			Left visible offset of the canvas (in pixels)
--		- {{CanvasWidth [ISG]}} (number)
--			The width of the canvas (in pixels)
--		- {{CanvasTop [ISG]}} (number)
--			Top visible offset of the canvas (in pixels)
--		- {{Child [IG]}} (object)
--			The element being contained by the Canvas for scrolling
--		- {{KeepMinHeight [IG]}} (boolean)
--			Report the minimum height of the Canvas's child object as the
--			Canvas' minimum height on the display
--		- {{KeepMinWidth [IG]}} (boolean)
--			Report the minimum width of the Canvas's child object as the
--			Canvas' minimum width on the display
--		- {{UnusedRegion [G]}} ([[#tek.lib.region : Region]])
--			Region of the Canvas which isn't covered by its {{Child}}
--		- {{VScrollStep [IG]}} (number)
--			Vertical scroll step (used for mousewheel)
--
--	IMPLEMENTS::
--		- Canvas:updateUnusedRegion()
--
--	OVERRIDES::
--		- Area:askMinMax()
--		- Element:cleanup()
--		- Element:connect()
--		- Element:disconnect()
--		- Area:draw()
--		- Area:getElement()
--		- Area:getElementByXY()
--		- Area:hide()
--		- Object.init()
--		- Area:layout()
--		- Area:markDamage()
--		- Area:passMsg()
--		- Area:refresh()
--		- Area:relayout()
--		- Element:setup()
--		- Area:show()
--
-------------------------------------------------------------------------------

local ui = require "tek.ui"
local Region = require "tek.lib.region"
local Area = ui.Area
local assert = assert
local max = math.max
local min = math.min
local unpack = unpack
local overlap = Region.overlapCoords

module("tek.ui.class.canvas", tek.ui.class.area)
_VERSION = "Canvas 9.0"
local Canvas = _M

local DEF_MARGIN = { 0, 0, 0, 0 }

-------------------------------------------------------------------------------
--	init: overrides
-------------------------------------------------------------------------------

function Canvas.init(self)
	self.AutoHeight = self.AutoHeight or false
	self.AutoWidth = self.AutoWidth or false
	self.CanvasHeight = self.CanvasHeight or 0
	self.CanvasLeft = self.CanvasLeft or 0
	self.CanvasTop = self.CanvasTop or 0
	self.CanvasWidth = self.CanvasWidth or 0
	self.KeepMinHeight = self.KeepMinHeight or false
	self.KeepMinWidth = self.KeepMinWidth or false
	self.Margin = self.Margin or DEF_MARGIN
	self.Child = self.Child or ui.Area:new { Margin = DEF_MARGIN }
	self.VScrollStep = self.VScrollStep or 10
	self.TempMsg = { }
	-- track intra-area damages, so that they can be applied to child object:
	self.TrackDamage = true
	self.UnusedRegion = false
	return Area.init(self)
end

-------------------------------------------------------------------------------
--	connect: overrides
-------------------------------------------------------------------------------

function Canvas:connect(parent)
	-- this connects recursively:
	ui.Application.connect(self.Child, self)
	return Area.connect(self, parent)
end

-------------------------------------------------------------------------------
--	disconnect: overrides
-------------------------------------------------------------------------------

function Canvas:disconnect(parent)
	Area.disconnect(self, parent)
	return ui.Element.disconnect(self.Child, parent)
end

-------------------------------------------------------------------------------
--	setup: overrides
-------------------------------------------------------------------------------

function Canvas:setup(app, window)
	Area.setup(self, app, window)
	self.Child:setup(app, window)
end

-------------------------------------------------------------------------------
--	cleanup: overrides
-------------------------------------------------------------------------------

function Canvas:cleanup()
	self.Child:cleanup()
	Area.cleanup(self)
end

-------------------------------------------------------------------------------
--	show: overrides
-------------------------------------------------------------------------------

function Canvas:show(display, drawable)
	if self.Child:show(display, drawable) then
		if Area.show(self, display, drawable) then
			return true
		end
		self.Child:hide()
	end
end

-------------------------------------------------------------------------------
--	hide: overrides
-------------------------------------------------------------------------------

function Canvas:hide()
	self.Child:hide()
	Area.hide(self)
end

-------------------------------------------------------------------------------
--	askMinMax: overrides
-------------------------------------------------------------------------------

function Canvas:askMinMax(m1, m2, m3, m4)
	local o = self.Child
	local c1, c2, c3, c4 = o:askMinMax(0, 0, self.MaxWidth, self.MaxHeight)
	m1 = m1 + c1
	m2 = m2 + c2
	m3 = m3 + c3
	m4 = m4 + c4
	m1 = self.KeepMinWidth and m1 or 0
	m2 = self.KeepMinHeight and m2 or 0
	m3 = self.MaxWidth and max(self.MaxWidth, m1) or self.CanvasWidth
	m4 = self.MaxHeight and max(self.MaxHeight, m2) or self.CanvasHeight
	return Area.askMinMax(self, m1, m2, m3, m4)
end

-------------------------------------------------------------------------------
--	layout: overrides
-------------------------------------------------------------------------------

function Canvas:layout(r1, r2, r3, r4, markdamage)

	local sizechanged = false

	local m = self.MarginAndBorder
	local r = self.Rect
	local mm = self.Child.MinMax

	local res = Area.layout(self, r1, r2, r3, r4, markdamage)

	if self.AutoWidth then
		local w = r3 - r1 + 1 - m[1] - m[3]
		assert(mm[1], self.Child:getClassName())
		w = max(w, mm[1])
		w = mm[3] and min(w, mm[3]) or w
		if w ~= self.CanvasWidth then
			self:setValue("CanvasWidth", w)
			sizechanged = true
		end
	end

	if self.AutoHeight then
		local h = r4 - r2 + 1 - m[2] - m[4]
		h = max(h, mm[2])
		h = mm[4] and min(h, mm[4]) or h
		if h ~= self.CanvasHeight then
			self:setValue("CanvasHeight", h)
			sizechanged = true
		end
	end

	-- set shift (information needed in subsequent relayouts):
	local d = self.Drawable
	local sx = r[1] - self.CanvasLeft
	local sy = r[2] - self.CanvasTop
	d:setShift(sx, sy)

	-- relayout object until width and height settle in:
	-- TODO: break out if they don't settle in?
	local iw, ih
	repeat
		iw, ih = self.CanvasWidth, self.CanvasHeight
		if self.Child:layout(0, 0, iw - 1, ih - 1, sizechanged) then
			sizechanged = true
		end
	until self.CanvasWidth == iw and self.CanvasHeight == ih

	-- unset shift:
	d:setShift(-sx, -sy)

	-- propagate intra-area damages calculated in Area.layout to child object:
	local dr = self.DamageRegion
	if dr and markdamage ~= false then
		local sx = self.CanvasLeft - r[1]
		local sy = self.CanvasTop - r[2]
		for _, r in dr:getRects() do
			local r1, r2, r3, r4 = dr:getRect(r)
			-- mark as damage shifted into canvas space:
			self.Child:markDamage(r1 + sx, r2 + sy, r3 + sx, r4 + sy)
		end
	end

	if res or sizechanged or not self.UnusedRegion then
		self:updateUnusedRegion()
	end

	if res or sizechanged then
		self.Redraw = true
		return true
	end

end

-------------------------------------------------------------------------------
--	updateUnusedRegion(): Updates the {{UnusedRegion}} attribute, which
--	contains the Canvas' area which isn't covered by its {{Child}}.
-------------------------------------------------------------------------------

function Canvas:updateUnusedRegion()
	-- determine unused region:
	local m = self.MarginAndBorder
	local r = self.Rect
	self.UnusedRegion = Region.new(r[1] + m[1], r[2] + m[2], r[3] - m[3],
		r[4] - m[4])
	local o = self.Child.Rect
	self.UnusedRegion:subRect(o[1] + r[1], o[2] + r[2], o[3] + r[1],
		o[4] + r[2])
	self.Redraw = true
end

-------------------------------------------------------------------------------
--	relayout: overrides
-------------------------------------------------------------------------------

function Canvas:relayout(e, r1, r2, r3, r4)
	local res, changed = Area.relayout(self, e, r1, r2, r3, r4)
	if res then
		return res, changed
	end
	local d = self.Drawable
	local r = self.Rect
	local sx = r[1] - self.CanvasLeft
	local sy = r[2] - self.CanvasTop
	d:pushClipRect(r[1], r[2], r[3], r[4])
	d:setShift(sx, sy)
	res, changed = self.Child:relayout(e, r1, r2, r3, r4)
	d:setShift(-sx, -sy)
	d:popClipRect()
	return res, changed
end

-------------------------------------------------------------------------------
--	draw: overrides
-------------------------------------------------------------------------------

function Canvas:draw()
	local d = self.Drawable
	local f = self.UnusedRegion
	local p = d.Pens[self.Child.Background]
	for _, r in f:getRects() do
		local r1, r2, r3, r4 = f:getRect(r)
		d:fillRect(r1, r2, r3, r4, p)
	end
end

-------------------------------------------------------------------------------
--	refresh: overrides
-------------------------------------------------------------------------------

function Canvas:refresh()
	Area.refresh(self)
	local d = self.Drawable
	local r = self.Rect
	local sx = r[1] - self.CanvasLeft
	local sy = r[2] - self.CanvasTop
	d:pushClipRect(r[1], r[2], r[3], r[4])
	d:setShift(sx, sy)
	self.Child:refresh()
	d:setShift(-sx, -sy)
	d:popClipRect()
end

-------------------------------------------------------------------------------
--	markDamage: overrides
-------------------------------------------------------------------------------

function Canvas:markDamage(r1, r2, r3, r4)
	Area.markDamage(self, r1, r2, r3, r4)
	-- clip absolute:
	local r = self.Rect
	r1, r2, r3, r4 = overlap(r1, r2, r3, r4, r[1], r[2], r[3], r[4])
	if r1 then
		-- shift into canvas space:
		local sx = self.CanvasLeft - r[1]
		local sy = self.CanvasTop - r[2]
		self.Child:markDamage(r1 + sx, r2 + sy, r3 + sx, r4 + sy)
	end
end

-------------------------------------------------------------------------------
--	checkArea [internal]: checks if x, y are inside our own rectangle,
--	and if so, returns the canvas shift by x and y
-------------------------------------------------------------------------------

function Canvas:checkArea(x, y)
	local r = self.Rect
	if x >= r[1] and x <= r[3] and y >= r[2] and y <= r[4] then
		return r[1] - self.CanvasLeft, r[2] - self.CanvasTop
	end
end

-------------------------------------------------------------------------------
--	getElementByXY: overrides
-------------------------------------------------------------------------------

function Canvas:getElementByXY(x, y)
	local sx, sy = self:checkArea(x, y)
	return sx and self.Child:getElementByXY(x - sx, y - sy)
end

-------------------------------------------------------------------------------
--	passMsg: overrides
-------------------------------------------------------------------------------

function Canvas:passMsg(msg)
	local isover = self:checkArea(msg[4], msg[5])
	if isover then
		if msg[2] == ui.MSG_MOUSEBUTTON then
			local r = self.Rect
			local h = self.CanvasHeight - (r[4] - r[2] + 1)
			if msg[3] == 64 then -- wheelup
				self:setValue("CanvasTop",
					max(0, min(h, self.CanvasTop - self.VScrollStep)))
				return false -- absorb
			elseif msg[3] == 128 then -- wheeldown
				self:setValue("CanvasTop",
					max(0, min(h, self.CanvasTop + self.VScrollStep)))
				return false -- absorb
			end
		end
	end
	if isover or
		msg[2] == ui.MSG_MOUSEMOVE and self.Window.MovingElement then
		-- operate on copy of the input message:
		local r = self.Rect
		local m = self.TempMsg
		m[1], m[2], m[3], m[4], m[5], m[6] = unpack(msg)
		-- shift mouse position into canvas area:
		m[4] = m[4] - r[1] + self.CanvasLeft
		m[5] = m[5] - r[2] + self.CanvasTop
		self.Child:passMsg(m)
	end
	return msg
end

-------------------------------------------------------------------------------
--	getElement: overrides
-------------------------------------------------------------------------------

function Canvas:getElement(mode)
	if mode == "firstchild" then
		return self.Child
	elseif mode == "lastchild" then
		return self.Child
	elseif mode == "children" then
		return { self.Child }
	end
	return Area.getElement(self, mode)
end
