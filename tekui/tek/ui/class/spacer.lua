-------------------------------------------------------------------------------
--
--	tek.ui.class.spacer
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
--		Spacer
--
--	OVERVIEW::
--		This class implements a separator
--		that helps to arrange elements in a group visually.
--
--	OVERRIDES::
--		- Area:askMinMax()
--		- Area:show()
--
-------------------------------------------------------------------------------

local ui = require "tek.ui"
local Frame = ui.Frame

module("tek.ui.class.spacer", tek.ui.class.frame)
_VERSION = "Spacer 1.3"

-------------------------------------------------------------------------------
--	Constants & Class data:
-------------------------------------------------------------------------------

local DEF_BORDER = { 1, 1, 1, 1 }
local DEF_PADDING = { 0, 0, 0, 0 }

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

local Spacer = _M

-------------------------------------------------------------------------------
--	show:
-------------------------------------------------------------------------------

function Spacer:show(display, drawable)
	local theme = display.Theme
	-- outer spacing:
	self.Margin = self.Margin or theme.SpacerMargin or false
	-- outer border:
	self.Border = self.Border or theme.SpacerBorder or DEF_BORDER
	-- inner spacing:
	self.Padding = self.Padding or theme.SpacerPadding or DEF_PADDING
	-- outer borderstyle:
	self.BorderStyle = self.BorderStyle or theme.SpacerBorderStyle or "recess"
	return Frame.show(self, display, drawable)
end

-------------------------------------------------------------------------------
--	askMinMax:
-------------------------------------------------------------------------------

function Spacer:askMinMax(m1, m2, m3, m4)
	local o = self.Parent and self.Parent:getStructure()
	if o == 1 then
		self.Height = "fill"
		self.Width = "auto"
	else
		self.Width = "fill"
		self.Height = "auto"
	end
	return Frame.askMinMax(self, m1, m2, m3, m4)
end
