#!/usr/bin/env lua

local ui = require "tek.ui"

ui.Application:new
{
	Title = "FloatText Demo",
	Children =
	{
		ui.Window:new
		{
			Orientation = "vertical",
			Children =
			{
				ui.Text:new
				{
					Text = "Poems",
					FontSpec = "__huge",
				},
				ui.Group:new
				{
					Children =
					{
						ui.Canvas:new
						{
							CanvasWidth = 500,
							CanvasHeight = 500,
							Child = ui.FloatText:new
							{
								BGPen = ui.PEN_SHINE,
								FGPen = ui.PEN_SHADOW,
								Text = [[

									Ecce homo
									Friedrich Nietzsche (1844-1900)

									Ja! Ich weiß, woher ich stamme!
									Ungesättigt gleich der Flamme
									Glühe und verzehr' ich mich.
									Licht wird alles, was ich fasse,
									Kohle alles, was ich lasse:
									Flamme bin ich sicherlich
								]]
							}
						},
						ui.Handle:new { },
						ui.ScrollGroup:new
						{
							HSliderMode = "off",
							VSliderMode = "on",
							Canvas = ui.Canvas:new
							{
								AutoWidth = true,
								Child = ui.FloatText:new
								{
									BGPen = ui.PEN_SHINE,
									FGPen = ui.PEN_SHADOW,
									Text = [[

										Under der linden
										Walther von der Vogelweide (1170-1230)

										Under der linden
										an der heide,
										dâ unser zweier bette was,
										dâ mugent ir vinden
										schône beide
										gebrochen bluomen unde gras.
										vor dem walde in einem tal,
										tandaradei,
										schône sanc diu nahtegal.

										Ich kam gegangen
										zuo der ouwe:
										dô was mîn friedel komen ê.
										dâ wart ich enpfangen
										hêre frouwe,
										daz ich bin sælic iemer mê.
										kuster mich? wol tûsentstunt:
										tandaradei,
										seht wie rôt mir ist der munt.

										Dô hete er gemachet
										alsô rîche
										von bluomen eine bettestat.
										des wirt noch gelachet
										inneclîche,
										kumt iemen an daz selbe pfat.
										bî den rôsen er wol mac,
										tandaradei,
										merken wâ mirz houbet lac.

										Daz er bî mir læge,
										wesse ez iemen
										(nu enwelle got!), sô schamte ich mich.
										wes er mit mir pflæge,
										niemer niemen
										bevinde daz, wan er unt ich,
										und ein kleinez vogellîn:
										tandaradei,
										daz mac wol getriuwe sîn.
									]]
								}
							}
						}
					}
				},
				ui.Text:new
				{
					Text = "_Okay",
					Mode = "button",
					Style = "button",
					Notifications =
					{
						["Pressed"] =
						{
							[false] =
							{
								{
									ui.NOTIFY_APPLICATION, "setValue", "Status", "quit"
								}
							}
						}
					}
				}
			}
		}
	}
}:run()
