
--
--	tek.ui.border.group
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--

local ui = require "tek.ui"
local Region = require "tek.lib.region"

local floor = math.floor

module("tek.ui.border.group", tek.ui.class.border)
_VERSION = "GroupBorder 3.1"

-------------------------------------------------------------------------------
--	Constants & Class data:
-------------------------------------------------------------------------------

local DEF_BORDER = { 1, 1, 1, 1 }

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

local Group = _M

local function getsizes(e, b)
	local tw, th
	b = b or e.Display.Theme.BorderGroupBorder or DEF_BORDER
	if e and e.Legend and e.LegendFont then
		tw, th = ui.Display:getTextSize(e.LegendFont, e.Legend)
	end
	return b[1], b[2], b[3], b[4], tw, th
end

function Group:getBorder(element, border)
	local b1, b2, b3, b4, _, th = getsizes(element, border)
	return b1, th or b2, b3, b4
end

function Group:draw(element, border, r1, r2, r3, r4)
	local b1, b2, b3, b4, tw, th = getsizes(element, border)
	local d = element.Drawable

	local p1, p2
	if element.Focus then
		p1, p2 = d.Pens[ui.PEN_FOCUSSHINE], d.Pens[ui.PEN_FOCUSSHADOW]
	else
		p1, p2 = d.Pens[ui.PEN_HALFSHADOW], d.Pens[ui.PEN_SHADOW]
	end

	d:fillRect(r1 - b1, r2 - b2, r3 + b3, r2 - 1, p1)
	d:fillRect(r3 + 1, r2, r3 + b3, r4 + b4, p2)
	d:fillRect(r1 - b1, r4 + 1, r3 + b3, r4 + b4, p2)
	d:fillRect(r1 - b1, r2, r1 - 1, r4, p1)
	if tw then
		local w = r3 - r1 + 1
		local tx = r1 + floor((w - tw) / 2)
		d:setFont(element.LegendFont)
		d:pushClipRect(r1 - b1, r2 - th, r3 + b3, r2 - 1)
		d:drawText(tx, r2 - th, element.Legend, d.Pens[ui.PEN_GROUPLABELTEXT],
			d.Pens[element.Background or ui.PEN_GROUPBACK])
		d:popClipRect()
	end
end

function Group:getRegion(element, border, x0, y0, x1, y1)
	local b1, b2, b3, b4, tw, th = getsizes(element, border)
	local b = Region.new(x0 - b1, y0 - b2, x1 + b3, y1 + b4)
	if tw then
		local w = x1 - x0 + 1
		local tx = x0 + floor((w - tw) / 2)
		b:orRect(tx, y0 - th, tx + tw - 1, y0 - 1)
	end
	b:subRect(x0, y0, x1, y1)
	return b
end
