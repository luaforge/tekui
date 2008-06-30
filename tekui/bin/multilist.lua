#!/usr/bin/env lua

local ui = require "tek.ui"
local List = require "tek.class.list"

ui.Application:new
{
	Children =
	{
		ui.Window:new
		{
			Children =
			{
				ui.Group:new
				{
					Orientation = "vertical",
					Children =
					{
						ui.ListView:new
						{
							Headers = { "Band", "Album", "Country", "Year", "Genre" },
							Child = ui.ListGadget:new
							{
								Id = "the-list",
								SelectMode = "multi",
								ListObject = List:new
								{
									Items =
									{
										{ { "Adramelech", "The Fall", "Finland", "1995", "Death Metal" } },
										{ { "Blood", "O Agios Pethane", "Germany", "1993", "Grindcore" } },
										{ { "Cathedral", "Forest of Equilibrium", "United Kingdom", "1991", "Doom Metal" } },
										{ { "Deeds of Flesh", "Mark of the Legion", "United States", "2001", "Death Metal" } },
										{ { "Entombed", "Clandestine", "Sweden", "1991", "Death Metal" } },
										{ { "Forest", "As a Song in the Harvest of Grief", "Russia", "1999", "Black Metal" } },
										{ { "Gorguts", "Considered Dead", "Canada", "1991", "Death Metal" } },
										{ { "Hellhammer", "Apocalyptic Raids", "Switzerland", "1985", "Black Metal" } },
										{ { "Immortal", "Battles in the North", "Norway", "1995", "Black Metal" } },
										{ { "Judas Iscariot", "Of Great Eternity", "United States", "1997", "Black Metal" } },
										{ { "Kampfar", "Mellom Skogkledde Aaser", "Norway", "1997", "Viking Metal" } },
										{ { "Limbonic Art", "Moon in the Scorpio", "Norway", "1996", "Black Metal" } },
										{ { "Massacra", "Enjoy the Violence", "France", "1991", "Death Metal" } },
										{ { "Negura Bunget", "'n Crugu Bradului", "Romania", "2002", "Black Metal" } },
										{ { "Oppressor", "Elements of Corrosion", "United States", "1998", "Death Metal" } },
										{ { "Pentagram", "Pentagram", "Chile", "2000", "Death Metal" } },
										{ { "Rotting Christ", "Thy Mighty Contract", "Greece", "1993", "Black Metal" } },
										{ { "Sepultura", "Beneath the Remains", "Brazil", "1989", "Death Metal" } },
										{ { "Throne of Ahaz", "Nifelheim", "Sweden", "1995", "Black Metal" } },
										{ { "Unleashed", "Where no Life Dwells", "Sweden", "1991", "Death Metal" } },
										{ { "Vader", "De Profundis", "Poland", "1995", "Death Metal" } },
										{ { "Watain", "Rabid Death's Curse", "Sweden", "2001", "Black Metal" } },
										{ { "Xibalba", "Ah Dzam Poop Ek", "Mexico", "1994", "Black Metal" } },
									}
								},
								Notifications =
								{
									["CursorLine"] =
									{
										[ui.NOTIFY_ALWAYS] =
										{
											{
												ui.NOTIFY_SELF, ui.NOTIFY_FUNCTION, function(self, lnr)
													local input = self.Application:getElementById("the-input")
													local line = self:getItem(lnr)
													input:setValue("Text", line and line[1][1] or "")
												end, ui.NOTIFY_VALUE
											}
										}
									},
									["DblClick"] =
									{
										[true] =
										{
											{
												ui.NOTIFY_SELF, ui.NOTIFY_FUNCTION, function(self)
													print "Doubleclick"
												end,
											}
										}
									},
								}
							}
						},
						ui.TextInput:new
						{
							Id = "the-input",
							Notifications =
							{
								["Enter"] =
								{
									[ui.NOTIFY_ALWAYS] =
									{
										{ ui.NOTIFY_ID, "the-list", "changeItem", ui.NOTIFY_VALUE,
											ui.NOTIFY_ID, "the-list", "CursorLine", ui.NOTIFY_GETFIELD },
										{ ui.NOTIFY_ID, "new-button", "setValue", "Focus", true },
									}
								}
							}
						},
					}
				},

				ui.Group:new
				{
					Orientation = "vertical",
					Width = "auto",
					SameSize = true,
					Children =
					{
						ui.Text:new { Mode = "button", Style = "button", Id = "new-button", Text = "_New",
							Notifications =
							{
								["Pressed"] =
								{
									[false] =
									{
										{ ui.NOTIFY_SELF, ui.NOTIFY_FUNCTION, function(self)
											local list = self.Application:getElementById("the-list")
											local input = self.Application:getElementById("the-input")
											list:addItem("")
											list:setValue("CursorLine", list:getN())
											input:setValue("Focus", true)
										end }
									}
								}
							}
						},
						ui.Text:new { Mode = "button", Style = "button", Id = "insert-button", Text = "_Insert",
							Notifications =
							{
								["Pressed"] =
								{
									[false] =
									{
										{ ui.NOTIFY_SELF, ui.NOTIFY_FUNCTION, function(self)
											local list = self.Application:getElementById("the-list")
											local cl = list.CursorLine
											if cl > 0 then
												local input = self.Application:getElementById("the-input")
												list:addItem("", list.CursorLine)
												input:setValue("Text", "")
												input:setValue("Focus", true)
											end
										end }
									}
								}
							}
						},
						ui.Text:new { Mode = "button", Style = "button", Text = "D_elete",
							Notifications =
							{
								["Pressed"] =
								{
									[false] =
									{
										{
											ui.NOTIFY_ID, "the-list", ui.NOTIFY_FUNCTION, function(self)
												local sl = self.SelectedLines
												-- delete from last to first.
												local t = { }
												for lnr in pairs(self.SelectedLines) do
													table.insert(t, lnr)
												end
												if #t > 0 then
													table.sort(t, function(a, b) return a > b end)
													local cl = t[#t]
													for _, lnr in ipairs(t) do
														self:remItem(lnr)
													end
													cl = math.min(self:getN(), cl)
													self:setValue("SelectedLine", cl)
													self:setValue("CursorLine", cl)
												end
											end, ui.NOTIFY_VALUE
										}
									}
								}
							}
						},
						ui.Text:new { Mode = "button", Style = "button", Text = "_Up",
							Notifications =
							{
								["Pressed"] =
								{
									[false] =
									{
										{
											ui.NOTIFY_ID, "the-list", ui.NOTIFY_FUNCTION, function(self)
												local cl = self.CursorLine
												local entry = self:remItem(cl)
												if entry then
													self:addItem(entry, cl - 1)
													self:setValue("CursorLine", math.max(1, cl - 1))
												end
											end, ui.NOTIFY_VALUE
										}
									}
								}
							}
						},
						ui.Text:new { Mode = "button", Style = "button", Text = "_Down",
							Notifications =
							{
								["Pressed"] =
								{
									[false] =
									{
										{
											ui.NOTIFY_ID, "the-list", ui.NOTIFY_FUNCTION, function(self)
												local cl = self.CursorLine
												local entry = self:remItem(cl)
												if entry then
													self:addItem(entry, cl + 1)
													self:setValue("CursorLine", cl + 1)
												end
											end, ui.NOTIFY_VALUE
										}
									}
								}
							}
						}
					}
				}
			}
		},
	}
}:run()
