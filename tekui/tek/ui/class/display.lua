-------------------------------------------------------------------------------
--
--	tek.ui.class.display
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	LINEAGE::
--		[[#ClassOverview]] :
--		[[#tek.class : Class]] /
--		[[#tek.class.object : Object]] /
--		Display
--
--	OVERVIEW::
--		This class manages a display device.
--
--	ATTRIBUTES::
--		- {{Theme [IG]}} ([[#tek.ui.class.them : Theme]])
--			Theme object; if none is supplied during initialization,
--			one is created.
--
--	IMPLEMENTS::
--		- Display:closeFont() - Close font
--		- Display:getFontAttrs() - Get font attributes
--		- Display:getTime() - Get system time
--		- Display:openFont() - Open a named font
--		- Display:openVisual() - Open a visual
--		- Display:sleep() - Sleep for a period of time
--		- Display:getTextSize() - Get size of text rendered with a given font
--		- Display:wait() - Wait for a list of visuals
--
--	OVERRIDES::
--		- Class.new()
--
-------------------------------------------------------------------------------

local db = require "tek.lib.debug"
local ui = require "tek.ui"
local Class = require "tek.class"
local Visual = require "tek.lib.visual"
local Theme = ui.Theme

local tonumber = tonumber

module("tek.ui.class.display", tek.class)
_VERSION = "Display 6.0"

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

local Display = _M

function Display.new(class, self)
	self.Theme = self.Theme or Theme:new { ImportConfig = true }
	self.FontCache = { }
	return Class.new(class, self)
end

-------------------------------------------------------------------------------
--	width, height = Display:getTextSize(font, text): Returns the width and
--	height of the specified {{text}} when it is rendered with the given
--	{{font}}.
-------------------------------------------------------------------------------

function Display:getTextSize(...)
	return Visual.textsize(...)
end

-------------------------------------------------------------------------------
--	font = Display:openFont(fontname): Opens the named font. For a discussion
--	of the fontname format, see [[#tek.ui.class.text : Text]].
-------------------------------------------------------------------------------

function Display:openFont(fname)
	fname = fname or ""
	if not self.FontCache[fname] then
		local deff = self.Theme.DefFonts
		local name, size = fname:match("^([^:]*):?(%d*)$")
		if deff[name] then
			local nname, nsize = deff[name]:match("^([^:]*):?(%d*)$")
			if size == "" then
				size = nsize
			end
			name = nname
		end
		size = size ~= "" and tonumber(size) or nil
		for name in name:gmatch("%s*([^,]*)%s*,?") do
			if name ~= "" then
				db.info("Fontname: %s -> %s:%d", fname, name, size or -1)
				local font = Visual.openfont(name, size)
				if font then
					local r = { font, font:getattrs { }, fname, name }
					self.FontCache[fname] = r
					self.FontCache[font] = r
					return font
				end
			end
		end
		return
	end
	return self.FontCache[fname][1]
end

-------------------------------------------------------------------------------
--	Display:closeFont(font): Closes the specified font
-------------------------------------------------------------------------------

function Display:closeFont(display, font)
end

-------------------------------------------------------------------------------
--	Display:getFontAttrs(font): Returns the font attributes height,
--	underline position and underline thickness.
-------------------------------------------------------------------------------

function Display:getFontAttrs(font)
	local a = self.FontCache[font][2]
	return a.Height, a.UlPosition, a.UlThickness
end

-------------------------------------------------------------------------------
--	wait:
-------------------------------------------------------------------------------

function Display:wait(...)
	return Visual.wait(...)
end

-------------------------------------------------------------------------------
--	sleep:
-------------------------------------------------------------------------------

function Display:sleep(...)
	return Visual.sleep(...)
end

-------------------------------------------------------------------------------
--	getTime:
-------------------------------------------------------------------------------

function Display:getTime(...)
	return Visual.gettime(...)
end

-------------------------------------------------------------------------------
--	openVisual:
-------------------------------------------------------------------------------

function Display:openVisual(...)
	return Visual.open(...)
end
