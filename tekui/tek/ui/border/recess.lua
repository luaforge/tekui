
--
--	tek.ui.class.border.recess
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--

local ui = require "tek.ui"
local unpack = unpack

module("tek.ui.border.recess", tek.ui.class.border)
_VERSION = "RecessBorder 2.0"

-------------------------------------------------------------------------------
--	Constants & Class data:
-------------------------------------------------------------------------------

local DEF_BORDER = { 2, 2, 2, 2 }

-------------------------------------------------------------------------------
-- Class implementation:
-------------------------------------------------------------------------------

local Recess = _M

function Recess:getBorder(element, border)
	return unpack(border or element.Display.Theme.BorderRecessBorder or
		DEF_BORDER)
end

function Recess:draw(element, border, r1, r2, r3, r4)
	local b1, b2, b3, b4 = self:getBorder(element, border)
	local d = element.Drawable
	local p1, p2
	if element.Focus then
		p1, p2 = d.Pens[ui.PEN_FOCUSSHADOW], d.Pens[ui.PEN_FOCUSSHINE]
	else
		p1, p2 = d.Pens[ui.PEN_HALFSHADOW], d.Pens[ui.PEN_HALFSHINE]
	end
	d:fillRect(r1 - b1, r2 - b2, r3 + b3, r2 - 1, p1)
	d:fillRect(r3 + 1, r2, r3 + b3, r4 + b4, p2)
	d:fillRect(r1 - b1, r4 + 1, r3 + b3, r4 + b4, p2)
	d:fillRect(r1 - b1, r2, r1 - 1, r4, p1)
end
