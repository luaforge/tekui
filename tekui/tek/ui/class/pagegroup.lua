-------------------------------------------------------------------------------
--
--	tek.ui.class.pagegroup
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
--		[[#tek.ui.class.group : Group]] /
--		PageGroup
--
--	OVERVIEW::
--		Implements a group whose children are layouted in individual
--		pages.
--
--	ATTRIBUTES::
--		- {{PageCaptions [IG]}} (table)
--			An array of strings containing captions for each page in
--			the group.
--		- {{PageNumber [IG]}} (number)
--			Number of the page that is initally selected. [Default: 1]
--
--	OVERRIDES::
--		- Area:askMinMax()
--		- Area:getElement()
--		- Area:getElementByXY()
--		- Area:hide()
--		- Area:layout()
--		- Area:markDamage()
--		- Class.new()
--		- Area:passMsg()
--		- Area:punch()
--		- Area:refresh()
--		- Area:relayout()
--		- Area:show()
--
-------------------------------------------------------------------------------

local ui = require "tek.ui"
local Gadget = ui.Gadget
local Group = ui.Group

local ipairs = ipairs
local tostring = tostring
local type = type

module("tek.ui.class.pagegroup", tek.ui.class.group)
_VERSION = "PageGroup 5.0"
local PageGroup = _M

-------------------------------------------------------------------------------
--	Constants & Class data:
-------------------------------------------------------------------------------

local DEF_NULL = { 0, 0, 0, 0 }
local DEF_TABBORDER = { 1, 1, 1, 0 }
local DEF_TABIBORDER = { 2, 2, 2, 0 }
local DEF_TABPADDING = { 4, 3, 4, 3 }
local DEF_PAGEGROUPMARGIN = { 0, 2, 0, 0 }

-------------------------------------------------------------------------------
--	TabButton:
-------------------------------------------------------------------------------

local TabButton = ui.Text:newClass { _NAME = "_tabbutton" }

function TabButton.init(self)
	self.BorderStyle = "blank"
	self.IBorderStyle = "tab"
	self.Border = DEF_TABBORDER
	self.IBorder = DEF_TABIBORDER
	self.Margin = DEF_PAGEGROUPMARGIN
	self.Padding = DEF_TABPADDING
	self.Mode = "touch"
	self.Width = "auto"
	return ui.Text.init(self)
end

function TabButton:setState(bg, fg)
	ui.Text.setState(self, bg, fg)
end

-------------------------------------------------------------------------------
--	new:
-------------------------------------------------------------------------------

local function changeTab(group, tabbuttons, newtabn)
	tabbuttons[group.PageNumber]:setValue("Selected", false)
	group.PageNumber = newtabn
	group.PageElement:hide()
	group.PageElement = group.Children[newtabn]
	group.PageElement:show(group.Display, group.Drawable)
	group.PageElement:rethinkLayout(2)
end

function PageGroup.new(class, self)

	self = self or { }

	local legend, id = self.Legend, self.Id
	self.Legend = false
	self.Id = false

	self.PageCaptions = self.PageCaptions or { }

	self.PageElement = type(self.PageNumber) == "number" and
		(self.PageNumber >= 1 and self.PageNumber <= #self.Children) and
		self.Children[self.PageNumber]
	if not self.PageElement and #self.Children > 0 then
		self.PageNumber = 1
		self.PageElement = self.Children[1]
	end

	self = Group.new(class, self)

	local tabbuttons = { }
	for i, c in ipairs(self.Children) do
		local text = self.PageCaptions[i] or tostring(i)
		tabbuttons[i] = TabButton:new
		{
			Text = text,
			Notifications = {
				["Pressed"] = {
					[true] = {
						{ self, ui.NOTIFY_FUNCTION, changeTab, tabbuttons, i }
					}
				}
			}
		}
	end

	if self.PageNumber then
		tabbuttons[self.PageNumber]:setValue("Selected", true)
	end

	return Group:new
	{
		Children =
		{
			Group:new
			{
				Width = "fill",
				Padding = DEF_NULL,
				Border = DEF_NULL,
				BorderStyle = "socket",
				MaxHeight = 0,
				Children = tabbuttons,
			},
			-- wrapped object:
			self,
		},
		Orientation = "vertical",
		HAlign = self.HAlign,
		Id = id,
		Legend = legend,
		MaxHeight = self.MaxHeight,
		MaxWidth = self.MaxWidth,
		VAlign = self.VAlign,
	}

end

-------------------------------------------------------------------------------
--	show: overrides
-------------------------------------------------------------------------------

function PageGroup:show(display, drawable)
	local theme = display.Theme
	self.Margin = self.Margin or theme.GroupMargin or DEF_NULL
	self.Border = self.Border or theme.GroupBorder or false
	self.Padding = self.Padding or theme.GroupPadding or DEF_NULL
	self.BorderStyle = self.BorderStyle or theme.GroupBorderStyle or ""
	if Gadget.show(self, display, drawable) then
		if not self.PageElement:show(display, drawable) then
			return self:hide()
		end
		return true
	end
end

-------------------------------------------------------------------------------
--	hide: overrides
-------------------------------------------------------------------------------

function PageGroup:hide()
	self.PageElement:hide()
	Gadget.hide(self)
end

-------------------------------------------------------------------------------
--	markDamage: mark damage in self and Children
-------------------------------------------------------------------------------

function PageGroup:markDamage(r1, r2, r3, r4)
	Gadget.markDamage(self, r1, r2, r3, r4)
	self.Redraw = self.Redraw or self.FreeRegion:checkOverlap(r1, r2, r3, r4)
	self.PageElement:markDamage(r1, r2, r3, r4)
end

-------------------------------------------------------------------------------
--	refresh: traverse tree, redraw if damaged
-------------------------------------------------------------------------------

function PageGroup:refresh()
	Gadget.refresh(self)
	self.PageElement:refresh()
end

-------------------------------------------------------------------------------
--	getElementByXY: probe element for all Children
-------------------------------------------------------------------------------

function PageGroup:getElementByXY(x, y)
	return self.PageElement:getElementByXY(x, y)
end

-------------------------------------------------------------------------------
--	askMinMax: returns minx, miny[, maxx[, maxy]]
-------------------------------------------------------------------------------

function PageGroup:askMinMax(m1, m2, m3, m4)
	m1, m2, m3, m4 = self.PageElement:askMinMax(m1, m2, m3, m4)
	return Gadget.askMinMax(self, m1, m2, m3, m4)
end

-------------------------------------------------------------------------------
--	punch: Punch a a hole into the background for the element
-------------------------------------------------------------------------------

function PageGroup:punch(region)
	self.PageElement:punch(region)
end

-------------------------------------------------------------------------------
--	layout: note that layouting takes place unconditionally here
-------------------------------------------------------------------------------

function PageGroup:layout(r1, r2, r3, r4, markdamage)
	Gadget.layout(self, r1, r2, r3, r4, markdamage)
	self.FreeRegion = self.Parent.FreeRegion
	return self.PageElement:layout(r1, r2, r3, r4, markdamage)
end

-------------------------------------------------------------------------------
--	relayout:
-------------------------------------------------------------------------------

function PageGroup:relayout(e, r1, r2, r3, r4)
	local res, changed = Gadget.relayout(self, e, r1, r2, r3, r4)
	if res then
		return res, changed
	end
	return self.PageElement:relayout(e, r1, r2, r3, r4)
end

-------------------------------------------------------------------------------
--	onSetDisable:
-------------------------------------------------------------------------------

function PageGroup:onDisable(onoff)
	return self.PageElement:setValue("Disabled", onoff)
end

-------------------------------------------------------------------------------
--	passMsg(msg)
-------------------------------------------------------------------------------

function PageGroup:passMsg(msg)
	return self.PageElement:passMsg(msg)
end

-------------------------------------------------------------------------------
--	getElement(mode)
-------------------------------------------------------------------------------

function PageGroup:getElement(mode)
	if mode == "parent" then
		return self.Parent
	elseif mode == "nextorparent" then
		return self.Parent:getElement("nextorparent")
	elseif mode == "prevorparent" then
		return self.Parent:getElement("prevorparent")
	elseif mode == "children" then
		return { self.PageElement }
	elseif mode == "firstchild" or mode == "lastchild" then
		return self.PageElement
	else
		return self.PageElement:getElement(mode)
	end
end
