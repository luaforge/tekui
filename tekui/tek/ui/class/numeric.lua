-------------------------------------------------------------------------------
--
--	tek.ui.class.numeric
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
--		Numeric
--
--	OVERVIEW::
--		This class implements the management of numerical
--		values. Without further specialization it has hardly any real-life
--		use and may be considered an abstract class. See also
--		[[#tek.ui.class.gauge : Gauge]] and [[#tek.ui.class.slider : Slider]]
--		for some of its child classes.
--
--	ATTRIBUTES::
--		- {{Default [IG]}} (number)
--			The default for {{Value}}, which can be revoked using the
--			Numeric:reset() method.
--		- {{Max [ISG]}} (number)
--			Maximum acceptable {{Value}}. Setting this value
--			invokes the Numeric:onSetMax() method.
--		- {{Min [ISG]}} (number)
--			Minimum acceptable {{Value}}. Setting this value
--			invokes the Numeric:onSetMin() method.
--		- {{Step [ISG]}} (number)
--			Default step value [Default: 1]
--		- {{Value [ISG]}} (number)
--			The current value represented by this class. Setting this
--			value causes the Numeric:onSetValue() method to be invoked.
--
--	IMPLEMENTS::
--		- Numeric:decrease() - Decreases {{Value}}
--		- Numeric:increase() - Increases {{Value}}
--		- Numeric:onSetMax() - Handler for the {{Max}} attribute
--		- Numeric:onSetMin() - Handler for the {{Min}} attribute
--		- Numeric:onSetValue() - Handler for the {{Value}} attribute
--		- Numeric:reset() - Resets {{Value}} to the {{Default}} value
--
--	OVERRIDES::
--		- Element:cleanup()
--		- Object.init()
--		- Element:setup()
--
-------------------------------------------------------------------------------

local db = require "tek.lib.debug"
local ui = require "tek.ui"
local Gadget = ui.Gadget

local floor = math.floor
local max = math.max
local min = math.min
local unpack = unpack

module("tek.ui.class.numeric", tek.ui.class.gadget)
_VERSION = "Numeric 1.3"

-------------------------------------------------------------------------------
-- Data:
-------------------------------------------------------------------------------

local NOTIFY_VALUE = { ui.NOTIFY_SELF, "onSetValue", ui.NOTIFY_VALUE }
local NOTIFY_MIN = { ui.NOTIFY_SELF, "onSetMin", ui.NOTIFY_VALUE }
local NOTIFY_MAX = { ui.NOTIFY_SELF, "onSetMax", ui.NOTIFY_VALUE }

-------------------------------------------------------------------------------
-- Class implementation:
-------------------------------------------------------------------------------

local Numeric = _M

function Numeric.init(self)
	self = self or { }
	self.Min = self.Min or 1
	self.Max = self.Max or 100
	self.Default = max(self.Min, min(self.Max, self.Default or self.Min))
	self.Value = max(self.Min, min(self.Max, self.Value or self.Default))
	self.Step = self.Step or 1
	return Gadget.init(self)
end

-------------------------------------------------------------------------------
--	setup: overrides
-------------------------------------------------------------------------------

function Numeric:setup(app, window)
	Gadget.setup(self, app, window)
	self:addNotify("Value", ui.NOTIFY_CHANGE, NOTIFY_VALUE, 1)
	self:addNotify("Min", ui.NOTIFY_CHANGE, NOTIFY_MIN, 1)
	self:addNotify("Max", ui.NOTIFY_CHANGE, NOTIFY_MAX, 1)
end

-------------------------------------------------------------------------------
--	cleanup: overrides
-------------------------------------------------------------------------------

function Numeric:cleanup()
	self:addNotify("Max", ui.NOTIFY_CHANGE, NOTIFY_MAX)
	self:addNotify("Min", ui.NOTIFY_CHANGE, NOTIFY_MIN)
	self:remNotify("Value", ui.NOTIFY_CHANGE, NOTIFY_VALUE)
	Gadget.cleanup(self)
end

-------------------------------------------------------------------------------
--	Numeric:increase([delta]): Increase {{Value}} by the specified {{delta}}.
--	If {{delta}} is omitted, the {{Step}} attribute is used in its place.
-------------------------------------------------------------------------------

function Numeric:increase(d)
	self:setValue("Value", self.Value + (d or self.Step))
end

-------------------------------------------------------------------------------
--	Numeric:decrease([delta]): Decrease {{Value}} by the specified {{delta}}.
--	If {{delta}} is omitted, the {{Step}} attribute is used in its place.
-------------------------------------------------------------------------------

function Numeric:decrease(d)
	self:setValue("Value", self.Value - (d or self.Step))
end

-------------------------------------------------------------------------------
--	Numeric:reset(): Reset {{Value}} to is {{Default}} value.
-------------------------------------------------------------------------------

function Numeric:reset()
	self:setValue("Value", self.Default)
end

-------------------------------------------------------------------------------
--	onSetValue(val): This handler is invoked when the Numeric's {{Value}}
--	attribute has changed.
-------------------------------------------------------------------------------

function Numeric:onSetValue(v)
	self.Value = max(self.Min, min(self.Max, v))
end

-------------------------------------------------------------------------------
--	onSetMin(min): This handler is invoked when the Numeric's {{Min}}
--	attribute has changed.
-------------------------------------------------------------------------------

function Numeric:onSetMin(v)
	self.Min = min(v, self.Max)
	self:setValue("Value", self.Value)
end

-------------------------------------------------------------------------------
--	onSetMax(max): This handler is invoked when the Numeric's {{Max}}
--	attribute has changed.
-------------------------------------------------------------------------------

function Numeric:onSetMax(v)
	self.Max = max(self.Min, v)
	local d = self.Value - self.Max
	if d > 0 then
		self.Value = self.Value - d
	end
	self:setValue("Value", self.Value)
end
