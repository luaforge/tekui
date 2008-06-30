
--
--	tek.ui.class.plasma
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--

local ui = require "tek.ui"
local db = require "tek.lib.debug"
local Frame = ui.Area

local cos = math.cos
local floor = math.floor
local max = math.max
local min = math.min
local pi = math.pi
local sin = math.sin
local unpack = unpack

module("tek.ui.class.plasma", tek.ui.class.area)
_VERSION = "Plasma 1.0"

local WIDTH = 80
local HEIGHT = 60
local PIXWIDTH = 6
local PIXHEIGHT = 6

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

local Plasma = _M

function Plasma:addgradient(sr, sg, sb, dr, dg, db, num)
	dr = (dr - sr) / (num - 1)
	dg = (dg - sg) / (num - 1)
	db = (db - sb) / (num - 1)
	local pal = self.Palette
	local pali = self.PalIndex
	for i = 0, num - 1 do
		pal[pali] = floor(sr) * 65536 + floor(sg) * 256 + floor(sb)
		pali = pali + 1
		sr = sr + dr
		sg = sg + dg
		sb = sb + db
	end
	self.PalIndex = pali
end

function Plasma.new(class, self)
	self = self or { }
	self.Screen = { }
	self.Palette = { }
	self.PalIndex = 0
	self.SinTab = { }
	self.Params = { 0, 0, 0, 0, 0 } -- xp1, xp2, yp1, yp2, yp3
	return Frame.new(class, self)
end

function Plasma.init(self)
	addgradient(self, 209,219,155, 79,33,57, 68)
	addgradient(self, 79,33,57, 209,130,255, 60)
	local sintab = self.SinTab
	for i = 0, 1023 do
		sintab[i] = sin(i / 1024 * pi * 2)
	end
	self.MinWidth = WIDTH * PIXWIDTH
	self.MinHeight = HEIGHT * PIXHEIGHT
	self.MaxWidth = WIDTH * PIXWIDTH
	self.MaxHeight = HEIGHT * PIXHEIGHT
	self.IntervalNotify = { self, "update" }
	return Frame.init(self)
end

function Plasma:show(display, drawable)
	if Frame.show(self, display, drawable) then
		self.Window:addNotify("Interval", ui.NOTIFY_ALWAYS,
			self.IntervalNotify)
		return true
	end
end

function Plasma:hide()
	Frame.hide(self)
	self.Window:remNotify("Interval", ui.NOTIFY_ALWAYS, self.IntervalNotify)
end

function Plasma:draw()

	local sintab = self.SinTab
	local palette = self.Palette
	local screen = self.Screen
	local pscale = #self.Palette / 10
	local p = self.Params
	local xp1, xp2, yp1, yp2, yp3 = unpack(p)
	local yc1, yc2, yc3 = yp1, yp2, yp3
	local c

	for y = 0, HEIGHT - 1 do
		local xc1, xc2 = xp1, xp2
		local ysin = sintab[yc1] + sintab[yc2] + sintab[yc3] + 5
		for x = y * WIDTH, (y + 1) * WIDTH - 1 do
			c = sintab[xc1] + sintab[xc2] + ysin
			screen[x] = palette[floor(c * pscale)]
			xc1 = (xc1 - 12) % 1024
			xc2 = (xc2 + 13) % 1024
		end
		yc1 = (yc1 + 8) % 1024
		yc2 = (yc2 + 11) % 1024
		yc3 = (yc3 - 18) % 1024
	end

	local d = self.Drawable
	local r = self.Rect
	d:drawRGB(r[1], r[2], screen, WIDTH, HEIGHT, 6, 6)

end

function Plasma:update()
	local p = self.Params
	local xp1, xp2, yp1, yp2, yp3 = unpack(p)
	yp1 = (yp1 - 9) % 1024
	yp2 = (yp2 + 4) % 1024
	yp3 = (yp3 + 5) % 1024
	xp1 = (xp1 + 7) % 1024
	xp2 = (xp2 - 2) % 1024
	p[1], p[2], p[3], p[4], p[5] = xp1, xp2, yp1, yp2, yp3
	self.Redraw = true
end
