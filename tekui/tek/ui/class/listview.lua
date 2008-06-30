-------------------------------------------------------------------------------
--
--	tek.ui.class.listview
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
--		ListView
--
--	OVERVIEW::
--		This class implements a [[#tek.ui.class.group : Group]] containing
--		a [[#tek.ui.class.scrollgroup : ScrollGroup]] and optionally a
--		group of column headers; its main purpose is to automate the somewhat
--		complicated setup of multi-column lists with headers, but it can be
--		used for single-column lists and lists without column headers as well.
--
--	ATTRIBUTES::
--		- {{Headers [I]}} (table)
--			An array of strings containing the captions of column headers.
--			[Default: unspecified]
--		- {{HSliderMode [I]}} (string)
--			This attribute is passed on the
--			[[#tek.ui.class.scrollgroup : ScrollGroup]] - see there.
--		- {{VSliderMode [I]}} (string)
--			This attribute is passed on the
--			[[#tek.ui.class.scrollgroup : ScrollGroup]] - see there.
--
-------------------------------------------------------------------------------

local ui = require "tek.ui"
local List = require "tek.class.list"
local Canvas = ui.Canvas
local Gadget = ui.Gadget
local Group = ui.Group
local ListGadget = ui.ListGadget
local ScrollBar = ui.ScrollBar
local ScrollGroup = ui.ScrollGroup
local Text = ui.Text

local ipairs = ipairs

module("tek.ui.class.listview", tek.ui.class.group)
_VERSION = "ListView 4.2"

-------------------------------------------------------------------------------
--	HeadItem:
-------------------------------------------------------------------------------

local DEF_HEADITEM_BORDER = { 0, 0, 0, 0 }
local DEF_HEADITEM_IBORDER = { 1, 1, 1, 1 }
local DEF_HEADITEM_MARGIN = { 0, 0, 0, 0 }
local DEF_HEADITEM_PADDING = { 0, 0, 0, 0 }

local HeadItem = Text:newClass { _NAME = "_listviewhead" }

function HeadItem.init(self)
	self = self or { }
	self.TextHAlign = "left"
	self.Margin = DEF_HEADITEM_MARGIN
	self.BorderStyle = "none"
	self.IBorderStyle = "button"
	self.Mode = "inert"
	self.IBorder = DEF_HEADITEM_IBORDER
	self.Border = DEF_HEADITEM_BORDER
	self.Width = "auto"
	self.Padding = DEF_HEADITEM_PADDING
	return Text.init(self)
end

function HeadItem:setState(bg)
	Text.setState(self, bg or ui.PEN_GROUPBACK)
end

function HeadItem:askMinMax(m1, m2, m3, m4)
	local w, h = self:getTextSize()
	m1 = m1 + w
	m2 = m2 + h
	m3 = m3 + w
	m4 = m4 + h
	return Gadget.askMinMax(self, m1, m2, m3, m4)
end

-------------------------------------------------------------------------------
--	ListView:
-------------------------------------------------------------------------------

local ListView = _M

function ListView.new(class, self)
	self = self or { }

	self.Child = self.Child or ListGadget:new()
	self.HeaderGroup = self.HeaderGroup or false
	self.Headers = self.Headers or false
	self.HSliderMode = self.HSliderMode or "on"
	self.VSliderGroup = false
	self.VSliderMode = self.VSliderMode or "on"

	if self.Headers and not self.HeaderGroup then
		local c = { }
		for i, caption in ipairs(self.Headers) do
			c[i] = HeadItem:new { Text = caption }
		end
		self.HeaderGroup = Group:new { Width = "fill", Children = c }
	end

	if self.HeaderGroup then
		self.VSliderGroup = ScrollBar:new { Orientation = "vertical", Min = 0 }
		self.Child.HeaderGroup = self.HeaderGroup
		self.Children =
		{
			ScrollGroup:new
			{
				VSliderMode = "off",
				HSliderMode = self.HSliderMode,
				KeepMinHeight = true,
				Canvas = Canvas:new
				{
					passMsg = function(self, msg)
						-- pass input unmodified:
						return self.Child:passMsg(msg)
					end,
					AutoHeight = true,
					AutoWidth = true,
					Child = Group:new
					{
						Orientation = "vertical",
						Children =
						{
							self.HeaderGroup,
							ScrollGroup:new
							{
								VSliderMode = "off",
								HSliderMode = "off",
								VSliderGroup = self.VSliderGroup,
								Canvas = Canvas:new
								{
									Child = self.Child
								}
							}
						}
					}
				}
			},
			self.VSliderGroup
		}
		-- point element that determines listgadget alignment to outer canvas:
		self.Child.AlignElement = self.Children[1].Canvas
	else
		self.Children =
		{
			ScrollGroup:new
			{
				VSliderMode = self.VSliderMode,
				HSliderMode = self.HSliderMode,
				Canvas = Canvas:new
				{
					Child = self.Child
				}
			}
		}
	end

	return Group.new(class, self)
end
