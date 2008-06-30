-------------------------------------------------------------------------------
--
--	tek.ui
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	OVERVIEW::
--		This is the base library of the user interface toolkit. It implements
--		a class loader and provides a central place for various toolkit
--		constants, which would otherwise be scattered over the class
--		hierarchy. To invoke the class loader, simply aquire a class from
--		the {{tek.ui}} table, e.g. this will load the
--		[[#tek.ui.class.application]] class as well as all subsequently needed
--		classes:
--				ui = require "tek.ui"
--				ui.Application:new { ...
--
--	CONSTANTS::
--		- {{NOTIFY_ALWAYS}} - see [[Object][#tek.class.object]]
--		- {{NOTIFY_CHANGE}} - see [[Object][#tek.class.object]]
--		- {{NOTIFY_VALUE}} - see [[Object][#tek.class.object]]
--		- {{NOTIFY_TOGGLE}} - see [[Object][#tek.class.object]]
--		- {{NOTIFY_FORMAT}} - see [[Object][#tek.class.object]]
--		- {{NOTIFY_SELF}} - see [[Object][#tek.class.object]]
--		- {{NOTIFY_OLDVALUE}} - see [[Object][#tek.class.object]]
--		- {{NOTIFY_FUNCTION}} - see [[Object][#tek.class.object]]
--		- {{NOTIFY_WINDOW}} - see [[Object][#tek.class.object]] -
--		defined in [[#tek.ui.class.element]]
--		- {{NOTIFY_APPLICATION}} - see [[Object][#tek.class.object]] -
--		defined in [[#tek.ui.class.element]]
--		- {{NOTIFY_ID}} - see [[Object][#tek.class.object]] -
--		defined in [[#tek.ui.class.element]]
--		- {{NOTIFY_COROUTINE}} - see [[Object][#tek.class.object]] -
--		defined in [[#tek.ui.class.element]]
--		- {{HUGE}} - use this value to express a "huge" spatial extent
--		- {{MSG_CLOSE}} - Input message type: Window closed
--		- {{MSG_FOCUS}} - Window activated/deactivated
--		- {{MSG_NEWSIZE}} - Window resized
--		- {{MSG_REFRESH}} - Window needs (partial) refresh
--		- {{MSG_MOUSEOVER}} - Mouse pointer entered/left window
--		- {{MSG_KEYDOWN}} - Key pressed down
--		- {{MSG_MOUSEMOVE}} - Mouse pointer moved
--		- {{MSG_MOUSEBUTTON}} - Mousebutton pressed/released
--		- {{MSG_INTERVAL}} - Timer interval message (default: 50Hz)
--		- {{MSG_KEYUP}} - Key released
--
-------------------------------------------------------------------------------

local db = require "tek.lib.debug"
local Object = require "tek.class.object"

local arg = arg
local error = error
local insert = table.insert
local package = package
local pcall = pcall
local require = require
local setmetatable = setmetatable

module "tek.ui"
_VERSION = "tekUI 6.2"

-- Old package path:
local OldPath = package and package.path or ""
local OldCPath = package and package.cpath or ""

-- Get executable path and name:
if arg and arg[0] then
	ProgDir, ProgName = arg[0]:match("^(.-/?)([^/]*)$")
end

-- Modified package path to find modules in the local program directory:
LocalPath = ProgDir and ProgDir .. "?.lua;" .. OldPath or OldPath
LocalCPath = ProgDir and ProgDir .. "?.so;" .. OldCPath or OldCPath

-------------------------------------------------------------------------------
--	class = loadClass(type, name[, pattern[, loader]]):
--	Loads a module with the given {{name}} from a path depending on the
--	class {{type}}, which is under control of the user interface library.
--	If {{pattern}} is specified, the supplied name will be checked against
--	it before an attempt is made to load the class. Possible values for the
--	{{type}} argument are:
--		- "ui" - tries to load a user interface element class
--		- "border" - tries to load a border class
--		- "layout" - tries to load a layouter
-------------------------------------------------------------------------------

local function loadProtected(name)
	return pcall(require, name)
end

local function loadSimple(name)
	return true, require(name)
end

local LoaderPaths =
{
	["ui"] = "tek.ui.class.",
	["border"] = "tek.ui.border.",
	["layout"] = "tek.ui.layout.",
}

function loadClass(class, name, pat, loader)
	if class and LoaderPaths[class] and name and
		not pat or name:match(pat) then
		name = LoaderPaths[class] .. name
		db.trace("Loading class '%s'...", name)
		package.path, package.cpath = LocalPath, LocalCPath
		local success, class = (loader or loadProtected)(name)
		package.path, package.cpath = OldPath, OldCPath
		if success then
			return class
		end
	end
end

-------------------------------------------------------------------------------
--	On-demand class loader:
-------------------------------------------------------------------------------

setmetatable(_M, {
	__index = function(tab, key)
		local pname = key:lower()
		local class = loadClass("ui", pname, "^%a+$", loadSimple)
		if class then
			db.info("Loaded class '%s'", pname)
			tab[pname] = class
			tab[key] = class
			return class
		else
			error("Failed to load class '" .. pname .. "'")
		end
	end
})

-------------------------------------------------------------------------------
--	Keycode aliases:
-------------------------------------------------------------------------------

KeyAliases =
{
	["IgnoreCase"] = { 0x0000, 0x00, 0x01, 0x02 },
	["Shift"] = { 0x0000, 0x01, 0x02 },
	["LShift"] = { 0x0000, 0x01 },
	["RShift"] = { 0x0000, 0x02 },
	["Ctrl"] = { 0x0000, 0x04, 0x08 },
	["LCtrl"] = { 0x0000, 0x04 },
	["RCtrl"] = { 0x0000, 0x08 },
	["Alt"] = { 0x0000, 0x10, 0x20 },
	["LAlt"] = { 0x0000, 0x10 },
	["RAlt"] = { 0x0000, 0x20 },
	["Del"] = { 0x007f },
	["F1"] = { 0xf001 },
	["F2"] = { 0xf002 },
	["F3"] = { 0xf003 },
	["F4"] = { 0xf004 },
	["F5"] = { 0xf005 },
	["F6"] = { 0xf006 },
	["F7"] = { 0xf007 },
	["F8"] = { 0xf008 },
	["F9"] = { 0xf009 },
	["F10"] = { 0xf00a },
	["F11"] = { 0xf00b },
	["F12"] = { 0xf00c },
	["Left"] = { 0xf010 },
	["Right"] = { 0xf011 },
	["Up"] = { 0xf012 },
	["Down"] = { 0xf013 },
	["BackSpc"] = { 0x0008 },
	["Tab"] = { 0x0009 },
	["Esc"] = { 0x001b },
	["Insert"] = { 0xf021 },
	["Overwrite"] = { 0xf022 },
	["PageUp"] = { 0xf023 },
	["PageDown"] = { 0xf024 },
	["Pos1"] = { 0xf025 },
	["End"] = { 0xf026 },
	["Print"] = { 0xf027 },
	["Scroll"] = { 0xf028 },
	["Pause"] = { 0xf029 },
}

-------------------------------------------------------------------------------
--	key, quals = resolveKeyCode: resolves a combined keycode specifier (e.g.
--	"Ctrl+Shift+Q" into a key string and a table of qualifier codes.
-------------------------------------------------------------------------------

local function addqual(key, quals, s)
	local a = KeyAliases[s]
	if a then
		if a[1] ~= 0 then
			key = a[1]
		end
		local n = #quals
		for i = 3, #a do
			for j = 1, n do
				insert(quals, quals[j] + a[i])
			end
		end
		if a[2] then
			for j = 1, n do
				quals[j] = quals[j] + a[2]
			end
		end
	else
		key = s
	end
	return key
end

function resolveKeyCode(code)
	local quals, key = { 0 }, ""
	local ignorecase
	for s in ("+" .. code):gmatch("%+(.[^+]*)") do
		if s == "IgnoreCase" then
			ignorecase = true
		end
		key = addqual(key, quals, s)
	end
	local lkey = key:lower()
	if not ignorecase and key == key:upper() and key ~= lkey then
		addqual(lkey, quals, "Shift")
	end
	return lkey, quals
end

-------------------------------------------------------------------------------
--	Constants: Note that 'Object' and 'Element' trigger the class loader
-------------------------------------------------------------------------------

DEBUG = false

HUGE = 1000000

NOTIFY_ALWAYS = Object.NOTIFY_ALWAYS
NOTIFY_CHANGE = Object.NOTIFY_CHANGE
NOTIFY_VALUE = Object.NOTIFY_VALUE
NOTIFY_TOGGLE = Object.NOTIFY_TOGGLE
NOTIFY_FORMAT = Object.NOTIFY_FORMAT
NOTIFY_SELF = Object.NOTIFY_SELF
NOTIFY_OLDVALUE = Object.NOTIFY_OLDVALUE
NOTIFY_FUNCTION = Object.NOTIFY_FUNCTION
NOTIFY_GETFIELD = Object.NOTIFY_GETFIELD
NOTIFY_WINDOW = Element.NOTIFY_WINDOW
NOTIFY_APPLICATION = Element.NOTIFY_APPLICATION
NOTIFY_ID = Element.NOTIFY_ID
NOTIFY_COROUTINE = Element.NOTIFY_COROUTINE

PEN_CURSOR = 1
PEN_CURSORTEXT = 2
PEN_AREABACK = 3
PEN_MENUBACK = 4
PEN_BUTTONTEXT = 5
PEN_BUTTONOVER = 6
PEN_BUTTONACTIVETEXT = 7
PEN_BUTTONACTIVE = 8
PEN_TEXTINPUTBACK = 9
PEN_TEXTINPUTTEXT = 10
PEN_TEXTINPUTOVER = 11
PEN_TEXTINPUTACTIVE = 12
PEN_LISTVIEWBACK = 13
PEN_LISTVIEWTEXT = 14
PEN_SHINE = 15
PEN_SHADOW = 16
PEN_HALFSHINE = 17
PEN_HALFSHADOW = 18
PEN_LISTVIEWACTIVE = 19
PEN_SLIDERBACK = 20
PEN_SLIDEROVER = 21
PEN_SLIDERACTIVE = 22
PEN_GROUPBACK = 23
PEN_GROUPLABELTEXT = 24
PEN_BUTTONDISABLED = 25
PEN_BUTTONDISABLEDSHADOW = 26
PEN_BUTTONDISABLEDSHINE = 27
PEN_BUTTONDISABLEDTEXT = 28
PEN_LISTVIEWACTIVETEXT = 29
PEN_FOCUSSHINE = 30
PEN_FOCUSSHADOW = 31
PEN_MENUACTIVE = 32
PEN_MENUACTIVETEXT = 33
PEN_LIGHTSHINE = 34
PEN_FILL = 35
PEN_BUTTONOVERDETAIL = 36
PEN_ALTLISTVIEWBACK = 37

MSG_CLOSE = 1
MSG_FOCUS = 2
MSG_NEWSIZE = 4
MSG_REFRESH = 8
MSG_MOUSEOVER	= 16
MSG_KEYDOWN = 256
MSG_MOUSEMOVE = 512
MSG_MOUSEBUTTON = 1024
MSG_INTERVAL = 2048
MSG_KEYUP = 4096
