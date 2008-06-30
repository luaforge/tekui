-------------------------------------------------------------------------------
--
--	tek.ui.class.menuitem
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
--		MenuItem
--
--	OVERVIEW::
--		This class implements the basic items for window menus and popups.
--		In particular, it displays a [[#tek.ui.class.popitem : PopItem]]'s
--		{{Shortcut}} attribute and an arrow to indicate that there is a
--		sub menu.
--
--	OVERRIDES::
--		- Area:askMinMax()
--		- Area:draw()
--		- Object.init()
--		- Area:layout()
--		- Class.new()
--		- Area:setState()
--		- Area:show()
--
-------------------------------------------------------------------------------

local db = require "tek.lib.debug"
local ui = require "tek.ui"
local PopItem = ui.PopItem
local VectorImage = ui.VectorImage
local Theme = ui.Theme
local max = math.max
local floor = math.floor

module("tek.ui.class.menuitem", tek.ui.class.popitem)
_VERSION = "MenuItem 3.1"

-------------------------------------------------------------------------------
--	Constants and class data:
-------------------------------------------------------------------------------

local DEF_PADDING_BASE = { 4, 2, 4, 2 }
local DEF_MARGIN_SUB = { 0, 0, 0, 0 }
local DEF_PADDING_SUB = { 4, 2, 16, 2 }

local prims = { { 0x1000, 3, Points = { 1, 2, 3 }, Pen = ui.PEN_BUTTONTEXT } }

local ArrowImage = VectorImage:new
{
	ImageData =
	{
		Coords = { -1,2, -1,-2, 1,0 },
		Primitives = prims,
		MinMax = { -4, -4, 4, 4 },
	}
}

-------------------------------------------------------------------------------
--	MenuItem class:
-------------------------------------------------------------------------------

local MenuItem = _M

function MenuItem.new(class, self)
	self = self or { }
	self.ImageRect = { 0, 0, 0, 0 }
	-- prevent superclass from filling in text records:
	self.TextRecords = self.TextRecords or { }
	return PopItem.new(class, self)
end

function MenuItem.init(self)
	self.ArrowImage = false
	self.MaxHeight = self.MaxHeight or 0
	if self.Children then
		self.Mode = "toggle"
	else
		self.Mode = "button"
	end
	self.TextHAlign = "left"
	return PopItem.init(self)
end

function MenuItem:show(display, drawable)

	local theme = display.Theme

	self.FontSpec = self.FontSpec or theme.MenuItemFontSpec or "__menu"

	if self.Parent.Style == "menubar" then
		self.Margin = self.Margin or theme.MenuItemMargin or false
		self.Border = self.Border or theme.MenuItemBorder or false
		self.IBorder = self.IBorder or theme.MenuItemIBorder or false
		self.Padding = self.Padding or theme.MenuItemPadding or
			DEF_PADDING_BASE
	else
		self.Margin = self.Margin or theme.MenuItemMargin or DEF_MARGIN_SUB
		self.Border = self.Border or theme.MenuItemBorder or false
		self.IBorder = self.IBorder or theme.MenuItemIBorder or false
		self.Padding = self.Padding or theme.MenuItemPadding or DEF_PADDING_SUB
	end

	if self.Children then
		self.BorderStyle = self.BorderStyle or
			theme.MenuItemChildrenBorderStyle or ""
		self.IBorderStyle = self.IBorderStyle or
			theme.MenuItemChildrenIBorderStyle or ""
		if self.Parent.Style ~= "menubar" then
			self.ArrowImage = ArrowImage
		end
	else
		self.BorderStyle = self.BorderStyle or
			theme.MenuItemBorderStyle or ""
		self.IBorderStyle = self.IBorderStyle or
			theme.MenuItemIBorderStyle or ""
	end

	if PopItem.show(self, display, drawable) then
		self:setTextRecord(1, self.Text, self.FontSpec, "left")
		if self.Shortcut and self.Parent.Style ~= "menubar" and
			not self.Children then
			self:setTextRecord(2, self.Shortcut, self.FontSpec, "left")
		end
		return true
	end

end

function MenuItem:submenu(val)
	-- subitems are handled in baseclass:
	PopItem.submenu(self, val)
	-- handle baseitem:
	if self.Window then
		local popup = self.Window.ActivePopup
		if popup then
			-- hilite over baseitem while another open popup in menubar:
			if val == true and popup ~= self then
				db.info("have another popup open")
				self:beginPopup()
				self:setValue("Selected", true)
			end
		end
	end
end

function MenuItem:beginPopup()
	if self.Window and self.Window.ActivePopup and
		self.Window.ActivePopup ~= self then
		-- close already open menu in same group:
		self.Window.ActivePopup:endPopup()
	end
	-- subitems are handled in baseclass:
	PopItem.beginPopup(self)
	-- handle baseitem:
	if self.Window then
		self.Window.ActivePopup = self
	end
end

function MenuItem:endPopup()
	-- subitems are handled in baseclass:
	PopItem.endPopup(self)
	-- handle baseitem:
	if self.Window then
		self.Window.ActivePopup = false
	end
end

function MenuItem:askMinMax(m1, m2, m3, m4)
	if self.Parent.Style == "menubar" then
		self.Width = "auto"
	end
	return PopItem.askMinMax(self, m1, m2, m3, m4)
end

function MenuItem:layout(x0, y0, x1, y1, markdamage)
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

function MenuItem:draw()
	PopItem.draw(self)
	local img = self.ArrowImage
	if img then
		prims[1].Pen = self.Foreground
		img:draw(self.Drawable, self.ImageRect)
	end
end

function MenuItem:setState(bg, fg)
	if self.Selected or self.Hilite or self.Focus then
		fg = fg or ui.PEN_MENUACTIVETEXT
		bg = bg or ui.PEN_MENUACTIVE
	end
	return PopItem.setState(self, bg, fg)
end
