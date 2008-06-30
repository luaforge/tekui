-------------------------------------------------------------------------------
--
--	tek.ui.class.dirlist
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	LINEAGE::
--		[[#ClassOverview]] :
--		[[#tek.class : Class]] /
--		[[#tek.class.object : Object]] /
--		[[#tek.ui.class.element : Element]] /
--		[[#tek.ui.class.area : Area]] /
--		[[#tek.ui.class.frame : Frame]] /
--		[[#tek.ui.class.gadget : Gadget]] /
--		[[#tek.ui.class.group : Group]] /
--		DirList
--
--	OVERVIEW::
--		This class implements a directory lister.
--
--	ATTRIBUTES::
--		- {{Path [IG]}} (string)
--			Directory in the file system
--		- {{Location [IG]}} (string)
--			The entry currently selected, may be a file or directory
--		- {{Selection [G]}} (table)
--			An array of selected entries
--		- {{Status [G]}} (string)
--			Status of the lister:
--				- "running" - the lister is currently being shown
--				- "selected" - the user has selected one or more entries
--				- "cancelled" - the lister has been cancelled by the user
--		- {{Style [IG]}} (string)
--			The visual appearance or purpose of the lister, which will
--			determine the arrangement of some interface elements:
--				- "requester" - for being used as a standalone file requester
--				- "lister" - for being used as a lister component
--				- "simple" - without accompanying user interface elements
--		- {{SelectMode [IG]}} (String)
--			Mode of selection:
--				- "single" - allows selections of one entry at a time
--				- "multi" - allows selections of multiple entries
--
--	IMPLEMENTS::
--		- DirList:ShowDirectory() - Reads and shows a directory
--		- DirList:goParent() - Goes to the parent of the current directory
--		- DirList:abortScan() - Abort scanning
--
--	OVERRIDES::
--		- Class.new()
--
-------------------------------------------------------------------------------

local lfs = require "lfs"
local db = require "tek.lib.debug"
local ui = require "tek.ui"
local List = require "tek.class.list"

local Group = ui.Group
local ListGadget = ui.ListGadget
local ListView = ui.ListView
local Text = ui.Text
local TextInput = ui.TextInput

local dir = lfs.dir
local insert = table.insert
local pairs = pairs
local pcall = pcall
local sort = table.sort
local stat = lfs.attributes

module("tek.ui.class.dirlist", tek.ui.class.group)
_VERSION = "DirList 3.2"

local DirList = _M

-------------------------------------------------------------------------------
--	readDir: returns an iterator over entries in a directory
-------------------------------------------------------------------------------

local function readDir(path)
	local success, dir = pcall(dir, path)
	if success then
		return function()
			local e
			repeat
				e = dir()
			until e ~= "." and e ~= ".."
			return e
		end
	end
end

-------------------------------------------------------------------------------
--	splitPath: splits a path, returning a path and a path/file part
-------------------------------------------------------------------------------

local function splitPath(path)
	local part
	path, part = (path .. "/"):match("^(/?.-)/*([^/]*)/+$")
	path = path:gsub("//*", "/")
	return path, part
end

-------------------------------------------------------------------------------
--	DirList:
-------------------------------------------------------------------------------

function DirList.new(class, self)

	self = self or { }

	self.Path = self.Path or ""
	self.Location = self.Location or ""

	-- Style: "requester", "lister", "simple"
	self.Style = self.Style or "lister"

	self.Selection = { }
	-- Status: "running", "cancelled", "selected"
	self.Status = "running"

	self.ScanMode = false
	self.DirList = false

	self.PathField = TextInput:new
	{
		KeyCode = "d",
		Text = self.Path
	}

	self.PathField:addNotify("Enter", ui.NOTIFY_ALWAYS,
		{ self, "scanDir", ui.NOTIFY_VALUE })

	self.ParentButton = Text:new
	{
		Text = "_Parent",
		Mode = "button",
		Style = "button",
		Width = "auto",
	}

	self.ParentButton:addNotify("Pressed", false,
		{ self, "goParent" })

	self.LocationField = TextInput:new
	{
		KeyCode = "f",
		Text = self.Location
	}

	self.LocationField:addNotify("Enter", ui.NOTIFY_ALWAYS,
		{ self, "setFileEntry", ui.NOTIFY_VALUE })

	self.ReloadButton = Text:new
	{
		Text = "_Reload",
		Mode = "button",
		Style = "button",
		Width = "auto",
	}

	self.ReloadButton:addNotify("Pressed", false,
		{ self, ui.NOTIFY_FUNCTION, function(self)
			self:scanDir(self.PathField.Text)
		end })

	self.StatusText = Text:new
	{
		Height = "fill",
		Width = "fill",
	}

	self.ListGadget = ListGadget:new
	{
		AlignColumn = 1,
		SelectMode = self.SelectMode or "single",
		ListObject = self.DirList,
	}

	self.ListGadget:addNotify("Pressed", true,
		{ self, "clickList" })
	self.ListGadget:addNotify("DblClick", true,
		{ self, "dblClickList" })
	self.ListGadget:addNotify("SelectedLine", ui.NOTIFY_ALWAYS,
		{ self, "showStats" })

	self.ListView = ListView:new
	{
		HSliderMode = self.HSliderMode or "off",
		Child = self.ListGadget
	}

	self.OpenGadget = Text:new
	{
		Text = "_Open",
		Mode = "button",
		Style = "button",
		Width = "fill",
	}

	self.OpenGadget:addNotify("Pressed", false,
		{ self, ui.NOTIFY_FUNCTION, function(self)
			local list = self.ListGadget
			local sel = self.Selection
			for line in pairs(list.SelectedLines) do
				local entry = list:getItem(line)
				insert(sel, entry[1][1])
			end
			self:setValue("Status", "selected")
		end })

	self.CancelGadget = Text:new
	{
		Text = "_Cancel",
		Mode = "button",
		Style = "button",
		Width = "fill",
	}

	self.CancelGadget:addNotify("Pressed", false,
		{ self, ui.NOTIFY_FUNCTION, function(self)
			self:setValue("Status", "cancelled")
		end })


	self.Orientation = "vertical"

	if self.Style == "requester" then

		self.Children =
		{
			Group:new
			{
				Orientation = "horizontal",
				Children =
				{
					Text:new
					{
						Text = "_Directory:",
						Width = "auto",
						Style = "caption",
					},
					self.PathField,
					self.ParentButton,
				}
			},
			self.ListView,
			Group:new
			{
				Width = "fill",
				GridWidth = 2,
				Children =
				{
					Group:new
					{
						Width = "fill",
						Children =
						{
							Text:new
							{
								Text = "_Location:",
								Width = "auto",
								Style = "caption",
							},
							self.LocationField,
						}
					},
					self.OpenGadget,
					Group:new
					{
						Width = "fill",
						Children =
						{
							self.ReloadButton,
							self.StatusText,
						}
					},
					self.CancelGadget
				}
			}
		}

	elseif self.Style == "lister" then

		self.Children =
		{
			Group:new
			{
				Orientation = "horizontal",
				Children =
				{
					Text:new
					{
						Text = "_Dir:",
						Width = "auto",
						Style = "caption",
					},
					self.PathField,
					self.ParentButton,
				}
			},
			self.ListView
		}

	else

		self.Children =
		{
			self.ListView
		}

	end

	self = Group.new(class, self)

	self:addNotify("Path", ui.NOTIFY_ALWAYS,
		{ self.PathField, "setValue", "Enter", ui.NOTIFY_VALUE })

	return self
end

function DirList:showStats(selected, total)
	local list = self.ListGadget
	selected = selected or list.NumSelectedLines
	total = total or list:getN()
	self.StatusText:setValue("Text",
		("%d/%d selected"):format(selected, total))
end

-------------------------------------------------------------------------------
--	DirList:abortScan(): This function aborts the coroutine which is
--	currently scanning the directory. The caller of this function must
--	be running in its own coroutine.
-------------------------------------------------------------------------------

function DirList:abortScan()
	-- if another coroutine is already scanning, abort it:
	while self.ScanMode do
		self.ScanMode = "abort"
		self.Application:suspend()
	end
end

function DirList:scanDir(path)
	local app = self.Application

	app:addCoroutine(function()

		self:abortScan()

		self.ScanMode = "scanning"

		local obj = self.ListGadget
		path = path == "" and "." or path
		obj:setValue("CursorLine", 0)
		obj:setList(List:new())

		diri = readDir(path)
		if diri then
			local list = { }
			local n = 0

			for name in diri do

				if n % 50 == 0 then
					self:showStats(0, n)
					app:suspend()
					if self.ScanMode ~= "scanning" then
						db.warn("scan aborted")
						self.ScanMode = false
						return
					end
				end
				n = n + 1

				local fullname = path .. "/" .. name
				local isdir = stat(fullname, "mode") == "directory"

				insert(list,
				{
					{
						name,
						isdir and "[Directory]" or stat(fullname, "size")
					},
					isdir
				})
			end

			sort(list, function(a, b)
				if a[2] == b[2] then
					return a[1][1]:lower() < b[1][1]:lower()
				end
				return a[2]
			end)

			self:showStats(0, n)
			app:suspend()

			for i = 1, #list do
				obj:addItem(list[i], nil, true)
			end

			obj:prepare(true)
			obj:setValue("CursorLine", 1)
			obj:setValue("Focus", true)
			self:showStats()
		end

		self.ScanMode = false

	end)

end

-------------------------------------------------------------------------------
--	DirList:showDirectory(path): Starts reading and displays the specified
--	directory.
-------------------------------------------------------------------------------

function DirList:showDirectory(path)
	path = path or self.Path
	self.Path = path
	local pathfield = self.PathField
	local locationfield = self.LocationField
	locationfield:setValue("Text", "")
-- 	pathfield:setValue("Text", path)
	pathfield:setValue("Enter", path)
end

-------------------------------------------------------------------------------
--	DirList:goParent(): Starts reading and displays the parent directory
--	of the current directory.
-------------------------------------------------------------------------------

function DirList:goParent()
	self:showDirectory(splitPath(self.PathField.Text))
end

function DirList:setFileEntry(entry)
	local list = self.ListGadget
	local pathfield = self.PathField
	local path = pathfield.Text:match("(.*[^/])/?$") or ""
	local fullpath = pathfield.Text .. "/" .. entry
	fullpath = fullpath:gsub("//*", "/")
	if stat(fullpath, "mode") == "directory" then
		self:showDirectory(fullpath)
		return true -- is directory
	end
end

function DirList:clickList()
	local list = self.ListGadget
	local locationfield = self.LocationField
	local entry = list:getItem(list.CursorLine)
	if entry then
		locationfield:setValue("Text", entry[1][1])
	end
end

function DirList:dblClickList()
	local list = self.ListGadget
	local entry = list:getItem(list.CursorLine)
	if entry then
		if not self:setFileEntry(entry[1][1]) then
			-- file doubleclicked, reactivate selection:
			if self.SelectMode == "single" then
				list:setValue("SelectedLine", list.CursorLine)
			end
			-- click on "Open":
			self.Window:clickElement(self.OpenGadget)
		end
	end
end
