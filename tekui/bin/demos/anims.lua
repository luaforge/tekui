#!/usr/bin/env lua

local ui = require "tek.ui"

local window = ui.Window:new
{
	Orientation = "vertical",
	Width = 400,
	Height = 400,
	Id = "anims-window",
	Title = "Animations",
	Status = "hide",
	Notifications =
	{
		["Status"] =
		{
			["show"] =
			{
				{ ui.NOTIFY_ID, "anims-window-button", "setValue", "Selected", true }
			},
			["hide"] =
			{
				{ ui.NOTIFY_ID, "anims-window-button", "setValue", "Selected", false }
			},
		},
	},
	Children =
	{
		ui.PageGroup:new
		{
			PageCaptions = { "_Tunnel", "_Boing", "_Plasma" },
			Children =
			{
				ui.Group:new
				{
					Orientation = "vertical",
					Children =
					{
						ui.Tunnel:new { Id = "the-tunnel", HAlign = "center" },
						ui.Group:new
						{
							Width = "fill",
							Height = "auto",
							Legend = "Parameters",
							GridWidth = 2,
							Children =
							{
								ui.text:new { Text = "Speed", Style = "caption", Width = "fill" },
								ui.Slider:new {
									Width = "free",
									Min = 1,
									Max = 19,
									Value = 13,
									Range = 20,
									Step = 1,
									Notifications = {
										["Value"] = {
											[ui.NOTIFY_CHANGE] = {
												{ ui.NOTIFY_ID, "the-tunnel", "setSpeed", ui.NOTIFY_VALUE },
											},
										}
									},
								},
								ui.text:new { Text = "Focus", Style = "caption", Width = "fill" },
								ui.Slider:new {
									Width = "free",
									Min = 0x10,
									Max = 0x1ff,
									Value = 0x50,
									Range = 0x200,
									Step = 20,
									Notifications = {
										["Value"] = {
											[ui.NOTIFY_CHANGE] = {
												{ ui.NOTIFY_ID, "the-tunnel", "setViewZ", ui.NOTIFY_VALUE },
											},
										}
									},
								},
								ui.text:new { Text = "Segments", Style = "caption", Width = "fill" },
								ui.Slider:new {
									Width = "free",
									Min = 1,
									Max = 19,
									Value = 6,
									Range = 20,
									Step = 1,
									Notifications = {
										["Value"] = {
											[ui.NOTIFY_CHANGE] = {
												{ ui.NOTIFY_ID, "the-tunnel", "setNumSeg", ui.NOTIFY_VALUE },
											},
										}
									},
								},
							},
						},
					}
				},
				ui.Group:new
				{
					Orientation = "vertical",
					Children =
					{
						ui.Boing:new { Id = "the-boing" },
						ui.Group:new
						{
							Height = "auto",
							Children =
							{
								ui.text:new
								{
									Mode = "button",
									Style = "button",
									Text = "_Start",
									Notifications =
									{
										["Pressed"] =
										{
											[false] =
											{
												{
													ui.NOTIFY_ID, "the-boing", "setValue", "Running", true
												}
											}
										}
									}
								},
								ui.text:new
								{
									Mode = "button",
									Style = "button",
									Text = "Sto_p",
									Notifications =
									{
										["Pressed"] =
										{
											[false] =
											{
												{
													ui.NOTIFY_ID, "the-boing", "setValue", "Running", false
												}
											}
										}
									}
								},
							},
						},
					}
				},
				ui.Plasma:new { },
			},
		}
	}
}

if ui.ProgName == "anims.lua" then
	local app = ui.Application:new()
	ui.Application.connect(window)
	app:addMember(window)
	window:setValue("Status", "show")
	app:run()
else
	return
	{
		Window = window,
		Name = "Animations",
		Description = [[
			This demo shows three different animated classes.

			The 'Boing' class registers its interval handler as soon as the window is opened, so when you switch away and back to its page, you will notice that the spot's position was udpated even while its page was invisible.

			The animation classes are not part of tekUI's system-wide installation; instead, they are loaded from the directory in which the application resides.

		]]
	}
end
