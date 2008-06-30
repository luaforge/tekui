-------------------------------------------------------------------------------
--
--	tek.ui.class.frame
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	LINEAGE::
--		[[#ClassOverview]] :
--		[[#tek.class : Class]] /
--		[[#tek.class.object : Object]] /
--		[[#tek.ui.class.element : Element]] /
--		[[#tek.ui.class.area : Area]] /
--		Frame
--
--	OVERVIEW::
--		Implements inner and outer borders and the element's
--		inner padding.
--
--	ATTRIBUTES::
--		- {{Border [IG]}} (table)
--			An array of four thicknesses (in pixels) for the element's outer
--			border, in the order left, right, top, bottom. If unspecified
--			during initialization, the class' default outer border thicknesses
--			are used.
--		- {{BorderRegion [G]}} ([[#tek.lib.region : Region]])
--			Region object holding the outline of the element's outer border
--		- {{BorderClass [G]}} (Class)
--			Border class used for the element's outer border, loaded from
--			the directory {{tek/ui/border}}.
--		- {{BorderStyle [IG]}} (string)
--			Name of a style used for an element's outer border, which
--			corresponds to the name of a border class (e.g. "recess"),
--			used to create the element's border during initialization.
--			If unspecified, the class' default outer border style is used.
--		- {{IBorder [IG]}} (table)
--			An array of four thicknesses in pixels for the element's inner
--			border, in the order left, right, top, bottom. If unspecified
--			during initialization, the class' default inner border thicknesses
--			are used.
--		- {{IBorderClass [G]}} (Class)
--			Class used for the element's inner border, loaded from
--			the directory {{tek/ui/border}}.
--		- {{IBorderStyle [IG]}} (string)
--			Name of a style used for an element's inner border, which
--			corresponds to the name of a border class (e.g. "button"),
--			used to create the element's border during initialization.
--			If unspecified, the class' default inner border style is used.
--		- {{Padding [IG]}} (table)
--			An array of four offsets for the element's inner padding in the
--			order left, right, top, bottom [pixels]. If unspecified during
--			initialization, the class' default paddings are used.
--
--	IMPLEMENTS::
--		- Frame:drawBorder() - Draws the element's outer border
--		- Frame:getBorder() - Returns the element's outer border
--		- Frame:getIBorder() - Returns the element's inner border
--		- Frame:onFocus() - Handler for {{Focus}}
--
--	OVERRIDES::
--		- Area:askMinMax()
--		- Element:cleanup()
--		- Area:draw()
--		- Area:hide()
--		- Object.init()
--		- Area:layout()
--		- Area:markDamage()
--		- Class.new()
--		- Area:punch()
--		- Area:refresh()
--		- Element:setup()
--		- Area:show()
--
-------------------------------------------------------------------------------

local ui = require "tek.ui"
local Area = ui.Area
local Border = ui.Border
local min = math.min
local max = math.max
local unpack = unpack

module("tek.ui.class.frame", tek.ui.class.area)
_VERSION = "Frame 2.11"

local Frame = _M

-------------------------------------------------------------------------------
--	Constants & Class data:
-------------------------------------------------------------------------------

local DEF_PADDING = { 1, 1, 1, 1 }
local NOTIFY_FOCUS = { ui.NOTIFY_SELF, "onFocus", ui.NOTIFY_VALUE }

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

function Frame.new(class, self)
	self = self or { }
	-- Combined padding and inner border offsets of the element:
	self.PaddingAndBorder = { 0, 0, 0, 0 }
	return Area.new(class, self)
end

function Frame.init(self)
	-- Outer border offsets of the element:
	self.Border = self.Border or false
	-- Region describing the outer border:
	self.BorderRegion = false
	-- Loaded class of the outer border:
	self.BorderClass = false
	-- Style name of the outer border:
	self.BorderStyle = self.BorderStyle or false
	-- Inner border offsets of the element:
	self.IBorder = self.IBorder or false
	-- Loaded class of the inner border:
	self.IBorderClass = false
	-- Style name of the inner border:
	self.IBorderStyle = self.IBorderStyle or false
	-- Padding (inner spacing) of the element:
	self.Padding = self.Padding or false
	-- Boolean to indicate whether the border needs to be redrawn:
	self.RedrawBorder = false
	return Area.init(self)
end

-------------------------------------------------------------------------------
--	setup: overrides
-------------------------------------------------------------------------------

function Frame:setup(app, window)
	Area.setup(self, app, window)
	self:addNotify("Focus", ui.NOTIFY_CHANGE, NOTIFY_FOCUS)
end

-------------------------------------------------------------------------------
--	cleanup: overrides
-------------------------------------------------------------------------------

function Frame:cleanup()
	self:remNotify("Focus", ui.NOTIFY_CHANGE, NOTIFY_FOCUS)
	Area.cleanup(self)
end

-------------------------------------------------------------------------------
--	show: overrides
-------------------------------------------------------------------------------

function Frame:show(display, drawable)
	local theme = display.Theme
	-- outer spacing:
	self.Margin = self.Margin or theme.FrameMargin or false
	-- outer border:
	self.Border = self.Border or theme.FrameBorder or false
	-- inner border:
	self.IBorder = self.IBorder or theme.FrameIBorder or false
	-- inner spacing:
	self.Padding = self.Padding or theme.FramePadding or DEF_PADDING
	-- outer borderstyle:
	self.BorderStyle = self.BorderStyle or theme.FrameBorderStyle or "socket"
	-- inner borderstyle:
	self.IBorderStyle = self.IBorderStyle or theme.FrameIBorderStyle or false
	-- border classes:
	self.BorderClass = Border.loadClass(self.BorderStyle)
	self.IBorderClass = Border.loadClass(self.IBorderStyle)
	if self.Focus then
		self:onFocus(self.Focus)
	end
	return Area.show(self, display, drawable)
end

-------------------------------------------------------------------------------
--	hide: overrides
-------------------------------------------------------------------------------

function Frame:hide()
	Area.hide(self)
	self.BorderClass = false
	self.BorderRegion = false
	self.IBorderClass = false
end

-------------------------------------------------------------------------------
--	askMinMax: overrides
-------------------------------------------------------------------------------

function Frame:askMinMax(m1, m2, m3, m4)
	local p = self.PaddingAndBorder
	m1 = m1 + p[1] + p[3]
	m2 = m2 + p[2] + p[4]
	m3 = m3 + p[1] + p[3]
	m4 = m4 + p[2] + p[4]
	m1 = max(self.MinWidth, m1)
	m2 = max(self.MinHeight, m2)
	return Area.askMinMax(self, m1, m2, m3, m4)
end

-------------------------------------------------------------------------------
--	calcOffsets: overrides
-------------------------------------------------------------------------------

function Frame:calcOffsets()
	-- calculate margin + outer border:
	local b1, b2, b3, b4 = self.BorderClass:getBorder(self, self.Border)
	local m = self.Margin
	local d = self.MarginAndBorder
	d[1], d[2], d[3], d[4] = b1 + m[1], b2 + m[2], b3 + m[3], b4 + m[4]
	-- calculate padding + inner border:
	b1, b2, b3, b4 = self.IBorderClass:getBorder(self, self.IBorder)
	local p = self.Padding
	d = self.PaddingAndBorder
	d[1], d[2], d[3], d[4] = b1 + p[1], b2 + p[2], b3 + p[3], b4 + p[4]
end

-------------------------------------------------------------------------------
--	border = Frame:getBorder(): Returns an element's table of outer border
--	strengths [pixels] in the order left, top, right, bottom.
-------------------------------------------------------------------------------

function Frame:getBorder()
	return self.BorderClass:getBorder(self, self.Border)
end

-------------------------------------------------------------------------------
--	iborder = Frame:getIBorder(): Returns an element's table of
--	inner border strengths [pixels] in the order left, top, right, bottom.
-------------------------------------------------------------------------------

function Frame:getIBorder()
	return self.IBorderClass:getBorder(self, self.IBorder)
end

-------------------------------------------------------------------------------
--	markDamage: overrides
-------------------------------------------------------------------------------

function Frame:markDamage(r1, r2, r3, r4)
	Area.markDamage(self, r1, r2, r3, r4)
	if self.BorderRegion and
		self.BorderRegion:checkOverlap(r1, r2, r3, r4) then
		self.RedrawBorder = true
	end
end

-------------------------------------------------------------------------------
--	layout: overrides - additionally maintains a border region
-------------------------------------------------------------------------------

function Frame:layout(r1, r2, r3, r4, markdamage)
	local res = Area.layout(self, r1, r2, r3, r4, markdamage)
	if res or not self.BorderRegion then
		self.BorderRegion = self.BorderClass:getRegion(self,
			self.Border, unpack(self.Rect))
		if self.BorderRegion then
			self.RedrawBorder = markdamage ~= false
			res = true
		end
	end
	return res
end

-------------------------------------------------------------------------------
--	punch: overrides
-------------------------------------------------------------------------------

function Frame:punch(region)
	local b1, b2, b3, b4 = self:getBorder()
	local r = self.Rect
	region:subRect(r[1] - b1, r[2] - b2, r[3] + b3, r[4] + b4)
end

-------------------------------------------------------------------------------
--	Frame:drawBorder(): This function draws the element's outer border.
-------------------------------------------------------------------------------

function Frame:drawBorder()
	self.BorderClass:draw(self, self.Border, unpack(self.Rect))
end

-------------------------------------------------------------------------------
--	draw: overrides
-------------------------------------------------------------------------------

function Frame:draw()
	Area.draw(self)
	local b1, b2, b3, b4 = self:getIBorder()
	local r = self.Rect
	b1, b2, b3, b4 = r[1] + b1, r[2] + b2, r[3] - b3, r[4] - b4
	self.IBorderClass:draw(self, self.IBorder, b1, b2, b3, b4)
end

-------------------------------------------------------------------------------
--	refresh: overrides
-------------------------------------------------------------------------------

function Frame:refresh()
	Area.refresh(self)
	if self.RedrawBorder then
		self:drawBorder()
		self.RedrawBorder = false
	end
end

-------------------------------------------------------------------------------
--	Frame:onFocus(focused): This method is invoked when the element's
--	{{Focus}} attribute has changed (see also [[#tek.ui.class.area : Area]]).
-------------------------------------------------------------------------------

function Frame:onFocus(focused)
	self.Window:setFocusElement(focused and self)
	self.RedrawBorder = true
	self.Redraw = true -- TODO: ugly; needed to redraw the border
	self:setState()
end
