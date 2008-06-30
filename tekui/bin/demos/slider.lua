#!/usr/bin/env lua

local ui = require "tek.ui"
local db = require "tek.lib.debug"
local window = ui.Window:new
{
	Id = "slider-window",
	Title = "Slider",
	Status = "hide",
	MaxWidth = ui.HUGE,
	MaxHeight = ui.HUGE,
	Orientation = "vertical",
	Notifications =
	{
		["Status"] =
		{
			["show"] =
			{
				{ ui.NOTIFY_ID, "slider-window-button", "setValue", "Selected", true }
			},
			["hide"] =
			{
				{ ui.NOTIFY_ID, "slider-window-button", "setValue", "Selected", false }
			},
		},
	},
	Children =
	{
		ui.Group:new
		{
			Legend = "Sliders",
			GridWidth = 3,
			Children =
			{
				ui.Text:new
				{
					Text = "Continuous",
					Width = "fill",
				},
				ui.ScrollBar:new
				{
					Id = "slider-slider-1",
					Width = "free",
					Min = 0,
					Max = 10,
					Style = "number",
					Notifications =
					{
						["Value"] =
						{
							[ui.NOTIFY_CHANGE] =
							{
								{ ui.NOTIFY_ID, "slider-text-1", "setValue", "Text", ui.NOTIFY_FORMAT, "%2.2f" },
								{ ui.NOTIFY_ID, "slider-slider-2", "setValue", "Value", ui.NOTIFY_VALUE },
								{ ui.NOTIFY_ID, "slider-gauge-1", "setValue", "Value", ui.NOTIFY_VALUE  }
							}
						}
					}
				},
				ui.Text:new
				{
					Id = "slider-text-1",
					Width = "fill",
					Text = "  0.00  ",
					KeepMinWidth = true,
				},

				ui.Text:new
				{
					Text = "Integer Step",
					Width = "fill",
				},
				ui.ScrollBar:new
				{
					Id = "slider-slider-2",
					Width = "free",
					Min = 0,
					Max = 10,
					ForceInteger = true,
					Style = "number",
					Notifications =
					{
						["Value"] =
						{
							[ui.NOTIFY_CHANGE] =
							{
								{ ui.NOTIFY_ID, "slider-text-2", "setValue", "Text", ui.NOTIFY_FORMAT, "%d" },
								{ ui.NOTIFY_ID, "slider-slider-1", "setValue", "Value", ui.NOTIFY_VALUE },
								{ ui.NOTIFY_ID, "slider-gauge-1", "setValue", "Value", ui.NOTIFY_VALUE  }
							}
						}
					}
				},
				ui.Text:new
				{
					Id = "slider-text-2",
					Width = "fill",
					Text = "  0  ",
					KeepMinWidth = true,
				},

				ui.Text:new
				{
					Text = "Range",
					Width = "fill",
				},
				ui.ScrollBar:new
				{
					Id = "slider-slider-3",
					Width = "free",
					Min = 10,
					Max = 20,
					ForceInteger = true,
					Style = "number",
					Notifications =
					{
						["Value"] =
						{
							[ui.NOTIFY_CHANGE] =
							{
								{ ui.NOTIFY_ID, "slider-text-3", "setValue", "Text", ui.NOTIFY_FORMAT, "%d" },
								{ ui.NOTIFY_ID, "slider-slider-1", "setValue", "Range", ui.NOTIFY_VALUE },
								{ ui.NOTIFY_ID, "slider-slider-2", "setValue", "Range", ui.NOTIFY_VALUE },
							}
						}
					}
				},
				ui.Text:new
				{
					Id = "slider-text-3",
					Width = "fill",
					Text = "  0  ",
					KeepMinWidth = true,
				},

			}
		},
		ui.Group:new
		{
			Legend = "Gauges",
			Children =
			{
				ui.Gauge:new
				{
					Min = 0,
					Max = 10,
					Id = "slider-gauge-1",
					Width = "free",
				},
			}
		},
	}
}

if ui.ProgName == "slider.lua" then
	local app = ui.Application:new()
	ui.Application.connect(window)
	app:addMember(window)
	window:setValue("Status", "show")
	app:run()
else
	return
	{
		Window = window,
		Name = "Slider",
		Description = "Slider",
	}
end
