-------------------------------------------------------------------------------
--
--	tek.ui.class.popupwindow
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	LINEAGE::
--		[[#ClassOverview]] :
--		[[#tek.class : Class]] /
--		[[#tek.class.object : Object]] /
--		[[#tek.ui.class.element : Element]] /
--		[[#tek.ui.class.area : Area]] /
--		[[#tek.ui.class.frame : Frame]] /
--		[[#tek.ui.class.gadget : Gadget]] /
--		[[#tek.ui.class.group : Group]] /
--		[[#tek.ui.class.group : Window]] /
--		PopupWindow
--
--	OVERVIEW::
--		This class specializes a Window for the use by a
--		[[#tek.ui.class.popitem : PopItem]].
--
--	OVERRIDES::
--		- Object.init()
--		- Area:show()
--		- Area:passMsg()
--
-------------------------------------------------------------------------------

local ui = require "tek.ui"
local Window = ui.Window
local ipairs = ipairs
local max = math.max

module("tek.ui.class.popupwindow", tek.ui.class.window)
_VERSION = "PopupWindow 1.2"

local PopupWindow = _M

-------------------------------------------------------------------------------
--	Constants and class data:
-------------------------------------------------------------------------------

local DEF_POPUPMARGIN = { 0, 0, 0, 0 }
local DEF_SHORTCUT_XOFFS = 20
local MSG_MOUSEOVER = ui.MSG_MOUSEOVER
local MSG_INTERVAL = ui.MSG_INTERVAL

-------------------------------------------------------------------------------
--	PopupWindow class:
-------------------------------------------------------------------------------

function PopupWindow.init(self)
	self.PopupBase = self.PopupBase or false
	self.BGPen = self.BGPen or ui.PEN_MENUBACK
	self.BeginPopupTicks = 0
	self.Border = self.Border or false
	self.BorderStyle = self.BorderStyle or "socket"
	self.DelayedBeginPopup = false
	self.DelayedEndPopup = false
	self.Margin = self.Margin or DEF_POPUPMARGIN
	self.MaxWidth = self.MaxWidth or 0
	self.MaxHeight = self.MaxHeight or 0
	self.Padding = self.Padding or false
	return Window.init(self)
end

function PopupWindow:show(display)
	if Window.show(self, display) then
		-- determine width of menuitems in group:
		local maxw = 0
		for _, e in ipairs(self.Children) do
			if e:checkDescend(ui.MenuItem) then
				maxw = max(maxw, e:getTextSize())
			end
		end
		for _, e in ipairs(self.Children) do
				-- align shortcut text (if present):
			if e:checkDescend(ui.MenuItem) and e.TextRecords[2] then
				e.TextRecords[2][5] = maxw + DEF_SHORTCUT_XOFFS
			end
		end
		return true
	end
end

function PopupWindow:passMsg(msg)
	if msg[2] == MSG_INTERVAL then
		self.BeginPopupTicks = self.BeginPopupTicks - 1
		if self.BeginPopupTicks < 0 then
			if self.DelayedBeginPopup then
				if not self.DelayedBeginPopup.PopupWindow then
					self.DelayedBeginPopup:beginPopup()
				end
				self.DelayedBeginPopup = false
			elseif self.DelayedEndPopup then
				if self.DelayedEndPopup.PopupWindow then
					self.DelayedEndPopup:endPopup()
				end
				self.DelayedEndPopup = false
			end
		end
	elseif msg[2] == MSG_MOUSEOVER then
		if msg[3] == 0 then
			if self.DelayedEndPopup then
				self:setHiliteElement(self.DelayedEndPopup)
				self.DelayedEndPopup = false
			end
		end
		-- do not pass control back to window msg handler:
		return false
	end
	return Window.passMsg(self, msg)
end
