-------------------------------------------------------------------------------
--
--	tek.ui.class.element
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	LINEAGE::
--		[[#ClassOverview]] :
--		[[#tek.class : Class]] /
--		[[#tek.class.object : Object]] /
--		Element
--
--	OVERVIEW::
--		This class implements the connection to a global environment and
--		the registration by Id.
--
--	ATTRIBUTES:
--		- {{Application [G]}} ([[#tek.ui.class.application : Application]])
--			The application the element is registered with, or '''false'''.
--			This attribute will be set when the Element:setup() method is
--			called.
--		- {{Children [IG]}} (table)
--			An array containing the object's children, or '''false'''.
--			This attribute is not handled by this Class, see
--			[[#tek.ui.class.group : Group]]
--		- {{Id [IG]}} (string)
--			An unique Id identifying this object, or '''false'''. If present,
--			this Id will be registered with the Application when the
--			Element:setup() method is called.
--		- {{Parent [G]}} (object)
--			Parent of this object, or '''false''', which will be set when
--			the Element:connect() method is called.
--		- {{Window [G]}} ([[#tek.ui.class.window : Window]])
--			The window the element is registered with, or '''false'''. This
--			attribute will be set when the Element:setup() method is called.
--
--	IMPLEMENTS::
--		- Element:connect() - Connects and element to a parent
--		- Element:disconnect() - Disconnects an element from a parent
--		- Element:setup() - Connects an element to its environment
--		- Element:cleanup() - Disconnects an element from its environment
--
--	OVERRIDES::
--		- Object.init()
--
-------------------------------------------------------------------------------

local Object = require "tek.class.object"
local assert = assert
local type = type
local insert = table.insert

module("tek.ui.class.element", tek.class.object)
_VERSION = "Element 8.0"
local Element = _M

-------------------------------------------------------------------------------
--	Placeholders for notification arguments:
-------------------------------------------------------------------------------

-- inserts the Window:
NOTIFY_WINDOW = function(a, n, i)
	insert(a, a[-1].Window)
	return 1
end

-- inserts the Application:
NOTIFY_APPLICATION = function(a, n, i)
	insert(a, a[-1].Application)
	return 1
end

-- inserts an object of the given Id:
NOTIFY_ID = function(a, n, i)
	insert(a, a[-1].Application:getElementById(n[i + 1]))
	return 2
end

-- denotes insertion of a function value as a new coroutine:
NOTIFY_COROUTINE = function(a, n, i)
	insert(a, function(...) a[-1].Application:addCoroutine(n[i + 1], ...) end)
	return 2
end

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

function Element.init(self)
	self.Application = false
	self.Children = self.Children or false
	self.Id = self.Id or false
	self.Parent = false
	self.Window = false
	return Object.init(self)
end

-------------------------------------------------------------------------------
--	success = Element:connect(parent): Attempts to connect an element to the
--	{{parent}} element; returns a boolean indicating whether the connection
--	succeeded.
-------------------------------------------------------------------------------

function Element:connect(parent)
	assert(parent)
	-- assert(not self.Parent)
	self.Parent = parent
	return true
end

-------------------------------------------------------------------------------
--	Element:disconnect(parent): Disconnects a formerly connected element
--	from its parent.
-------------------------------------------------------------------------------

function Element:disconnect(parent)
	assert(parent and self.Parent == parent)
	self.Parent = false
	return true
end

-------------------------------------------------------------------------------
--	Element:setup(app, window): This function connects an element
--	to the environment determined by an
--	[[#tek.ui.class.application : Application]] and
--	[[#tek.ui.class.window : Window]].
-------------------------------------------------------------------------------

function Element:setup(app, window)
	assert(not (self.Application or self.Window), "Element already connected")
	self.Application = app
	self.Window = window
	if self.Id then
		self.Application:addElement(self)
	end
end

-------------------------------------------------------------------------------
--	Element:cleanup(): This function disconnects an element from the
--	[[#tek.ui.class.application : Application]] and
--	[[#tek.ui.class.window : Window]] it is connected to.
-------------------------------------------------------------------------------

function Element:cleanup()
	assert(self.Application)
	if self.Id then
		self.Application:remElement(self)
	end
	self.Application = false
	self.Window = false
end
