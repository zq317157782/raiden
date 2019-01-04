local pbrt2lua_curlyhair = {}

local file
local line
local index

function string:split(sep)
    if sep == nil then
        sep = '%s'
    end
    local t = {}
    local i = 1
    for str in string.gmatch(self, '([^' .. sep .. ']+)') do
        t[i] = str
        i = i + 1
    end
    return t
end

local function getline()
    line = file:read('l')
    index = 1
end

local function getfunctionname()
    if line == nil or index >= #line then
        getline()
        if line == nil then
            return nil
        end
    end
    local s, e = line:find('%a+', index)
    index = e + 1
    return line:sub(s, e)
end

local function gettype()
    local c = line:match('%g', index)
    if c == '"' then
        local s, e = line:find('"%a+"', index)
        index = e + 1
        return line:sub(s + 1, e - 1)
    else
        return nil
    end
end

local function getparams()
    local c = line:match('%g', index)
    if c == '"' then
        local params = {}
        while true do
            local s, e = line:find('%[', index)
            if s == nil then
                break
            end
            while true do
                s, e = line:find('%]', index)
                if s == nil then
                    line = line .. " " .. file:read("l")
                else
                    break
                end
            end
            local str = line:sub(index + 1, e)
            index = e + 1
            local type, name = str:match('"([%a%d]+) ([%a%d]+)"')
            local value = str:match('%[ (.*) %]')
            value = value:split(" ")
            for i, v in ipairs(value) do
                if v:sub(1,1) == "\"" then
                    value[i] = v:sub(2, -2)
                else
                    value[i] = tonumber(v)
                end
            end
            local param = {type = type, name = name, value = value}
            params[name]= param
        end
        return params
    else
        return nil
    end
end

function pbrt2lua_curlyhair.parse(filename)
    file = io.open(filename, 'r')
end

function pbrt2lua_curlyhair.close()
    file:close()
end

function pbrt2lua_curlyhair.next()
    local name = getfunctionname()
    if name == nil then
        return nil
    end
    local type = gettype()
    local params = {}
    if type then
        params = getparams()
    end
    local call = {
        name = name,
        type = type,
        params = params
    }
    return call
end

return pbrt2lua_curlyhair
