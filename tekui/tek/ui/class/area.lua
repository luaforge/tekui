-------------------------------------------------------------------------------
--
--	tek.ui.class.area
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	LINEAGE::
--		[[#ClassOverview]] :
--		[[#tek.class : Class]] /
--		[[#tek.class.object : Object]] /
--		[[#tek.ui.class.element : Element]] /
--		Area
--
--	OVERVIEW::
--		This class implements an outer margin, layouting and drawing.
--
--	ATTRIBUTES::
--		- {{BGPen [G]}} (userdata)
--			A colored pen for painting the background of the element
--		- {{DamageRegion [G]}} ([[#tek.lib.region : Region]])
--			see {{TrackDamage}}
--		- {{Disabled [ISG]}} (boolean)
--			If '''true''', the element is in disabled state. This attribute is
--			not handled by Area; see [[#tek.ui.class.gadget : Gadget]]
--		- {{Focus [ISG]}} (boolean)
--			If '''true''', the element has the input focus. This attribute
--			is not handled by Area; see [[#tek.ui.class.frame : Frame]]
--		- {{HAlign [IG]}} ("left", "center", "right")
--			Horizontal alignment of the element in its group [default:
--			"left"]
--		- {{Height [IG]}} (number, '''false''', or "auto", "fill", "free")
--			Height of the element, in pixels, or
--				- '''false''' - unspecified; during initialization, the class'
--				default will be used
--				- "auto" - Reserves the minimal height needed for the element.
--				- "free" - Allows the element's height to grow to any size.
--				- "fill" - Completely fills up the height that other elements
--				in the same group have left, but does not claim more.
--			Note: Normally, "fill" is useful only once per group.
--		- {{Hilite [ISG]}} (boolean)
--			If '''true''', the element is in highlighted state. This
--			attribute is not handled by Area; see
--			[[#tek.ui.class.gadget : Gadget]]
--		- {{Margin [IG]}} (table)
--			An array of four offsets for the element's outer margin in the
--			order left, right, top, bottom [pixels]. If unspecified during
--			initialization, the class' default margins are used.
--		- {{MaxHeight [IG]}} (number)
--			Maximum height of the element, in pixels [default: {{ui.HUGE}}]
--		- {{MaxWidth [IG]}} (number)
--			Maximum width of the element, in pixels [default: {{ui.HUGE}}]
--		- {{MinHeight [IG]}} (number)
--			Minimum height of the element, in pixels [default: 0]
--		- {{MinWidth [IG]}} (number)
--			Minimum width of the element, in pixels [default: 0]
--		- {{Selected [ISG]}} (boolean)
--			If '''true''', the element is in selected state. This
--			attribute is not handled by Area; see
--			[[#tek.ui.class.gadget : Gadget]]
--		- {{TrackDamage [IG]}} (boolean)
--			If '''true''', the element gathers intra-area damages in a
--			Region named {{DamageRegion}}, which can be used by class writers
--			for implementing minimally invasive redrawing. [Default:
--			'''false''', the element is redrawn in its entirety.)
--		- {{VAlign [IG]}} ("top", "center", "bottom")
--			Vertical alignment of the element in its group [default: "top"]
--		- {{Weight [ISG]}} (number)
--			Determines the weight that is attributed to the element, relative
--			to its siblings in its group. Invokes the Area:onSetWeight()
--			method. Area:onSetWeight() recalculates all weights in the group
--			and possibly causes relayouting and redrawing it. Note: By
--			recommendation, the weights set in a group should sum up to
--			0x10000.
--		- {{Width [IG]}} (number, '''false''', or "auto", "fill", "free")
--			Width of the element, in pixels, or
--				- '''false''' - unspecified; during initialization, the class'
--				default will be used
--				- "auto" - Reserves the minimal width needed for the element.
--				- "free" - Allows the element's width to grow to any size.
--				- "fill" - Completely fills up the width that other elements
--				in the same group have left, but does not claim more.
--			Note: Normally, "fill" is useful only once per group.
--
--	IMPLEMENTS::
--		- Area:askMinMax() - Query minimum and maximum dimensions
--		- Area:checkFocus() - Check if the element can receive the focus
--		- Area:draw() - Draws the element
--		- Area:getElement() - Returns an element's neighbours
--		- Area:getElementByXY() - Check if the element covers a coordinate
--		- Area:hide() - Removes an Area from its Display and Drawable
--		- Area:layout() - Layout the element into a rectangle
--		- Area:markDamage() - Pass a damage rectangle to an element
--		- Area:onSetWeight() - Handler called when Area's weight is changed
--		- Area:passMsg() - Filters an input message
--		- Area:punch() - Subtract Element from a [[#tek.lib.region : Region]]
--		- Area:refresh() - Redraws an element
--		- Area:relayout() - Search and relayouts an element
--		- Area:rethinkLayout() - Causes relayout of the Area and its group
--		- Area:setState() - Sets the Background attribute of an element.
--		- Area:show() - Passes an Area a Display and Drawable
--
--	OVERRIDES::
--		- Element:cleanup()
--		- Object.init()
--		- Class.new()
--		- Element:setup()
--
-------------------------------------------------------------------------------

local db = require "tek.lib.debug"
local Region = require "tek.lib.region"
local ui = require "tek.ui"
local Element = ui.Element
local overlap = Region.overlapCoords

local floor = math.floor
local insert = table.insert
local ipairs = ipairs
local max = math.max
local min = math.min
local remove = table.remove
local tonumber = tonumber

module("tek.ui.class.area", tek.ui.class.element)
_VERSION = "Area 9.0"
local Area = _M

-------------------------------------------------------------------------------
--	Constants & Class data:
-------------------------------------------------------------------------------

local DEF_MARGIN = { 2, 2, 2, 2 }
local NOTIFY_WEIGHT = { ui.NOTIFY_SELF, "onSetWeight", ui.NOTIFY_VALUE }

-------------------------------------------------------------------------------
--	new:
-------------------------------------------------------------------------------

function Area.new(class, self)
	self = self or { }
	-- Combined margin and border offsets of the element:
	self.MarginAndBorder = { 0, 0, 0, 0 }
	-- Calculated minimum/maximum sizes of the element:
	self.MinMax = { 0, 0, 0, 0 }
	-- The layouted rectangle of the element on the display:
	self.Rect = { -1, -1, -1, -1 }
	return Element.new(class, self)
end

-------------------------------------------------------------------------------
--	init:
-------------------------------------------------------------------------------

function Area.init(self)
	-- Element's background properties (color, pattern...) [internal]:
	self.Background = false
	-- Background pen:
	self.BGPen = self.BGPen or false
	-- Region to collect damages to this element:
	self.DamageRegion = false
	-- Disabled state of the element (defined, but not handled by Area):
	self.Disabled = self.Disabled or false
	-- The Display this element is connected to [internal]:
	self.Display = false
	-- The Drawable this element is connected to [internal]:
	self.Drawable = false
	-- Focus state of the element (defined, but not handled by Area):
	self.Focus = self.Focus or false
	-- Horizontal alignment in group ("left", "center", "right")
	self.HAlign = self.HAlign or "left"
	-- Fixed height:
	self.Height = self.Height or false
	-- Hilite state of the element (defined, but not handled by Area):
	self.Hilite = false
	-- Margin offsets of the element:
	self.Margin = self.Margin or false
	-- Maximum height of the element:
	self.MaxHeight = self.MaxHeight or ui.HUGE
	-- Maximum width of the element:
	self.MaxWidth = self.MaxWidth or ui.HUGE
	-- Minimum height of the element:
	self.MinHeight = self.MinHeight or 0
	-- Minimum width of the element:
	self.MinWidth = self.MinWidth or 0
	-- Indicates whether the element needs to be redrawn [internal]:
	self.Redraw = false
	-- Selected state of the element (defined, but not handled by Area):
	self.Selected = self.Selected or false
	-- Boolean to indicate whether intra-area damages are to be collected:
	self.TrackDamage = self.TrackDamage or false
	-- Vertical alignment in group ("top", "center", "bottom")
	self.VAlign = self.VAlign or "top"
	-- Weight of the element, relative to its siblings in the same group:
	self.Weight = self.Weight or false
	-- Fixed width:
	self.Width = self.Width or false
	return Element.init(self)
end

-------------------------------------------------------------------------------
--	setup: overrides
-------------------------------------------------------------------------------

function Area:setup(app, window)
	Element.setup(self, app, window)
	self:addNotify("Weight", ui.NOTIFY_CHANGE, NOTIFY_WEIGHT)
end

-------------------------------------------------------------------------------
--	cleanup: overrides
-------------------------------------------------------------------------------

function Area:cleanup()
	self:remNotify("Weight", ui.NOTIFY_CHANGE, NOTIFY_WEIGHT)
	Element.cleanup(self)
end

-------------------------------------------------------------------------------
--	success = Area:show(display, drawable): Passes an element the
--	[[#tek.ui.class.display : Display]] and
--	[[#tek.ui.class.drawable : Drawable]] it will be rendered to. Returns
--	a boolean indicating success. If you override this method, pass the call
--	to your super class and check and propagate its return value. See also:
--	Area:hide().
-------------------------------------------------------------------------------

function Area:show(display, drawable)
	self.Margin = self.Margin or display.Theme.AreaMargin or DEF_MARGIN
	self.Display = display
	self.Drawable = drawable
	self:calcOffsets()
	self:setState()
	return true
end

-------------------------------------------------------------------------------
--	Area:hide(): Removes the display and drawable from an element.
--	Override this method to free all display-related resources previously
--	allocated in Area:show().
-------------------------------------------------------------------------------

function Area:hide()
	self.DamageRegion = false
	self.Drawable = false
	self.Display = false
end

-------------------------------------------------------------------------------
--	Area:calcOffsets() [internal] - This function calculates the
--	{{MarginAndBorder}} property. See also Frame:calcOffsets().
-------------------------------------------------------------------------------

function Area:calcOffsets()
	local s, d = self.Margin, self.MarginAndBorder
	d[1], d[2], d[3], d[4] = s[1], s[2], s[3], s[4]
end

-------------------------------------------------------------------------------
--	Area:rethinkLayout(damage): This method causes a relayout of the
--	element and possibly the [[#tek.ui.class.group : Group]] in which it
--	resides. While the request bubbles up to the topmost group (the
--	[[#tek.ui.class.window : Window]]), the argument {{damage}} (a boolean)
--	can be used to avoid unconditionally marking parent groups as damaged,
--	thus allowing them to be repainted only if their layout actually changed
--	due to modifications in their children.
-------------------------------------------------------------------------------

function Area:rethinkLayout(damage)
	if self.Display then
		self:calcOffsets()
		local parent = self:getElement("parent")
		self.Window:addLayoutGroup(parent, damage)
		-- this causes the rethink to bubble up until it reaches the window:
		parent:rethinkLayout(false) -- cause no more damage
	else
		db.info("%s : Cannot rethink layout - not connected to a display",
			self:getClassName())
	end
end

-------------------------------------------------------------------------------
--	onSetWeight(weight): Sets the weight of the element relative to the
--	group in which it resides, and causes a relayout of this group.
-------------------------------------------------------------------------------

function Area:onSetWeight(w)
	self.Weight = tonumber(w) or false
	self.Parent:calcWeights()
	self:rethinkLayout(true)
end

-------------------------------------------------------------------------------
--	minw, minh, maxw, maxh = Area:askMinMax(minw, minh, maxw, maxh): This
--	method is called during the layouting process for adding the required
--	spatial extents (width and height) of this class to the min/max values
--	passed from a child class, before passing them on to its super class.
--	{{minw}}, {{minh}} are cumulative of the minimal size of the element,
--	while {{maxw}}, {{maxw}} collect the size the element is allowed to
--	expand to. Use {{ui.HUGE}} to indicate a 'huge' spatial extent.
-------------------------------------------------------------------------------

function Area:askMinMax(m1, m2, m3, m4)
	m1 = max(self.MinWidth, m1)
	m2 = max(self.MinHeight, m2)
	m3 = max(min(self.MaxWidth, m3), m1)
	m4 = max(min(self.MaxHeight, m4), m2)
	local m = self.MarginAndBorder
	m1 = m1 + m[1] + m[3]
	m2 = m2 + m[2] + m[4]
	m3 = m3 + m[1] + m[3]
	m4 = m4 + m[2] + m[4]
	local mm = self.MinMax
	mm[1], mm[2], mm[3], mm[4] = m1, m2, m3, m4
	return m1, m2, m3, m4
end

-------------------------------------------------------------------------------
--	changed = Area:layout(x0, y0, x1, y1[, markdamage]): Layouts the element
--	into the specified rectangle. If the element's (or any of its childrens')
--	coordinates change, returns '''true''' and marks the element as damaged,
--	unless the optional argument {{markdamage}} is set to '''false'''.
-------------------------------------------------------------------------------

function Area:layout(x0, y0, x1, y1, markdamage)

	local r = self.Rect
	local m = self.MarginAndBorder

	x0 = x0 + m[1]
	y0 = y0 + m[2]
	x1 = x1 - m[3]
	y1 = y1 - m[4]

	if r[1] ~= x0 or r[2] ~= y0 or r[3] ~= x1 or r[4] ~= y1 then

		-- shift, size:
		local dx, dy, dw, dh
		if r[1] then
			dx, dy = x0 - r[1], y0 - r[2]
			dw, dh = x1 - x0 - r[3] + r[1], y1 - y0 - r[4] + r[2]
		end

		-- cannot refresh by copy if element is shifted:
		local sx, sy = self.Drawable:getShift()

		if dx and sx == 0 and sy == 0 and ((dx == 0) ~= (dy == 0)) and
			((dw == 0 and dh == 0) or self.TrackDamage) then
			-- can refresh this element by shifting:

			local s1, s2, s3, s4 = overlap(r[1] - m[1], r[2] - m[2],
				r[3] + m[3], r[4] + m[4], x0 - dx - m[1], y0 - dy - m[2],
				x1 - dx + m[3], y1 - dy + m[4])
			if s1 then
				local key = ("%d:%d"):format(dx, dy)
				-- local key = dx == 0 and dy or dx
				local ca = self.Window.CopyArea
				if ca[key] then
					ca[key][3]:orRect(s1, s2, s3, s4)
				else
					ca[key] = { dx, dy, Region.new(s1, s2, s3, s4) }
				end
				-- redraw background:
				if self.Parent then
					self.Parent.Redraw = true
				end
			end

			if dw > 0 or dh > 0 then
				-- grow + move:
				self.DamageRegion = Region.new(x0, y0, x1, y1)
				self.DamageRegion:subRect(r[1] + dx, r[2] + dy, r[3] + dx,
					r[4] + dy)
			end

		else
			-- something changed:
			if self.TrackDamage then
				self.DamageRegion = Region.new(x0, y0, x1, y1)
				-- avoid damages from resizing the area without moving it:
				if dx == 0 and dy == 0 then
					self.DamageRegion:subRect(r[1], r[2], r[3], r[4])
				end
			end
			if markdamage ~= false then
				self.Redraw = true
			end
		end

		r[1], r[2], r[3], r[4] = x0, y0, x1, y1
		return true

	else
		-- nothing changed:
		self.DamageRegion = false
	end

end

-------------------------------------------------------------------------------
--	found[, changed] = Area:relayout(element, x0, y0, x1, y1) [internal]:
--	Searches for the specified element, and if this class (or the class of
--	one of its children) is responsible for it, layouts it to the specified
--	rectangle. Returns '''true''' if the element was found and its layout
--	updated. A secondary return value of '''true''' indicates whether
--	relayouting actually caused a change, i.e. a damage to the object.
-------------------------------------------------------------------------------

function Area:relayout(e, r1, r2, r3, r4)
	if self == e then
		return true, self:layout(r1, r2, r3, r4)
	end
end

-------------------------------------------------------------------------------
--	Area:punch(region) [internal]: Subtracts the element from (punching a
--	hole into) the specified Region. This function is called by the layouter.
-------------------------------------------------------------------------------

function Area:punch(region)
	local r = self.Rect
	region:subRect(r[1], r[2], r[3], r[4])
end

-------------------------------------------------------------------------------
--	Area:markDamage(x0, y0, x1, y1): If the element overlaps with the given
--	rectangle, this function marks it as damaged.
-------------------------------------------------------------------------------

function Area:markDamage(r1, r2, r3, r4)
	if self.TrackDamage or not self.Redraw then
		local r = self.Rect
		r1, r2, r3, r4 = overlap(r1, r2, r3, r4, r[1], r[2], r[3], r[4])
		if r1 then
			self.Redraw = true
			if self.DamageRegion then
				self.DamageRegion:orRect(r1, r2, r3, r4)
			elseif self.TrackDamage then
				self.DamageRegion = Region.new(r1, r2, r3, r4)
			end
		end
	end
end

-------------------------------------------------------------------------------
--	Area:draw(): Draws the element into the rectangle assigned to it by
--	the layouter; the coordinates can be found in the element's {{Rect}}
--	table. Note: Applications are not allowed to call this function directly.
-------------------------------------------------------------------------------

function Area:draw()
	local d = self.Drawable
	local bgpen = d.Pens[self.Background]
	local dr = self.DamageRegion
	if dr then
		-- repaint intra-area damagerects:
		for _, r in dr:getRects() do
			local r1, r2, r3, r4 = dr:getRect(r)
			d:fillRect(r1, r2, r3, r4, bgpen)
		end
		self.DamageRegion = false
	else
		local r = self.Rect
		d:fillRect(r[1], r[2], r[3], r[4], bgpen)
	end
end

-------------------------------------------------------------------------------
--	Area:refresh() [internal]: Redraws the element (and all possible children)
--	if they are marked as damaged. Note: This function is called inside the
--	[[#tek.ui.class.window : Window]] class; applications and classes are
--	not allowed to call it themselves.
-------------------------------------------------------------------------------

function Area:refresh()
	if self.Redraw then
		self:draw()
		self.Redraw = false
	end
end

-------------------------------------------------------------------------------
--	self = Area:getElementByXY(x, y): Returns {{self}} if the element covers
--	the specified coordinate.
-------------------------------------------------------------------------------

function Area:getElementByXY(x, y)
	local r = self.Rect
	return x >= r[1] and x <= r[3] and y >= r[2] and y <= r[4] and self
end

-------------------------------------------------------------------------------
--	msg = Area:passMsg(msg): This function filters the specified input
--	message. After processing, it is free to return the message unmodified
--	(thus passing it on to the next message handler), to return a copy that
--	has certain fields in the message modified, or to 'swallow' the message
--	by returning '''false'''. If you override this function, you are not
--	allowed to modify any data inside the original message; to alter a
--	message, you must operate on and return a copy.
-------------------------------------------------------------------------------

function Area:passMsg(msg)
	return msg
end

-------------------------------------------------------------------------------
--	Area:setState(bg): Sets the {{Background}} attribute according to
--	the state of the element, and if it changed, slates the element
--	for repainting.
-------------------------------------------------------------------------------

function Area:setState(bg)
	bg = bg or self.BGPen or ui.PEN_AREABACK
	if bg ~= self.Background then
		self.Background = bg
		self.Redraw = true
	end
end

-------------------------------------------------------------------------------
--	can_receive = Area:checkFocus(): Returns '''true''' if this element can
--	receive the input focus. (As an Area is non-interactive, the return value
--	of this class' implementation is always '''false'''.)
-------------------------------------------------------------------------------

function Area:checkFocus()
	return false
end

-------------------------------------------------------------------------------
--	element = Area:getElement(mode): Returns an element's neighbours. This
--	function can be overridden to control a class-specific tab cycle behavior.
--	Possible values for {{mode}} are:
--		- "parent" - returns the elements' parent element.
--		- "children" - returns a table containing the element's children, or
--		'''nil''' if the element has no children.
--		- "siblings" - returns a table containing the element's siblings
--		(including the element itself), or a table containing only the
--		element, if it is not member of a group.
--		- "next" - returns the next element in the group, or '''nil''' if
--		the element has no successors.
--		- "prev" - returns the previous element in the group, or '''nil''' if
--		the element has no predecessors.
--		- "nextorparent" - returns the next element in a group, or, if the
--		element has no successor, the next element in the parent group (and
--		so forth, until it reaches the topmost group).
--		- "prevorparent" - returns the previous element in a group, or, if the
--		element has no predecessor, the next element in the parent group (and
--		so forth, until it reaches the topmost group).
--		- "firstchild" - returns the element's first child, or '''nil''' if
--		the element has no children.
--		- "lastchild" - returns the element's last child, or '''nil''' if
--		the element has no children.
--
--	Note: Tables returned by this function must be considered read-only.
-------------------------------------------------------------------------------

function Area:getElement(mode)
	if mode == "parent" then
		return self.Parent
	elseif mode == "children" then
		return -- an area has no children
	elseif mode == "siblings" then
		local p = self:getElement("parent")
		return p and p:getElement("children")
	end
	local g = self:getElement("siblings")
	if g then
		local n = #g
		for i, e in ipairs(g) do
			if e == self then
				if mode == "next" then
					return g[i % n + 1]
				elseif mode == "prev" then
					return g[(i - 2) % n + 1]
				elseif mode == "nextorparent" then
					if i == n then
						return self:getElement("parent"):
							getElement("nextorparent")
					end
					return g[i % n + 1]
				elseif mode == "prevorparent" then
					if i == 1 then
						return self:getElement("parent"):
							getElement("prevorparent")
					end
					return g[(i - 2) % n + 1]
				end
				break
			end
		end
	end
end
