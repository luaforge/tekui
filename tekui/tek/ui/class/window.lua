-------------------------------------------------------------------------------
--
--	tek.ui.class.window
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
--		Window
--
--	OVERVIEW::
--		This class implements a window on the Display.
--
--	ATTRIBUTES::
--		- {{Center [IG]}} (boolean)
--			Instructs the Window to open centered on the Display.
--		- {{Fullscreen [IG]}} (boolean)
--			Instructs the Window to open borderless and in fullscreen mode;
--			this however may be in conflict with the {{MaxWidth}},
--			{{MinWidth}} attributes, which have precedence in this case.
--		- {{Modal [IG]}} (boolean)
--			Instructs all other windows to deny input while this window is
--			open.
--		- {{MouseX [SG]}}, {{MouseY [SG]}} (number)
--			The current screen coordinates of the pointing device.
--		- {{Status [ISG]}} (string)
--			Status of the Window, can be:
--				- "initializing" - The window has not yet been initialized.
--				- "hide" - The window is hidden; if you initialize the
--				attribute with this value, the Window will be created in
--				hidden state.
--				- "opening" - The window is about to open.
--				- "show" - The window is shown.
--				- "closing" - The Window is about to hide.
--			Changing this attribute invokes the Window:onChangeStatus()
--			method.
--		- {{Title [IG]}} (string)
--			The window's title.
--		- {{Left [IG]}} (number)
--			The window's left offset on the display
--		- {{Top [IG]}} (number)
--			The window's top offset on the display
--
--	IMPLEMENTS::
--		- Window:clickElement() - Emulate a click on an element
--		- Window:onChangeStatus() - Handler for {{Status}}
--		- Window:addInputHandler() - Adds an input handler
--		- Window:remInputHandler() - Removes an input handler
--		- Window:setActiveElement() - Sets window's active element
--		- Window:setDblClickElement() - Sets window's doubleclick element
--		- Window:setFocusElement() - Sets window's focused element
--		- Window:setHiliteElement() - Sets window's hovered element
--		- Window:setMovingElement() - Sets window's moving element
--
--	OVERRIDES::
--		- Area:getElement()
--		- Area:hide()
--		- Object.init()
--		- Area:layout()
--		- Area:rethinkLayout()
--		- Element:setup()
--		- Area:show()
--
-------------------------------------------------------------------------------

local db = require "tek.lib.debug"
local ui = require "tek.ui"

local Drawable = ui.Drawable
local Group = ui.Group
local Gadget = ui.Gadget

local assert = assert
local insert = table.insert
local ipairs = ipairs
local max = math.max
local min = math.min
local pairs = pairs
local remove = table.remove
local sort = table.sort
local type = type
local unpack = unpack

module("tek.ui.class.window", tek.ui.class.group)
_VERSION = "Window 6.2"

-------------------------------------------------------------------------------
--	Constants & Class data:
-------------------------------------------------------------------------------

local HUGE = ui.HUGE

local NOTIFY_STATUS = { ui.NOTIFY_SELF, "onChangeStatus", ui.NOTIFY_VALUE,
	ui.NOTIFY_OLDVALUE }
local DEF_MARGIN = { 2, 2, 2, 2 }

local DEF_DBLCLICKTIMELIMIT = 0.32 -- in seconds
local DEF_DBLCLICKDISTLIMIT = 25 -- pixel_distance^2 between clicks

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

local Window = _M

function Window.init(self)
	self.ActiveElement = false
	-- Item in this window in which an active popup is anchored:
	self.ActivePopup = false
	-- Root window in a cascade of popup windows:
	self.PopupRootWindow = self.PopupRootWindow or false
	self.CanvasStack = { }
	self.Center = self.Center or false
	self.CopyArea = { }
	self.DblClickCheckElement = false
	self.DblClickElement = false
	self.DblClickCheckTime = false
	self.DblClickCheckMouseX = false
	self.DblClickCheckMouseY = false
	self.FocusElement = false
	self.Fullscreen = self.Fullscreen or false
	self.HiliteElement = false
	-- Active hold tick counter - number of ticks left to next hold event:
	self.HoldTickActive = 0
	-- Hold tick counter reinitialization (see below):
	self.HoldTickActiveInit = 0
	-- Number of ticks for first hold tick counter initialization:
	self.HoldTickInitFirst = 22
	-- Number of ticks for hold counter repeat initialization:
	self.HoldTickInitRepeat = 7
	self.HoverElement = false
	self.InputHandlers = { }
	self.Interval = false
	self.KeyShortcuts = { }
	self.LayoutGroup = { }
	if self.BorderStyle then
		self.Margin = self.Margin or DEF_MARGIN
	end
	self.Modal = self.Modal or false
	self.MouseX = false
	self.MouseY = false
	self.MovingElement = false
	self.Status = self.Status or "initializing"
	self.Title = self.Title or false
	self.WindowFocus = false
	self.WindowMinMax = { }
	self.Left = self.Left or false
	self.Top = self.Top or false
	return Group.init(self)
end

-------------------------------------------------------------------------------
--	setup: overrides
-------------------------------------------------------------------------------

function Window:setup(app)
	-- pass ourselves as the window argument:
	Group.setup(self, app, self)
end

-------------------------------------------------------------------------------
--	show: overrides
-------------------------------------------------------------------------------

function Window:show(display)
	assert(not self.Drawable)
	self.Drawable = Drawable:new { Display = display }
	if Group.show(self, display, self.Drawable) then
		-- window main input handler:
		self:addInputHandler(self, Window.handleInput)
		-- notification handlers:
		self:addNotify("Status", ui.NOTIFY_CHANGE, NOTIFY_STATUS)
		return true
	end
end

-------------------------------------------------------------------------------
--	hide: overrides
-------------------------------------------------------------------------------

function Window:hide()
	self:remNotify("Status", ui.NOTIFY_CHANGE, NOTIFY_STATUS)
	self:remInputHandler(self, Window.handleInput)
	local d = self.Drawable
	assert(d)
	self:hideWindow()
	Group.hide(self)
	d:close()
end

-------------------------------------------------------------------------------
--	Window:addInputHandler(object, func): Adds an {{object}} and {{function}}
--	to the window's input chain. The input handlers is invoked as follows:
--			message = function(object, message)
--	The handler is expected to return the message, which will pass it on to
--	the next handler in the window's chain.
-------------------------------------------------------------------------------

function Window:addInputHandler(object, handler)
	insert(self.InputHandlers, 1, { object, handler })
end

-------------------------------------------------------------------------------
--	Window:remInputHandler(object, func): Removes an input handler that
--	was previously registered with the window using Window:addInputHandler().
-------------------------------------------------------------------------------

function Window:remInputHandler(object, handler)
	for i, h in ipairs(self.InputHandlers) do
		if h[1] == object and h[2] == handler then
			remove(self.InputHandlers, i)
			return
		end
	end
end

-------------------------------------------------------------------------------
--	showWindow:
-------------------------------------------------------------------------------

function Window:showWindow()
	self:setValue("Status", "opening")
	self.Application:openWindow(self)
	-- necessary to setup a freeregion:
	self:layout()
	self:setValue("Status", "show")
	self:addLayoutGroup(self, 2)
end

-------------------------------------------------------------------------------
--	hideWindow:
-------------------------------------------------------------------------------

function Window:hideWindow()
	self:setValue("Status", "closing")
	self.Application:closeWindow(self)
	self:setValue("Status", "hide")
	self.FreeRegion = false
end

-------------------------------------------------------------------------------
--	Window:onChangeStatus(status, oldstatus): This method is invoked when
--	the Window's {{Status}} has changed.
-------------------------------------------------------------------------------

function Window:onChangeStatus(showhide, oldvalue)
	if showhide == "show" then
		self.Status = oldvalue
		self:showWindow()
	elseif showhide == "hide" then
		self.Status = oldvalue
		self:hideWindow()
	end
end

-------------------------------------------------------------------------------
--	getWindowDimensions:
-------------------------------------------------------------------------------

function Window:getWindowDimensions()
	local mw, mh = self.MinWidth, self.MinHeight
	self.MinWidth, self.MinHeight = 0, 0
	local m1, m2, m3, m4 = self:askMinMax(0, 0, self.MaxWidth, self.MaxHeight)
	self.MinWidth = mw
	self.MinHeight = mh
	m1, m2 = max(mw, m1), max(mh, m2)
	local x, y, w, h = self.Left, self.Top, self.Width, self.Height
	w = type(w) == "number" and max(w, m1)
	h = type(h) == "number" and max(h, m2)
	-- if x and x >= 0 then
	-- 	h = h and m4 and min(h, m4) or m2
	-- end
	m3 = (m3 and m3 > 0 and m3 < HUGE) and m3 or nil
	m4 = (m4 and m4 > 0 and m4 < HUGE) and m4 or nil
	w = w or self.MaxWidth == 0 and m1 or w
	h = h or self.MaxHeight == 0 and m2 or h
	return m1, m2, m3, m4, x or nil, y or nil, w or nil, h or nil
end

-------------------------------------------------------------------------------
--	openWindow:
-------------------------------------------------------------------------------

function Window:openWindow()
	if self.Status ~= "show" then
		local m1, m2, m3, m4, x, y, w, h = self:getWindowDimensions()
		if self.Drawable:open(self.Title or self.Application.Title or nil,
			w, h, m1, m2, m3, m4, x, y, self.Center, self.Fullscreen) then
			local wm = self.WindowMinMax
			wm[1], wm[2], wm[3], wm[4] = m1, m2, m3, m4
			self.Status = "show"
		end
	end
	return self.Status
end

-------------------------------------------------------------------------------
--	closeWindow:
-------------------------------------------------------------------------------

function Window:closeWindow()
	local d = self.Drawable
	assert(d)
	if self.Status ~= "hide" then
		if d:close() then
			self.Width, self.Height = d.Width, d.Height
			self:setValue("Status", "hide")
		end
	end
	return self.Status
end

-------------------------------------------------------------------------------
--	rethinkLayout: overrides
-------------------------------------------------------------------------------

function Window:rethinkLayout()
	if self.Status == "show" then
		local wm = self.WindowMinMax
		local m1, m2, m3, m4 = self:getWindowDimensions()
		if m1 ~= wm[1] or m2 ~= wm[2] or m3 ~= wm[3] or m4 ~= wm[4] then
			wm[1], wm[2], wm[3], wm[4] = m1, m2, m3, m4
			db.trace("New window minmax: %d %d %d %d", m1, m2, m3, m4)
			local w, h = self.Drawable:getAttrs()
			if (m1 and w < m1) or (m2 and h < m2) then
				-- window needs to grow; mark new region as damaged:
				w = max(w, m1 or w)
				h = max(h, m2 or h)
				self:markDamage(0, 0, w - 1, h - 1)
			end
			self.Drawable:setAttrs { MinWidth = m1, MinHeight = m2,
				MaxWidth = m3, MaxHeight = m4 }
			return true
		end
	end
end

-------------------------------------------------------------------------------
--	getMsg:
-------------------------------------------------------------------------------

function Window:getMsg()
	if self.Status == "show" then
		return self.Drawable:getMsg()
	end
end

-------------------------------------------------------------------------------
--	passMsg: overrides
-------------------------------------------------------------------------------

function Window:passMsg(msg)
	for _, hnd in ipairs { unpack(self.InputHandlers) } do
		msg = hnd[2](hnd[1], msg)
		if not msg then
			return false
		end
	end
	return msg
end

-------------------------------------------------------------------------------
--	finishPopup:
-------------------------------------------------------------------------------

function Window:finishPopup()
	local w = self.PopupRootWindow or self
	if w.ActivePopup then
		w.ActivePopup:setValue("Selected", false)
	end
end

-------------------------------------------------------------------------------
--	handleInput:
-------------------------------------------------------------------------------

local MsgHandlers =
{
	[ui.MSG_CLOSE] = function(self, msg)
		self:setValue("Status", "hide")
		return msg
	end,
	[ui.MSG_FOCUS] = function(self, msg)
		self:setValue("WindowFocus", msg[3])
		self:setHiliteElement()
		self:setActiveElement()
		return msg
	end,
	[ui.MSG_NEWSIZE] = function(self, msg)
		self:addLayoutGroup(self)
		return msg
	end,
	[ui.MSG_REFRESH] = function(self, msg)
		self:markDamage(msg[7], msg[8], msg[9], msg[10])
		return msg
	end,
	[ui.MSG_MOUSEOVER] = function(self, msg)
		self:setHiliteElement()
		return msg
	end,
	[ui.MSG_INTERVAL] = function(self, msg)
		local ae = self.ActiveElement
		if ae and ae.Active then
			assert(self.HoldTickActive > 0)
			self.HoldTickActive = self.HoldTickActive - 1
			if self.HoldTickActive == 0 then
				self.HoldTickActiveInit = self.HoldTickInitRepeat
				self.HoldTickActive = self.HoldTickInitRepeat
				ae:setValue("Hold", true)
			end
		end
		self:setValue("Interval", true)
		return msg
	end,
	[ui.MSG_MOUSEBUTTON] = function(self, msg)
		if msg[3] == 1 then -- leftdown:
			-- send "Pressed" to window:
			self:setValue("Pressed", true)
		elseif msg[3] == 2 then -- leftup:
			local ae = self.ActiveElement
			local he = self.HoverElement
			-- release Hold state:
			if ae and ae.Active and ae.Hold and self.HoldTickActive > 0 then
				ae:setValue("Hold", false)
			end
			if not ae and he and he:checkFocus() then
				-- support releasing a button over a popup item that wasn't
				-- activated before, i.e. button was pressed all the time:
				he:setValue("Active", false, true)
			end
			self:setMovingElement()
			self:setActiveElement()
			-- release window:
			self:setValue("Pressed", false)
		end
		return msg
	end,
	[ui.MSG_KEYDOWN] = function(self, msg)
		-- pass message to active popup element:
		if self.ActivePopup then
			-- return
			db.info("propagating keydown to active popup...")
			msg = self.ActivePopup.PopupWindow:passMsg(msg)
			if not msg then
				return false
			end
		end
		local fe = self.FocusElement
		local key = msg[3]
		-- activate window:
		self:setValue("Active", true)
		if key == 27 then
			local pr = self.PopupRootWindow
			if pr then
				pr.ActivePopup:endPopup()
			else
				self:setValue("Status", "hide")
			end
			return
		elseif key == 9 then -- tab key:
			if msg[6] == 0 then -- no qualifier:
				self:setFocusElement(self:getNextElement(fe))
			elseif msg[6] == 1 then -- shift qualifier:
				self:setFocusElement(self:getNextElement(fe, true))
			end
			return
		elseif key == 61459 or key == 61457 then -- cursor down, right:
			self:setFocusElement(self:getNextElement(fe))
			return
		elseif key == 61458 or key == 61456 then -- cursor up, left:
			self:setFocusElement(self:getNextElement(fe, true))
			return
		elseif key == 13 or key == 32 then -- space or enter key:
			if fe and not fe.Active then
				self:setHiliteElement(fe)
				self:setActiveElement(fe)
			end
			return
		end
		-- serve elements with keyboard shortcuts:
		db.trace("keydown: %s - qual: %d", msg[7], msg[6])
		local s = self:getShortcutElements(msg[7], msg[6])
		if s then
			for _, e in ipairs(s) do
				self:setHiliteElement(e)
				self:setFocusElement(e)
				self:setActiveElement(e)
			end
		end
		return msg
	end,
	[ui.MSG_KEYUP] = function(self, msg)
		-- pass message to active popup element:
		if self.ActivePopup then
			-- return
			db.info("propagating keyup to active popup...")
			msg = self.ActivePopup.PopupWindow:passMsg(msg)
			if not msg then
				return false
			end
		end
		local key = msg[3]
		if key == 13 or key == 32 then
			self:setActiveElement()
			self:setHiliteElement(self.HoverElement)
		end
		-- serve elements with keyboard shortcuts:
		db.trace("keyup: %s - qual: %d", msg[7], msg[6])
		local s = self:getShortcutElements(msg[7], msg[6])
		if s then
			for _, e in ipairs(s) do
				self:setActiveElement()
			end
			-- self:setHiliteElement(self.HoverElement)
		end
		return msg
	end,
	[ui.MSG_MOUSEMOVE] = function(self, msg)
		self.HoverElement = self:getHoverElementByXY(msg[4], msg[5])
		return msg
	end,
}

function Window:handleInput(msg)
	self.MouseX, self.MouseY = msg[4], msg[5]
	msg = MsgHandlers[msg[2]](self, msg)
	if not msg then
		return false
	end
	return Group.passMsg(self, msg)
end

-------------------------------------------------------------------------------
--	getHoverElementByXY: returns the element hovered by the mouse pointer,
--	given that it descends from the Gadget class
-------------------------------------------------------------------------------

function Window:getHoverElementByXY(mx, my)
	local he = self:getElementByXY(mx, my)
	return he and he:checkDescend(Gadget) and he
end

-------------------------------------------------------------------------------
--	addLayoutGroup: mark a group for relayout
--	markdamage = false/nil: do not mark as damaged
--	markdamage = 1/true: mark as damaged only if coodinates changed
--	markdamage = 2: unconditionally mark as damaged
-------------------------------------------------------------------------------

function Window:addLayoutGroup(group, markdamage)
	if markdamage == true then
		markdamage = 1
	end
	if group:checkDescend(Group) then
		local record = self.LayoutGroup[group]
		if not record then
			record = { group }
			self.LayoutGroup[group] = record
			insert(self.LayoutGroup, record)
		end
		-- ugly:
		if markdamage and (not record[2] or markdamage > record[2]) then
			record[2] = markdamage
		end
	else
		db.warn("Attempt to relayout non-group: %s", group:getClassName())
	end
end

-------------------------------------------------------------------------------
--	update:
-------------------------------------------------------------------------------

local function sortcopies(a, b) return a[1] > b[1] end

function Window:handleCopies()
	local ca = self.CopyArea
	if ca then
		local t = { }
		for key, e in pairs(ca) do
			local dx, dy, region = e[1], e[2], e[3]
			local dir = (dx == 0 and dy or dx) > 0 and 1 or -1
			for _, r in region:getRects() do
				local r1, r2, r3, r4 = region:getRect(r)
				insert(t, { (dx == 0 and r2 or r1) * dir,
					dx, dy, r1, r2, r3, r4 })
			end
			ca[key] = nil
		end
		sort(t, sortcopies)
		local d = self.Drawable
		for _, r in ipairs(t) do
			local t = { }
			d:copyArea(r[4], r[5], r[6], r[7], r[4] + r[2], r[5] + r[3], t)
			for i = 1, #t, 4 do
				self:markDamage(t[i], t[i + 1], t[i + 2], t[i + 3])
			end
		end
	end
end

function Window:update()

	if self.Status == "show" then

		self:handleCopies()

		if #self.LayoutGroup > 0 then

			-- Handle partial relayouts. Note that new partial relayouts
			-- might be added while we process them

			while #self.LayoutGroup > 0 do
				-- list could be modified, need copy:
				local lg = { unpack(self.LayoutGroup) }
				self.LayoutGroup = { }
				while #lg > 0 do
					local record = remove(lg, 1)
					local group = record[1]
					local markdamage = record[2]
					group:calcWeights()
					local r = group.Rect
					local m = group.MarginAndBorder
					db.trace("relayout group %s - markdamage: %s",
						group:getClassName(), markdamage)

					local res, changed =
						self:relayout(group, r[1] - m[1], r[2] - m[2],
						r[3] + m[3], r[4] + m[4])

					if markdamage and (changed or markdamage == 2) then
						-- in a group, this redraws only the background:
						group.Redraw = true
					end

					if markdamage == 2 then -- mark always?
						group:markDamage(r[1], r[2], r[3], r[4])
					end

					lg[group] = nil
					self:handleCopies()
				end
			end

			-- mouse could point to a different element now:
			if self.HoverElement then
				self.HoverElement = self:getHoverElementByXY(self.MouseX,
					self.MouseY)
			end

		end

		-- refresh everything that is damaged:
		self:refresh()

	end

	return self.Status == "show"
end

-------------------------------------------------------------------------------
--	layout: overrides
-------------------------------------------------------------------------------

function Window:layout(_, _, _, _, markdamage)
	local changed
	self.FreeRegion = false
	local w, h = self.Drawable:getAttrs()
	return Group.layout(self, 0, 0, w - 1, h - 1, markdamage)
end

-------------------------------------------------------------------------------
--	Window:setHiliteElement(element[, notify]): Sets/unsets the element
--	which is being hovered by the mouse pointer. If {{notify}} is '''false''',
--	the value is set, but no notification is triggered.
-------------------------------------------------------------------------------

function Window:setHiliteElement(e, notify)
	local he = self.HiliteElement
	if e ~= he then
		if he then
			he:setValue("Hover", false, notify)
		end
		self.HiliteElement = e or false
		if e and not e.Disabled then
			e:setValue("Hover", true, notify)
		end
	end
end

-------------------------------------------------------------------------------
--	Window:setFocusElement(element[, notify]): Sets/unsets the element
--	which is marked for receiving input. If {{notify}} is '''false''', the
--	value is set, but no notification is triggered.
-------------------------------------------------------------------------------

function Window:setFocusElement(e, notify)
	local fe = self.FocusElement
	if e ~= fe then
		if fe then
			fe:setValue("Focus", false, notify)
		end
		self.FocusElement = e or false
		if e then
			e:setValue("Focus", true, notify)
		end
	end
end

-------------------------------------------------------------------------------
--	Window:setActiveElement(element[, notify]): Sets/unsets the element
--	which is activated (or 'in use'). If {{notify}} is '''false''', the
--	value is set, but no notification is triggered.
-------------------------------------------------------------------------------

function Window:setActiveElement(e, notify)
	local se = self.ActiveElement
	if e ~= se then
		if se then
			se:setValue("Active", false, notify)
		end
		self.ActiveElement = e or false
		if e then
			self.HoldTickActive = self.HoldTickInitFirst
			self.HoldTickActiveInit = self.HoldTickInitFirst
			e:setValue("Active", true, notify)
		end
	end
end

-------------------------------------------------------------------------------
--	Window:setDblClickElement(element[, notify]): Sets/unsets the element
--	which is candidate for doubleclick detection. If the element is set twice
--	in a sufficiently short period of time, and the pointing device did not
--	move too much since the first time, the doubleclick is triggered by
--	notifying the {{DblClick}} attribute in the element. See also
--	[[#tek.ui.class.gadget : Gadget]] for further information.
-------------------------------------------------------------------------------

function Window:setDblClickElement(e, notify)
	local de = self.DblClickElement
	if de then
		de:setValue("DblClick", false, notify)
		self.DblClickElement = false
	end
	if e and self.Display then
		local de = self.DblClickCheckElement
		local t = self.Display:getTime()
		if de == e then
			local d1 = self.MouseX - self.DblClickCheckMouseX
			local d2 = self.MouseY - self.DblClickCheckMouseY
			local d = d1 * d1 + d2 * d2
			if t - self.DblClickCheckTime < DEF_DBLCLICKTIMELIMIT and
				d < DEF_DBLCLICKDISTLIMIT then
				self.DblClickElement = e
				de:setValue("DblClick", true, notify)
			end
		end
		self.DblClickCheckTime = t
		self.DblClickCheckMouseX = self.MouseX
		self.DblClickCheckMouseY = self.MouseY
		self.DblClickCheckElement = e or false
	end
end

-------------------------------------------------------------------------------
--	Window:setMovingElement(element): Sets/unsets the element which is
--	being moved around by the user.
-------------------------------------------------------------------------------

function Window:setMovingElement(e)
	if e ~= self.MovingElement then
		self.MovingElement = e or false
	end
end

-------------------------------------------------------------------------------
--	getElement: overrides
-------------------------------------------------------------------------------

function Window:getElement(mode)
	if mode == "parent" or mode == "siblings" then
		return -- a window has no parent or siblings
	elseif mode == "prevorparent" then
		return self:getElement("lastchild")
	elseif mode == "nextorparent" then
		return self:getElement("firstchild")
	end
	return Group.getElement(self, mode)
end

-------------------------------------------------------------------------------
--	getNextElement: Cycles through elements, gets the next element that can
--	receive the focus.
-------------------------------------------------------------------------------

function Window:getNextElement(e, backward)
	local oe = e
	local ne
	repeat
		oe = oe or ne
		if oe then
			ne = e:getElement(backward and "lastchild" or "firstchild") or
				e:getElement(backward and "prevorparent" or "nextorparent")
		else
			ne = self:getElement(backward and "lastchild" or "firstchild")
		end
		if ne and ne:checkFocus() then
			return ne
		end
		e = ne
	until not e or e == oe
end

-------------------------------------------------------------------------------
--	addKeyShortcut:
-------------------------------------------------------------------------------

function Window:addKeyShortcut(keycode, element)

	local key, quals = ui.resolveKeyCode(keycode)
	db.info("binding shortcut: %s -> %s", key, element:getClassName())

	local keytab = self.KeyShortcuts[key]
	if not keytab then
		keytab = { }
		self.KeyShortcuts[key] = keytab
	end
	for _, qual in ipairs(quals) do
		local qualtab = keytab[qual]
		if not qualtab then
			qualtab = { }
			keytab[qual] = qualtab
		end
		db.trace("%s : adding qualifier %d", key, qual)
		insert(qualtab, element)
		qualtab[element] = #qualtab
	end
end

-------------------------------------------------------------------------------
--	remKeyShortcut:
-------------------------------------------------------------------------------

function Window:remKeyShortcut(keycode, element)
	local key, quals = ui.resolveKeyCode(keycode)
	db.info("removing shortcut: %s -> %s", key, element:getClassName())
	local keytab = self.KeyShortcuts[key]
	if keytab then
		for _, qual in ipairs(quals) do
			local qualtab = keytab[qual]
			if qualtab then
				local idx = qualtab[element]
				if idx then
					remove(qualtab, idx)
					qualtab[element] = nil
					db.trace("%s : removing qualifier %d", key, qual)
				end
			end
		end
	end
end

-------------------------------------------------------------------------------
--	getShortcutElements:
-------------------------------------------------------------------------------

function Window:getShortcutElements(key, qual)
	local keytab = self.KeyShortcuts[key:lower()]
	return keytab and keytab[qual]
end

-------------------------------------------------------------------------------
--	Window:clickElement(element): This function performs a "click" on the
--	specified {{element}}; if {{element}} is a string, it will be looked up
--	using Application:getElementById(). This function is actually a shorthand
--	for Window:setHiliteElement(), followed by Window:setActiveElement() twice
--	(once to enable, once to disable it).
-------------------------------------------------------------------------------

function Window:clickElement(e)
	if type(e) == "string" then
		e = self.Application:getElementById(e)
		assert(e, "Unknown Id")
	end
	local he = self.HiliteElement
	self:setHiliteElement(e)
	self:setActiveElement(e)
	self:setActiveElement()
	self:setHiliteElement(he)
end
