-------------------------------------------------------------------------------
--
--	tek.class.list
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	LINEAGE::
--		[[#ClassOverview]] :
--		[[#tek.class : Class]] /
--		List
--
--	OVERVIEW::
--		This class implements a list container.
--
--	IMPLEMENTS::
--		- List:addItem() - Adds an item to the list
--		- List:changeItem() - Replaces an item in the list
--		- List:checkPosition() - Verify position in the list
--		- List:getN() - Returns the number of items in the list
--		- List:getItem() - Returns the item at the specified position
--		- List:remItem() - Removes an item from the list
--
--	OVERRIDES::
--		- Class.new()
--
-------------------------------------------------------------------------------

local Class = require "tek.class"
local insert = table.insert
local max = math.max
local min = math.min
local remove = table.remove

module("tek.class.list", tek.class)
_VERSION = "List 1.3"
local List = _M

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

function List.new(class, self)
	self = self or { }
	self.Items = self.Items or { }
	return Class.new(class, self)
end

-------------------------------------------------------------------------------
--	List:getN(): Returns the number of elements in the list.
-------------------------------------------------------------------------------

function List:getN()
	return #self.Items
end

-------------------------------------------------------------------------------
--	List:getItem(pos): Returns the item at the specified position.
-------------------------------------------------------------------------------

function List:getItem(lnr)
	return self.Items[lnr]
end

-------------------------------------------------------------------------------
--	pos = List:addItem(entry[, pos]): Adds an item to the list, optionally
--	inserting it at the specified position. Returns the position at which the
--	item was added.
-------------------------------------------------------------------------------

function List:addItem(entry, lnr)
	local numl = self:getN() + 1
	if not lnr then
		insert(self.Items, entry)
		lnr = numl
	else
		lnr = min(max(1, lnr), numl)
		insert(self.Items, lnr, entry)
	end
	return lnr
end

-------------------------------------------------------------------------------
--	item = List:remItem(pos): Removes an item at the specified position
--	in the list. The item is returned to the caller.
-------------------------------------------------------------------------------

function List:remItem(lnr)
	if lnr > 0 and lnr <= self:getN() then
		return remove(self.Items, lnr)
	end
end

-------------------------------------------------------------------------------
--	success = List:changeItem(entry, pos): Changes the item at the specified
--	position in the list. Returns a boolean indicating whether there was an
--	item changed.
-------------------------------------------------------------------------------

function List:changeItem(entry, pos)
	local numl = self:getN()
	if lnr > 0 and lnr <= numl then
		self.Items[lnr] = entry
		return true
	end
end

-------------------------------------------------------------------------------
--	success[, position] = checkPosition(position[, null_valid]) -
--	Verifies that the given position is a valid index in the list; if the
--	optional {{null_valid}} is specified, {{0}} is considered a valid index.
--	If position is past (or before) the valid range, the last (respectively
--	the first) valid index is returned as well.
-------------------------------------------------------------------------------

function List:checkPosition(lnr, null_valid)
	if lnr == nil then
		return false
	elseif lnr == 0 then
		return null_valid or false, 0
	end
	local numl = self:getN()
	if lnr < 1 then
		return false, 1
	elseif lnr > numl then
		return false, numl
	end
	return true, lnr
end
