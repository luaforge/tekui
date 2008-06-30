-------------------------------------------------------------------------------
--
--	tek.ui.class.application
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	LINEAGE::
--		[[#ClassOverview]] :
--		[[#tek.class : Class]] /
--		[[#tek.ui.class.family : Family]] /
--		Application
--
--	OVERVIEW::
--		This class implements the framework's entrypoint and main loop.
--
--	MEMBERS::
--		- {{Author [IG]}}
--			Name of the application's author
--		- {{Copyright [IG]}}
--			Copyright notice applying to the application
--		- {{ProgramName [IG]}}
--			Name of the application
--		- {{Status [G]}}
--			Status of the application, can be: "connected", "connecting",
--			"disconnected", "disconnecting", "initializing", "error",
--			"running"
--		- {{Theme [IG]}}
--			Optional custom theme object; if unspecified, a default
--			theme will be created
--		- {{Title [IG]}}
--			Title of the application, which will be inherited by windows;
--			if unspecified, {{ProgramName}} will be used
--
--	IMPLEMENTS::
--		- Application:addCoroutine() - adds a coroutine to the application
--		- Application:connect() - connects children recursively
--		- Application:getElementById() - returns an element by Id
--		- Application:run() - runs the application
--		- Application:suspend() - suspends the caller's coroutine
--		- Application:requestFile() - opens a file requester
--
--	OVERRIDES::
--		- Family:addMember()
--		- Object.init()
--		- Class.new()
--		- Family:remMember()
--
-------------------------------------------------------------------------------

local db = require "tek.lib.debug"
local ui = require "tek.ui"

local Display = ui.Display
local Family = ui.Family
local Group = ui.Group
local Window = ui.Window

local assert = assert
local cocreate = coroutine.create
local coresume = coroutine.resume
local corunning = coroutine.running
local costatus = coroutine.status
local coyield = coroutine.yield
local insert = table.insert
local ipairs = ipairs
local max = math.max
local min = math.min
local remove = table.remove
local traceback = debug.traceback
local unpack = unpack

module("tek.ui.class.application", tek.ui.class.family)
_VERSION = "Application 2.6"

-------------------------------------------------------------------------------
--	class implementation:
-------------------------------------------------------------------------------

local Application = _M

function Application.new(class, self)
	self = Family.new(class, self)
	-- Check linkage of members and connect them recursively:
	if self:connect() then
		self.Status = "disconnected"
		self:setup()
		self.Display = self.Display or Display:new { Theme = self.Theme }
		self:show(self.Display)
	else
		db.error("Could not connect elements")
		self.Status = "error"
	end
	return self
end

function Application.init(self)
	self.Author = self.Author or false
	self.Copyright = self.Copyright or false
	self.Coroutines = { }
	self.Display = false
	self.ElementById = { }
	self.ModalWindow = false
	self.MsgActive = { }
	self.OpenWindows = { }
	self.ProgramName = self.ProgramName or self.Title or false
	self.Status = "initializing"
	self.Theme = self.Theme or false
	self.Title = self.Title or self.ProgramName or false
	self.WaitVisuals = { }
	self.WaitWindows = { }
	return Family.init(self)
end

-------------------------------------------------------------------------------
--	Application:connect(parent): Checks member linkage and connects all
--	children by invoking their [[connect()][#Element:connect]]
--	methods. Note that unlike Element:connect(), this function is recursive.
-------------------------------------------------------------------------------

function Application:connect(parent)
	if self.Children then -- TODO: must always have children
		for _, child in ipairs(self.Children) do
			if not self:checkMember(child) then
				db.error("Connection failed: %s <- %s", self:getClassName(),
					child:getClassName())
				return false
			end
			if child:checkDescend(Group) then
				-- descend into group:
				if not connect(child, self) then
					return false
				end
			else
				if not child:connect(self, parent) then
					db.error("Connection failed: %s <- %s",
						self:getClassName(), child:getClassName())
					return false
				end
			end
		end
		if parent then
			return self:connect(parent)
		end
		return true
	end
end

-------------------------------------------------------------------------------
--	checkMember: check if an element fits into this group as a child member
-------------------------------------------------------------------------------

function Application:checkMember(child)
	-- only elements descending from Window can be children of an application:
	return child:checkDescend(Window)
end

-------------------------------------------------------------------------------
--	addMember: overrides
-------------------------------------------------------------------------------

function Application:addMember(child, pos)
	if self:checkMember(child) then
		assert(child:checkDescend(Window), "Child must be a window")
		child:setup(self, child)
		if child:show(self.Display) then
			-- this will also invoke checkMember():
			if Family.addMember(self, child, pos) then
				return child
			end
			child:hide()
		end
		child:cleanup()
	end
end

-------------------------------------------------------------------------------
--	remMember: overrides
-------------------------------------------------------------------------------

function Application:remMember(child)
	assert(child.Parent == self)
	Family.remMember(self, child)
	child:hide()
	child:cleanup()
end

-------------------------------------------------------------------------------
-- 	addElement:
-------------------------------------------------------------------------------

function Application:addElement(e)
	assert(not self.ElementById[e.Id], ("Id '%s' already exists"):format(e.Id))
	self.ElementById[e.Id] = e
end

-------------------------------------------------------------------------------
-- 	remElement:
-------------------------------------------------------------------------------

function Application:remElement(e)
	assert(self.ElementById[e.Id])
	self.ElementById[e.Id] = nil
end

-------------------------------------------------------------------------------
-- 	element = Application:getElementById(): Returns the element that was
--	registered with the Application under its unique {{Id}}. Returns
--	'''nil''' if the Id was not found.
-------------------------------------------------------------------------------

function Application:getElementById(Id)
	return self.ElementById[Id]
end

-------------------------------------------------------------------------------
--	setup: internal
-------------------------------------------------------------------------------

function Application:setup()
	db.trace("setup")
	if self.Status == "disconnected" then
		self.Status = "connecting"
		for _, child in ipairs(self.Children) do
			child:setup(self, child)
		end
		self.Status = "connected"
	end
end

-------------------------------------------------------------------------------
--	cleanup: internal
-------------------------------------------------------------------------------

function Application:cleanup()
	db.trace("cleanup")
	assert(self.Status == "connected")
	self.Status = "disconnecting"
	for _, child in ipairs(self.Children) do
		child:cleanup()
	end
	self.Status = "disconnected"
end

-------------------------------------------------------------------------------
--	show: internal
-------------------------------------------------------------------------------

function Application:show(display)
	db.trace("setup")
	self.Display = display
	for _, w in ipairs(self.Children) do
		w:show(display)
	end
	return true
end

-------------------------------------------------------------------------------
--	hide: internal
-------------------------------------------------------------------------------

function Application:hide()
	db.trace("cleanup")
	for _, w in ipairs(self.Children) do
		w:hide()
	end
	self.Display = nil
end

-------------------------------------------------------------------------------
--	openWindow: internal
-------------------------------------------------------------------------------

function Application:openWindow(window)
	local status = window.Status
	if status ~= "show" then
		status = window:openWindow()
		if status == "show" then
			if window.Modal then
				assert(not self.ModalWindow, "More than one modal window")
				self.ModalWindow = window
			end
			insert(self.OpenWindows, window)
		end
	end
	return status
end

-------------------------------------------------------------------------------
--	closeWindow: internal
-------------------------------------------------------------------------------

function Application:closeWindow(window)
	local status = window.Status
	if status ~= "hide" then
		status = window:closeWindow()
		if window == self.ModalWindow then
			self.ModalWindow = false
		end
		-- NOTE: windows are purged from OpenWindows list during wait()
	end
	return status
end

-------------------------------------------------------------------------------
--	showWindow: make a window visible.
--	if no window is specified, show all windows that aren't of Status "hide"
-------------------------------------------------------------------------------

function Application:showWindow(window)
	if window then
		window:showWindow()
	else
		for _, window in ipairs(self.Children) do
			if window.Status ~= "hide" then
				window:showWindow()
			end
		end
	end
end

-------------------------------------------------------------------------------
--	hideWindow: hide a window. if no window is specified, hide all windows.
-------------------------------------------------------------------------------

function Application:hideWindow(window)
	if window then
		return window:hideWindow()
	else
		for _, window in ipairs(self.Children) do
			window:hideWindow()
		end
	end
end

-------------------------------------------------------------------------------
-- 	wait:
-------------------------------------------------------------------------------

function Application:wait()

	local ow = self.OpenWindows
	local vt = self.WaitVisuals
	local ct = self.WaitWindows
	local numv = 0

	-- update windows:
	for _, w in ipairs(ow) do
		if w.Status == "show" and w:update() then
			numv = numv + 1
			vt[numv] = w.Drawable.Visual
			ct[numv] = w
		end
	end

	-- purge hidden windows from list:
	for i = #ow, 1, -1 do
		if ow[i].Status ~= "show" then
			remove(ow, i)
		end
	end

	-- wait for open windows:
	self.Display:wait(vt, numv)

	-- service windows that have messages pending:
	for i = 1, numv do
		if vt[i] then
			insert(self.MsgActive, ct[i])
		end
	end

end

-------------------------------------------------------------------------------
-- 	success, status = Application:run(): Runs the application. Returns when
--	all child windows are closed or when the application's {{Status}} is set
--	to "quit".
-------------------------------------------------------------------------------

function Application:run()

	assert(self.Status == "connected", "Application not in connected state")

	-- open all windows that aren't in "hide" state:
	self:showWindow()

	self.Status = "running"

	while self.Status == "running" and #self.OpenWindows > 0 do
		self:serviceCoroutines()
		self:wait()
		while #self.MsgActive > 0 do
			local window = remove(self.MsgActive, 1)
			local msg = window:getMsg()
			if msg then
				local refresh, newsize
				repeat
					local mt = msg[2]
					if mt == 8 then
						-- while we have messages pending, bundle damagerects:
						if not refresh then
							refresh = { unpack(msg) }
						else
							refresh[1] = msg[1] -- update timestamp
							refresh[7] = min(refresh[7], msg[7])
							refresh[8] = min(refresh[8], msg[8])
							refresh[9] = max(refresh[9], msg[9])
							refresh[10] = max(refresh[10], msg[10])
						end
					elseif mt == 4 then
						-- while we have messages pending, bundle newsizes:
						if not newsize then
							newsize = { unpack(msg) }
						else
							newsize[1] = msg[1] -- update timestamp
						end
					else
						-- pass other messages to the respective handler:
						local mw = self.ModalWindow
						if not mw or mw == window or
							mt == 2 or mt == 4 or mt == 8 or mt == 2048 then
							window:passMsg(msg)
						end
					end
					msg = window:getMsg()
				until not msg
				if newsize then
					window:passMsg(newsize)
				end
				if refresh then
					window:passMsg(refresh)
				end
			end
		end
	end

	-- hide all windows:
	self:hideWindow()

	-- self:hide()
	-- self:cleanup()

	return true, self.Status
end

-------------------------------------------------------------------------------
--	Application:addCoroutine(function, arg1, ...): Adds a new coroutine to
--	the application's list of serviced coroutines. The new coroutine is not
--	immediately started, but at a later time during the application's
--	update procedure. This gives the application an opportunity to service
--	all pending messages and updates before the coroutine is actually started.
-------------------------------------------------------------------------------

function Application:addCoroutine(func, ...)
	insert(self.Coroutines, cocreate(function()
		func(unpack(arg))
	end))
end

-------------------------------------------------------------------------------
--	serviceCoroutine: internal
-------------------------------------------------------------------------------

function Application:serviceCoroutines()
	local crt = self.Coroutines
	local c = remove(crt, 1)
	if c then
		local success, errmsg = coresume(c)
		local s = costatus(c)
		if s == "suspended" then
			insert(crt, c)
		else
			if success then
				db.info("Coroutine finished successfully")
			else
				db.error("Error in coroutine:\n%s\n%s", errmsg, traceback(c))
			end
		end
	end
end

-------------------------------------------------------------------------------
--	Application:suspend(): Suspends the caller (which must be running in a
--	coroutine previously registered using Application:addCoroutine()) until
--	it is getting rescheduled by the application. This gives the application
--	an opportunity to service all pending messages and updates before the
--	coroutine is continued.
-------------------------------------------------------------------------------

function Application:suspend()
	coyield()
end

-------------------------------------------------------------------------------
--	status[, path, selection] = Application:requestFile(args):
--	Requests a single or multiple files or directories. Possible keys in
--	the {{args}} table are:
--		- {{Path}} - The initial path
--		- {{Title}} - Window title [default "Select file or directory..."]
--		- {{SelectMode}} - "multi" or "single" [default "single"]
--	The first return value is a string reading either "selected" or
--	"cancelled". If the status is "selected", the second return value is
--	the path where the requester was left, and the third value is a table
--	of the items that were selected.
--	Note: The caller of this function must be running in a coroutine
--	(see Application:addCoroutine()).
-------------------------------------------------------------------------------

function Application:requestFile(args)

	assert(corunning(), "Must be called in a coroutine")

	args = args or { }

	local dirlist = ui.DirList:new
	{
		Path = args.Path or "/",
		Style = "requester",
		SelectMode = args.SelectMode or "single",
	}

	local window = Window:new
	{
		Title = args.Title or "Select file or directory...",
		Modal = true,
		Width = 400,
		Height = 500,
		Center = true,
		Children = { dirlist }
	}

	Application.connect(window)
	self:addMember(window)
	window:setValue("Status", "show")

	dirlist:showDirectory()

	repeat
		Application.suspend()
		if window.Status ~= "show" then
			-- window closed:
			dirlist.Status = "cancelled"
		end
	until dirlist.Status ~= "running"

	dirlist:abortScan()

	self:remMember(window)

	if dirlist.Status == "selected" then
		return dirlist.Status, dirlist.Path, dirlist.Selection
	end

	return dirlist.Status

end
