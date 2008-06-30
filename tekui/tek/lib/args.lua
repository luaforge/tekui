-------------------------------------------------------------------------------
--
--	tek.lib.args
--	Written by Timm S. Mueller <tmueller at schulze-mueller.de>
--	See copyright notice in COPYRIGHT
--
--	OVERVIEW::
--	This library implements an argument parser.
--
--	FORMAT DESCRIPTION::
--	A string is parsed into an array of arguments, according to a format
--	template. Arguments in the template are separated by commas. Each
--	argument in the template consists of a keyword, optionally followed by
--	one or more aliases delimited by equal signs, and an optional set of
--	modifiers delimited by slashes. Modifiers denote a) the expected datatype,
--	b) if the argument is mandatory and c) whether a keyword must precede its
--	value to form a valid argument. Example argument template:
--
--			SOURCE=-s/A/M,DEST=-d/A/K
--
--	This template would require one or more arguments to satisfy {{SOURCE}},
--	and exactly one argument to satisfy {{DEST}}. Neither can be omitted.
--	Either {{-d}} or {{DEST}} must precede a value to be accepted as the
--	second argument. Examples:
--
--	{{SOURCE one two three DEST foo}} || valid
--	{{DEST foo -s one}}               || valid
--	{{DEST foo}}                      || rejected; source argument missing
--	{{one two three foo}}             || rejected; keyword missing
--	{{one two dest foo}}              || valid; keywords are case insensitive
--	{{one two three -d="foo" four}}   || valid; "four" added to {{SOURCE}}
--
--	An option without modifiers represents an optional string argument.
--	Available modifiers are:
--
--		* {{/S}} - ''Switch''; considered a boolean value. When the keyword is
--		present, '''true''' will be written into the respective slot in the
--		results array.
--		* {{/N}} - ''Number''; the value will be converted to a number.
--		* {{/K}} - ''Keyword''; the keyword (or an alias) must precede its value.
--		* {{/A}} - ''Required''; This argument cannot be omitted.
--		* {{/M}} - ''Multiple''; Any number of strings will be accepted, and all
--		values that cannot be assigned to other arguments will show up in this
--		argument. No more than one {{/M}} modifier may appear in a template.
--		* {{/F}} - ''Fill''; literal rest of line. If present, this must be the
--		last argument.
--
--	Quoting and escaping: Double quotes can be used to enclose arguments
--	containing equal signs and spaces. [TODO: Backslashes can be used for
--	escaping quotes, spaces and themselves (besides everything else)].
--
-------------------------------------------------------------------------------

local type = type
local insert, remove, concat = table.insert, table.remove, table.concat
local ipairs = ipairs
local pairs = pairs
local print = print
local tonumber = tonumber

module "tek.lib.args"
_VERSION = "Args 1.4"

local function checkfl(self, fl)
	for s in fl:gmatch(".") do
		if self.flags:find(s, 1, true) then
			return true
		end
	end
end

-------------------------------------------------------------------------------
--	parsetemplate: check validity and parse option template into a table
--	with records of flags and keys
-------------------------------------------------------------------------------

local function parsetemplate(t)
	if type(t) ~= "string" then
		return nil, "Invalid template"
	end
	local template = { }
	local arg, key, keys, modif
	repeat
		arg, t = t:match("^([%a%-_][%w%-=_/]*),?(.*)")
		if not t then
			return nil, "Invalid characters in template"
		end
		keys, mod = arg:match("^([^/]*)(/?[/aksnmfAKSNMF]*)$")
		if not keys then
			return nil, "Invalid modifiers in template"
		end
		local record = { flags = mod:lower():gsub("/", ""),
			keys = { }, keymulti = {}, check = checkfl }
		repeat
			key, keys = keys:match("^([^=/]+)=?(.*)")
			if not key then
				return nil, "Invalid option name"
			end
			key = key:lower()
			insert(record.keys, key)
			template[key] = record
		until keys == ""
		insert(template, record)
		record.idx = #template
	until t == ""
	-- extra item at the end to collect multiple arguments:
	insert(template, { flags = "m", keys = { }, check = checkfl })
	return template
end

-------------------------------------------------------------------------------
--	items: iterator over items in argument string or table
-------------------------------------------------------------------------------

local function items(rest)
	if type(rest) == "table" then
		rest = concat(rest, " ")
	elseif type(rest) ~= "string" then
		return nil, "Invalid arguments"
	end
	return function()
		if rest then
			local oldrest = rest
			local arg, newa = rest:match('^%s*"([^"]*)"%s*=?%s*(.*)')
			if not newa then
				arg, newa = rest:match("^%s*([^%s=]+)%s*=?%s*(.*)")
			end
			rest = newa ~= "" and newa or nil
			return arg, oldrest
		end
	end
end

-------------------------------------------------------------------------------
--	res, msg = read(template, args): Parses {{args}} according to
--	{{template}}. {{args}} can be a string or a table of strings. If the
--	arguments can be successfully matched against the template (see
--	[[#tek.lib.args]] for details), the result will be a table of parsed
--	arguments, indexed by both keywords and numerical order in which they
--	appear in {{template}}). If the arguments cannot be matched against the
--	template, the result will be '''nil''' followed by an error message.
-------------------------------------------------------------------------------

function read(tmpl, args)

	local tmpl, msg = parsetemplate(tmpl)
	if not tmpl then
		return nil, msg
	end

	local nextitem, msg = items(args)
	if not nextitem then
		return nil, msg
	end

	local nextn, n = 1
	local item, rest, done, waitkey
	local multi = { }

	repeat
		while true do
			n = nextn
			nextn = n + 1

			local tt = tmpl[n]
			if not tt then
				done = true
				break
			end

			if tt.arg then
				break -- continue
			end

			if tt:check("ks") then
				waitkey = tt.idx
				break -- continue waiting for argument
			else
				waitkey = false
			end

			item, rest = nextitem()
			if not item then
				done = true -- no more items
				break
			end

			local rec = tmpl[item:lower()]
			if rec then
				waitkey = true
			end

			if rec and not rec.arg then
				nextn = n -- retry current in next turn
				n = rec.idx
				tt = tmpl[n]
				if not rec:check("s") then
					item, rest = nextitem()
					if not item then
						done = true
						break
					end
				end
			elseif waitkey then
				return nil, "key expected"
			end

			if tt:check("m") then
				if waitkey or tt:check("k") then
					insert(tt.keymulti, item)
				else
					insert(multi, item)
				end
				nextn = n -- retry
			elseif tt:check("f") then
				tt.arg = rest
				done = true
				break
			elseif tt:check("n") then
				tt.arg = tonumber(item)
			elseif tt:check("s") then
				tt.arg = true
			else
				tt.arg = item
			end

			waitkey = false

		end
	until done

	-- unfilled /A steal from multi:
	for i = #tmpl, 1, -1 do
		local tt = tmpl[i]
		if not tt.arg and tt:check("a") and not tt:check("m") then
			if not tt:check("k") then
				tt.arg = remove(multi)
			end
			if not tt.arg then
				return nil, "Required argument missing (1)"
			end
		end
	end

	-- put remaining multi into /M:
	for i = 1, #tmpl - 1 do
		local tt = tmpl[i]
		if tt.flags:match("[m]") then
			tt.arg = tt.keymulti
			if not tt:check("k") then
				for key, val in ipairs(multi) do
					insert(tt.arg, val)
				end
			end
			if tt:check("a") and #tt.arg == 0 then
				return nil, "Required argument missing (2)"
			end
		end
	end

	-- arguments left?
	if n == #tmpl and #multi > 0 then
		return nil, "Too many arguments"
	end

	local res = { }
	for i, tt in ipairs(tmpl) do
		res[i] = tt.arg
		if tt.keys[1] then
			res[tt.keys[1]] = tt.arg
		end
	end

	return res
end

-------------------------------------------------------------------------------
--	unit tests:
-------------------------------------------------------------------------------

-- local function test(template, args, expected)
-- 	if expected == false then
-- 		expected = nil
-- 	end
-- 	local function result(status)
-- 		print(status .. ': argparse("'..template..'", "'..args..'")')
-- 		return status
-- 	end
-- 	local res = read(template, args)
-- 	if res == expected then
-- 		return result("ok")
-- 	elseif not res or not expected then
-- 		return result("failed")
-- 	else
-- 		for key, val in pairs(expected) do
-- 			if type(val) == "table" then
-- 				if #val ~= #res[key] then
-- 					return result("failed")
-- 				end
-- 				for i, v in ipairs(val) do
-- 					if res[key][i] ~= v then
-- 						return result("failed")
-- 					end
-- 				end
-- 			else
-- 				if res[key] ~= val then
-- 					return result("failed")
-- 				end
-- 			end
-- 		end
-- 		return result("ok")
-- 	end
-- end
--
-- test("bla,fasel", "1", {bla="1"})
-- test("bla,fasel", "1 2", {bla="1",fasel="2"})
-- test("bla,fasel", "1 2 3", false)
-- test("src=-s/m/a,dst=-d/a", "1 2 3 -s 4", {src={ "4","1","2"},dst="3"})
-- test("bla,fasel", "bla fasel fasel bla", {bla="fasel",fasel="bla"})
-- test("bla,fasel", "bla fasel fasel bla blub", false)
-- test("src/m/a,dst/a", "1", false)
-- test("src/m/a,dst/a", "1 2", {src={"1"},dst="2"})
-- test("src/m/a,dst/a", "1 2 3", {src={"1","2"},dst="3"})
-- test("src=-s/m/a,dst=-d/a", "-s 1 -s 2 -d 3 4", {src={"1","2","4"},dst="3"})
-- test("src=-s/m/a/k,dst=-d/a/k", "-s 1 -s 2 -d 3", {src={"1","2"},dst="3"})
-- test("src=-s/m/a/k,dst=-d/a/k", "-s 1 -s 2 3", false)
-- test("src=-s/m/a/k,dst=-d/a/k", "-s 1 2 -d 3", false)
-- test("bla/a,fasel/a", "1", false)
-- test("bla/a,fasel/a", "1 2", {bla="1",fasel="2"})
-- test("bla/a/k,fasel/a/k", "bla 1 fasel 2", {bla="1",fasel="2"})
-- test("bla/a/k,fasel/a/k", "bla 1 fasel", false)
-- test("bla/a/k,fasel/a", "bla 1 fasel", false)
-- test("SOURCE=-s/A/M,DEST=-d/A/K", "SOURCE one two three DEST foo",
-- 	{source={"one","two","three"},dest="foo"})
-- test("SOURCE=-s/A/M,DEST=-d/A/K", "DEST foo -s one",
-- 	{source={"one"},dest="foo"})
-- test("SOURCE=-s/A/M,DEST=-d/A/K", "one two three foo", false)
-- test("SOURCE=-s/A/M,DEST=-d/A/K", "one two dest foo",
-- 	{source={"one","two"},dest="foo"})
-- test("SOURCE=-s/A/M,DEST=-d/A/K", 'one two three -d="foo" four',
-- 	{source={"one","two","three","four"},dest="foo"})
-- test("a/s,b/s", "a b", {a=true, b=true})
-- test("file/k,bla/k", "file foo bla fasel", {file=foo, bla=fasel})
-- test("file/k,bla/k,a/s", "file foo bla fasel", {file=foo, bla=fasel})
-- test("file=-f/k,bla/k,a/s", "-f foo bla fasel a", {file=foo, bla=fasel, a=true})
-- test("file=-f/k,bla/k,a/s", "-f foo a", {file=foo, a=true})
-- test("file=-f/k,bla/k,a/s", "bla fasel a", {bla=fasel, a=true})
-- test("file=-f/k,silent=-s/s,targets/m", "-f file -s", {file=file, silent=true, targets={}})
-- test("file=-f/k,silent=-s/s,targets/m", "-f file -s eins", {file=file, silent=true, targets={"eins"}})
-- test("file=-f/k,silent=-s/s,targets/m", "-f file eins", {file=file, targets={"eins"}})
-- test("file=-f,silent=-s/s,targets/m", "eins", {file="eins"})
-- test("file=-f/k,silent=-s/s,targets/m", "targets eins", {targets={"eins"}})
-- test("file=-f/k,silent=-s/s,targets/m", "eins", {targets={"eins"}})
-- test("file=-f/k,silent=-s/s,targets/m", "eins", {targets={"eins"}})
-- test("file=-f/k,silent=-s/s,targets/m", "eins -f file", {file=file, targets={"eins"}})

-------------------------------------------------------------------------------
--	Test tool:
-------------------------------------------------------------------------------
--
-- template = "-h=HELP/S,-t=TEMPLATE/A,-a=ARGS/F"
-- args, msg = argparse(template, table.concat(arg, " "))
-- if not args or args[1] == true then
-- 	print("Argument parser test tool")
-- 	print("Usage: " .. template)
-- 	if msg then
-- 		print("*** " .. msg)
-- 	end
-- 	return
-- end
--
-- template = args[2]
-- argstring = args[3]
-- print("=======================================")
-- args, msg = argparse(template, argstring)
-- if args then
-- 	local _, numarg = template:gsub(",", ",")
-- 	numarg = numarg + 1
-- 	for i = 1, numarg do
-- 		local v = args[i]
-- 		if type(v) == "table" then
-- 			for _, v in ipairs(v) do
-- 				print("multi:", v)
-- 			end
-- 		elseif type(v) == "number" then
-- 			print("number:", v)
-- 		elseif type(v) == "boolean" then
-- 			print("bool:", v)
-- 		elseif type(v) == "string" then
-- 			print("string:", v)
-- 		else
-- 			print("not set")
-- 		end
-- 	end
-- else
-- 	print("*** Arguments do not match template " .. template)
-- 	if msg then
-- 		print("*** " .. msg)
-- 	end
-- end
