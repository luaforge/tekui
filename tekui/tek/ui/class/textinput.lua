-------------------------------------------------------------------------------
--
--	tek.ui.class.textinput
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
--		TextInput
--
--	OVERVIEW::
--		This class implements a gadget for editing and entering text.
--
--	ATTRIBUTES::
--		- {{Enter [ISG]}} - Text that is being 'entered' (by pressing the
--		return key) into the TextInput field.
--
--	IMPLEMENTS::
--		- TextInput:onEnter() - Handler invoked when {{Enter}} ist set
--
--	OVERRIDES::
--		- Area:askMinMax()
--		- Element:cleanup()
--		- Area:draw()
--		- Element:hide()
--		- Object.init()
--		- Frame:onFocus()
--		- Gadget:onSelect()
--		- Text:onSetText()
--		- Area:passMsg()
--		- Area:setState()
--		- Element:setup()
--		- Element:show()
--
-------------------------------------------------------------------------------

local ui = require "tek.ui"

local Area = ui.Area
local Display = ui.Display
local Gadget = ui.Gadget
local Text = ui.Text
local UTF8String = require "tek.class.utf8string"

local char = string.char
local floor = math.floor
local min = math.min
local max = math.max
local unpack = unpack

module("tek.ui.class.textinput", tek.ui.class.text)
_VERSION = "TextInput 4.3"

-------------------------------------------------------------------------------
--	Constants & Class data:
-------------------------------------------------------------------------------

local DEF_BORDERSTYLE = "recess"

local NOTIFY_ENTER = { ui.NOTIFY_SELF, "onEnter", ui.NOTIFY_VALUE }

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

local TextInput = _M

function TextInput.init(self)
	self.BlinkState = false
	self.BlinkTick = false
	self.BlinkTickInit = false
	self.Editing = false
	self.Enter = false
	self.FHeight = false
	self.FWidth = false
	self.IntervalNotify = { self, "interval" }
	self.Text = self.Text or ""
	self.TextBuffer = UTF8String:new(self.Text)
	self.Mode = "touch"
	self.ShortcutMark = false
	-- cursor position in characters:
	self.TextCursor = self.TextCursor or self.TextBuffer:len() + 1
	-- character offset in displayed text:
	self.TextOffset = false
	-- rectangle of text:
	self.TextRect = false
	-- max. visible width in characters:
	self.TextWidth = false
	return Text.init(self)
end

-------------------------------------------------------------------------------
--	setup: overrides
-------------------------------------------------------------------------------

function TextInput:setup(app, window)
	Text.setup(self, app, window)
	self:addNotify("Enter", ui.NOTIFY_ALWAYS, NOTIFY_ENTER)
end

-------------------------------------------------------------------------------
--	cleanup: overrides
-------------------------------------------------------------------------------

function TextInput:cleanup()
	self:remNotify("Enter", ui.NOTIFY_ALWAYS, NOTIFY_ENTER)
	Text.cleanup(self)
end

-------------------------------------------------------------------------------
--	show: overrides
-------------------------------------------------------------------------------

function TextInput:show(display, drawable)
	local theme = display.Theme
	self.FontSpec = self.FontSpec or theme.TextInputFontSpec or "__fixed"
	self.BorderStyle = self.BorderStyle or theme.TextInputBorderStyle or
		DEF_BORDERSTYLE
	if Text.show(self, display, drawable) then
		self.FWidth, self.FHeight =
			Display:getTextSize(self.TextRecords[1][2], " ")
		self.TextRect = { }
		self.TextOffset = 0
		self.BlinkTick = 0
		self.BlinkTickInit = 18
		return true
	end
end

-------------------------------------------------------------------------------
--	hide: overrides
-------------------------------------------------------------------------------

function TextInput:hide()
	Text.hide(self)
	self.TextOffset = 0
	self.TextCursor = 0
	self.TextRect = false
end

-------------------------------------------------------------------------------
--	askMinMax: overrides
-------------------------------------------------------------------------------

function TextInput:askMinMax(m1, m2, m3, m4)
	local w, h = self.FWidth * 2, self.FHeight -- +1 char for cursor
	m1 = m1 + w + 1 -- +1 for disabled state
	m2 = m2 + h + 1
	m3 = m3 + w + 1
	m4 = m4 + h + 1
	return Gadget.askMinMax(self, m1, m2, m3, m4)
end

-------------------------------------------------------------------------------
--	draw: overrides
-------------------------------------------------------------------------------

function TextInput:draw()

	Area.draw(self)

	local r = self.Rect
	local d = self.Drawable
	local pens = d.Pens
	local bgpen = pens[self.Background]

	local tr = self.TextRect
	local to = self.TextOffset
	local tc = self.TextCursor

	local text = self.TextRecords[1]
	d:setFont(text[2])

	local fw, fh = self.FWidth, self.FHeight
	local p = self.PaddingAndBorder
	local w = r[3] - r[1] + 1 - p[1] - p[3]
	local h = r[4] - r[2] + 1 - p[2] - p[4]

	-- total len of text, in characters:
	local len = self.TextBuffer:len()

	-- max. visible width in characters:
	local tw = floor(w / fw)

	if tc >= tw then
		to = to + (tc - tw + 1)
		tc = tw - 1
	end
	tc = min(tc, len - to)

	self.TextWidth = tw
	self.TextOffset = to
	self.TextCursor = tc

	-- total len of text, including space required for cursor:
	local clen = len
	if to + tc >= len then
		clen = clen + 1
	end

	-- actual visible TextWidth (including cursor) in characters:
	local twc = min(tw, clen)

	-- actual visible TextWidth in pixels:
	tw = twc * fw

	-- the visible text:
	text = self.TextBuffer:sub(to + 1, to + twc)

	-- visible text rect, left aligned:
	tr[1] = r[1] + p[1] -- centered: + (w - tw) / 2
	tr[2] = r[2] + p[2] + floor((h - fh) / 2)
	tr[3] = tr[1] + tw - 1
	tr[4] = tr[2] + fh - 1

	local x, y = tr[1], tr[2]

	if self.Disabled then
		d:drawText(x + 1, y + 1, text, pens[ui.PEN_BUTTONDISABLEDSHINE])
		d:drawText(x, y, text, pens[ui.PEN_BUTTONDISABLEDSHADOW])
	else
		d:drawText(x, y, text, pens[ui.PEN_TEXTINPUTTEXT], bgpen)
		if self.Window.FocusElement == self then
			local s = self.TextBuffer:sub(tc + to + 1, tc + to + 1)
			s = s == "" and " " or s
			if self.BlinkState == 1 then
				d:drawText(tr[1] + tc * fw, tr[2], s, pens[ui.PEN_CURSORTEXT],
					pens[ui.PEN_CURSOR])
			else
				d:drawText(tr[1] + tc * fw, tr[2], s,
					pens[ui.PEN_TEXTINPUTTEXT], bgpen)
			end
		end
	end
end

-------------------------------------------------------------------------------
--	clickMouse: internal
-------------------------------------------------------------------------------

function TextInput:clickMouse(x, y)
	if x and y then
		local tr, tc = self.TextRect, self.TextCursor
		local fw, fh = self.FWidth, self.FHeight
		if x >= tr[1] and x <= tr[3] and y >= tr[2] and y <= tr[4] then
			tc = floor((x - tr[1]) / fw)
		elseif x < tr[1] then
			tc = 0
		elseif x > tr[3] then
			tc = floor((tr[3] - tr[1]) / fw) + 1
		end
		if tc ~= self.TextCursor then
			self.TextCursor = tc
			self.BlinkTick = 0
			self.BlinkState = 0
		end
	end
end

-------------------------------------------------------------------------------
--	select: internal
-------------------------------------------------------------------------------

function TextInput:select(onoff)
	if onoff and not self.Editing then
		self.Editing = true
		self.Window:addInputHandler(self, TextInput.handleInput)
		self.Window:addNotify("Interval", ui.NOTIFY_ALWAYS,
			self.IntervalNotify)
		self:clickMouse()
	elseif not onoff and self.Editing then
		self.Window:remNotify("Interval", ui.NOTIFY_ALWAYS,
			self.IntervalNotify)
		self.Window:remInputHandler(self, TextInput.handleInput)
		self.Editing = false
	end
end

-------------------------------------------------------------------------------
--	onSelect: overrides
-------------------------------------------------------------------------------

function TextInput:onSelect(selected)
	self:select(selected)
	Text.onSelect(self, selected)
end

-------------------------------------------------------------------------------
--	onFocus: overrides
-------------------------------------------------------------------------------

function TextInput:onFocus(focused)
	self.Window:setFocusElement(self)
	-- autoselect on focus:
	self:setValue("Selected", focused)
	self:select(focused)
	Text.onFocus(self, focused)
end

function TextInput:interval()
	self.BlinkTick = self.BlinkTick - 1
	if self.BlinkTick < 0 then
		self.BlinkTick = self.BlinkTickInit
		local bs = ((self.BlinkState == 1) and 0) or 1
		if bs ~= self.BlinkState then
			self.BlinkState = bs
			self.Redraw = true
		end
	end
end

local function crsrleft(self)
	self.TextCursor = self.TextCursor - 1
	if self.TextCursor < 0 then
		self.TextCursor = 0
		if self.TextOffset > 0 then
			self.TextOffset = self.TextOffset - 1
		else
			return false
		end
	end
	return true
end

local function crsrright(self)
	self.TextCursor = self.TextCursor + 1
	if self.TextCursor >= self.TextWidth then
		self.TextCursor = self.TextWidth - 1
		if self.TextOffset < self.TextBuffer:len() - self.TextWidth + 1 then
			self.TextOffset = self.TextOffset + 1
		else
			return false
		end
	end
	return true
end

-------------------------------------------------------------------------------
--	msg = passMsg(msg)
-------------------------------------------------------------------------------

function TextInput:passMsg(msg)
	if msg[2] == ui.MSG_MOUSEBUTTON then
		if msg[3] == 1 then -- leftdown:
			if self.Window.HoverElement == self and not self.Disabled then
				self:clickMouse(msg[4], msg[5])
			end
		end
	end
	return Text.passMsg(self, msg)
end

function TextInput.handleInput(self, msg)
	if msg[2] == ui.MSG_KEYDOWN and
		self == self.Window.FocusElement then
		local code = msg[3]

		local utf8code = msg[7]
		local t = self.TextBuffer
		local to = self.TextOffset
		local tc = self.TextCursor
		while true do
			if code == 61456 then
				crsrleft(self)
			elseif code == 61457 then
				crsrright(self)
			elseif code == 8 then -- backspace:
				if crsrleft(self) then
					t:erase(to + tc, to + tc)
				end
			elseif code == 127 then -- del:
				if t:len() > 0 then
					t:erase(to + tc + 1, to + tc + 1)
				end
			elseif code == 27 or code == 13 then -- escape, return:
				self.Window:setFocusElement()
				self:setValue("Selected", false)
				if code == 13 then -- return:
					self:setValue("Text", t:get())
					self:setValue("Enter", t:get())
					return false
				end
			elseif code == 0xf025 then -- pos1
				self.TextCursor = 0
				self.TextOffset = 0
			elseif code == 0xf026 then -- posend
				self.TextCursor = self.TextBuffer:len() + 1
			elseif code > 31 and code < 256 then
				t:insert(utf8code, to + tc + 1)
				crsrright(self)
			else
				break
			end
			-- something changed:
			self.BlinkTick = 0
			self.BlinkState = 0

			-- TODO: self:setValue("Text", t:get())

			return false
		end
	elseif msg[2] == ui.MSG_KEYUP and self.Editing then
		-- swallow this key event:
		return false
	end
	-- pass to next handler:
	return msg
end

-------------------------------------------------------------------------------
--	setState:
-------------------------------------------------------------------------------

function TextInput:setState(bg, fg)
	if not bg then
		if self.Disabled then
			bg = ui.PEN_BUTTONDISABLED
		elseif self.Selected then
			bg = ui.PEN_TEXTINPUTACTIVE
		elseif self.Hilite then
			bg = ui.PEN_TEXTINPUTOVER
		else
			bg = ui.PEN_TEXTINPUTBACK
		end
	end
	Text.setState(self, bg, fg)
end

-------------------------------------------------------------------------------
--	onSetText: overrides
-------------------------------------------------------------------------------

function TextInput:onSetText(text)
	-- intercept notification and do not pass the control back
	-- to Text, as it performs a rethinkLayout() on text changes
	self:makeTextRecords(text)
	self.TextBuffer = UTF8String:new(text)
	self.TextOffset = 0 -- TODO
	self.Redraw = true
end

-------------------------------------------------------------------------------
--	onEnter(text): This method is called when the {{Enter}} attribute is
--	set. It can be overridden for reacting on entering text by pressing
--	the return key. This method also sets the {{Text}} attribute (see
--	[[#tek.ui.class.text : Text]]).
-------------------------------------------------------------------------------

function TextInput:onEnter(text)
	self:setValue("Text", text)
end
