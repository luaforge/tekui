#!/usr/bin/env lua

local ui = require "tek.ui"

ui.Application:new
{
	Children =
	{
		ui.Window:new
		{
			Title = "Groups Demo",
			Children =
			{
				ui.ScrollGroup:new
				{
					Legend = "Virtual Group",
					Width = 500,
					Height = 500,
					HSliderMode = "on",
					VSliderMode = "on",
					Canvas = ui.Canvas:new
					{
						MaxWidth = 500,
						MaxHeight = 500,
						CanvasWidth = 500,
						CanvasHeight = 500,
						Child = ui.Group:new
						{
							GridWidth = 2,
							Children =
							{
								ui.text:new { Mode = "button", Style = "button", Width = "free", Height = "free", Text = "foo" },
								ui.text:new { Mode = "button", Style = "button", Width = "free", Height = "free", Text = "foo" },
								ui.text:new { Mode = "button", Style = "button", Width = "free", Height = "free", Text = "foo" },
								ui.ScrollGroup:new
								{
									Legend = "Virtual Group",
									Width = 500,
									Height = 500,
									HSliderMode = "on",
									VSliderMode = "on",
									Canvas = ui.Canvas:new
									{
										CanvasWidth = 500,
										CanvasHeight = 500,
										Child = ui.Group:new
										{
											GridWidth = 2,
											Children =
											{
												ui.text:new { Mode = "button", Style = "button", Width = "free", Height = "free", Text = "foo" },
												ui.text:new { Mode = "button", Style = "button", Width = "free", Height = "free", Text = "foo" },
												ui.text:new { Mode = "button", Style = "button", Width = "free", Height = "free", Text = "foo" },
												ui.ScrollGroup:new
												{
													Legend = "Virtual Group",
													Width = 500,
													Height = 500,
													HSliderMode = "on",
													VSliderMode = "on",
													Canvas = ui.Canvas:new
													{
														CanvasWidth = 500,
														CanvasHeight = 500,
														Child = ui.Group:new
														{
															GridWidth = 2,
															Children =
															{
																ui.text:new { Mode = "button", Style = "button", Width = "free", Height = "free", Text = "foo" },
																ui.text:new { Mode = "button", Style = "button", Width = "free", Height = "free", Text = "foo" },
																ui.text:new { Mode = "button", Style = "button", Width = "free", Height = "free", Text = "foo" },
																ui.text:new { Mode = "button", Style = "button", Width = "free", Height = "free", Text = "foo" },
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
				}
			}
		}
	}
}:run()
