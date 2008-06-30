-------------------------------------------------------------------------------
--
--	tek.ui.class.family
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	LINEAGE::
--		[[#ClassOverview]] :
--		[[#tek.class : Class]] /
--		[[#tek.class.object : Object]] /
--		Family
--
--	OVERVIEW::
--		This class implements a container for child object.
--
--	ATTRIBUTES::
--		- {{Children [IG]}} (table)
--			Array of child objects
--
--	FUNCTIONS::
--		- Family:addMember()
--		- Family:remMember()
--
-------------------------------------------------------------------------------

local db = require "tek.lib.debug"
local Object = require "tek.class.object"
local insert = table.insert
local ipairs = ipairs
local remove = table.remove

module("tek.ui.class.family", tek.class.object)
_VERSION = "Family 2.3"

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

local Family = _M

function Family.init(self)
	self.Children = self.Children or { }
	return Object.init(self)
end

-------------------------------------------------------------------------------
--	success = addMember(child[, pos]): Invokes the specified child's
--	[[connect()][#Element:connect]] method to check for its ability to
--	integrate into the family, and if successful, inserts it into the
--	family's list of children. Optionally, the child is inserted into the
--	list at the specified position.
-------------------------------------------------------------------------------

function Family:addMember(child, pos)
	if child:connect(self) then
		if pos then
			insert(self.Children, pos, child)
		else
			insert(self.Children, child)
		end
		return true
	end
end

-------------------------------------------------------------------------------
--	success = remMember(child): Looks up {{child}} in the family's list of
--	children, and if it can be found, invokes its
--	[[disconnect()][#Element:disconnect]] method and removes it from the list.
-------------------------------------------------------------------------------

function Family:remMember(child)
	for pos, e in ipairs(self.Children) do
		if e == child then
			child:disconnect(self)
			remove(self.Children, pos)
			return true
		end
	end
end
