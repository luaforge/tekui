-------------------------------------------------------------------------------
--
--	tek.ui.class.radiobutton
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
--		[[#tek.ui.class.gadget : Gadget]] /
--		[[#tek.ui.class.text : Text]] /
--		[[#tek.ui.class.checkmark : CheckMark]] /
--		RadioButton
--
--	OVERVIEW::
--		Specialization of a [[#tek.ui.class.checkmark : CheckMark]] to
--		implement mutually exclusive 'radio buttons'; they really make
--		sense only if more than one of their kind are grouped together.
--
--	OVERRIDES::
--		- Object.init()
--		- Gadget:onSelect()
--
-------------------------------------------------------------------------------

local ui = require "tek.ui"
local CheckMark = ui.CheckMark
local VectorImage = ui.VectorImage

local ipairs = ipairs

module("tek.ui.class.radiobutton", tek.ui.class.checkmark)
_VERSION = "RadioButton 1.5"

-------------------------------------------------------------------------------
--	Constants & Class data:
-------------------------------------------------------------------------------

local coords =
{
	-- dot:
	0, 0, -1,	2, -2, 1, -2, -1, -1, -2, 1, -2, 2, -1, 2, 1, 1, 2,
	-- shadow:
	-3, 4, -4, 3, -3, -2, -4, -3, -2, -3, -3, -4, 4, -3, 3, -4,
	-- shine:
	-3, 3, 3, 4, 2, 3, 4, 3, 3, 2, 3, -3,
}

-- shadow:
local points1 = { 10, 18, 19, 20, 21, 22, 16, 23 }
-- shine:
local points2 = { 10, 11, 12, 13, 14, 15, 16, 17 }
-- dot:
local points3 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 2 }

local RadioImage1 = VectorImage:new
{
	ImageData =
	{
		Coords = coords,
		Primitives =
		{
			{	0x1000, 8, Points = points1, Pen = ui.PEN_HALFSHADOW },
			{ 0x1000, 8, Points = points2, Pen = ui.PEN_HALFSHINE },
		},
		MinMax = { -4, 4, 4, -4 },
	}
}

local RadioImage2 = VectorImage:new
{
	ImageData =
	{
		Coords = coords,
		Primitives = {
			{	0x1000, 8, Points = points1, Pen = ui.PEN_HALFSHINE },
			{ 0x1000, 8, Points = points2, Pen = ui.PEN_HALFSHADOW },
			{	0x2000, 10, Points = points3, Pen = ui.PEN_BUTTONTEXT },
		},
		MinMax = { -4, 4, 4, -4 },
	}
}

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

local RadioButton = _M

function RadioButton.init(self)
	self.Image = self.Image or RadioImage1
	self.AltImage = self.AltImage or RadioImage2
	self.Mode = self.Mode or "touch"
	return CheckMark.init(self)
end

-------------------------------------------------------------------------------
--	onSelect:
-------------------------------------------------------------------------------

function RadioButton:onSelect(selected)
	if selected then
		-- unselect siblings in group:
		local myclass = self:getClass()
		for _, e in ipairs(self.Parent.Children) do
			if e ~= self and e:getClass() == myclass and e.Selected then
				e:setValue("Selected", false) -- no notify
			end
		end
	end
	CheckMark.onSelect(self, selected)
end
