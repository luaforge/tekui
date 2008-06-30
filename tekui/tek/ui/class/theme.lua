
--
--	tek.ui.class.theme
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--

local db = require "tek.lib.debug"
local ui = require "tek.ui"

local Object = require "tek.class.object"

local getenv = os.getenv
local insert = table.insert
local ipairs = ipairs
local min = math.min
local open = io.open
local tonumber = tonumber

module("tek.ui.class.theme", tek.class.object)
_VERSION = "Theme 4.3"

-------------------------------------------------------------------------------
--	Class data and constants:
-------------------------------------------------------------------------------

local DEF_MAINFONT = "sans-serif,helvetica,Vera:12"
local DEF_SMALLFONT = "sans-serif,helvetica,Vera:10"
local DEF_MENUFONT = "sans-serif,helvetica,Vera:14"
local DEF_FIXEDFONT = "monospace,fixed,VeraMono:14"
local DEF_LARGEFONT = "sans-serif,helvetica,Vera:20"
local DEF_HUGEFONT = "sans-serif,helvetica,Vera:28"

local DEF_RGBTAB =
{
	{ 210, 210, 210 },	-- 1: background
	{ 000, 000, 000 },	-- 2: black
	{ 255, 255, 255 },	-- 3: white
	{ 110, 130, 160 },	-- 4: active
	{ 225, 225, 225 },	-- 5: bright gray
	{ 120, 120, 120 },	-- 6: dark gray
	{ 200, 080, 020 },	-- 7: signal/focus
	{ 190, 190, 190 },	-- 8: medium gray
}

local DEF_PENTAB =
{
	[ui.PEN_AREABACK] = 1,

	[ui.PEN_BUTTONTEXT] = 2,
	[ui.PEN_BUTTONOVER] = 5,
	[ui.PEN_BUTTONACTIVE] = 8,
	[ui.PEN_BUTTONACTIVETEXT] = 2,
	[ui.PEN_BUTTONOVERDETAIL] = 2,

	[ui.PEN_BUTTONDISABLED] = 1,
	[ui.PEN_BUTTONDISABLEDSHADOW] = 6,
	[ui.PEN_BUTTONDISABLEDSHINE] = 3,
	[ui.PEN_BUTTONDISABLEDTEXT] = 1,

	[ui.PEN_TEXTINPUTBACK] = 1,
	[ui.PEN_TEXTINPUTTEXT] = 2,
	[ui.PEN_TEXTINPUTOVER] = 5,
	[ui.PEN_TEXTINPUTACTIVE] = 5,

	[ui.PEN_LISTVIEWBACK] = 1,
	[ui.PEN_LISTVIEWTEXT] = 2,
	[ui.PEN_LISTVIEWACTIVE] = 4,
	[ui.PEN_LISTVIEWACTIVETEXT] = 3,
	[ui.PEN_ALTLISTVIEWBACK] = 5,

	[ui.PEN_CURSOR] = 7,
	[ui.PEN_CURSORTEXT] = 3,

	[ui.PEN_SHINE] = 3,
	[ui.PEN_SHADOW] = 2,
	[ui.PEN_HALFSHINE] = 5,
	[ui.PEN_HALFSHADOW] = 6,

	[ui.PEN_SLIDERBACK] = 1,
	[ui.PEN_SLIDEROVER] = 5,
	[ui.PEN_SLIDERACTIVE] = 5,

	[ui.PEN_GROUPBACK] = 8,
	[ui.PEN_GROUPLABELTEXT] = 2,

	[ui.PEN_MENUBACK] = 1,
	[ui.PEN_MENUACTIVE] = 4,
	[ui.PEN_MENUACTIVETEXT] = 3,

	[ui.PEN_FOCUSSHINE] = 7,
	[ui.PEN_FOCUSSHADOW] = 7,

	[ui.PEN_LIGHTSHINE] = 5,

	[ui.PEN_FILL] = 4,
}

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

local Theme = _M

function Theme.new(class, self)

	self = self or { }

	self.DefFonts = self.DefFonts or { }
	self.DefFonts.__main = self.DefFonts.__main or DEF_MAINFONT
	self.DefFonts.__small = self.DefFonts.__small or DEF_SMALLFONT
	self.DefFonts.__menu = self.DefFonts.__menu or DEF_MENUFONT
	self.DefFonts.__fixed = self.DefFonts.__fixed or DEF_FIXEDFONT
	self.DefFonts.__large = self.DefFonts.__large or DEF_LARGEFONT
	self.DefFonts.__huge = self.DefFonts.__huge or DEF_HUGEFONT
	self.DefFonts[""] = self.DefFonts[""] or self.DefFonts.__main

	self.RGBTab = self.RGBTab or DEF_RGBTAB
	self.PenTab = self.PenTab or DEF_PENTAB

	self.AreaMargin = self.AreaMargin or self.AreaMargin or false

	self.BorderButtonBorder = self.BorderButtonBorder or false
	self.BorderGroupBorder = self.BorderGroupBorder or false
	self.BorderRecessBorder = self.BorderRecessBorder or false
	self.BorderSocketBorder = self.BorderSocketBorder or false
	self.BorderBlankBorder = self.BorderBlankBorder or false
	self.BorderCursorBorder = self.BorderCursorBorder or false

	self.TextPadding = self.TextPadding or false
	self.TextFontSpec = self.TextFontSpec or false
	self.TextBorderStyle = self.TextBorderStyle or false
	self.TextIBorderStyle = self.TextIBorderStyle or false

	self.ButtonBorderStyle = self.ButtonBorderStyle or false
	self.ButtonIBorderStyle = self.ButtonIBorderStyle or false

	self.FrameMargin = self.FrameMargin or false
	self.FrameBorder = self.FrameBorder or false
	self.FrameIBorder = self.FrameIBorder or false
	self.FramePadding = self.FramePadding or false
	self.FrameBorderStyle = self.FrameBorderStyle or false
	self.FrameIBorderStyle = self.FrameIBorderStyle or false

	self.GadgetBorder = self.GadgetBorder or false
	self.GadgetBorderStyle = self.GadgetBorderStyle or false
	self.GadgetIBorder = self.GadgetIBorder or false
	self.GadgetIBorderStyle = self.GadgetIBorderStyle or false
	self.GadgetMargin = self.GadgetMargin or false
	self.GadgetPadding = self.GadgetPadding or false

	self.GaugeBorder = self.GaugeBorder or false
	self.GaugeBorderStyle = self.GaugeBorderStyle or false
	self.GaugeIBorder = self.GaugeIBorder or false
	self.GaugeIBorderStyle = self.GaugeIBorderStyle or false
	self.GaugeKnobBorder = self.GaugeKnobBorder or false
	self.GaugeKnobBorderStyle = self.GaugeKnobBorderStyle or false
	self.GaugeKnobIBorder = self.GaugeKnobIBorder or false
	self.GaugeKnobIBorderStyle = self.GaugeKnobIBorderStyle or false
	self.GaugeKnobMargin = self.GaugeKnobMargin or false
	self.GaugeKnobMinHeight = self.GaugeKnobMinHeight or false
	self.GaugeKnobMinWidth = self.GaugeKnobMinWidth or false
	self.GaugeMargin = self.GaugeMargin or false
	self.GaugePadding = self.GaugePadding or false

	self.GroupBorder = self.GroupBorder or false
	self.GroupBorderLegend = self.GroupBorderLegend or false
	self.GroupBorderStyle = self.GroupBorderStyle or false
	self.GroupBorderStyleLegend = self.GroupBorderStyleLegend or false
	self.GroupLegendFontSpec = self.GroupLegendFontSpec or false
	self.GroupMargin = self.GroupMargin or false
	self.GroupMarginLegend = self.GroupMarginLegend or false
	self.GroupPadding = self.GroupPadding or false
	self.GroupPaddingLegend = self.GroupPaddingLegend or false

	self.HandleBorder = self.HandleBorder or false
	self.HandleBorderStyle = self.HandleBorderStyle or false
	self.HandleIBorder = self.HandleIBorder or false
	self.HandleIBorderStyle = self.HandleIBorderStyle or false
	self.HandleMargin = self.HandleMargin or false
	self.HandlePadding = self.HandlePadding or false

	self.ImageAreaMargin = self.ImageAreaMargin or false

	self.ListAreaFontSpec = self.ListAreaFontSpec or false
	self.ListAreaMargin = self.ListAreaMargin or false
	self.ListAreaPadding = self.ListAreaPadding or false

	self.MenuBarBackPen = self.MenuBarBackPen or false
	self.MenuBarBorder = self.MenuBarBorder or false
	self.MenuBarBorderStyle = self.MenuBarBorderStyle or false

	self.MenuItemBorder = self.MenuItemBorder or false
	self.MenuItemBorderStyle = self.MenuItemBorderStyle or false
	self.MenuItemChildrenBorderStyle = self.MenuItemChildrenBorderStyle or false
	self.MenuItemChildrenIBorderStyle = self.MenuItemChildrenIBorderStyle or false
	self.MenuItemFontSpec = self.MenuItemFontSpec or false
	self.MenuItemIBorder = self.MenuItemIBorder or false
	self.MenuItemIBorderStyle = self.MenuItemIBorderStyle or false
	self.MenuItemMargin = self.MenuItemMargin or false
	self.MenuItemPadding = self.MenuItemPadding or false

	self.PopItemBorder = self.PopItemBorder or false
	self.PopItemBorderStyle = self.PopItemBorderStyle or false
	self.PopItemChildrenBorderStyle = self.PopItemChildrenBorderStyle or false
	self.PopItemChildrenIBorderStyle = self.PopItemChildrenIBorderStyle or false
	self.PopItemFontSpec = self.PopItemFontSpec or false
	self.PopItemIBorder = self.PopItemIBorder or false
	self.PopItemIBorderStyle = self.PopItemIBorderStyle or false
	self.PopItemMargin = self.PopItemMargin or false
	self.PopItemPadding = self.PopItemPadding or false

	self.PopupBorder = self.PopupBorder or false
	self.PopupBorderStyle = self.PopupBorderStyle or false
	self.PopupMargin = self.PopupMargin or false
	self.PopupPadding = self.PopupPadding or false

	self.RadioButtonBorder = self.RadioButtonBorder or false
	self.RadioButtonBorderStyle = self.RadioButtonBorderStyle or false
	self.RadioButtonIBorder = self.RadioButtonIBorder or false
	self.RadioButtonIBorderStyle = self.RadioButtonIBorderStyle or false
	self.RadioButtonMargin = self.RadioButtonMargin or false
	self.RadioButtonPadding = self.RadioButtonPadding or false

	self.SliderBorder = self.SliderBorder or false
	self.SliderBorderStyle = self.SliderBorderStyle or false
	self.SliderIBorder = self.SliderIBorder or false
	self.SliderIBorderStyle = self.SliderIBorderStyle or false
	self.SliderKnobBorder = self.SliderKnobBorder or false
	self.SliderKnobBorderStyle = self.SliderKnobBorderStyle or false
	self.SliderKnobIBorder = self.SliderKnobIBorder or false
	self.SliderKnobIBorderStyle = self.SliderKnobIBorderStyle or false
	self.SliderKnobMargin = self.SliderKnobMargin or false
	self.SliderKnobMinHeight = self.SliderKnobMinHeight or false
	self.SliderKnobMinWidth = self.SliderKnobMinWidth or false
	self.SliderMargin = self.SliderMargin or false
	self.SliderPadding = self.SliderPadding or false

	self.SpacerBorder = self.SpacerBorder or false
	self.SpacerBorderStyle = self.SpacerBorderStyle or false
	self.SpacerMargin = self.SpacerMargin or false
	self.SpacerPadding = self.SpacerPadding or false

	self.TextInputBorderStyle = self.TextInputBorderStyle or false
	self.TextInputFontSpec = self.TextInputFontSpec or false

	self.CheckMarkMargin = self.CheckMarkMargin or false
	self.CheckMarkBorder = self.CheckMarkBorder or false
	self.CheckMarkIBorder = self.CheckMarkIBorder or false
	self.CheckMarkPadding = self.CheckMarkPadding or false
	self.CheckMarkBorderStyle = self.CheckMarkBorderStyle or false
	self.CheckMarkIBorderStyle = self.CheckMarkIBorderStyle or false

	self.ImportConfig = self.ImportConfig == nil and true or self.ImportConfig

	self = Object.new(class, self)

	if self.ImportConfig ~= false then
		importConfig(self)
	end

	return self

end

-------------------------------------------------------------------------------
--	getconfig: determine colors from GTK+ configuration
-------------------------------------------------------------------------------

local function lum(rgb, l)
-- 	local y = 0.299 * rgb[1] + 0.587 * rgb[2] + 0.114 * rgb[3]
-- 	local u = -0.147 * rgb[1] - 0.289 * rgb[2] + 0.436 * rgb[3]
-- 	local v = 0.615 * rgb[1] - 0.515 * rgb[2] - 0.100 * rgb[3]
-- 	y = y * l
-- 	return { min(255, y + 1.140 * v), min(255, y - 0.396 * u - 0.581 * v),
-- 		min(255, y + 2.029 * u) }
	return { min(255, rgb[1] * l), min(255, rgb[2] * l), min(255, rgb[3] * l) }
end

function Theme:importConfig()
	local p = getenv("GTK2_RC_FILES")
	if p then
		local paths = { }
		p:gsub("([^:]+):?", function(p)
			insert(paths, p)
		end)
		for _, fname in ipairs(paths) do
			db.info("Trying config file %s", fname)
			local f = open(fname)
			if f then
				local style
				for line in f:lines() do
					line = line:match("^%s*(.*)%s*$")
					local newstyle = line:match("^style%s+\"(%w+)\"$")
					if newstyle then
						style = newstyle
					end
					if style == "default" then
						local color, r, g, b =
							line:match("^(%w+%[%w+%])%s*=%s*{%s*([%d.]+)%s*,%s*([%d.]+)%s*,%s*([%d.]+)%s*}$")
						if color then
							if color == "bg[NORMAL]" then
								self.RGBTab[9] = { tonumber(r) * 255,
									tonumber(g) * 255, tonumber(b) * 255 }
								self.RGBTab[14] = lum(self.RGBTab[9], 1.4)
								self.RGBTab[15] = lum(self.RGBTab[9], 0.6)
								self.RGBTab[16] = lum(self.RGBTab[9], 1.2)
								self.RGBTab[17] = lum(self.RGBTab[9], 0.85)
								self.RGBTab[19] = lum(self.RGBTab[9], 1.3)
								self.PenTab[ui.PEN_LIGHTSHINE] = 19
								self.PenTab[ui.PEN_GROUPBACK] = 16
								self.PenTab[ui.PEN_HALFSHINE] = 14
								self.PenTab[ui.PEN_HALFSHADOW] = 15
								self.PenTab[ui.PEN_BUTTONOVER] = 16
								self.PenTab[ui.PEN_SLIDERBACK] = 17
								self.PenTab[ui.PEN_SLIDEROVER] = 17
								self.PenTab[ui.PEN_SLIDERACTIVE] = 17
								self.PenTab[ui.PEN_BUTTONDISABLED] = 9
								self.PenTab[ui.PEN_BUTTONDISABLEDSHADOW] = 15
								self.PenTab[ui.PEN_BUTTONDISABLEDSHINE] = 14
								self.PenTab[ui.PEN_BUTTONDISABLEDTEXT] = 9
								self.PenTab[ui.PEN_MENUBACK] = 9
								self.PenTab[ui.PEN_AREABACK] = 9
							elseif color == "base[NORMAL]" then
								self.RGBTab[18] = { tonumber(r) * 255,
									tonumber(g) * 255, tonumber(b) * 255 }
								self.RGBTab[20] = lum(self.RGBTab[18], 1.05)
								self.RGBTab[18] = lum(self.RGBTab[18], 0.95)
								self.PenTab[ui.PEN_LISTVIEWBACK] = 18
								self.PenTab[ui.PEN_TEXTINPUTBACK] = 18
								self.PenTab[ui.PEN_TEXTINPUTOVER] = 18
								self.PenTab[ui.PEN_ALTLISTVIEWBACK] = 20
							elseif color == "bg[SELECTED]" then
								self.RGBTab[10] = { tonumber(r) * 255,
									tonumber(g) * 255, tonumber(b) * 255 }
								self.PenTab[ui.PEN_LISTVIEWACTIVE] = 10
								self.PenTab[ui.PEN_CURSOR] = 10
								self.PenTab[ui.PEN_FOCUSSHADOW] = 10
								self.PenTab[ui.PEN_FOCUSSHINE] = 10
								self.PenTab[ui.PEN_MENUACTIVE] = 10
								self.PenTab[ui.PEN_FILL] = 10
							elseif color == "bg[ACTIVE]" then
								self.RGBTab[11] = { tonumber(r) * 255,
									tonumber(g) * 255, tonumber(b) * 255 }
								self.PenTab[ui.PEN_BUTTONACTIVE] = 11
								self.PenTab[ui.PEN_TEXTINPUTACTIVE] = 11
							elseif color == "text[NORMAL]" then
								self.RGBTab[12] = { tonumber(r) * 255,
									tonumber(g) * 255, tonumber(b) * 255 }
								self.PenTab[ui.PEN_BUTTONTEXT] = 12
								self.PenTab[ui.PEN_LISTVIEWTEXT] = 12
								self.PenTab[ui.PEN_BUTTONACTIVETEXT] = 12
								self.PenTab[ui.PEN_BUTTONOVERDETAIL] = 12
								self.PenTab[ui.PEN_TEXTINPUTTEXT] = 12
								self.PenTab[ui.PEN_GROUPLABELTEXT] = 12
								self.PenTab[ui.PEN_MENUACTIVETEXT] = 12
							elseif color == "fg[NORMAL]" then
								self.RGBTab[13] = { tonumber(r) * 255,
									tonumber(g) * 255, tonumber(b) * 255 }
							end
						end
					end
				end
				f:close()
			end

		end
	end
end

