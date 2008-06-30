#!/usr/bin/env lua

ui = require "tek.ui"

ui.Application:new
{
	Children =
	{
		ui.Window:new
		{
			Title = "Hello",
			Children =
			{
				ui.Text:new
				{
					Text = "_Hello, World!",
					Style = "button",
					Mode = "button",
					Notifications =
					{
						["Pressed"] =
						{
							[false] =
							{
								{ ui.NOTIFY_SELF, ui.NOTIFY_FUNCTION,
									function(self)
										print "Hello, World!"
									end
								},
							},
						},
					},
				},
			},
		},
	},
}:run()
