-------------------------------------------------------------------------------
--
--	tek.ui.class.scrollbar
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
--		[[#tek.ui.class.group: : Group]] /
--		ScrollBar
--
--	OVERVIEW::
--		Implements a group containing a
--		[[#tek.ui.class.slider : Slider]] and arrow buttons.
--
--	ATTRIBUTES::
--		- {{Orientation [IG]}} (string)
--			The orientation of the scrollbar, which can be "horizontal"
--			or "vertical"
--		- {{Max [ISG]}} (number)
--			The maximum value the slider can accept. Setting this value
--			invokes the ScrollBar:onSetMax() method.
--		- {{Min [ISG]}} (number)
--			The minimum value the slider can accept. Setting this value
--			invokes the ScrollBar:onSetMin() method.
--		- {{Range [ISG]}} (number)
--			The range of the slider, i.e. the size it represents. Setting
--			this value invokes the ScrollBar:onSetRange() method.
--		- {{Style [IG]}} (string)
--			See [[#tek.ui.class.slider : Slider]]
--		- {{Value [ISG]}} (number)
--			The value of the slider. Setting this value invokes the
--			ScrollBar:onSetValue() method.
--
--	IMPLEMENTS::
--		- ScrollBar:onSetMax() - Handler for the {{Max}} attribute
--		- ScrollBar:onSetMin() - Handler for the {{Min}} attribute
--		- ScrollBar:onSetRange() - Handler for the {{Range}} attribute
--		- ScrollBar:onSetValue() - Handler for the {{Value}} attribute
--
--	OVERRIDES::
--		- Element:cleanup()
--		- Class.new()
--		- Element:setup()
--
-------------------------------------------------------------------------------

local db = require "tek.lib.debug"
local ui = require "tek.ui"
local Image = ui.Image
local Group = ui.Group

local max = math.max
local min = math.min

module("tek.ui.class.scrollbar", tek.ui.class.group)
_VERSION = "ScrollBar 5.2"

local ScrollBar = _M

-------------------------------------------------------------------------------
--	Data:
-------------------------------------------------------------------------------

local SLIDERMARGIN = { 0, 0, 0, 0 }
local SLIDERPADDING = { 0, 0, 0, 0 }

local ARROWMARGIN = { 0, 0, 0, 0 }
local ARROWPADDING = { 0, 0, 0, 0 }
local ARROWIMAGEMARGIN = { 1, 1, 1, 1 }
local ARROWBORDER1_HORIZ = { 1, 1, 0, 1 }
local ARROWBORDER2_HORIZ = { 0, 1, 1, 1 }
local ARROWBORDER1_VERT = { 1, 1, 1, 0 }
local ARROWBORDER2_VERT = { 1, 0, 1, 0 }

local coordx = { 0,0, 10,10, 10,-10 }
local coordy = { 0,0, 10,10, -10,10 }
local prims = { { 0x1000, 3, Points = { 1, 2, 3 }, Pen = ui.PEN_BUTTONTEXT } }

local ArrowUpImage = ui.VectorImage:new { ImageData = { Coords = coordy,
	Primitives = prims, MinMax = { 12,12, -12,-2 } } }
local ArrowDownImage = ui.VectorImage:new { ImageData = { Coords = coordy,
	Primitives = prims, MinMax = { -12,-2, 12,12 } } }
local ArrowLeftImage = ui.VectorImage:new { ImageData = { Coords = coordx,
	Primitives = prims, MinMax = { -2,-12, 12,12 } } }
local ArrowRightImage = ui.VectorImage:new { ImageData = { Coords = coordx,
	Primitives = prims, MinMax = { 12,12, -2,-12 } } }

local NOTIFY_VALUE = { ui.NOTIFY_SELF, "onSetValue", ui.NOTIFY_VALUE }
local NOTIFY_MIN = { ui.NOTIFY_SELF, "onSetMin", ui.NOTIFY_VALUE }
local NOTIFY_MAX = { ui.NOTIFY_SELF, "onSetMax", ui.NOTIFY_VALUE }
local NOTIFY_RANGE = { ui.NOTIFY_SELF, "onSetRange", ui.NOTIFY_VALUE }

-------------------------------------------------------------------------------
--	ArrowButton:
-------------------------------------------------------------------------------

local ArrowButton = Image:newClass { _NAME = "_sbarrow" }

function ArrowButton.init(self)
	self.Mode = "button"
	self.IBorderStyle = "button"
	self.Padding = ARROWPADDING
	self.Margin = ARROWMARGIN
	self.ImageMargin = ARROWIMAGEMARGIN
	self.MinWidth = self.MinWidth or 14
	self.MinHeight = self.MinWidth or 14
	self.MaxWidth = 14
	self.MaxHeight = 14
	self.Width = "fill"
	self.Height = "fill"
	self.Increase = self.Increase or 1
	return Image.init(self)
end

function ArrowButton:draw()
	-- TODO: modifying static data is ugly and wouldn't be thread-safe:
	prims[1].Pen =
		self.Disabled and ui.PEN_BUTTONDISABLEDSHADOW or
		self.Selected and ui.PEN_BUTTONACTIVETEXT or
		self.Hover and ui.PEN_BUTTONOVERDETAIL or
		ui.PEN_BUTTONTEXT
	Image.draw(self)
end

function ArrowButton:onPress(pressed)
	if pressed then
		self.Slider:increase(self.Increase)
	end
	Image.onPress(self, pressed)
end

function ArrowButton:onHold(hold)
	if hold then
		self.Slider:increase(self.Increase)
	end
	Image.onHold(self, hold)
end

-------------------------------------------------------------------------------
--	Slider:
-------------------------------------------------------------------------------

local SBSlider = ui.Slider:newClass { _NAME = "_sbslider" }

function SBSlider:onSetValue(v)
	self.ScrollBar:setValue("Value", v)
	ui.Slider.onSetValue(self, v)
end

local function updateSlider(self)
	local disabled = self.Min == self.Max
	local sb = self.ScrollBar.Children
	if disabled ~= sb[1].Disabled then
		sb[1]:setValue("Disabled", disabled)
		sb[2]:setValue("Disabled", disabled)
		sb[3]:setValue("Disabled", disabled)
	end
end

function SBSlider:onSetRange(r)
	self.ScrollBar:setValue("Range", r)
	ui.Slider.onSetRange(self, r)
	updateSlider(self)
end

function SBSlider:onSetMin(m)
	self.ScrollBar:setValue("Min", m)
	ui.Slider.onSetMin(self, m)
	updateSlider(self)
end

function SBSlider:onSetMax(m)
	self.ScrollBar:setValue("Max", m)
	ui.Slider.onSetMax(self, m)
	updateSlider(self)
end

-------------------------------------------------------------------------------
--	ScrollBar:
-------------------------------------------------------------------------------

function ScrollBar.new(class, self)
	self = self or { }

	self.Orientation = self.Orientation or "horizontal"
	self.Min = self.Min or 1
	self.Max = self.Max or 100
	self.Default = max(self.Min, min(self.Max, self.Default or self.Min))
	self.Value = max(self.Min, min(self.Max, self.Value or self.Default))
	self.Range = max(self.Max, self.Range or self.Max)
	self.Step = self.Step or 1
	self.ForceInteger = self.ForceInteger or false
	self.VAlign = self.VAlign or "center"
	self.HAlign = self.HAlign or "center"
	self.Child = self.Child or false
	self.ArrowOrientation = self.ArrowOrientation or self.Orientation

	self.Style = self.Style or false

	self.Slider = self.Slider or SBSlider:new
	{
		Child = self.Child,
		ScrollBar = self,
		Min = self.Min,
		Max = self.Max,
		Value = self.Value,
		Range = self.Range,
		Step = self.Step,
		Notifications = self.Notifications,
		Margin = SLIDERMARGIN,
		Padding = SLIDERPADDING,
		Orientation = self.Orientation,
		ForceInteger = self.ForceInteger,
		Style = self.Style or "scrollbar",
	}
	self.Notifications = false

	if self.Orientation == "vertical" then
		local increase = -1
		local border1, border2 = false, false
		local img1, img2 = ArrowUpImage, ArrowDownImage
		if self.ArrowOrientation == "horizontal" then
			img1, img2 = ArrowLeftImage, ArrowRightImage
			border1 = ARROWBORDER1_HORIZ
			increase = -increase
		end
		self.ArrowButton1 = ArrowButton:new
		{
			Image = img1,
			Border = border1,
			Slider = self.Slider,
			Increase = increase,
		}
		self.ArrowButton2 = ArrowButton:new
		{
			Border = border2,
			Image = img2,
			Slider = self.Slider,
			Increase = -increase,
		}
		self.Width = self.Width or "auto"
		self.Height = self.Height or "fill"
	else
		local border1, border2 = false, false
		local increase = -1
		local img1, img2 = ArrowLeftImage, ArrowRightImage
		if self.ArrowOrientation == "vertical" then
			img1, img2 = ArrowUpImage, ArrowDownImage
			border1 = ARROWBORDER1_VERT
			increase = -increase
		end
		self.ArrowButton1 = ArrowButton:new
		{
			Image = img1,
			Border = border1,
			Slider = self.Slider,
			Increase = increase,
		}
		self.ArrowButton2 = ArrowButton:new
		{
			Image = img2,
			Border = border2,
			Slider = self.Slider,
			Increase = -increase,
		}
		self.Width = self.Width or "fill"
		self.Height = self.Height or "auto"
	end

	if self.ArrowOrientation ~= self.Orientation then
		self.Children =
		{
			self.Slider,
			ui.Group:new
			{
				Orientation = "horizontal" and "vertical" or self.Orientation,
				Children =
				{
					self.ArrowButton1,
					self.ArrowButton2
				}
			}
		}
	else
		self.Children =
		{
			self.Slider,
			self.ArrowButton1,
			self.ArrowButton2
		}
	end
	return Group.new(class, self)

end

-------------------------------------------------------------------------------
--	setup: overrides
-------------------------------------------------------------------------------

function ScrollBar:setup(app, window)
	Group.setup(self, app, window)
	self:addNotify("Value", ui.NOTIFY_CHANGE, NOTIFY_VALUE, 1)
	self:addNotify("Min", ui.NOTIFY_CHANGE, NOTIFY_MIN, 1)
	self:addNotify("Max", ui.NOTIFY_CHANGE, NOTIFY_MAX, 1)
	self:addNotify("Range", ui.NOTIFY_CHANGE, NOTIFY_RANGE, 1)
end

-------------------------------------------------------------------------------
--	cleanup: overrides
-------------------------------------------------------------------------------

function ScrollBar:cleanup()
	self:remNotify("Range", ui.NOTIFY_CHANGE, NOTIFY_RANGE)
	self:remNotify("Max", ui.NOTIFY_CHANGE, NOTIFY_MAX)
	self:remNotify("Min", ui.NOTIFY_CHANGE, NOTIFY_MIN)
	self:remNotify("Value", ui.NOTIFY_CHANGE, NOTIFY_VALUE)
	Group.cleanup(self)
end

-------------------------------------------------------------------------------
--	onSetMin(min): This handler is invoked when the ScrollBar's {{Min}}
--	attribute has changed. See also Numeric:onSetMin().
-------------------------------------------------------------------------------

function ScrollBar:onSetMin(v)
	self.Slider:setValue("Min", v)
	self.Min = self.Slider.Min
end

-------------------------------------------------------------------------------
--	onSetMax(max): This handler is invoked when the ScrollBar's {{Max}}
--	attribute has changed. See also Numeric:onSetMax().
-------------------------------------------------------------------------------

function ScrollBar:onSetMax(v)
	self.Slider:setValue("Max", v)
	self.Max = self.Slider.Max
end

-------------------------------------------------------------------------------
--	onSetValue(val): This handler is invoked when the ScrollBar's {{Value}}
--	attribute has changed. See also Numeric:onSetValue().
-------------------------------------------------------------------------------

function ScrollBar:onSetValue(v)
	self.Slider:setValue("Value", v)
	self.Value = self.Slider.Value
end

-------------------------------------------------------------------------------
--	onSetRange(range): This handler is invoked when the ScrollBar's {{Range}}
--	attribute has changed. See also Slider:onSetRange().
-------------------------------------------------------------------------------

function ScrollBar:onSetRange(v)
	self.Slider:setValue("Range", v)
	self.Range = self.Slider.Range
end
