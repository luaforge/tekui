-------------------------------------------------------------------------------
--
--	tek.class.utf8string
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	LINEAGE::
--		[[#ClassOverview]] :
--		[[#tek.class : Class]] /
--		UTF8String
--
--	OVERVIEW::
--		This class supports the manipulation of UTF-8 encoded strings.
--
--	IMPLEMENTS::
--		- UTF8String:byte() - Return Unicode character codes
--		- UTF8String.char() - Return UTF8-encoded character sequences
--		- UTF8String:erase() - Remove substring
--		- UTF8String:get() - Return the string in UTF-8-encoded form
--		- UTF8String:insert() - Insert a string at a position
--		- UTF8String:len() - Return the length in characters
--		- UTF8String:sub() - Return substring of a string
--
--	OVERRIDES::
--		- UTF8String.new() - create an UTF-8 string object from a string
--
-------------------------------------------------------------------------------

local Class = require "tek.class"
local concat = table.concat
local floor = math.floor
local tinsert = table.insert
local ipairs = ipairs
local max = math.max
local min = math.min
local remove = table.remove
local schar = string.char
local select = select
local tostring = tostring
local type = type

module("tek.class.utf8string", tek.class)
_VERSION = "UTF8String 1.1"

local UTF8String = _M

-------------------------------------------------------------------------------
--	utf8values: iterator over UTF-8 encoded Unicode chracter codes
-------------------------------------------------------------------------------

local function utf8values(readc, data)
	local accu = 0
	local numa = 0
	local min, buf
	local i = 0

	return function()
		local c
		while true do
			if buf then
				c = buf
				buf = nil
			else
				c = readc(data)
			end
			if not c then
				return
			end
			if c == 254 or c == 255 then
				break
			end
			if c < 128 then
				if numa > 0 then
					buf = c
					break
				end
				i = i + 1
				return i, c
			elseif c < 192 then
				if numa == 0 then break end
				accu = accu * 64 + c - 128
				numa = numa - 1
				if numa == 0 then
					if accu == 0 or accu < min or
						(accu >= 55296 and accu <= 57343) then
						break
					end
					c = accu
					accu = 0
					i = i + 1
					return i, c
				end
			else
				if numa > 0 then
					buf = c
					break
				end
				if c < 224 then
					min = 128
					accu = c - 192
					numa = 1
				elseif c < 240 then
					min = 2048
					accu = c - 224
					numa = 2
				elseif c < 248 then
					min = 65536
					accu = c - 240
					numa = 3
				elseif c < 252 then
					min = 2097152
					accu = c - 248
					numa = 4
				else
					min = 67108864
					accu = c - 252
					numa = 5
				end
			end
		end
		accu = 0
		numa = 0
		return 65533 -- bad character
	end
end

-------------------------------------------------------------------------------
--	encodeutf8
-------------------------------------------------------------------------------

local function encodeutf8(c)
	if c < 128 then
		return schar(c)
	elseif c < 0x800 then
		return schar(0xc0 + floor(c / 0x40)) ..
			schar(0x80 + c % 0x40)
	elseif c < 0x10000 then
		return schar(0xe0 + floor(c / 0x1000)) ..
			schar(0x80 + floor((c % 0x1000) / 0x40)) ..
			schar(0x80 + c % 0x40)
	elseif c < 0x200000 then
		return schar(0xf0 + floor(c / 0x40000)) ..
			schar(0x80 + floor((c % 0x40000) / 0x1000)) ..
			schar(0x80 + floor((c % 0x1000) / 0x40)) ..
			schar(0x80 + c % 0x40)
	elseif c < 0x4000000 then
		return schar(0xf8 + floor(c / 0x1000000)) ..
			schar(0x80 + floor((c % 0x1000000) / 0x40000)) ..
			schar(0x80 + floor((c % 0x40000) / 0x1000)) ..
			schar(0x80 + floor((c % 0x1000) / 0x40)) ..
			schar(0x80 + c % 0x40)
	else
		return schar(0xfc + floor(c / 0x40000000)) ..
			schar(0x80 + floor((c % 0x40000000) / 0x1000000)) ..
			schar(0x80 + floor((c % 0x1000000) / 0x40000)) ..
			schar(0x80 + floor((c % 0x40000) / 0x1000)) ..
			schar(0x80 + floor((c % 0x1000) / 0x40)) ..
			schar(0x80 + c % 0x40)
	end
end

local function readstring(data)
	data[4] = data[4] + 1
	return data[3]:byte(data[4])
end

-------------------------------------------------------------------------------
--	object = UTF8String.new(class, string): Create an UTF-8 string object
--	from an (already UTF-8 encoded) regular string.
-------------------------------------------------------------------------------

function UTF8String.new(class, s)
	local buf = { }
	local self = { s, buf }
	if s then
		self[3], self[4] = s, 0
		for i, c in utf8values(readstring, self) do
			buf[i] = c
		end
	end
	return Class.new(class, self)
end

-------------------------------------------------------------------------------
--	UTF8String:insert(string[, position]): Insert the UTF-8 encoded string
--	at the specified {{position}} to the string object. If {{position}} is
--	absent, the string is added to the end.
-------------------------------------------------------------------------------

function UTF8String:insert(s, pos)
	local buf = self[2]
	self[3], self[4] = s, 0
	if pos and pos > 0 then
		pos = pos - 1
		for i, c in utf8values(readstring, self) do
			tinsert(buf, pos + i, c)
		end
	else
		for _, c in utf8values(readstring, self) do
			tinsert(buf, c)
		end
	end
	self[1] = false
end

function __tostring(self)
	if not self[1] then
		local t = { }
		for i, c in ipairs(self[2]) do
			tinsert(t, char(c))
		end
		self[1] = concat(t)
	end
	return self[1]
end

-------------------------------------------------------------------------------
--	string = UTF8String:get(): Return the string in UTF-8 encoded form.
-------------------------------------------------------------------------------

get = __tostring

function __concat(self, s)
	if type(self) == "string" then
		return self .. tostring(s)
	end
	return __tostring(self) .. tostring(s)
end

-------------------------------------------------------------------------------
--	len = UTF8String:len(): Return the length of the string, in characters.
-------------------------------------------------------------------------------

function len(self)
	return #self[2]
end

local function getfirstlast(self, p0, p1)
	local len = #self[2]
	if len > 0 then
		if p0 < 0 then
			p0 = len + 1 + p0
		end
		p1 = p1 or -1
		if p1 < 0 then
			p1 = len + 1 + p1
		end
		p0 = max(1, p0)
		if p0 <= len then
			p1 = min(max(p0, p1), len)
			return p0, p1
		end
	end
end

-------------------------------------------------------------------------------
--	substring = UTF8String:sub(i[, j]): Returns an UTF-8 encoded substring.
--	The semantics are the same as for
--	[[string.sub][http://www.lua.org/manual/5.1/manual.html#pdf-string.sub]].
-------------------------------------------------------------------------------

function UTF8String:sub(p0, p1)
	local t = { }
	local p0, p1 = getfirstlast(self, p0, p1)
	if p0 then
		local tb = self[2]
		for i = p0, p1 do
			tinsert(t, char(tb[i]))
		end
	end
	return concat(t)
end

-------------------------------------------------------------------------------
--	UTF8String:erase(i[, j]): Erase the characters at the positions from {{i}}
--	to {{j}}. The semantics for the range are the same as for UTF8String:sub().
-------------------------------------------------------------------------------

function UTF8String:erase(p0, p1)
	local p0, p1 = getfirstlast(self, p0, p1)
	if p0 then
		local tb = self[2]
		for _ = p0, p1 do
			remove(tb, p0)
		end
		self[1] = false
	end
end

-------------------------------------------------------------------------------
--	UTF8String.char(...): Equivalent to
--	[[string.char][http://www.lua.org/manual/5.1/manual.html#pdf-string.char]],
--	except for that it accepts character codes in the Unicode range.
-------------------------------------------------------------------------------

function UTF8String.char(...)
	local t = { }
	for i = 1, select("#", ...) do
		tinsert(t, encodeutf8(select(i, ...)))
	end
	return concat(t)
end

-------------------------------------------------------------------------------
--	UTF8String:byte(i[, j]): Equivalent to
--	[[string.byte][http://www.lua.org/manual/5.1/manual.html#pdf-string.byte]],
--	except for that it may return characters in the Unicode range.
-------------------------------------------------------------------------------

function UTF8String:byte(p0, p1)
	local p0, p1 = getfirstlast(self, p0, p1)
	if p0 then
		local tb = self[2]
		local t = { }
		for i = p0, p1 do
			tinsert(t, tb[i])
		end
		return unpack(t)
	end
end
