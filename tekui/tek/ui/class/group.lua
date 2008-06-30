-------------------------------------------------------------------------------
--
--	tek.ui.class.group
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
--		Group
--
--	OVERVIEW::
--		This class implements a container for child elements and
--		various layouting options.
--
--	ATTRIBUTES::
--		- {{Children [G]}} (table)
--			Array of child objects
--		- {{FreeRegion [G]}} ([[#tek.lib.region : Region]])
--			Region inside the group that is not covered by child elements
--		- {{GridHeight [IG]}} (number)
--			Grid height, in number of elements. [Default: 1, not a grid]
--		- {{GridWidth [IG]}} (number)
--			Grid width, in number of elements [Default: 1, not a grid]
--		- {{Legend [IG]}} (string)
--			Legend, group caption text [Default: '''false''']
--		- {{LegendFontSpec [IG]}} (string)
--			Font specifier for the group legend [Default: undefined]. See
--			also [[#tek.ui.class.text : Text]] for a format description.
--		- {{Orientation [IG]}} (string)
--			Orientation of the group; can be
--				- "horizontal" - The elements are layouted horizontally
--				- "vertical" - The elements are layouted vertically
--			Default: "horizontal"
--		- {{SameHeight [IG]}} (boolean)
--			Indicates that the same height should be reserved for all
--			elements in the group [Default: '''false''']
--		- {{SameWidth [IG]}} (boolean)
--			Indicates that the same width should be reserved for all
--			elements in the group [Default: '''false''']
--		- {{SameSize [IG]}} (boolean)
--			A shortcut for {{SameWidth}} and {{SameHeight}}
--		- {{Style [IG]}} (string)
--			Style of the group, can be
--				- "menubar" - The group is used for a menubar
--				- "normal" - This is a regular group
--
--	IMPLEMENTS::
--		- Group:getStructure() - Get group's structural parameters
--		- {{Group:addMember()}} - See Family:addMember()
--		- {{Group:remMember()}} - See Family:remMember()
--
--	OVERRIDES::
--		- Area:askMinMax()
--		- Element:cleanup()
--		- Area:draw()
--		- Area:getElement()
--		- Area:getElementByXY()
--		- Area:hide()
--		- Object.init()
--		- Area:layout()
--		- Area:markDamage()
--		- Class.new()
--		- Area:passMsg()
--		- Area:punch()
--		- Area:refresh()
--		- Area:relayout()
--		- Element:setup()
--		- Area:setState()
--		- Area:show()
--
-------------------------------------------------------------------------------

local ui = require "tek.ui"
local Region = require "tek.lib.region"
local Family = ui.Family
local Gadget = ui.Gadget

local assert = assert
local floor = math.floor
local ipairs = ipairs

module("tek.ui.class.group", tek.ui.class.gadget)
_VERSION = "Group 12.0"
local Group = _M

-------------------------------------------------------------------------------
--	Constants & Class data:
-------------------------------------------------------------------------------

local DEF_MARGIN = { 0, 0, 0, 0 }
local DEF_PADDING = { 0, 0, 0, 0 }
local DEF_BORDER_MENUBAR = { 0, 0, 0, 1 }

-------------------------------------------------------------------------------
--	class implementation:
-------------------------------------------------------------------------------

function Group.init(self)
	self = self or { }
	self.Children = self.Children or { }
	self.FreeRegion = false
	self.GridWidth = self.GridWidth or false
	self.GridHeight = self.GridHeight or false
	self.Layout = self.Layout or ui.loadClass("layout", "default"):new { }
	self.Legend = self.Legend or false
	self.LegendFont = false
	self.LegendFontSpec = self.LegendFontSpec or false
	self.Orientation = self.Orientation or "horizontal"
	self.SameHeight = self.SameHeight or false
	self.SameSize = self.SameSize or false
	self.SameWidth = self.SameWidth or false
	self.Style = self.Style or "normal"
	if self.Style == "menubar" then
		self.Width = "fill"
		self.Height = "auto"
	end
	self.Weights = self.Weights or { }
	return Gadget.init(self)
end

function Group.new(class, self)
	self = Gadget.new(class, self)
	self:calcWeights()
	return self
end

-------------------------------------------------------------------------------
--	setup: overrides
-------------------------------------------------------------------------------

function Group:setup(app, window)
	Gadget.setup(self, app, window)
	for _, c in ipairs(self.Children) do
		c:setup(app, window)
	end
end

-------------------------------------------------------------------------------
--	cleanup: overrides
-------------------------------------------------------------------------------

function Group:cleanup()
	for _, c in ipairs(self.Children) do
		c:cleanup()
	end
	Gadget.cleanup(self)
end

-------------------------------------------------------------------------------
--	show: overrides
-------------------------------------------------------------------------------

function Group:show(display, drawable)
	local theme = display.Theme
	if self.Style == "menubar" then
		self.Margin = self.Margin or theme.GroupMargin or DEF_MARGIN
		self.Border = self.Border or theme.MenuBarBorder or DEF_BORDER_MENUBAR
		self.Padding = self.Padding or theme.GroupPadding or DEF_PADDING
		self.BorderStyle = self.BorderStyle or theme.MenuBarBorderStyle or
			"group"
		self.BGPen = self.BGPen or theme.MenuBarBackPen or ui.PEN_MENUBACK
	elseif self.Legend then
		self.Margin = self.Margin or theme.GroupMarginLegend or false
		self.Padding = self.Padding or theme.GroupPaddingLegend or false
		self.Border = self.Border or theme.GroupBorderLegend or false
		self.BorderStyle = self.BorderStyle or theme.GroupBorderStyleLegend or
			"group"
		self.LegendFontSpec = self.LegendFontSpec or
			theme.GroupLegendFontSpec or "__small"
		self.LegendFont = display:openFont(self.LegendFontSpec)
	else
		self.Margin = self.Margin or theme.GroupMargin or DEF_MARGIN
		self.Border = self.Border or theme.GroupBorder or false
		self.Padding = self.Padding or theme.GroupPadding or DEF_PADDING
		self.BorderStyle = self.BorderStyle or theme.GroupBorderStyle or ""
	end
	if Gadget.show(self, display, drawable) then
		for _, c in ipairs(self.Children) do
			if not c:show(display, drawable) then
				return self:hide()
			end
		end
		return true
	end
end

-------------------------------------------------------------------------------
--	hide: overrides
-------------------------------------------------------------------------------

function Group:hide()
	for _, c in ipairs(self.Children) do
		c:hide()
	end
	self.Display:closeFont(self.LegendFont)
	self.LegendFont = false
	Gadget.hide(self)
end

-------------------------------------------------------------------------------
--	width, height, orientation = Group:getStructure(): get Group's structural
--	parameters.
-------------------------------------------------------------------------------

function Group:getStructure()
	local gw, gh, nc = self.GridWidth, self.GridHeight, #self.Children
	if gw then
		return 1, gw, floor((nc + gw - 1) / gw)
	elseif gh then
		return 2, floor((nc + gh - 1) / gh), gh
	elseif self.Orientation == "horizontal" then
		return 1, nc, 1
	end
	return 2, 1, nc
end

-------------------------------------------------------------------------------
--	getSameSize: tell if the group is in 'samesize' mode on the
--	given axis
-------------------------------------------------------------------------------

function Group:getSameSize(axis)
	return self.SameSize or (axis == 1 and self.SameWidth) or
		(axis == 2 and self.SameHeight)
end

-------------------------------------------------------------------------------
--	calcWeights: (Re-)alculates and updates the group's weights array
-------------------------------------------------------------------------------

function Group:calcWeights()
	local wx, wy = { }, { }
	local cidx = 1
	local _, gw, gh = self:getStructure()
	for y = 1, gh do
		for x = 1, gw do
			local c = self.Children[cidx]
			if not c then
				break
			end
			local w = c.Weight
			if w then
				wx[x] = (wx[x] or 0) + w
				wy[y] = (wy[y] or 0) + w
			end
			cidx = cidx + 1
		end
	end
	self.Weights[1], self.Weights[2] = wx, wy
end

-------------------------------------------------------------------------------
--	checkMember: check if an element fits into this group as a child member
-------------------------------------------------------------------------------

function Group:checkMember(child)
	-- only elements descending from Area can be children of a group:
	return child:checkDescend(ui.Area)
end

-------------------------------------------------------------------------------
--	addMember: add a child member (see Family:addMember())
-------------------------------------------------------------------------------

function Group:addMember(child, pos)
	if self:checkMember(child) then
		child:setup(self.Application, self.Window)
		if child:show(self.Display, self.Drawable) then
			if Family.addMember(self, child, pos) then
				self:rethinkLayout(true)
				return child
			end
			child:hide()
		end
		child:cleanup()
	end
end

-------------------------------------------------------------------------------
--	remMember: remove a child member (see Family:remMember())
-------------------------------------------------------------------------------

function Group:remMember(child)
	assert(child.Parent == self)
	if child == self.Window.FocusElement then
		self.Window:setFocusElement()
	end
	Family.remMember(self, child)
	child:hide()
	child:cleanup()
	self:rethinkLayout(true)
end

-------------------------------------------------------------------------------
--	markDamage: overrides
-------------------------------------------------------------------------------

function Group:markDamage(r1, r2, r3, r4)
	Gadget.markDamage(self, r1, r2, r3, r4)
	self.Redraw = self.Redraw or self.FreeRegion:checkOverlap(r1, r2, r3, r4)
	for _, c in ipairs(self.Children) do
		c:markDamage(r1, r2, r3, r4)
	end
end

-------------------------------------------------------------------------------
--	draw: overrides
-------------------------------------------------------------------------------

function Group:draw()
	local d = self.Drawable
	local f = self.FreeRegion
	local p = d.Pens[self.Background]
	for _, r in f:getRects() do
		local r1, r2, r3, r4 = f:getRect(r)
		d:fillRect(r1, r2, r3, r4, p)
	end
end

-------------------------------------------------------------------------------
--	refresh: overrides
-------------------------------------------------------------------------------

function Group:refresh()
	Gadget.refresh(self)
	for _, c in ipairs(self.Children) do
		c:refresh()
	end
end

-------------------------------------------------------------------------------
--	getElementByXY: overrides
-------------------------------------------------------------------------------

function Group:getElementByXY(x, y)
	for _, c in ipairs(self.Children) do
		local ret = c:getElementByXY(x, y)
		if ret then
			return ret
		end
	end
	return false
end

-------------------------------------------------------------------------------
--	askMinMax: overrides
-------------------------------------------------------------------------------

function Group:askMinMax(m1, m2, m3, m4)
	m1, m2, m3, m4 = self.Layout:askMinMax(self, m1, m2, m3, m4)
	return Gadget.askMinMax(self, m1, m2, m3, m4)
end

-------------------------------------------------------------------------------
--	punch: overrides
-------------------------------------------------------------------------------

function Group:punch(region)
	local b = self.MarginAndBorder
	local r = self.Rect
	region:subRect(r[1] - b[1], r[2] - b[2], r[3] + b[3], r[4] + b[4])
end

-------------------------------------------------------------------------------
--	layout: overrides; note that layouting takes place unconditionally here
-------------------------------------------------------------------------------

function Group:layout(r1, r2, r3, r4, markdamage)
	Gadget.layout(self, r1, r2, r3, r4, markdamage)
	-- layout and update free region:
	local f = Region.new(r1, r2, r3, r4)
	self.FreeRegion = f
	self.Layout:layout(self, r1, r2, r3, r4, markdamage)
	local b = self.BorderRegion
	if b then
		for _, r in b:getRects() do
			f:subRect(b:getRect(r))
 		end
 	end
	return true
end

-------------------------------------------------------------------------------
--	relayout: overrides
-------------------------------------------------------------------------------

function Group:relayout(e, r1, r2, r3, r4)
	local res, changed = Gadget.relayout(self, e, r1, r2, r3, r4)
	if res then
		return res, changed
	end
	for _, c in ipairs(self.Children) do
		res, changed = c:relayout(e, r1, r2, r3, r4)
		if res then
			return res, changed
		end
	end
end

-------------------------------------------------------------------------------
--	passMsg: overrides
-------------------------------------------------------------------------------

function Group:passMsg(msg)
	for _, c in ipairs(self.Children) do
		msg = c:passMsg(msg)
		if not msg then
			return false
		end
	end
	return msg
end

-------------------------------------------------------------------------------
--	setState: overrides
-------------------------------------------------------------------------------

function Group:setState(bg)
	bg = bg or self.BGPen or ui.PEN_GROUPBACK
	Gadget.setState(self, bg)
end

-------------------------------------------------------------------------------
--	getElement: overrides
-------------------------------------------------------------------------------

function Group:getElement(mode)
	local c = self.Children
	if mode == "firstchild" then
		return c[1]
	elseif mode == "lastchild" then
		return c[#c]
	elseif mode == "children" then
		return c
	end
	return Gadget.getElement(self, mode)
end
