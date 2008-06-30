-------------------------------------------------------------------------------
--
--	tek.class.object
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	LINEAGE::
--		[[#ClassOverview]] :
--		[[#tek.class : Class]] /
--		Object
--
--	OVERVIEW::
--		This class implements notifications.
--
--	ATTRIBUTES:
--		- {{Notifications [I]}} (table)
--			Initial set of notifications. Static initialization of
--			notifications has this form:
--
--					Notifications =
--					{
--					  ["attribute-name1"] =
--					  {
--					    [value1] =
--					    {
--					      action1,
--					      action2,
--					      ...
--					    }
--					    [value2] = ...
--					  },
--					  ["attribute-name2"] = ...
--					}
--
--			Refer to Object:addNotify() for the possible placeholders for
--			{{value}} and a description of the {{action}} data structure.
--
--	IMPLEMENTS::
--		- Object:addNotify() - Adds a notification to an object
--		- Object.init() - (Re-)initialize an object
--		- Object:remNotify() - Removes a notification from an object
--		- Object:setMulti() - Sets multiple attributes, triggering
--		notifications
--		- Object:setValue() - Sets an attribute, triggering notifications
--		- Object:toggleValue() - Toggles an attribute, triggering
--		notifications
--
--	OVERRIDES::
--		- Class.new()
--
-------------------------------------------------------------------------------

local Class = require "tek.class"
local db = require "tek.lib.debug"

local assert = assert
local error = error
local insert = table.insert
local ipairs = ipairs
local remove = table.remove
local select = select
local type = type
local unpack = unpack

module("tek.class.object", tek.class)
_VERSION = "Object 8.0"
local Object = _M

-------------------------------------------------------------------------------
--	Placeholders:
-------------------------------------------------------------------------------

-- denotes that any value causes an object to be notified:
NOTIFY_ALWAYS = { }
-- denotes that any change of a value causes an object to be notified:
NOTIFY_CHANGE = { }
-- denotes insertion of the object itself:
NOTIFY_SELF = function(a, n, i) insert(a, a[-1]) return 1 end
-- denotes insertion of the value that triggered the notification:
NOTIFY_VALUE = function(a, n, i) insert(a, a[0]) return 1 end
-- denotes insertion of the value of the attribute prior to setting it:
NOTIFY_OLDVALUE = function(a, n, i) insert(a, a[-2]) return 1 end
-- denotes insertion of logical negation of the value:
NOTIFY_TOGGLE = function(a, n, i) insert(a, not a[0]) return 1 end
-- denotes insertion of the value, using the next argument as format string:
NOTIFY_FORMAT = function(a, n, i) insert(a, n[i+1]:format(a[0])) return 2 end
-- denotes insertion of a function value:
NOTIFY_FUNCTION = function(a, n, i) insert(a, n[i+1]) return 2 end
-- pops name and object, pushes the value of the named attribute:
NOTIFY_GETFIELD = function(a, n, i)
	local name, idx = remove(a), #a
	a[idx] = a[idx][name]
	return 1
end

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

function Object.new(class, self)
	return Class.new(class, class.init(self or { }))
end

-------------------------------------------------------------------------------
--	object = Object.init(object): This function is called during Object.new()
--	right before passing control to {{superclass.new()}}; by convention,
--	{{new()}} is used to claim resources (e.g. to create tables), whereas the
--	{{init()}} function is used to initialize them. Calling {{init()}}
--	separately from {{new()}} is of particular interest in classes such as
--	[[#tek.ui.class.popitem : PopItem]], which reinitialize their children
--	for reuse.
-------------------------------------------------------------------------------

function Object.init(self)
	self.Notifications = self.Notifications or { }
	return self
end

-------------------------------------------------------------------------------
--	Object:setValue(key, value[, notify]): Sets an {{object}}'s {{key}} to
--	the specified {{value}}, and, if {{notify}} is not '''false''', triggers
--	notifications that were previously registered with the object. If
--	{{value}} is '''nil''', the key's present value is set. To enforce
--	notifications that were supposed to react only on changes (registered
--	with the {{ui.NOTIFY_CHANGE}} placeholder), set {{notify}} to '''true'''.
--	For setting multiple keys, see Object:setMulti(). For details on
--	registering notifications, see Object:addNotify().
-------------------------------------------------------------------------------

local function doNotify(self, n, key, oldval)
	if n then
		if not n[0] then
			n[0] = true
			for _, n in ipairs(n) do
				local a = { [-2] = oldval, [-1] = self, [0] = self[key] }
				local i, v = 1
				while i <= #n do
					v = n[i]
					if type(v) == "function" then
						i = i + v(a, n, i)
					else
						insert(a, v)
						i = i + 1
					end
				end
				if a[1] then
					local func = remove(a, 2)
					if type(func) == "string" then
						func = a[1][func]
					end
					if func then
						func(unpack(a))
					end
				end
			end
			n[0] = false
		-- else
		-- 	db.warn("dropping cyclic notification")
		end
	end
end

function Object:setValue(key, val, notify)
	local oldval = self[key]
	if val == nil then
		val = oldval
	end
	local n = self.Notifications[key]
	if n and notify ~= false then
		if val ~= oldval or notify then
			self[key] = val
			doNotify(self, n[NOTIFY_CHANGE], key, oldval)
		end
		doNotify(self, n[val], key, oldval)
		doNotify(self, n[NOTIFY_ALWAYS], key, oldval)
	else
		self[key] = val
	end
end

-------------------------------------------------------------------------------
--	Object:toggleValue(key[, notify]): Logically toggles the value associated
--	with a key, and, if {{notify}} is not '''false''', triggers notifications
--	that were previously registered with the object. See also
--	Object:setValue().
-------------------------------------------------------------------------------

function Object:toggleValue(key, notify)
	self:setValue(key, not self[key], notify)
end

-------------------------------------------------------------------------------
--	Object:setMulti(key1, val1, key2, val2, ...):
--	Sets multiple keys in an object to the specified values, each of them
--	triggering possible notifications. See also Object:setValue() for details.
-------------------------------------------------------------------------------

function Object:setMulti(...)
	for i = 1, select('#', ...) - 1, 2 do
		self:setValue(select(i, ...), select(i + 1, ...))
	end
end

-------------------------------------------------------------------------------
--	Object:addNotify(key, val, dest[, pos]):
--	Adds a notification to an object. {{key}} is the name of an attribute to
--	react on setting its value. {{val}} indicates the value that triggers
--	the notification. Alternatively, the following placeholders for {{val}}
--	are supported:
--		* {{ui.NOTIFY_ALWAYS}} to react on any value
--		* {{ui.NOTIFY_CHANGE}} to react on any value, but only if it is
--		different from its prior value
--	{{dest}} is a table describing the action to take when the notification
--	occurs; it has the form
--			{ object, method, arg1, ... }
--	{{object}} indicates the target of the notification.
--	{{method}} can be either a string denoting the name of a function in the
--	addressed object, or a function itself. However, for passing a function
--	value in {{method}} or in the list of arguments, it must be preceded by
--	the {{ui.NOTIFY_FUNCTION}} placeholder.
--	The following placeholders are supported:
--		* {{ui.NOTIFY_VALUE}}, the value causing the notification
--		* {{ui.NOTIFY_TOGGLE}}, the logical negation of the value
--		* {{ui.NOTIFY_OLDVALUE}}, the attributes's value prior to setting it
--		* {{ui.NOTIFY_FORMAT}}, taking the next argument as a format string
--		to format the value
--		* {{ui.NOTIFY_FUNCTION}} to pass the next argument without inspection
--		for placeholders - this is needed for function values.
--	If the value is set in a child of the [[Element][#tek.ui.class.element]]
--	class, the following additional placeholders are supported:
--		* {{ui.NOTIFY_ID}} to address the [[Element][#tek.ui.class.element]]
--		with the Id given in the next argument
--		* {{ui.NOTIFY_WINDOW}} to address the [[Window][#tek.ui.class.window]]
--		the object is connected to
--		* {{ui.NOTIFY_APPLICATION}} to address the
--		[[Application][#tek.ui.class.application]] the object is connected to
--		* {{ui.NOTIFY_COROUTINE}}, to pass the next argument as a function,
--		which will be launched as a coroutine by the
--		[[Application][#tek.ui.class.application]]. See also
--		Application:addCoroutine() for further information.
--
--	In any case, the {{method}} will be invoked as follows:
--			method(object, arg1, ...)
--	The optional {{pos}} argument allows for insertion at an arbitrary
--	position in the notification list. By default, notifications are
--	added at the end, and the only reasonable value for {{pos}} would be
--	{{1}}.
--
--	If the destination object or addressed method cannot be determined,
--	nothing else besides setting the attribute will happen.
--	Notifications should be removed using Object:remNotify() when they are
--	no longer needed, to reduce overhead and memory use.
------------------------------------------------------------------------------

function Object:addNotify(attr, val, dest, pos)
	if dest then
		local n = self.Notifications
		n[attr] = n[attr] or { }
		n[attr][val] = n[attr][val] or { }
		if pos then
			insert(n[attr][val], pos, dest)
		else
			insert(n[attr][val], dest)
		end
	else
		error("No notify destination given")
	end
end

-------------------------------------------------------------------------------
--	success = Object:remNotify(key, val, dest):
--	Removes a notification from an object and returns '''true''' when it
--	was found and removed successfully. You must specify the exact set of
--	arguments as for Object:addNotify() to identify a notification.
-------------------------------------------------------------------------------

function Object:remNotify(attr, val, dest)
	local n = self.Notifications
	if n[attr] and n[attr][val] then
		for i, v in ipairs(n[attr][val]) do
			if v == dest then
				remove(n[attr][val], i)
				-- if #n[attr][val] == 0 then
				--	n[attr][val] = nil
				-- end
				return
			end
		end
	end
	-- error("notification not found")
	return false
end
