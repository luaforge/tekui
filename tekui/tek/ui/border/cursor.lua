
--
--	tek.ui.class.border.cursor
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--

local ui = require "tek.ui"
local unpack = unpack

module("tek.ui.border.cursor", tek.ui.class.border)
_VERSION = "Cursor Border 2.1"

-------------------------------------------------------------------------------
--	Constants & Class data:
-------------------------------------------------------------------------------

local DEF_BORDER = { 1, 1, 1, 1 }

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

local Cursor = _M

function Cursor:getBorder(element, border)
	return unpack(border or
		element.Display.Theme.BorderCursorBorder or DEF_BORDER)
end

function Cursor:draw(element, border, r1, r2, r3, r4)
	local b1, b2, b3, b4 = self:getBorder(element, border)
	local d = element.Drawable
	local p1 = d.Pens[ui.PEN_SHINE]
	d:fillRect(r1 - b1, r2 - b2, r3 + b3, r2 - 1, p1)
	d:fillRect(r3 + 1, r2, r3 + b3, r4 + b4, p1)
	d:fillRect(r1 - b1, r4 + 1, r3 + b3, r4 + b4, p1)
	d:fillRect(r1 - b1, r2, r1 - 1, r4, p1)
end
