#!/usr/bin/env lua

local List = require "tek.class.list"
local ui = require "tek.ui"

ui.Application:new
{
	Children =
	{
		ui.Window:new
		{
			Orientation = "vertical",
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
								ui.MenuItem:new { Text = "New" },
								ui.Spacer:new { },
								ui.MenuItem:new { Text = "Open..." },
								ui.MenuItem:new { Text = "Open Recent" },
								ui.MenuItem:new { Text = "Open With",
									Children =
									{
										ui.MenuItem:new { Text = "Lua" },
										ui.MenuItem:new { Text = "KWrite" },
										ui.MenuItem:new { Text = "Other..." },
									},
								},
								ui.MenuItem:new
								{
									Text = "_Bla",
									Children =
									{
										ui.MenuItem:new { Text = "Bla" },
										ui.MenuItem:new { Text = "Bl_ub" },
										ui.MenuItem:new { Text = "Fomp" },
									},
								},
								ui.Spacer:new { },
								ui.MenuItem:new { Text = "Save..." },
								ui.MenuItem:new { Text = "Save as" },
								ui.Spacer:new { },
								ui.MenuItem:new { Text = "_Reload" },
								ui.MenuItem:new { Text = "Print" },
								ui.Spacer:new { },
								ui.MenuItem:new { Text = "Close" },
								ui.MenuItem:new { Text = "Close all", Shortcut = "Shift+Ctrl+Q" },
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
													ui.NOTIFY_SELF, ui.NOTIFY_FUNCTION, function(self)
														self.Application:setValue("Status", "quit")
													end
												}
											}
										}
									},
								},
							},
						},
						ui.MenuItem:new
						{
							Text = "Edit",
							Children =
							{
								ui.MenuItem:new { Text = "Undo" },
								ui.MenuItem:new { Text = "Redo", Disabled = true },
								ui.Spacer:new { },
								ui.MenuItem:new { Text = "Cut" },
								ui.MenuItem:new { Text = "Copy" },
								ui.MenuItem:new { Text = "Paste" },
								ui.Spacer:new { },
								ui.MenuItem:new { Text = "Select all" },
								ui.MenuItem:new { Text = "Deselect", Disabled = true },
							},
						},
					},
				},

				ui.Group:new
				{
					Children =
					{
						ui.PopItem:new
						{
							Text = "_PopItem",
							-- these children are not connected:
							Children =
							{
								ui.popitem:new
								{
									Text = "_Langer text",
									Children =
									{
										ui.popitem:new { Text = "Eins" },
										ui.popitem:new { Text = "Zwei" },
										ui.popitem:new { Text = "Drei" },
									},
								},
								ui.popitem:new
								{
									Text = "_Bar",
									Children =
									{
										ui.popitem:new { Text = "Hallo" },
										ui.popitem:new
										{
											Text = "_Außerordentlich langer Text",
											Children =
											{
												ui.popitem:new { Text = "Eins" },
												ui.spacer:new { },
												ui.popitem:new { Text = "_Zwei" },
												ui.popitem:new { Text = "Drei" },
											}
										},
										ui.popitem:new
										{
											Text = "EXIT",
											Notifications =
											{
												["Pressed"] =
												{
													[false] =
													{
														{
															ui.NOTIFY_SELF, ui.NOTIFY_FUNCTION, function(self)
																self.Application:setValue("Status", "quit")
															end
														}
													}
												}
											}
										},
									},
								},
							},
						},
						ui.PopList:new
						{
							Text = "Combo Box",
							ListObject = List:new
							{
								Items =
								{
									{ { "Uno - Ichi - One" } },
									{ { "Dos - Ni - Two" } },
									{ { "Tres - San - Three" } },
									{ { "Cuatro - Yon - Four" } },
									{ { "Cinco - Go - Five" } },
									{ { "Seis - Roku - Six" } },
								},
							},
							Notifications =
							{
								["SelectedEntry"] =
								{
									[ui.NOTIFY_CHANGE] =
									{
										{
											ui.NOTIFY_SELF,
											ui.NOTIFY_FUNCTION, function(self)
												local id = self.Application:getElementById("display")
												local item = self.ListObject:getItem(self.SelectedEntry)
												id:setValue("Text", item[1][1])
											end
										}
									}
								}
							}
						}
					}
				},
				ui.Text:new { Id = "display", Text = "Popup Tests", Height = "free", FontSpec = "__huge" },
			},
		},
	},
}:run()
