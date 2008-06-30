-------------------------------------------------------------------------------
--
--	tek.ui.class.drawable
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	LINEAGE::
--		[[#ClassOverview]] :
--		[[#tek.class : Class]] /
--		[[#tek.class.object : Object]] /
--		Drawable
--
--	OVERVIEW::
--		This class implements a graphical context which can be painted on.
--
--	IMPLEMENTS::
--		- Drawable:fillRect() - Draws a filled rectangle
--		- Drawable:drawRect() - Draws an unfilled rectangle
--		- Drawable:drawText() - Renders text
--		- Drawable:drawLine() - Draws a line
--		- Drawable:drawPlot() - Draws a point
--		- Drawable:setFont() - Sets a font
--		- Drawable:getTextSize() - Determines the width and height of text
--		- Drawable:getMsg() - Gets the next pending input message
--		- Drawable:pushClipRect() - Push a new cliprect on the drawable
--		- Drawable:popClipRect() - Pop the topmost cliprect
--		- Drawable:setShift() - Add coordinate displacement
--		- Drawable:getShift() - Get current displacement
--		- Drawable:copyArea() - Copy area
--
--	OVERRIDES::
--		- Object.init()
--
-------------------------------------------------------------------------------

local ui = require "tek.ui"
local Object = require "tek.class.object"
local Region = require "tek.lib.region"

local assert = assert
local ipairs = ipairs
local unpack = unpack
local insert = table.insert
local remove = table.remove
local min = math.min
local max = math.max
local overlap = Region.overlapCoords
local HUGE = ui.HUGE

module("tek.ui.class.drawable", tek.class.object)
_VERSION = "Drawable 7.0"

DELAY = 0.003

-------------------------------------------------------------------------------
-- Class implementation:
-------------------------------------------------------------------------------

local Drawable = _M

function Drawable.init(self)
	assert(self.Display)
	self.Visual = false
	self.Pens = { }
	self.Left = false
	self.Top = false
	self.Width = false
	self.Height = false
	self.AspectX = 1
	self.AspectY = 1
	self.ShiftX = 0
	self.ShiftY = 0
	self.ClipStack = { }
	self.ClipRect = { }
	self.RectPool = { }
	self.DebugPen1 = false
	self.DebugPen2 = false
	return Object.init(self)
end

function Drawable:open(title, w, h, minw, minh, maxw, maxh, x, y, center,
	fulls)
	assert(not w or w > 0)
	assert(not h or h > 0)
	assert(not minw or minw >= 0)
	assert(not minh or minh >= 0)
	assert(not maxw or maxw > 0)
	assert(not maxh or maxh > 0)
	if not self.Visual then

		self.Visual = self.Display:openVisual
		{
			Title = title,
			Width = w,
			Height = h,
			Left = x,
			Top = y,
			MinWidth = minw,
			MinHeight = minh,
			MaxWidth = maxw,
			MaxHeight = maxh,
			Borderless = (x or y) and true,
			Center = center,
			Fullscreen = fulls,
		}

		self.DebugPen1 = self.Visual:allocpen(255, 255, 0)
		self.DebugPen2 = self.Visual:allocpen(0, 0, 0)

		self.Visual:setinput("close", "keydown", "keyup", "newsize",
			"mousebutton", "refresh", "mousemove", "mouseover", "interval",
			"focus")

		local penalloc = { }
		for i, v in ipairs(self.Display.Theme.RGBTab) do
			penalloc[i] = self.Visual:allocpen(unpack(v))
		end

		local pentab = self.Display.Theme.PenTab
		for i = 1, #pentab do
			self.Pens[i] = penalloc[pentab[i]]
		end

		return true
	end
end

function Drawable:close()
	if self.Visual then
		self:getAttrs()
		self.Visual:close()
		self.Visual = false
		return true
	end
end

-------------------------------------------------------------------------------
--	Drawable:fillRect(x0, y0, x1, y1, pen): Draws a filled rectangle.
-------------------------------------------------------------------------------

function Drawable:fillRect_normal(...)
	self.Visual:frect(...)
end

-------------------------------------------------------------------------------
--	Drawable:drawRect(x0, y0, x1, y1, pen): Draws an unfilled rectangle.
-------------------------------------------------------------------------------

function Drawable:drawRect_normal(...)
	self.Visual:rect(...)
end

-------------------------------------------------------------------------------
--	Drawable:drawText(x0, y0, text, fgpen[, bgpen]): Renders text
--	with the specified foreground pen. If the optional background pen is
--	specified, the background under the text is filled in this color.
-------------------------------------------------------------------------------

function Drawable:drawText_normal(...)
	self.Visual:text(...)
end

function Drawable:drawImage(...)
	self.Visual:drawimage(...)
end

function Drawable:drawRGB(...)
	self.Visual:drawrgb(...)
end

-------------------------------------------------------------------------------
--	Drawable:drawLine(x0, y0, x1, y1, pen): Draws a line using the specified
--	pen.
-------------------------------------------------------------------------------

function Drawable:drawLine_normal(...)
	self.Visual:line(...)
end

-------------------------------------------------------------------------------
--	Drawable:drawPlot(x0, y0, pen): Draws a point.
-------------------------------------------------------------------------------

function Drawable:drawPlot_normal(...)
	self.Visual:plot(...)
end

-------------------------------------------------------------------------------
--	Drawable:setFont(font): Sets the specified font.
-------------------------------------------------------------------------------

function Drawable:setFont(...)
	self.Visual:setfont(...)
end

-------------------------------------------------------------------------------
--	width, height = Drawable:getTextSize(text): Determines the width and height
--	of a text using the font which is currently set on the Drawable.
-------------------------------------------------------------------------------

function Drawable:getTextSize(...)
	return self.Visual:textsize(...)
end

-------------------------------------------------------------------------------
--	msg = Drawable:getMsg(): Gets the next pending message from the Drawable.
-------------------------------------------------------------------------------

function Drawable:getMsg()
	return self.Visual:getmsg()
end

function Drawable:setAttrs(...)
	self.Visual:setattrs(...)
end

function Drawable:getAttrs()
	self.Width, self.Height, self.Left, self.Top =
		self.Visual:getattrs()
	return self.Width, self.Height, self.Left, self.Top
end

-------------------------------------------------------------------------------
--	Drawable:pushClipRect(x0, y0, x1, y1): Pushes a new cliprect on the top
--	of the drawable's stack of cliprects.
-------------------------------------------------------------------------------

function Drawable:pushClipRect(x0, y0, x1, y1)
	local cr = self.ClipRect
	local sx = self.ShiftX
	local sy = self.ShiftY
	x0 = x0 + sx
	y0 = y0 + sy
	x1 = x1 + sx
	y1 = y1 + sy
	local r = remove(self.RectPool) or { }
	r[1], r[2], r[3], r[4] = x0, y0, x1, y1
	insert(self.ClipStack, r)
	if cr[1] then
		x0, y0, x1, y1 = overlap(x0, y0, x1, y1, cr[1], cr[2], cr[3], cr[4])
		if not x0 then
			x0, y0, x1, y1 = -1, -1, -1, -1
		end
	end
	cr[1], cr[2], cr[3], cr[4] = x0, y0, x1, y1
	self.Visual:setcliprect(x0, y0, x1 - x0 + 1, y1 - y0 + 1)
end

-------------------------------------------------------------------------------
--	Drawable:popClipRect(): Pop the topmost cliprect from the Drawable.
-------------------------------------------------------------------------------

function Drawable:popClipRect()
	local cs = self.ClipStack
	local cr = self.ClipRect
	insert(self.RectPool, remove(cs))
	local x0, y0, x1, y1
	if #cs > 0 then
		x0, y0, x1, y1 = 0, 0, HUGE, HUGE
		for i = 1, #cs do
			x0, y0, x1, y1 = overlap(x0, y0, x1, y1, unpack(cs[i]))
			if not x0 then
				x0, y0, x1, y1 = -1, -1, -1, -1
				break
			end
		end
	end
	cr[1], cr[2], cr[3], cr[4] = x0, y0, x1, y1
	if x0 then
		self.Visual:setcliprect(x0, y0, x1 - x0 + 1, y1 - y0 + 1)
	else
		self.Visual:unsetcliprect()
	end
end

-------------------------------------------------------------------------------
--	Drawable:setShift(deltax, deltay): Add a delta to the Drawable's
--	coordinate displacement.
-------------------------------------------------------------------------------

function Drawable:setShift(dx, dy)
	self.ShiftX = self.ShiftX + dx
	self.ShiftY = self.ShiftY + dy
	return self.Visual:setshift(dx, dy)
end

-------------------------------------------------------------------------------
--	shiftx, shifty = Drawable:getShift(): Get the Drawable's current
--	coordinate displacement.
-------------------------------------------------------------------------------

function Drawable:getShift()
	return self.ShiftX, self.ShiftY
end

-------------------------------------------------------------------------------
--	Drawable:copyArea(x0, y0, x1, y1, deltax, deltay, exposures): Copy the
--	specified rectangle to the position determined by the relative
--	coordinates {{deltax}} and {{deltay}}. The {{exposures}} argument is a
--	table used for collecting the raw coordinates of rectangles which got
--	exposed as a result of the copy operation.
-------------------------------------------------------------------------------

function Drawable:copyArea(x0, y0, x1, y1, dx, dy, t)
	self.Visual:copyarea(x0, y0, x1 - x0 + 1, y1 - y0 + 1, dx, dy, t)
end

function Drawable:fillRect_debug(...)
	local x0, y0, x1, y1, p = ...
	self.Visual:frect(x0, y0, x1, y1, self.DebugPen1)
	self.Display:sleep(DELAY)
	self.Visual:frect(x0, y0, x1, y1, self.DebugPen2)
	self.Display:sleep(DELAY)
	self.Visual:frect(x0, y0, x1, y1, p)
end

function Drawable:drawRect_debug(...)
	local x0, y0, x1, y1, p = ...
	self.Visual:rect(x0, y0, x1, y1, self.DebugPen1)
	self.Display:sleep(DELAY)
	self.Visual:rect(x0, y0, x1, y1, self.DebugPen2)
	self.Display:sleep(DELAY)
	self.Visual:rect(x0, y0, x1, y1, p)
end

function Drawable:drawLine_debug(...)
	local x0, y0, x1, y1, p = ...
	self.Visual:line(x0, y0, x1, y1, self.DebugPen1)
	self.Display:sleep(DELAY)
	self.Visual:line(x0, y0, x1, y1, self.DebugPen2)
	self.Display:sleep(DELAY)
	self.Visual:line(x0, y0, x1, y1, p)
end

function Drawable:drawPlot_debug(...)
	local x0, y0, p = ...
	self.Visual:plot(x0, y0, self.DebugPen1)
	self.Display:sleep(DELAY)
	self.Visual:plot(x0, y0, self.DebugPen2)
	self.Display:sleep(DELAY)
	self.Visual:plot(x0, y0, p)
end

function Drawable:drawText_debug(...)
	local x0, y0, text, p1, p2 = ...
	self.Visual:text(x0, y0, text, self.DebugPen1, p2 and self.DebugPen2)
	self.Display:sleep(DELAY)
	self.Visual:text(x0, y0, text, self.DebugPen2, p2 and self.DebugPen1)
	self.Display:sleep(DELAY)
	self.Visual:text(x0, y0, text, p1, p2)
end

function Drawable.enableDebug(enabled)
	if enabled then
		fillRect = fillRect_debug
		drawRect = drawRect_debug
		drawText = drawText_debug
		drawLine = drawLine_debug
		drawPlot = drawPlot_debug
	else
		fillRect = fillRect_normal
		drawRect = drawRect_normal
		drawText = drawText_normal
		drawLine = drawLine_normal
		drawPlot = drawPlot_normal
	end
end

enableDebug(ui.DEBUG)
