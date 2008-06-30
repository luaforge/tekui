
--
--	tek.ui.class.vectorimage
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	OVERVIEW::
--	Implements vector graphics rendering
--

module("tek.ui.class.vectorimage", tek.class)
_VERSION = "VectorImage 1.1"

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

local VectorImage = _M

function VectorImage:draw(drawable, rect)
	VectorImage.render(drawable, rect, self.ImageData)
end

-------------------------------------------------------------------------------
--	Static methods:
-------------------------------------------------------------------------------

function VectorImage.render(drawable, rect, data)
 	data.PenTable = drawable.Pens
	data.Rect = rect
	drawable:drawImage(data) -- TODO: pass PenTable, Rect
end
