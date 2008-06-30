-------------------------------------------------------------------------------
--
--	tek.lib.debug
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	OVERVIEW::
--	Debug library - implements debug output and debug levels:
--
--	2  || TRACE || used for tracking bugs
--	4  || INFO  || informational messages
--	5  || WARN  || something unexpected happened
--	10 || ERROR || something went wrong, e.g. resource unavailable
--	20 || FAIL  || something went wrong that can't be coped with
--
--	The default debug level is 10 {{ERROR}}. To set the debug level
--	globally, e.g.:
--			db = require "tek.lib.debug"
--			db.level = db.INFO
--
--	The default debug output stream is {{stderr}}.
--	To override it globally, e.g.:
--			db = require "tek.lib.debug"
--			f = io.open("logfile", "w")
--			db.out = function(...) f:write(...) end
--
--	FUNCTIONS::
--		- debug.print() - Print a text in the specified debug level
--		- debug.execute() - Execute a function in the specified debug level
--		- debug.trace() - Print a text in the {{TRACE}} debug level
--		- debug.info() - Print a text in the {{INFO}} debug level
--		- debug.warn() - Print a text in the {{WARN}} debug level
--		- debug.error() - Print a text in the {{ERROR}} debug level
--		- debug.fail() - Print a text in the {{FAIL}} debug level
--		- debug.stacktrace() - Print a stacktrace in the specified debug level
--
-------------------------------------------------------------------------------

local debug = require "debug"
local getinfo = debug.getinfo
local traceback = debug.traceback
local stderr = require "io".stderr
local tostring = tostring
local tonumber = tonumber
local type = type
local unpack = unpack
local select = select
local time = os.time

module "tek.lib.debug"
_VERSION = "Debug 3.0"

-- symbolic:

TRACE = 2
INFO = 4
WARN = 5
ERROR = 10
FAIL = 20

-- global defaults:

level = ERROR
out = function(...) stderr:write(...) end

-------------------------------------------------------------------------------
--	print(lvl, msg, ...): Prints formatted text if the global debug level
--	is less or equal the specified level.
-------------------------------------------------------------------------------

function print(lvl, msg, ...)
	if level and lvl >= level then
		local t = getinfo(3, "lS")
		local arg = { }
		for i = 1, select('#', ...) do
			local v = select(i, ...)
			arg[i] = v and type(v) ~= "number" and tostring(v) or v or 0
		end
		out(("(%02d %d %s:%d) " .. msg):format(lvl,
			time(), t.short_src, t.currentline, unpack(arg)) .. "\n")
	end
end

-------------------------------------------------------------------------------
--	execute(lvl, func, ...): Executes the specified function if the global
--	debug library is less or equal the specified level.
-------------------------------------------------------------------------------

function execute(lvl, func, ...)
	if level and lvl >= level then
		return func(...)
	end
end

-------------------------------------------------------------------------------
--	trace(msg, ...): Prints formatted debug info with {{TRACE}} level
-------------------------------------------------------------------------------
function trace(msg, ...) print(2, msg, ...) end

-------------------------------------------------------------------------------
--	info(msg, ...): Prints formatted debug info with {{INFO}} level
-------------------------------------------------------------------------------
function info(msg, ...) print(4, msg, ...) end

-------------------------------------------------------------------------------
--	warn(msg, ...): Prints formatted debug info with {{WARN}} level
-------------------------------------------------------------------------------
function warn(msg, ...) print(5, msg, ...) end

-------------------------------------------------------------------------------
--	error(msg, ...): Prints formatted debug info with {{ERROR}} level
-------------------------------------------------------------------------------
function error(msg, ...) print(10, msg, ...) end

-------------------------------------------------------------------------------
--	fail(msg, ...): Prints formatted debug info with {{FAIL}} level
-------------------------------------------------------------------------------
function fail(msg, ...) print(20, msg, ...) end

-------------------------------------------------------------------------------
--	stacktrace(debuglevel, stacklevel): Prints a stacktrace starting at
--	the function of the given {{level}} on the stack (excluding the
--	{{stracktrace}} function itself) if the global debug level is less
--	or equal the specified {{debuglevel}}.
-------------------------------------------------------------------------------

function stacktrace(lvl, level)
	print(lvl, traceback("", level or 1 + 1))
end
