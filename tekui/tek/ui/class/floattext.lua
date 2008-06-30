-------------------------------------------------------------------------------
--
--	tek.ui.class.floattext
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	LINEAGE::
--		[[#ClassOverview]] :
--		[[#tek.class : Class]] /
--		[[#tek.class.object : Object]] /
--		[[#tek.ui.class.element : Element]] /
--		[[#tek.ui.class.area : Area]] /
--		FloatText
--
--	OVERVIEW::
--		Implements a scrollable text display. This class is
--		normally the direct child of a [[#tek.ui.class.canvas : Canvas]]
--
--	ATTRIBUTES::
--		- {{BGPen [IG]}}
--			Pen for filling the background
--		- {{FGPen [IG]}}
--			Pen for rendering the text
--		- {{FontSpec [IG]}}
--			Font specifier; see [[#tek.ui.class.text : Text]] for a
--			format description
--		- {{Text [ISG]]}}
--			The text to be displayed
--
--	IMPLEMENTS::
--		- FloatText:onSetText() - Handler called when {{Text}} is changed
--
--	OVERRIDES::
--		- Area:askMinMax()
--		- Element:cleanup()
--		- Object.init()
--		- Area:layout()
--		- Class.new()
--		- Area:setState()
--		- Element:setup()
--
-------------------------------------------------------------------------------

local ui = require "tek.ui"
local Region = require "tek.lib.region"
local Area = ui.Area

local concat = table.concat
local insert = table.insert
local ipairs = ipairs
local max = math.max
local overlap = Region.overlapCoords
local remove = table.remove
local unpack = unpack

module("tek.ui.class.floattext", tek.ui.class.area)
_VERSION = "FloatText 3.1"

local FloatText = _M

-------------------------------------------------------------------------------
--	Constants & Class data:
-------------------------------------------------------------------------------

local DEF_MARGIN = { 0, 0, 0, 0 }
local NOTIFY_TEXT = { ui.NOTIFY_SELF, "onSetText" }

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

function FloatText.init(self)
	self.BGPen = self.BGPen or ui.PEN_LISTVIEWBACK
	self.Canvas = false
	self.CanvasHeight = false
	self.FGPen = self.FGPen or ui.PEN_BUTTONTEXT
	self.FHeight = false
	self.Font = false
	self.FontSpec = self.FontSpec or false
	self.FWidth = false
	self.Lines = false
	self.Margin = DEF_MARGIN
	self.TrackDamage = self.TrackDamage or true
	self.UnusedRegion = false
	self.WordLengths = false
	self.WordSpacing = false
	return Area.init(self)
end

-------------------------------------------------------------------------------
--	setup: overrides
-------------------------------------------------------------------------------

function FloatText:setup(app, window)
	self.Canvas = self.Parent
	Area.setup(self, app, window)
	self:addNotify("Text", ui.NOTIFY_CHANGE, NOTIFY_TEXT)
end

-------------------------------------------------------------------------------
--	cleanup: overrides
-------------------------------------------------------------------------------

function FloatText:cleanup()
	self:remNotify("Text", ui.NOTIFY_CHANGE, NOTIFY_TEXT)
	Area.cleanup(self)
	self.Canvas = false
end

-------------------------------------------------------------------------------
--	show: overrides
-------------------------------------------------------------------------------

function FloatText:show(display, drawable)
	if Area.show(self, display, drawable) then
		self.Font = display:openFont(self.FontSpec)
		self.FWidth, self.FHeight = ui.Display:getTextSize(self.Font, "W")
		self:prepareText()
		return true
	end
end

-------------------------------------------------------------------------------
--	hide: overrides
-------------------------------------------------------------------------------

function FloatText:hide()
	self.Display:closeFont(self.Font)
	self.Font = false
	Area.hide(self)
end

-------------------------------------------------------------------------------
--	draw: overrides
-------------------------------------------------------------------------------

function FloatText:draw()

	local d = self.Drawable
	local p = d.Pens
	local bp = p[self.Background]

	-- repaint intra-area damagerects:
	local dr = self.DamageRegion
	if dr then
		-- determine visible rectangle:
		local ca = self.Canvas
		local x0 = ca and ca.CanvasLeft or 0
		local x1 = ca and x0 + ca.CanvasWidth - 1 or self.Rect[3]

		local fp = p[self.FGPen]
		d:setFont(self.Font)
		for _, r in dr:getRects() do
			local r1, r2, r3, r4 = dr:getRect(r)
			d:pushClipRect(r1, r2, r3, r4)
			for lnr, t in ipairs(self.Lines) do
				-- overlap between damage and line:
				if overlap(r1, r2, r3, r4, x0, t[2], x1, t[4]) then
					-- draw line background:
					d:fillRect(x0, t[2], x1, t[4], bp)
					-- overlap between damage and text:
					if overlap(r1, r2, r3, r4, t[1], t[2], t[3], t[4]) then
						-- draw text:
						d:drawText(t[1], t[2], t[5], fp)
					end
				end
			end
			d:popClipRect()
		end
		self.DamageRegion = false
	end

	local ur = self.UnusedRegion
	if ur then
		for _, r in ur:getRects() do
			local r1, r2, r3, r4 = ur:getRect(r)
			d:fillRect(r1, r2, r3, r4, bp)
		end
	end

end

-------------------------------------------------------------------------------
--	prepareText: internal
-------------------------------------------------------------------------------

function FloatText:prepareText()
	local lw = 0 -- longest word in text
	local w, h
	local wl = { }
	self.WordLengths = wl
	local i = 0
	for spc, word in self.Text:gmatch("(%s*)([^%s]*)") do
		w, h = ui.Display:getTextSize(self.Font, word)
		lw = max(lw, w)
		i = i + 1
		wl[i] = w
	end
	self.WordSpacing = ui.Display:getTextSize(self.Font, " ")
	self.MinWidth, self.MinHeight = lw, h
	return lw, h
end

-------------------------------------------------------------------------------
--	askMinMax: overrides
-------------------------------------------------------------------------------

function FloatText:askMinMax(m1, m2, m3, m4)
	m1 = m1 + self.MinWidth
	m2 = m2 + self.FHeight
	m3 = ui.HUGE
	m4 = ui.HUGE
	return Area.askMinMax(self, m1, m2, m3, m4)
end

-------------------------------------------------------------------------------
--	layoutText: internal
-------------------------------------------------------------------------------

local function insline(text, line, x, y, tw, so, fh)
	line = line and concat(line, " ")
	insert(text, { x, y, x + tw + so - 1, y + fh - 1, line })
end

function FloatText:layoutText(x, y, width, text)
	text = text or { }

	local line = false
	local fh = self.FHeight
	local tw = 0
	local i = 0
	local wl = self.WordLengths
	local ws = self.WordSpacing
	local so = 0

	for spc, word in self.Text:gmatch("(%s*)([^%s]*)") do
		for s in spc:gmatch("%s") do
			if s == "\n" then
				insline(text, line, x, y, tw, so, fh)
				y = y + fh
				line = false
				so = 0
				tw = 0
			end
		end
		if word then
			line = line or { }
			insert(line, word)
			i = i + 1
			tw = tw + wl[i]
			if tw + so > width then
				if #line > 0 then
					remove(line)
					insline(text, line, x, y, tw, so, fh)
					y = y + fh
				end
				line = { word }
				so = 0
				tw = wl[i]
			end
			so = so + ws
		end
	end
	if line and #line > 0 then
		insline(text, line, x, y, tw, so, fh)
		y = y + fh
	end

	return text, y
end

-------------------------------------------------------------------------------
--	layout: overrides
-------------------------------------------------------------------------------

function FloatText:layout(r1, r2, r3, r4, markdamage)
	local redraw = self.Redraw
	local width = r3 - r1 + 1
	local ch = self.CanvasHeight
	local r = self.Rect
	local m = self.MarginAndBorder
	local x0 = r1 + m[1]
	local y0 = r2 + m[2]
	local x1 = r3 - m[3]
	if not ch or (r[1] and r[3] - r[1] + 1 ~= width) then
		self.Lines, ch = self:layoutText(r1, r2, width)
		self.CanvasHeight = ch
		redraw = true
	end
	local y1 = self.Canvas and r2 + ch - 1 - m[4] or r4 - m[4]
	if redraw or markdamage or
		r[1] ~= x0 or r[2] ~= y0 or r[3] ~= x1 or r[4] ~= y1 then
		if self.Canvas then
			self.Canvas:setValue("CanvasHeight", self.CanvasHeight)
		end
		self.DamageRegion = Region.new(x0, y0, x1, y1)
		if not markdamage and not redraw and r[1] == x0 and r[2] == y0 then
			self.DamageRegion:subRect(r[1], r[2], r[3], r[4])
		end
		r[1], r[2], r[3], r[4] = x0, y0, x1, y1
		self:updateUnusedRegion()
		self.Redraw = true
		return true
	end
end

-------------------------------------------------------------------------------
--	setState: overrides
-------------------------------------------------------------------------------

function FloatText:setState(bg)
	Area.setState(self, bg or self.BGPen)
end

-------------------------------------------------------------------------------
--	onSetText(text): Handler called when a new {{Text}} is set.
-------------------------------------------------------------------------------

function FloatText:onSetText()
	self:prepareText()
	self.CanvasHeight = false
	self:rethinkLayout(2)
end

-------------------------------------------------------------------------------
--	updateUnusedRegion: internal
-------------------------------------------------------------------------------

function FloatText:updateUnusedRegion()
	-- determine unused region:
	local m = self.MarginAndBorder
	local r = self.Rect
	self.UnusedRegion = Region.new(r[1] + m[1], r[2] + m[2], r[3] - m[3],
		r[4] - m[4])
	self.UnusedRegion:subRect(r[1] + m[1], r[2] + m[2], r[3] - m[3],
		r[2] + m[2] + self.CanvasHeight - 1)
end
