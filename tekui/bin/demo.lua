#!/usr/bin/env lua

local lfs = require "lfs"
local List = require "tek.class.list"
local ui = require "tek.ui"
local db = require "tek.lib.debug"

function lfs.readdir(path)
	local dir = lfs.dir(path)
	return function()
		local e
		repeat
			e = dir()
		until e ~= "." and e ~= ".."
		return e
	end
end

-- -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
--	Load demos and insert them to the application:
-- -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

function loaddemos(app)

	local demogroup = app:getElementById("demo-group")

	local demos = { }

	for fname in lfs.readdir(ui.ProgDir .. "demos/") do
		fname = ui.ProgDir .. "demos/" .. fname
		db.info("Loading demo '%s' ...", fname)
		local success, res = pcall(dofile, fname)
		if success then
			table.insert(demos, res)
		else
			db.error("*** Error loading demo '%s'", fname)
			db.error(res)
		end
	end

	table.sort(demos, function(a, b) return a.Name < b.Name end)

	for _, demo in ipairs(demos) do
		ui.Application.connect(demo.Window)
		app:addMember(demo.Window)
		local button = ui.Text:new
		{
			Id = demo.Window.Id .. "-button",
			Text = demo.Name,
			Mode = "toggle",
			Width = "free",
			Padding = { 10, 4, 10, 4 },
			Style = "button",
			UserData = demo.Window
		}
		button:addNotify("Selected", true, { ui.NOTIFY_ID, "info-text", "setValue", "Text", demo.Description })
		button:addNotify("Selected", true, { ui.NOTIFY_ID, demo.Window.Id, "setValue", "Status", "show" })
		button:addNotify("Selected", false, { ui.NOTIFY_ID, demo.Window.Id, "setValue", "Status", "hide" })
		ui.Application.connect(button)
		demogroup:addMember(button)
	end

end

-- -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
--	Application:
-- -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

app = ui.Application:new
{
	ProgramName = "tekUI Demo",
	Author = "Timm S. Müller",
	CopyRight = "Copyright © 2008, Schulze-Müller GbR",

	Children =
	{
		ui.Window:new
		{
			Width = 400,
			Height = 500,
			UserData =
			{
				MemRefreshTickCount = 0,
				MemRefreshTickInit = 25,
				MinMem = false,
				MaxMem = false,
				IntervalNotify = { ui.NOTIFY_SELF, ui.NOTIFY_FUNCTION, function(self)
					local data = self.UserData
					data.MemRefreshTickCount = data.MemRefreshTickCount - 1
					if data.MemRefreshTickCount <= 0 then
						data.MemRefreshTickCount = data.MemRefreshTickInit
						local m = collectgarbage("count")
						data.MinMem = math.min(data.MinMem or m, m)
						data.MaxMem = math.max(data.MaxMem or m, m)
						self.Application:getElementById("about-mem-used"):setValue("Text", ("%dk - min: %dk - max: %dk"):format(m, data.MinMem, data.MaxMem))
						local gauge = self.Application:getElementById("about-mem-gauge")
						gauge:setValue("Min", data.MinMem)
						gauge:setValue("Max", data.MaxMem)
						gauge:setValue("Value", m)
					end
				end },
			},

			Center = true,
			Orientation = "vertical",
			Id = "about-window",
			Status = "hide",
			Title = "About tekUI",
			Notifications =
			{
				["Status"] =
				{
					["opening"] =
					{
						{ ui.NOTIFY_SELF, ui.NOTIFY_FUNCTION, function(self)
							self.Window:addNotify("Interval", ui.NOTIFY_ALWAYS, self.UserData.IntervalNotify)
						end },
						{ ui.NOTIFY_ID, "about-mem-refresh", "setValue", "Pressed", false },
					},
					["closing"] =
					{
						{ ui.NOTIFY_SELF, ui.NOTIFY_FUNCTION, function(self)
							self.Window:remNotify("Interval", ui.NOTIFY_ALWAYS, self.UserData.IntervalNotify)
						end },
					},

					["show"] =
					{
						{ ui.NOTIFY_ID, "about-button", "setValue", "Selected", true },
					},
					["hide"] =
					{
						{ ui.NOTIFY_ID, "about-button", "setValue", "Selected", false }
					},
				},
			},
			Children =
			{
				ui.Text:new { FontSpec = "__large", Text = "About tekUI" },
				ui.PageGroup:new
				{
					PageCaptions = { "_Application", "_License", "_System" },
					PageNumber = 3,
					Children =
					{
						ui.Group:new
						{
							Orientation = "vertical",
							Children =
							{
								ui.Group:new
								{
									Legend = "Application Information",
									Children =
									{
										ui.ListView:new
										{
											Headers = { "Property", "Value" },
											Child = ui.ListGadget:new
											{
												SelectMode = "none",
												ListObject = List:new
												{
													Items =
													{
														{ { "ProgramName", "tekUI Demo" } },
														{ { "Version", "1.0" } },
														{ { "Author", "Timm S. Müller" } },
														{ { "Copyright", "© 2008, Schulze-Müller GbR" } },
													}
												}
											}
										}
									}
								}
							}
						},
						ui.Group:new
						{
							Orientation = "vertical",
							Children =
							{
								ui.PageGroup:new
								{
									PageCaptions = { "tekUI", "Lua", "Disclaimer" },
									Children =
									{
										ui.ScrollGroup:new
										{
											Legend = "tekUI License",
											VSliderMode = "on",
											Canvas = ui.Canvas:new
											{
												KeepMinHeight = true,
												AutoWidth = true,
												Child = ui.FloatText:new
												{
													Text = [[

														Copyright © 2008 by the authors:

														Timm S. Müller <tmueller@schulze-mueller.de>
														Franciska Schulze <fschulze@schulze-mueller.de>

														Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

														The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

													]]
												}
											}
										},
										ui.ScrollGroup:new
										{
											Legend = "Lua license",
											VSliderMode = "on",
											Canvas = ui.Canvas:new
											{
												KeepMinHeight = true,
												AutoWidth = true,
												Child = ui.FloatText:new
												{
													Text = [[

														Copyright © 1994-2007 Lua.org, PUC-Rio.

														Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

														The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

													]]
												}
											}
										},
										ui.ScrollGroup:new
										{
											Legend = "Disclaimer",
											VSliderMode = "on",
											Canvas = ui.Canvas:new
											{
												KeepMinHeight = true,
												AutoWidth = true,
												Child = ui.FloatText:new
												{
													Text = [[

														THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

													]]
												}
											}
										}
									}
								}
							}
						},
						ui.Group:new
						{
							Orientation = "vertical",
							Children =
							{
								ui.Group:new
								{
									Legend = "System Information",
									Orientation = "vertical",
									Children =
									{
										ui.ScrollGroup:new
										{
											VSliderMode = "on",
											Canvas = ui.Canvas:new
											{
												AutoWidth = true,
												Child = ui.FloatText:new
												{
													Text = [[
														Interpreter Version: ]] .. _VERSION .. [[
													]]
												}
											}
										},
										ui.Group:new
										{
											Legend = "Lua VM",
											GridWidth = 2,
											Children =
											{
												ui.Text:new { Text = "Memory usage:", TextHAlign = "right", Style = "caption", Width = "fill" },
												ui.Group:new
												{
													Orientation = "vertical",
													Children =
													{
														ui.Group:new
														{
															Children =
															{
																ui.Text:new { Id = "about-mem-used" },
																ui.Text:new { Width = "auto", Text = "_Reset", Style = "button", Mode = "button",
																	Notifications =
																	{
																		["Pressed"] =
																		{
																			[false] =
																			{
																				{ ui.NOTIFY_WINDOW, ui.NOTIFY_FUNCTION, function(self)
																					self.UserData.MinMem = false
																					self.UserData.MaxMem = false
																				end }
																			}
																		}
																	}
																},
															}
														},
														ui.Gauge:new { Id = "about-mem-gauge" },
													}
												}
											}
										},

										ui.Group:new
										{
											Legend = "Debugging",
											GridWidth = 2,
											Children =
											{
												ui.Text:new { Text = "Debug Level:", Width = "fill", TextHAlign = "right", Style = "caption" },
												ui.ScrollBar:new
												{
													ArrowOrientation = "vertical",
													Width = "free",
													Min = 1,
													Max = 20,
													Value = db.level,
													Child = ui.Text:new
													{
														Id = "about-system-debuglevel",
														FontSpec = "__small",
														Style = "knob",
														Text = tostring(db.level)
													},
													Notifications =
													{
														["Value"] =
														{
															[ui.NOTIFY_CHANGE] =
															{
																{ ui.NOTIFY_ID, "about-system-debuglevel", "setValue", "Text", ui.NOTIFY_FORMAT, "%d" },
																{ ui.NOTIFY_SELF, ui.NOTIFY_FUNCTION, function(self, value)
																	db.level = math.floor(value)
																end, ui.NOTIFY_VALUE }
															}
														}
													}
												},

												ui.Text:new { Text = "Debug Options:", Width = "fill", TextHAlign = "right", Style = "caption" },
												ui.CheckMark:new
												{
													Selected = ui.DEBUG,
													Text = "Slow Rendering",
													Notifications =
													{
														["Selected"] =
														{
															[ui.NOTIFY_CHANGE] =
															{
																{ ui.NOTIFY_SELF, ui.NOTIFY_FUNCTION, function(self, value)
																	ui.DEBUG = value
																	ui.Drawable.enableDebug(value)
																end, ui.NOTIFY_VALUE }
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				},
				ui.Text:new
				{
					Focus = true,
					Mode = "button",
					Style = "button",
					Text = "_Okay",
					Width = "fill",
					Notifications =
					{
						["Pressed"] =
						{
							[false] =
							{
								{ ui.NOTIFY_WINDOW, "setValue", "Status", "hide" }
							}
						}
					}
				}
			}
		},
		ui.Window:new
		{
			Orientation = "vertical",
			Notifications =
			{
				["Status"] =
				{
					["hide"] =
					{
						{ ui.NOTIFY_APPLICATION, "setValue", "Status", "quit" }
					},
				},
			},
			MaxWidth = ui.HUGE,
			MaxHeight = ui.HUGE,
			Children =
			{
				ui.Group:new
				{
					Style = "menubar",
					Children =
					{
						ui.MenuItem:new
						{
							Text = "_File",
							Children =
							{
								ui.MenuItem:new
								{
									Text = "_About...",
									Shortcut = "Ctrl+?",
									Notifications =
									{
										["Pressed"] =
										{
											[false] =
											{
												{ ui.NOTIFY_ID, "about-window", "setValue", "Status", "show" }
											},
										},
									},
								},
								ui.Spacer:new { },
								ui.MenuItem:new
								{
									Text = "_Quit",
									Shortcut = "Ctrl+Q",
									Notifications =
									{
										["Pressed"] =
										{
											[false] =
											{
												{
													ui.NOTIFY_APPLICATION, "setValue", "Status", "quit"
												}
											},
										},
									},
								},
							},
						},
					},
				},
				ui.Text:new { FontSpec = "__large", Text = "tekUI demo" },
				ui.Group:new
				{
					Children =
					{
						ui.ScrollGroup:new
						{
							Weight = 0,
							Legend = "Available Demos",
							MaxWidth = ui.HUGE,
							VSliderMode = "on",
							Canvas = ui.Canvas:new
							{
								KeepMinWidth = true,
								AutoWidth = true,
								AutoHeight = true,
								Child = ui.Group:new
								{
									Id = "demo-group",
									MaxWidth = ui.HUGE,
									Orientation = "vertical",
								},
							},
						},
						ui.Handle:new { },
						ui.ScrollGroup:new
						{
							Weight = 0x10000,
							Legend = "Comment",
							VSliderMode = "on",
							Canvas = ui.Canvas:new
							{
								AutoWidth = true,
								Child = ui.FloatText:new
								{
									Id = "info-text",
									Text = [[
										Welcome to the tekUI Demo.

										In the list to the left you find some examples to demonstrate the abilities of the tekUI toolkit.

										Please note that the whole demo is written as a single constructor; thanks to the inbuilt notification system, no functions are needed to perform simple interconnections. ]]
								}
							}
						},
					},
				},
			},
		},


	}
}

loaddemos(app)

-- -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
--	run application:
-- -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

app:run()

