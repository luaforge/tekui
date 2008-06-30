-------------------------------------------------------------------------------
--
--	tek.ui.class.poplist
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
--		[[#tek.ui.class.popitem : PopItem]] /
--		PopList
--
--	OVERVIEW::
--		This class is a specialization of a PopItem allowing the user
--		to choose an item from a list.
--
--	ATTRIBUTES::
--		- {{ListObject [IG]}} ([[#tek.class.list : List]])
--			List object
--		- {{SelectedEntry [ISG]}} (number)
--			Number of the selected entry, or 0 if none is selected. Changing
--			this attribute invokes the PopList:onSelectEntry() method.
--
--	IMPLEMENTS::
--		- PopList:onSelectEntry() - Handler for the {{SelectedEntry}}
--		attribute
--
--	OVERRIDES::
--		- Area:askMinMax()
--		- Area:draw()
--		- Object.init()
--		- Area:layout()
--		- Class.new()
--		- Area:show()
--
-------------------------------------------------------------------------------

local ui = require "tek.ui"

local Canvas = ui.Canvas
local Gadget = ui.Gadget
local List = require "tek.class.list"
local ListGadget = ui.ListGadget
local PopItem = ui.PopItem
local ScrollGroup = ui.ScrollGroup
local Text = ui.Text
local VectorImage = ui.VectorImage

local insert = table.insert

module("tek.ui.class.poplist", tek.ui.class.popitem)
_VERSION = "PopList 3.0"

-------------------------------------------------------------------------------
--	Constants and class data:
-------------------------------------------------------------------------------

local prims = { { 0x1000, 3, Points = { 1, 2, 3 }, Pen = ui.PEN_BUTTONTEXT } }

local ArrowImage = VectorImage:new
{
	ImageData =
	{
		Coords = { -2, 1, 3, 1, 0, -1 },
		Primitives = prims,
		MinMax = { -3, -3, 4, 3 },
	}
}

local NOTIFY_SELECT = { ui.NOTIFY_SELF, "onSelectEntry", ui.NOTIFY_VALUE }

-------------------------------------------------------------------------------
--	PopListGadget:
-------------------------------------------------------------------------------

local PopListGadget = ListGadget:newClass()

function PopListGadget:passMsg(msg)
	if msg[2] == ui.MSG_MOUSEMOVE then
		if msg[4] >= 0 and msg[4] < self.CanvasHeight then
			local lnr = self:findLine(msg[5])
			if lnr then
				if lnr ~= self.SelectedLine then
					self:setValue("CursorLine", lnr)
					self:setValue("SelectedLine", lnr)
				end
			end
		end
	end
end

function PopListGadget:onActivate(active)
	if active == false then
		local lnr = self.CursorLine
		local entry = self:getItem(lnr)
		if entry then
			local popitem = self.Window.PopupBase
			popitem:setValue("SelectedEntry", lnr)
			popitem:setValue("Text", entry[1][1])
		end
		-- needed to unregister input-handler:
		self:setValue("Focus", false)
		self.Window:finishPopup()
	end
	ListGadget.onActivate(self, active)
end

function PopListGadget:askMinMax(m1, m2, m3, m4)
	m1 = m1 + self.MinWidth
	m2 = m2 + self.CanvasHeight
	m3 = ui.HUGE
	m4 = m4 + self.CanvasHeight
	return Gadget.askMinMax(self, m1, m2, m3, m4)
end

-------------------------------------------------------------------------------
--	PopList:
-------------------------------------------------------------------------------

local PopList = _M

function PopList.init(self)
	self.ImageRect = { 0, 0, 0, 0 }
	self.TextHAlign = self.TextHAlign or "left"
	self.SelectedEntry = self.SelectedEntry or 0
	return PopItem.init(self)
end

function PopList.new(class, self)
	self = self or { }
	self.ListObject = self.ListObject or List:new()
	self.ListGadget = PopListGadget:new { ListObject = self.ListObject }
	self.Children =
	{
		ScrollGroup:new
		{
			VSliderMode = "auto",
			Canvas = Canvas:new
			{
				KeepMinWidth = true,
				KeepMinHeight = true,
				AutoWidth = true,
				Child = self.ListGadget
			}
		}
	}
	return PopItem.new(class, self)
end

function PopList:setup(app, window)
	PopItem.setup(self, app, window)
	self:addNotify("SelectedEntry", ui.NOTIFY_CHANGE, NOTIFY_SELECT)
end

function PopList:cleanup()
	self:remNotify("SelectedEntry", ui.NOTIFY_CHANGE, NOTIFY_SELECT)
	PopItem.cleanup(self)
end


function PopList:askMinMax(m1, m2, m3, m4)
	local lo = self.ListObject
	if lo then
		local w, h = 0, 0
		local tr = { }
		local font = self.Display:openFont(self.FontSpec)
		for lnr = 1, lo:getN() do
			local entry = lo:getItem(lnr)
			local t = self:newTextRecord(entry[1][1], font, self.TextHAlign,
				self.TextVAlign, 0, 0, 20, 0)
			insert(tr, t)
		end
		w, h = self:getTextSize(tr)
		w = w + 1 -- for disabled state
		h = h + 1
		if self.KeepMinWidth and self.MinWidth == 0 then
			self.MinWidth = w
		end
		if self.KeepMinHeight and self.MinHeight == 0 then
			self.MinHeight = h
		end
		m1 = m1 + w
		m2 = m2 + h
		m3 = m3 + w
		m4 = m4 + h
	end
	return Gadget.askMinMax(self, m1, m2, m3, m4)
end

function PopList:layout(x0, y0, x1, y1, markdamage)
	if PopItem.layout(self, x0, y0, x1, y1, markdamage) then
		local r = self.Rect
		local p = self.PaddingAndBorder
		local ih = r[4] - r[2] - p[4] - p[2] + 1
		local d = self.Drawable
		local iw = ih * d.AspectX / d.AspectY
		local x = r[3] - iw
		local y = r[2] + p[2]
		local i = self.ImageRect
		i[1], i[2], i[3], i[4] = x, y, x + iw - 1, y + ih - 1
		return true
	end
end

function PopList:draw()
	PopItem.draw(self)
	local img = ArrowImage
	if img then
		prims[1].Pen = self.Foreground
		img:draw(self.Drawable, self.ImageRect)
	end
end

function PopList:beginPopup()
	PopItem.beginPopup(self)
	self.ListGadget:setValue("Focus", true)
end

-------------------------------------------------------------------------------
--	PopList:onSelectEntry(line): This method is invoked when the
--	{{SelectedEntry}} attribute is set.
-------------------------------------------------------------------------------

function PopList:onSelectEntry(lnr)
end
