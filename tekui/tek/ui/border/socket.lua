
--
--	tek.ui.class.border.socket
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--

local ui = require "tek.ui"
local unpack = unpack

module("tek.ui.border.socket", tek.ui.class.border)
_VERSION = "SocketBorder 2.1"

-------------------------------------------------------------------------------
--	Constants & Class data:
-------------------------------------------------------------------------------

local DEF_BORDER = { 1, 1, 1, 1 }

-------------------------------------------------------------------------------
-- Class implementation:
-------------------------------------------------------------------------------

local Socket = _M

function Socket:getBorder(element, border)
	return unpack(border or element.Display.Theme.BorderSocketBorder or
		DEF_BORDER)
end

function Socket:draw(element, border, r1, r2, r3, r4)
	local b1, b2, b3, b4 = self:getBorder(element, border)
	local d = element.Drawable
	local p1, p2
	if element.Focus then
		p1, p2 = d.Pens[ui.PEN_FOCUSSHINE], d.Pens[ui.PEN_FOCUSSHADOW]
	else
		p1, p2 = d.Pens[ui.PEN_SHADOW], d.Pens[ui.PEN_SHADOW]
	end
	d:fillRect(r1 - b1, r2 - b2, r3 + b3, r2 - 1, p1)
	d:fillRect(r3 + 1, r2, r3 + b3, r4 + b4, p2)
	d:fillRect(r1 - b1, r4 + 1, r3 + b3, r4 + b4, p2)
	d:fillRect(r1 - b1, r2, r1 - 1, r4, p1)
end
