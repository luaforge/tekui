#!/usr/bin/env lua

local ui = require "tek.ui"
local db = require "tek.lib.debug"

local window = ui.Window:new
{
	Orientation = "vertical",
	Id = "layout-window",
	Title = "Layout",
	Status = "hide",
	MaxWidth = ui.HUGE,
	MaxHeight = ui.HUGE,
	Notifications =
	{
		["Status"] =
		{
			["show"] =
			{
				{ ui.NOTIFY_ID, "layout-window-button", "setValue", "Selected", true }
			},
			["hide"] =
			{
				{ ui.NOTIFY_ID, "layout-window-button", "setValue", "Selected", false }
			},
		},
	},
	Children =
	{
		ui.Group:new
		{
			Legend = "Relative Sizes",
			Children =
			{
				ui.Text:new { Text = "1", MaxWidth = ui.HUGE },
				ui.Spacer:new { },
				ui.Text:new { Text = "12", MaxWidth = ui.HUGE },
				ui.Spacer:new { },
				ui.Text:new { Text = "123", MaxWidth = ui.HUGE },
				ui.Spacer:new { },
				ui.Text:new { Text = "1234", MaxWidth = ui.HUGE },
				ui.Spacer:new { },
				ui.Text:new { Text = "12345", MaxWidth = ui.HUGE },
			},
		},
		ui.Group:new
		{
			SameSize = true,
			Legend = "Same Sizes",
			Children =
			{
				ui.Text:new { Text = "1", MaxWidth = ui.HUGE },
				ui.Spacer:new { },
				ui.Text:new { Text = "12", MaxWidth = ui.HUGE },
				ui.Spacer:new { },
				ui.Text:new { Text = "123", MaxWidth = ui.HUGE },
				ui.Spacer:new { },
				ui.Text:new { Text = "1234", MaxWidth = ui.HUGE },
				ui.Spacer:new { },
				ui.Text:new { Text = "12345", MaxWidth = ui.HUGE },
			},
		},
		ui.Group:new
		{
			Legend = "Balancing Group",
			Children =
			{
				ui.Text:new { Id="1", Text = "free", Height = "fill" },
				ui.Handle:new { Id="2" },
				ui.Text:new { Id="3", Text = "free", Height = "fill" },
				ui.Handle:new { Id="4" },
				ui.Text:new { Id="5", Text = "free", Height = "fill" },
			},
		},
		ui.Handle:new { },
		ui.Group:new
		{
			Height = "free",
			Legend = "Grid",
			GridWidth = 3,
			SameSize = true,
			Children =
			{
				ui.Text:new { Text = "1", Height = "free" },
				ui.Text:new { Text = "12", Height = "free" },
				ui.Text:new { Text = "123", Height = "free" },
				ui.Text:new { Text = "1234", Height = "free" },
				ui.Text:new { Text = "12345", Height = "free" },
				ui.Text:new { Text = "123456", Height = "free" },
			},
		},
		ui.Group:new
		{
			Legend = "Fixed vs. Free",
			Children =
			{
				ui.Text:new { Text = "fix" },
				ui.Text:new { Text = "25%", MaxWidth = ui.HUGE, Weight = 0x4000 },
				ui.Text:new { Text = "fix" },
				ui.Text:new { Text = "75%", MaxWidth = ui.HUGE, Weight = 0xc000 },
				ui.Text:new { Text = "fix" },
			},
		},
		ui.Group:new
		{
			MaxHeight = ui.HUGE,
			Legend = "Different Weights",
			Children =
			{
				ui.Text:new { Text = "25%", MaxWidth = ui.HUGE, MaxHeight = ui.HUGE, Weight = 0x4000 },
				ui.Spacer:new { },
				ui.Text:new { Text = "25%", MaxWidth = ui.HUGE, MaxHeight = ui.HUGE, Weight = 0x4000 },
				ui.Spacer:new { },
				ui.Text:new { Text = "50%", MaxWidth = ui.HUGE, MaxHeight = ui.HUGE, Weight = 0x8000 },
			},
		},
	},
}

if ui.ProgName == "layout.lua" then
	local app = ui.Application:new()
	ui.Application.connect(window)
	app:addMember(window)
	window:setValue("Status", "show")
	app:run()
else
	return
	{
		Window = window,
		Name = "Layout",
		Description = "This demonstrates the various layouting options.",
	}
end
