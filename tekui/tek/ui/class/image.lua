
--
--	tek.ui.class.image
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--

local db = require "tek.lib.debug"
local ui = require "tek.ui"
local Gadget = ui.Gadget
local floor = math.floor
local max = math.max

module("tek.ui.class.image", tek.ui.class.gadget)
_VERSION = "Image 1.6"

-------------------------------------------------------------------------------
--	Constants & Class data:
-------------------------------------------------------------------------------

local DEF_IMAGEMARGIN = { 0, 0, 0, 0 }

-------------------------------------------------------------------------------
-- Class implementation:
-------------------------------------------------------------------------------

local Image = _M

function Image.init(self)
	self.DrawRect = { 0, 0, 0, 0 }
	self.ImageMargin = self.ImageMargin or false
	return Gadget.init(self)
end

-------------------------------------------------------------------------------
--	show:
-------------------------------------------------------------------------------

function Image:show(display, drawable)
	local theme = display.Theme
	self.ImageMargin = self.ImageMargin or DEF_IMAGEMARGIN
	return Gadget.show(self, display, drawable)
end

-------------------------------------------------------------------------------
--	askMinMax:
-------------------------------------------------------------------------------

function Image:askMinMax(m1, m2, m3, m4)
	local m = self.ImageMargin
	m1 = m1 + m[1] + m[3]
	m2 = m2 + m[2] + m[4]
	m3 = m3 + m[1] + m[3]
	m4 = m4 + m[2] + m[4]
	return Gadget.askMinMax(self, m1, m2, m3, m4)
end

-------------------------------------------------------------------------------
--	layout:
-------------------------------------------------------------------------------

function Image:layout(x0, y0, x1, y1, markdamage)
	if Gadget.layout(self, x0, y0, x1, y1, markdamage) then
		local r = self.Rect
		local p = self.PaddingAndBorder
		local m = self.ImageMargin
		local w = r[3] - r[1] - p[1] - p[3] - m[1] - m[3] + 1
		local h = r[4] - r[2] - p[2] - p[4] - m[2] - m[3] + 1
		local x = r[1] + p[1] + m[1]
		local y = r[2] + p[2] + m[2]
		local d = self.DrawRect
		d[1] = x
		d[2] = y
		d[3] = x + w - 1
		d[4] = y + h - 1
		return true
	end
end

-------------------------------------------------------------------------------
--	draw:
-------------------------------------------------------------------------------

function Image:draw()
	Gadget.draw(self)
	if self.Image then
		self.Image:draw(self.Drawable, self.DrawRect)
	end
end
