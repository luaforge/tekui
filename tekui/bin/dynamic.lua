#!/usr/bin/env lua

require "tek.lib.debug".level = 4
local ui = require "tek.ui"
-- ui.DEBUG = true

ui.application:new
{
	Children =
	{
		ui.window:new
		{
			Title = "Dynamic Weight 1",
			Children =
			{
				ui.group:new
				{
					Legend = "Dynamic Weight",
					Children =
					{
						ui.group:new
						{
							Title = "Hallo",
							Width = "free",
							Children =
							{
								ui.Slider:new
								{
									Min = 0,
									Max = 100000,
									Notifications =
									{
										["Value"] =
										{
											[ui.NOTIFY_CHANGE] = {
												{ ui.NOTIFY_ID, "weight-1", "setValue", "Text", ui.NOTIFY_FORMAT, "%d" },
											},
										},
									},
								},
								ui.Text:new
								{
									FontSpec = "utopia:100",
									Id = "weight-1",
									Text = "0",
									Width = "auto"
								},
							},
						},
					},
				},
			},
		},
		ui.window:new
		{
			Title = "Dynamic Weight 2",
			Legend = "Dynamic Weight",
			Orientation = "vertical",
			Children =
			{
				ui.group:new
				{
					Children =
					{
						ui.Slider:new
						{
							Knob = ui.Text:new
							{
								Id = "slider-knob",
								Style = "button",
								Text = "$08000",
							},
							Id = "slider-2",
							Min = 0,
							Max = 0x10000,
							Width = "free",
							Default = 0x8000,
							Step = 0x400,
							Notifications =
							{
								["Value"] =
								{
									[ui.NOTIFY_CHANGE] =
									{
										{ ui.NOTIFY_ID, "slider-weight-1", "setValue", "Text", ui.NOTIFY_FORMAT, "$%05x" },
										{ ui.NOTIFY_ID, "slider-weight-1", "setValue", "Weight", ui.NOTIFY_VALUE },
										{ ui.NOTIFY_ID, "slider-knob", "setValue", "Text", ui.NOTIFY_FORMAT, "$%05x" },
									},
								},
							},
						},
						ui.text:new
						{
							Mode = "button",
							Style = "button",
							Text = "Reset",
							Width = "auto",
							Notifications =
							{
								["Pressed"] =
								{
									[false] =
									{
										{
											ui.NOTIFY_SELF, ui.NOTIFY_FUNCTION, function(self)
												local e = self.Application:getElementById("slider-weight-1")
												self.Application:getElementById("slider-2"):reset()
												e:setValue("Weight", false)
											end
										}
									}
								}
							}
						},
					},
				},
				ui.group:new
				{
					Children =
					{
						ui.text:new { Id="slider-weight-1", Text = " $08000 ", FontSpec="utopia:60", KeepMinWidth = true },
						ui.frame:new { Height = "fill" },
					},
				},
			},
		},
		ui.window:new
		{
			Title = "Border Thickness",
			Legend = "Border Thickness",
			Children =
			{
				ui.text:new
				{
					Mode = "button",
					Style = "button",
					Width = "auto",
					Id = "border-button",
					Text = "Watch borders",
				},
				ui.Slider:new
				{
					Width = "free",
					Min = 0,
					Max = 20,
					ForceInteger = true,
					Notifications =
					{
						["Value"] =
						{
							[ui.NOTIFY_CHANGE] =
							{
								{
									ui.NOTIFY_SELF, ui.NOTIFY_FUNCTION, function(self, val)
										local e = self.Application:getElementById("border-button")
										e.Border = { val, val, val, val }
										e:rethinkLayout(true)
										self.Border = { val, val, val, val }
										self:rethinkLayout(true)
									end, ui.NOTIFY_VALUE
								}
							}
						}
					},
				},
			},
		},
	},
}:run()

