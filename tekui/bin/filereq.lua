#!/usr/bin/env lua

local ui = require "tek.ui"

app = ui.Application:new
{
	Children =
	{
		ui.Window:new
		{
			Title = "File Request",
			Orientation = "vertical",
			Children =
			{
				ui.Group:new
				{
					GridWidth = 2,
					SameHeight = true,
					Children =
					{
						ui.Text:new
						{
							Text = "_Path:",
							Width = "auto",
							Style = "caption",
							HAlign = "right",
						},
						ui.TextInput:new
						{
							Id = "pathfield",
							Text = "/home",
							KeyCode = "p",
						},
						ui.Text:new
						{
							Text = "Selected:",
							Width = "auto",
							Style = "caption",
							HAlign = "right",
						},
						ui.TextInput:new
						{
							Id = "filefield",
						},
						ui.Text:new
						{
							Text = "Status:",
							Width = "auto",
							Style = "caption",
							HAlign = "right",
						},
						ui.Text:new
						{
							Id = "statusfield",
							TextHAlign = "left",
						},
						ui.Text:new
						{
							Text = "_Multiselect:",
							Width = "auto",
							Style = "caption",
							HAlign = "right",
						},
						ui.CheckMark:new
						{
							Id = "multiselect",
							KeyCode = "m",
						},
					}
				},
				ui.Text:new
				{
					Text = "_Choose File...",
					Style = "button",
					Mode = "button",
					Width = "auto",
					HAlign = "right",
					Notifications =
					{
						["Pressed"] =
						{
							[false] =
							{
								{ ui.NOTIFY_APPLICATION, ui.NOTIFY_COROUTINE, function(self)
									local pathfield = self:getElementById("pathfield")
									local filefield = self:getElementById("filefield")
									local statusfield = self:getElementById("statusfield")
									local status, path, select = self:requestFile
									{
										Path = pathfield.Text,
										SelectMode = self:getElementById("multiselect").Selected and "multi" or "single"
									}
									statusfield:setValue("Text", status)
									if status == "selected" then
										pathfield:setValue("Text", path)
										self:getElementById("filefield"):setValue("Text", table.concat(select, ", "))
									end
								end }
							}
						}
					}
				}
			}
		}
	}
}:run()
