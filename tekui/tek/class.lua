-------------------------------------------------------------------------------
--
--	tek.class
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	LINEAGE::
--		[[#ClassOverview]] :
--		Class
--
--	OVERVIEW::
--		This module implements inheritance and the creation of objects
--		from classes.
--
--	IMPLEMENTS::
--		- Class:checkDescend() - Checks if an object descends from a class
--		- Class:getClass() - Returns the class of an object, or the super
--		class of a class
--		- Class:getClassName() - Returns class name of an object or class
--		- Class:getSuper() - Returns the super class of an object or class
--		- Class.new() - Creates and returns a new object
--		- Class.newClass() - Creates a child class from a super class
--		- Class:setClass() - Changes the class of an object, or the super
--		class of a class
--
-------------------------------------------------------------------------------

local error = error
local tostring = tostring
local getmetatable = getmetatable
local setmetatable = setmetatable

-- use proxied object model:
local PROXY = true
-- in proxied mode, trace uninitialized variable accesses:
local DEBUG = true

module "tek.class"
_VERSION = "Class 6.1"

local Class = _M

Class.__index = _M

if PROXY then

	function Class.new(class, self)
		self = self or { }

		local mt = { __class = class }

		if DEBUG then
			function mt.__index(tab, key)
				local val = mt[key]
				if not val then
					error(("Uninitialized read: %s.%s"):format(
						tab:getClassName(), key))
				end
				return val
			end
			function mt.__newindex(tab, key, val)
				error(("Uninitialized write: %s.%s=%s"):format(
					tab:getClassName(), key,
					tostring(val)))
				mt[key] = val
			end
		else
			mt.__index = mt
			mt.__newindex = mt
		end

		setmetatable(mt, class)
		return setmetatable(self, mt)
	end

	function Class:getClass()
		mt = getmetatable(self)
		return mt.__class or mt
	end

	function Class:setClass(class)
		mt = getmetatable(self)
		mt.__class = class
		setmetatable(mt, class)
	end

	function Class:getSuper()
		return getmetatable(self.__class or self)
	end

else

-------------------------------------------------------------------------------
--	object = Class.new(class[, object]):
--	Creates and returns a new object of the given {{class}}. Optionally,
--	it just prepares the specified table {{object}} for inheritance and
--	attaches the class methods and data.
-------------------------------------------------------------------------------

	function Class.new(class, self)
		return setmetatable(self or { }, class)
	end

-------------------------------------------------------------------------------
--	class = object:getClass():
--	This function returns the class of the specified object. If applied to
--	a class instead of an object, it returns its super class, e.g.:
--			superclass = Class.getClass(class)
-------------------------------------------------------------------------------

	function Class:getClass()
		return getmetatable(self)
	end

-------------------------------------------------------------------------------
--	object:setClass(class): Changes the class of an object
--	or the super class of a class.
-------------------------------------------------------------------------------

	function Class:setClass(class)
		setmetatable(self, class)
	end

-------------------------------------------------------------------------------
--	object:getSuper(): Gets the super class of an object (or class).
-------------------------------------------------------------------------------

	function Class:getSuper()
		return getmetatable(self.__index or getmetatable(self))
	end

end

-------------------------------------------------------------------------------
--	class = Class.newClass(superclass[, class]):
--	Derives a new class from the specified {{superclass}}. Optionally,
--	an existing class table can be used. In this case, if a {{_NAME}}
--	attribute exists in the class table, it will be used. Otherwise, or if
--	a new class is being created, {{class._NAME}} will be composed from
--	{{superclass._NAME}} and an unique identifier. The same functionality
--	can be achieved by calling a class like a function, so these invocations
--	are equivalent:
--			class = Class.newClass(superclass)
--			class = superclass()
--	The second notation allows a super class to be passed as the second
--	argument to Lua's {{module}} function, which will set up a child class
--	inheriting from {{superclass}} in the module table.
-------------------------------------------------------------------------------

function Class.newClass(superclass, class)
	local class = class or { }
	class.__index = class
	class.__call = newClass
	class._NAME = class._NAME or superclass._NAME ..
		tostring(class):gsub("^table: 0x(.*)$", ".%1")
	return setmetatable(class, superclass)
end

-------------------------------------------------------------------------------
--	name = object:getClassName(): This function returns the {{_NAME}}
--	attribute of the specified class or object's class.
-------------------------------------------------------------------------------

function Class:getClassName()
	return self._NAME
end

-------------------------------------------------------------------------------
--	is_descendant = object:checkDescend(class):
--	Returns '''true''' if {{object}} is an instance of a class descending
--	from the specified {{class}}. It is also possible to apply this function
--	to a class instead of an object, e.g.:
--			Class.checkDescend(Button, Area)
-------------------------------------------------------------------------------

function Class:checkDescend(ancestor)
	while self ~= ancestor do
		self = getmetatable(self)
		if not self then
			return false
		end
	end
	return true
end

-------------------------------------------------------------------------------
--	make Class and its descendants functors:
-------------------------------------------------------------------------------

__call = Class.newClass
setmetatable(_M, { __call = newClass })
