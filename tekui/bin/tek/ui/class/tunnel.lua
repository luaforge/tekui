
--
--	tek.ui.class.tunnel
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--

local ui = require "tek.ui"
local db = require "tek.lib.debug"
local Frame = ui.Frame

local floor = math.floor
local max = math.max
local min = math.min
local pi = math.pi
local sin = math.sin

module("tek.ui.class.tunnel", tek.ui.class.frame)
_VERSION = "Tunnel 1.8"

-------------------------------------------------------------------------------
--	Class implementation:
-------------------------------------------------------------------------------

local Tunnel = _M

function Tunnel:show(display, drawable)
	if Frame.show(self, display, drawable) then
		self.Window:addNotify("Interval", ui.NOTIFY_ALWAYS,
			self.IntervalNotify)
		return true
	end
end

function Tunnel:hide()
	Frame.hide(self)
	self.Window:remNotify("Interval", ui.NOTIFY_ALWAYS, self.IntervalNotify)
end

-------------------------------------------------------------------------------

function Tunnel:setNumSeg(val)
	self.numseg = val
end
function Tunnel:setSpeed(val)
	self.speed = val
end
function Tunnel:setViewZ(val)
	self.viewz = val
end

-------------------------------------------------------------------------------

function Tunnel.init(self)

	self.MinWidth = self.MinWidth or 128
	self.MinHeight = self.MinHeight or 128
	self.MaxWidth = self.MaxWidth or 640
	self.MaxHeight = self.MaxHeight or 480

	self.IntervalNotify = { self, "updateInterval" }

	-- movement table:
	self.dx = {  }
	self.ndx = 32
	for i = 1, self.ndx do
		self.dx[i] = sin(i * pi * 2 / 32) * 5
	end

	-- current offs in movement table:
	self.cx = 1
	self.cy = 8

	self.numseg = 6
	self.speed = 13
	self.z = 0
	self.viewz = 0x50
	self.dist = 0x100
	self.size = { 320, 256 }

	return Frame.init(self)
end

function Tunnel:draw()
	local d = self.Drawable
	local p0, p1 = d.Pens[ui.PEN_SHADOW], d.Pens[ui.PEN_SHINE]
	local r = self.Rect

	d:fillRect(r[1], r[2], r[3], r[4], p0)

	local sx = floor((r[1] + r[3]) / 2)
	local sy = floor((r[2] + r[4]) / 2)

	local z = self.z + self.viewz
	local cx = self.cx
	local cy = self.cy

	for i = 1, self.numseg do

		local x = self.size[1] * self.viewz / z
		local y = self.size[2] * self.viewz / z

		local dx = self.dx[cx] * z / 256
		local dy = self.dx[cy] * z / 256

		local x0 = min(max(sx - x + dx, r[1]), r[3])
		local y0 = min(max(sy - y + dy, r[2]), r[4])
		local x1 = min(max(sx + x + dx, r[1]), r[3])
		local	y1 = min(max(sy + y + dy, r[2]), r[4])
		if x0 ~= r[1] or x1 ~= r[3] or y0 ~= r[2] or y1 ~= r[4] then
			d:drawRect(x0, y0, x1, y1, p1)
		end
		z = z + self.dist
		cx = cx == self.ndx and 1 or cx + 1
		cy = cy == self.ndx and 1 or cy + 1
	end
end

function Tunnel:updateInterval()
-- 	db.warn("update")
	self.z = self.z - self.speed
	if self.z < 0 then
		self.z = self.z + self.dist
		self.cx = self.cx == self.ndx and 1 or self.cx + 1
		self.cy = self.cy == self.ndx and 1 or self.cy + 1
	end
	self.Redraw = true
end
